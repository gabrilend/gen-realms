/*
 * 08-trade-select.c - LLM Trade Row Selection Implementation
 *
 * Implements LLM-guided trade row selection with scoring system.
 * Balances faction representation, singleton encouragement, and
 * narrative fit to create interesting marketplace situations.
 */

#include "08-trade-select.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Scoring weights for candidate selection */
#define WEIGHT_FACTION_NEED  0.30f
#define WEIGHT_SINGLETON     0.25f
#define WEIGHT_COST_VARIETY  0.20f
#define WEIGHT_NARRATIVE     0.25f

/* System prompt for trade row selection */
static const char* TRADE_SELECT_SYSTEM =
    "You are the Dungeon Master for Symbeline Realms, a fantasy card game.\n"
    "Your role is to select cards for the trade row that create interesting\n"
    "strategic and narrative opportunities. Consider:\n"
    "- Faction balance (don't let one faction dominate)\n"
    "- Player strategies (offer counters and synergies)\n"
    "- Narrative drama (exciting cards at climactic moments)\n"
    "Respond with ONLY the card name, nothing else.";

/* {{{ trade_select_context_create */
TradeSelectContext* trade_select_context_create(Game* game,
                                                  WorldState* world_state,
                                                  LLMConfig* llm_config) {
    TradeSelectContext* ctx = malloc(sizeof(TradeSelectContext));
    if (!ctx) return NULL;

    ctx->game = game;
    ctx->world_state = world_state;
    ctx->llm_config = llm_config;
    ctx->use_llm = (llm_config != NULL);
    ctx->llm_weight = WEIGHT_NARRATIVE;

    return ctx;
}
/* }}} */

/* {{{ trade_select_context_free */
void trade_select_context_free(TradeSelectContext* ctx) {
    if (!ctx) return;
    /* Note: Does NOT free game, world_state, or llm_config */
    free(ctx);
}
/* }}} */

/* {{{ faction_balance_add
 * Helper: adds a faction to the balance count.
 */
static void faction_balance_add(FactionBalance* balance, Faction faction) {
    switch (faction) {
        case FACTION_NEUTRAL:   balance->neutral++; break;
        case FACTION_MERCHANT:  balance->merchant++; break;
        case FACTION_WILDS:     balance->wilds++; break;
        case FACTION_KINGDOM:   balance->kingdom++; break;
        case FACTION_ARTIFICER: balance->artificer++; break;
        default: break;
    }
}
/* }}} */

/* {{{ trade_select_get_balance */
FactionBalance trade_select_get_balance(TradeRow* row) {
    FactionBalance balance = {0, 0, 0, 0, 0};
    if (!row) return balance;

    for (int i = 0; i < TRADE_ROW_SLOTS; i++) {
        if (row->slots[i] && row->slots[i]->type) {
            faction_balance_add(&balance, row->slots[i]->type->faction);
        }
    }

    return balance;
}
/* }}} */

/* {{{ trade_select_get_deck_balance */
FactionBalance trade_select_get_deck_balance(TradeRow* row) {
    FactionBalance balance = {0, 0, 0, 0, 0};
    if (!row) return balance;

    for (int i = 0; i < row->trade_deck_count; i++) {
        if (row->trade_deck[i]) {
            faction_balance_add(&balance, row->trade_deck[i]->faction);
        }
    }

    return balance;
}
/* }}} */

/* {{{ trade_select_underrepresented */
Faction trade_select_underrepresented(FactionBalance balance) {
    /* Find minimum count (excluding neutral) */
    int min_count = balance.merchant;
    Faction min_faction = FACTION_MERCHANT;

    if (balance.wilds < min_count) {
        min_count = balance.wilds;
        min_faction = FACTION_WILDS;
    }
    if (balance.kingdom < min_count) {
        min_count = balance.kingdom;
        min_faction = FACTION_KINGDOM;
    }
    if (balance.artificer < min_count) {
        min_count = balance.artificer;
        min_faction = FACTION_ARTIFICER;
    }

    return min_faction;
}
/* }}} */

/* {{{ trade_select_gather_candidates */
int trade_select_gather_candidates(TradeRow* row, CandidateCard* candidates,
                                    int max_count) {
    if (!row || !candidates || max_count <= 0) return 0;

    int count = 0;
    int limit = (row->trade_deck_count < max_count) ?
                row->trade_deck_count : max_count;

    for (int i = 0; i < limit && count < max_count; i++) {
        if (row->trade_deck[i]) {
            candidates[count].card = row->trade_deck[i];
            memset(&candidates[count].score, 0, sizeof(SelectionScore));
            count++;
        }
    }

    return count;
}
/* }}} */

