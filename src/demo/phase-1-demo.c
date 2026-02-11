/* phase-1-demo.c - CLI demonstration of Phase 1 game mechanics
 *
 * A text-based game client that demonstrates all Phase 1 mechanics:
 * - Card play, buying, combat
 * - Auto-draw resolution
 * - Base deployment and spawning
 * - Draw order choice
 * - d10/d4 deck flow tracking
 *
 * Run with: ./bin/phase-1-demo
 */

#define _POSIX_C_SOURCE 200809L

#include "../core/01-card.h"
#include "../core/02-deck.h"
#include "../core/03-player.h"
#include "../core/04-trade-row.h"
#include "../core/05-game.h"
#include "../core/06-combat.h"
#include "../core/07-effects.h"
#include "../core/08-auto-draw.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* ========================================================================== */
/*                              Configuration                                 */
/* ========================================================================== */

#define MAX_INPUT_LEN 256
#define STARTING_AUTHORITY 50

/* ========================================================================== */
/*                              Card Database                                 */
/* ========================================================================== */

/* {{{ create_demo_card_types
 * Creates a set of demo cards for the game.
 * Returns array of CardType pointers. Caller takes ownership.
 */
static CardType** create_demo_card_types(int* count) {
    /* Create card types for the demo */
    #define NUM_TRADE_CARDS 12

    CardType** types = calloc(NUM_TRADE_CARDS, sizeof(CardType*));
    if (!types) return NULL;

    int idx = 0;

    /* Merchant faction cards */
    types[idx] = card_type_create("guild_courier", "Guild Courier", 2,
                                   FACTION_MERCHANT, CARD_KIND_SHIP);
    types[idx]->effects = effect_array_create(2);
    types[idx]->effects[0] = (Effect){ EFFECT_TRADE, 2, NULL };
    types[idx]->effects[1] = (Effect){ EFFECT_DRAW, 1, NULL };  /* Auto-draw! */
    types[idx]->effect_count = 2;
    idx++;

    types[idx] = card_type_create("trade_caravan", "Trade Caravan", 3,
                                   FACTION_MERCHANT, CARD_KIND_SHIP);
    types[idx]->effects = effect_array_create(1);
    types[idx]->effects[0] = (Effect){ EFFECT_TRADE, 3, NULL };
    types[idx]->effect_count = 1;
    types[idx]->ally_effects = effect_array_create(1);
    types[idx]->ally_effects[0] = (Effect){ EFFECT_TRADE, 2, NULL };
    types[idx]->ally_effect_count = 1;
    idx++;

    types[idx] = card_type_create("trading_post", "Trading Post", 3,
                                   FACTION_MERCHANT, CARD_KIND_BASE);
    card_type_set_base_stats(types[idx], 4, false);
    types[idx]->effects = effect_array_create(1);
    types[idx]->effects[0] = (Effect){ EFFECT_TRADE, 2, NULL };
    types[idx]->effect_count = 1;
    idx++;

    /* Wilds faction cards */
    types[idx] = card_type_create("dire_bear", "Dire Bear", 4,
                                   FACTION_WILDS, CARD_KIND_SHIP);
    types[idx]->effects = effect_array_create(1);
    types[idx]->effects[0] = (Effect){ EFFECT_COMBAT, 5, NULL };
    types[idx]->effect_count = 1;
    types[idx]->ally_effects = effect_array_create(1);
    types[idx]->ally_effects[0] = (Effect){ EFFECT_DRAW, 1, NULL };
    types[idx]->ally_effect_count = 1;
    idx++;

    types[idx] = card_type_create("wolf_scout", "Wolf Scout", 1,
                                   FACTION_WILDS, CARD_KIND_SHIP);
    types[idx]->effects = effect_array_create(1);
    types[idx]->effects[0] = (Effect){ EFFECT_COMBAT, 2, NULL };
    types[idx]->effect_count = 1;
    idx++;

    types[idx] = card_type_create("beast_den", "Beast Den", 4,
                                   FACTION_WILDS, CARD_KIND_BASE);
    card_type_set_base_stats(types[idx], 5, false);
    card_type_set_spawns(types[idx], "wolf_pup");
    idx++;

    /* Kingdom faction cards */
    types[idx] = card_type_create("knight_commander", "Knight Commander", 5,
                                   FACTION_KINGDOM, CARD_KIND_SHIP);
    types[idx]->effects = effect_array_create(2);
    types[idx]->effects[0] = (Effect){ EFFECT_COMBAT, 4, NULL };
    types[idx]->effects[1] = (Effect){ EFFECT_AUTHORITY, 2, NULL };
    types[idx]->effect_count = 2;
    idx++;

    types[idx] = card_type_create("castle_wall", "Castle Wall", 3,
                                   FACTION_KINGDOM, CARD_KIND_BASE);
    card_type_set_base_stats(types[idx], 6, true);  /* Outpost! */
    types[idx]->effects = effect_array_create(1);
    types[idx]->effects[0] = (Effect){ EFFECT_AUTHORITY, 1, NULL };
    types[idx]->effect_count = 1;
    idx++;

    /* Artificer faction cards */
    types[idx] = card_type_create("battle_golem", "Battle Golem", 4,
                                   FACTION_ARTIFICER, CARD_KIND_SHIP);
    types[idx]->effects = effect_array_create(1);
    types[idx]->effects[0] = (Effect){ EFFECT_COMBAT, 4, NULL };
    types[idx]->effect_count = 1;
    types[idx]->scrap_effects = effect_array_create(1);
    types[idx]->scrap_effects[0] = (Effect){ EFFECT_COMBAT, 2, NULL };
    types[idx]->scrap_effect_count = 1;
    idx++;

    types[idx] = card_type_create("tech_workshop", "Tech Workshop", 4,
                                   FACTION_ARTIFICER, CARD_KIND_BASE);
    card_type_set_base_stats(types[idx], 4, false);
    types[idx]->effects = effect_array_create(1);
    types[idx]->effects[0] = (Effect){ EFFECT_DRAW, 1, NULL };  /* Auto-draw on base! */
    types[idx]->effect_count = 1;
    idx++;

    /* Neutral cards */
    types[idx] = card_type_create("sellsword", "Sellsword", 2,
                                   FACTION_NEUTRAL, CARD_KIND_SHIP);
    types[idx]->effects = effect_array_create(2);
    types[idx]->effects[0] = (Effect){ EFFECT_TRADE, 1, NULL };
    types[idx]->effects[1] = (Effect){ EFFECT_COMBAT, 2, NULL };
    types[idx]->effect_count = 2;
    idx++;

    types[idx] = card_type_create("fortune_teller", "Fortune Teller", 2,
                                   FACTION_NEUTRAL, CARD_KIND_SHIP);
    types[idx]->effects = effect_array_create(2);
    types[idx]->effects[0] = (Effect){ EFFECT_TRADE, 1, NULL };
    types[idx]->effects[1] = (Effect){ EFFECT_DRAW, 1, NULL };  /* Auto-draw! */
    types[idx]->effect_count = 2;
    idx++;

    *count = idx;
    return types;
}
/* }}} */

