/*
 * test-force-desc.c - Tests for Force Description Prompts
 *
 * Validates force descriptions, faction themes, and caching.
 * Run with: gcc -o test-force-desc test-force-desc.c ../src/llm/04-force-description.c
 *           ../src/llm/02-prompts.c -I. && ./test-force-desc
 */

#include "../src/llm/04-force-description.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define TEST(name) static void name(void)
#define RUN_TEST(name) do { printf("Running %s...", #name); name(); printf(" PASS\n"); } while(0)

/* Mock structures for testing */
static Player* create_mock_player(const char* name) {
    Player* p = malloc(sizeof(Player));
    p->id = 1;
    p->name = strdup(name);
    p->authority = 42;
    p->trade = 5;
    p->combat = 3;
    p->deck = NULL;

    for (int i = 0; i < FACTION_COUNT; i++) {
        p->factions_played[i] = false;
    }

    return p;
}

static void free_mock_player(Player* p) {
    if (p == NULL) return;
    free(p->name);
    free(p);
}

static CardType* create_mock_card_type(const char* name, Faction faction) {
    CardType* ct = malloc(sizeof(CardType));
    memset(ct, 0, sizeof(CardType));
    ct->id = strdup("test-card-001");
    ct->name = strdup(name);
    ct->faction = faction;
    ct->kind = CARD_KIND_SHIP;
    ct->cost = 3;
    ct->defense = 0;
    ct->flavor = NULL;
    ct->effects = NULL;
    ct->effect_count = 0;
    return ct;
}

static void free_mock_card_type(CardType* ct) {
    if (ct == NULL) return;
    free(ct->id);
    free(ct->name);
    free(ct);
}

static CardInstance* create_mock_card(CardType* type) {
    CardInstance* ci = malloc(sizeof(CardInstance));
    memset(ci, 0, sizeof(CardInstance));
    ci->instance_id = strdup("inst-001");
    ci->type = type;
    ci->attack_bonus = 0;
    ci->trade_bonus = 0;
    ci->authority_bonus = 0;
    ci->placement = ZONE_NONE;
    return ci;
}

static void free_mock_card(CardInstance* ci) {
    if (ci == NULL) return;
    free(ci->instance_id);
    free(ci);
}

// {{{ test_cache_create
TEST(test_cache_create) {
    ForceDescCache* cache = force_desc_cache_create();
    assert(cache != NULL);
    assert(cache->count == 0);

    force_desc_cache_free(cache);
}
// }}}

// {{{ test_cache_free_null
TEST(test_cache_free_null) {
    force_desc_cache_free(NULL);
}
// }}}

// {{{ test_cache_set_get
TEST(test_cache_set_get) {
    ForceDescCache* cache = force_desc_cache_create();

    force_desc_cache_set(cache, "card-001", "A mighty warrior!", 5);
    assert(cache->count == 1);

    const char* desc = force_desc_cache_get(cache, "card-001");
    assert(desc != NULL);
    assert(strcmp(desc, "A mighty warrior!") == 0);

    force_desc_cache_free(cache);
}
// }}}

// {{{ test_cache_get_missing
TEST(test_cache_get_missing) {
    ForceDescCache* cache = force_desc_cache_create();

    const char* desc = force_desc_cache_get(cache, "nonexistent");
    assert(desc == NULL);

    force_desc_cache_free(cache);
}
// }}}

// {{{ test_cache_update
TEST(test_cache_update) {
    ForceDescCache* cache = force_desc_cache_create();

    force_desc_cache_set(cache, "card-001", "Version 1", 1);
    force_desc_cache_set(cache, "card-001", "Version 2", 2);

    assert(cache->count == 1);  // Should update, not add

    const char* desc = force_desc_cache_get(cache, "card-001");
    assert(strcmp(desc, "Version 2") == 0);

    force_desc_cache_free(cache);
}
// }}}

// {{{ test_cache_circular
TEST(test_cache_circular) {
    ForceDescCache* cache = force_desc_cache_create();

    // Fill beyond capacity
    for (int i = 0; i < FORCE_DESC_CACHE_SIZE + 5; i++) {
        char card_id[32];
        snprintf(card_id, sizeof(card_id), "card-%03d", i);
        force_desc_cache_set(cache, card_id, "Description", i);
    }

    assert(cache->count == FORCE_DESC_CACHE_SIZE);

    force_desc_cache_free(cache);
}
// }}}

// {{{ test_get_theme
TEST(test_get_theme) {
    const FactionTheme* theme = force_desc_get_theme(FACTION_WILDS);
    assert(theme != NULL);
    assert(theme->faction == FACTION_WILDS);
    assert(strstr(theme->name, "Wilds") != NULL);

    theme = force_desc_get_theme(FACTION_MERCHANT);
    assert(theme != NULL);
    assert(strstr(theme->name, "Merchant") != NULL);
}
// }}}

// {{{ test_get_theme_invalid
TEST(test_get_theme_invalid) {
    const FactionTheme* theme = force_desc_get_theme(FACTION_COUNT);
    assert(theme != NULL);  // Should return neutral
    assert(theme->faction == FACTION_NEUTRAL);
}
// }}}

