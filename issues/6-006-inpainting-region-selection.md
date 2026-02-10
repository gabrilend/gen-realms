# 6-006: Inpainting Region Selection

## Current Behavior
No system to determine which canvas region to generate next.

## Intended Behavior
Logic to select which region to inpaint based on game events:
- Maps game events to appropriate regions
- Prioritizes unfilled regions
- Considers narrative importance
- Supports progressive reveal
- Manages generation queue

## Suggested Implementation Steps

1. Create `src/visual/region-selector.h`:
   ```c
   // {{{ selector types
   typedef struct {
       CanvasRegion region;
       int priority;
       GameEvent* trigger_event;
       char* prompt_context;
   } RegionTask;

   typedef struct {
       RegionTask* queue;
       int queue_size;
       int queue_capacity;
       BattleCanvas* canvas;
   } RegionSelector;
   // }}}
   ```

2. Define event-to-region mapping:
   ```c
   // {{{ event mapping
   typedef struct {
       GameEventType event_type;
       CanvasRegion region;
       int base_priority;
   } EventRegionMap;

   static const EventRegionMap EVENT_REGIONS[] = {
       {EVENT_GAME_START, REGION_SKY, 100},
       {EVENT_BASE_PLAYED, REGION_P1_BASE, 80},
       {EVENT_CARD_PLAYED, REGION_P1_FORCES, 60},
       {EVENT_ATTACK, REGION_CENTER, 90},
       {EVENT_GAME_OVER, REGION_CENTER, 100}
   };
   // }}}
   ```

3. Implement `selector_init()`:
   ```c
   // {{{ init
   RegionSelector* selector_init(BattleCanvas* canvas) {
       RegionSelector* rs = malloc(sizeof(RegionSelector));
       rs->canvas = canvas;
       rs->queue_capacity = 32;
       rs->queue = calloc(rs->queue_capacity, sizeof(RegionTask));
       return rs;
   }
   // }}}
   ```

4. Implement `selector_queue_event()`:
   ```c
   // {{{ queue event
   void selector_queue_event(RegionSelector* rs, GameEvent* event) {
       CanvasRegion region = map_event_to_region(event);

       // Skip if region already filled
       if (rs->canvas->region_filled[region]) return;

       // Add to priority queue
       RegionTask task = {region, get_priority(event), event, NULL};
       queue_insert_sorted(rs, &task);
   }
   // }}}
   ```

5. Implement `selector_get_next()` to pop highest priority

6. Implement `selector_generate_mask()` for inpainting mask

7. Add region dependency tracking (background first)

8. Write tests for selection logic

## Related Documents
- 6-005-battle-canvas-manager.md
- 1-007-card-effect-system.md

## Dependencies
- 6-005: Battle Canvas Manager
- 1-007: Card Effect System (for game events)

## Selection Priority

| Event Type | Region | Priority |
|------------|--------|----------|
| Game Start | SKY | 100 (sets scene) |
| Attack | CENTER | 90 (action) |
| Base Played | P1/P2_BASE | 80 |
| Card Played | P1/P2_FORCES | 60 |
| Turn End | (none) | - |

## Sub-Issues
This issue has been split into sub-issues for manageable implementation:
- 6-006a: Region Selector Structure
- 6-006b: Event-to-Region Mapping
- 6-006c: Region Priority Queue

## Acceptance Criteria
- [ ] Events map to correct regions (6-006b)
- [ ] Priority queue works correctly (6-006c)
- [ ] Unfilled regions selected first (6-006c)
- [ ] Masks generated for inpainting (6-006c)
- [ ] Dependencies respected (bg first) (6-006c)
