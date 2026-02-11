# 1-001: Card Data Structure

## Current Behavior
No card data structure exists. The project has no way to represent cards in memory.

## Intended Behavior
C structs that can represent any card in the game, including:
- Basic attributes (name, cost, faction)
- Effect definitions (combat, trade, authority, draw)
- Card type (ship, base, unit)
- Ally abilities (triggered when same-faction card is played)
- Scrap abilities (one-time use by removing from game)
- Metadata (id, art reference, flavor text)

Additionally, a CardInstance struct for tracking individual card copies with upgrades.

## Suggested Implementation Steps

1. Create `src/core/01-card.h` with type definitions
2. Create `src/core/01-card.c` with implementation
3. Define the CardType struct:
   ```c
   typedef enum {
       FACTION_NEUTRAL,
       FACTION_MERCHANT,
       FACTION_WILDS,
       FACTION_KINGDOM,
       FACTION_ARTIFICER
   } Faction;

   typedef enum {
       CARD_TYPE_SHIP,
       CARD_TYPE_BASE,
       CARD_TYPE_UNIT
   } CardKind;

   typedef struct {
       char* id;
       char* name;
       int cost;
       Faction faction;
       CardKind card_type;
       Effect* effects;
       int effect_count;
       Effect* ally_effects;
       int ally_effect_count;
       Effect* scrap_effects;
       int scrap_effect_count;
       char* spawns_id;      // for bases that spawn units
       int defense;          // for bases
       bool is_outpost;      // for bases
       char* flavor;
   } CardType;
   ```
4. Define CardInstance struct for in-game copies:
   ```c
   typedef struct {
       CardType* type;       // pointer to base card definition
       char* instance_id;    // unique identifier
       int attack_bonus;     // permanent upgrade
       int trade_bonus;      // permanent upgrade
       int authority_bonus;  // permanent upgrade
       int image_seed;       // for art generation
       bool needs_regen;     // art is outdated
   } CardInstance;
   ```
5. Implement `CardInstance* card_instance_create(CardType* type)`
6. Implement `void card_instance_free(CardInstance* inst)`
7. Write tests in `tests/test-card.c`

## Related Documents
- docs/01-architecture-overview.md
- docs/02-game-mechanics.md
- docs/04-architecture-c-server.md

## Acceptance Criteria
- [x] Card structs compile without errors
- [x] Can create card instances with all required fields
- [x] Memory allocation/deallocation works correctly
- [x] Test demonstrates CardType and CardInstance creation

## Implementation Notes (2026-02-10)
- Created src/core/01-card.h with Effect, CardType, CardInstance structs
- Created src/core/01-card.c with create/free functions
- Added upgrade support via card_instance_apply_upgrade()
- Added total_combat/trade/authority helpers for upgrade calculation
- 20 tests passing in tests/test-core.c