// {{{ test_faction_adjective
TEST(test_faction_adjective) {
    const char* adj = force_desc_get_faction_adjective(FACTION_KINGDOM);
    assert(adj != NULL);
    assert(strlen(adj) > 0);
}
// }}}

// {{{ test_faction_noun
TEST(test_faction_noun) {
    const char* noun = force_desc_get_faction_noun(FACTION_ARTIFICER);
    assert(noun != NULL);
    assert(strlen(noun) > 0);
}
// }}}

// {{{ test_dominant_faction_neutral
TEST(test_dominant_faction_neutral) {
    Player* p = create_mock_player("Test");

    Faction dom = force_desc_get_dominant_faction(p);
    assert(dom == FACTION_NEUTRAL);

    free_mock_player(p);
}
// }}}

// {{{ test_dominant_faction_played
TEST(test_dominant_faction_played) {
    Player* p = create_mock_player("Test");
    p->factions_played[FACTION_WILDS] = true;

    Faction dom = force_desc_get_dominant_faction(p);
    assert(dom == FACTION_WILDS);

    free_mock_player(p);
}
// }}}

// {{{ test_summarize_forces_empty
TEST(test_summarize_forces_empty) {
    Player* p = create_mock_player("Test");

    char* summary = force_desc_summarize_forces(p);
    assert(summary != NULL);
    assert(strstr(summary, "scouts") != NULL);

    free(summary);
    free_mock_player(p);
}
// }}}

// {{{ test_summarize_forces_factions
TEST(test_summarize_forces_factions) {
    Player* p = create_mock_player("Test");
    p->factions_played[FACTION_WILDS] = true;
    p->factions_played[FACTION_KINGDOM] = true;

    char* summary = force_desc_summarize_forces(p);
    assert(summary != NULL);
    assert(strstr(summary, "Wilds") != NULL);
    assert(strstr(summary, "Kingdom") != NULL);

    free(summary);
    free_mock_player(p);
}
// }}}

// {{{ test_build_player_forces
TEST(test_build_player_forces) {
    Player* p = create_mock_player("Lady Morgaine");
    p->factions_played[FACTION_WILDS] = true;

    char* prompt = force_desc_build_player_forces(p, NULL);
    assert(prompt != NULL);
    assert(strstr(prompt, "Morgaine") != NULL);
    assert(strstr(prompt, "Wilds") != NULL);

    free(prompt);
    free_mock_player(p);
}
// }}}

// {{{ test_build_card_played
TEST(test_build_card_played) {
    Player* p = create_mock_player("Lord Theron");
    CardType* ct = create_mock_card_type("Dire Bear", FACTION_WILDS);
    CardInstance* ci = create_mock_card(ct);

    char* prompt = force_desc_build_card_played(ci, p);
    assert(prompt != NULL);
    assert(strstr(prompt, "Theron") != NULL);
    assert(strstr(prompt, "Dire Bear") != NULL);

    free(prompt);
    free_mock_card(ci);
    free_mock_card_type(ct);
    free_mock_player(p);
}
// }}}

// {{{ test_build_base
TEST(test_build_base) {
    CardType* ct = create_mock_card_type("Watchtower", FACTION_KINGDOM);
    ct->kind = CARD_KIND_BASE;
    ct->defense = 5;

    CardInstance* ci = create_mock_card(ct);

    char* prompt = force_desc_build_base(ci, ZONE_FRONTIER);
    assert(prompt != NULL);
    assert(strstr(prompt, "Watchtower") != NULL);
    assert(strstr(prompt, "frontier") != NULL);
    assert(strstr(prompt, "5 defense") != NULL);

    free(prompt);
    free_mock_card(ci);
    free_mock_card_type(ct);
}
// }}}

// {{{ test_build_attack
TEST(test_build_attack) {
    Player* attacker = create_mock_player("Lady Morgaine");
    Player* defender = create_mock_player("Lord Theron");
    defender->authority = 35;

    char* prompt = force_desc_build_attack(attacker, defender, 7);
    assert(prompt != NULL);
    assert(strstr(prompt, "Morgaine") != NULL);
    assert(strstr(prompt, "Theron") != NULL);
    assert(strstr(prompt, "7") != NULL);
    assert(strstr(prompt, "35") != NULL);

    free(prompt);
    free_mock_player(attacker);
    free_mock_player(defender);
}
// }}}

// {{{ main
int main(void) {
    printf("=== Force Description Tests ===\n");

    RUN_TEST(test_cache_create);
    RUN_TEST(test_cache_free_null);
    RUN_TEST(test_cache_set_get);
    RUN_TEST(test_cache_get_missing);
    RUN_TEST(test_cache_update);
    RUN_TEST(test_cache_circular);
    RUN_TEST(test_get_theme);
    RUN_TEST(test_get_theme_invalid);
    RUN_TEST(test_faction_adjective);
    RUN_TEST(test_faction_noun);
    RUN_TEST(test_dominant_faction_neutral);
    RUN_TEST(test_dominant_faction_played);
    RUN_TEST(test_summarize_forces_empty);
    RUN_TEST(test_summarize_forces_factions);
    RUN_TEST(test_build_player_forces);
    RUN_TEST(test_build_card_played);
    RUN_TEST(test_build_base);
    RUN_TEST(test_build_attack);

    printf("\nAll tests passed!\n");
    return 0;
}
// }}}
