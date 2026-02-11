/*
 * 03-image-cache.h - Image Caching and Persistence
 *
 * Server-side caching system for generated images. Caches images by prompt
 * hash to avoid redundant generation, persists canvas states for game replay,
 * and supports final battle scene export.
 */

#ifndef IMAGE_CACHE_H
#define IMAGE_CACHE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

/* {{{ ImageFormat */
typedef enum {
    IMAGE_FORMAT_PNG,
    IMAGE_FORMAT_JPEG
} ImageFormat;
/* }}} */

/* {{{ ImageCacheEntry */
typedef struct {
    char* prompt_hash;          /* SHA256 hash of generation prompt */
    unsigned char* image_data;  /* Raw image bytes */
    size_t image_size;          /* Size of image data */
    time_t generated_at;        /* Unix timestamp of generation */
    time_t last_accessed;       /* Unix timestamp of last access */
    uint32_t use_count;         /* Number of cache hits */
    uint32_t width;             /* Image width in pixels */
    uint32_t height;            /* Image height in pixels */
    char* source_prompt;        /* Original prompt (for debugging) */
} ImageCacheEntry;
/* }}} */

/* {{{ FrameSnapshot */
typedef struct {
    uint32_t frame_number;      /* Sequential frame number */
    time_t captured_at;         /* Unix timestamp */
    unsigned char* image_data;  /* Raw PNG bytes */
    size_t image_size;          /* Size of image data */
    char* event_description;    /* What happened this frame */
} FrameSnapshot;
/* }}} */

/* {{{ ImageCacheStats */
typedef struct {
    uint32_t total_entries;     /* Number of cached images */
    uint32_t cache_hits;        /* Total cache hits */
    uint32_t cache_misses;      /* Total cache misses */
    uint64_t memory_used;       /* Bytes used by cache */
    uint64_t disk_used;         /* Bytes used on disk */
    uint32_t evictions;         /* Number of LRU evictions */
} ImageCacheStats;
/* }}} */

/* {{{ ImageCache */
typedef struct {
    ImageCacheEntry* entries;   /* Array of cache entries */
    uint32_t entry_count;       /* Current number of entries */
    uint32_t max_entries;       /* Maximum entries before eviction */
    uint64_t max_memory;        /* Maximum memory usage in bytes */
    uint64_t current_memory;    /* Current memory usage */

    FrameSnapshot* frames;      /* Array of frame snapshots */
    uint32_t frame_count;       /* Number of captured frames */
    uint32_t max_frames;        /* Maximum frames to keep */

    char* cache_dir;            /* Base cache directory path */
    char* game_id;              /* Current game session ID */
    char* game_dir;             /* Full path to game cache directory */

    ImageCacheStats stats;      /* Cache statistics */
} ImageCache;
/* }}} */

/* {{{ image_cache_create
 * Creates a new image cache.
 * @param cache_dir - Base directory for cache storage
 * @param game_id - Unique game session identifier
 * @param max_entries - Maximum cached images before eviction
 * @param max_memory - Maximum memory usage in bytes (0 for unlimited)
 */
ImageCache* image_cache_create(const char* cache_dir,
                                const char* game_id,
                                uint32_t max_entries,
                                uint64_t max_memory);
/* }}} */

/* {{{ image_cache_free
 * Frees all memory associated with cache.
 * Does NOT delete files from disk.
 */
void image_cache_free(ImageCache* cache);
/* }}} */

/* {{{ image_cache_get
 * Retrieves a cached image by prompt hash.
 * @param cache - The image cache
 * @param prompt_hash - SHA256 hash of the prompt
 * @param out_size - Output: size of image data
 * @return Image data (do not free) or NULL if not cached
 */
const unsigned char* image_cache_get(ImageCache* cache,
                                      const char* prompt_hash,
                                      size_t* out_size);
/* }}} */

/* {{{ image_cache_set
 * Adds an image to the cache.
 * @param cache - The image cache
 * @param prompt_hash - SHA256 hash of the prompt
 * @param image_data - Raw image bytes (copied)
 * @param image_size - Size of image data
 * @param width - Image width in pixels
 * @param height - Image height in pixels
 * @param source_prompt - Original prompt text (can be NULL)
 * @return true on success, false on failure
 */
