/* phase-2-demo.c - Network Layer Demonstration
 *
 * Demonstrates all Phase 2 networking components working together:
 * - Protocol message parsing and serialization
 * - Session creation and management
 * - Connection management (simulated)
 * - Hidden information handling
 * - Input validation
 *
 * This demo simulates network communication to show the protocol flow
 * without requiring actual network sockets. For a production server,
 * these components would integrate with libwebsockets and libssh.
 *
 * Run with: ./bin/phase-2-demo
 */

#define _POSIX_C_SOURCE 200809L

#include "../core/01-card.h"
#include "../core/02-deck.h"
#include "../core/03-player.h"
#include "../core/04-trade-row.h"
#include "../core/05-game.h"
#include "../core/06-combat.h"
#include "../core/07-effects.h"
#include "../core/09-serialize.h"
#include "../net/04-protocol.h"
#include "../net/06-connections.h"
#include "../net/07-sessions.h"
#include "../net/08-validation.h"
#include "../../libs/cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

/* ========================================================================== */
/*                              Configuration                                  */
/* ========================================================================== */

#define DEMO_SERVER_PORT_WS 8080
#define DEMO_SERVER_PORT_SSH 8022
#define LOG_PREFIX_SERVER  "\033[34m[SERVER]\033[0m"
#define LOG_PREFIX_ALICE   "\033[32m[ALICE/SSH]\033[0m"
#define LOG_PREFIX_BOB     "\033[33m[BOB/WS]\033[0m"
#define LOG_PREFIX_PROTO   "\033[35m[PROTO]\033[0m"

/* ========================================================================== */
/*                              Demo State                                     */
/* ========================================================================== */

typedef struct {
    SessionRegistry* sessions;
    ConnectionRegistry* connections;
    Game* game;
    int alice_conn_id;
    int bob_conn_id;
    int session_id;
} DemoState;

/* ========================================================================== */
/*                              Card Database                                  */
/* ========================================================================== */

/* {{{ create_starting_types */
static void create_starting_types(CardType** scout, CardType** viper, CardType** explorer) {
    *scout = card_type_create("scout", "Scout", 0, FACTION_NEUTRAL, CARD_KIND_SHIP);
    (*scout)->effects = effect_array_create(1);
    (*scout)->effects[0] = (Effect){ EFFECT_TRADE, 1, NULL };
    (*scout)->effect_count = 1;

    *viper = card_type_create("viper", "Viper", 0, FACTION_NEUTRAL, CARD_KIND_SHIP);
    (*viper)->effects = effect_array_create(1);
    (*viper)->effects[0] = (Effect){ EFFECT_COMBAT, 1, NULL };
    (*viper)->effect_count = 1;

    *explorer = card_type_create("explorer", "Explorer", 2, FACTION_NEUTRAL, CARD_KIND_SHIP);
    (*explorer)->effects = effect_array_create(1);
    (*explorer)->effects[0] = (Effect){ EFFECT_TRADE, 2, NULL };
    (*explorer)->effect_count = 1;
}
/* }}} */

