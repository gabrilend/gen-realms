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
| Trade Federation | Merchant Guilds | Trade, Kingdom Coin, Forecast |
| Blob | The Wilds | Combat, Frontier, Pack Charge |
| Star Empire | High Kingdom | Authority, Recruit (March Column) |
| Machine Cult | Artificer Order | Scrap, Upgrades |

## Faction-Specific Mechanics

### Merchant Guilds: Kingdom Coin & Forecast

#### Kingdom Coin

A secondary currency used by the Merchant Guilds. Unlike trade (which
resets each turn), Kingdom Coin persists between turns.

**The Coin Itself:**
- Stamped with the castle seal
- Made from a common metal with very high melting point (tungsten alloy)
- Too hot to de-mint, too common to covet—it just... is
- Used for inter-guild trade and hiring mercenaries

**Generating Coin:**
| Card | Cost | Coin Generated |
|------|------|----------------|
| Royal Coin Mint | 3 | +1 (ally: +1 more) |
| Exchange House | 5 | +1 |
| Guild Observatory | 7 | +2 |

**Spending Coin (Requires Merchant Ally):**
Coins persist between turns, but spending them requires a Merchant ally
in play. Various Merchant cards have `spend_coin` ally effects that let
you return coins for bonuses.

| Card | Ally Effect | Coin Cost | Reward |
|------|-------------|-----------|--------|
| Guild Courier | spend_coin | 1 | +1 Combat |
| Guild Factor | spend_coin | 2 | Spawn Coin Guard |
| Gold Mage | spend_coin | 3 | Spawn Coin Scout |
| Merchant Prince | spend_coin | 5 | Spawn Coin Captain |

**Coin Troops (Spawned Units):**
| Unit | Effects | Notes |
|------|---------|-------|
| Coin Guard | Draw 1, +1 Combat | Spawned for 2 coin |
| Coin Scout | Draw 1, +1 Trade | Spawned for 3 coin |
| Coin Captain | Draw 1, +2 Combat, +1 Authority | Spawned for 5 coin |

**Auto-Draw (Pre-Turn Resolution):**
All Coin Troops have `auto_draw: true`. Their draw effects resolve at
the very start of your turn, before you choose draw order. This creates
a "deck acceleration" effect—you see more cards before deciding.

**Design Intent:**
The Survey Ship from Star Realms (cost 3, draw 1, +1 trade) creates
permanent deck improvement. Coin Troops do the same but require:
1. Merchant infrastructure to generate coins (Coin Mint)
2. Merchant allies to spend coins (ally effects)
Small effects that compound over time—the merchant way. The coin economy
rewards building a dedicated Merchant deck.

#### Forecast

High-level Merchant bases reveal your actual hand size for next turn.
Not which cards—just the count.

**How It Works:**
If you normally draw 5 cards, but the first card has "draw a card",
you'll actually draw 6. Forecast shows "6". It follows the draw chain
to its conclusion and tells you the final count.

**Example:**
```
Normal draw: 5 cards
Card 1 has "draw 1" → now drawing 6
Card 6 has "draw 1" → now drawing 7
Forecast shows: "7"
```

**Forecast Bases:**
| Base | Cost | Effect |
|------|------|--------|
| Exchange House | 5 | Shows next hand size |
| Guild Observatory | 7 | Shows next hand size |

**Strategic Value:**
- Know exactly how many cards you'll see next turn
- Plan purchases: "I'll have 7 cards, so I can afford to spend big now"
- Decide when to charge (Wilds) or recruit (Kingdom)
- Simple, actionable number—not probabilities

**Design Intent:**
Merchants trade in certainty. The forecast isn't a guess—it's a ledger
entry for tomorrow. Knowing "7 cards next turn" lets you commit fully
this turn, confident in what's coming.

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

#### 4. Wilds Base Utilities

Wilds bases have low defense (3-5) but provide unique utility effects:

**Spawn Per Ally (`spawn_per_ally: true`):**
When a base has this flag, it spawns one unit per Wilds ally in play,
not just one unit. This makes ally-heavy builds extremely productive.
- Claw and Tooth Mill: Spawns Hardwood Fang tokens per ally
- Elder's Hollow: Spawns Wolf Tokens per ally

**Frontier Bonus (`frontier_bonus`):**
Some bases grant bonuses to cards that charge from their frontier:
- Moss Garden Grove: Cards charging from here gain +1 Attack permanently
- Bone Circle: Draw 1 card when any card charges

**On Destroyed Triggers (`on_destroyed`):**
Certain bases have effects when they're destroyed:
- Bramble Thicket: When destroyed, triggers a charge (all frontier cards)

**Base Variety:**
| Base | Cost | Def | Type | Special Effect |
|------|------|-----|------|----------------|
| Moonlit Den | 2 | 3 | Outpost | Cheap protection |
| Moss Garden Grove | 3 | 3 | Base | Frontier bonus: +1 Attack on charge |
| Crystal Hollow | 3 | 3 | Base | +1 Trade, stacking with allies |
| Bramble Thicket | 3 | 4 | Outpost | Trigger charge on destruction |
| Honey Cache | 2 | 3 | Base | +2 Authority, +1 per ally |
| Claw and Tooth Mill | 4 | 4 | Base | Spawn tokens per ally |
| Bone Circle | 4 | 4 | Base | Draw 1 on charge |
| Elder's Hollow | 5 | 5 | Base | Draw 1, spawn wolves per ally |

**Strategic Value:**
- Bases are common in Wilds decks, making beast cards rarer to draw
- Lower defense encourages frontier stacking for protection
- Utility effects reward long-game base strategies
- Mix of outposts (must attack) and regular bases (can ignore)

### High Kingdom: Recruit (The March Column)

**Unique Effect:** `recruit` - Send a card from hand to the back of
your deck (the march column), then draw from the front.

The Kingdom's army marches in an endless 2x2 mile column of sworn
countrymen. When you recruit, you're sending a soldier to the back
of the line to gather more forces—peasants filling armor, squires
seeking knights, heralds spreading the call. They'll catch up when
the column cycles (deck reshuffles).

**Strategic Value:**
- Filter out bad draws without losing the card permanently
- Smooth your draws while keeping options for later
- Build momentum: front of the column is your best troops
- Endurance playstyle: outlast opponents through superior logistics

**Cards with Recruit:**
| Card | Cost | When |
|------|------|------|
| Royal Herald | 1 | Main effect (optional) |
| Watchtower | 2 | Main effect (optional) |
| Battle Priest | 2 | Ally effect (optional) |
| Blessed Guardian | 5 | Ally effect (optional) |

**Thematic Note:**
Ploughshares to ploughs—when soldiers choose peace, they're building.
When peaceful folk choose soldiery, they intend to capitalize on it.
The recruited card isn't "buried" or discarded—it's sent to gather
reinforcements, and will return stronger when the column cycles.

**Design Intent:**
The High Kingdom fights through discipline and logistics. While other
factions burn cards permanently (Artificer) or overwhelm with pack
tactics (Wilds), Kingdom players manage the march column—always
ensuring the best troops are at the front, the rest gathering strength.

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
