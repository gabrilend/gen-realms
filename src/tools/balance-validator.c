/*
 * balance-validator.c - Card Balance Analysis Tool
 *
 * Analyzes the card database for balance issues using weighted
 * value calculations and statistical outlier detection.
 */

#include "balance-validator.h"
#include "../../libs/cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <math.h>

/* Faction names for indexing */
static const char* FACTION_NAMES[] = {
    "neutral", "merchant", "wilds", "kingdom", "artificer"
};

/* {{{ balance_create_default_weights
 * Default weights tuned for typical deck-builder balance.
 * Trade and combat are base value, draw is premium.
 */
ValueWeights balance_create_default_weights(void) {
    ValueWeights w = {
        .trade = 1.0f,
        .combat = 1.0f,
        .authority = 0.5f,      /* Health is less valuable than resources */
        .draw = 2.0f,           /* Card draw is very powerful */
        .discard = 1.5f,        /* Disruption is strong */
        .scrap_trade = 0.5f,    /* Scrapping trade row is situational */
        .scrap_own = 1.0f,      /* Deck thinning is valuable */
        .acquire_free = 2.5f,   /* Free cards are strong (per cost avoided) */
        .spawn = 1.5f,          /* Spawning creates recurring value */
        .upgrade = 2.0f,        /* Permanent bonuses are very strong */
        .defense = 0.3f,        /* Defense per point */
        .outpost_bonus = 1.0f   /* Bonus for must-attack property */
    };
    return w;
}
/* }}} */

/* {{{ balance_get_faction_index */
int balance_get_faction_index(const char* faction) {
    if (!faction) return 0;
    for (int i = 0; i < MAX_FACTIONS; i++) {
        if (strcmp(faction, FACTION_NAMES[i]) == 0) return i;
    }
    return 0; /* Default to neutral */
}
/* }}} */

/* {{{ balance_get_faction_name */
const char* balance_get_faction_name(int index) {
    if (index < 0 || index >= MAX_FACTIONS) return "unknown";
    return FACTION_NAMES[index];
}
/* }}} */

/* {{{ calculate_effect_value
 * Calculates the value of a single effect.
 */
static float calculate_effect_value(cJSON* effect, const ValueWeights* w) {
    if (!effect || !w) return 0.0f;

    cJSON* type_json = cJSON_GetObjectItem(effect, "type");
    cJSON* value_json = cJSON_GetObjectItem(effect, "value");

    if (!type_json || !cJSON_IsString(type_json)) return 0.0f;

    const char* type = type_json->valuestring;
    int value = value_json && cJSON_IsNumber(value_json) ? value_json->valueint : 1;

    /* Map effect types to weighted values */
    if (strcmp(type, "add_trade") == 0) {
        return value * w->trade;
    } else if (strcmp(type, "add_combat") == 0) {
        return value * w->combat;
    } else if (strcmp(type, "add_authority") == 0) {
        return value * w->authority;
    } else if (strcmp(type, "add_coin") == 0) {
        return value * w->trade * 0.8f; /* Coins are delayed trade */
    } else if (strcmp(type, "draw_card") == 0) {
        return value * w->draw;
    } else if (strcmp(type, "opponent_discard") == 0) {
        return value * w->discard;
    } else if (strcmp(type, "scrap_trade_row") == 0) {
        return w->scrap_trade;
    } else if (strcmp(type, "scrap_hand_or_discard") == 0) {
        return w->scrap_own;
    } else if (strcmp(type, "acquire_free") == 0) {
        cJSON* max_cost = cJSON_GetObjectItem(effect, "max_cost");
        int cost = max_cost && cJSON_IsNumber(max_cost) ? max_cost->valueint : 3;
        return cost * w->acquire_free * 0.5f; /* Avg value is half max */
    } else if (strcmp(type, "spawn") == 0) {
        return w->spawn;
    } else if (strcmp(type, "upgrade_attack") == 0 ||
               strcmp(type, "upgrade_trade") == 0 ||
               strcmp(type, "upgrade_authority") == 0) {
        return value * w->upgrade;
    } else if (strcmp(type, "copy_ship") == 0) {
        return 3.0f; /* Approximate average ship value */
    } else if (strcmp(type, "recruit") == 0) {
        return 2.0f; /* Recruiting is situationally powerful */
    }

    return 0.0f;
}
/* }}} */

/* {{{ calculate_effects_total
 * Sums the value of an effects array.
 */
