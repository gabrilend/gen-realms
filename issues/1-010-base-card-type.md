# 1-010: Base Card Type

## Current Behavior
No base card type exists. All cards are treated as ships that leave play.
Bases are currently stored in a single `bases` array in Deck.

## Intended Behavior
C-based base card handling with **frontier/interior placement choice**:

### Core Base Mechanics
- Stays in play across turns (until destroyed)
- Has defense value (must be attacked to destroy)
- Effects trigger each turn (not just when played)
- Effects activate the turn AFTER being played (deployment delay)
- Tracks current damage taken

### Frontier/Interior Zones (replaces outpost concept)
When playing a base, the player chooses to place it in either zone:

**Frontier Zone:**
- Exposed position, attacked first
- ALL frontier bases must be destroyed before interior can be targeted
- Art generation uses "exposed, battle-ready, weathered" themes
- Thematically: forward camps, watchtowers, front-line fortifications
- Strategic: sacrifice these to protect interior

**Interior Zone:**
- Protected position, attacked last
- Cannot be targeted while any frontier base exists
- Art generation uses "secure, established, ornate" themes
- Thematically: castles, guild halls, deep sanctuaries
- Strategic: place high-value bases here for protection

### Combat Targeting Priority
1. Frontier bases (all must be destroyed first)
2. Interior bases (only targetable when frontier is empty)
3. Player authority (only when no bases remain)

### Art Generation Impact
The placement zone affects the ComfyUI prompt:
```
Frontier: "exposed forward camp, battle-worn, edge of wilderness, temporary fortification"
Interior: "fortified stronghold, protected walls, ornate banners, established seat of power"
```

**Key design point:** Same base card type placed in different zones = visually distinct art.
A "Tower" placed on frontier looks like a wooden watchtower; placed in interior looks like a stone castle tower.

## Suggested Implementation Steps

1. Add placement enum to CardInstance:
   ```c
   typedef enum {
       ZONE_NONE = 0,      // Not placed (ships, hand cards)
       ZONE_FRONTIER,      // Exposed, attacked first
       ZONE_INTERIOR       // Protected, attacked last
   } BasePlacement;
   ```

2. Modify CardInstance (from 1-001):
   ```c
   // Add to CardInstance:
   BasePlacement placement;  // Where base is placed
   bool deployed;            // true after first full turn
   int damage_taken;         // tracks damage for bases
   ```

3. Split Deck base storage into two zones:
   ```c
   // Replace single bases array with:
   CardInstance** frontier_bases;
   int frontier_base_count;
   int frontier_base_capacity;

   CardInstance** interior_bases;
   int interior_base_count;
   int interior_base_capacity;
   ```

4. Implement placement functions:
   ```c
   bool deck_play_base_to_frontier(Deck* deck, CardInstance* base);
   bool deck_play_base_to_interior(Deck* deck, CardInstance* base);
   ```

5. Update combat targeting (in 06-combat.c):
   ```c
   int combat_get_valid_targets(Game* game, CombatTarget* out, int max) {
       // Check frontier bases first (all must be destroyed)
       // Then interior bases (only if frontier empty)
       // Finally player (only if no bases remain)
   }
   ```

6. Add helper for art prompt generation:
   ```c
   const char* base_placement_art_modifier(BasePlacement placement);
   // Returns "frontier" or "interior" theme keywords
   ```

7. Implement deployment delay in turn start:
   ```c
   void game_deploy_bases(Game* game, Player* player) {
       // Process both frontier and interior bases
       for each base in frontier_bases + interior_bases {
           if (!base->deployed) {
               base->deployed = true;
           } else {
               // trigger base effects
           }
       }
   }
   ```

8. Update game action to include placement choice:
   ```c
   typedef struct {
       ActionType type;
       // ... existing fields ...
       BasePlacement base_placement;  // For ACTION_PLAY_CARD when card is base
   } Action;
   ```

9. Write tests in `tests/test-base.c`

## Related Documents
- docs/02-game-mechanics.md (base rules)
- docs/04-architecture-c-server.md
- 1-006-basic-combat-resolution.md

## Dependencies
- 1-001: Card Data Structure (base properties in CardType)
- 1-002: Deck Management (base tracking array)
- 1-006: Combat (base destruction)

## Acceptance Criteria
- [ ] Bases stay in play after turn ends
- [ ] Bases do not trigger effects on first turn (deployment delay)
- [ ] Deployed bases trigger effects each turn
- [ ] Player can choose frontier or interior placement when playing a base
- [ ] Frontier bases must all be destroyed before interior can be targeted
- [ ] Interior bases cannot be attacked while frontier bases exist
- [ ] Player authority only targetable when no bases remain
- [ ] Destroyed bases go to owner's discard pile
- [ ] Base damage tracking works correctly
- [ ] Placement zone stored on CardInstance for art generation
- [ ] base_placement_art_modifier() returns zone-specific prompt keywords