/* {{{ create_starting_types
 * Creates the basic starting cards (scout, viper, explorer).
 */
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
    (*explorer)->scrap_effects = effect_array_create(1);
    (*explorer)->scrap_effects[0] = (Effect){ EFFECT_COMBAT, 2, NULL };
    (*explorer)->scrap_effect_count = 1;
}
/* }}} */

/* {{{ create_unit_types
 * Creates spawnable unit types.
 */
static CardType* create_wolf_pup(void) {
    CardType* pup = card_type_create("wolf_pup", "Wolf Pup", 0,
                                      FACTION_WILDS, CARD_KIND_UNIT);
    pup->effects = effect_array_create(1);
    pup->effects[0] = (Effect){ EFFECT_COMBAT, 1, NULL };
    pup->effect_count = 1;
    return pup;
}
/* }}} */

/* ========================================================================== */
/*                            Display Functions                               */
/* ========================================================================== */

/* {{{ clear_screen */
static void clear_screen(void) {
    printf("\033[2J\033[H");  /* ANSI escape: clear screen, home cursor */
}
/* }}} */

/* {{{ faction_symbol */
static const char* faction_symbol(Faction f) {
    switch (f) {
        case FACTION_MERCHANT:  return "[M]";
        case FACTION_WILDS:     return "[W]";
        case FACTION_KINGDOM:   return "[K]";
        case FACTION_ARTIFICER: return "[A]";
        default:                return "[N]";
    }
}
/* }}} */

