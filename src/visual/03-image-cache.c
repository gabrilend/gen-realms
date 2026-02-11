/*
 * 03-image-cache.c - Image Caching and Persistence Implementation
 *
 * Server-side caching system for generated images. Uses SHA256 for prompt
 * hashing, LRU eviction for memory management, and file-based persistence.
 */

#include "03-image-cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

/* Simple SHA256 implementation for prompt hashing */
/* For production, link against OpenSSL or similar */

/* {{{ sha256_simple
 * Simple hash function for prompts (not cryptographic).
 * Production should use proper SHA256 from OpenSSL.
 */
static uint64_t hash64(const char* str) {
    uint64_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}
/* }}} */

/* {{{ make_directory
 * Creates directory if it doesn't exist.
 */
static bool make_directory(const char* path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) != 0 && errno != EEXIST) {
            return false;
        }
    }
    return true;
}
/* }}} */

/* {{{ write_file
 * Writes binary data to file.
 */
static bool write_file(const char* path, const unsigned char* data, size_t size) {
    FILE* f = fopen(path, "wb");
    if (!f) return false;

    size_t written = fwrite(data, 1, size, f);
    fclose(f);

    return written == size;
}
/* }}} */

/* {{{ read_file
 * Reads binary data from file.
 * Returns malloc'd buffer, caller must free.
 */
static unsigned char* read_file(const char* path, size_t* out_size) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0) {
        fclose(f);
        return NULL;
    }

    unsigned char* data = malloc(size);
    if (!data) {
        fclose(f);
        return NULL;
    }

    size_t read = fread(data, 1, size, f);
    fclose(f);

    if (read != (size_t)size) {
        free(data);
        return NULL;
    }

    *out_size = size;
    return data;
}
/* }}} */

/* {{{ find_entry_index
 * Finds entry index by prompt hash.
 * Returns -1 if not found.
 */
static int find_entry_index(ImageCache* cache, const char* prompt_hash) {
    for (uint32_t i = 0; i < cache->entry_count; i++) {
        if (cache->entries[i].prompt_hash &&
            strcmp(cache->entries[i].prompt_hash, prompt_hash) == 0) {
            return (int)i;
        }
    }
    return -1;
}
/* }}} */

/* {{{ find_lru_index
 * Finds least recently used entry.
 * Returns -1 if cache empty.
 */
static int find_lru_index(ImageCache* cache) {
    if (cache->entry_count == 0) return -1;

    int lru_idx = 0;
    time_t lru_time = cache->entries[0].last_accessed;

    for (uint32_t i = 1; i < cache->entry_count; i++) {
        if (cache->entries[i].last_accessed < lru_time) {
            lru_time = cache->entries[i].last_accessed;
            lru_idx = (int)i;
        }
    }

    return lru_idx;
}
/* }}} */

/* {{{ free_entry
 * Frees memory for a single entry.
 */
static void free_entry(ImageCacheEntry* entry) {
    if (entry->prompt_hash) {
        free(entry->prompt_hash);
        entry->prompt_hash = NULL;
    }
    if (entry->image_data) {
        free(entry->image_data);
        entry->image_data = NULL;
    }
    if (entry->source_prompt) {
        free(entry->source_prompt);
        entry->source_prompt = NULL;
    }
    entry->image_size = 0;
}
/* }}} */

/* {{{ image_cache_create */
ImageCache* image_cache_create(const char* cache_dir,
                                const char* game_id,
                                uint32_t max_entries,
                                uint64_t max_memory) {
    ImageCache* cache = calloc(1, sizeof(ImageCache));
    if (!cache) return NULL;

    cache->cache_dir = strdup(cache_dir);
    cache->game_id = strdup(game_id);
    cache->max_entries = max_entries > 0 ? max_entries : 256;
    cache->max_memory = max_memory > 0 ? max_memory : 256 * 1024 * 1024; /* 256 MB default */
    cache->max_frames = 1000;

    cache->entries = calloc(cache->max_entries, sizeof(ImageCacheEntry));
    cache->frames = calloc(cache->max_frames, sizeof(FrameSnapshot));

    if (!cache->entries || !cache->frames) {
        image_cache_free(cache);
        return NULL;
    }

    /* Create game-specific cache directory */
    size_t path_len = strlen(cache_dir) + strlen(game_id) + 16;
    cache->game_dir = malloc(path_len);
    snprintf(cache->game_dir, path_len, "%s/%s", cache_dir, game_id);

    /* Create directories */
    make_directory(cache_dir);
    make_directory(cache->game_dir);

    /* Create subdirectories */
    char subdir[512];
    snprintf(subdir, sizeof(subdir), "%s/cache", cache->game_dir);
    make_directory(subdir);

    snprintf(subdir, sizeof(subdir), "%s/frames", cache->game_dir);
    make_directory(subdir);

    return cache;
}
/* }}} */