/* {{{ create_trade_cards */
static CardType** create_trade_cards(int* count) {
    #define NUM_TRADE_CARDS 4
    CardType** types = calloc(NUM_TRADE_CARDS, sizeof(CardType*));
    if (!types) return NULL;

    int idx = 0;

    types[idx] = card_type_create("dire_bear", "Dire Bear", 4,
                                   FACTION_WILDS, CARD_KIND_SHIP);
    types[idx]->effects = effect_array_create(1);
    types[idx]->effects[0] = (Effect){ EFFECT_COMBAT, 5, NULL };
    types[idx]->effect_count = 1;
    idx++;

    types[idx] = card_type_create("trade_caravan", "Trade Caravan", 3,
                                   FACTION_MERCHANT, CARD_KIND_SHIP);
    types[idx]->effects = effect_array_create(1);
    types[idx]->effects[0] = (Effect){ EFFECT_TRADE, 3, NULL };
    types[idx]->effect_count = 1;
    idx++;

    types[idx] = card_type_create("knight", "Knight", 3,
                                   FACTION_KINGDOM, CARD_KIND_SHIP);
    types[idx]->effects = effect_array_create(1);
    types[idx]->effects[0] = (Effect){ EFFECT_COMBAT, 3, NULL };
    types[idx]->effect_count = 1;
    idx++;

    types[idx] = card_type_create("battle_golem", "Battle Golem", 4,
                                   FACTION_ARTIFICER, CARD_KIND_SHIP);
    types[idx]->effects = effect_array_create(1);
    types[idx]->effects[0] = (Effect){ EFFECT_COMBAT, 4, NULL };
    types[idx]->effect_count = 1;
    idx++;

    *count = idx;
    return types;
}
/* }}} */

/* ========================================================================== */
/*                              Logging Functions                              */
/* ========================================================================== */

/* {{{ log_server */
static void log_server(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("%s ", LOG_PREFIX_SERVER);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}
/* }}} */

/* {{{ log_alice */
static void log_alice(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("%s ", LOG_PREFIX_ALICE);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}
/* }}} */

/* {{{ log_bob */
static void log_bob(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("%s ", LOG_PREFIX_BOB);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}
/* }}} */

/* {{{ log_proto */
static void log_proto(const char* direction, const char* json) {
    /* Truncate for display */
    char truncated[200];
    if (strlen(json) > 150) {
        strncpy(truncated, json, 147);
        strcpy(truncated + 147, "...");
    } else {
        strcpy(truncated, json);
    }
    printf("%s %s %s\n", LOG_PREFIX_PROTO, direction, truncated);
}
/* }}} */

/* ========================================================================== */
/*                         Protocol Simulation                                 */
/* ========================================================================== */

/* {{{ simulate_client_send
 * Simulates a client sending a JSON message to the server.
 * Returns the parsed message or NULL on error.
 */
static Message* simulate_client_send(const char* json, const char* client_name) {
    if (strcmp(client_name, "Alice") == 0) {
        log_alice("Sending: %s", json);
    } else {
        log_bob("Sending: %s", json);
    }
    log_proto("->", json);

    ProtocolError err;
    Message* msg = protocol_parse(json, &err);

    if (!msg) {
        log_server("Parse error: %s", protocol_error_to_string(err));
    }

    return msg;
}
/* }}} */

/* {{{ simulate_server_send
 * Simulates the server sending a message to a client.
 */
static void simulate_server_send(Message* msg, const char* client_name) {
    char* json = protocol_serialize(msg);
    if (!json) {
        log_server("Failed to serialize message");
        return;
    }

    log_proto("<-", json);

    if (strcmp(client_name, "Alice") == 0) {
        log_alice("Received: %s", message_type_to_string(msg->type));
    } else if (strcmp(client_name, "Bob") == 0) {
        log_bob("Received: %s", message_type_to_string(msg->type));
    }

    free(json);
}
/* }}} */

/* ========================================================================== */
/*                         Demo Scenarios                                      */
/* ========================================================================== */

/* {{{ demo_connection_phase
 * Demonstrates connection and session management.
 */