/* {{{ display_card_brief */
static void display_card_brief(CardInstance* card, int index) {
    if (!card || !card->type) {
        printf("  [%d] (empty)\n", index);
        return;
    }

    CardType* t = card->type;
    printf("  [%d] %s (%dg) %s", index, t->name, t->cost, faction_symbol(t->faction));

    /* Show effects summary */
    printf(" -");
    for (int i = 0; i < t->effect_count; i++) {
        Effect* e = &t->effects[i];
        switch (e->type) {
            case EFFECT_TRADE:     printf(" +%dT", e->value); break;
            case EFFECT_COMBAT:    printf(" +%dC", e->value); break;
            case EFFECT_AUTHORITY: printf(" +%dA", e->value); break;
            case EFFECT_DRAW:      printf(" Draw%d", e->value); break;
            default: break;
        }
    }

    /* Show upgrades */
    if (card->trade_bonus > 0) printf(" [+%dT upgrade]", card->trade_bonus);
    if (card->attack_bonus > 0) printf(" [+%dC upgrade]", card->attack_bonus);

    /* Base info */
    if (t->kind == CARD_KIND_BASE) {
        printf(" {%ddef", t->defense);
        if (t->is_outpost) printf(",outpost");
        if (card->deployed) printf(",active");
        else printf(",deploying");
        if (card->damage_taken > 0) printf(",%ddmg", card->damage_taken);
        printf("}");
    }

    printf("\n");
}
/* }}} */

/* {{{ display_trade_row */
static void display_trade_row(TradeRow* row) {
    printf("\n--- Trade Row ---\n");
    for (int i = 0; i < TRADE_ROW_SLOTS; i++) {
        if (row->slots[i]) {
            display_card_brief(row->slots[i], i);
        } else {
            printf("  [%d] (empty)\n", i);
        }
    }
    if (row->explorer_type) {
        printf("  [E] %s (2g) - Always available\n", row->explorer_type->name);
    }
    printf("  Deck: %d cards remaining\n", row->trade_deck_count);
}
/* }}} */

/* {{{ display_player_hand */
static void display_player_hand(Player* player) {
    printf("\n--- Your Hand ---\n");
    if (player->deck->hand_count == 0) {
        printf("  (empty)\n");
        return;
    }
    for (int i = 0; i < player->deck->hand_count; i++) {
        display_card_brief(player->deck->hand[i], i);
    }
}
/* }}} */

/* {{{ display_player_bases */
static void display_player_bases(Player* player, const char* label) {
    int total = deck_total_base_count(player->deck);
    if (total == 0) return;

    printf("\n--- %s Bases ---\n", label);

    /* Frontier */
    for (int i = 0; i < player->deck->frontier_base_count; i++) {
        CardInstance* base = player->deck->frontier_bases[i];
        printf("  [F%d] %s (Frontier)", i, base->type->name);
        printf(" %d/%d def", base->type->defense - base->damage_taken, base->type->defense);
        if (base->deployed) printf(" [Active]");
        else printf(" [Deploying]");
        printf("\n");
    }

    /* Interior */
    for (int i = 0; i < player->deck->interior_base_count; i++) {
        CardInstance* base = player->deck->interior_bases[i];
        printf("  [I%d] %s (Interior)", i, base->type->name);
        printf(" %d/%d def", base->type->defense - base->damage_taken, base->type->defense);
        if (base->deployed) printf(" [Active]");
        else printf(" [Deploying]");
        printf("\n");
    }
}
/* }}} */

