/* phase-3-demo.c - Client Renderer Demonstration
 *
 * Demonstrates all Phase 3 client rendering components:
 * - Terminal (ncurses) UI with split-screen layout
 * - Card rendering with faction colors
 * - Status bar, hand, trade row, bases, narrative panels
 * - Input parsing and command handling
 * - Narrative buffer with scrolling
 *
 * This demo runs in the terminal to show the ncurses-based client.
 * For browser client, open assets/web/index.html after starting server.
 *
 * Run with: ./bin/phase-3-demo
 */

#define _POSIX_C_SOURCE 200809L

#include "../core/01-card.h"
#include "../core/02-deck.h"
#include "../core/03-player.h"
#include "../core/04-trade-row.h"
#include "../core/05-game.h"
#include "../core/06-combat.h"
#include "../core/07-effects.h"
#include "../client/01-terminal.h"
#include "../client/02-terminal-render.h"
#include "../client/03-terminal-input.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

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

/* {{{ create_faction_cards */
static CardType** create_faction_cards(int* count) {
    #define NUM_FACTION_CARDS 8
    CardType** types = calloc(NUM_FACTION_CARDS, sizeof(CardType*));
    if (!types) return NULL;

    int idx = 0;

    /* Merchant Guild cards - yellow/trade focused */
    types[idx] = card_type_create("trade_caravan", "Trade Caravan", 3,
                                   FACTION_MERCHANT, CARD_KIND_SHIP);
    types[idx]->effects = effect_array_create(2);
    types[idx]->effects[0] = (Effect){ EFFECT_TRADE, 3, NULL };
    types[idx]->effects[1] = (Effect){ EFFECT_AUTHORITY, 1, NULL };
    types[idx]->effect_count = 2;
    idx++;

    types[idx] = card_type_create("trading_post", "Trading Post", 4,
                                   FACTION_MERCHANT, CARD_KIND_BASE);
    types[idx]->effects = effect_array_create(1);
    types[idx]->effects[0] = (Effect){ EFFECT_TRADE, 2, NULL };
    types[idx]->effect_count = 1;
    types[idx]->defense = 4;
    idx++;

    /* The Wilds cards - green/combat focused */
    types[idx] = card_type_create("dire_bear", "Dire Bear", 4,
                                   FACTION_WILDS, CARD_KIND_SHIP);
    types[idx]->effects = effect_array_create(1);
    types[idx]->effects[0] = (Effect){ EFFECT_COMBAT, 5, NULL };
    types[idx]->effect_count = 1;
    idx++;

    types[idx] = card_type_create("thornwood_grove", "Thornwood Grove", 3,
                                   FACTION_WILDS, CARD_KIND_BASE);
    types[idx]->effects = effect_array_create(1);
    types[idx]->effects[0] = (Effect){ EFFECT_COMBAT, 2, NULL };
    types[idx]->effect_count = 1;
    types[idx]->defense = 3;
    idx++;

    /* High Kingdom cards - blue/authority focused */
    types[idx] = card_type_create("knight_cmdr", "Knight Commander", 5,
                                   FACTION_KINGDOM, CARD_KIND_SHIP);
    types[idx]->effects = effect_array_create(2);
    types[idx]->effects[0] = (Effect){ EFFECT_COMBAT, 4, NULL };
    types[idx]->effects[1] = (Effect){ EFFECT_AUTHORITY, 3, NULL };
    types[idx]->effect_count = 2;
    idx++;

    types[idx] = card_type_create("watchtower", "Watchtower", 3,
                                   FACTION_KINGDOM, CARD_KIND_BASE);
    types[idx]->effects = effect_array_create(1);
    types[idx]->effects[0] = (Effect){ EFFECT_DRAW, 1, NULL };
    types[idx]->effect_count = 1;
    types[idx]->defense = 4;
    types[idx]->is_outpost = true;
    idx++;

    /* Artificer Order cards - red/tech focused */
    types[idx] = card_type_create("battle_golem", "Battle Golem", 4,
                                   FACTION_ARTIFICER, CARD_KIND_SHIP);
    types[idx]->effects = effect_array_create(1);
    types[idx]->effects[0] = (Effect){ EFFECT_COMBAT, 4, NULL };
    types[idx]->effect_count = 1;
    idx++;

    types[idx] = card_type_create("forge", "The Forge", 5,
                                   FACTION_ARTIFICER, CARD_KIND_BASE);
    types[idx]->effects = effect_array_create(2);
    types[idx]->effects[0] = (Effect){ EFFECT_TRADE, 1, NULL };
    types[idx]->effects[1] = (Effect){ EFFECT_COMBAT, 2, NULL };
    types[idx]->effect_count = 2;
    types[idx]->defense = 5;
    idx++;

    *count = idx;
    return types;
}
/* }}} */