/* {{{ trade_select_score_faction_need */
float trade_select_score_faction_need(CardType* card, FactionBalance balance) {
    if (!card) return 0.0f;

    /* Get total and faction-specific counts */
    int total = balance.neutral + balance.merchant + balance.wilds +
                balance.kingdom + balance.artificer;

    if (total == 0) return 0.5f;  /* Empty row, neutral score */

    int faction_count;
    switch (card->faction) {
        case FACTION_NEUTRAL:   faction_count = balance.neutral; break;
        case FACTION_MERCHANT:  faction_count = balance.merchant; break;
        case FACTION_WILDS:     faction_count = balance.wilds; break;
        case FACTION_KINGDOM:   faction_count = balance.kingdom; break;
        case FACTION_ARTIFICER: faction_count = balance.artificer; break;
        default: return 0.0f;
    }

    /* Calculate ideal count per faction (roughly 1 each in 5 slots) */
    float ideal = (float)total / 4.0f;  /* 4 main factions */
    float current = (float)faction_count;

    /* Score based on how much below ideal */
    if (current >= ideal) return 0.0f;
    return (ideal - current) / ideal;
}
/* }}} */

/* {{{ trade_select_score_singleton */
float trade_select_score_singleton(CardType* card, TradeRow* row) {
    if (!card || !row || !row->card_buy_counts) return 0.5f;

    /* Find card index in buy counts (this assumes card IDs match indices) */
    /* For now, use a simple hash-based lookup */
    int max_buys = 1;
    int card_buys = 0;

    /* Find max buys and this card's buys by comparing card IDs */
    for (int i = 0; i < row->card_type_count; i++) {
        if (row->card_buy_counts[i] > max_buys) {
            max_buys = row->card_buy_counts[i];
        }
    }

    /* Simple linear search for this card's index */
    /* In a real implementation, we'd have a proper mapping */
    /* For now, return middle score as we don't have the mapping */
    (void)card_buys;  /* Suppress unused warning */

    /* Cards that haven't been bought get max bonus */
    return 1.0f;  /* Default to high score for singleton encouragement */
}
/* }}} */

/* {{{ trade_select_score_cost_variety */
float trade_select_score_cost_variety(CardType* card, TradeRow* row) {
    if (!card || !row) return 0.5f;

    /* Count costs present in trade row */
    int cost_counts[10] = {0};  /* Costs 0-9+ */

    for (int i = 0; i < TRADE_ROW_SLOTS; i++) {
        if (row->slots[i] && row->slots[i]->type) {
            int cost = row->slots[i]->type->cost;
            if (cost > 9) cost = 9;
            cost_counts[cost]++;
        }
    }

    /* Score based on how rare this cost is in current row */
    int card_cost = card->cost;
    if (card_cost > 9) card_cost = 9;

    if (cost_counts[card_cost] == 0) return 1.0f;  /* No card at this cost */
    if (cost_counts[card_cost] == 1) return 0.5f;  /* One card at this cost */
    return 0.2f;  /* Multiple cards at this cost */
}
/* }}} */

/* {{{ trade_select_score_candidate */
void trade_select_score_candidate(CandidateCard* candidate, TradeRow* row,
                                   TradeSelectContext* ctx) {
    if (!candidate || !candidate->card) return;

    FactionBalance balance = trade_select_get_balance(row);

    candidate->score.faction_need =
        trade_select_score_faction_need(candidate->card, balance);

    candidate->score.singleton_bonus =
        trade_select_score_singleton(candidate->card, row);

    candidate->score.cost_variety =
        trade_select_score_cost_variety(candidate->card, row);

    /* Narrative fit is set by LLM, default to neutral */
    candidate->score.narrative_fit = 0.5f;

    /* Calculate total weighted score */
    float llm_weight = (ctx && ctx->use_llm) ? ctx->llm_weight : 0.0f;
    float base_weight = 1.0f - llm_weight;

    candidate->score.total =
        (candidate->score.faction_need * WEIGHT_FACTION_NEED +
         candidate->score.singleton_bonus * WEIGHT_SINGLETON +
         candidate->score.cost_variety * WEIGHT_COST_VARIETY) * base_weight +
        candidate->score.narrative_fit * llm_weight;
}
/* }}} */

/* {{{ trade_select_build_prompt */
char* trade_select_build_prompt(CandidateCard* candidates, int count,
                                 TradeSelectContext* ctx) {
    if (!candidates || count <= 0) return NULL;

    /* Build cards list */
    char cards_list[2048] = "";
    int offset = 0;

    for (int i = 0; i < count && offset < 1900; i++) {
        if (candidates[i].card) {
            offset += snprintf(cards_list + offset, sizeof(cards_list) - offset,
                "%d. %s (%s, cost %d)\n",
                i + 1,
                candidates[i].card->name,
                faction_to_string(candidates[i].card->faction),
                candidates[i].card->cost);
        }
    }

    /* Build game state summary */
    char state_summary[1024] = "Current game state:\n";
    if (ctx && ctx->game) {
        snprintf(state_summary, sizeof(state_summary),
            "Turn %d. Players have %d and %d authority.\n",
            ctx->game->turn_number,
            ctx->game->players[0]->authority,
            ctx->game->players[1]->authority);
    }

    /* Build trade row summary */
    char row_summary[512] = "";
    if (ctx && ctx->game && ctx->game->trade_row) {
        FactionBalance bal = trade_select_get_balance(ctx->game->trade_row);
        snprintf(row_summary, sizeof(row_summary),
            "Trade row factions: Merchant=%d, Wilds=%d, Kingdom=%d, Artificer=%d\n",
            bal.merchant, bal.wilds, bal.kingdom, bal.artificer);
    }

    /* Combine into final prompt */
    size_t total_len = strlen(state_summary) + strlen(row_summary) +
                       strlen(cards_list) + 256;
    char* prompt = malloc(total_len);
    if (!prompt) return NULL;

    snprintf(prompt, total_len,
        "%s%s\n"
        "Available cards to add to trade row:\n%s\n"
        "Select the card that would create the most interesting "
        "strategic and narrative opportunity.\n"
        "Respond with ONLY the card name.",
        state_summary, row_summary, cards_list);

    return prompt;
}
/* }}} */