/* {{{ display_played_cards */
static void display_played_cards(Player* player) {
    if (player->deck->played_count == 0) return;

    printf("\n--- In Play ---\n");
    for (int i = 0; i < player->deck->played_count; i++) {
        CardInstance* card = player->deck->played[i];
        printf("  %s %s\n", card->type->name, faction_symbol(card->type->faction));
    }
}
/* }}} */

/* {{{ display_player_status */
static void display_player_status(Player* player, bool is_active) {
    printf("%s%s: Authority %d | d10: %d | d4: %d",
           is_active ? ">> " : "   ",
           player->name,
           player->authority,
           player->d10,
           player->d4);

    if (is_active) {
        printf(" | Trade: %d | Combat: %d", player->trade, player->combat);
    }

    printf(" | Deck: %d | Discard: %d\n",
           player->deck->draw_pile_count,
           player->deck->discard_count);
}
/* }}} */

/* {{{ display_game_state */
static void display_game_state(Game* game) {
    clear_screen();

    printf("=== SYMBELINE REALMS - Phase 1 Demo ===\n");
    printf("Turn %d | Phase: %s\n\n",
           game->turn_number,
           game_phase_to_string(game->phase));

    /* Display both players */
    for (int i = 0; i < game->player_count; i++) {
        display_player_status(game->players[i], i == game->active_player);
    }

    Player* active = game_get_active_player(game);

    /* Active player's hand and bases */
    display_player_hand(active);
    display_played_cards(active);
    display_player_bases(active, "Your");

    /* Opponent bases (public info) */
    Player* opponent = game_get_opponent(game, 0);
    display_player_bases(opponent, "Opponent");

    /* Trade row */
    display_trade_row(game->trade_row);
}
/* }}} */

/* ========================================================================== */
/*                             Input Handling                                 */
/* ========================================================================== */

/* {{{ read_line */
static char* read_line(char* buffer, int max_len) {
    printf("\n> ");
    fflush(stdout);

    if (!fgets(buffer, max_len, stdin)) {
        return NULL;
    }

    /* Remove newline */
    int len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }

    return buffer;
}
/* }}} */

/* {{{ parse_int */
static int parse_int(const char* str, int* out) {
    if (!str || !*str) return 0;

    char* end;
    long val = strtol(str, &end, 10);
    if (end == str) return 0;

    *out = (int)val;
    return 1;
}
/* }}} */

/* {{{ handle_draw_order_phase */
static void handle_draw_order_phase(Game* game) {
    Player* player = game_get_active_player(game);
    int hand_size = player_get_hand_size(player);

    printf("\n--- Draw Order Selection ---\n");
    printf("You will draw %d cards. Enter order (e.g., '0,1,2,3,4') or 's' to skip:\n", hand_size);

    char input[MAX_INPUT_LEN];
    if (!read_line(input, MAX_INPUT_LEN)) {
        game_skip_draw_order(game);
        return;
    }

    if (input[0] == 's' || input[0] == 'S' || input[0] == '\0') {
        printf("Drawing in default order...\n");
        game_skip_draw_order(game);
        return;
    }

    /* Parse comma-separated order */
    int order[10];
    int count = 0;
    char* token = strtok(input, ",");
    while (token && count < hand_size) {
        int idx;
        if (parse_int(token, &idx)) {
            order[count++] = idx;
        }
        token = strtok(NULL, ",");
    }

    if (count == hand_size) {
        game_submit_draw_order(game, order, count);
    } else {
        printf("Invalid order (got %d, need %d). Using default.\n", count, hand_size);
        game_skip_draw_order(game);
    }
}
/* }}} */