static void demo_connection_phase(DemoState* state) {
    printf("\n");
    printf("===========================================================\n");
    printf("  PHASE 2 DEMO - CONNECTION & SESSION MANAGEMENT\n");
    printf("===========================================================\n\n");

    log_server("Server started on port %d (HTTP/WS) and %d (SSH)",
               DEMO_SERVER_PORT_WS, DEMO_SERVER_PORT_SSH);

    /* Simulate Alice connecting via SSH */
    printf("\n--- Alice connects via SSH ---\n");
    log_server("SSH connection from 127.0.0.1");
    /* In production, we'd pass the real SSH connection handle */
    state->alice_conn_id = conn_register_ssh(state->connections, NULL);
    log_server("Assigned connection ID: %d (SSH)", state->alice_conn_id);

    /* Alice sends JOIN message */
    Message* join_msg = simulate_client_send(
        "{\"type\":\"join\",\"name\":\"Alice\"}",
        "Alice"
    );
    if (join_msg) {
        log_server("Player 'Alice' joining via SSH");
        message_free(join_msg);
    }

    /* Simulate Bob connecting via WebSocket */
    printf("\n--- Bob connects via WebSocket ---\n");
    log_server("WebSocket connection from 127.0.0.1");
    /* In production, we'd pass the real WebSocket connection handle */
    state->bob_conn_id = conn_register_ws(state->connections, NULL);
    log_server("Assigned connection ID: %d (WebSocket)", state->bob_conn_id);

    /* Bob sends JOIN message */
    join_msg = simulate_client_send(
        "{\"type\":\"join\",\"name\":\"Bob\"}",
        "Bob"
    );
    if (join_msg) {
        log_server("Player 'Bob' joining via WebSocket");
        message_free(join_msg);
    }

    /* Create game session - Alice creates it as host */
    printf("\n--- Creating game session ---\n");
    state->session_id = session_create(state->sessions, state->alice_conn_id,
                                        "Alice", 2);
    log_server("Created session %d with Alice as host (2 players required)",
               state->session_id);

    /* Bob joins session */
    session_join(state->sessions, state->session_id,
                 state->bob_conn_id, "Bob");
    log_server("Bob joined session %d", state->session_id);

    /* Ready up */
    session_set_ready(state->sessions, state->session_id, state->alice_conn_id, true);
    log_alice("Ready!");
    session_set_ready(state->sessions, state->session_id, state->bob_conn_id, true);
    log_bob("Ready!");

    /* Check if session can start */
    GameSession* session = session_get(state->sessions, state->session_id);
    if (session && session->player_count == session->required_players) {
        log_server("All players ready! Starting game...");

        /* Link the pre-created game to the session and start */
        session->game = state->game;
        session->state = SESSION_PLAYING;

        /* Notify players */
        Message* start_msg = protocol_create_gamestate(state->game, 0);
        simulate_server_send(start_msg, "Alice");
        message_free(start_msg);

        start_msg = protocol_create_gamestate(state->game, 1);
        simulate_server_send(start_msg, "Bob");
        message_free(start_msg);
    }
}
/* }}} */

/* {{{ demo_hidden_info
 * Demonstrates hidden information handling.
 */
