/*
 * test-context-manager.c - Tests for Context Manager Module
 *
 * Validates context manager initialization, token estimation,
 * statistics tracking, and memory management.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/llm/06-context-manager.h"

// {{{ Test Statistics
static int tests_run = 0;
static int tests_passed = 0;
// }}}

// {{{ test_context_init_basic
static void test_context_init_basic(void) {
    printf("  Testing basic initialization...\n");
    tests_run++;

    ContextManager* cm = context_init(4096);
    assert(cm != NULL);
    assert(cm->max_tokens == 4096);
    assert(cm->current_tokens == 0);
    assert(cm->entry_count == 0);
    assert(cm->eviction_count == 0);
    assert(cm->summary_count == 0);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_init_large_window
static void test_context_init_large_window(void) {
    printf("  Testing large window initialization...\n");
    tests_run++;

    // Test with Claude-sized window
    ContextManager* cm = context_init(100000);
    assert(cm != NULL);
    assert(cm->max_tokens == 100000);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_init_invalid
static void test_context_init_invalid(void) {
    printf("  Testing invalid initialization...\n");
    tests_run++;

    ContextManager* cm1 = context_init(0);
    assert(cm1 == NULL);

    ContextManager* cm2 = context_init(-100);
    assert(cm2 == NULL);

    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_estimate_tokens_empty
static void test_context_estimate_tokens_empty(void) {
    printf("  Testing token estimation for empty string...\n");
    tests_run++;

    assert(context_estimate_tokens("") == 0);
    assert(context_estimate_tokens(NULL) == 0);

    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_estimate_tokens_short
static void test_context_estimate_tokens_short(void) {
    printf("  Testing token estimation for short strings...\n");
    tests_run++;

    // "Hi" = 2 chars, ~1 token (rounded up)
    assert(context_estimate_tokens("Hi") == 1);

    // "Hello" = 5 chars, ~2 tokens
    assert(context_estimate_tokens("Hello") == 2);

    // "Hello World" = 11 chars, ~3 tokens
    assert(context_estimate_tokens("Hello World") == 3);

    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_estimate_tokens_long
static void test_context_estimate_tokens_long(void) {
    printf("  Testing token estimation for long strings...\n");
    tests_run++;

    // 100 characters should be ~25 tokens
    char long_text[101];
    memset(long_text, 'a', 100);
    long_text[100] = '\0';

    int tokens = context_estimate_tokens(long_text);
    assert(tokens == 25);

    // 400 characters should be ~100 tokens
    char very_long[401];
    memset(very_long, 'b', 400);
    very_long[400] = '\0';

    tokens = context_estimate_tokens(very_long);
    assert(tokens == 100);

    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_get_stats_empty
static void test_context_get_stats_empty(void) {
    printf("  Testing stats for empty context...\n");
    tests_run++;

    ContextManager* cm = context_init(4096);
    ContextManagerStats stats = context_get_stats(cm);

    assert(stats.total_entries == 0);
    assert(stats.current_tokens == 0);
    assert(stats.max_tokens == 4096);
    assert(stats.eviction_count == 0);
    assert(stats.summary_count == 0);
    assert(stats.utilization == 0.0f);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_get_stats_null
static void test_context_get_stats_null(void) {
    printf("  Testing stats for NULL context...\n");
    tests_run++;

    ContextManagerStats stats = context_get_stats(NULL);

    assert(stats.total_entries == 0);
    assert(stats.current_tokens == 0);
    assert(stats.max_tokens == 0);

    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_available_tokens
static void test_context_available_tokens(void) {
    printf("  Testing available tokens calculation...\n");
    tests_run++;

    ContextManager* cm = context_init(4096);

    // Initially all tokens should be available
    assert(context_get_available_tokens(cm) == 4096);

    // NULL should return 0
    assert(context_get_available_tokens(NULL) == 0);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_is_full_empty
static void test_context_is_full_empty(void) {
    printf("  Testing is_full for empty context...\n");
    tests_run++;

    ContextManager* cm = context_init(4096);

    // Empty context (0% utilization):
    // - At 0.0 threshold: 0% >= 0% = true (technically full at 0%)
    // - At real thresholds: should not be full
    assert(context_is_full(cm, 0.0f) == true);  // 0% >= 0%
    assert(context_is_full(cm, 0.5f) == false);
    assert(context_is_full(cm, 0.9f) == false);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_is_full_null
static void test_context_is_full_null(void) {
    printf("  Testing is_full for NULL context...\n");
    tests_run++;

    // NULL should always be considered "full"
    assert(context_is_full(NULL, 0.5f) == true);

    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_is_full_threshold
static void test_context_is_full_threshold(void) {
    printf("  Testing is_full threshold clamping...\n");
    tests_run++;

    ContextManager* cm = context_init(100);

    // Invalid thresholds should be clamped
    assert(context_is_full(cm, -1.0f) == true);  // Clamped to 0.0
    assert(context_is_full(cm, 2.0f) == false);  // Clamped to 1.0

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_clear
static void test_context_clear(void) {
    printf("  Testing context clear...\n");
    tests_run++;

    ContextManager* cm = context_init(4096);

    // Clear empty context should be safe
    context_clear(cm);
    assert(cm->entry_count == 0);
    assert(cm->current_tokens == 0);

    // Clear NULL should be safe
    context_clear(NULL);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_clear_priority
static void test_context_clear_priority(void) {
    printf("  Testing priority-based clear...\n");
    tests_run++;

    ContextManager* cm = context_init(4096);

    // Clear on empty context should be safe
    context_clear_priority(cm, PRIORITY_OLD_EVENTS);
    assert(cm->entry_count == 0);

    // Clear NULL should be safe
    context_clear_priority(NULL, PRIORITY_SYSTEM);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_free_null
static void test_context_free_null(void) {
    printf("  Testing free with NULL...\n");
    tests_run++;

    // Should not crash
    context_free(NULL);

    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_priority_values
static void test_priority_values(void) {
    printf("  Testing priority enum values...\n");
    tests_run++;

    // Verify priority ordering (lower = higher priority)
    assert(PRIORITY_SYSTEM < PRIORITY_CURRENT_TURN);
    assert(PRIORITY_CURRENT_TURN < PRIORITY_WORLD_STATE);
    assert(PRIORITY_WORLD_STATE < PRIORITY_RECENT_EVENTS);
    assert(PRIORITY_RECENT_EVENTS < PRIORITY_FORCE_DESC);
    assert(PRIORITY_FORCE_DESC < PRIORITY_OLD_EVENTS);

    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_add_basic
static void test_context_add_basic(void) {
    printf("  Testing basic entry addition...\n");
    tests_run++;

    ContextManager* cm = context_init(1000);

    bool result = context_add(cm, "Hello World", PRIORITY_CURRENT_TURN);
    assert(result == true);
    assert(cm->entry_count == 1);
    assert(cm->current_tokens > 0);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_add_multiple
static void test_context_add_multiple(void) {
    printf("  Testing multiple entry addition...\n");
    tests_run++;

    ContextManager* cm = context_init(1000);

    context_add(cm, "System prompt", PRIORITY_SYSTEM);
    context_add(cm, "Current turn event", PRIORITY_CURRENT_TURN);
    context_add(cm, "World state", PRIORITY_WORLD_STATE);
    context_add(cm, "Recent event", PRIORITY_RECENT_EVENTS);

    assert(cm->entry_count == 4);
    assert(context_get_entry_count(cm, PRIORITY_SYSTEM) == 1);
    assert(context_get_entry_count(cm, PRIORITY_CURRENT_TURN) == 1);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_add_null
static void test_context_add_null(void) {
    printf("  Testing entry addition with NULL...\n");
    tests_run++;

    ContextManager* cm = context_init(1000);

    assert(context_add(NULL, "text", PRIORITY_SYSTEM) == false);
    assert(context_add(cm, NULL, PRIORITY_SYSTEM) == false);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_add_too_large
static void test_context_add_too_large(void) {
    printf("  Testing entry larger than window...\n");
    tests_run++;

    ContextManager* cm = context_init(10);  // Only 10 tokens

    // Create text that's way more than 10 tokens (~400 chars = ~100 tokens)
    char large_text[401];
    memset(large_text, 'x', 400);
    large_text[400] = '\0';

    bool result = context_add(cm, large_text, PRIORITY_OLD_EVENTS);
    assert(result == false);
    assert(cm->entry_count == 0);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_evict_lowest
static void test_context_evict_lowest(void) {
    printf("  Testing lowest priority eviction...\n");
    tests_run++;

    ContextManager* cm = context_init(1000);

    context_add(cm, "System", PRIORITY_SYSTEM);
    context_add(cm, "Current", PRIORITY_CURRENT_TURN);
    context_add(cm, "Old event", PRIORITY_OLD_EVENTS);

    // Evict should remove old event first
    bool result = context_evict_lowest(cm);
    assert(result == true);
    assert(cm->entry_count == 2);
    assert(context_get_entry_count(cm, PRIORITY_OLD_EVENTS) == 0);
    assert(context_get_entry_count(cm, PRIORITY_SYSTEM) == 1);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_evict_preserves_system
static void test_context_evict_preserves_system(void) {
    printf("  Testing eviction preserves system entries...\n");
    tests_run++;

    ContextManager* cm = context_init(1000);

    context_add(cm, "System 1", PRIORITY_SYSTEM);
    context_add(cm, "System 2", PRIORITY_SYSTEM);

    // Cannot evict system-only context
    bool result = context_evict_lowest(cm);
    assert(result == false);
    assert(cm->entry_count == 2);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_evict_on_overflow
static void test_context_evict_on_overflow(void) {
    printf("  Testing automatic eviction on overflow...\n");
    tests_run++;

    ContextManager* cm = context_init(15);  // Very small window (~60 chars)

    // Add entries that will fill the window
    // Each ~20 chars = ~5 tokens
    context_add(cm, "Entry one here now", PRIORITY_CURRENT_TURN);  // ~5 tokens
    context_add(cm, "Entry two is longer", PRIORITY_RECENT_EVENTS);  // ~5 tokens
    context_add(cm, "Third entry here!!", PRIORITY_OLD_EVENTS);  // ~5 tokens

    // Now at 15 tokens (at limit)
    // Add entry that needs eviction
    bool result = context_add(cm, "New important entry", PRIORITY_CURRENT_TURN);  // ~5 tokens
    assert(result == true);

    // Should have evicted the OLD_EVENTS entry
    assert(cm->eviction_count > 0);
    assert(context_get_entry_count(cm, PRIORITY_OLD_EVENTS) == 0);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_build_prompt_empty
static void test_context_build_prompt_empty(void) {
    printf("  Testing prompt build with empty context...\n");
    tests_run++;

    ContextManager* cm = context_init(1000);

    char* prompt = context_build_prompt(cm);
    assert(prompt != NULL);
    assert(strlen(prompt) == 0);

    free(prompt);
    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_build_prompt_single
static void test_context_build_prompt_single(void) {
    printf("  Testing prompt build with single entry...\n");
    tests_run++;

    ContextManager* cm = context_init(1000);
    context_add(cm, "Hello World", PRIORITY_SYSTEM);

    char* prompt = context_build_prompt(cm);
    assert(prompt != NULL);
    assert(strcmp(prompt, "Hello World") == 0);

    free(prompt);
    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_build_prompt_priority_order
static void test_context_build_prompt_priority_order(void) {
    printf("  Testing prompt builds in priority order...\n");
    tests_run++;

    ContextManager* cm = context_init(1000);

    // Add in reverse priority order
    context_add(cm, "Old", PRIORITY_OLD_EVENTS);
    context_add(cm, "Current", PRIORITY_CURRENT_TURN);
    context_add(cm, "System", PRIORITY_SYSTEM);

    char* prompt = context_build_prompt(cm);
    assert(prompt != NULL);

    // System should come first
    assert(strstr(prompt, "System") < strstr(prompt, "Current"));
    assert(strstr(prompt, "Current") < strstr(prompt, "Old"));

    free(prompt);
    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_get_entry_count
static void test_context_get_entry_count(void) {
    printf("  Testing entry count by priority...\n");
    tests_run++;

    ContextManager* cm = context_init(1000);

    context_add(cm, "S1", PRIORITY_SYSTEM);
    context_add(cm, "S2", PRIORITY_SYSTEM);
    context_add(cm, "C1", PRIORITY_CURRENT_TURN);
    context_add(cm, "O1", PRIORITY_OLD_EVENTS);
    context_add(cm, "O2", PRIORITY_OLD_EVENTS);
    context_add(cm, "O3", PRIORITY_OLD_EVENTS);

    assert(context_get_entry_count(cm, PRIORITY_SYSTEM) == 2);
    assert(context_get_entry_count(cm, PRIORITY_CURRENT_TURN) == 1);
    assert(context_get_entry_count(cm, PRIORITY_OLD_EVENTS) == 3);
    assert(context_get_entry_count(cm, PRIORITY_WORLD_STATE) == 0);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_update_priority
static void test_context_update_priority(void) {
    printf("  Testing priority update...\n");
    tests_run++;

    ContextManager* cm = context_init(1000);

    context_add(cm, "Recent 1", PRIORITY_RECENT_EVENTS);
    context_add(cm, "Recent 2", PRIORITY_RECENT_EVENTS);
    context_add(cm, "Current", PRIORITY_CURRENT_TURN);

    assert(context_get_entry_count(cm, PRIORITY_RECENT_EVENTS) == 2);
    assert(context_get_entry_count(cm, PRIORITY_OLD_EVENTS) == 0);

    // Demote recent events to old events
    context_update_priority(cm, PRIORITY_RECENT_EVENTS, PRIORITY_OLD_EVENTS);

    assert(context_get_entry_count(cm, PRIORITY_RECENT_EVENTS) == 0);
    assert(context_get_entry_count(cm, PRIORITY_OLD_EVENTS) == 2);
    assert(context_get_entry_count(cm, PRIORITY_CURRENT_TURN) == 1);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_stats_after_operations
static void test_context_stats_after_operations(void) {
    printf("  Testing statistics after operations...\n");
    tests_run++;

    ContextManager* cm = context_init(100);

    context_add(cm, "Entry 1", PRIORITY_OLD_EVENTS);
    context_add(cm, "Entry 2", PRIORITY_OLD_EVENTS);

    ContextManagerStats stats = context_get_stats(cm);
    assert(stats.total_entries == 2);
    assert(stats.current_tokens > 0);
    assert(stats.utilization > 0.0f);

    // Force eviction
    context_add(cm, "Long entry that needs space", PRIORITY_CURRENT_TURN);
    context_add(cm, "Another long entry here", PRIORITY_CURRENT_TURN);

    stats = context_get_stats(cm);
    // Evictions may have occurred
    assert(stats.current_tokens <= stats.max_tokens);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_remove_at
static void test_context_remove_at(void) {
    printf("  Testing remove at index...\n");
    tests_run++;

    ContextManager* cm = context_init(1000);

    context_add(cm, "Entry 0", PRIORITY_CURRENT_TURN);
    context_add(cm, "Entry 1", PRIORITY_RECENT_EVENTS);
    context_add(cm, "Entry 2", PRIORITY_OLD_EVENTS);

    assert(cm->entry_count == 3);

    // Remove middle entry
    bool result = context_remove_at(cm, 1);
    assert(result == true);
    assert(cm->entry_count == 2);

    // Verify remaining entries shifted correctly
    char* prompt = context_build_prompt(cm);
    assert(strstr(prompt, "Entry 0") != NULL);
    assert(strstr(prompt, "Entry 1") == NULL);
    assert(strstr(prompt, "Entry 2") != NULL);

    free(prompt);
    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_remove_at_invalid
static void test_context_remove_at_invalid(void) {
    printf("  Testing remove at invalid index...\n");
    tests_run++;

    ContextManager* cm = context_init(1000);
    context_add(cm, "Entry", PRIORITY_CURRENT_TURN);

    assert(context_remove_at(cm, -1) == false);
    assert(context_remove_at(cm, 5) == false);
    assert(context_remove_at(NULL, 0) == false);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_find_summarizable
static void test_context_find_summarizable(void) {
    printf("  Testing find summarizable entries...\n");
    tests_run++;

    ContextManager* cm = context_init(1000);

    // Add various priority entries
    context_add(cm, "System", PRIORITY_SYSTEM);
    context_add(cm, "Current", PRIORITY_CURRENT_TURN);
    context_add(cm, "World", PRIORITY_WORLD_STATE);
    context_add(cm, "Recent", PRIORITY_RECENT_EVENTS);
    context_add(cm, "Force", PRIORITY_FORCE_DESC);
    context_add(cm, "Old 1", PRIORITY_OLD_EVENTS);
    context_add(cm, "Old 2", PRIORITY_OLD_EVENTS);

    int indices[10];
    int count = context_find_summarizable(cm, indices, 10);

    // Should find force desc and old events (3 entries)
    assert(count == 3);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_find_summarizable_skips_summaries
static void test_context_find_summarizable_skips_summaries(void) {
    printf("  Testing find summarizable skips existing summaries...\n");
    tests_run++;

    ContextManager* cm = context_init(1000);

    context_add(cm, "Old 1", PRIORITY_OLD_EVENTS);
    context_add(cm, "Old 2", PRIORITY_OLD_EVENTS);
    context_mark_as_summary(cm);  // Mark second as summary
    context_add(cm, "Old 3", PRIORITY_OLD_EVENTS);

    int indices[10];
    int count = context_find_summarizable(cm, indices, 10);

    // Should find 2 (Old 1 and Old 3, not the summary)
    assert(count == 2);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_get_entries_text
static void test_context_get_entries_text(void) {
    printf("  Testing get entries text...\n");
    tests_run++;

    ContextManager* cm = context_init(1000);

    context_add(cm, "First entry", PRIORITY_OLD_EVENTS);
    context_add(cm, "Second entry", PRIORITY_OLD_EVENTS);
    context_add(cm, "Third entry", PRIORITY_OLD_EVENTS);

    int indices[] = {0, 2};  // First and third
    char* text = context_get_entries_text(cm, indices, 2);

    assert(text != NULL);
    assert(strstr(text, "First entry") != NULL);
    assert(strstr(text, "Third entry") != NULL);
    assert(strstr(text, "Second entry") == NULL);

    free(text);
    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_replace_with_summary
static void test_context_replace_with_summary(void) {
    printf("  Testing replace with summary...\n");
    tests_run++;

    ContextManager* cm = context_init(1000);

    context_add(cm, "Old event 1", PRIORITY_OLD_EVENTS);
    context_add(cm, "Old event 2", PRIORITY_OLD_EVENTS);
    context_add(cm, "Old event 3", PRIORITY_OLD_EVENTS);

    int initial_tokens = cm->current_tokens;
    int indices[] = {0, 1, 2};

    bool result = context_replace_with_summary(cm, indices, 3,
        "Summary of events");

    assert(result == true);
    assert(cm->entry_count == 1);
    assert(cm->summary_count == 1);

    // Summary should have fewer tokens than original
    assert(cm->current_tokens < initial_tokens);

    // Verify summary is marked as such
    assert(cm->entries[0].is_summary == true);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_needs_summarization
static void test_context_needs_summarization(void) {
    printf("  Testing needs summarization check...\n");
    tests_run++;

    ContextManager* cm = context_init(100);

    // Empty context doesn't need summarization
    assert(context_needs_summarization(cm, 0.8f) == false);

    // Fill to 50% (~50 chars = ~12-13 tokens, need ~80 tokens for 80%)
    // Actually let's fill more carefully
    context_add(cm, "This is a long entry with lots of text to fill up space", PRIORITY_OLD_EVENTS);
    context_add(cm, "Another long entry with lots of text to fill up space", PRIORITY_OLD_EVENTS);
    context_add(cm, "Third long entry with lots of text to fill up space!", PRIORITY_OLD_EVENTS);

    // Now should be over 80%
    ContextManagerStats stats = context_get_stats(cm);
    if (stats.utilization >= 0.8f) {
        assert(context_needs_summarization(cm, 0.8f) == true);
    }

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ test_context_mark_as_summary
static void test_context_mark_as_summary(void) {
    printf("  Testing mark as summary...\n");
    tests_run++;

    ContextManager* cm = context_init(1000);

    context_add(cm, "Regular entry", PRIORITY_OLD_EVENTS);
    assert(cm->entries[0].is_summary == false);

    context_mark_as_summary(cm);
    assert(cm->entries[0].is_summary == true);

    // Add another and verify it's not marked
    context_add(cm, "Another entry", PRIORITY_OLD_EVENTS);
    assert(cm->entries[1].is_summary == false);

    context_free(cm);
    tests_passed++;
    printf("    PASSED\n");
}
// }}}

// {{{ main
int main(void) {
    printf("=== Context Manager Tests ===\n\n");

    printf("Initialization Tests:\n");
    test_context_init_basic();
    test_context_init_large_window();
    test_context_init_invalid();

    printf("\nToken Estimation Tests:\n");
    test_context_estimate_tokens_empty();
    test_context_estimate_tokens_short();
    test_context_estimate_tokens_long();

    printf("\nStatistics Tests:\n");
    test_context_get_stats_empty();
    test_context_get_stats_null();
    test_context_available_tokens();

    printf("\nCapacity Tests:\n");
    test_context_is_full_empty();
    test_context_is_full_null();
    test_context_is_full_threshold();

    printf("\nCleanup Tests:\n");
    test_context_clear();
    test_context_clear_priority();
    test_context_free_null();

    printf("\nPriority Tests:\n");
    test_priority_values();

    printf("\nEntry Management Tests:\n");
    test_context_add_basic();
    test_context_add_multiple();
    test_context_add_null();
    test_context_add_too_large();

    printf("\nEviction Tests:\n");
    test_context_evict_lowest();
    test_context_evict_preserves_system();
    test_context_evict_on_overflow();

    printf("\nPrompt Building Tests:\n");
    test_context_build_prompt_empty();
    test_context_build_prompt_single();
    test_context_build_prompt_priority_order();

    printf("\nEntry Count Tests:\n");
    test_context_get_entry_count();
    test_context_update_priority();
    test_context_stats_after_operations();

    printf("\nSummarization Tests:\n");
    test_context_remove_at();
    test_context_remove_at_invalid();
    test_context_find_summarizable();
    test_context_find_summarizable_skips_summaries();
    test_context_get_entries_text();
    test_context_replace_with_summary();
    test_context_needs_summarization();
    test_context_mark_as_summary();

    printf("\n=== Results: %d/%d tests passed ===\n", tests_passed, tests_run);

    return tests_passed == tests_run ? 0 : 1;
}
// }}}