/* {{{ handle_main_phase */
static bool handle_main_phase(Game* game) {
    Player* player = game_get_active_player(game);

    printf("\nCommands: (p)lay N, (b)uy N, (e)xplorer, (a)ttack [N], (f)rontier N, (i)nterior N, (end)\n");

    char input[MAX_INPUT_LEN];
    if (!read_line(input, MAX_INPUT_LEN)) {
        return false;  /* EOF - quit */
    }

    /* Parse command */
    char cmd = tolower(input[0]);
    int arg = -1;
    if (strlen(input) > 1) {
        parse_int(input + 1, &arg);
    }

    Action* action = NULL;

    switch (cmd) {
        case 'p':  /* Play card from hand */
            if (arg >= 0 && arg < player->deck->hand_count) {
                action = action_create(ACTION_PLAY_CARD);
                action->card_instance_id = strdup(player->deck->hand[arg]->instance_id);

                /* Check if it's a base - need to choose zone */
                CardInstance* card = player->deck->hand[arg];
                if (card->type->kind == CARD_KIND_BASE) {
                    printf("Place base in (f)rontier or (i)nterior? ");
                    char zone_input[MAX_INPUT_LEN];
                    if (read_line(zone_input, MAX_INPUT_LEN)) {
                        if (tolower(zone_input[0]) == 'i') {
                            card->placement = ZONE_INTERIOR;
                        } else {
                            card->placement = ZONE_FRONTIER;
                        }
                    }
                }
            } else {
                printf("Invalid card index.\n");
            }
            break;

        case 'b':  /* Buy from trade row */
            if (arg >= 0 && arg < TRADE_ROW_SLOTS) {
                action = action_create(ACTION_BUY_CARD);
                action->slot = arg;
            } else {
                printf("Invalid slot.\n");
            }
            break;

        case 'e':  /* Buy explorer */
            action = action_create(ACTION_BUY_EXPLORER);
            break;

        case 'a':  /* Attack */
            if (arg >= 0) {
                /* Attack base - need to determine which */
                Player* opponent = game_get_opponent(game, 0);
                int frontier = deck_frontier_count(opponent->deck);

                action = action_create(ACTION_ATTACK_BASE);
                if (arg < frontier) {
                    action->card_instance_id = strdup(opponent->deck->frontier_bases[arg]->instance_id);
                } else {
                    int interior_idx = arg - frontier;
                    if (interior_idx < deck_interior_count(opponent->deck)) {
                        action->card_instance_id = strdup(opponent->deck->interior_bases[interior_idx]->instance_id);
                    } else {
                        printf("Invalid base index.\n");
                        action_free(action);
                        action = NULL;
                    }
                }
                if (action) {
                    printf("How much damage? (max %d) ", player->combat);
                    char dmg_input[MAX_INPUT_LEN];
                    if (read_line(dmg_input, MAX_INPUT_LEN)) {
                        parse_int(dmg_input, &action->amount);
                    }
                }
            } else {
                /* Attack player */
                action = action_create(ACTION_ATTACK_PLAYER);
                printf("How much damage? (max %d) ", player->combat);
                char dmg_input[MAX_INPUT_LEN];
                if (read_line(dmg_input, MAX_INPUT_LEN)) {
                    parse_int(dmg_input, &action->amount);
                }
            }
            break;

        case 'q':  /* Quit */
            return false;

        default:
            if (strncmp(input, "end", 3) == 0) {
                action = action_create(ACTION_END_TURN);
            } else {
                printf("Unknown command: %s\n", input);
            }
            break;
    }

    if (action) {
        bool ok = game_process_action(game, action);
        if (!ok) {
            printf("Action failed!\n");
        }
        action_free(action);
    }

    return true;
}
/* }}} */

/* ========================================================================== */
/*                            Auto-Draw Listener                              */
/* ========================================================================== */

/* {{{ demo_autodraw_listener
 * Displays auto-draw events to the user.
 */