/* {{{ image_cache_free */
void image_cache_free(ImageCache* cache) {
    if (!cache) return;

    /* Free entries */
    if (cache->entries) {
        for (uint32_t i = 0; i < cache->entry_count; i++) {
            free_entry(&cache->entries[i]);
        }
        free(cache->entries);
    }

    /* Free frames */
    if (cache->frames) {
        for (uint32_t i = 0; i < cache->frame_count; i++) {
            if (cache->frames[i].image_data) {
                free(cache->frames[i].image_data);
            }
            if (cache->frames[i].event_description) {
                free(cache->frames[i].event_description);
            }
        }
        free(cache->frames);
    }

    free(cache->cache_dir);
    free(cache->game_id);
    free(cache->game_dir);
    free(cache);
}
/* }}} */

/* {{{ image_cache_get */
const unsigned char* image_cache_get(ImageCache* cache,
                                      const char* prompt_hash,
                                      size_t* out_size) {
    if (!cache || !prompt_hash || !out_size) return NULL;

    int idx = find_entry_index(cache, prompt_hash);
    if (idx < 0) {
        cache->stats.cache_misses++;

        /* Try loading from disk */
        if (image_cache_load_entry(cache, prompt_hash)) {
            idx = find_entry_index(cache, prompt_hash);
            if (idx >= 0) {
                cache->stats.cache_hits++;
                cache->entries[idx].last_accessed = time(NULL);
                cache->entries[idx].use_count++;
                *out_size = cache->entries[idx].image_size;
                return cache->entries[idx].image_data;
            }
        }

        return NULL;
    }

    cache->stats.cache_hits++;
    cache->entries[idx].last_accessed = time(NULL);
    cache->entries[idx].use_count++;
    *out_size = cache->entries[idx].image_size;
    return cache->entries[idx].image_data;
}
/* }}} */

/* {{{ image_cache_set */
bool image_cache_set(ImageCache* cache,
                     const char* prompt_hash,
                     const unsigned char* image_data,
                     size_t image_size,
                     uint32_t width,
                     uint32_t height,
                     const char* source_prompt) {
    if (!cache || !prompt_hash || !image_data || image_size == 0) {
        return false;
    }

    /* Check if already in memory (don't check disk - caller may be loading from disk) */
    if (find_entry_index(cache, prompt_hash) >= 0) {
        return true;  /* Already in memory */
    }

    /* Check memory limits */
    if (cache->current_memory + image_size > cache->max_memory) {
        image_cache_evict_lru(cache, image_size);
    }

    /* Check entry limits */
    if (cache->entry_count >= cache->max_entries) {
        image_cache_evict_lru(cache, 0);
    }

    /* Find slot */
    uint32_t idx = cache->entry_count;
    if (idx >= cache->max_entries) {
        /* Eviction failed, use LRU slot */
        int lru = find_lru_index(cache);
        if (lru < 0) return false;
        idx = (uint32_t)lru;
        cache->current_memory -= cache->entries[idx].image_size;
        free_entry(&cache->entries[idx]);
    } else {
        cache->entry_count++;
    }

    /* Create entry */
    ImageCacheEntry* entry = &cache->entries[idx];
    entry->prompt_hash = strdup(prompt_hash);
    entry->image_data = malloc(image_size);
    if (!entry->image_data) {
        free(entry->prompt_hash);
        entry->prompt_hash = NULL;
        cache->entry_count--;
        return false;
    }

    memcpy(entry->image_data, image_data, image_size);
    entry->image_size = image_size;
    entry->width = width;
    entry->height = height;
    entry->generated_at = time(NULL);
    entry->last_accessed = entry->generated_at;
    entry->use_count = 0;

    if (source_prompt) {
        entry->source_prompt = strdup(source_prompt);
    }

    cache->current_memory += image_size;
    cache->stats.total_entries = cache->entry_count;
    cache->stats.memory_used = cache->current_memory;

    /* Persist to disk */
    image_cache_persist_entry(cache, entry);

    return true;
}
/* }}} */

/* {{{ image_cache_has */
bool image_cache_has(ImageCache* cache, const char* prompt_hash) {
    if (!cache || !prompt_hash) return false;

    if (find_entry_index(cache, prompt_hash) >= 0) {
        return true;
    }

    /* Check disk */
    char path[512];
    snprintf(path, sizeof(path), "%s/cache/%s.png", cache->game_dir, prompt_hash);

    struct stat st;
    return stat(path, &st) == 0;
}
/* }}} */

