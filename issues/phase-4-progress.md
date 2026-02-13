# Phase 4 Progress: Card Content

## Goal
Create the actual game content with fantasy-themed cards, faction designs, and a complete playable card set defined in JSON.

## Status: In Progress

## Issues

| ID | Description | Status |
|----|-------------|--------|
| 4-001 | Card JSON Schema | completed |
| 4-002 | Faction Design - Merchant Guilds | completed |
| 4-003 | Faction Design - The Wilds | completed |
| 4-004 | Faction Design - High Kingdom | completed |
| 4-005 | Faction Design - Artificer Order | completed |
| 4-006 | Starting Deck Definition | completed |
| 4-007 | Neutral Trade Deck Cards | completed |
| 4-008 | Upgrade Effect Cards | completed |
| 4-009 | Card Balance Validator | pending |
| 4-010 | Phase 4 Demo | pending |

## Completed: 8/10

## Card Statistics

| Category | Count |
|----------|-------|
| Starting cards | 2 (scout, viper) |
| Neutral trade cards | 6 |
| Merchant Guilds | 24 (11 ships, 6 bases, 4 tokens, 3 coin troops) |
| The Wilds | 35 (11 ships, 11 summoner ships, 11 temp bases, 2 tokens) |
| High Kingdom | 15 (11 ships, 3 bases, 1 token) |
| Artificer Order | 15 (11 ships, 3 bases, 1 token) |
| **Total** | **97 cards** |

## Notes
Phase 4 transforms the game engine into a playable game with real content. Cards are defined in JSON with faction themes, upgrade effects, and spawning abilities. Focus on balanced faction identities and fantasy flavor.

## Implementation Notes (2026-02-10)
- Created comprehensive JSON schema at `assets/cards/schema.json`
- All card files use JSON format with effect types matching 1-001 card structure
- Upgrade effect cards (blacksmith, goldweaver, enchanter) included in Artificer faction
- Each faction has spawning bases with corresponding token units
- The `always_available` flag marks Wandering Merchant as always purchasable

## Kingdom Faction Update (2026-02-10)
- **Recruit Mechanic**: Send card to bottom of deck (march column), draw from front
- Thematic identity: logistics and deck quality through the endless march column
- See docs/02-game-mechanics.md for full Recruit mechanic documentation

## Merchant Guilds Expansion (2026-02-10)
Major faction expansion with two new mechanics:
- **Kingdom Coin**: Persistent currency (tungsten alloy, castle-stamped)
- **Spend Coin**: Ally effects that return coins for bonuses/spawns
- **Coin Troops**: Auto-draw units spawned by spending coin
- **Forecast**: High-level bases reveal draw count on top N cards
- **New Schema Fields**: `add_coin`, `spend_coin`, `auto_draw`, `forecast`
- New cards: Coin Mint, Exchange House, Guild Observatory, 3 Coin Troops

## Wilds Faction Redesign (2026-02-10)
Major redesign implemented to create unique faction identity:
- **Effect Split**: Main effects combat/draw only; ally effects resources/heal only
- **Frontier Mechanic**: Cards played to frontier add cost to base defense
- **Frontier Leaders**: Trigger all frontier cards to charge into play
- **More Bases**: 11 bases (was 3) with low defense (3-5) and utility effects
- **New Schema Fields**: `spawn_per_ally`, `frontier_bonus`, `on_destroyed`
- See docs/02-game-mechanics.md for full Frontier mechanic documentation

## Wilds Summoner Ship Conversion (2026-02-12)
Major faction restructure - all bases converted to summoner ship + temporary base pairs:
- **Summoner Ships**: Ships that spawn temporary bases, cycle through deck
- **Temporary Bases**: Have `temporary_base: true`, scrapped when destroyed
- **Wave Tactics**: Build bases, charge, lose them, replay summoners next shuffle
- **New Schema Field**: `temporary_base` for scrap-on-destroy behavior
- 11 summoner ships + 11 temporary base tokens added (24 cards → 35 cards)
- See issue 4-003 Addendum 3 for full details

## Remaining Work
- 4-009: Balance validator requires C implementation (Track A dependency)
- 4-010: Phase 4 demo requires playable game loop (Tracks A-C dependency)
