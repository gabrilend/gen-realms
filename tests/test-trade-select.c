/*
 * test-trade-select.c - Unit Tests for LLM Trade Row Selection
 *
 * Tests faction balance tracking, candidate scoring, prompt building,
 * response parsing, and the complete selection callback.
 */

#include "../src/llm/08-trade-select.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Test counters */
static int tests_passed = 0;
static int tests_failed = 0;

/* {{{ Test helpers */
#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("  Testing %s...", #name); \
    test_##name(); \
    printf(" PASSED\n"); \
    tests_passed++; \
} while(0)

#define ASSERT(expr) do { \
    if (!(expr)) { \
        printf(" FAILED at line %d: %s\n", __LINE__, #expr); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_NE(a, b) ASSERT((a) != (b))
#define ASSERT_GT(a, b) ASSERT((a) > (b))
#define ASSERT_LT(a, b) ASSERT((a) < (b))
#define ASSERT_TRUE(a) ASSERT(a)
#define ASSERT_FALSE(a) ASSERT(!(a))
#define ASSERT_NULL(a) ASSERT((a) == NULL)
#define ASSERT_NOT_NULL(a) ASSERT((a) != NULL)
/* }}} */

/* {{{ Helper: create test card type */
static CardType* create_test_card(const char* id, const char* name,
                                   int cost, Faction faction) {
    CardType* card = card_type_create(id, name, cost, faction, CARD_KIND_SHIP);
    return card;
}
/* }}} */

/* {{{ Helper: create test trade row */
static TradeRow* create_test_trade_row(void) {
    /* Create a few test cards */
    CardType* cards[10];
    cards[0] = create_test_card("wolf", "Forest Wolf", 2, FACTION_WILDS);
    cards[1] = create_test_card("trader", "Trade Wagon", 3, FACTION_MERCHANT);
    cards[2] = create_test_card("knight", "Steel Knight", 4, FACTION_KINGDOM);
    cards[3] = create_test_card("construct", "Bronze Construct", 3, FACTION_ARTIFICER);
    cards[4] = create_test_card("bear", "Dire Bear", 5, FACTION_WILDS);
    cards[5] = create_test_card("caravan", "Merchant Caravan", 4, FACTION_MERCHANT);
    cards[6] = create_test_card("guard", "Royal Guard", 3, FACTION_KINGDOM);
    cards[7] = create_test_card("golem", "Iron Golem", 6, FACTION_ARTIFICER);
    cards[8] = create_test_card("scout", "Scout", 2, FACTION_NEUTRAL);
    cards[9] = create_test_card("bard", "Wandering Bard", 1, FACTION_NEUTRAL);

    CardType* explorer = create_test_card("explorer", "Explorer", 2, FACTION_NEUTRAL);

    TradeRow* row = trade_row_create(cards, 10, explorer);

    /* Free the card type array (trade_row_create copies them) */
    /* Note: Actually trade_row_create takes ownership, so don't free */

    return row;
}
/* }}} */

/* {{{ test_context_create_free */
TEST(context_create_free) {
    TradeSelectContext* ctx = trade_select_context_create(NULL, NULL, NULL);
    ASSERT_NOT_NULL(ctx);
    ASSERT_NULL(ctx->game);
    ASSERT_NULL(ctx->world_state);
    ASSERT_NULL(ctx->llm_config);
    ASSERT_FALSE(ctx->use_llm);

    trade_select_context_free(ctx);
}
/* }}} */

/* {{{ test_context_with_llm */
TEST(context_with_llm) {
    LLMConfig* config = llm_config_create();
    TradeSelectContext* ctx = trade_select_context_create(NULL, NULL, config);

    ASSERT_NOT_NULL(ctx);
    ASSERT_TRUE(ctx->use_llm);
    ASSERT_NOT_NULL(ctx->llm_config);

    trade_select_context_free(ctx);
    llm_config_free(config);
}
/* }}} */

/* {{{ test_get_balance_empty */
TEST(get_balance_empty) {
    /* Create row but don't fill slots */
    TradeRow* row = create_test_trade_row();

    /* Clear slots manually for this test */
    for (int i = 0; i < TRADE_ROW_SLOTS; i++) {
        if (row->slots[i]) {
            card_instance_free(row->slots[i]);
            row->slots[i] = NULL;
        }
    }

    FactionBalance balance = trade_select_get_balance(row);
    ASSERT_EQ(balance.neutral, 0);
    ASSERT_EQ(balance.merchant, 0);
    ASSERT_EQ(balance.wilds, 0);
    ASSERT_EQ(balance.kingdom, 0);
    ASSERT_EQ(balance.artificer, 0);

    trade_row_free(row);
}
/* }}} */

/* {{{ test_get_balance_filled */
TEST(get_balance_filled) {
    TradeRow* row = create_test_trade_row();
    trade_row_fill_slots(row);

    FactionBalance balance = trade_select_get_balance(row);

    /* Should have 5 total factions counted */
    int total = balance.neutral + balance.merchant + balance.wilds +
                balance.kingdom + balance.artificer;
    ASSERT_EQ(total, 5);

    trade_row_free(row);
}
/* }}} */

/* {{{ test_get_deck_balance */
TEST(get_deck_balance) {
    TradeRow* row = create_test_trade_row();

    /* After creation, all 10 cards should be in deck initially */
    FactionBalance balance = trade_select_get_deck_balance(row);

    /* We have: 2 wilds, 2 merchant, 2 kingdom, 2 artificer, 2 neutral */
    int total = balance.neutral + balance.merchant + balance.wilds +
                balance.kingdom + balance.artificer;
    ASSERT_GT(total, 0);

    trade_row_free(row);
}
/* }}} */

/* {{{ test_underrepresented_balanced */
TEST(underrepresented_balanced) {
    FactionBalance balance = {0, 1, 1, 1, 1};  /* All equal */
    Faction under = trade_select_underrepresented(balance);

    /* Should return one of the main factions (merchant is first checked) */
    ASSERT_NE(under, FACTION_NEUTRAL);
}
/* }}} */

/* {{{ test_underrepresented_wilds_low */
TEST(underrepresented_wilds_low) {
    FactionBalance balance = {0, 2, 0, 2, 2};  /* Wilds is 0 */
    Faction under = trade_select_underrepresented(balance);
    ASSERT_EQ(under, FACTION_WILDS);
}
/* }}} */

/* {{{ test_underrepresented_kingdom_low */
TEST(underrepresented_kingdom_low) {
    FactionBalance balance = {1, 2, 2, 0, 2};  /* Kingdom is 0 */
    Faction under = trade_select_underrepresented(balance);
    ASSERT_EQ(under, FACTION_KINGDOM);
}
/* }}} */

/* {{{ test_gather_candidates */
TEST(gather_candidates) {
    TradeRow* row = create_test_trade_row();

    CandidateCard candidates[5];
    int count = trade_select_gather_candidates(row, candidates, 5);

    ASSERT_GT(count, 0);
    ASSERT_TRUE(count <= 5);

    for (int i = 0; i < count; i++) {
        ASSERT_NOT_NULL(candidates[i].card);
        ASSERT_NOT_NULL(candidates[i].card->name);
    }

    trade_row_free(row);
}
/* }}} */

/* {{{ test_gather_candidates_null */
TEST(gather_candidates_null) {
    CandidateCard candidates[5];
    int count = trade_select_gather_candidates(NULL, candidates, 5);
    ASSERT_EQ(count, 0);

    TradeRow* row = create_test_trade_row();
    count = trade_select_gather_candidates(row, NULL, 5);
    ASSERT_EQ(count, 0);

    count = trade_select_gather_candidates(row, candidates, 0);
    ASSERT_EQ(count, 0);

    trade_row_free(row);
}
/* }}} */

/* {{{ test_score_faction_need_underrep */
TEST(score_faction_need_underrep) {
    CardType* wilds_card = create_test_card("test", "Test", 3, FACTION_WILDS);
    FactionBalance balance = {0, 2, 0, 2, 2};  /* Wilds is underrepresented */

    float score = trade_select_score_faction_need(wilds_card, balance);
    ASSERT_GT(score, 0.0f);

    card_type_free(wilds_card);
}
/* }}} */

/* {{{ test_score_faction_need_overrep */
TEST(score_faction_need_overrep) {
    CardType* merchant_card = create_test_card("test", "Test", 3, FACTION_MERCHANT);
    FactionBalance balance = {0, 3, 1, 1, 1};  /* Merchant is overrepresented */

    float score = trade_select_score_faction_need(merchant_card, balance);
    ASSERT_LT(score, 0.5f);  /* Should be low or zero */

    card_type_free(merchant_card);
}
/* }}} */

/* {{{ test_score_cost_variety_unique */
TEST(score_cost_variety_unique) {
    TradeRow* row = create_test_trade_row();
    trade_row_fill_slots(row);

    /* Create a card with a cost not in the row */
    CardType* card = create_test_card("test", "Test", 8, FACTION_NEUTRAL);

    float score = trade_select_score_cost_variety(card, row);
    ASSERT_EQ(score, 1.0f);  /* Unique cost should get max score */

    card_type_free(card);
    trade_row_free(row);
}
/* }}} */

/* {{{ test_score_candidate_complete */
TEST(score_candidate_complete) {
    TradeRow* row = create_test_trade_row();
    trade_row_fill_slots(row);

    TradeSelectContext* ctx = trade_select_context_create(NULL, NULL, NULL);

    CandidateCard candidate;
    candidate.card = create_test_card("test", "Test", 3, FACTION_WILDS);
    memset(&candidate.score, 0, sizeof(SelectionScore));

    trade_select_score_candidate(&candidate, row, ctx);

    /* All scores should be set */
    ASSERT_GT(candidate.score.total, 0.0f);

    card_type_free(candidate.card);
    trade_select_context_free(ctx);
    trade_row_free(row);
}
/* }}} */

/* {{{ test_build_prompt */
TEST(build_prompt) {
    CardType* card1 = create_test_card("wolf", "Forest Wolf", 2, FACTION_WILDS);
    CardType* card2 = create_test_card("trader", "Trade Wagon", 3, FACTION_MERCHANT);

    CandidateCard candidates[2];
    candidates[0].card = card1;
    candidates[1].card = card2;

    char* prompt = trade_select_build_prompt(candidates, 2, NULL);
    ASSERT_NOT_NULL(prompt);

    /* Should contain card names */
    ASSERT_TRUE(strstr(prompt, "Forest Wolf") != NULL);
    ASSERT_TRUE(strstr(prompt, "Trade Wagon") != NULL);

    free(prompt);
    card_type_free(card1);
    card_type_free(card2);
}
/* }}} */

/* {{{ test_build_prompt_null */
TEST(build_prompt_null) {
    char* prompt = trade_select_build_prompt(NULL, 0, NULL);
    ASSERT_NULL(prompt);
}
/* }}} */

/* {{{ test_parse_response_name */
TEST(parse_response_name) {
    CardType* card1 = create_test_card("wolf", "Forest Wolf", 2, FACTION_WILDS);
    CardType* card2 = create_test_card("trader", "Trade Wagon", 3, FACTION_MERCHANT);

    CandidateCard candidates[2];
    candidates[0].card = card1;
    candidates[1].card = card2;

    int idx = trade_select_parse_response("Forest Wolf", candidates, 2);
    ASSERT_EQ(idx, 0);

    idx = trade_select_parse_response("Trade Wagon", candidates, 2);
    ASSERT_EQ(idx, 1);

    card_type_free(card1);
    card_type_free(card2);
}
/* }}} */

/* {{{ test_parse_response_case_insensitive */
TEST(parse_response_case_insensitive) {
    CardType* card = create_test_card("wolf", "Forest Wolf", 2, FACTION_WILDS);

    CandidateCard candidates[1];
    candidates[0].card = card;

    int idx = trade_select_parse_response("FOREST WOLF", candidates, 1);
    ASSERT_EQ(idx, 0);

    idx = trade_select_parse_response("forest wolf", candidates, 1);
    ASSERT_EQ(idx, 0);

    card_type_free(card);
}
/* }}} */

/* {{{ test_parse_response_not_found */
TEST(parse_response_not_found) {
    CardType* card = create_test_card("wolf", "Forest Wolf", 2, FACTION_WILDS);

    CandidateCard candidates[1];
    candidates[0].card = card;

    int idx = trade_select_parse_response("Dire Bear", candidates, 1);
    ASSERT_EQ(idx, -1);

    card_type_free(card);
}
/* }}} */

/* {{{ test_parse_response_null */
TEST(parse_response_null) {
    int idx = trade_select_parse_response(NULL, NULL, 0);
    ASSERT_EQ(idx, -1);
}
/* }}} */

/* {{{ test_best_candidate_single */
TEST(best_candidate_single) {
    CandidateCard candidates[1];
    candidates[0].card = NULL;
    candidates[0].score.total = 0.5f;

    int best = trade_select_best_candidate(candidates, 1);
    ASSERT_EQ(best, 0);
}
/* }}} */

/* {{{ test_best_candidate_multiple */
TEST(best_candidate_multiple) {
    CandidateCard candidates[3];
    candidates[0].card = NULL;
    candidates[0].score.total = 0.3f;
    candidates[1].card = NULL;
    candidates[1].score.total = 0.8f;  /* Best */
    candidates[2].card = NULL;
    candidates[2].score.total = 0.5f;

    int best = trade_select_best_candidate(candidates, 3);
    ASSERT_EQ(best, 1);
}
/* }}} */

/* {{{ test_best_candidate_null */
TEST(best_candidate_null) {
    int best = trade_select_best_candidate(NULL, 0);
    ASSERT_EQ(best, -1);
}
/* }}} */

/* {{{ test_dm_callback_no_llm */
TEST(dm_callback_no_llm) {
    TradeRow* row = create_test_trade_row();
    TradeSelectContext* ctx = trade_select_context_create(NULL, NULL, NULL);

    CardType* selected = trade_select_dm_callback(row, ctx);

    /* Should return a card from the trade deck */
    /* (or NULL if deck is empty, which won't happen with our test setup) */
    ASSERT_NOT_NULL(selected);
    ASSERT_NOT_NULL(selected->name);

    trade_select_context_free(ctx);
    trade_row_free(row);
}
/* }}} */

/* {{{ test_dm_callback_null */
TEST(dm_callback_null) {
    CardType* selected = trade_select_dm_callback(NULL, NULL);
    ASSERT_NULL(selected);
}
/* }}} */

/* {{{ test_dm_callback_selects_from_deck */
TEST(dm_callback_selects_from_deck) {
    TradeRow* row = create_test_trade_row();
    TradeSelectContext* ctx = trade_select_context_create(NULL, NULL, NULL);

    /* Note which cards are in the deck */
    int initial_deck_count = row->trade_deck_count;

    CardType* selected = trade_select_dm_callback(row, ctx);

    /* Selected card should be valid */
    ASSERT_NOT_NULL(selected);

    /* And it should be one of our known cards */
    bool found = false;
    const char* known_cards[] = {
        "wolf", "trader", "knight", "construct", "bear",
        "caravan", "guard", "golem", "scout", "bard"
    };
    for (int i = 0; i < 10; i++) {
        if (selected->id && strcmp(selected->id, known_cards[i]) == 0) {
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
    (void)initial_deck_count;  /* Suppress unused warning */

    trade_select_context_free(ctx);
    trade_row_free(row);
}
/* }}} */

/* {{{ main */
int main(void) {
    printf("=== Trade Selection Tests ===\n\n");

    /* Initialize LLM (needed for config creation) */
    llm_init();

    printf("Context tests:\n");
    RUN_TEST(context_create_free);
    RUN_TEST(context_with_llm);

    printf("\nBalance tests:\n");
    RUN_TEST(get_balance_empty);
    RUN_TEST(get_balance_filled);
    RUN_TEST(get_deck_balance);

    printf("\nUnderrepresented tests:\n");
    RUN_TEST(underrepresented_balanced);
    RUN_TEST(underrepresented_wilds_low);
    RUN_TEST(underrepresented_kingdom_low);

    printf("\nCandidate gathering tests:\n");
    RUN_TEST(gather_candidates);
    RUN_TEST(gather_candidates_null);

    printf("\nScoring tests:\n");
    RUN_TEST(score_faction_need_underrep);
    RUN_TEST(score_faction_need_overrep);
    RUN_TEST(score_cost_variety_unique);
    RUN_TEST(score_candidate_complete);

    printf("\nPrompt building tests:\n");
    RUN_TEST(build_prompt);
    RUN_TEST(build_prompt_null);

    printf("\nResponse parsing tests:\n");
    RUN_TEST(parse_response_name);
    RUN_TEST(parse_response_case_insensitive);
    RUN_TEST(parse_response_not_found);
    RUN_TEST(parse_response_null);

    printf("\nBest candidate tests:\n");
    RUN_TEST(best_candidate_single);
    RUN_TEST(best_candidate_multiple);
    RUN_TEST(best_candidate_null);

    printf("\nDM callback tests:\n");
    RUN_TEST(dm_callback_no_llm);
    RUN_TEST(dm_callback_null);
    RUN_TEST(dm_callback_selects_from_deck);

    llm_cleanup();

    printf("\n=== Results: %d passed, %d failed ===\n",
           tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
/* }}} */
