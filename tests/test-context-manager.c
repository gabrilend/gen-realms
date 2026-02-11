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

    printf("\n=== Results: %d/%d tests passed ===\n", tests_passed, tests_run);

    return tests_passed == tests_run ? 0 : 1;
}
// }}}
