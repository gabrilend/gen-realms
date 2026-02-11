/*
 * test-narrative-cache.c - Tests for Narrative Caching Module
 *
 * Validates cache initialization, get/set operations, TTL expiration,
 * LRU eviction, and event signature generation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>  // For sleep()
#include "../src/llm/07-narrative-cache.h"

// {{{ Test Statistics
static int tests_run = 0;
static int tests_passed = 0;
// }}}

// {{{ Mock Data
static Player mock_player1;
static Player mock_player2;
static CardType mock_card_type;
static CardInstance mock_card;
static CardType mock_base_type;
static CardInstance mock_base;

static void setup_mocks(void) {
    mock_player1.name = "Commander Vex";
    mock_player1.authority = 35;

    mock_player2.name = "Admiral Thorne";
    mock_player2.authority = 28;

    mock_card_type.name = "Battle Cruiser";
    mock_card_type.faction = FACTION_KINGDOM;
    mock_card_type.cost = 5;

    mock_card.type = &mock_card_type;

    mock_base_type.name = "Orbital Station";
    mock_base_type.faction = FACTION_ARTIFICER;
    mock_base_type.defense = 5;

    mock_base.type = &mock_base_type;
}
// }}}

// {{{ test_cache_init_basic
static void test_cache_init_basic(void) {
    printf("  Testing basic initialization...\n");
    tests_run++;

    NarrativeCache* cache = narrative_cache_init(100, 3600);
    assert(cache != NULL);
    assert(cache->max_entries == 100);
    assert(cache->ttl_seconds == 3600);
    assert(cache->count == 0);
    assert(cache->hits == 0);
    assert(cache->misses == 0);

    narrative_cache_free(cache);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_cache_init_no_ttl
static void test_cache_init_no_ttl(void) {
    printf("  Testing initialization with no TTL...\n");
    tests_run++;

    NarrativeCache* cache = narrative_cache_init(50, 0);
    assert(cache != NULL);
    assert(cache->ttl_seconds == 0);

    narrative_cache_free(cache);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_cache_init_invalid
static void test_cache_init_invalid(void) {
    printf("  Testing invalid initialization...\n");
    tests_run++;

    NarrativeCache* cache = narrative_cache_init(0, 100);
    assert(cache == NULL);

    cache = narrative_cache_init(-5, 100);
    assert(cache == NULL);

    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_cache_set_get_basic
static void test_cache_set_get_basic(void) {
    printf("  Testing basic set and get...\n");
    tests_run++;

    NarrativeCache* cache = narrative_cache_init(10, 0);

    bool result = narrative_cache_set(cache, "test:sig:1", "The battle begins!");
    assert(result == true);
    assert(cache->count == 1);

    const char* narrative = narrative_cache_get(cache, "test:sig:1");
    assert(narrative != NULL);
    assert(strcmp(narrative, "The battle begins!") == 0);

    narrative_cache_free(cache);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_cache_get_miss
static void test_cache_get_miss(void) {
    printf("  Testing cache miss...\n");
    tests_run++;

    NarrativeCache* cache = narrative_cache_init(10, 0);

    const char* narrative = narrative_cache_get(cache, "nonexistent");
    assert(narrative == NULL);
    assert(cache->misses == 1);

    narrative_cache_free(cache);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_cache_hit_count
static void test_cache_hit_count(void) {
    printf("  Testing hit counting...\n");
    tests_run++;

    NarrativeCache* cache = narrative_cache_init(10, 0);

    narrative_cache_set(cache, "sig:1", "Narrative one");

    // Multiple gets should increment hits
    narrative_cache_get(cache, "sig:1");
    narrative_cache_get(cache, "sig:1");
    narrative_cache_get(cache, "sig:1");

    assert(cache->hits == 3);
    assert(cache->entries[0].use_count == 3);

    narrative_cache_free(cache);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_cache_update_existing
static void test_cache_update_existing(void) {
    printf("  Testing update of existing entry...\n");
    tests_run++;

    NarrativeCache* cache = narrative_cache_init(10, 0);

    narrative_cache_set(cache, "sig:1", "Original narrative");
    narrative_cache_set(cache, "sig:1", "Updated narrative");

    assert(cache->count == 1);  // Still only one entry

    const char* narrative = narrative_cache_get(cache, "sig:1");
    assert(strcmp(narrative, "Updated narrative") == 0);

    narrative_cache_free(cache);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_cache_multiple_entries
static void test_cache_multiple_entries(void) {
    printf("  Testing multiple entries...\n");
    tests_run++;

    NarrativeCache* cache = narrative_cache_init(10, 0);

    narrative_cache_set(cache, "sig:1", "Narrative 1");
    narrative_cache_set(cache, "sig:2", "Narrative 2");
    narrative_cache_set(cache, "sig:3", "Narrative 3");

    assert(cache->count == 3);

    assert(strcmp(narrative_cache_get(cache, "sig:1"), "Narrative 1") == 0);
    assert(strcmp(narrative_cache_get(cache, "sig:2"), "Narrative 2") == 0);
    assert(strcmp(narrative_cache_get(cache, "sig:3"), "Narrative 3") == 0);

    narrative_cache_free(cache);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_cache_lru_eviction
static void test_cache_lru_eviction(void) {
    printf("  Testing LRU eviction...\n");
    tests_run++;

    NarrativeCache* cache = narrative_cache_init(3, 0);  // Small cache

    // Fill cache with small delays to ensure different timestamps
    narrative_cache_set(cache, "sig:1", "Narrative 1");
    sleep(1);
    narrative_cache_set(cache, "sig:2", "Narrative 2");
    sleep(1);
    narrative_cache_set(cache, "sig:3", "Narrative 3");

    // sig:1 was added first, so it should be LRU
    // Add new entry - should evict sig:1 (oldest)
    narrative_cache_set(cache, "sig:4", "Narrative 4");

    assert(cache->count == 3);
    assert(cache->evictions == 1);

    // sig:1 should be gone (it was oldest/LRU)
    assert(narrative_cache_get(cache, "sig:1") == NULL);
    // Others should still exist
    assert(narrative_cache_get(cache, "sig:2") != NULL);
    assert(narrative_cache_get(cache, "sig:3") != NULL);
    assert(narrative_cache_get(cache, "sig:4") != NULL);

    narrative_cache_free(cache);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_cache_remove
static void test_cache_remove(void) {
    printf("  Testing entry removal...\n");
    tests_run++;

    NarrativeCache* cache = narrative_cache_init(10, 0);

    narrative_cache_set(cache, "sig:1", "Narrative 1");
    narrative_cache_set(cache, "sig:2", "Narrative 2");

    bool result = narrative_cache_remove(cache, "sig:1");
    assert(result == true);
    assert(cache->count == 1);
    assert(narrative_cache_get(cache, "sig:1") == NULL);
    assert(narrative_cache_get(cache, "sig:2") != NULL);

    // Remove nonexistent
    result = narrative_cache_remove(cache, "nonexistent");
    assert(result == false);

    narrative_cache_free(cache);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_cache_clear
static void test_cache_clear(void) {
    printf("  Testing cache clear...\n");
    tests_run++;

    NarrativeCache* cache = narrative_cache_init(10, 0);

    narrative_cache_set(cache, "sig:1", "Narrative 1");
    narrative_cache_set(cache, "sig:2", "Narrative 2");
    narrative_cache_set(cache, "sig:3", "Narrative 3");

    narrative_cache_clear(cache);

    assert(cache->count == 0);
    assert(narrative_cache_get(cache, "sig:1") == NULL);

    narrative_cache_free(cache);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_cache_stats
static void test_cache_stats(void) {
    printf("  Testing statistics...\n");
    tests_run++;

    NarrativeCache* cache = narrative_cache_init(10, 0);

    narrative_cache_set(cache, "sig:1", "Narrative 1");
    narrative_cache_get(cache, "sig:1");  // Hit
    narrative_cache_get(cache, "sig:1");  // Hit
    narrative_cache_get(cache, "sig:2");  // Miss

    NarrativeCacheStats stats = narrative_cache_get_stats(cache);

    assert(stats.total_entries == 1);
    assert(stats.hits == 2);
    assert(stats.misses == 1);
    assert(stats.hit_rate > 0.6f && stats.hit_rate < 0.7f);  // ~66%

    narrative_cache_free(cache);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_cache_reset_stats
static void test_cache_reset_stats(void) {
    printf("  Testing stats reset...\n");
    tests_run++;

    NarrativeCache* cache = narrative_cache_init(10, 0);

    narrative_cache_set(cache, "sig:1", "Narrative 1");
    narrative_cache_get(cache, "sig:1");
    narrative_cache_get(cache, "nonexistent");

    narrative_cache_reset_stats(cache);

    NarrativeCacheStats stats = narrative_cache_get_stats(cache);
    assert(stats.hits == 0);
    assert(stats.misses == 0);
    assert(stats.total_entries == 1);  // Entries not affected

    narrative_cache_free(cache);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_cache_ttl_expiration
static void test_cache_ttl_expiration(void) {
    printf("  Testing TTL expiration...\n");
    tests_run++;

    // Use 1 second TTL for fast test
    NarrativeCache* cache = narrative_cache_init(10, 1);

    narrative_cache_set(cache, "sig:1", "Narrative 1");

    // Should be available immediately
    assert(narrative_cache_get(cache, "sig:1") != NULL);

    // Wait for expiration
    sleep(2);

    // Should be expired now
    const char* result = narrative_cache_get(cache, "sig:1");
    assert(result == NULL);
    assert(cache->expirations == 1);

    narrative_cache_free(cache);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_cache_cleanup_expired
static void test_cache_cleanup_expired(void) {
    printf("  Testing cleanup of expired entries...\n");
    tests_run++;

    NarrativeCache* cache = narrative_cache_init(10, 1);

    narrative_cache_set(cache, "sig:1", "Narrative 1");
    narrative_cache_set(cache, "sig:2", "Narrative 2");

    sleep(2);  // Let them expire

    // Add a fresh entry
    narrative_cache_set(cache, "sig:3", "Narrative 3");

    int removed = narrative_cache_cleanup_expired(cache);
    assert(removed == 2);
    assert(cache->count == 1);
    assert(narrative_cache_get(cache, "sig:3") != NULL);

    narrative_cache_free(cache);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_event_signature_card_played
static void test_event_signature_card_played(void) {
    printf("  Testing event signature for card played...\n");
    tests_run++;

    NarrationEvent* event = event_narration_create(GAME_EVENT_CARD_PLAYED);
    event->actor = &mock_player1;
    event->card = &mock_card;

    char* sig = event_build_signature(event);
    assert(sig != NULL);
    assert(strstr(sig, "card played") != NULL);
    assert(strstr(sig, "Battle Cruiser") != NULL);
    assert(strstr(sig, "Commander Vex") != NULL);

    free(sig);
    event_narration_free(event);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_event_signature_attack
static void test_event_signature_attack(void) {
    printf("  Testing event signature for attack...\n");
    tests_run++;

    NarrationEvent* event = event_narration_create(GAME_EVENT_ATTACK_PLAYER);
    event->actor = &mock_player1;
    event->target = &mock_player2;
    event->damage = 7;

    char* sig = event_build_signature(event);
    assert(sig != NULL);
    assert(strstr(sig, "attack on player") != NULL);
    assert(strstr(sig, "Commander Vex") != NULL);
    assert(strstr(sig, "Admiral Thorne") != NULL);
    assert(strstr(sig, "7") != NULL);

    free(sig);
    event_narration_free(event);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_event_signature_game_over
static void test_event_signature_game_over(void) {
    printf("  Testing event signature for game over...\n");
    tests_run++;

    NarrationEvent* event = event_narration_create(GAME_EVENT_GAME_OVER);
    event->actor = &mock_player1;
    event->target = &mock_player2;
    event->damage = 35;  // Final authority

    char* sig = event_build_signature(event);
    assert(sig != NULL);
    assert(strstr(sig, "game over") != NULL);

    free(sig);
    event_narration_free(event);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_event_signature_null
static void test_event_signature_null(void) {
    printf("  Testing event signature with NULL...\n");
    tests_run++;

    char* sig = event_build_signature(NULL);
    assert(sig == NULL);

    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_cache_null_safety
static void test_cache_null_safety(void) {
    printf("  Testing NULL safety...\n");
    tests_run++;

    // All these should not crash
    narrative_cache_free(NULL);
    assert(narrative_cache_get(NULL, "sig") == NULL);
    assert(narrative_cache_set(NULL, "sig", "text") == false);
    assert(narrative_cache_remove(NULL, "sig") == false);
    narrative_cache_clear(NULL);
    narrative_cache_reset_stats(NULL);

    NarrativeCache* cache = narrative_cache_init(10, 0);
    assert(narrative_cache_get(cache, NULL) == NULL);
    assert(narrative_cache_set(cache, NULL, "text") == false);
    assert(narrative_cache_set(cache, "sig", NULL) == false);

    narrative_cache_free(cache);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ main
int main(void) {
    printf("=== Narrative Cache Tests ===\n\n");

    setup_mocks();

    printf("Initialization Tests:\n");
    test_cache_init_basic();
    test_cache_init_no_ttl();
    test_cache_init_invalid();

    printf("\nBasic Operations Tests:\n");
    test_cache_set_get_basic();
    test_cache_get_miss();
    test_cache_hit_count();
    test_cache_update_existing();
    test_cache_multiple_entries();

    printf("\nEviction Tests:\n");
    test_cache_lru_eviction();
    test_cache_remove();
    test_cache_clear();

    printf("\nStatistics Tests:\n");
    test_cache_stats();
    test_cache_reset_stats();

    printf("\nTTL Tests:\n");
    test_cache_ttl_expiration();
    test_cache_cleanup_expired();

    printf("\nEvent Signature Tests:\n");
    test_event_signature_card_played();
    test_event_signature_attack();
    test_event_signature_game_over();
    test_event_signature_null();

    printf("\nSafety Tests:\n");
    test_cache_null_safety();

    printf("\n=== Results: %d/%d tests passed ===\n", tests_passed, tests_run);

    return tests_passed == tests_run ? 0 : 1;
}
// }}}
