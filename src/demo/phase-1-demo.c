/* phase-1-demo.c - Enhanced CLI demonstration of Phase 1 game mechanics
 *
 * A text-based game client that demonstrates all Phase 1 mechanics:
 * - Card play, buying, combat
 * - Auto-draw resolution
 * - Base deployment and spawning
 * - Draw order choice
 * - d10/d4 deck flow tracking
 * - Upgrade effects
 * - Pending action resolution
 * - Ally abilities and scrap effects
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
/*                              ANSI Colors                                   */
/* ========================================================================== */

/* {{{ color definitions */
#define RESET       "\033[0m"
#define BOLD        "\033[1m"
#define DIM         "\033[2m"
#define UNDERLINE   "\033[4m"

/* Faction colors */
#define COL_MERCHANT  "\033[33m"      /* Yellow */
#define COL_WILDS     "\033[32m"      /* Green */
#define COL_KINGDOM   "\033[34m"      /* Blue */
#define COL_ARTIFICER "\033[35m"      /* Magenta */
#define COL_NEUTRAL   "\033[37m"      /* White */

/* Resource colors */
#define COL_TRADE     "\033[93m"      /* Bright Yellow */
#define COL_COMBAT    "\033[91m"      /* Bright Red */
#define COL_AUTHORITY "\033[92m"      /* Bright Green */
#define COL_DRAW      "\033[96m"      /* Bright Cyan */

/* UI colors */
#define COL_ACTIVE    "\033[97;44m"   /* White on Blue */
#define COL_DAMAGE    "\033[31m"      /* Red */
#define COL_UPGRADE   "\033[95m"      /* Bright Magenta */
#define COL_PENDING   "\033[93;40m"   /* Yellow on Black */
#define COL_SUCCESS   "\033[92m"      /* Bright Green */
#define COL_ERROR     "\033[91m"      /* Bright Red */
#define COL_INFO      "\033[94m"      /* Bright Blue */
#define COL_HEADER    "\033[97;100m"  /* White on Gray */
/* }}} */

/* ========================================================================== */
/*                              Box Drawing                                   */
/* ========================================================================== */

/* {{{ box drawing characters */
#define BOX_TL "┌"
#define BOX_TR "┐"
#define BOX_BL "└"
#define BOX_BR "┘"
#define BOX_H  "─"
#define BOX_V  "│"
#define BOX_LT "├"
#define BOX_RT "┤"
#define BOX_TT "┬"
#define BOX_BT "┴"
#define BOX_X  "┼"

#define BOX_THICK_H "━"
#define BOX_THICK_V "┃"
/* }}} */

/* ========================================================================== */
/*                              Card Database                                 */
/* ========================================================================== */

/* {{{ create_demo_card_types
 * Creates an expanded set of demo cards showcasing all mechanics.
 * Returns array of CardType pointers. Caller takes ownership.
 */
