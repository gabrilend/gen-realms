# 5-005: Event Narration Prompts

## Status: COMPLETED

## Current Behavior
Event narration system generates dramatic narrative prompts for all game events with intensity scaling based on game state, damage values, and event significance.

## Intended Behavior
Event-specific prompts that generate narrative for:
- Card plays
- Purchases
- Attacks
- Base destruction
- Turn transitions
- Game end

## Implementation Details

### Files Created
- `src/llm/05-event-narration.h` - Header with event types and function declarations
- `src/llm/05-event-narration.c` - Implementation with prompt builders
- `tests/test-event-narration.c` - Test suite (23 tests)

### Key Components

1. **GameEventType Enum**: 10 event types including:
   - GAME_EVENT_CARD_PLAYED
   - GAME_EVENT_CARD_PURCHASED
   - GAME_EVENT_ATTACK_PLAYER
   - GAME_EVENT_ATTACK_BASE
   - GAME_EVENT_BASE_DESTROYED
   - GAME_EVENT_TURN_START
   - GAME_EVENT_TURN_END
   - GAME_EVENT_GAME_OVER
   - GAME_EVENT_ALLY_TRIGGERED
   - GAME_EVENT_SCRAP

2. **NarrationIntensity Enum**: 4 intensity levels:
   - INTENSITY_LOW: Brief, understated (turn transitions, small damage)
   - INTENSITY_MEDIUM: Standard dramatic
   - INTENSITY_HIGH: Very dramatic (large damage, base destroyed, high tension)
   - INTENSITY_EPIC: Maximum drama (game over)

3. **NarrationEvent Struct**: Contains event context including:
   - Actor and target players
   - Card and base involved
   - Damage, cost, and turn values
   - Calculated intensity

4. **Intensity Calculation**: Based on:
   - Event type (game over = epic, base destroyed = high)
   - World state tension (>0.7 = high)
   - Damage amount (>=10 = high, <=2 = low)
   - Purchase cost (<=2 = low)
   - Turn transitions (always low)

### Functions Implemented
- `event_narration_create()` - Create event struct
- `event_narration_free()` - Free event struct
- `event_narration_build()` - Main dispatch function
- `event_narration_build_card_played()` - Card play prompts
- `event_narration_build_purchase()` - Purchase prompts
- `event_narration_build_attack()` - Attack on player prompts
- `event_narration_build_base_attack()` - Attack on base prompts
- `event_narration_build_base_destroyed()` - Base destruction prompts
- `event_narration_build_turn_start()` - Turn start prompts
- `event_narration_build_turn_end()` - Turn end summary prompts
- `event_narration_build_game_over()` - Epic game ending prompts
- `event_narration_calculate_intensity()` - Dynamic intensity
- `event_narration_get_intensity_word()` - Intensity to word
- `event_type_to_string()` - Type to string

### Test Results
All 23 tests passing:
- Event creation (2 tests)
- String conversions (2 tests)
- Intensity calculations (6 tests)
- Narration builds (9 tests)
- Event struct dispatch (4 tests)

## Related Documents
- 5-002-prompt-network-structure.md
- 1-008d-event-emission.md

## Dependencies
- 5-002: Prompt Network Structure (completed)
- 5-004: Force Description Prompts (uses faction themes)

## Acceptance Criteria
- [x] All event types have templates
- [x] Narration quality is good
- [x] Attack intensity scales with damage
- [x] Turn transitions are brief
- [x] Game over is epic
