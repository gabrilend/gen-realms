# Symbeline Realms - Game Mechanics

## Base Rules (Star Realms Foundation)

### Setup
- Each player starts with:
  - 50 Authority (health)
  - Starting deck of basic cards
  - Empty discard pile

### Turn Structure
1. Draw phase (with order choice, see below)
2. Main phase (play cards, buy from trade row, attack)
3. Discard phase (discard hand and played cards)

### Trade Row
- 5 cards available for purchase
- When a card is bought, replace from trade deck
- LLM DM influences card selection (singleton encouragement)

## Symbeline Modifications

### Draw Order Choice

Players control the order they draw cards from their deck:

1. At draw phase start, player sees N "card backs" (deck positions)
2. Player chooses order: "draw 3rd, then 1st, then 5th..."
3. Server reveals cards in chosen order
4. Each card triggers image regeneration as it's drawn
5. Auto-draw effects resolve in draw order

**Strategic Value:**
- See trade row state before committing to draws
- Sequence draws to maximize ally ability chains
- Delay drawing a card until after seeing what's available

**Roguelike Feel:**
- Cards must still be drawn eventually (rotation)
- Cannot permanently skip cards in deck
- Creates moment-to-moment tactical decisions

### Deck Flow Tracker

```
        BUY CARD              SCRAP CARD
            |                     |
            v                     v
    [d10] +1 -----> 10? -----> reset to 0
                     |         +1 to d4 (extra draws)
                     |
    [d10] -1 <----- 0? <----- reset to 9
                     |         -1 from d4 (fewer draws)
```

**Initial State:**
- d10 = 5
- d4 = 0 (no bonus draws)

**Overflow (d10: 9 -> 0):**
- d4 += 1
- Draw one extra card each turn

**Underflow (d10: 0 -> 9):**
- d4 -= 1
- Draw one fewer card each turn (minimum 1)

### Auto-Draw Resolution

At the start of each turn:
1. Player selects draw order (see above)
2. Cards drawn in chosen order
3. Scan hand for auto-draw effects
4. Resolve all auto-draws immediately
5. Mark effects as "spent" until reshuffle
6. Present final hand to player

This eliminates the classic deck-builder tedium of:
- Play draw card -> draw -> play another draw card -> draw -> etc.

### Card Instance System

Each card in play is a unique instance, not just a reference to a card type:

```c
CardInstance {
    card_id: "dire_bear"        // base card type
    instance_id: "abc123"       // unique identifier
    attack_bonus: 0             // permanent upgrade
    trade_bonus: 0              // permanent upgrade
    authority_bonus: 0          // permanent upgrade
    applied_upgrades: []        // list of upgrade names
    image_seed: 42              // for art generation
    needs_regeneration: false   // art is outdated
}
```

This enables:
- Permanent card upgrades
- Unique art per card instance
- Tracking individual card history

### Card Upgrade System

Some cards permanently modify other cards:

**Upgrade Flow (Blacksmith Example):**
1. Turn N: Play "Blacksmith" (temporary card, scraps on play, draws a card)
2. Blacksmith effect: Choose a card in discard, apply "+1 attack"
3. Turn N: Upgraded card still in discard (effect pending visual update)
4. Turn N+X: Deck shuffles, upgraded card enters draw pile
5. Turn N+Y: Upgraded card drawn, new art generated showing better weapons

**Upgrade Types:**
- +Attack: Better weapons, fiercer appearance
- +Trade: Richer clothing, more cargo
- +Authority: More commanding presence, banners
- Special: Faction-specific upgrades

**Visual Impact:**
- Upgraded cards trigger art regeneration
- New art reflects upgrades (armored bear, golden merchant ship)
- Creates visual progression throughout game

### Dynamic Card Art

Card artwork regenerates throughout the game:

**Image Lifecycle:**
1. **Card drawn** → Browser client requests new image from ComfyUI
2. **Card in hand** → Current image displayed
3. **Card played** → Image persists for this action
4. **Card discarded** → Old image cached, marked for regeneration
5. **Deck shuffled** → Regeneration queue processed
6. **Card upgraded** → Forced regeneration with upgrade modifiers

**Terminal Mode:**
- No images, text descriptions only
- Upgrade status shown as text: "Dire Bear (+1 ATK)"

**Client Style Preferences:**
- Players can customize their art generation prompts
- Stored in browser localStorage
- Example: "dark fantasy, oil painting" vs "watercolor, whimsical"

### Base Mechanics

**Deployment Delay:**
- Base effects trigger on the turn AFTER being played
- Represents "setup time" for the base

**Spawning:**
- Some bases create unit cards each turn (after deployment)
- Units go to discard pile (available after shuffle)
- Example: "Castle of the Dawn" spawns "Dawn Infantry"

**Unit Cards:**
- Typically have: draw effect OR scrap-for-bonus effect
- Scrapped units "return to base" (thematic regroup)

### Card Permanence Costs

Cards that generate other cards follow this rule:
- Has draw/scrap effect: normal cost
- No draw/scrap effect (permanent tokens): +1 cost

This balances persistent value against tempo.

## Faction Mapping (Sci-Fi to Fantasy)