static CardType** create_demo_card_types(int* count) {
    #define NUM_TRADE_CARDS 16

    CardType** types = calloc(NUM_TRADE_CARDS, sizeof(CardType*));
    if (!types) return NULL;

    int idx = 0;

    /* ===== Merchant faction cards ===== */
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

    /* Master Merchant - has upgrade effect! */
    types[idx] = card_type_create("master_merchant", "Master Merchant", 5,
                                   FACTION_MERCHANT, CARD_KIND_SHIP);
    types[idx]->effects = effect_array_create(2);
    types[idx]->effects[0] = (Effect){ EFFECT_TRADE, 3, NULL };
    types[idx]->effects[1] = (Effect){ EFFECT_UPGRADE_TRADE, 1, NULL };
    types[idx]->effect_count = 2;
    idx++;

    /* ===== Wilds faction cards ===== */
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
    types[idx]->scrap_effects = effect_array_create(1);
    types[idx]->scrap_effects[0] = (Effect){ EFFECT_D10_UP, 1, NULL };
    types[idx]->scrap_effect_count = 1;
    idx++;

    types[idx] = card_type_create("beast_den", "Beast Den", 4,
                                   FACTION_WILDS, CARD_KIND_BASE);
    card_type_set_base_stats(types[idx], 5, false);
    card_type_set_spawns(types[idx], "wolf_pup");
    idx++;

    /* Alpha Wolf - upgrade attack! */
    types[idx] = card_type_create("alpha_wolf", "Alpha Wolf", 5,
                                   FACTION_WILDS, CARD_KIND_SHIP);
    types[idx]->effects = effect_array_create(2);
    types[idx]->effects[0] = (Effect){ EFFECT_COMBAT, 4, NULL };
    types[idx]->effects[1] = (Effect){ EFFECT_UPGRADE_ATTACK, 2, NULL };
    types[idx]->effect_count = 2;
    idx++;

    /* ===== Kingdom faction cards ===== */
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

    /* Royal Healer - upgrade authority! */
    types[idx] = card_type_create("royal_healer", "Royal Healer", 4,
                                   FACTION_KINGDOM, CARD_KIND_SHIP);
    types[idx]->effects = effect_array_create(2);
    types[idx]->effects[0] = (Effect){ EFFECT_AUTHORITY, 3, NULL };
    types[idx]->effects[1] = (Effect){ EFFECT_UPGRADE_AUTH, 1, NULL };
    types[idx]->effect_count = 2;
    idx++;

    /* ===== Artificer faction cards ===== */
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

    /* Construct Factory - spawns constructs! */
    types[idx] = card_type_create("construct_factory", "Construct Factory", 6,
                                   FACTION_ARTIFICER, CARD_KIND_BASE);
    card_type_set_base_stats(types[idx], 5, false);
    card_type_set_spawns(types[idx], "mini_construct");
    types[idx]->effects = effect_array_create(1);
    types[idx]->effects[0] = (Effect){ EFFECT_TRADE, 1, NULL };
    types[idx]->effect_count = 1;
    idx++;

    /* ===== Neutral cards ===== */
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

static CardType* create_mini_construct(void) {
    CardType* construct = card_type_create("mini_construct", "Mini Construct", 0,
                                            FACTION_ARTIFICER, CARD_KIND_UNIT);
    construct->effects = effect_array_create(1);
    construct->effects[0] = (Effect){ EFFECT_TRADE, 1, NULL };
    construct->effect_count = 1;
    return construct;
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

/* {{{ faction_color */
static const char* faction_color(Faction f) {
    switch (f) {
        case FACTION_MERCHANT:  return COL_MERCHANT;
        case FACTION_WILDS:     return COL_WILDS;
        case FACTION_KINGDOM:   return COL_KINGDOM;
        case FACTION_ARTIFICER: return COL_ARTIFICER;
        default:                return COL_NEUTRAL;
    }
}
/* }}} */

/* {{{ faction_name */
static const char* faction_name(Faction f) {
    switch (f) {
        case FACTION_MERCHANT:  return "Merchant";
        case FACTION_WILDS:     return "Wilds";
        case FACTION_KINGDOM:   return "Kingdom";
        case FACTION_ARTIFICER: return "Artificer";
        default:                return "Neutral";
    }
}
/* }}} */

/* {{{ kind_symbol */
static const char* kind_symbol(CardKind k) {
    switch (k) {
        case CARD_KIND_SHIP: return "⚓";
        case CARD_KIND_BASE: return "🏰";
        case CARD_KIND_UNIT: return "⚔";
        default:             return "?";
    }
}
/* }}} */

/* {{{ effect_to_string */
static void effect_to_string(Effect* e, char* buf, int buf_size) {
    switch (e->type) {
        case EFFECT_TRADE:
            snprintf(buf, buf_size, "%s+%dT%s", COL_TRADE, e->value, RESET);
            break;
        case EFFECT_COMBAT:
            snprintf(buf, buf_size, "%s+%dC%s", COL_COMBAT, e->value, RESET);
            break;
        case EFFECT_AUTHORITY:
            snprintf(buf, buf_size, "%s+%dA%s", COL_AUTHORITY, e->value, RESET);
            break;
        case EFFECT_DRAW:
            snprintf(buf, buf_size, "%sDraw %d%s", COL_DRAW, e->value, RESET);
            break;
        case EFFECT_UPGRADE_TRADE:
            snprintf(buf, buf_size, "%s↑T+%d%s", COL_UPGRADE, e->value, RESET);
            break;
        case EFFECT_UPGRADE_ATTACK:
            snprintf(buf, buf_size, "%s↑C+%d%s", COL_UPGRADE, e->value, RESET);
            break;
        case EFFECT_UPGRADE_AUTH:
            snprintf(buf, buf_size, "%s↑A+%d%s", COL_UPGRADE, e->value, RESET);
            break;
        case EFFECT_D10_UP:
            snprintf(buf, buf_size, "%sd10↑%d%s", COL_INFO, e->value, RESET);
            break;
        case EFFECT_D10_DOWN:
            snprintf(buf, buf_size, "%sd10↓%d%s", COL_DAMAGE, e->value, RESET);
            break;
        case EFFECT_SPAWN:
            snprintf(buf, buf_size, "%sSpawn%s", COL_SUCCESS, RESET);
            break;
        default:
            snprintf(buf, buf_size, "?");
            break;
    }
}
/* }}} */

/* {{{ print_header */
static void print_header(const char* text) {
    int len = strlen(text);
    int pad = (60 - len - 2) / 2;

    printf("\n%s", COL_HEADER);
    printf("%s", BOX_TL);
    for (int i = 0; i < 58; i++) printf("%s", BOX_H);
    printf("%s%s\n", BOX_TR, RESET);

    printf("%s%s", COL_HEADER, BOX_V);
    for (int i = 0; i < pad; i++) printf(" ");
    printf("%s", text);
    for (int i = 0; i < 58 - pad - len; i++) printf(" ");
    printf("%s%s\n", BOX_V, RESET);

    printf("%s%s", COL_HEADER, BOX_BL);
    for (int i = 0; i < 58; i++) printf("%s", BOX_H);
    printf("%s%s\n", BOX_BR, RESET);
}
/* }}} */

/* {{{ print_section */
static void print_section(const char* text) {
    printf("\n%s%s─── %s ───%s\n", BOLD, DIM, text, RESET);
}
/* }}} */

/* {{{ display_card_detailed */
static void display_card_detailed(CardInstance* card, int index, bool show_index) {
    if (!card || !card->type) {
        if (show_index) printf("  [%d] %s(empty)%s\n", index, DIM, RESET);
        return;
    }

    CardType* t = card->type;
    const char* col = faction_color(t->faction);

    /* Index and name */
    if (show_index) {
        printf("  %s[%d]%s ", DIM, index, RESET);
    } else {
        printf("      ");
    }

    printf("%s%s%s %s%s%s",
           col, BOLD, t->name, RESET,
           kind_symbol(t->kind),
           t->cost > 0 ? "" : "");

    if (t->cost > 0) {
        printf(" %s(%dg)%s", DIM, t->cost, RESET);
    }

    /* Faction tag */
    printf(" %s[%s]%s", col, faction_name(t->faction), RESET);

    printf("\n");

    /* Effects line */
    char effect_buf[64];
    printf("       ");

    /* Main effects */
    for (int i = 0; i < t->effect_count; i++) {
        effect_to_string(&t->effects[i], effect_buf, sizeof(effect_buf));
        printf("%s ", effect_buf);
    }

    /* Ally effects */
    if (t->ally_effect_count > 0) {
        printf("%s│Ally:%s ", DIM, RESET);
        for (int i = 0; i < t->ally_effect_count; i++) {
            effect_to_string(&t->ally_effects[i], effect_buf, sizeof(effect_buf));
            printf("%s ", effect_buf);
        }
    }

    /* Scrap effects */
    if (t->scrap_effect_count > 0) {
        printf("%s│Scrap:%s ", DIM, RESET);
        for (int i = 0; i < t->scrap_effect_count; i++) {
            effect_to_string(&t->scrap_effects[i], effect_buf, sizeof(effect_buf));
            printf("%s ", effect_buf);
        }
    }

    printf("\n");

    /* Upgrades and base info */
    bool has_extras = false;
    if (card->trade_bonus > 0 || card->attack_bonus > 0 || card->authority_bonus > 0) {
        printf("       %s★ Upgrades:%s", COL_UPGRADE, RESET);
        if (card->trade_bonus > 0) printf(" %s+%dT%s", COL_TRADE, card->trade_bonus, RESET);
        if (card->attack_bonus > 0) printf(" %s+%dC%s", COL_COMBAT, card->attack_bonus, RESET);
        if (card->authority_bonus > 0) printf(" %s+%dA%s", COL_AUTHORITY, card->authority_bonus, RESET);
        has_extras = true;
    }

    /* Base info */
    if (t->kind == CARD_KIND_BASE) {
        if (!has_extras) printf("       ");
        else printf(" │ ");

        int remaining = t->defense - card->damage_taken;
        printf("%s🛡 %d/%d%s", remaining <= 2 ? COL_DAMAGE : COL_SUCCESS,
               remaining, t->defense, RESET);
        if (t->is_outpost) printf(" %sOUTPOST%s", BOLD, RESET);
        if (card->deployed) printf(" %s[Active]%s", COL_SUCCESS, RESET);
        else printf(" %s[Deploying]%s", COL_PENDING, RESET);
        if (t->spawns_id) printf(" │ Spawns: %s", t->spawns_id);
        has_extras = true;
    }

    if (has_extras) printf("\n");
}
/* }}} */

/* {{{ display_card_brief - compact version for trade row */
static void display_card_brief(CardInstance* card, int index) {
    if (!card || !card->type) {
        printf("  [%d] %s(empty)%s\n", index, DIM, RESET);
        return;
    }

    CardType* t = card->type;
    const char* col = faction_color(t->faction);
    char effect_buf[64];

    printf("  [%d] %s%s%s %s(%dg)%s ", index, col, t->name, RESET,
           DIM, t->cost, RESET);

    /* Compact effects */
    for (int i = 0; i < t->effect_count; i++) {
        effect_to_string(&t->effects[i], effect_buf, sizeof(effect_buf));
        printf("%s ", effect_buf);
    }

    /* Show ally/scrap indicators */
    if (t->ally_effect_count > 0) printf("%s[A]%s ", DIM, RESET);
    if (t->scrap_effect_count > 0) printf("%s[S]%s ", DIM, RESET);
    if (t->spawns_id) printf("%s[Spawn]%s ", DIM, RESET);

    printf("\n");
}
/* }}} */

/* {{{ display_trade_row */
static void display_trade_row(TradeRow* row) {
    print_section("Trade Row");

    for (int i = 0; i < TRADE_ROW_SLOTS; i++) {
        if (row->slots[i]) {
            display_card_brief(row->slots[i], i);
        } else {
            printf("  [%d] %s(empty)%s\n", i, DIM, RESET);
        }
    }

    if (row->explorer_type) {
        printf("\n  %s[E]%s %sExplorer%s (2g) %s+2T%s │ %sScrap: +2C%s\n",
               BOLD, RESET, COL_NEUTRAL, RESET,
               COL_TRADE, RESET, DIM, RESET);
    }

    printf("\n  %sDeck: %d cards remaining%s\n", DIM, row->trade_deck_count, RESET);
}
/* }}} */

/* {{{ display_player_hand */
static void display_player_hand(Player* player) {
    print_section("Your Hand");

    if (player->deck->hand_count == 0) {
        printf("  %s(empty)%s\n", DIM, RESET);
        return;
    }

    for (int i = 0; i < player->deck->hand_count; i++) {
        display_card_detailed(player->deck->hand[i], i, true);
    }
}
/* }}} */

/* {{{ display_player_bases */
static void display_player_bases(Player* player, const char* label) {
    int total = deck_total_base_count(player->deck);
    if (total == 0) return;

    char section_title[64];
    snprintf(section_title, sizeof(section_title), "%s Bases", label);
    print_section(section_title);

    /* Frontier */
    if (player->deck->frontier_base_count > 0) {
        printf("  %sFRONTIER (attacked first)%s\n", DIM, RESET);
        for (int i = 0; i < player->deck->frontier_base_count; i++) {
            display_card_detailed(player->deck->frontier_bases[i], i, true);
        }
    }

    /* Interior */
    if (player->deck->interior_base_count > 0) {
        printf("  %sINTERIOR (protected)%s\n", DIM, RESET);
        for (int i = 0; i < player->deck->interior_base_count; i++) {
            display_card_detailed(player->deck->interior_bases[i],
                                  player->deck->frontier_base_count + i, true);
        }
    }
}
/* }}} */

/* {{{ display_played_cards */
static void display_played_cards(Player* player) {
    if (player->deck->played_count == 0) return;

    print_section("In Play");

    /* Count factions for ally display */
    int faction_counts[5] = {0};
    for (int i = 0; i < player->deck->played_count; i++) {
        CardInstance* card = player->deck->played[i];
        if (card->type->faction < 5) {
            faction_counts[card->type->faction]++;
        }
    }

    /* Show ally status */
    printf("  %sAllies:%s ", DIM, RESET);
    const char* faction_names[] = {"Neutral", "Merchant", "Wilds", "Kingdom", "Artificer"};
    const char* faction_cols[] = {COL_NEUTRAL, COL_MERCHANT, COL_WILDS, COL_KINGDOM, COL_ARTIFICER};
    bool first = true;
    for (int i = 1; i < 5; i++) {  /* Skip neutral */
        if (faction_counts[i] >= 2) {
            if (!first) printf(", ");
            printf("%s%s×%d%s", faction_cols[i], faction_names[i], faction_counts[i], RESET);
            first = false;
        }
    }
    if (first) printf("%snone active%s", DIM, RESET);
    printf("\n\n");

    /* List played cards */
    for (int i = 0; i < player->deck->played_count; i++) {
        CardInstance* card = player->deck->played[i];
        const char* col = faction_color(card->type->faction);
        printf("  %s%s%s %s\n", col, card->type->name, RESET, kind_symbol(card->type->kind));
    }
}
/* }}} */

/* {{{ display_player_status */
static void display_player_status(Player* player, bool is_active, int player_num) {
    const char* prefix = is_active ? COL_ACTIVE : "";
    const char* suffix = is_active ? RESET : "";

    printf("%s %s %s%s\n", prefix, player->name, suffix, is_active ? " ◀" : "");

    printf("  %s♥%s Authority: %s%d%s",
           COL_AUTHORITY, RESET,
           player->authority <= 10 ? COL_DAMAGE : "",
           player->authority, RESET);

    printf("  │  %sd10: %d%s  %sd4: %d%s",
           COL_INFO, player->d10, RESET,
           COL_INFO, player->d4, RESET);

    if (is_active) {
        printf("  │  %s💰 %d%s  %s⚔ %d%s",
               COL_TRADE, player->trade, RESET,
               COL_COMBAT, player->combat, RESET);
    }

    printf("\n  %sDeck: %d │ Discard: %d │ Hand: %d%s\n",
           DIM,
           player->deck->draw_pile_count,
           player->deck->discard_count,
           player->deck->hand_count,
           RESET);

    (void)player_num;
}
/* }}} */

/* {{{ display_pending_action */
static void display_pending_action(Game* game) {
    if (!game_has_pending_action(game)) return;

    PendingAction* pending = game_get_pending_action(game);
    if (!pending) return;

    printf("\n%s", COL_PENDING);
    printf("┌──────────────────────────────────────────────────────────┐\n");
    printf("│ ⚠  PENDING ACTION REQUIRED                              │\n");
    printf("├──────────────────────────────────────────────────────────┤\n");
    printf("│ ");

    switch (pending->type) {
        case PENDING_DISCARD:
            printf("Discard %d card(s) from hand", pending->count);
            break;
        case PENDING_SCRAP_TRADE_ROW:
            printf("Scrap a card from the trade row (optional)");
            break;
        case PENDING_SCRAP_HAND_DISCARD:
            printf("Scrap a card from hand or discard (optional)");
            break;
        case PENDING_TOP_DECK:
            printf("Put a card on top of your deck (optional)");
            break;
        case PENDING_UPGRADE:
            printf("Upgrade a card: +%d %s (optional)",
                   pending->upgrade_value,
                   pending->upgrade_type == EFFECT_UPGRADE_ATTACK ? "Combat" :
                   pending->upgrade_type == EFFECT_UPGRADE_TRADE ? "Trade" : "Authority");
            break;
        case PENDING_COPY_SHIP:
            printf("Choose a ship in play to copy its effects");
            break;
        case PENDING_DESTROY_BASE:
            printf("Choose an opponent's base to destroy");
            break;
        default:
            printf("Unknown pending action");
            break;
    }

    /* Pad to fill box */
    printf("%*s│\n", 40, "");
    printf("│ Use: (r)esolve N  or  (s)kip if optional                 │\n");
    printf("└──────────────────────────────────────────────────────────┘\n");
    printf("%s", RESET);
}
/* }}} */

/* {{{ display_discard_pile */
static void display_discard_pile(Player* player) {
    print_section("Discard Pile");

    if (player->deck->discard_count == 0) {
        printf("  %s(empty)%s\n", DIM, RESET);
        return;
    }

    for (int i = 0; i < player->deck->discard_count; i++) {
        display_card_brief(player->deck->discard[i], i);
    }
}
/* }}} */

/* {{{ display_help */
static void display_help(void) {
    clear_screen();
    print_header("COMMAND REFERENCE");

    printf("\n%s BASIC COMMANDS %s\n", BOLD, RESET);
    printf("  p N       Play card at index N from hand\n");
    printf("  b N       Buy card at slot N from trade row\n");
    printf("  e         Buy Explorer (always available, costs 2)\n");
    printf("  end       End your turn\n");
    printf("  q         Quit game\n");

    printf("\n%s COMBAT COMMANDS %s\n", BOLD, RESET);
    printf("  a         Attack opponent player (prompts for damage)\n");
    printf("  a N       Attack base at index N (prompts for damage)\n");

    printf("\n%s SPECIAL COMMANDS %s\n", BOLD, RESET);
    printf("  scrap N   Scrap card at index N from hand\n");
    printf("  r N       Resolve pending action with card/slot N\n");
    printf("  s         Skip optional pending action\n");
    printf("  d         View your discard pile\n");
    printf("  h / ?     Show this help\n");

    printf("\n%s EFFECT SYMBOLS %s\n", BOLD, RESET);
    printf("  %s+NT%s  Gain N trade     ", COL_TRADE, RESET);
    printf("  %s+NC%s  Gain N combat\n", COL_COMBAT, RESET);
    printf("  %s+NA%s  Gain N authority ", COL_AUTHORITY, RESET);
    printf("  %sDraw N%s  Draw N cards\n", COL_DRAW, RESET);
    printf("  %s↑T/C/A%s  Upgrade effect  ", COL_UPGRADE, RESET);
    printf("  %s[A]%s  Has ally ability\n", DIM, RESET);
    printf("  %s[S]%s  Has scrap ability ", DIM, RESET);
    printf("  %s[Spawn]%s  Spawns units\n", DIM, RESET);

    printf("\n%s ZONES %s\n", BOLD, RESET);
    printf("  Frontier bases are attacked first\n");
    printf("  Interior bases are protected while frontier exists\n");
    printf("  Outposts MUST be destroyed before attacking player\n");

    printf("\n%sPress Enter to continue...%s", DIM, RESET);
    getchar();
}
/* }}} */

/* {{{ display_game_state */
static void display_game_state(Game* game) {
    clear_screen();

    /* Title bar */
    printf("%s", COL_HEADER);
    printf("  SYMBELINE REALMS │ Turn %d │ %s  ",
           game->turn_number, game_phase_to_string(game->phase));
    printf("%s\n", RESET);

    printf("\n");

    /* Display both players */
    for (int i = 0; i < game->player_count; i++) {
        display_player_status(game->players[i], i == game->active_player, i);
        printf("\n");
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

    /* Pending action alert */
    display_pending_action(game);
}
/* }}} */

/* ========================================================================== */
/*                             Input Handling                                 */
/* ========================================================================== */

/* {{{ read_line */
static char* read_line(char* buffer, int max_len) {
    printf("\n%s>%s ", BOLD, RESET);
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

    print_section("Draw Order Selection");
    printf("  You will draw %s%d cards%s.\n", BOLD, hand_size, RESET);
    printf("  Enter order (e.g., '0,1,2,3,4') or press Enter to skip:\n");

    char input[MAX_INPUT_LEN];
    if (!read_line(input, MAX_INPUT_LEN)) {
        game_skip_draw_order(game);
        return;
    }

    if (input[0] == '\0') {
        printf("  %sDrawing in default order...%s\n", DIM, RESET);
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
        printf("  %s✓ Draw order set%s\n", COL_SUCCESS, RESET);
    } else {
        printf("  %s✗ Invalid order (got %d, need %d). Using default.%s\n",
               COL_ERROR, count, hand_size, RESET);
        game_skip_draw_order(game);
    }
}
/* }}} */

/* {{{ handle_pending_action */
static bool handle_pending_action(Game* game, const char* input) {
    if (!game_has_pending_action(game)) return false;

    PendingAction* pending = game_get_pending_action(game);
    Player* player = game_get_active_player(game);

    char cmd = tolower(input[0]);
    int arg = -1;
    if (strlen(input) > 1) {
        parse_int(input + 1, &arg);
    }

    if (cmd == 's') {
        /* Skip optional pending */
        if (pending->optional) {
            game_skip_pending_action(game);
            printf("  %s✓ Skipped optional action%s\n", COL_SUCCESS, RESET);
            return true;
        } else {
            printf("  %s✗ This action is not optional%s\n", COL_ERROR, RESET);
            return true;
        }
    }

    if (cmd == 'r' && arg >= 0) {
        bool success = false;

        switch (pending->type) {
            case PENDING_DISCARD:
                if (arg < player->deck->hand_count) {
                    CardInstance* card = player->deck->hand[arg];
                    success = game_resolve_discard(game, card->instance_id);
                }
                break;

            case PENDING_UPGRADE:
                if (arg < player->deck->hand_count) {
                    CardInstance* card = player->deck->hand[arg];
                    success = game_resolve_upgrade(game, card->instance_id);
                }
                break;

            case PENDING_SCRAP_TRADE_ROW:
                if (arg < TRADE_ROW_SLOTS && game->trade_row->slots[arg]) {
                    success = game_resolve_scrap_trade_row(game, arg);
                }
                break;

            case PENDING_COPY_SHIP:
                if (arg < player->deck->played_count) {
                    CardInstance* card = player->deck->played[arg];
                    success = game_resolve_copy_ship(game, card->instance_id);
                }
                break;

            case PENDING_DESTROY_BASE: {
                Player* opp = game_get_opponent(game, 0);
                int total_bases = deck_total_base_count(opp->deck);
                if (arg < total_bases) {
                    CardInstance* base;
                    if (arg < opp->deck->frontier_base_count) {
                        base = opp->deck->frontier_bases[arg];
                    } else {
                        base = opp->deck->interior_bases[arg - opp->deck->frontier_base_count];
                    }
                    success = game_resolve_destroy_base(game, base->instance_id);
                }
                break;
            }

            default:
                printf("  %s✗ Cannot resolve this pending action type yet%s\n",
                       COL_ERROR, RESET);
                return true;
        }

        if (success) {
            printf("  %s✓ Action resolved%s\n", COL_SUCCESS, RESET);
        } else {
            printf("  %s✗ Failed to resolve action%s\n", COL_ERROR, RESET);
        }
        return true;
    }

    return false;
}
/* }}} */

/* {{{ handle_scrap_command */
static void handle_scrap_command(Game* game, int index) {
    Player* player = game_get_active_player(game);

    if (index < 0 || index >= player->deck->hand_count) {
        printf("  %s✗ Invalid card index%s\n", COL_ERROR, RESET);
        return;
    }

    CardInstance* card = player->deck->hand[index];
    CardType* type = card->type;

    if (type->scrap_effect_count == 0) {
        printf("  %s✗ This card has no scrap ability%s\n", COL_ERROR, RESET);
        return;
    }

    /* Execute scrap effects */
    for (int i = 0; i < type->scrap_effect_count; i++) {
        effects_execute(game, player, &type->scrap_effects[i], card);
    }

    /* Remove card from hand and add to scrap pile */
    printf("  %s✓ Scrapped %s%s\n", COL_SUCCESS, type->name, RESET);

    /* Move to discard for now (proper scrap pile not implemented) */
    deck_add_to_discard(player->deck, card);
    for (int i = index; i < player->deck->hand_count - 1; i++) {
        player->deck->hand[i] = player->deck->hand[i + 1];
    }
    player->deck->hand_count--;
}
/* }}} */

/* {{{ handle_main_phase */
static bool handle_main_phase(Game* game) {
    Player* player = game_get_active_player(game);

    /* Show available commands */
    if (game_has_pending_action(game)) {
        printf("\n%sResolve pending action: (r)esolve N, (s)kip%s\n", COL_PENDING, RESET);
    }
    printf("%sCommands: (p)lay (b)uy (e)xplorer (a)ttack (scrap) (end) (d)iscard (h)elp (q)uit%s\n",
           DIM, RESET);

    char input[MAX_INPUT_LEN];
    if (!read_line(input, MAX_INPUT_LEN)) {
        return false;  /* EOF - quit */
    }

    /* Check for pending action handling first */
    if (game_has_pending_action(game)) {
        if (handle_pending_action(game, input)) {
            return true;
        }
    }

    /* Parse command */
    char cmd = tolower(input[0]);
    int arg = -1;

    /* Handle multi-character commands */
    if (strncmp(input, "scrap", 5) == 0) {
        parse_int(input + 5, &arg);
        handle_scrap_command(game, arg);
        return true;
    }

    if (strncmp(input, "end", 3) == 0) {
        Action* action = action_create(ACTION_END_TURN);
        bool ok = game_process_action(game, action);
        action_free(action);
        if (ok) printf("  %s✓ Turn ended%s\n", COL_SUCCESS, RESET);
        return true;
    }

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
                    printf("  Place base in %s(f)rontier%s or %s(i)nterior%s? ",
                           BOLD, RESET, BOLD, RESET);
                    char zone_input[MAX_INPUT_LEN];
                    if (read_line(zone_input, MAX_INPUT_LEN)) {
                        if (tolower(zone_input[0]) == 'i') {
                            card->placement = ZONE_INTERIOR;
                            printf("  %s→ Placing in interior%s\n", DIM, RESET);
                        } else {
                            card->placement = ZONE_FRONTIER;
                            printf("  %s→ Placing in frontier%s\n", DIM, RESET);
                        }
                    }
                }
            } else {
                printf("  %s✗ Invalid card index%s\n", COL_ERROR, RESET);
            }
            break;

        case 'b':  /* Buy from trade row */
            if (arg >= 0 && arg < TRADE_ROW_SLOTS) {
                if (game->trade_row->slots[arg]) {
                    action = action_create(ACTION_BUY_CARD);
                    action->slot = arg;
                } else {
                    printf("  %s✗ Slot is empty%s\n", COL_ERROR, RESET);
                }
            } else {
                printf("  %s✗ Invalid slot (0-%d)%s\n", COL_ERROR, TRADE_ROW_SLOTS - 1, RESET);
            }
            break;

        case 'e':  /* Buy explorer */
            action = action_create(ACTION_BUY_EXPLORER);
            break;

        case 'a':  /* Attack */
            if (arg >= 0) {
                /* Attack base */
                Player* opponent = game_get_opponent(game, 0);
                int frontier = deck_frontier_count(opponent->deck);
                int total = deck_total_base_count(opponent->deck);

                if (arg >= total) {
                    printf("  %s✗ Invalid base index%s\n", COL_ERROR, RESET);
                    break;
                }

                action = action_create(ACTION_ATTACK_BASE);
                if (arg < frontier) {
                    action->card_instance_id = strdup(opponent->deck->frontier_bases[arg]->instance_id);
                } else {
                    action->card_instance_id = strdup(opponent->deck->interior_bases[arg - frontier]->instance_id);
                }

                printf("  Damage amount? (you have %s%d combat%s): ",
                       COL_COMBAT, player->combat, RESET);
                char dmg_input[MAX_INPUT_LEN];
                if (read_line(dmg_input, MAX_INPUT_LEN)) {
                    parse_int(dmg_input, &action->amount);
                }
            } else {
                /* Attack player */
                action = action_create(ACTION_ATTACK_PLAYER);
                printf("  Damage amount? (you have %s%d combat%s): ",
                       COL_COMBAT, player->combat, RESET);
                char dmg_input[MAX_INPUT_LEN];
                if (read_line(dmg_input, MAX_INPUT_LEN)) {
                    parse_int(dmg_input, &action->amount);
                }
            }
            break;

        case 'd':  /* View discard */
            display_discard_pile(player);
            printf("\n%sPress Enter to continue...%s", DIM, RESET);
            getchar();
            break;

        case 'h':
        case '?':
            display_help();
            break;

        case 'q':  /* Quit */
            return false;

        default:
            printf("  %s✗ Unknown command. Type 'h' for help.%s\n", COL_ERROR, RESET);
            break;
    }

    if (action) {
        bool ok = game_process_action(game, action);
        if (!ok) {
            printf("  %s✗ Action failed%s\n", COL_ERROR, RESET);
        } else {
            printf("  %s✓ Done%s\n", COL_SUCCESS, RESET);
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
 * Displays auto-draw events to the user with color.
 */
static void demo_autodraw_listener(Game* game, Player* player,
                                    AutoDrawEvent* event, void* context) {
    (void)game; (void)context;

    switch (event->type) {
        case AUTODRAW_EVENT_START:
            printf("\n  %s┌─── Auto-Draw Resolution ───┐%s\n", COL_DRAW, RESET);
            break;

        case AUTODRAW_EVENT_TRIGGER:
            if (event->source && event->source->type) {
                printf("  %s│%s %s%s%s triggers!\n",
                       COL_DRAW, RESET,
                       faction_color(event->source->type->faction),
                       event->source->type->name, RESET);
            }
            break;

        case AUTODRAW_EVENT_CARD:
            if (event->drawn && event->drawn->type) {
                printf("  %s│%s  → Drew: %s%s%s\n",
                       COL_DRAW, RESET,
                       faction_color(event->drawn->type->faction),
                       event->drawn->type->name, RESET);
            }
            break;

        case AUTODRAW_EVENT_COMPLETE:
            printf("  %s└─── %d card(s) drawn ───┘%s\n",
                   COL_DRAW, event->total_drawn, RESET);
            printf("  %s has %d cards in hand\n",
                   player->name, player->deck->hand_count);
            break;
    }
}
/* }}} */

/* {{{ demo_effect_listener
 * Displays effect execution feedback.
 * Signature: (Game*, Player*, CardInstance* source, Effect*, void* context)
 */
static void demo_effect_listener(Game* game, Player* player,
                                  CardInstance* source, Effect* effect,
                                  void* context) {
    (void)game; (void)context; (void)player;

    char effect_str[64];
    effect_to_string(effect, effect_str, sizeof(effect_str));

    const char* source_name = source && source->type ? source->type->name : "Effect";

    printf("  %s⚡ %s → %s%s\n", DIM, source_name, effect_str, RESET);

    /* Special messages for certain effects */
    if (effect->type == EFFECT_UPGRADE_TRADE ||
        effect->type == EFFECT_UPGRADE_ATTACK ||
        effect->type == EFFECT_UPGRADE_AUTH) {
        printf("    %s→ Choose a card to upgrade!%s\n", COL_PENDING, RESET);
    }

    if (effect->type == EFFECT_SPAWN && effect->target_card_id) {
        printf("    %s→ Spawned %s to discard%s\n", COL_SUCCESS, effect->target_card_id, RESET);
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

    clear_screen();
    print_header("SYMBELINE REALMS - Phase 1 Demo");

    printf("\n");
    printf("  This demo showcases %sall Phase 1 game mechanics%s:\n", BOLD, RESET);
    printf("  • Card play, buying, and combat\n");
    printf("  • %sAuto-draw%s resolution chains\n", COL_DRAW, RESET);
    printf("  • Base deployment with %sfrontier/interior%s zones\n", BOLD, RESET);
    printf("  • %sSpawning%s mechanics\n", COL_SUCCESS, RESET);
    printf("  • %sUpgrade%s effects\n", COL_UPGRADE, RESET);
    printf("  • Ally abilities and scrap effects\n");
    printf("  • d10/d4 deck flow tracking\n");
    printf("  • Draw order choice\n");
    printf("\n");
    printf("  Two players take turns until one reaches 0 authority.\n");
    printf("  Type %s'h'%s for help at any time.\n", BOLD, RESET);

    /* Initialize effects system */
    effects_init();

    /* Register listeners */
    autodraw_register_listener(demo_autodraw_listener, NULL);
    effects_register_callback(demo_effect_listener, NULL);

    /* Create card types */
    CardType* scout;
    CardType* viper;
    CardType* explorer;
    create_starting_types(&scout, &viper, &explorer);

    CardType* wolf_pup = create_wolf_pup();
    CardType* mini_construct = create_mini_construct();

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
    game_register_card_type(game, mini_construct);

    /* Create trade row deck (multiple copies of each card) */
    int deck_size = trade_card_count * 3;  /* 3 copies each */
    CardType** trade_deck = malloc(deck_size * sizeof(CardType*));
    for (int i = 0; i < trade_card_count; i++) {
        for (int j = 0; j < 3; j++) {
            trade_deck[i * 3 + j] = trade_cards[i];
        }
    }

    game->trade_row = trade_row_create(trade_deck, deck_size, explorer);
    free(trade_deck);

    /* Start game */
    if (!game_start(game)) {
        printf("%s✗ Failed to start game!%s\n", COL_ERROR, RESET);
        return 1;
    }

    printf("\n  %s✓ Game started! Player 1 goes first.%s\n", COL_SUCCESS, RESET);
    printf("\n  %sPress Enter to continue...%s", DIM, RESET);
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
                    printf("\n  Game aborted.\n");
                    goto cleanup;
                }
                break;

            default:
                printf("  Unexpected phase: %s\n", game_phase_to_string(game->phase));
                break;
        }
    }

    /* Game over */
    clear_screen();
    print_header("GAME OVER");

    printf("\n");
    if (game->winner >= 0) {
        printf("  %s%s%s %sWINS!%s\n\n",
               COL_SUCCESS, BOLD, game->players[game->winner]->name, RESET, RESET);
    } else {
        printf("  %sIt's a draw!%s\n\n", BOLD, RESET);
    }

    printf("  %sFinal Scores:%s\n", UNDERLINE, RESET);
    for (int i = 0; i < game->player_count; i++) {
        Player* p = game->players[i];
        printf("  %s: Authority %s%d%s, d10: %d, d4: %d\n",
               p->name,
               p->authority <= 0 ? COL_DAMAGE : COL_SUCCESS,
               p->authority, RESET,
               p->d10, p->d4);
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

    printf("\n  Thanks for playing!\n\n");
    return 0;
}
/* }}} */
