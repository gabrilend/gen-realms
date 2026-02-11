/*
 * test-image-cache.c - Unit tests for image caching system
 *
 * Tests cache operations, LRU eviction, frame snapshots, and persistence.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>

#include "../src/visual/03-image-cache.h"

/* Test configuration */
static const char* TEST_CACHE_DIR = "/tmp/symbeline-test-cache";
static int tests_passed = 0;
static int tests_failed = 0;

/* {{{ test utilities */
#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("  Testing %s... ", #name); \
    test_##name(); \
    printf("PASSED\n"); \
    tests_passed++; \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAILED at line %d: %s\n", __LINE__, #cond); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_NE(a, b) ASSERT((a) != (b))
#define ASSERT_NULL(p) ASSERT((p) == NULL)
#define ASSERT_NOT_NULL(p) ASSERT((p) != NULL)
#define ASSERT_STR_EQ(a, b) ASSERT(strcmp((a), (b)) == 0)

/* Create fake image data */
static unsigned char* create_test_image(size_t size, unsigned char fill) {
    unsigned char* data = malloc(size);
    if (data) {
        memset(data, fill, size);
        /* Fake PNG header */
        data[0] = 0x89;
        data[1] = 'P';
        data[2] = 'N';
        data[3] = 'G';
    }
    return data;
}

/* Clean up test directory */
static void cleanup_test_dir(void) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", TEST_CACHE_DIR);
    system(cmd);
}
/* }}} */

/* {{{ test_cache_create */
TEST(cache_create) {
    cleanup_test_dir();

    ImageCache* cache = image_cache_create(TEST_CACHE_DIR, "game001", 100, 0);
    ASSERT_NOT_NULL(cache);

    /* Verify directories created */
    struct stat st;
    ASSERT_EQ(stat(TEST_CACHE_DIR, &st), 0);

    char game_dir[256];
    snprintf(game_dir, sizeof(game_dir), "%s/game001", TEST_CACHE_DIR);
    ASSERT_EQ(stat(game_dir, &st), 0);

    char cache_subdir[256];
    snprintf(cache_subdir, sizeof(cache_subdir), "%s/game001/cache", TEST_CACHE_DIR);
    ASSERT_EQ(stat(cache_subdir, &st), 0);

    image_cache_free(cache);
}
/* }}} */

/* {{{ test_cache_set_get */
TEST(cache_set_get) {
    cleanup_test_dir();

    ImageCache* cache = image_cache_create(TEST_CACHE_DIR, "game002", 100, 0);
    ASSERT_NOT_NULL(cache);

    /* Create test image */
    unsigned char* img = create_test_image(1024, 0xAB);
    ASSERT_NOT_NULL(img);

    /* Cache it */
    bool set_ok = image_cache_set(cache, "hash123", img, 1024, 64, 64, "test prompt");
    ASSERT(set_ok);

    /* Retrieve it */
    size_t out_size;
    const unsigned char* retrieved = image_cache_get(cache, "hash123", &out_size);
    ASSERT_NOT_NULL(retrieved);
    ASSERT_EQ(out_size, 1024);
    ASSERT_EQ(memcmp(retrieved, img, 1024), 0);

    /* Cache stats */
    ImageCacheStats stats = image_cache_get_stats(cache);
    ASSERT_EQ(stats.total_entries, 1);
    ASSERT_EQ(stats.cache_hits, 1);
    ASSERT_EQ(stats.cache_misses, 0);

    free(img);
    image_cache_free(cache);
}
/* }}} */

/* {{{ test_cache_miss */
TEST(cache_miss) {
    cleanup_test_dir();

    ImageCache* cache = image_cache_create(TEST_CACHE_DIR, "game003", 100, 0);
    ASSERT_NOT_NULL(cache);

    size_t out_size;
    const unsigned char* retrieved = image_cache_get(cache, "nonexistent", &out_size);
    ASSERT_NULL(retrieved);

    ImageCacheStats stats = image_cache_get_stats(cache);
    ASSERT_EQ(stats.cache_misses, 1);

    image_cache_free(cache);
}
/* }}} */

/* {{{ test_cache_has */
TEST(cache_has) {
    cleanup_test_dir();

    ImageCache* cache = image_cache_create(TEST_CACHE_DIR, "game004", 100, 0);
    ASSERT_NOT_NULL(cache);

    ASSERT(!image_cache_has(cache, "hash456"));

    unsigned char* img = create_test_image(512, 0xCD);
    image_cache_set(cache, "hash456", img, 512, 32, 32, NULL);

    ASSERT(image_cache_has(cache, "hash456"));

    free(img);
    image_cache_free(cache);
}
/* }}} */

