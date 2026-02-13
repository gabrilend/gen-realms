# 8-013: Faction Synergy Heuristics

## Current Behavior
AI evaluates card purchases based on immediate value (cost, effects) without
considering how cards interact with existing deck composition. No preference
is given to cards that would trigger ally abilities on previously acquired cards.

## Intended Behavior
A heuristic system that biases AI card acquisition toward faction synergies:
- Cards sharing a faction with existing ally-effect cards are valued higher
- The bonus compounds: more ally effects of a faction = stronger preference
- Creates emergent faction-focused deck building without explicit rules
- AI naturally develops coherent deck strategies over the course of a game

## Core Concept

When the AI has a card with an ally effect (e.g., "Ally: Draw a card" on a
Merchant Guilds card), future Merchant Guilds cards become more valuable because:
1. They can trigger the ally effect on the first card
2. They may have their own ally effects, creating more triggers
3. This creates positive feedback toward faction concentration

```
Turn 1: Buy "Trade Caravan" (Merchant, Ally: +2 Trade)
        → Merchant faction value increases

Turn 3: Evaluating trade row:
        - "War Beast" (Wilds, 4 cost)     → base value: 3.2
        - "Gold Courier" (Merchant, 4 cost) → base value: 3.0
                                           + synergy bonus: +0.8
                                           = effective value: 3.8 ← preferred!
```

## Suggested Implementation Steps

1. Create `src/ai/13-synergy.h` with synergy tracking types:
   ```c
   /* {{{ synergy types */
   typedef struct FactionSynergyTracker FactionSynergyTracker;

   /* Ally effect tracking per faction */
   typedef struct {
       Faction faction;
       int ally_effect_count;      /* Cards with ally abilities */
       int faction_card_count;     /* Total cards of this faction */
       float trigger_probability;  /* Estimated chance to trigger */
   } FactionAllyStats;

   /* Synergy score components */
   typedef struct {
       float existing_ally_bonus;   /* Bonus from triggering existing allies */
       float new_ally_potential;    /* Bonus if this card has ally effect */
       float faction_density;       /* How focused the deck is */
       float total_synergy;         /* Combined score */
   } SynergyScore;
   /* }}} */
   ```

2. Create `src/ai/13-synergy.c` implementing:
   ```c
   /* {{{ tracker lifecycle */
   FactionSynergyTracker* synergy_tracker_create(void);
   void synergy_tracker_destroy(FactionSynergyTracker* tracker);
   void synergy_tracker_reset(FactionSynergyTracker* tracker);
   /* }}} */

   /* {{{ deck analysis */
   /* Analyze player's deck for ally effects */
   void synergy_analyze_deck(FactionSynergyTracker* tracker,
                              Player* player);

   /* Get stats for a specific faction */
   FactionAllyStats synergy_get_faction_stats(FactionSynergyTracker* tracker,
                                                Faction faction);
   /* }}} */

   /* {{{ card scoring */
   /* Score a card based on synergy with existing deck */
   SynergyScore synergy_score_card(FactionSynergyTracker* tracker,
                                    CardType* card,
                                    Player* player);

   /* Quick check: does buying this card improve faction coherence? */
   float synergy_faction_fit(FactionSynergyTracker* tracker,
                              Faction faction);
   /* }}} */
   ```

3. Implement ally effect detection:
   ```c
   /* {{{ ally detection */
   /* Check if a card has ally abilities */
   bool card_has_ally_effect(CardType* card);

   /* Get the faction that triggers this card's ally */
   Faction card_ally_faction(CardType* card);

   /* Estimate value of an ally effect */
   float estimate_ally_value(Effect* ally_effect);
   /* }}} */
   ```

4. Implement trigger probability calculation:
   ```c
   /* {{{ trigger probability */
   /* Estimate probability of drawing faction card with ally card */
   float synergy_trigger_probability(FactionSynergyTracker* tracker,
                                      Faction faction,
                                      Player* player);

   /* Factors:
    * - faction_cards / total_cards in deck
    * - average hand size
    * - cards drawn per turn
    */
   /* }}} */
   ```