static float calculate_effects_total(cJSON* effects, const ValueWeights* w) {
    if (!effects || !cJSON_IsArray(effects)) return 0.0f;

    float total = 0.0f;
    cJSON* effect;
    cJSON_ArrayForEach(effect, effects) {
        total += calculate_effect_value(effect, w);
    }
    return total;
}
/* }}} */

/* {{{ balance_calculate_card_value */
void balance_calculate_card_value(CardStats* stats, void* card_json,
                                   const ValueWeights* weights) {
    if (!stats || !card_json || !weights) return;

    cJSON* card = (cJSON*)card_json;

    /* Basic info */
    cJSON* id = cJSON_GetObjectItem(card, "id");
    cJSON* name = cJSON_GetObjectItem(card, "name");
    cJSON* faction = cJSON_GetObjectItem(card, "faction");
    cJSON* card_type = cJSON_GetObjectItem(card, "card_type");
    cJSON* cost = cJSON_GetObjectItem(card, "cost");
    cJSON* defense = cJSON_GetObjectItem(card, "defense");
    cJSON* is_outpost = cJSON_GetObjectItem(card, "is_outpost");
    cJSON* spawns = cJSON_GetObjectItem(card, "spawns");

    if (id && cJSON_IsString(id)) {
        strncpy(stats->id, id->valuestring, sizeof(stats->id) - 1);
    }
    if (name && cJSON_IsString(name)) {
        strncpy(stats->name, name->valuestring, sizeof(stats->name) - 1);
    }
    if (faction && cJSON_IsString(faction)) {
        strncpy(stats->faction, faction->valuestring, sizeof(stats->faction) - 1);
    }
    if (card_type && cJSON_IsString(card_type)) {
        strncpy(stats->card_type, card_type->valuestring, sizeof(stats->card_type) - 1);
    }

    stats->cost = cost && cJSON_IsNumber(cost) ? cost->valueint : 0;
    stats->defense = defense && cJSON_IsNumber(defense) ? defense->valueint : 0;
    stats->is_outpost = is_outpost && cJSON_IsTrue(is_outpost);
    stats->has_spawn = spawns && cJSON_IsString(spawns);

    /* Calculate effect values */
    cJSON* effects = cJSON_GetObjectItem(card, "effects");
    cJSON* ally_effects = cJSON_GetObjectItem(card, "ally_effects");
    cJSON* scrap_effects = cJSON_GetObjectItem(card, "scrap_effects");

    stats->base_value = calculate_effects_total(effects, weights);
    stats->ally_value = calculate_effects_total(ally_effects, weights) * 0.6f; /* Ally conditional */
    stats->scrap_value = calculate_effects_total(scrap_effects, weights) * 0.4f; /* One-time use */

    /* Add base-specific values */
    if (strcmp(stats->card_type, "base") == 0) {
        stats->base_value += stats->defense * weights->defense;
        if (stats->is_outpost) {
            stats->base_value += weights->outpost_bonus;
        }
        if (stats->has_spawn) {
            stats->base_value += weights->spawn * 2.0f; /* Recurring spawns */
        }
    }

    /* Total value */
    stats->total_value = stats->base_value + stats->ally_value + stats->scrap_value;

    /* Value per cost (avoid division by zero for starting cards) */
    if (stats->cost > 0) {
        stats->value_per_cost = stats->total_value / (float)stats->cost;
    } else {
        stats->value_per_cost = stats->total_value; /* Starting cards have "infinite" efficiency */
    }
}
/* }}} */

/* {{{ load_card_file
 * Loads a single card JSON file and adds it to the report.
 */
static int load_card_file(BalanceReport* report, const char* filepath,
                          const ValueWeights* weights) {
    FILE* f = fopen(filepath, "r");
    if (!f) return -1;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* content = malloc(len + 1);
    if (!content) {
        fclose(f);
        return -1;
    }

    size_t read_len = fread(content, 1, len, f);
    content[read_len] = '\0';
    fclose(f);

    cJSON* card = cJSON_Parse(content);
    free(content);

    if (!card) return -1;

    /* Skip schema file */
    cJSON* schema = cJSON_GetObjectItem(card, "$schema");
    if (schema) {
        cJSON_Delete(card);
        return 0;
    }

    /* Skip token cards (they're spawned, not bought) */
    cJSON* id = cJSON_GetObjectItem(card, "id");
    if (id && cJSON_IsString(id)) {
        const char* id_str = id->valuestring;
        if (strstr(id_str, "_token") != NULL) {
            cJSON_Delete(card);
            return 0;
        }
    }

    if (report->card_count >= MAX_CARDS) {
        cJSON_Delete(card);
        return -1;
    }

    CardStats* stats = &report->cards[report->card_count];
    memset(stats, 0, sizeof(CardStats));

    balance_calculate_card_value(stats, card, weights);
    report->card_count++;

    cJSON_Delete(card);
    return 1;
}
/* }}} */