/* {{{ test_cache_remove */
TEST(cache_remove) {
    cleanup_test_dir();

    ImageCache* cache = image_cache_create(TEST_CACHE_DIR, "game005", 100, 0);
    ASSERT_NOT_NULL(cache);

    unsigned char* img = create_test_image(512, 0xEF);
    image_cache_set(cache, "hash789", img, 512, 32, 32, NULL);

    ASSERT(image_cache_has(cache, "hash789"));
    ASSERT(image_cache_remove(cache, "hash789"));
    ASSERT(!image_cache_has(cache, "hash789"));

    /* Remove non-existent */
    ASSERT(!image_cache_remove(cache, "nonexistent"));

    free(img);
    image_cache_free(cache);
}
/* }}} */

/* {{{ test_cache_clear */
TEST(cache_clear) {
    cleanup_test_dir();

    ImageCache* cache = image_cache_create(TEST_CACHE_DIR, "game006", 100, 0);
    ASSERT_NOT_NULL(cache);

    /* Add multiple entries */
    for (int i = 0; i < 5; i++) {
        char hash[16];
        snprintf(hash, sizeof(hash), "hash_%d", i);
        unsigned char* img = create_test_image(256, (unsigned char)i);
        image_cache_set(cache, hash, img, 256, 16, 16, NULL);
        free(img);
    }

    ImageCacheStats stats = image_cache_get_stats(cache);
    ASSERT_EQ(stats.total_entries, 5);

    image_cache_clear(cache);

    stats = image_cache_get_stats(cache);
    ASSERT_EQ(stats.total_entries, 0);

    image_cache_free(cache);
}
/* }}} */

/* {{{ test_lru_eviction */
TEST(lru_eviction) {
    cleanup_test_dir();

    /* Create cache with max 3 entries */
    ImageCache* cache = image_cache_create(TEST_CACHE_DIR, "game007", 3, 0);
    ASSERT_NOT_NULL(cache);

    /* Add 3 entries */
    for (int i = 0; i < 3; i++) {
        char hash[16];
        snprintf(hash, sizeof(hash), "entry_%d", i);
        unsigned char* img = create_test_image(256, (unsigned char)i);
        image_cache_set(cache, hash, img, 256, 16, 16, NULL);
        free(img);
        usleep(10000);  /* Ensure different timestamps */
    }

    ASSERT(image_cache_has(cache, "entry_0"));
    ASSERT(image_cache_has(cache, "entry_1"));
    ASSERT(image_cache_has(cache, "entry_2"));

    /* Access entry_0 to make it recently used */
    size_t size;
    image_cache_get(cache, "entry_0", &size);
    usleep(10000);

    /* Add 4th entry, should evict entry_1 (LRU) */
    unsigned char* img4 = create_test_image(256, 0xFF);
    image_cache_set(cache, "entry_3", img4, 256, 16, 16, NULL);
    free(img4);

    /* entry_0 should still exist (was accessed) */
    ASSERT(image_cache_has(cache, "entry_0"));

    /* entry_3 should exist (just added) */
    ASSERT(image_cache_has(cache, "entry_3"));

    ImageCacheStats stats = image_cache_get_stats(cache);
    ASSERT_EQ(stats.evictions, 1);

    image_cache_free(cache);
}
/* }}} */

/* {{{ test_memory_limit_eviction */
TEST(memory_limit_eviction) {
    cleanup_test_dir();

    /* Create cache with 1KB memory limit */
    ImageCache* cache = image_cache_create(TEST_CACHE_DIR, "game008", 100, 1024);
    ASSERT_NOT_NULL(cache);

    /* Add 512-byte image */
    unsigned char* img1 = create_test_image(512, 0x11);
    image_cache_set(cache, "mem_1", img1, 512, 32, 32, NULL);
    free(img1);

    /* Add another 512-byte image */
    unsigned char* img2 = create_test_image(512, 0x22);
    image_cache_set(cache, "mem_2", img2, 512, 32, 32, NULL);
    free(img2);

    /* Add third - should trigger eviction */
    unsigned char* img3 = create_test_image(512, 0x33);
    image_cache_set(cache, "mem_3", img3, 512, 32, 32, NULL);
    free(img3);

    ImageCacheStats stats = image_cache_get_stats(cache);
    ASSERT(stats.memory_used <= 1024);
    ASSERT(stats.evictions >= 1);

    image_cache_free(cache);
}
/* }}} */

