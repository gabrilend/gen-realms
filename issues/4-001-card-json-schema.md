# 4-001: Card JSON Schema

## Current Behavior
Card data structure defined in 1-001 but no external card definition format exists.

## Intended Behavior
A standardized file format for defining cards that:
- Uses JSON for card definitions
- Supports all card properties from the card data structure
- Is human-readable and editable
- Can be validated against JSON schema
- Organizes cards by faction/type

## Suggested Implementation Steps

1. Create `assets/cards/` directory structure:
   ```
   assets/cards/
     schema.json
     starting/
     merchant/
     wilds/
     kingdom/
     artificer/
     neutral/
   ```
2. Define JSON Schema with all card fields
3. Include effect type definitions
4. Support optional fields (ally_effects, scrap_effects, etc.)
5. Create example cards demonstrating all properties

## Related Documents
- 1-001-card-data-structure.md
- docs/01-architecture-overview.md
- docs/02-game-mechanics.md

## Dependencies
- 1-001: Card Data Structure (defines what cards contain)

## Acceptance Criteria
- [x] JSON schema created at `assets/cards/schema.json`
- [x] Schema validates required fields (id, name, cost, faction, card_type, effects)
- [x] Schema supports all optional fields
- [x] Effect type definitions comprehensive
- [x] Directory structure created for all factions

## Completion Notes (2026-02-10)

**Status: COMPLETED**

Created comprehensive JSON schema at `assets/cards/schema.json`.

### Schema Features:
- **Required fields**: id, name, cost, faction, card_type, effects
- **Optional fields**: ally_effects, scrap_effects, defense, is_outpost, spawns, always_available, flavor, art_prompt

### Effect Types Supported:
- `add_trade`, `add_combat`, `add_authority`
- `draw_card`
- `opponent_discard`
- `scrap_trade_row`, `scrap_hand_or_discard`
- `acquire_free` (with max_cost)
- `spawn` (with card_id)
- `upgrade_attack`, `upgrade_trade`, `upgrade_authority`
- `copy_ship`

### Directory Structure Created:
```
assets/cards/
  schema.json
  starting/     (2 cards)
  neutral/      (6 cards)
  merchant/     (14 cards)
  wilds/        (14 cards)
  kingdom/      (14 cards)
  artificer/    (15 cards)
```

### Design Decisions:
- Used JSON instead of Lua for broader tool compatibility
- Added `always_available` flag for Explorer-like cards
- Added `art_prompt` field for AI image generation
- Conditional validation: bases require defense field
