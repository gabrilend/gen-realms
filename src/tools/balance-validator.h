/*
 * balance-validator.h - Card Balance Analysis Tool
 *
 * Analyzes the card database to identify balance issues:
 * - Value-per-cost ratios for each card
 * - Faction power curves by cost tier
 * - Statistical outliers (over/underpowered cards)
 * - Ally bonus distribution
 * - Scrap value analysis
 */

#ifndef TOOLS_BALANCE_VALIDATOR_H
#define TOOLS_BALANCE_VALIDATOR_H

#include <stdbool.h>

/* Maximum cards and factions */
#define MAX_CARDS 200
#define MAX_FACTIONS 5
#define MAX_COST_TIERS 10

/* {{{ Value Weights
 * Weights for different effect types when calculating card value.
 * These can be tuned based on gameplay analysis.
 */
typedef struct {
    float trade;          /* Trade generation */
    float combat;         /* Combat power */
    float authority;      /* Authority gain/heal */
    float draw;           /* Card draw (very powerful) */
    float discard;        /* Opponent discard */
    float scrap_trade;    /* Scrap from trade row */
    float scrap_own;      /* Scrap from hand/discard */
    float acquire_free;   /* Free card acquisition */
    float spawn;          /* Unit spawning */
    float upgrade;        /* Permanent upgrades */
    float defense;        /* Base defense per point */
    float outpost_bonus;  /* Bonus for being an outpost */
} ValueWeights;
/* }}} */

/* {{{ CardStats
 * Calculated statistics for a single card.
 */
typedef struct {
    char id[64];
    char name[64];
    char faction[16];
    char card_type[16];
    int cost;

    float base_value;      /* Value from main effects */
    float ally_value;      /* Value from ally effects */
    float scrap_value;     /* Value from scrap effects */
    float total_value;     /* Combined value */
    float value_per_cost;  /* Efficiency ratio */

    int defense;           /* For bases */
    bool is_outpost;       /* For bases */
    bool has_spawn;        /* Spawns units */
} CardStats;
/* }}} */

/* {{{ FactionStats
 * Aggregate statistics for a faction.
 */
typedef struct {
    char name[16];
    int card_count;
    float avg_value_per_cost;
    float power_curve[MAX_COST_TIERS];  /* Avg value at each cost tier */
    int cards_per_tier[MAX_COST_TIERS]; /* Count at each cost tier */
    float total_ally_value;
    float total_scrap_value;
} FactionStats;
/* }}} */

/* {{{ BalanceReport
 * Complete balance analysis results.
 */
typedef struct {
    CardStats cards[MAX_CARDS];
    int card_count;

    FactionStats factions[MAX_FACTIONS];
    int faction_count;

    /* Global statistics */
    float global_avg_vpc;        /* Average value-per-cost */
    float global_std_dev;        /* Standard deviation */
    float min_vpc;
    float max_vpc;

    /* Outliers (>2 std dev from mean) */
    CardStats* high_outliers[MAX_CARDS];
    int high_outlier_count;
    CardStats* low_outliers[MAX_CARDS];
    int low_outlier_count;

    /* Weights used for analysis */
    ValueWeights weights;
} BalanceReport;
/* }}} */

/* {{{ balance_create_default_weights
 * Creates default value weights based on typical card game balance.
 */
ValueWeights balance_create_default_weights(void);
/* }}} */

/* {{{ balance_load_cards
 * Loads all card JSON files from the assets/cards directory.
 * @param report - Report to populate
 * @param cards_dir - Path to cards directory
 * @return Number of cards loaded, or -1 on error
 */
int balance_load_cards(BalanceReport* report, const char* cards_dir);
/* }}} */

/* {{{ balance_calculate_card_value
 * Calculates the total value of a card based on its effects.
 * @param stats - Card stats to calculate (modified in place)
 * @param card_json - Parsed JSON for the card
 * @param weights - Value weights to use
 */
void balance_calculate_card_value(CardStats* stats, void* card_json,
                                   const ValueWeights* weights);
/* }}} */

/* {{{ balance_analyze
 * Performs full balance analysis on loaded cards.
 * @param report - Report with loaded cards (modified in place)
 */
void balance_analyze(BalanceReport* report);
/* }}} */

/* {{{ balance_calculate_faction_stats
 * Calculates per-faction statistics.
 * @param report - Report to analyze (modified in place)
 */
void balance_calculate_faction_stats(BalanceReport* report);
/* }}} */

/* {{{ balance_find_outliers
 * Identifies cards that are statistical outliers.
 * @param report - Report to analyze (modified in place)
 * @param threshold - Number of standard deviations for outlier (default 2.0)
 */
void balance_find_outliers(BalanceReport* report, float threshold);
/* }}} */

/* {{{ balance_print_report
 * Prints a human-readable balance report to stdout.
 * @param report - Report to print
 */
void balance_print_report(const BalanceReport* report);
/* }}} */

/* {{{ balance_print_faction_curves
 * Prints faction power curves.
 * @param report - Report to print
 */
void balance_print_faction_curves(const BalanceReport* report);
/* }}} */

/* {{{ balance_print_outliers
 * Prints outlier cards.
 * @param report - Report to print
 */
void balance_print_outliers(const BalanceReport* report);
/* }}} */

/* {{{ balance_print_all_cards
 * Prints all card statistics sorted by value-per-cost.
 * @param report - Report to print
 */
void balance_print_all_cards(const BalanceReport* report);
/* }}} */

/* {{{ balance_get_faction_index
 * Returns the index for a faction name.
 */
int balance_get_faction_index(const char* faction);
/* }}} */

/* {{{ balance_get_faction_name
 * Returns the display name for a faction.
 */
const char* balance_get_faction_name(int index);
/* }}} */

#endif /* TOOLS_BALANCE_VALIDATOR_H */