/* {{{ test_frame_snapshots */
TEST(frame_snapshots) {
    cleanup_test_dir();

    ImageCache* cache = image_cache_create(TEST_CACHE_DIR, "game009", 100, 0);
    ASSERT_NOT_NULL(cache);

    /* Save frames */
    for (int i = 1; i <= 5; i++) {
        unsigned char* img = create_test_image(256, (unsigned char)i);
        char desc[64];
        snprintf(desc, sizeof(desc), "Event %d happened", i);

        uint32_t frame_num = image_cache_save_frame(cache, img, 256, desc);
        ASSERT_EQ(frame_num, (uint32_t)i);
        free(img);
    }

    /* Retrieve frames */
    const FrameSnapshot* frame = image_cache_get_frame(cache, 3);
    ASSERT_NOT_NULL(frame);
    ASSERT_EQ(frame->frame_number, 3);
    ASSERT_NOT_NULL(frame->event_description);
    ASSERT_STR_EQ(frame->event_description, "Event 3 happened");

    /* Invalid frame number */
    ASSERT_NULL(image_cache_get_frame(cache, 0));
    ASSERT_NULL(image_cache_get_frame(cache, 100));

    image_cache_free(cache);
}
/* }}} */

/* {{{ test_export_final */
TEST(export_final) {
    cleanup_test_dir();

    ImageCache* cache = image_cache_create(TEST_CACHE_DIR, "game010", 100, 0);
    ASSERT_NOT_NULL(cache);

    unsigned char* img = create_test_image(2048, 0xFE);
    char* path = image_cache_export_final(cache, img, 2048, IMAGE_FORMAT_PNG);
    ASSERT_NOT_NULL(path);

    /* Verify file exists */
    struct stat st;
    ASSERT_EQ(stat(path, &st), 0);
    ASSERT_EQ(st.st_size, 2048);

    free(path);
    free(img);
    image_cache_free(cache);
}
/* }}} */

/* {{{ test_export_replay */
TEST(export_replay) {
    cleanup_test_dir();

    ImageCache* cache = image_cache_create(TEST_CACHE_DIR, "game011", 100, 0);
    ASSERT_NOT_NULL(cache);

    /* Save some frames */
    for (int i = 0; i < 3; i++) {
        unsigned char* img = create_test_image(128, (unsigned char)i);
        image_cache_save_frame(cache, img, 128, NULL);
        free(img);
    }

    char output_dir[256];
    snprintf(output_dir, sizeof(output_dir), "%s/replay_export", TEST_CACHE_DIR);

    int exported = image_cache_export_replay(cache, output_dir);
    ASSERT_EQ(exported, 3);

    /* Verify files */
    struct stat st;
    char path[256];
    snprintf(path, sizeof(path), "%s/frame_0001.png", output_dir);
    ASSERT_EQ(stat(path, &st), 0);

    image_cache_free(cache);
}
/* }}} */

/* {{{ test_persistence */
TEST(persistence) {
    cleanup_test_dir();

    /* Create cache and add entry */
    ImageCache* cache1 = image_cache_create(TEST_CACHE_DIR, "game012", 100, 0);
    ASSERT_NOT_NULL(cache1);

    unsigned char* img = create_test_image(512, 0xAA);
    image_cache_set(cache1, "persist_hash", img, 512, 32, 32, "persist test");
    free(img);

    image_cache_save_metadata(cache1);
    image_cache_free(cache1);

    /* Reload cache */
    ImageCache* cache2 = image_cache_load_game(TEST_CACHE_DIR, "game012");
    ASSERT_NOT_NULL(cache2);

    /* Entry should be loadable from disk */
    ASSERT(image_cache_has(cache2, "persist_hash"));

    size_t size;
    const unsigned char* data = image_cache_get(cache2, "persist_hash", &size);
    ASSERT_NOT_NULL(data);
    ASSERT_EQ(size, 512);

    image_cache_free(cache2);
}
/* }}} */