/* {{{ load_cards_from_dir
 * Recursively loads cards from a directory.
 */
static int load_cards_from_dir(BalanceReport* report, const char* dir_path,
                               const ValueWeights* weights) {
    DIR* dir = opendir(dir_path);
    if (!dir) return -1;

    int loaded = 0;
    struct dirent* entry;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char path[512];
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        /* Check if it's a directory */
        DIR* subdir = opendir(path);
        if (subdir) {
            closedir(subdir);
            int sub_loaded = load_cards_from_dir(report, path, weights);
            if (sub_loaded > 0) loaded += sub_loaded;
        } else if (strstr(entry->d_name, ".json") != NULL) {
            int result = load_card_file(report, path, weights);
            if (result > 0) loaded++;
        }
    }

    closedir(dir);
    return loaded;
}
/* }}} */

/* {{{ balance_load_cards */
int balance_load_cards(BalanceReport* report, const char* cards_dir) {
    if (!report || !cards_dir) return -1;

    memset(report, 0, sizeof(BalanceReport));
    report->weights = balance_create_default_weights();

    return load_cards_from_dir(report, cards_dir, &report->weights);
}
/* }}} */

/* {{{ balance_calculate_faction_stats */
void balance_calculate_faction_stats(BalanceReport* report) {
    if (!report) return;

    /* Initialize faction stats */
    report->faction_count = MAX_FACTIONS;
    for (int i = 0; i < MAX_FACTIONS; i++) {
        memset(&report->factions[i], 0, sizeof(FactionStats));
        strncpy(report->factions[i].name, FACTION_NAMES[i],
                sizeof(report->factions[i].name) - 1);
    }

    /* Accumulate per-faction data */
    for (int i = 0; i < report->card_count; i++) {
        CardStats* card = &report->cards[i];
        int fi = balance_get_faction_index(card->faction);

        if (card->cost > 0) { /* Skip starting cards */
            report->factions[fi].card_count++;
            report->factions[fi].avg_value_per_cost += card->value_per_cost;

            int tier = card->cost < MAX_COST_TIERS ? card->cost : MAX_COST_TIERS - 1;
            report->factions[fi].power_curve[tier] += card->total_value;
            report->factions[fi].cards_per_tier[tier]++;
        }

        report->factions[fi].total_ally_value += card->ally_value;
        report->factions[fi].total_scrap_value += card->scrap_value;
    }

    /* Calculate averages */
    for (int i = 0; i < MAX_FACTIONS; i++) {
        FactionStats* f = &report->factions[i];
        if (f->card_count > 0) {
            f->avg_value_per_cost /= f->card_count;
        }

        for (int t = 0; t < MAX_COST_TIERS; t++) {
            if (f->cards_per_tier[t] > 0) {
                f->power_curve[t] /= f->cards_per_tier[t];
            }
        }
    }
}
/* }}} */

/* {{{ balance_analyze */
void balance_analyze(BalanceReport* report) {
    if (!report || report->card_count == 0) return;

    /* Calculate global statistics (excluding cost-0 cards) */
    float sum = 0.0f;
    int count = 0;
    report->min_vpc = 999.0f;
    report->max_vpc = 0.0f;

    for (int i = 0; i < report->card_count; i++) {
        CardStats* card = &report->cards[i];
        if (card->cost > 0) {
            sum += card->value_per_cost;
            count++;

            if (card->value_per_cost < report->min_vpc) {
                report->min_vpc = card->value_per_cost;
            }
            if (card->value_per_cost > report->max_vpc) {
                report->max_vpc = card->value_per_cost;
            }
        }
    }

    if (count > 0) {
        report->global_avg_vpc = sum / count;

        /* Calculate standard deviation */
        float var_sum = 0.0f;
        for (int i = 0; i < report->card_count; i++) {
            CardStats* card = &report->cards[i];
            if (card->cost > 0) {
                float diff = card->value_per_cost - report->global_avg_vpc;
                var_sum += diff * diff;
            }
        }
        report->global_std_dev = sqrtf(var_sum / count);
    }

    /* Calculate faction stats */
    balance_calculate_faction_stats(report);

    /* Find outliers with default 2.0 threshold */
    balance_find_outliers(report, 2.0f);
}
/* }}} */