/* ========================================================================== */
/*                              Demo State                                     */
/* ========================================================================== */

typedef struct {
    Game* game;
    TerminalUI* ui;
    NarrativeBuffer* narrative;
    InputHistory* history;
    bool running;
    int demo_step;
    CardType* scout;
    CardType* viper;
    CardType* explorer;
    CardType** faction_cards;
    int faction_card_count;
} DemoState;

/* ========================================================================== */
/*                              Narrative Content                              */
/* ========================================================================== */

static const char* DEMO_NARRATIVE[] = {
    "The morning sun rises over the realm of Symbeline...",
    "",
    "Two great powers marshal their forces for the",
    "contest that will determine the fate of nations.",
    "",
    "In the merchant quarters, caravans load with",
    "precious trade goods. Gold changes hands as",
    "alliances are forged and broken.",
    "",
    "From Thornwood's depths, the beasts stir...",
    "The Dire Bear awakens from its slumber.",
    "",
    "At the High Kingdom's walls, knights prepare",
    "their warhorses. The Watchtower's horn sounds.",
    "",
    "In the Artificer workshops, gears turn and",
    "steam hisses. The Battle Golems march.",
    "",
    "Press 'h' for commands, 'q' to exit demo.",
    "",
    "Try: p 0 (play card), b 0 (buy card), e (end turn)"
};
#define NARRATIVE_LINES (sizeof(DEMO_NARRATIVE) / sizeof(DEMO_NARRATIVE[0]))

/* ========================================================================== */
/*                              Demo Functions                                  */
/* ========================================================================== */

/* {{{ demo_init */
static DemoState* demo_init(void) {
    /* Set locale for UTF-8 support */
    setlocale(LC_ALL, "");

    DemoState* state = calloc(1, sizeof(DemoState));
    if (!state) return NULL;

    /* Initialize effects system */
    effects_init();

    /* Create card types */
    create_starting_types(&state->scout, &state->viper, &state->explorer);
    state->faction_cards = create_faction_cards(&state->faction_card_count);

    /* Create game with 2 players */
    state->game = game_create(2);
    game_add_player(state->game, "You");
    game_add_player(state->game, "Opponent");
    game_set_starting_types(state->game, state->scout, state->viper, state->explorer);

    /* Create trade row with faction cards */
    int deck_size = state->faction_card_count * 3;
    CardType** trade_deck = malloc(deck_size * sizeof(CardType*));
    for (int i = 0; i < state->faction_card_count; i++) {
        for (int j = 0; j < 3; j++) {
            trade_deck[i * 3 + j] = state->faction_cards[i];
        }
    }
    state->game->trade_row = trade_row_create(trade_deck, deck_size, state->explorer);
    free(trade_deck);

    /* Start game and skip to main phase */
    game_start(state->game);
    game_skip_draw_order(state->game);

    /* Add some variety to the demo state */
    Player* player = state->game->players[0];
    Player* opponent = state->game->players[1];

    /* Give player some resources for buying */
    player->trade = 4;
    player->combat = 3;

    /* Give opponent a base to display */
    CardInstance* opp_base = card_instance_create(state->faction_cards[3]); /* Thornwood Grove */
    deck_add_base(opponent->deck, opp_base);

    /* Initialize terminal UI */
    state->ui = terminal_init();
    if (!state->ui) {
        fprintf(stderr, "Failed to initialize terminal\n");
        free(state);
        return NULL;
    }

    /* Create narrative buffer and fill with intro text */
    state->narrative = narrative_buffer_create();
    for (size_t i = 0; i < NARRATIVE_LINES; i++) {
        narrative_buffer_add(state->narrative, DEMO_NARRATIVE[i]);
    }

    /* Create input history */
    state->history = input_history_create();

    state->running = true;
    state->demo_step = 0;

    return state;
}
/* }}} */

