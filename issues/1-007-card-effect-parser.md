# 1-007: Card Effect System

## Current Behavior
No effect execution exists. Card data cannot translate to game actions.

## Intended Behavior
A C-based effect system using dispatch tables that:
- Reads effect definitions from card data
- Executes effects in correct order
- Supports all effect types (trade, combat, authority, draw, scrap, upgrade, etc.)
- Handles conditional effects (ally abilities)
- Handles choice effects ("choose one")
- Provides hooks for effect execution events (narrative, visuals)
- Supports card upgrades that modify effect values

## Sub-Issues

This issue has been split into the following sub-issues:

| ID | Description | Status |
|----|-------------|--------|
| 1-007a | Effect Dispatch Infrastructure | pending |
| 1-007b | Resource Effects | pending |
| 1-007c | Card Manipulation Effects | pending |
| 1-007d | Special Effects | pending |
| 1-007e | Upgrade and Spawn Effects | pending |
| 1-007f | Conditional and Ally Abilities | pending |

## Implementation Order

1. **1-007a** first - establishes infrastructure all others depend on
2. **1-007b** second - simple effects to validate infrastructure
3. **1-007c** third - card manipulation requires deck integration
4. **1-007d** fourth - special effects build on basics
5. **1-007e** fifth - Symbeline-specific mechanics
6. **1-007f** last - conditional logic wraps everything together

## Related Documents
- docs/02-game-mechanics.md (upgrade system)
- docs/04-architecture-c-server.md
- 1-001-card-data-structure.md

## Dependencies
- 1-001: Card Data Structure (effects stored in cards, upgrades in instances)
- 1-003: Player State Management (effects modify player)
- 1-002: Deck Management (draw effects)

## Acceptance Criteria
- [ ] Effect dispatch table compiles and works
- [ ] All basic effect types execute correctly
- [ ] Ally abilities trigger when faction matches
- [ ] Scrap abilities work correctly
- [ ] Complex cards with multiple effects work
- [ ] Upgrade bonuses from CardInstance are applied
- [ ] Event callbacks fire for narrative integration