/* {{{ str_contains_ignore_case
 * Helper: case-insensitive substring search.
 */
static bool str_contains_ignore_case(const char* haystack, const char* needle) {
    if (!haystack || !needle) return false;

    size_t h_len = strlen(haystack);
    size_t n_len = strlen(needle);
    if (n_len > h_len) return false;

    for (size_t i = 0; i <= h_len - n_len; i++) {
        bool match = true;
        for (size_t j = 0; j < n_len && match; j++) {
            if (tolower((unsigned char)haystack[i + j]) !=
                tolower((unsigned char)needle[j])) {
                match = false;
            }
        }
        if (match) return true;
    }
    return false;
}
/* }}} */

/* {{{ trade_select_parse_response */
int trade_select_parse_response(const char* response, CandidateCard* candidates,
                                 int count) {
    if (!response || !candidates || count <= 0) return -1;

    /* Try to find each candidate's name in the response */
    for (int i = 0; i < count; i++) {
        if (candidates[i].card && candidates[i].card->name) {
            if (str_contains_ignore_case(response, candidates[i].card->name)) {
                return i;
            }
        }
    }

    /* Also try matching by card ID */
    for (int i = 0; i < count; i++) {
        if (candidates[i].card && candidates[i].card->id) {
            if (str_contains_ignore_case(response, candidates[i].card->id)) {
                return i;
            }
        }
    }

    return -1;  /* Not found */
}
/* }}} */

/* {{{ trade_select_best_candidate */
int trade_select_best_candidate(CandidateCard* candidates, int count) {
    if (!candidates || count <= 0) return -1;

    int best_idx = 0;
    float best_score = candidates[0].score.total;

    for (int i = 1; i < count; i++) {
        if (candidates[i].score.total > best_score) {
            best_score = candidates[i].score.total;
            best_idx = i;
        }
    }

    return best_idx;
}
/* }}} */

/* {{{ trade_select_dm_callback */
CardType* trade_select_dm_callback(TradeRow* row, void* context) {
    TradeSelectContext* ctx = (TradeSelectContext*)context;
    if (!row || !ctx) return NULL;

    /* Gather candidates from trade deck */
    CandidateCard candidates[TRADE_SELECT_MAX_CANDIDATES];
    int count = trade_select_gather_candidates(row, candidates,
                                                TRADE_SELECT_MAX_CANDIDATES);

    if (count <= 0) return NULL;  /* No candidates, use random */

    /* Score each candidate */
    for (int i = 0; i < count; i++) {
        trade_select_score_candidate(&candidates[i], row, ctx);
    }

    /* Try LLM selection if enabled */
    if (ctx->use_llm && ctx->llm_config) {
        char* prompt = trade_select_build_prompt(candidates, count, ctx);
        if (prompt) {
            LLMResponse* response = llm_request(ctx->llm_config,
                                                 TRADE_SELECT_SYSTEM,
                                                 prompt);
            free(prompt);

            if (response && response->success && response->text) {
                int llm_choice = trade_select_parse_response(response->text,
                                                              candidates, count);
                if (llm_choice >= 0) {
                    /* LLM made a valid selection */
                    candidates[llm_choice].score.narrative_fit = 1.0f;
                    /* Recalculate total with narrative boost */
                    candidates[llm_choice].score.total =
                        candidates[llm_choice].score.faction_need * WEIGHT_FACTION_NEED +
                        candidates[llm_choice].score.singleton_bonus * WEIGHT_SINGLETON +
                        candidates[llm_choice].score.cost_variety * WEIGHT_COST_VARIETY +
                        1.0f * ctx->llm_weight;  /* Full narrative bonus */
                }
            }

            if (response) llm_response_free(response);
        }
    }

    /* Select best candidate by total score */
    int best = trade_select_best_candidate(candidates, count);
    if (best < 0) return NULL;

    /* Return selected card type */
    /* Note: We need to find and return it from the trade deck, not a copy */
    CardType* selected = candidates[best].card;

    /* Find this card in the trade deck and swap it to position 0 */
    /* so trade_row_select_next() will return it */
    for (int i = 0; i < row->trade_deck_count; i++) {
        if (row->trade_deck[i] == selected) {
            /* Swap to front */
            CardType* temp = row->trade_deck[0];
            row->trade_deck[0] = row->trade_deck[i];
            row->trade_deck[i] = temp;
            break;
        }
    }

    return selected;
}
/* }}} */