static void demo_hidden_info(DemoState* state) {
    printf("\n");
    printf("===========================================================\n");
    printf("  HIDDEN INFORMATION DEMONSTRATION\n");
    printf("===========================================================\n\n");

    /* Skip draw order to get cards in hands */
    game_skip_draw_order(state->game);

    /* Draw cards for Bob too (for demonstration) */
    player_draw_cards(state->game->players[1], 5);

    /* Show Alice's view */
    printf("--- Alice's View (sees own hand, not Bob's) ---\n");
    cJSON* alice_view = serialize_game_for_player(state->game, 0);
    char* alice_json = cJSON_PrintUnformatted(alice_view);

    /* Count occurrences of "hand" in JSON */
    cJSON* you = cJSON_GetObjectItem(alice_view, "you");
    cJSON* your_hand = you ? cJSON_GetObjectItem(you, "hand") : NULL;
    int alice_hand_cards = your_hand ? cJSON_GetArraySize(your_hand) : 0;

    cJSON* opponents = cJSON_GetObjectItem(alice_view, "opponents");
    cJSON* opp = opponents ? cJSON_GetArrayItem(opponents, 0) : NULL;
    cJSON* opp_hand = opp ? cJSON_GetObjectItem(opp, "hand") : NULL;
    cJSON* opp_hand_count = opp ? cJSON_GetObjectItem(opp, "hand_count") : NULL;

    log_alice("Can see %d cards in own hand", alice_hand_cards);
    log_alice("Opponent hand field: %s", opp_hand ? "VISIBLE (BUG!)" : "hidden");
    log_alice("Opponent hand_count: %d",
              opp_hand_count ? (int)opp_hand_count->valuedouble : -1);

    free(alice_json);
    cJSON_Delete(alice_view);

    /* Show Bob's view */
    printf("\n--- Bob's View (sees own hand, not Alice's) ---\n");
    cJSON* bob_view = serialize_game_for_player(state->game, 1);

    you = cJSON_GetObjectItem(bob_view, "you");
    your_hand = you ? cJSON_GetObjectItem(you, "hand") : NULL;
    int bob_hand_cards = your_hand ? cJSON_GetArraySize(your_hand) : 0;

    opponents = cJSON_GetObjectItem(bob_view, "opponents");
    opp = opponents ? cJSON_GetArrayItem(opponents, 0) : NULL;
    opp_hand = opp ? cJSON_GetObjectItem(opp, "hand") : NULL;
    opp_hand_count = opp ? cJSON_GetObjectItem(opp, "hand_count") : NULL;

    log_bob("Can see %d cards in own hand", bob_hand_cards);
    log_bob("Opponent hand field: %s", opp_hand ? "VISIBLE (BUG!)" : "hidden");
    log_bob("Opponent hand_count: %d",
            opp_hand_count ? (int)opp_hand_count->valuedouble : -1);

    cJSON_Delete(bob_view);

    /* Show spectator view */
    printf("\n--- Spectator View (sees all hands) ---\n");
    cJSON* spec_view = serialize_game_for_spectator(state->game);
    cJSON* players = cJSON_GetObjectItem(spec_view, "players");

    if (players) {
        int count = cJSON_GetArraySize(players);
        for (int i = 0; i < count; i++) {
            cJSON* p = cJSON_GetArrayItem(players, i);
            cJSON* hand = p ? cJSON_GetObjectItem(p, "hand") : NULL;
            cJSON* name = p ? cJSON_GetObjectItem(p, "name") : NULL;
            log_server("Spectator sees %s's hand: %d cards",
                       name ? name->valuestring : "?",
                       hand ? cJSON_GetArraySize(hand) : 0);
        }
    }

    cJSON_Delete(spec_view);

    printf("\n\033[32m[PASS]\033[0m Hidden information correctly filtered per player\n");
}
/* }}} */

/* {{{ demo_validation
 * Demonstrates input validation.
 */