/* {{{ demo_cleanup */
static void demo_cleanup(DemoState* state) {
    if (!state) return;

    if (state->history) input_history_free(state->history);
    if (state->narrative) narrative_buffer_free(state->narrative);
    if (state->ui) terminal_cleanup(state->ui);
    if (state->game) game_free(state->game);

    /* Free card types */
    if (state->scout) card_type_free(state->scout);
    if (state->viper) card_type_free(state->viper);
    if (state->explorer) card_type_free(state->explorer);

    if (state->faction_cards) {
        for (int i = 0; i < state->faction_card_count; i++) {
            card_type_free(state->faction_cards[i]);
        }
        free(state->faction_cards);
    }

    free(state);
}
/* }}} */

/* {{{ demo_add_narrative */
static void demo_add_narrative(DemoState* state, const char* text) {
    narrative_buffer_add(state->narrative, text);
    /* Auto-scroll to bottom */
    state->narrative->scroll_offset = state->narrative->line_count;
}
/* }}} */

/* {{{ demo_render */
static void demo_render(DemoState* state) {
    /* Handle terminal resize */
    if (state->ui->needs_resize) {
        terminal_handle_resize(state->ui);
    }

    /* Render all game windows */
    terminal_render(state->ui, state->game, 0);

    /* Render narrative with scroll position */
    const char* lines[NARRATIVE_MAX_LINES];
    for (int i = 0; i < state->narrative->line_count; i++) {
        lines[i] = state->narrative->lines[i];
    }
    terminal_render_narrative(state->ui, lines, state->narrative->line_count,
                              state->narrative->scroll_offset);

    /* Render input prompt */
    terminal_render_input(state->ui, "> ");
}
/* }}} */

/* {{{ demo_handle_command */
static void demo_handle_command(DemoState* state, Command* cmd) {
    Player* player = state->game->players[0];
    char buf[128];

    switch (cmd->type) {
        case CMD_PLAY:
            if (cmd->target >= 0 && cmd->target < player->deck->hand_count) {
                CardInstance* card = player->deck->hand[cmd->target];
                snprintf(buf, sizeof(buf), "You play %s!", card->type->name);
                demo_add_narrative(state, buf);

                /* Process the play action */
                Action* action = action_create(ACTION_PLAY_CARD);
                action->card_instance_id = strdup(card->instance_id);
                game_process_action(state->game, action);
                action_free(action);
            } else {
                demo_add_narrative(state, "Invalid card index!");
            }
            break;

        case CMD_BUY:
            if (cmd->target >= 0 && cmd->target < TRADE_ROW_SLOTS) {
                CardInstance* card = state->game->trade_row->slots[cmd->target];
                if (card && player->trade >= card->type->cost) {
                    snprintf(buf, sizeof(buf), "You buy %s for %d gold!",
                             card->type->name, card->type->cost);
                    demo_add_narrative(state, buf);

                    Action* action = action_create(ACTION_BUY_CARD);
                    action->slot = cmd->target;
                    game_process_action(state->game, action);
                    action_free(action);
                } else if (card) {
                    snprintf(buf, sizeof(buf), "Not enough trade! Need %d, have %d",
                             card->type->cost, player->trade);
                    demo_add_narrative(state, buf);
                } else {
                    demo_add_narrative(state, "Slot is empty!");
                }
            }
            break;

        case CMD_BUY_WANDERER:
            if (player->trade >= EXPLORER_COST) {
                demo_add_narrative(state, "You buy an Explorer!");
                Action* action = action_create(ACTION_BUY_EXPLORER);
                game_process_action(state->game, action);
                action_free(action);
            } else {
                snprintf(buf, sizeof(buf), "Not enough trade! Need %d, have %d",
                         EXPLORER_COST, player->trade);
                demo_add_narrative(state, buf);
            }
            break;

        case CMD_ATTACK_PLAYER:
            if (player->combat > 0) {
                snprintf(buf, sizeof(buf), "You attack for %d damage!",
                         player->combat);
                demo_add_narrative(state, buf);
                combat_attack_player(state->game, 1, player->combat);  /* Attack player 1 (opponent) */
                player->combat = 0;
            } else {
                demo_add_narrative(state, "No combat available!");
            }
            break;

        case CMD_END_TURN:
            demo_add_narrative(state, "--- End of turn ---");
            game_end_turn(state->game);
            game_start_turn(state->game);
            game_skip_draw_order(state->game);
            /* Give player some resources for next turn */
            player->trade = 4;
            player->combat = 2;
            snprintf(buf, sizeof(buf), "Turn %d begins. Draw your cards!",
                     state->game->turn_number);
            demo_add_narrative(state, buf);
            break;

        case CMD_HELP:
            demo_add_narrative(state, "");
            demo_add_narrative(state, "=== COMMANDS ===");
            demo_add_narrative(state, "p N  - Play card N from hand");
            demo_add_narrative(state, "b N  - Buy card N from trade row");
            demo_add_narrative(state, "b w  - Buy Wanderer/Explorer");
            demo_add_narrative(state, "a    - Attack opponent");
            demo_add_narrative(state, "e    - End turn");
            demo_add_narrative(state, "h    - Show this help");
            demo_add_narrative(state, "q    - Quit demo");
            demo_add_narrative(state, "PageUp/Down - Scroll narrative");
            break;

        case CMD_QUIT:
            demo_add_narrative(state, "Goodbye!");
            state->running = false;
            break;

        case CMD_UNKNOWN:
            demo_add_narrative(state, "Unknown command. Press 'h' for help.");
            break;

        default:
            break;
    }
}
/* }}} */

