# 6-003d: Cache Invalidation

## Parent Issue
6-003: Dynamic Art Regeneration

## Current Behavior
No image cache management exists.

## Intended Behavior
Cache system that:
- Stores generated images by instance_id
- Invalidates on upgrade or style change
- Persists across page reloads (IndexedDB)
- Manages storage limits
- Supports manual cache clear

## Suggested Implementation Steps

1. Create cache structure:
   ```javascript
   // {{{ ImageCache class
   class ImageCache {
       constructor(options = {}) {
           this.memoryCache = new Map();  // Fast lookup
           this.maxMemorySize = options.maxMemorySize || 50;
           this.dbName = 'symbeline_images';
           this.storeName = 'card_images';
           this.db = null;
       }
   }
   // }}}
   ```

2. Initialize IndexedDB:
   ```javascript
   // {{{ init db
   async init() {
       return new Promise((resolve, reject) => {
           const request = indexedDB.open(this.dbName, 1);

           request.onerror = () => reject(request.error);
           request.onsuccess = () => {
               this.db = request.result;
               resolve();
           };

           request.onupgradeneeded = (event) => {
               const db = event.target.result;
               if (!db.objectStoreNames.contains(this.storeName)) {
                   const store = db.createObjectStore(this.storeName, {
                       keyPath: 'instance_id'
                   });
                   store.createIndex('timestamp', 'timestamp');
                   store.createIndex('seed', 'seed');
               }
           };
       });
   }
   // }}}
   ```

3. Implement cache get:
   ```javascript
   // {{{ get
   async get(instanceId) {
       // Check memory first
       if (this.memoryCache.has(instanceId)) {
           return this.memoryCache.get(instanceId);
       }

       // Check IndexedDB
       const entry = await this.getFromDB(instanceId);
       if (entry) {
           // Promote to memory cache
           this.memoryCache.set(instanceId, entry.imageData);
           this.evictIfNeeded();
           return entry.imageData;
       }

       return null;
   }
   // }}}
   ```

4. Implement cache set:
   ```javascript
   // {{{ set
   async set(instanceId, imageData, seed) {
       const entry = {
           instance_id: instanceId,
           imageData: imageData,
           seed: seed,
           timestamp: Date.now()
       };

       // Store in memory
       this.memoryCache.set(instanceId, imageData);
       this.evictIfNeeded();

       // Store in IndexedDB
       await this.saveToDB(entry);
   }
   // }}}
   ```

5. Implement invalidation:
   ```javascript
   // {{{ invalidate
   async invalidate(instanceId) {
       this.memoryCache.delete(instanceId);
       await this.deleteFromDB(instanceId);
   }

   async invalidateAll() {
       this.memoryCache.clear();
       await this.clearDB();
   }

   async invalidateByCondition(predicate) {
       // Invalidate entries matching condition
       const entries = await this.getAllFromDB();
       for (const entry of entries) {
           if (predicate(entry)) {
               await this.invalidate(entry.instance_id);
           }
       }
   }
   // }}}
   ```

6. Handle style change invalidation:
   ```javascript
   // {{{ style change
   function onStylePreferencesChanged() {
       // Could invalidate all, or mark for lazy regen
       // For now, invalidate all to ensure consistency
       imageCache.invalidateAll();
       artTracker.markAllForRegeneration();
   }
   // }}}
   ```

7. Implement LRU eviction:
   ```javascript
   // {{{ evict
   evictIfNeeded() {
       while (this.memoryCache.size > this.maxMemorySize) {
           // Remove oldest entry
           const firstKey = this.memoryCache.keys().next().value;
           this.memoryCache.delete(firstKey);
       }
   }
   // }}}
   ```

8. Add cache statistics:
   ```javascript
   // {{{ stats
   getStats() {
       return {
           memorySize: this.memoryCache.size,
           maxMemory: this.maxMemorySize,
           // dbSize requires async query
       };
   }
   // }}}
   ```

9. Write tests for cache behavior

## Related Documents
- 6-003a-regeneration-tracking.md
- 6-003b-style-guide-integration.md
- 6-009-image-caching-persistence.md

## Dependencies
- IndexedDB (browser API)

## Cache Invalidation Rules

| Event | Action |
|-------|--------|
| Card upgraded | Invalidate that card's entry |
| Style guide changed | Invalidate all entries |
| Manual regen request | Invalidate that card's entry |
| Storage full | LRU eviction from memory |
| User clears cache | Invalidate all entries |

## Acceptance Criteria
- [ ] Memory cache provides fast lookup
- [ ] IndexedDB persists across reloads
- [ ] Invalidation clears correct entries
- [ ] LRU eviction manages memory
- [ ] Style change triggers invalidation
- [ ] Manual clear works