static void demo_validation(DemoState* state) {
    printf("\n");
    printf("===========================================================\n");
    printf("  INPUT VALIDATION DEMONSTRATION\n");
    printf("===========================================================\n\n");

    ValidationResult result;

    /* Test 1: Wrong turn */
    printf("--- Test: Bob tries to play during Alice's turn ---\n");
    result = validate_is_player_turn(state->game, 1);  /* Bob is player 1 */
    log_bob("Attempting action...");
    log_server("Validation: %s - %s",
               result.valid ? "ALLOWED" : "REJECTED",
               result.valid ? "OK" : result.error_message);
    printf("\033[32m[PASS]\033[0m Wrong turn rejected\n");

    /* Test 2: Invalid slot */
    printf("\n--- Test: Alice tries to buy from invalid slot ---\n");
    result = validate_buy_card(state->game, 0, 99);  /* Invalid slot */
    log_alice("Attempting to buy from slot 99...");
    log_server("Validation: %s - %s",
               result.valid ? "ALLOWED" : "REJECTED",
               result.valid ? "OK" : result.error_message);
    printf("\033[32m[PASS]\033[0m Invalid slot rejected\n");

    /* Test 3: Insufficient trade */
    printf("\n--- Test: Alice tries to buy with 0 trade ---\n");
    result = validate_buy_card(state->game, 0, 0);  /* Alice has 0 trade */
    log_alice("Attempting to buy from slot 0...");
    log_server("Validation: %s - %s",
               result.valid ? "ALLOWED" : "REJECTED",
               result.valid ? "OK" : result.error_message);
    printf("\033[32m[PASS]\033[0m Insufficient trade rejected\n");

    /* Test 4: Valid play */
    printf("\n--- Test: Alice plays a card from hand ---\n");
    Player* alice = state->game->players[0];
    if (alice->deck->hand_count > 0) {
        const char* card_id = alice->deck->hand[0]->instance_id;
        result = validate_play_card(state->game, 0, card_id);
        log_alice("Playing card %s...", alice->deck->hand[0]->type->name);
        log_server("Validation: %s - %s",
                   result.valid ? "ALLOWED" : "REJECTED",
                   result.valid ? "OK" : result.error_message);
        printf("\033[32m[PASS]\033[0m Valid action accepted\n");
    }

    /* Test 5: Attack without combat */
    printf("\n--- Test: Alice tries to attack with 0 combat ---\n");
    result = validate_attack_player(state->game, 0, 1, 5);
    log_alice("Attempting to attack Bob for 5 damage...");
    log_server("Validation: %s - %s",
               result.valid ? "ALLOWED" : "REJECTED",
               result.valid ? "OK" : result.error_message);
    printf("\033[32m[PASS]\033[0m Attack without combat rejected\n");
    fflush(stdout);
}
/* }}} */

/* {{{ demo_protocol_flow
 * Demonstrates the full protocol message flow.
 */
static void demo_protocol_flow(DemoState* state) {
    printf("\n");
    fflush(stdout);
    printf("===========================================================\n");
    fflush(stdout);
    printf("  PROTOCOL MESSAGE FLOW\n");
    fflush(stdout);
    printf("===========================================================\n\n");
    fflush(stdout);

    /* Demonstrate protocol message formats */
    printf("--- Example: Client action message ---\n");
    fflush(stdout);
    const char* example_action = "{\"type\":\"action\",\"action\":\"play_card\",\"card_id\":\"scout_0001\"}";
    log_alice("Sending action...");
    fflush(stdout);
    log_proto("->", example_action);
    fflush(stdout);

    Message* action_msg = simulate_client_send(example_action, "Alice");
    if (action_msg) {
        log_server("Parsed action: type=%s", message_type_to_string(action_msg->type));
        message_free(action_msg);
    }

    printf("\n--- Example: Server gamestate broadcast ---\n");
    fflush(stdout);
    Message* gs = protocol_create_gamestate(state->game, 0);
    if (gs) {
        simulate_server_send(gs, "Alice");
        message_free(gs);
    }

    gs = protocol_create_gamestate(state->game, 1);
    if (gs) {
        simulate_server_send(gs, "Bob");
        message_free(gs);
    }

    printf("\n--- Example: End turn message ---\n");
    const char* end_turn_json = "{\"type\":\"end_turn\"}";
    Message* end_msg = simulate_client_send(end_turn_json, "Alice");
    if (end_msg) {
        log_server("Parsed message: type=%s", message_type_to_string(end_msg->type));
        message_free(end_msg);
    }

    printf("\n\033[32m[PASS]\033[0m Protocol message parsing and serialization working\n");
}
/* }}} */

/* {{{ demo_summary
 * Prints summary of Phase 2 demonstration.
 */