| Star Realms | Symbeline Realms | Primary Focus |
|-------------|------------------|---------------|
| Trade Federation | Merchant Guilds | Trade, Authority |
| Blob | The Wilds | Combat, Swarming |
| Star Empire | High Kingdom | Authority, Disruption |
| Machine Cult | Artificer Order | Scrap, Upgrades |

## Faction-Specific Mechanics

### The Wilds: Frontier & The Charge

The Wilds has two unique mechanics that work together:

#### 1. Effect Split: Combat/Draw vs Resources/Healing

Wilds cards split their effects:
- **Main effects:** Combat damage and card draw (offensive)
- **Ally effects:** Trade and Authority only (defensive/resource)

This creates a tension: play cards normally for damage, or...

#### 2. The Frontier

Wilds cards can be played to the **Frontier** instead of the play area.

**Playing to Frontier:**
1. Play a Wilds card and declare "to the Frontier"
2. Place it on/near a Wilds base you control
3. The card's **cost** adds to the base's defense temporarily
4. Ally effects trigger immediately (you get the healing/resources now)
5. Main effects do NOT trigger (no combat/draw yet)

**Example - Building the Pack:**
```
Turn 1: Play Thornwall (base, defense 4)
Turn 2: Play Wolf Scout (cost 1) to Frontier
        → Thornwall now has 5 defense
        → Ally triggers: +1 Authority (per ally rules)
Turn 3: Play Pack Hunter (cost 2) to Frontier
        → Thornwall now has 7 defense
        → Ally triggers: +1 Authority ×2 (2 allies at frontier)
Turn 4: Play Dire Bear (cost 4) to Frontier
        → Thornwall now has 11 defense
        → Ally triggers: +2 Authority ×3 (3 allies at frontier)
```

#### 3. The Charge (Frontier Leaders)

Certain Wilds cards are **Frontier Leaders** (marked with `frontier_leader: true`):
- Beastcaller (cost 3)
- Forest Shaman (cost 4)
- Alpha Pack Leader (cost 5)
- Primal Titan (cost 7)

**When a Frontier Leader is played:**
1. ALL cards at the Frontier immediately enter play
2. Their main effects trigger simultaneously
3. The base's defense returns to normal
4. Ally effects trigger again based on the full pack

**Example - The Charge:**
```
Frontier has: Wolf Scout (2), Pack Hunter (3), Dire Bear (5,draw)
Thornwall defense: 4 + 1 + 2 + 4 = 11

Play Alpha Pack Leader!
→ All 4 cards enter play simultaneously
→ Wolf Scout: 2 Combat
→ Pack Hunter: 3 Combat
→ Dire Bear: 5 Combat, Draw 1
→ Alpha Pack Leader: 5 Combat, Draw 2
→ Total: 15 Combat, Draw 3 cards

Ally effects (each card has 3 other allies):
→ 4 cards × (authority values) × 3 = massive healing burst
```

#### Strategic Implications

**Pros of Frontier:**
- Temporarily fortify your outpost (survive aggro)
- Bank ally triggers for sustained healing
- Set up devastating charge turns

**Cons of Frontier:**
- No combat damage while cards wait
- Vulnerable if base is destroyed (cards go to discard)
- Need to draw a Leader to trigger the charge

**Design Intent:**
The pack gathers at the guardhouse, strengthening its walls while
they wait. Each beast that joins heals the tribe. Then the alpha
howls, and they all charge together—an unstoppable wave.

### Artificer Order: Scrap-Draw Chain

**Unique Rule:** When an Artificer card scraps from your hand, you
immediately draw a card to replace it.

This makes deck-thinning card-neutral:
- You lose a card from hand (the scrapped card)
- You gain a card from deck (the draw)
- Net result: Same hand size, but your deck is now smaller and better

**Cards with Scrap-Draw:**
- Tinker (ally effect): Scrap from hand → Draw 1
- Arcane Workshop (base effect): Scrap from hand → Draw 1

**Note:** Scrapping from *discard pile* (like Recycler) does NOT trigger
a draw, since you're not losing a card from hand.

**Design Intent:**
Artificers should feel rewarded for deck-thinning, not punished. Without
the draw, scrapping from hand feels bad (you lose card advantage). With
the draw, it feels like transmutation—bad card becomes good card.

**Implementation:**
Effects use `requires_previous: true` to indicate the draw only happens
if the scrap was executed:
```json
"effects": [
  { "type": "scrap_hand_or_discard", "target": "hand", "optional": true },
  { "type": "draw_card", "value": 1, "requires_previous": true }
]
```

### Artificer Order: Permanent Upgrades

The Artificer Order specializes in permanent upgrades:
- Blacksmith: +1 Attack to a card in discard
- Enchanter: +1 Authority to a card in discard
- Goldweaver: +1 Trade to a card in discard
- Master Artificer: Choose any upgrade type

This makes Artificer Order uniquely valuable for long games.

## Victory Conditions

- Reduce opponent's Authority to 0
- (Future: alternative victory conditions per scenario)

## Multiplayer Considerations

### Hidden Information
- Hand contents hidden until played
- Deck order hidden (only count visible)
- Opponent sees: authority, d10/d4, base count, hand count

### Connection Modes
- **Browser:** Full visual experience with dynamic art
- **Terminal (SSH):** Text-only, same mechanics, no images
- **Spectator:** View both hands, all information

### Turn Timing
- Configurable per-session
- Default: no time limit
- Tournament: 60 seconds per turn