/* {{{ demo_process_input */
static void demo_process_input(DemoState* state) {
    int ch = getch();

    if (ch == ERR) return;  /* No input available */

    /* Handle special keys */
    switch (ch) {
        case KEY_PPAGE:  /* Page Up - scroll narrative up */
            narrative_buffer_scroll_up(state->narrative, 3);
            return;

        case KEY_NPAGE:  /* Page Down - scroll narrative down */
            narrative_buffer_scroll_down(state->narrative, 3);
            return;

        case KEY_UP:
            narrative_buffer_scroll_up(state->narrative, 1);
            return;

        case KEY_DOWN:
            narrative_buffer_scroll_down(state->narrative, 1);
            return;

        case KEY_RESIZE:
            state->ui->needs_resize = 1;
            return;

        case 'q':
        case 'Q':
            state->running = false;
            return;

        case 'h':
        case 'H':
        case '?': {
            Command cmd = { .type = CMD_HELP };
            demo_handle_command(state, &cmd);
            return;
        }

        case 'p':
        case 'P': {
            /* Simple play command - wait for digit */
            nodelay(stdscr, FALSE);  /* Block for next char */
            int digit = getch();
            nodelay(stdscr, TRUE);   /* Back to non-blocking */
            if (digit >= '0' && digit <= '9') {
                Command cmd = { .type = CMD_PLAY, .target = digit - '0' };
                demo_handle_command(state, &cmd);
            }
            return;
        }

        case 'b':
        case 'B': {
            /* Buy command */
            nodelay(stdscr, FALSE);
            int next = getch();
            nodelay(stdscr, TRUE);
            if (next >= '0' && next <= '9') {
                Command cmd = { .type = CMD_BUY, .target = next - '0' };
                demo_handle_command(state, &cmd);
            } else if (next == 'w' || next == 'W' || next == 'e' || next == 'E') {
                Command cmd = { .type = CMD_BUY_WANDERER };
                demo_handle_command(state, &cmd);
            }
            return;
        }

        case 'a':
        case 'A': {
            Command cmd = { .type = CMD_ATTACK_PLAYER };
            demo_handle_command(state, &cmd);
            return;
        }

        case 'e':
        case 'E': {
            Command cmd = { .type = CMD_END_TURN };
            demo_handle_command(state, &cmd);
            return;
        }

        default:
            break;
    }
}
/* }}} */