bool image_cache_set(ImageCache* cache,
                     const char* prompt_hash,
                     const unsigned char* image_data,
                     size_t image_size,
                     uint32_t width,
                     uint32_t height,
                     const char* source_prompt);
/* }}} */

/* {{{ image_cache_has
 * Checks if an image is cached.
 * @return true if cached, false otherwise
 */
bool image_cache_has(ImageCache* cache, const char* prompt_hash);
/* }}} */

/* {{{ image_cache_remove
 * Removes an image from cache (memory and disk).
 * @return true if found and removed, false if not found
 */
bool image_cache_remove(ImageCache* cache, const char* prompt_hash);
/* }}} */

/* {{{ image_cache_clear
 * Clears all cached images (memory only, preserves disk).
 */
void image_cache_clear(ImageCache* cache);
/* }}} */

/* {{{ image_cache_save_frame
 * Captures a frame snapshot for game replay.
 * @param cache - The image cache
 * @param image_data - Raw image bytes (PNG)
 * @param image_size - Size of image data
 * @param event_description - Description of what happened
 * @return Frame number assigned
 */
uint32_t image_cache_save_frame(ImageCache* cache,
                                 const unsigned char* image_data,
                                 size_t image_size,
                                 const char* event_description);
/* }}} */

/* {{{ image_cache_get_frame
 * Retrieves a frame snapshot by number.
 * @param cache - The image cache
 * @param frame_number - Frame to retrieve
 * @return Frame snapshot (do not free) or NULL if not found
 */
const FrameSnapshot* image_cache_get_frame(ImageCache* cache,
                                            uint32_t frame_number);
/* }}} */

/* {{{ image_cache_export_final
 * Exports the final battle scene to a file.
 * @param cache - The image cache
 * @param image_data - Raw image bytes
 * @param image_size - Size of image data
 * @param format - Output format (PNG or JPEG)
 * @return Path to exported file (caller must free) or NULL on failure
 */
char* image_cache_export_final(ImageCache* cache,
                                const unsigned char* image_data,
                                size_t image_size,
                                ImageFormat format);
/* }}} */

/* {{{ image_cache_export_replay
 * Exports all frame snapshots as an image sequence.
 * @param cache - The image cache
 * @param output_dir - Directory to write frames (created if needed)
 * @return Number of frames exported, or -1 on error
 */
int image_cache_export_replay(ImageCache* cache, const char* output_dir);
/* }}} */

/* {{{ image_cache_load_game
 * Loads cached data for a previous game session.
 * @param cache_dir - Base cache directory
 * @param game_id - Game session to load
 * @return Loaded cache or NULL if not found
 */
ImageCache* image_cache_load_game(const char* cache_dir, const char* game_id);
/* }}} */

/* {{{ image_cache_save_metadata
 * Saves cache metadata to disk for later reload.
 * @return true on success, false on failure
 */
bool image_cache_save_metadata(ImageCache* cache);
/* }}} */

/* {{{ image_cache_get_stats
 * Returns cache statistics.
 */
ImageCacheStats image_cache_get_stats(const ImageCache* cache);
/* }}} */

/* {{{ image_cache_hash_prompt
 * Generates a hash for a generation prompt.
 * Uses SHA256, returns hex string.
 * @param prompt - The prompt to hash
 * @param seed - Random seed used for generation
 * @return Hex hash string (caller must free)
 */
char* image_cache_hash_prompt(const char* prompt, uint32_t seed);
/* }}} */

/* {{{ image_cache_evict_lru
 * Evicts least recently used entries until under memory limit.
 * @param cache - The image cache
 * @param bytes_needed - Bytes to free up
 * @return Number of entries evicted
 */
int image_cache_evict_lru(ImageCache* cache, uint64_t bytes_needed);
/* }}} */

/* {{{ image_cache_persist_entry
 * Writes a single cache entry to disk.
 * @return true on success, false on failure
 */
bool image_cache_persist_entry(ImageCache* cache, const ImageCacheEntry* entry);
/* }}} */

/* {{{ image_cache_load_entry
 * Loads a cache entry from disk into memory.
 * @param cache - The image cache
 * @param prompt_hash - Hash of entry to load
 * @return true if loaded, false if not found
 */
bool image_cache_load_entry(ImageCache* cache, const char* prompt_hash);
/* }}} */

#endif /* IMAGE_CACHE_H */
