# 2-006: Starting Deck Definition

## Current Behavior
No starting deck defined. Players have no initial cards.

## Intended Behavior
Define the starting deck that each player begins with:
- 8 Scout cards (basic trade)
- 2 Viper cards (basic combat)
- All cards are neutral (no faction)
- Simple effects for learning the game
- Fantasy-themed names and flavor

## Suggested Implementation Steps

1. Create `assets/cards/starting/` directory
2. Define Scout card:
   ```lua
   {
     id = "scout",
     name = "Village Scout",
     cost = 0,  -- cannot be bought
     faction = "neutral",
     card_type = "ship",
     effects = {{type = "add_trade", value = 1}},
     flavor = "The roads are long, but coin makes them shorter."
   }
   ```
3. Define Viper card:
   ```lua
   {
     id = "viper",
     name = "Hedge Knight",
     cost = 0,  -- cannot be bought
     faction = "neutral",
     card_type = "ship",
     effects = {{type = "add_combat", value = 1}},
     flavor = "Steel speaks louder than gold."
   }
   ```
4. Create deck initialization function
5. Ensure starting deck shuffles before first draw
6. Write tests for starting deck setup

## Related Documents
- docs/02-game-mechanics.md
- 1-002-deck-management-system.md

## Dependencies
- 2-001: Card Template Format

## Acceptance Criteria
- [ ] Scout and Viper cards defined
- [ ] Each player starts with 8 Scouts, 2 Vipers
- [ ] Starting deck shuffles before game
- [ ] Cards cannot be purchased (cost 0 or marker)
- [ ] Fantasy flavor appropriate