/* {{{ balance_find_outliers */
void balance_find_outliers(BalanceReport* report, float threshold) {
    if (!report) return;

    report->high_outlier_count = 0;
    report->low_outlier_count = 0;

    float high_threshold = report->global_avg_vpc + (threshold * report->global_std_dev);
    float low_threshold = report->global_avg_vpc - (threshold * report->global_std_dev);

    for (int i = 0; i < report->card_count; i++) {
        CardStats* card = &report->cards[i];
        if (card->cost == 0) continue; /* Skip starting cards */

        if (card->value_per_cost > high_threshold) {
            if (report->high_outlier_count < MAX_CARDS) {
                report->high_outliers[report->high_outlier_count++] = card;
            }
        } else if (card->value_per_cost < low_threshold) {
            if (report->low_outlier_count < MAX_CARDS) {
                report->low_outliers[report->low_outlier_count++] = card;
            }
        }
    }
}
/* }}} */

/* {{{ compare_cards_by_vpc
 * Comparison function for sorting cards by value-per-cost.
 */
static int compare_cards_by_vpc(const void* a, const void* b) {
    const CardStats* ca = (const CardStats*)a;
    const CardStats* cb = (const CardStats*)b;
    float diff = cb->value_per_cost - ca->value_per_cost;
    if (diff > 0) return 1;
    if (diff < 0) return -1;
    return 0;
}
/* }}} */