/* {{{ test_hash_prompt */
TEST(hash_prompt) {
    char* hash1 = image_cache_hash_prompt("A brave knight in shining armor", 12345);
    ASSERT_NOT_NULL(hash1);
    ASSERT_EQ(strlen(hash1), 32);

    /* Same prompt, same seed = same hash */
    char* hash2 = image_cache_hash_prompt("A brave knight in shining armor", 12345);
    ASSERT_STR_EQ(hash1, hash2);

    /* Different seed = different hash */
    char* hash3 = image_cache_hash_prompt("A brave knight in shining armor", 54321);
    ASSERT_NE(strcmp(hash1, hash3), 0);

    /* Different prompt = different hash */
    char* hash4 = image_cache_hash_prompt("A fierce dragon breathing fire", 12345);
    ASSERT_NE(strcmp(hash1, hash4), 0);

    free(hash1);
    free(hash2);
    free(hash3);
    free(hash4);
}
/* }}} */

/* {{{ test_duplicate_set */
TEST(duplicate_set) {
    cleanup_test_dir();

    ImageCache* cache = image_cache_create(TEST_CACHE_DIR, "game013", 100, 0);
    ASSERT_NOT_NULL(cache);

    unsigned char* img = create_test_image(256, 0xBB);

    /* Set same hash twice */
    ASSERT(image_cache_set(cache, "dup_hash", img, 256, 16, 16, NULL));
    ASSERT(image_cache_set(cache, "dup_hash", img, 256, 16, 16, NULL));

    /* Should still only have one entry */
    ImageCacheStats stats = image_cache_get_stats(cache);
    ASSERT_EQ(stats.total_entries, 1);

    free(img);
    image_cache_free(cache);
}
/* }}} */

/* {{{ test_use_count */
TEST(use_count) {
    cleanup_test_dir();

    ImageCache* cache = image_cache_create(TEST_CACHE_DIR, "game014", 100, 0);
    ASSERT_NOT_NULL(cache);

    unsigned char* img = create_test_image(256, 0xCC);
    image_cache_set(cache, "count_hash", img, 256, 16, 16, NULL);
    free(img);

    /* Access multiple times */
    size_t size;
    for (int i = 0; i < 5; i++) {
        image_cache_get(cache, "count_hash", &size);
    }

    ImageCacheStats stats = image_cache_get_stats(cache);
    ASSERT_EQ(stats.cache_hits, 5);

    image_cache_free(cache);
}
/* }}} */

/* {{{ test_null_handling */
TEST(null_handling) {
    /* NULL cache */
    ASSERT_NULL(image_cache_get(NULL, "hash", NULL));
    ASSERT(!image_cache_set(NULL, "hash", NULL, 0, 0, 0, NULL));
    ASSERT(!image_cache_has(NULL, "hash"));
    ASSERT(!image_cache_remove(NULL, "hash"));

    cleanup_test_dir();
    ImageCache* cache = image_cache_create(TEST_CACHE_DIR, "game015", 100, 0);

    /* NULL prompt_hash */
    ASSERT_NULL(image_cache_get(cache, NULL, NULL));
    ASSERT(!image_cache_set(cache, NULL, NULL, 0, 0, 0, NULL));

    /* NULL image data */
    ASSERT(!image_cache_set(cache, "hash", NULL, 100, 16, 16, NULL));

    /* Zero size */
    unsigned char dummy = 0;
    ASSERT(!image_cache_set(cache, "hash", &dummy, 0, 16, 16, NULL));

    image_cache_free(cache);
}
/* }}} */

/* {{{ main */
int main(void) {
    printf("Image Cache Tests\n");
    printf("=================\n\n");

    RUN_TEST(cache_create);
    RUN_TEST(cache_set_get);
    RUN_TEST(cache_miss);
    RUN_TEST(cache_has);
    RUN_TEST(cache_remove);
    RUN_TEST(cache_clear);
    RUN_TEST(lru_eviction);
    RUN_TEST(memory_limit_eviction);
    RUN_TEST(frame_snapshots);
    RUN_TEST(export_final);
    RUN_TEST(export_replay);
    RUN_TEST(persistence);
    RUN_TEST(hash_prompt);
    RUN_TEST(duplicate_set);
    RUN_TEST(use_count);
    RUN_TEST(null_handling);

    printf("\n=================\n");
    printf("Results: %d passed, %d failed\n", tests_passed, tests_failed);

    cleanup_test_dir();

    return tests_failed > 0 ? 1 : 0;
}
/* }}} */
