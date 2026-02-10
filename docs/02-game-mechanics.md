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

### Artificer Order Special: Upgrades

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