5. Implement scoring formula:
   ```c
   /* {{{ scoring formula */
   /* Base formula for synergy bonus:
    *
    * synergy_bonus = sum over factions f:
    *   (ally_effects[f] * trigger_prob[f] * ally_value[f])
    *
    * Where:
    *   ally_effects[f] = count of ally-effect cards for faction f in deck
    *   trigger_prob[f] = P(drawing card of faction f with ally card)
    *   ally_value[f]   = average value of ally effects for faction f
    *
    * For a card of faction X being evaluated:
    *   - If deck has N cards with "Ally (X): effect"
    *   - Buying this card increases trigger chance for all N
    *   - Bonus scales with N and effect quality
    */
   /* }}} */
   ```

6. Integrate with 8-003 (Option Generation) and 8-007 (Evaluation):
   ```c
   /* {{{ integration */
   /* Add synergy component to economy evaluation */
   float eval_economy_with_synergy(Game* state,
                                    int player_id,
                                    FactionSynergyTracker* tracker);

   /* Modify card purchase scoring in option generation */
   float option_score_purchase(GeneratedOption* opt,
                                Game* state,
                                FactionSynergyTracker* tracker);
   /* }}} */
   ```

## Synergy Scoring Example

```
Player's Deck:
  - 2x Merchant cards with "Ally: +2 Trade"
  - 1x Merchant card with "Ally: Draw 1"
  - 3x Wilds cards (no ally effects)
  - 4x Neutral cards

Evaluating "Gold Courier" (Merchant, 3 cost, +2 Trade):
  - Base value: 2.5
  - Ally bonus calculation:
    * Merchant ally effects in deck: 3
    * Trigger probability: ~35% (3 Merchant / 10 total)
    * Average ally value: 2.0 (+2 trade ≈ 1.5, draw 1 ≈ 2.5)
    * Synergy bonus: 3 × 0.35 × 2.0 × SYNERGY_WEIGHT = +0.84
  - New ally potential:
    * Gold Courier has no ally effect: +0.0
  - Final score: 2.5 + 0.84 = 3.34

Evaluating "Dire Wolf" (Wilds, 3 cost, +3 Combat):
  - Base value: 2.8
  - Ally bonus calculation:
    * Wilds ally effects in deck: 0
    * Synergy bonus: 0
  - Final score: 2.8

→ AI prefers Gold Courier despite slightly lower base value
```

## Configuration Constants

```c
/* {{{ synergy weights */
#define SYNERGY_WEIGHT_BASE      0.4f  /* Base multiplier for synergy score */
#define SYNERGY_WEIGHT_STACKING  0.15f /* Bonus per additional ally effect */
#define SYNERGY_DENSITY_BONUS    0.2f  /* Bonus for high faction density */
#define SYNERGY_NEW_ALLY_WEIGHT  0.3f  /* Value of gaining new ally effect */
#define SYNERGY_MIN_THRESHOLD    2     /* Min ally cards to apply bonus */
/* }}} */
```

## Faction Density Effects

```
Faction Density    Synergy Multiplier    Effect
     < 20%              0.8x             Discourage dilution
    20-40%              1.0x             Neutral
    40-60%              1.2x             Encourage focus
     > 60%              1.4x             Strong faction lock
```

## Related Documents
- docs/02-game-mechanics.md (ally ability rules)
- 8-003-option-generation-system.md (economy options)
- 8-007-tree-pruning-evaluation.md (heuristic evaluation)

## Dependencies
- 8-001: AI Runner Infrastructure (action execution)
- 8-003: Option Generation (purchase option scoring)
- 8-007: Tree Pruning/Evaluation (heuristic integration)
- 1-007: Card Effect Parser (ally effect detection)

## Acceptance Criteria
- [ ] Ally effects detected correctly for all cards
- [ ] Synergy tracker updates as deck changes
- [ ] Trigger probability calculated reasonably
- [ ] Synergy bonus increases purchase preference for matching factions
- [ ] AI builds more faction-coherent decks over time
- [ ] Bonus compounds appropriately (more allies = stronger preference)
- [ ] Integration with evaluation heuristics working
- [ ] Unit tests cover scoring formula edge cases

## Notes

This heuristic creates emergent faction-focused deck building without
hard-coded "build Merchant" or "build Wilds" strategies. The AI
naturally gravitates toward factions where it already has ally synergies,
leading to more coherent and powerful deck compositions.

The key insight is that ally effects create network effects in deck building:
- First ally effect card is worth its face value
- Second card of same faction unlocks the first card's bonus
- Third card can trigger two ally effects
- etc.

This positive feedback loop is what makes faction concentration powerful
in Star Realms-style games, and the AI should recognize and exploit it.