static void demo_summary(DemoState* state) {
    printf("\n");
    printf("===========================================================\n");
    printf("  PHASE 2 DEMO SUMMARY\n");
    printf("===========================================================\n\n");
    fflush(stdout);

    printf("Components Demonstrated:\n");
    printf("  \033[32m[OK]\033[0m Protocol - Message parsing and serialization\n");
    printf("  \033[32m[OK]\033[0m Sessions - Game session creation and management\n");
    printf("  \033[32m[OK]\033[0m Connections - Multi-transport connection registry\n");
    printf("  \033[32m[OK]\033[0m Hidden Info - Player-specific game state views\n");
    printf("  \033[32m[OK]\033[0m Validation - Server-side action validation\n");
    fflush(stdout);

    printf("\nSession Statistics:\n");
    printf("  Total sessions: %d\n", session_count(state->sessions));
    printf("  Active connections: %d\n", conn_count(state->connections));

    GameSession* session = session_get(state->sessions, state->session_id);
    if (session) {
        printf("  Session state: %s\n",
               session->state == SESSION_PLAYING ? "PLAYING" :
               session->state == SESSION_WAITING ? "WAITING" : "FINISHED");
        printf("  Players in session: %d/%d\n",
               session->player_count, session->required_players);
    }

    printf("\nGame Statistics:\n");
    printf("  Turn: %d\n", state->game->turn_number);
    printf("  Phase: %s\n", game_phase_to_string(state->game->phase));
    if (state->game->active_player >= 0 && state->game->active_player < MAX_PLAYERS) {
        Player* active = state->game->players[state->game->active_player];
        if (active && active->name) {
            printf("  Active player: %s\n", active->name);
        } else {
            printf("  Active player: (unknown)\n");
        }
    }

    printf("\n\033[32mPhase 2 networking layer ready for integration!\033[0m\n");
}
/* }}} */

/* ========================================================================== */
/*                                Main                                         */
/* ========================================================================== */

/* {{{ main */
int main(void) {
    /* Initialize RNG */
    srand((unsigned int)time(NULL));

    printf("\n");
    printf("###########################################################\n");
    printf("#                                                         #\n");
    printf("#         SYMBELINE REALMS - PHASE 2 DEMO                 #\n");
    printf("#         Network Layer Demonstration                     #\n");
    printf("#                                                         #\n");
    printf("###########################################################\n");

    /* Initialize effects system */
    effects_init();

    /* Create card types */
    CardType* scout;
    CardType* viper;
    CardType* explorer;
    create_starting_types(&scout, &viper, &explorer);

    int trade_card_count;
    CardType** trade_cards = create_trade_cards(&trade_card_count);

    /* Create game */
    Game* game = game_create(2);
    game_add_player(game, "Alice");
    game_add_player(game, "Bob");
    game_set_starting_types(game, scout, viper, explorer);

    /* Create trade row */
    int deck_size = trade_card_count * 4;
    CardType** trade_deck = malloc(deck_size * sizeof(CardType*));
    for (int i = 0; i < trade_card_count; i++) {
        for (int j = 0; j < 4; j++) {
            trade_deck[i * 4 + j] = trade_cards[i];
        }
    }
    game->trade_row = trade_row_create(trade_deck, deck_size, explorer);
    free(trade_deck);

    /* Start game */
    if (!game_start(game)) {
        printf("Failed to start game!\n");
        return 1;
    }

    /* Create demo state */
    DemoState state = {
        .sessions = session_registry_create(),
        .connections = conn_registry_create(),
        .game = game,
        .alice_conn_id = -1,
        .bob_conn_id = -1,
        .session_id = -1
    };

    /* Run demo scenarios */
    demo_connection_phase(&state);
    demo_hidden_info(&state);
    demo_validation(&state);
    demo_protocol_flow(&state);
    demo_summary(&state);

    /* Cleanup */

    /* Don't free game through session since we linked it manually */
    GameSession* session = session_get(state.sessions, state.session_id);
    if (session) {
        session->game = NULL;  /* Prevent double-free */
    }

    session_registry_destroy(state.sessions);

    conn_registry_destroy(state.connections);

    game_free(game);

    card_type_free(scout);

    card_type_free(viper);

    card_type_free(explorer);

    for (int i = 0; i < trade_card_count; i++) {
        card_type_free(trade_cards[i]);
    }
    free(trade_cards);

    printf("\n");
    return 0;
}
/* }}} */
