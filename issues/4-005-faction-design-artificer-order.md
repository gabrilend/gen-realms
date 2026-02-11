# 2-005: Faction Design - Artificer Order

## Current Behavior
No faction cards exist.

## Intended Behavior
Design and implement the Artificer Order faction with:
- Theme: Magical constructs, recycling, efficiency, transformation
- Primary mechanic: Scrapping (self and trade row)
- Secondary mechanic: Deck thinning, card manipulation
- Ally bonus focus: Scrap synergies, free acquisitions
- Fantasy flavor: Golem crafters, arcane engineers, magical workshops

## Suggested Implementation Steps

1. Design 8-12 faction cards with cost curve (1-7 trade):
   ```
   Low cost (1-2): Apprentice, small constructs
   Mid cost (3-4): Journeyman, battle golems
   High cost (5-7): Master artificers, colossi
   ```
2. Design 2-3 faction bases:
   - Workshop (scrap trade row cards)
   - Golem foundry (spawns construct tokens)
   - Arcane forge (transform cards)
3. Create card files in `assets/cards/artificer/`
4. Include strong scrap abilities
5. Focus on deck efficiency and quality over quantity
6. Write flavor text reflecting crafting themes
7. Document faction strategy guide

## Related Documents
- docs/02-game-mechanics.md
- 2-001-card-template-format.md

## Dependencies
- 2-001: Card Template Format (file structure)

## Example Cards

**Construct Apprentice** (Cost 1, Ship)
- Effects: +1 Trade
- Scrap: +2 Combat

**Battle Golem** (Cost 3, Ship)
- Effects: +4 Combat
- Ally: Scrap a card from trade row

**Master Artificer** (Cost 6, Ship)
- Effects: +3 Trade, +3 Combat
- Ally: Acquire a ship costing 4 or less for free
- Scrap: Draw 2 cards

**Golem Foundry** (Cost 4, Base, Defense 5)
- Effects: Spawn Construct Token each turn
- Construct Token: +2 Combat, scrap to gain +3 Trade

**Arcane Workshop** (Cost 3, Base, Defense 4)
- Effects: You may scrap a card from your hand or discard

## Acceptance Criteria
- [x] 8-12 ship cards created
- [x] 2-3 base cards created
- [x] Scrap abilities are central
- [x] Deck thinning strategy viable
- [x] Free acquisition balanced

## Completion Notes (2026-02-10)

**Status: COMPLETED**

### Cards Created (15 total)
See `assets/cards/artificer/` for full card files.

### Addendum (2026-02-11) - Upgrade Restriction to Starting Cards

**Upgrade specialists now only target Scouts and Vipers in hand:**

| Card | Target | Effect | Notes |
|------|--------|--------|-------|
| Blacksmith | Viper only | +2 Attack | Was +1 to any card in discard |
| Goldweaver | Scout only | +1 Trade ×2 | Two upgrades per turn |
| Enchanter | Scout or Viper | +2 Authority | Was +1 to any card in discard |

**Design Rationale:**
- Creates scrap vs upgrade tension (can't do both to same card)
- Upgrades target hand, not discard (need right card drawn)
- Cross-faction value: any player can splash upgraders
- Thematic: Blacksmith sharpens weapons (Vipers), Goldweaver gilds purses (Scouts)

**Schema Additions:**

`target_card` field restricts which cards can be upgraded:
- `viper`: Vipers only
- `scout`: Scouts only
- `scout_or_viper`: Either starting card
- `any`: No restriction (default)

`workshop` field (boolean) for persistent upgraders:
- Card enters protected workshop zone when played
- Cannot be attacked while waiting
- Draw effect triggers immediately
- Upgrade resolves when target card appears in hand
- Card discards after upgrade completes

Like Kingdom Coin, workshop lets you invest now and collect later.