/* {{{ demo_show_intro */
static void demo_show_intro(DemoState* state) {
    /* Clear screen and show centered intro */
    clear();
    refresh();

    int h = state->ui->term_height;
    int w = state->ui->term_width;
    int start_y = h / 2 - 8;

    attron(A_BOLD);
    mvprintw(start_y, (w - 42) / 2, "============================================");
    mvprintw(start_y + 1, (w - 42) / 2, "           SYMBELINE REALMS               ");
    mvprintw(start_y + 2, (w - 42) / 2, "     Phase 3 Demo: Client Rendering       ");
    mvprintw(start_y + 3, (w - 42) / 2, "============================================");
    attroff(A_BOLD);

    mvprintw(start_y + 5, (w - 42) / 2, "This demo showcases the terminal client:");
    mvprintw(start_y + 7, (w - 42) / 2, "  * Ncurses split-screen layout");
    mvprintw(start_y + 8, (w - 42) / 2, "  * Faction-colored card rendering");
    mvprintw(start_y + 9, (w - 42) / 2, "  * Status bar with player stats");
    mvprintw(start_y + 10, (w - 42) / 2, "  * Hand, trade row, and base displays");
    mvprintw(start_y + 11, (w - 42) / 2, "  * Scrollable narrative panel");
    mvprintw(start_y + 12, (w - 42) / 2, "  * Command input with history");

    attron(A_BLINK);
    mvprintw(start_y + 14, (w - 30) / 2, "   Press any key to continue...   ");
    attroff(A_BLINK);

    refresh();

    /* Wait for keypress */
    nodelay(stdscr, FALSE);
    getch();
    nodelay(stdscr, TRUE);
}
/* }}} */

/* {{{ demo_show_summary */
static void demo_show_summary(void) {
    printf("\n");
    printf("===========================================================\n");
    printf("  PHASE 3 DEMO COMPLETE\n");
    printf("===========================================================\n\n");

    printf("Components Demonstrated:\n");
    printf("  \033[32m[OK]\033[0m Terminal UI initialization and cleanup\n");
    printf("  \033[32m[OK]\033[0m Split-screen window layout\n");
    printf("  \033[32m[OK]\033[0m Status bar with authority, D10/D4, resources\n");
    printf("  \033[32m[OK]\033[0m Hand rendering with faction colors\n");
    printf("  \033[32m[OK]\033[0m Trade row with affordability highlighting\n");
    printf("  \033[32m[OK]\033[0m Base display for both players\n");
    printf("  \033[32m[OK]\033[0m Scrollable narrative panel\n");
    printf("  \033[32m[OK]\033[0m Command input (play, buy, attack, end turn)\n");
    printf("  \033[32m[OK]\033[0m Terminal resize handling\n\n");

    printf("Browser Client (assets/web/index.html):\n");
    printf("  - HTML5 Canvas renderer\n");
    printf("  - Card animations (play, buy, draw)\n");
    printf("  - Attack/damage effects\n");
    printf("  - Preferences panel (localStorage)\n");
    printf("  - Draw order selection UI\n\n");

    printf("Files Created:\n");
    printf("  src/client/01-terminal.c    - Terminal initialization\n");
    printf("  src/client/02-terminal-render.c - Rendering functions\n");
    printf("  src/client/03-terminal-input.c  - Input handling\n");
    printf("  assets/web/*.js             - Browser client modules\n\n");

    printf("\033[32mPhase 3 client renderers ready for integration!\033[0m\n\n");
}
/* }}} */

/* ========================================================================== */
/*                                Main                                         */
/* ========================================================================== */

/* {{{ main */
int main(void) {
    /* Initialize demo */
    DemoState* state = demo_init();
    if (!state) {
        fprintf(stderr, "Failed to initialize demo\n");
        return 1;
    }

    /* Show intro screen */
    demo_show_intro(state);

    /* Main loop */
    while (state->running) {
        demo_render(state);
        demo_process_input(state);

        /* Small delay to prevent CPU spinning */
        napms(16);  /* ~60 FPS */
    }

    /* Cleanup terminal */
    demo_cleanup(state);

    /* Show summary after ncurses is done */
    demo_show_summary();

    return 0;
}
/* }}} */