/* {{{ balance_print_report */
void balance_print_report(const BalanceReport* report) {
    if (!report) return;

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════════╗\n");
    printf("║                    SYMBELINE REALMS                              ║\n");
    printf("║                  Card Balance Report                             ║\n");
    printf("╚══════════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    printf("=== Summary ===\n\n");
    printf("  Cards Analyzed:      %d\n", report->card_count);
    printf("  Global Avg VPC:      %.2f\n", report->global_avg_vpc);
    printf("  Standard Deviation:  %.2f\n", report->global_std_dev);
    printf("  VPC Range:           %.2f - %.2f\n", report->min_vpc, report->max_vpc);
    printf("  High Outliers:       %d (>%.2f VPC)\n",
           report->high_outlier_count,
           report->global_avg_vpc + 2.0f * report->global_std_dev);
    printf("  Low Outliers:        %d (<%.2f VPC)\n",
           report->low_outlier_count,
           report->global_avg_vpc - 2.0f * report->global_std_dev);
    printf("\n");

    /* Print faction summaries */
    printf("=== Faction Summary ===\n\n");
    printf("  %-12s  %5s  %8s  %8s  %8s\n",
           "Faction", "Cards", "Avg VPC", "Ally Val", "Scrap Val");
    printf("  %-12s  %5s  %8s  %8s  %8s\n",
           "------------", "-----", "--------", "--------", "---------");

    for (int i = 0; i < MAX_FACTIONS; i++) {
        const FactionStats* f = &report->factions[i];
        if (f->card_count > 0) {
            printf("  %-12s  %5d  %8.2f  %8.2f  %8.2f\n",
                   f->name, f->card_count, f->avg_value_per_cost,
                   f->total_ally_value, f->total_scrap_value);
        }
    }
    printf("\n");

    balance_print_faction_curves(report);
    balance_print_outliers(report);
}
/* }}} */

/* {{{ balance_print_faction_curves */
void balance_print_faction_curves(const BalanceReport* report) {
    if (!report) return;

    printf("=== Faction Power Curves (Avg Value by Cost) ===\n\n");
    printf("  %-12s ", "Cost:");
    for (int t = 1; t < 9; t++) {
        printf("  %4d ", t);
    }
    printf("\n");

    printf("  %-12s ", "------------");
    for (int t = 1; t < 9; t++) {
        printf(" -----");
    }
    printf("\n");

    for (int i = 0; i < MAX_FACTIONS; i++) {
        const FactionStats* f = &report->factions[i];
        if (f->card_count == 0) continue;

        printf("  %-12s ", f->name);
        for (int t = 1; t < 9; t++) {
            if (f->cards_per_tier[t] > 0) {
                printf(" %5.1f", f->power_curve[t]);
            } else {
                printf("     -");
            }
        }
        printf("\n");
    }
    printf("\n");
}
/* }}} */

/* {{{ balance_print_outliers */
void balance_print_outliers(const BalanceReport* report) {
    if (!report) return;

    if (report->high_outlier_count > 0) {
        printf("=== High Outliers (Potentially Overpowered) ===\n\n");
        for (int i = 0; i < report->high_outlier_count; i++) {
            const CardStats* c = report->high_outliers[i];
            printf("  [HIGH] %-20s  Cost: %d  VPC: %.2f  (avg: %.2f)\n",
                   c->name, c->cost, c->value_per_cost, report->global_avg_vpc);
            printf("         Faction: %-10s  Base: %.1f  Ally: %.1f  Scrap: %.1f\n",
                   c->faction, c->base_value, c->ally_value, c->scrap_value);
        }
        printf("\n");
    }

    if (report->low_outlier_count > 0) {
        printf("=== Low Outliers (Potentially Underpowered) ===\n\n");
        for (int i = 0; i < report->low_outlier_count; i++) {
            const CardStats* c = report->low_outliers[i];
            printf("  [LOW]  %-20s  Cost: %d  VPC: %.2f  (avg: %.2f)\n",
                   c->name, c->cost, c->value_per_cost, report->global_avg_vpc);
            printf("         Faction: %-10s  Base: %.1f  Ally: %.1f  Scrap: %.1f\n",
                   c->faction, c->base_value, c->ally_value, c->scrap_value);
        }
        printf("\n");
    }

    if (report->high_outlier_count == 0 && report->low_outlier_count == 0) {
        printf("=== No Statistical Outliers Found ===\n");
        printf("  All cards are within 2 standard deviations of the mean VPC.\n\n");
    }
}
/* }}} */

/* {{{ balance_print_all_cards */
void balance_print_all_cards(const BalanceReport* report) {
    if (!report) return;

    /* Create sorted copy */
    CardStats sorted[MAX_CARDS];
    memcpy(sorted, report->cards, report->card_count * sizeof(CardStats));
    qsort(sorted, report->card_count, sizeof(CardStats), compare_cards_by_vpc);

    printf("=== All Cards (Sorted by Value-Per-Cost) ===\n\n");
    printf("  %-24s  %-10s  %4s  %5s  %5s  %5s  %5s\n",
           "Name", "Faction", "Cost", "Base", "Ally", "Scrp", "VPC");
    printf("  %-24s  %-10s  %4s  %5s  %5s  %5s  %5s\n",
           "------------------------", "----------", "----", "-----", "-----", "-----", "-----");

    for (int i = 0; i < report->card_count; i++) {
        const CardStats* c = &sorted[i];
        printf("  %-24s  %-10s  %4d  %5.1f  %5.1f  %5.1f  %5.2f\n",
               c->name, c->faction, c->cost,
               c->base_value, c->ally_value, c->scrap_value,
               c->value_per_cost);
    }
    printf("\n");
}
/* }}} */

/* {{{ main
 * Main entry point for balance validator tool.
 */
int main(int argc, char** argv) {
    const char* cards_dir = "assets/cards";
    int verbose = 0;

    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [options] [cards_dir]\n", argv[0]);
            printf("\nOptions:\n");
            printf("  -v, --verbose   Show all cards sorted by value-per-cost\n");
            printf("  -h, --help      Show this help message\n");
            printf("\nDefault cards_dir: assets/cards\n");
            return 0;
        } else {
            cards_dir = argv[i];
        }
    }

    BalanceReport report;

    printf("Loading cards from: %s\n", cards_dir);
    int loaded = balance_load_cards(&report, cards_dir);

    if (loaded < 0) {
        fprintf(stderr, "Error: Failed to load cards from %s\n", cards_dir);
        return 1;
    }

    printf("Loaded %d cards.\n", loaded);

    balance_analyze(&report);
    balance_print_report(&report);

    if (verbose) {
        balance_print_all_cards(&report);
    }

    /* Generate recommendations */
    printf("=== Recommendations ===\n\n");

    if (report.high_outlier_count > 0) {
        printf("  Overpowered cards:\n");
        for (int i = 0; i < report.high_outlier_count; i++) {
            const CardStats* c = report.high_outliers[i];
            float excess = (c->value_per_cost / report.global_avg_vpc - 1.0f) * 100.0f;
            printf("  - Consider reducing %s effects by ~%.0f%% or increasing cost\n",
                   c->name, excess);
        }
        printf("\n");
    }

    if (report.low_outlier_count > 0) {
        printf("  Underpowered cards:\n");
        for (int i = 0; i < report.low_outlier_count; i++) {
            const CardStats* c = report.low_outliers[i];
            float deficit = (1.0f - c->value_per_cost / report.global_avg_vpc) * 100.0f;
            printf("  - Consider buffing %s effects by ~%.0f%% or reducing cost\n",
                   c->name, deficit);
        }
        printf("\n");
    }

    if (report.high_outlier_count == 0 && report.low_outlier_count == 0) {
        printf("  No major balance issues detected.\n");
        printf("  Card values are well-distributed around the mean.\n\n");
    }

    return 0;
}
/* }}} */
