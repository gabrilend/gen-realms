/*
 * 08-trade-select.h - LLM Trade Row Selection
 *
 * Implements LLM-guided trade row card selection. When a slot opens
 * in the trade row, the DM can select which card to place based on
 * game state, faction balance, and narrative considerations.
 */

#ifndef LLM_TRADE_SELECT_H
#define LLM_TRADE_SELECT_H

#include "../core/04-trade-row.h"
#include "../core/05-game.h"
#include "01-api-client.h"
#include "03-world-state.h"
#include <stdbool.h>

/* Maximum candidates to consider per selection */
#define TRADE_SELECT_MAX_CANDIDATES 10

/* {{{ FactionBalance
 * Tracks faction representation in the current trade row
 * for balanced card selection.
 */
typedef struct {
    int neutral;
    int merchant;
    int wilds;
    int kingdom;
    int artificer;
} FactionBalance;
/* }}} */

/* {{{ SelectionScore
 * Scoring factors for candidate card selection.
 * Higher scores are more desirable.
 */
typedef struct {
    float faction_need;      /* 0.0-1.0: underrepresented faction bonus */
    float singleton_bonus;   /* 0.0-1.0: bonus for less-bought cards */
    float cost_variety;      /* 0.0-1.0: bonus for filling cost gaps */
    float narrative_fit;     /* 0.0-1.0: LLM-assigned narrative score */
    float total;             /* Combined weighted score */
} SelectionScore;
/* }}} */

/* {{{ CandidateCard
 * A card being considered for selection with its score.
 */
typedef struct {
    CardType* card;
    SelectionScore score;
} CandidateCard;
/* }}} */

/* {{{ TradeSelectContext
 * Context passed to the DM selection callback.
 * Contains game state and LLM configuration.
 */
typedef struct {
    Game* game;              /* Current game state */
    WorldState* world_state; /* Narrative context */
    LLMConfig* llm_config;   /* LLM API configuration */
    bool use_llm;            /* True to use LLM, false for scoring only */
    float llm_weight;        /* Weight for LLM narrative score (0.0-1.0) */
} TradeSelectContext;
/* }}} */

/* {{{ trade_select_context_create
 * Creates a new selection context.
 * @param game - Current game state
 * @param world_state - Narrative context (can be NULL)
 * @param llm_config - LLM configuration (can be NULL for no LLM)
 */
TradeSelectContext* trade_select_context_create(Game* game,
                                                  WorldState* world_state,
                                                  LLMConfig* llm_config);
/* }}} */

/* {{{ trade_select_context_free
 * Frees selection context memory.
 * Does NOT free game, world_state, or llm_config.
 */
void trade_select_context_free(TradeSelectContext* ctx);
/* }}} */

/* {{{ trade_select_dm_callback
 * DM callback function for trade row selection.
 * Install with trade_row_set_dm(row, trade_select_dm_callback, context).
 * @param row - The trade row
 * @param context - TradeSelectContext pointer
 * @return Selected card type, or NULL for random selection
 */
CardType* trade_select_dm_callback(TradeRow* row, void* context);
/* }}} */

/* {{{ trade_select_get_balance
 * Gets current faction balance in the trade row.
 * @param row - The trade row
 * @return Faction balance counts
 */
FactionBalance trade_select_get_balance(TradeRow* row);
/* }}} */

/* {{{ trade_select_get_deck_balance
 * Gets faction balance in the remaining trade deck.
 * @param row - The trade row
 * @return Faction balance counts
 */
FactionBalance trade_select_get_deck_balance(TradeRow* row);
/* }}} */

/* {{{ trade_select_underrepresented
 * Returns the most underrepresented faction in the trade row.
 * @param balance - Current faction balance
 * @return Most underrepresented faction
 */
Faction trade_select_underrepresented(FactionBalance balance);
/* }}} */

/* {{{ trade_select_gather_candidates
 * Gathers candidate cards from trade deck for selection.
 * @param row - The trade row
 * @param candidates - Output array (must have space for max_count)
 * @param max_count - Maximum candidates to gather
 * @return Number of candidates gathered
 */
int trade_select_gather_candidates(TradeRow* row, CandidateCard* candidates,
                                    int max_count);
/* }}} */

/* {{{ trade_select_score_candidate
 * Scores a candidate card based on current game state.
 * @param candidate - Card to score (score fields will be filled)
 * @param row - Current trade row
 * @param ctx - Selection context
 */
void trade_select_score_candidate(CandidateCard* candidate, TradeRow* row,
                                   TradeSelectContext* ctx);
/* }}} */

/* {{{ trade_select_score_faction_need
 * Scores a card based on faction representation need.
 * @param card - Card to score
 * @param balance - Current trade row balance
 * @return Score 0.0-1.0 (higher = more needed)
 */
float trade_select_score_faction_need(CardType* card, FactionBalance balance);
/* }}} */

/* {{{ trade_select_score_singleton
 * Scores a card based on how rarely it has been bought.
 * @param card - Card to score
 * @param row - Trade row with buy counts
 * @return Score 0.0-1.0 (higher = less bought)
 */
float trade_select_score_singleton(CardType* card, TradeRow* row);
/* }}} */

/* {{{ trade_select_score_cost_variety
 * Scores a card based on cost distribution in trade row.
 * @param card - Card to score
 * @param row - Current trade row
 * @return Score 0.0-1.0 (higher = fills cost gap)
 */
float trade_select_score_cost_variety(CardType* card, TradeRow* row);
/* }}} */

/* {{{ trade_select_build_prompt
 * Builds LLM prompt for card selection.
 * Caller must free returned string.
 * @param candidates - Array of candidate cards
 * @param count - Number of candidates
 * @param ctx - Selection context
 */
char* trade_select_build_prompt(CandidateCard* candidates, int count,
                                 TradeSelectContext* ctx);
/* }}} */

/* {{{ trade_select_parse_response
 * Parses LLM response to find selected card.
 * @param response - LLM response text
 * @param candidates - Array of candidate cards
 * @param count - Number of candidates
 * @return Index of selected card, or -1 if not found
 */
int trade_select_parse_response(const char* response, CandidateCard* candidates,
                                 int count);
/* }}} */

/* {{{ trade_select_best_candidate
 * Returns the best candidate by total score.
 * @param candidates - Array of scored candidates
 * @param count - Number of candidates
 * @return Index of best candidate, or -1 if empty
 */
int trade_select_best_candidate(CandidateCard* candidates, int count);
/* }}} */

#endif /* LLM_TRADE_SELECT_H */
