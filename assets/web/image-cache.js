/**
 * image-cache.js - Card Image Cache for Symbeline Realms
 *
 * Two-tier caching system with fast memory cache and persistent IndexedDB.
 * Manages storage limits with LRU eviction and supports cache invalidation
 * on card upgrades or style preference changes.
 */

(function() {
    'use strict';

    /* {{{ Constants */
    var DB_NAME = 'symbeline_images';
    var DB_VERSION = 1;
    var STORE_NAME = 'card_images';
    var DEFAULT_MAX_MEMORY = 50;
    /* }}} */

    /* {{{ ImageCache class */
    function ImageCache(options) {
        options = options || {};

        this.memoryCache = new Map();           /* Fast lookup: instance_id -> imageData */
        this.memoryMeta = new Map();            /* Metadata: instance_id -> { timestamp, seed } */
        this.maxMemorySize = options.maxMemorySize || DEFAULT_MAX_MEMORY;
        this.db = null;
        this.dbReady = false;
        this.initPromise = null;

        /* Stats */
        this.hits = 0;
        this.misses = 0;
    }

    /* {{{ init
     * Initialize IndexedDB for persistent storage.
     */
    ImageCache.prototype.init = function() {
        var self = this;

        if (this.initPromise) {
            return this.initPromise;
        }

        this.initPromise = new Promise(function(resolve, reject) {
            if (!window.indexedDB) {
                console.warn('ImageCache: IndexedDB not available, using memory only');
                self.dbReady = false;
                resolve();
                return;
            }

            var request = indexedDB.open(DB_NAME, DB_VERSION);

            request.onerror = function() {
                console.error('ImageCache: Failed to open IndexedDB:', request.error);
                self.dbReady = false;
                resolve();  /* Don't reject, fall back to memory-only */
            };

            request.onsuccess = function() {
                self.db = request.result;
                self.dbReady = true;
                resolve();
            };

            request.onupgradeneeded = function(event) {
                var db = event.target.result;

                if (!db.objectStoreNames.contains(STORE_NAME)) {
                    var store = db.createObjectStore(STORE_NAME, {
                        keyPath: 'instance_id'
                    });
                    store.createIndex('timestamp', 'timestamp');
                    store.createIndex('seed', 'seed');
                }
            };
        });

        return this.initPromise;
    };
    /* }}} */

    /* {{{ has
     * Check if image exists in cache (memory or IndexedDB).
     */
    ImageCache.prototype.has = function(instanceId) {
        return this.memoryCache.has(instanceId);
    };
    /* }}} */

    /* {{{ get
     * Get image from cache. Checks memory first, then IndexedDB.
     * Returns Promise that resolves to image data or null.
     */
    ImageCache.prototype.get = function(instanceId) {
        var self = this;

        /* Check memory first */
        if (this.memoryCache.has(instanceId)) {
            this.hits++;
            return Promise.resolve(this.memoryCache.get(instanceId));
        }

        /* Check IndexedDB */
        if (!this.dbReady) {
            this.misses++;
            return Promise.resolve(null);
        }

        return this._getFromDB(instanceId).then(function(entry) {
            if (entry) {
                self.hits++;
                /* Promote to memory cache */
                self.memoryCache.set(instanceId, entry.imageData);
                self.memoryMeta.set(instanceId, {
                    timestamp: entry.timestamp,
                    seed: entry.seed
                });
                self._evictIfNeeded();
                return entry.imageData;
            }

            self.misses++;
            return null;
        });
    };
    /* }}} */

    /* {{{ set
     * Store image in cache (both memory and IndexedDB).
     */
    ImageCache.prototype.set = function(instanceId, imageData, seed) {
        var self = this;
        var timestamp = Date.now();

        /* Store in memory */
        this.memoryCache.set(instanceId, imageData);
        this.memoryMeta.set(instanceId, {
            timestamp: timestamp,
            seed: seed || 0
        });
        this._evictIfNeeded();

        /* Store in IndexedDB */
        if (!this.dbReady) {
            return Promise.resolve(true);
        }

        var entry = {
            instance_id: instanceId,
            imageData: imageData,
            seed: seed || 0,
            timestamp: timestamp
        };

        return this._saveToDB(entry);
    };
    /* }}} */

    /* {{{ invalidate
     * Remove a specific image from both caches.
     */
    ImageCache.prototype.invalidate = function(instanceId) {
        this.memoryCache.delete(instanceId);
        this.memoryMeta.delete(instanceId);

        if (!this.dbReady) {
            return Promise.resolve(true);
        }

        return this._deleteFromDB(instanceId);
    };
    /* }}} */

    /* {{{ invalidateAll
     * Clear all cached images.
     */
    ImageCache.prototype.invalidateAll = function() {
        this.memoryCache.clear();
        this.memoryMeta.clear();

        if (!this.dbReady) {
            return Promise.resolve(true);
        }

        return this._clearDB();
    };
    /* }}} */

    /* {{{ invalidateByCondition
     * Invalidate entries matching a predicate function.
     * Predicate receives entry object with { instance_id, seed, timestamp }.
     */
    ImageCache.prototype.invalidateByCondition = function(predicate) {
        var self = this;
        var toRemove = [];

        /* Check memory entries */
        this.memoryMeta.forEach(function(meta, instanceId) {
            if (predicate({ instance_id: instanceId, seed: meta.seed, timestamp: meta.timestamp })) {
                toRemove.push(instanceId);
            }
        });

        /* Remove from memory */
        for (var i = 0; i < toRemove.length; i++) {
            this.memoryCache.delete(toRemove[i]);
            this.memoryMeta.delete(toRemove[i]);
        }

        /* Remove from IndexedDB */
        if (!this.dbReady) {
            return Promise.resolve(toRemove.length);
        }

        var promises = toRemove.map(function(id) {
            return self._deleteFromDB(id);
        });

        return Promise.all(promises).then(function() {
            return toRemove.length;
        });
    };
    /* }}} */

    /* {{{ getSeed
     * Get the seed used to generate a cached image.
     */
    ImageCache.prototype.getSeed = function(instanceId) {
        var meta = this.memoryMeta.get(instanceId);
        return meta ? meta.seed : null;
    };
    /* }}} */

    /* {{{ getStats
     * Get cache statistics.
     */
    ImageCache.prototype.getStats = function() {
        var hitRate = (this.hits + this.misses) > 0
            ? this.hits / (this.hits + this.misses)
            : 0;

        return {
            memorySize: this.memoryCache.size,
            maxMemory: this.maxMemorySize,
            hits: this.hits,
            misses: this.misses,
            hitRate: hitRate,
            dbReady: this.dbReady
        };
    };
    /* }}} */

    /* {{{ resetStats
     * Reset hit/miss statistics.
     */
    ImageCache.prototype.resetStats = function() {
        this.hits = 0;
        this.misses = 0;
    };
    /* }}} */

    /* {{{ _evictIfNeeded
     * Internal: LRU eviction from memory cache.
     */
    ImageCache.prototype._evictIfNeeded = function() {
        while (this.memoryCache.size > this.maxMemorySize) {
            /* Map maintains insertion order, first key is oldest */
            var firstKey = this.memoryCache.keys().next().value;
            this.memoryCache.delete(firstKey);
            this.memoryMeta.delete(firstKey);
        }
    };
    /* }}} */

    /* {{{ _getFromDB
     * Internal: Get entry from IndexedDB.
     */
    ImageCache.prototype._getFromDB = function(instanceId) {
        var self = this;

        return new Promise(function(resolve, reject) {
            if (!self.db) {
                resolve(null);
                return;
            }

            try {
                var transaction = self.db.transaction([STORE_NAME], 'readonly');
                var store = transaction.objectStore(STORE_NAME);
                var request = store.get(instanceId);

                request.onsuccess = function() {
                    resolve(request.result || null);
                };

                request.onerror = function() {
                    console.error('ImageCache: DB get error:', request.error);
                    resolve(null);
                };
            } catch (e) {
                console.error('ImageCache: DB transaction error:', e);
                resolve(null);
            }
        });
    };
    /* }}} */

    /* {{{ _saveToDB
     * Internal: Save entry to IndexedDB.
     */
    ImageCache.prototype._saveToDB = function(entry) {
        var self = this;

        return new Promise(function(resolve, reject) {
            if (!self.db) {
                resolve(false);
                return;
            }

            try {
                var transaction = self.db.transaction([STORE_NAME], 'readwrite');
                var store = transaction.objectStore(STORE_NAME);
                var request = store.put(entry);

                request.onsuccess = function() {
                    resolve(true);
                };

                request.onerror = function() {
                    console.error('ImageCache: DB save error:', request.error);
                    resolve(false);
                };
            } catch (e) {
                console.error('ImageCache: DB transaction error:', e);
                resolve(false);
            }
        });
    };
    /* }}} */

    /* {{{ _deleteFromDB
     * Internal: Delete entry from IndexedDB.
     */
    ImageCache.prototype._deleteFromDB = function(instanceId) {
        var self = this;

        return new Promise(function(resolve, reject) {
            if (!self.db) {
                resolve(true);
                return;
            }

            try {
                var transaction = self.db.transaction([STORE_NAME], 'readwrite');
                var store = transaction.objectStore(STORE_NAME);
                var request = store.delete(instanceId);

                request.onsuccess = function() {
                    resolve(true);
                };

                request.onerror = function() {
                    console.error('ImageCache: DB delete error:', request.error);
                    resolve(false);
                };
            } catch (e) {
                console.error('ImageCache: DB transaction error:', e);
                resolve(false);
            }
        });
    };
    /* }}} */

    /* {{{ _clearDB
     * Internal: Clear all entries from IndexedDB.
     */
    ImageCache.prototype._clearDB = function() {
        var self = this;

        return new Promise(function(resolve, reject) {
            if (!self.db) {
                resolve(true);
                return;
            }

            try {
                var transaction = self.db.transaction([STORE_NAME], 'readwrite');
                var store = transaction.objectStore(STORE_NAME);
                var request = store.clear();

                request.onsuccess = function() {
                    resolve(true);
                };

                request.onerror = function() {
                    console.error('ImageCache: DB clear error:', request.error);
                    resolve(false);
                };
            } catch (e) {
                console.error('ImageCache: DB transaction error:', e);
                resolve(false);
            }
        });
    };
    /* }}} */

    /* }}} */

    /* {{{ Style change handler */

    /* When style preferences change, invalidate all cached images
     * since they need to be regenerated with new styles. */
    function onStylePreferencesChanged() {
        if (window.imageCache) {
            window.imageCache.invalidateAll().then(function() {
                console.log('ImageCache: Invalidated all images due to style change');

                /* Mark all cards for regeneration */
                if (window.artTracker) {
                    /* artTracker will handle re-marking visible cards */
                    window.artTracker.clearAll();
                }
            });
        }
    }

    /* }}} */

    /* {{{ Public API */
    window.imageCache = new ImageCache();

    /* Initialize on load */
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', function() {
            window.imageCache.init();
        });
    } else {
        window.imageCache.init();
    }

    /* Export style change handler */
    window.imageCache.onStylePreferencesChanged = onStylePreferencesChanged;
    /* }}} */

})();