static void demo_autodraw_listener(Game* game, Player* player,
                                    AutoDrawEvent* event, void* context) {
    (void)game; (void)context;

    switch (event->type) {
        case AUTODRAW_EVENT_START:
            printf("\n--- Auto-Draw Resolution ---\n");
            break;

        case AUTODRAW_EVENT_TRIGGER:
            if (event->source && event->source->type) {
                printf("  %s triggers auto-draw!\n", event->source->type->name);
            }
            break;

        case AUTODRAW_EVENT_CARD:
            if (event->drawn && event->drawn->type) {
                printf("  -> Drew: %s\n", event->drawn->type->name);
            }
            break;

        case AUTODRAW_EVENT_COMPLETE:
            if (event->total_drawn > 0) {
                printf("  Auto-draw complete: %d card(s) drawn\n", event->total_drawn);
            }
            printf("--- %s's hand now has %d cards ---\n",
                   player->name, player->deck->hand_count);
            break;
    }
}
/* }}} */

/* ========================================================================== */
/*                                Main                                        */
/* ========================================================================== */

/* {{{ main */
int main(void) {
    /* Initialize RNG */
    srand((unsigned int)time(NULL));

    printf("=== SYMBELINE REALMS - Phase 1 Demo ===\n\n");
    printf("This demo showcases all Phase 1 game mechanics.\n");
    printf("Two players take turns playing cards, buying from the trade row,\n");
    printf("and attacking until one player reaches 0 authority.\n\n");

    /* Initialize effects system */
    effects_init();

    /* Register auto-draw listener to display events */
    autodraw_register_listener(demo_autodraw_listener, NULL);

    /* Create card types */
    CardType* scout;
    CardType* viper;
    CardType* explorer;
    create_starting_types(&scout, &viper, &explorer);

    CardType* wolf_pup = create_wolf_pup();

    int trade_card_count;
    CardType** trade_cards = create_demo_card_types(&trade_card_count);

    /* Create game */
    Game* game = game_create(2);
    game_add_player(game, "Player 1");
    game_add_player(game, "Player 2");

    /* Set starting authority */
    game->players[0]->authority = STARTING_AUTHORITY;
    game->players[1]->authority = STARTING_AUTHORITY;

    /* Set starting types */
    game_set_starting_types(game, scout, viper, explorer);

    /* Register unit types for spawning */
    game_register_card_type(game, wolf_pup);

    /* Create trade row deck (multiple copies of each card) */
    int deck_size = trade_card_count * 4;  /* 4 copies each */
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

    printf("Game started! Player 1 goes first.\n");
    printf("Press Enter to continue...");
    getchar();

    /* Main game loop */
    while (!game_is_over(game)) {
        display_game_state(game);

        switch (game->phase) {
            case PHASE_DRAW_ORDER:
                handle_draw_order_phase(game);
                break;

            case PHASE_MAIN:
                if (!handle_main_phase(game)) {
                    printf("\nGame aborted.\n");
                    goto cleanup;
                }
                break;

            default:
                printf("Unexpected phase: %s\n", game_phase_to_string(game->phase));
                break;
        }
    }

    /* Game over */
    clear_screen();
    printf("=== GAME OVER ===\n\n");
    if (game->winner >= 0) {
        printf("%s WINS!\n", game->players[game->winner]->name);
    } else {
        printf("It's a draw!\n");
    }

    printf("\nFinal scores:\n");
    for (int i = 0; i < game->player_count; i++) {
        Player* p = game->players[i];
        printf("  %s: Authority %d, d10: %d, d4: %d\n",
               p->name, p->authority, p->d10, p->d4);
    }

cleanup:
    /* Cleanup */
    autodraw_clear_listeners();
    effects_clear_callbacks();
    game_free(game);

    /* Free card types not owned by game */
    card_type_free(scout);
    card_type_free(viper);
    card_type_free(explorer);
    for (int i = 0; i < trade_card_count; i++) {
        card_type_free(trade_cards[i]);
    }
    free(trade_cards);

    printf("\nThanks for playing!\n");
    return 0;
}
/* }}} */
