# 1-002: Deck Management System

## Current Behavior
No deck management exists. Cards cannot be organized into decks, hands, or discard piles.

## Intended Behavior
A deck management system in C that handles:
- Draw pile (face-down, ordered, supports draw order choice)
- Hand (face-up, unordered for game purposes)
- Discard pile (face-up, ordered by discard time)
- Played area (cards active this turn)
- Shuffle operation (randomize draw pile, reset regeneration flags)
- Draw operation (move from draw pile to hand in player-chosen order)
- Discard operation (move from hand/played to discard)
- Reshuffle trigger (when draw pile empty, shuffle discard into draw)

## Suggested Implementation Steps

1. Create `src/core/02-deck.h` with type definitions
2. Create `src/core/02-deck.c` with implementation
3. Define deck state structure:
   ```c
   typedef struct {
       CardInstance** draw_pile;
       int draw_pile_count;
       int draw_pile_capacity;

       CardInstance** hand;
       int hand_count;
       int hand_capacity;

       CardInstance** discard;
       int discard_count;
       int discard_capacity;

       CardInstance** played;
       int played_count;
       int played_capacity;

       CardInstance** in_play_bases;
       int base_count;
       int base_capacity;
   } Deck;
   ```
4. Implement `Deck* deck_create(void)`
5. Implement `void deck_free(Deck* deck)`
6. Implement `void deck_shuffle(Deck* deck)` - randomize and reset regen flags
7. Implement `CardInstance* deck_draw_at(Deck* deck, int index)` - draw specific position
8. Implement `void deck_draw_ordered(Deck* deck, int* order, int count)` - draw in order
9. Implement `void deck_discard(Deck* deck, CardInstance* card)`
10. Implement `void deck_play(Deck* deck, CardInstance* card)`
11. Implement `void deck_end_turn(Deck* deck)` - move played/hand to discard
12. Write tests in `tests/test-deck.c`

## Related Documents
- docs/01-architecture-overview.md
- docs/02-game-mechanics.md (draw order choice)
- 1-001-card-data-structure.md

## Dependencies
- 1-001: Card Data Structure (CardInstance must exist)

## Acceptance Criteria
- [ ] Deck struct compiles without errors
- [ ] Can initialize deck with starting cards
- [ ] Draw correctly moves cards from draw pile to hand
- [ ] Draw order choice works (draw_at, draw_ordered)
- [ ] Empty draw pile triggers automatic reshuffle of discard
- [ ] Shuffle resets needs_regen flags on card instances
- [ ] End turn correctly moves all cards to discard
- [ ] Bases persist across turns in separate area
- [ ] Memory management is correct (no leaks)