/* {{{ image_cache_remove */
bool image_cache_remove(ImageCache* cache, const char* prompt_hash) {
    if (!cache || !prompt_hash) return false;

    int idx = find_entry_index(cache, prompt_hash);
    if (idx >= 0) {
        cache->current_memory -= cache->entries[idx].image_size;
        free_entry(&cache->entries[idx]);

        /* Compact array */
        if ((uint32_t)idx < cache->entry_count - 1) {
            memmove(&cache->entries[idx],
                    &cache->entries[idx + 1],
                    (cache->entry_count - idx - 1) * sizeof(ImageCacheEntry));
        }
        cache->entry_count--;
    }

    /* Remove from disk */
    char path[512];
    snprintf(path, sizeof(path), "%s/cache/%s.png", cache->game_dir, prompt_hash);
    remove(path);

    return idx >= 0;
}
/* }}} */

/* {{{ image_cache_clear */
void image_cache_clear(ImageCache* cache) {
    if (!cache) return;

    for (uint32_t i = 0; i < cache->entry_count; i++) {
        free_entry(&cache->entries[i]);
    }
    cache->entry_count = 0;
    cache->current_memory = 0;
    cache->stats.total_entries = 0;
    cache->stats.memory_used = 0;
}
/* }}} */

/* {{{ image_cache_save_frame */
uint32_t image_cache_save_frame(ImageCache* cache,
                                 const unsigned char* image_data,
                                 size_t image_size,
                                 const char* event_description) {
    if (!cache || !image_data || image_size == 0) return 0;

    if (cache->frame_count >= cache->max_frames) {
        /* Remove oldest frame */
        if (cache->frames[0].image_data) {
            free(cache->frames[0].image_data);
        }
        if (cache->frames[0].event_description) {
            free(cache->frames[0].event_description);
        }
        memmove(&cache->frames[0], &cache->frames[1],
                (cache->max_frames - 1) * sizeof(FrameSnapshot));
        cache->frame_count--;
    }

    uint32_t frame_num = cache->frame_count + 1;
    FrameSnapshot* frame = &cache->frames[cache->frame_count];

    frame->frame_number = frame_num;
    frame->captured_at = time(NULL);
    frame->image_data = malloc(image_size);
    if (frame->image_data) {
        memcpy(frame->image_data, image_data, image_size);
        frame->image_size = image_size;
    }

    if (event_description) {
        frame->event_description = strdup(event_description);
    }

    cache->frame_count++;

    /* Save to disk */
    char path[512];
    snprintf(path, sizeof(path), "%s/frames/frame_%04u.png",
             cache->game_dir, frame_num);
    write_file(path, image_data, image_size);

    return frame_num;
}
/* }}} */

/* {{{ image_cache_get_frame */
const FrameSnapshot* image_cache_get_frame(ImageCache* cache,
                                            uint32_t frame_number) {
    if (!cache || frame_number == 0 || frame_number > cache->frame_count) {
        return NULL;
    }

    return &cache->frames[frame_number - 1];
}
/* }}} */

/* {{{ image_cache_export_final */
char* image_cache_export_final(ImageCache* cache,
                                const unsigned char* image_data,
                                size_t image_size,
                                ImageFormat format) {
    if (!cache || !image_data || image_size == 0) return NULL;

    const char* ext = (format == IMAGE_FORMAT_JPEG) ? "jpg" : "png";

    size_t path_len = strlen(cache->game_dir) + 32;
    char* path = malloc(path_len);
    snprintf(path, path_len, "%s/final.%s", cache->game_dir, ext);

    if (!write_file(path, image_data, image_size)) {
        free(path);
        return NULL;
    }

    return path;
}
/* }}} */

/* {{{ image_cache_export_replay */
int image_cache_export_replay(ImageCache* cache, const char* output_dir) {
    if (!cache || !output_dir) return -1;

    if (!make_directory(output_dir)) {
        return -1;
    }

    int exported = 0;
    char path[512];

    for (uint32_t i = 0; i < cache->frame_count; i++) {
        FrameSnapshot* frame = &cache->frames[i];
        if (!frame->image_data) continue;

        snprintf(path, sizeof(path), "%s/frame_%04u.png", output_dir, i + 1);
        if (write_file(path, frame->image_data, frame->image_size)) {
            exported++;
        }
    }

    return exported;
}
/* }}} */

/* {{{ image_cache_load_game */
ImageCache* image_cache_load_game(const char* cache_dir, const char* game_id) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s/metadata.json", cache_dir, game_id);

    struct stat st;
    if (stat(path, &st) != 0) {
        return NULL;  /* Game not found */
    }

    ImageCache* cache = image_cache_create(cache_dir, game_id, 256, 0);
    if (!cache) return NULL;

    /* Load frames from disk */
    snprintf(path, sizeof(path), "%s/%s/frames", cache_dir, game_id);
    DIR* dir = opendir(path);
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strncmp(entry->d_name, "frame_", 6) == 0) {
                char frame_path[768];
                snprintf(frame_path, sizeof(frame_path), "%s/%s", path, entry->d_name);

                size_t size;
                unsigned char* data = read_file(frame_path, &size);
                if (data) {
                    image_cache_save_frame(cache, data, size, NULL);
                    free(data);
                }
            }
        }
        closedir(dir);
    }

    /* Note: Cached images are loaded on-demand via image_cache_get() */
    /* which calls image_cache_load_entry() on cache miss */

    return cache;
}
/* }}} */

