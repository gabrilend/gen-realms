# 4-008: Upgrade Effect Cards

## Current Behavior
No cards exist that permanently modify other cards.

## Intended Behavior
Create cards that apply permanent upgrades to other cards in the player's deck:
- Upgrade cards that target cards in discard pile
- Types: +Attack, +Trade, +Authority
- Integrated into Artificer Order faction theme
- Visual indication of upgraded cards (art regeneration trigger)

## Suggested Implementation Steps

1. Define upgrade effect types in schema:
   - `upgrade_attack`: Add permanent +X combat to target card
   - `upgrade_trade`: Add permanent +X trade to target card
   - `upgrade_authority`: Add permanent +X authority to target card
2. Create upgrade cards in Artificer Order:
   - Blacksmith: +1 attack upgrade
   - Goldweaver: +1 trade upgrade
   - Enchanter: +1 authority upgrade
3. Ensure upgraded cards have `needs_regen` flag set
4. Document upgrade stacking rules

## Related Documents
- docs/02-game-mechanics.md (Card Upgrade System section)
- 4-005-faction-design-artificer-order.md

## Dependencies
- 4-001: Card JSON Schema (upgrade effect types)
- 4-005: Artificer Order (faction integration)

## Acceptance Criteria
- [x] Upgrade effect types defined in schema
- [x] Blacksmith, Goldweaver, Enchanter cards created
- [x] Cards integrated into Artificer Order faction
- [x] Effects target cards in discard pile

## Completion Notes (2026-02-10)

**Status: COMPLETED**

Upgrade effect cards were implemented as part of the Artificer Order faction (4-005).

### Cards Created:
- **Blacksmith** (cost 4): Draw 1, upgrade_attack +1 to discard
- **Goldweaver** (cost 4): Draw 1, upgrade_trade +1 to discard
- **Enchanter** (cost 4): Draw 1, upgrade_authority +1 to discard

### Design Rationale:
- All upgrade cards cost 4 trade, creating consistent cost curve
- Each draws a card to maintain tempo while upgrading
- All have relevant scrap effects (+3 of their stat type)
- Upgrade targets discard pile, requiring strategic planning

### Schema Support:
The schema.json includes these effect types:
```json
"upgrade_attack": { "value": 1, "target": "discard" }
"upgrade_trade": { "value": 1, "target": "discard" }
"upgrade_authority": { "value": 1, "target": "discard" }
```

### Integration Notes:
- Upgrades fit Artificer Order's theme of magical enhancement
- Cards synergize with faction's scrap/deck-thinning strategy
- Master Artificer provides choice of any upgrade type as ally ability alternative
