# Track D Progress: Game Content

## Overview
Track D handles card design and balance work, creating the actual game content
with fantasy-themed cards, faction designs, and complete playable card sets.

## Current Status: Alpha → Delta (Content Creation)

## Checkpoint Progress

| Checkpoint | Status | Notes |
|------------|--------|-------|
| ALPHA (Structures) | waiting | Depends on 1-001 card struct from Track A |
| BETA (Serialization) | blocked | Depends on Track A |
| GAMMA (Protocol) | blocked | Depends on Track B |
| DELTA (Playable) | blocked | Depends on Tracks A-C |
| EPSILON (Content Complete) | in_progress | Content created, balance pending |

## Issues

### Pre-Alpha (Schema)
| ID | Description | Status | Notes |
|----|-------------|--------|-------|
| 4-001 | Card JSON Schema | completed | `assets/cards/schema.json` |

### Alpha → Delta (Content Creation)
| ID | Description | Status | Notes |
|----|-------------|--------|-------|
| 4-002 | Merchant Guilds | completed | 14 cards (10 ships, 3 bases, 1 token) |
| 4-003 | The Wilds | completed | 14 cards (10 ships, 3 bases, 1 token) |
| 4-004 | High Kingdom | completed | 14 cards (10 ships, 3 bases, 1 token) |
| 4-005 | Artificer Order | completed | 15 cards (11 ships, 3 bases, 1 token) |
| 4-006 | Starting Deck | completed | 2 cards (scout, viper) |
| 4-007 | Neutral Trade | completed | 6 cards |
| 4-008 | Upgrade Effects | completed | Integrated in Artificer faction |

### Delta → Epsilon (Balance)
| ID | Description | Status | Notes |
|----|-------------|--------|-------|
| 4-009 | Balance Validator | pending | Requires Track A game engine |
| 4-010 | Phase 4 Demo | pending | Requires Tracks A-C complete |

## Statistics

**Total Cards Created: 65**

| Faction | Ships | Bases | Tokens | Total |
|---------|-------|-------|--------|-------|
| Starting | 2 | 0 | 0 | 2 |
| Neutral | 6 | 0 | 0 | 6 |
| Merchant Guilds | 10 | 3 | 1 | 14 |
| The Wilds | 10 | 3 | 1 | 14 |
| High Kingdom | 10 | 3 | 1 | 14 |
| Artificer Order | 11 | 3 | 1 | 15 |

## Faction Themes

### Merchant Guilds
- **Primary**: Trade generation, card acquisition
- **Secondary**: Authority gain (prosperity)
- **Ally Focus**: Extra trade, card draw
- **Key Cards**: Merchant Prince, Master of Coin, Market Fortress
- **Token**: Gold Tribute (trade + authority scrap)

### The Wilds
- **Primary**: Combat power, swarming
- **Secondary**: Card draw (pack hunting)
- **Ally Focus**: Combat multipliers
- **Key Cards**: Primal Titan, Alpha Pack Leader, Heart of the Forest
- **Token**: Spirit Wolf (combat + draw scrap)

### High Kingdom
- **Primary**: Authority gain, defensive bases
- **Secondary**: Opponent disruption (discard)
- **Ally Focus**: Authority, enemy weakening
- **Key Cards**: Divine Sovereign, High Paladin, Cathedral of Light
- **Token**: Castle Infantry (draw + combat)

### Artificer Order
- **Primary**: Scrapping, deck thinning
- **Secondary**: Card manipulation, upgrades
- **Ally Focus**: Scrap synergies, free acquisitions
- **Key Cards**: Blacksmith, Goldweaver, Enchanter, Master Artificer
- **Token**: Battle Construct (combat + trade scrap)

## Deliverables

- [x] `assets/cards/schema.json` - Card definition schema
- [x] `assets/cards/starting/*.json` - Starting deck cards
- [x] `assets/cards/neutral/*.json` - Neutral trade deck
- [x] `assets/cards/merchant/*.json` - Merchant Guilds faction
- [x] `assets/cards/wilds/*.json` - The Wilds faction
- [x] `assets/cards/kingdom/*.json` - High Kingdom faction
- [x] `assets/cards/artificer/*.json` - Artificer Order faction
- [ ] `src/tools/balance-validator.c` - Balance analysis (4-009)
- [ ] `issues/completed/demos/phase-4-demo.sh` - Demo script (4-010)

## Session Log

### 2026-02-10
- Created JSON schema with all effect types
- Implemented all 4 factions with full card sets
- Created starting deck and neutral trade cards
- Upgrade effects integrated into Artificer faction (blacksmith, goldweaver, enchanter)
- Each faction has spawning mechanic via bases with token units
- Total 65 cards created

## Blocking Dependencies
- 4-009 and 4-010 cannot proceed until Track A (game engine) is complete
- Balance testing requires functional game loop