/* {{{ image_cache_save_metadata */
bool image_cache_save_metadata(ImageCache* cache) {
    if (!cache) return false;

    char path[512];
    snprintf(path, sizeof(path), "%s/metadata.json", cache->game_dir);

    FILE* f = fopen(path, "w");
    if (!f) return false;

    fprintf(f, "{\n");
    fprintf(f, "  \"game_id\": \"%s\",\n", cache->game_id);
    fprintf(f, "  \"entry_count\": %u,\n", cache->entry_count);
    fprintf(f, "  \"frame_count\": %u,\n", cache->frame_count);
    fprintf(f, "  \"cache_hits\": %u,\n", cache->stats.cache_hits);
    fprintf(f, "  \"cache_misses\": %u,\n", cache->stats.cache_misses);
    fprintf(f, "  \"memory_used\": %lu\n", (unsigned long)cache->current_memory);
    fprintf(f, "}\n");

    fclose(f);
    return true;
}
/* }}} */

/* {{{ image_cache_get_stats */
ImageCacheStats image_cache_get_stats(const ImageCache* cache) {
    if (!cache) {
        ImageCacheStats empty = {0};
        return empty;
    }
    return cache->stats;
}
/* }}} */

/* {{{ image_cache_hash_prompt */
char* image_cache_hash_prompt(const char* prompt, uint32_t seed) {
    if (!prompt) return NULL;

    /* Combine prompt and seed for hash */
    size_t len = strlen(prompt) + 16;
    char* combined = malloc(len);
    snprintf(combined, len, "%s:%u", prompt, seed);

    uint64_t h1 = hash64(combined);
    uint64_t h2 = hash64(combined + strlen(combined) / 2);

    free(combined);

    /* Format as hex string */
    char* hash = malloc(33);
    snprintf(hash, 33, "%016lx%016lx", (unsigned long)h1, (unsigned long)h2);

    return hash;
}
/* }}} */

/* {{{ image_cache_evict_lru */
int image_cache_evict_lru(ImageCache* cache, uint64_t bytes_needed) {
    if (!cache) return 0;

    int evicted = 0;
    uint64_t freed = 0;

    while ((bytes_needed > 0 && freed < bytes_needed) ||
           cache->entry_count >= cache->max_entries) {

        int lru = find_lru_index(cache);
        if (lru < 0) break;

        freed += cache->entries[lru].image_size;
        cache->current_memory -= cache->entries[lru].image_size;

        /* Don't delete from disk on eviction, just memory */
        free_entry(&cache->entries[lru]);

        /* Compact array */
        if ((uint32_t)lru < cache->entry_count - 1) {
            memmove(&cache->entries[lru],
                    &cache->entries[lru + 1],
                    (cache->entry_count - lru - 1) * sizeof(ImageCacheEntry));
        }
        cache->entry_count--;
        evicted++;
        cache->stats.evictions++;

        if (bytes_needed == 0) break;  /* Only needed to free one slot */
    }

    cache->stats.memory_used = cache->current_memory;
    return evicted;
}
/* }}} */

/* {{{ image_cache_persist_entry */
bool image_cache_persist_entry(ImageCache* cache, const ImageCacheEntry* entry) {
    if (!cache || !entry || !entry->prompt_hash || !entry->image_data) {
        return false;
    }

    char path[512];
    snprintf(path, sizeof(path), "%s/cache/%s.png",
             cache->game_dir, entry->prompt_hash);

    if (!write_file(path, entry->image_data, entry->image_size)) {
        return false;
    }

    /* Update disk usage stats */
    cache->stats.disk_used += entry->image_size;

    return true;
}
/* }}} */

/* {{{ image_cache_load_entry */
bool image_cache_load_entry(ImageCache* cache, const char* prompt_hash) {
    if (!cache || !prompt_hash) return false;

    char path[512];
    snprintf(path, sizeof(path), "%s/cache/%s.png",
             cache->game_dir, prompt_hash);

    size_t size;
    unsigned char* data = read_file(path, &size);
    if (!data) return false;

    /* Add to memory cache */
    bool result = image_cache_set(cache, prompt_hash, data, size, 0, 0, NULL);
    free(data);

    return result;
}
/* }}} */
