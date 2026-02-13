# 4-003: Faction Design - The Wilds

## Current Behavior
No faction cards exist.

## Intended Behavior
Design and implement The Wilds faction with:
- Theme: Nature's fury, beasts, growth, overwhelming numbers
- Primary mechanic: Combat power, swarming
- Secondary mechanic: Card draw (pack hunting)
- Ally bonus focus: Combat multipliers
- Fantasy flavor: Dire beasts, forest spirits, druids, primal magic

## Suggested Implementation Steps

1. Design 8-12 faction cards with cost curve (1-7 trade):
   ```
   Low cost (1-2): Wolf scouts, forest sprites
   Mid cost (3-4): Dire beasts, pack leaders
   High cost (5-7): Ancient treants, primal titans
   ```
2. Design 2-3 faction bases:
   - Sacred grove (spawns beast tokens)
   - Thornwall (defensive outpost)
   - Heart of the Forest (massive ally bonuses)
3. Create card files in `assets/cards/wilds/`
4. Include spawning bases (grove spawns wolves)
5. Emphasize combat through ally chains
6. Write flavor text reflecting primal nature
7. Document faction strategy guide

## Related Documents
- docs/02-game-mechanics.md
- 4-001-card-json-schema.md

## Dependencies
- 4-001: Card JSON Schema

## Acceptance Criteria
- [x] 8-12 ship cards created
- [x] 2-3 base cards created
- [x] Strong combat focus evident
- [x] Spawning mechanic utilized
- [x] Ally chains feel impactful

## Completion Notes (2026-02-10)

**Status: COMPLETED**

### Cards Created (15 total)

**Ships (11):**
| Card | Cost | Effects | Ally Effects |
|------|------|---------|--------------|
| Wolf Scout | 1 | +2 Combat | +2 Combat |
| Forest Sprite | 1 | +1 Combat | Draw 1 |
| Pack Hunter | 2 | +2 Combat | +1 Combat, Draw 1 |
| Beastcaller | 3 | +2 Combat | +3 Combat, Draw 1 |
| Thornback Boar | 3 | +4 Combat | - (Scrap: +3 Combat) |
| Swarm of Crows | 3 | +3 Combat | +2 Combat |
| Dire Bear | 4 | +5 Combat | Draw 1 |
| Forest Shaman | 4 | +2 Combat, Draw 1 | +3 Combat |
| Alpha Pack Leader | 5 | +4 Combat, Draw 1 | +4 Combat |
| Ancient Treant | 5 | +6 Combat | +2 Combat, +2 Authority |
| Primal Titan | 7 | +8 Combat, Draw 1 | +4 Combat |

**Bases (3):**
| Card | Cost | Defense | Effects |
|------|------|---------|---------|
| Thornwall | 3 | 4 (Outpost) | +2 Combat (Ally: +1 Combat) |
| Sacred Grove | 4 | 5 | +1 Combat, Spawns Wolf Token |
| Heart of the Forest | 6 | 7 | +2 Combat (Ally: +4 Combat, Draw 1) |

**Token (1):**
- Spirit Wolf: +2 Combat (Scrap: Draw 1)

### Addendum 1 (2026-02-10)
Added **Beastcaller** (Cost 3) to bring faction to 15 cards total.

### Addendum 2 (2026-02-10) - Frontier Mechanic & New Bases

**Major faction redesign implemented:**

1. **Effect Split**: Main effects now combat/draw only; ally effects resources/heal only
2. **Frontier Mechanic**: Cards can be played to frontier (stacked on bases) to increase base defense
3. **Frontier Leaders**: High-cost Wilds ships trigger all frontier cards to charge into play
4. **New Schema Fields**: `spawn_per_ally`, `frontier_bonus`, `on_destroyed`

**New Bases Added (8 total, replacing original 3):**

| Base | Cost | Def | Type | Special |
|------|------|-----|------|---------|
| Moonlit Den | 2 | 3 | Outpost | Cheap protection |
| Honey Cache | 2 | 3 | Base | +2 Authority, +1/ally |
| Moss Garden Grove | 3 | 3 | Base | Frontier bonus: +1 Attack on charge |
| Crystal Hollow | 3 | 3 | Base | +1 Trade, +1/ally |
| Bramble Thicket | 3 | 4 | Outpost | Triggers charge when destroyed |
| Bone Circle | 4 | 4 | Base | Draw on charge |
| Claw and Tooth Mill | 4 | 4 | Base | Spawns Hardwood Fang per ally |
| Elder's Hollow | 5 | 5 | Base | Draw 1, spawns wolves per ally |

**New Token:**
- Hardwood Fang: +1 Combat (Scrap: +2 Combat)

**Frontier Leaders (marked with `frontier_leader: true`):**
- Beastcaller (cost 3)
- Forest Shaman (cost 4)
- Alpha Pack Leader (cost 5)
- Primal Titan (cost 7)

**Design Intent:**
- More bases means beasts are rarer to draw (valuable)
- Low base defense (3-5) encourages frontier stacking
- Utility effects reward strategic base acquisition
- Pack scaling creates n² ally synergies (1,4,9,16 power curve)

### Addendum 3 (2026-02-12) - Summoner Ships & Temporary Bases

**Major restructure: bases converted to summoner ship + temporary base pairs**

All Wilds bases are now **summoner ships** that spawn **temporary bases**:
- Ships cycle through deck (replayable each shuffle)
- Temporary bases have `temporary_base: true` and are **scrapped** when destroyed
- This creates wave tactics: build bases, charge, lose them, rebuild

**Summoner Ship → Temporary Base Pairs (11 total):**

| Summoner Ship | Cost | Effects | Temporary Base | Def | Type |
|---------------|------|---------|----------------|-----|------|
| Den Mother | 2 | +1 Combat, spawn | Moonlit Den | 3 | Outpost |
| Honey Keeper | 2 | +1 Authority, spawn | Honey Cache | 3 | Base |
| Moss Weaver | 3 | +1 Trade, spawn | Moss Garden | 3 | Base |
| Crystal Singer | 3 | +1 Trade, spawn | Crystal Hollow | 3 | Base |
| Thornwall Summoner | 3 | +1 Combat, spawn | Thornwall | 4 | Outpost |
| Bramble Shaper | 3 | +1 Combat, spawn | Bramble Thicket | 4 | Outpost |
| Bone Speaker | 4 | Draw 1, spawn | Bone Circle | 4 | Base |
| Mill Master | 4 | +1 Combat, spawn | Claw and Tooth Mill | 4 | Base |
| Grove Keeper | 4 | +1 Combat, spawn | Sacred Grove | 5 | Base |
| Elder Summoner | 5 | Draw 1, spawn | Elder's Hollow | 5 | Base |
| Heart Caller | 6 | +2 Combat, Draw 1, spawn | Heart of the Forest | 7 | Base |

**New Schema Field:**
```json
"temporary_base": {
  "type": "boolean",
  "default": false,
  "description": "WILDS ONLY: Base is scrapped when destroyed instead of going to discard."
}
```

**Updated Card Count:**
- 11 regular ships (unchanged)
- 11 summoner ships (converted from bases)
- 11 temporary base tokens (new)
- 2 unit tokens (wolf_token, hardwood_fang)
- **Total: 35 Wilds cards**

**Design Intent:**
- Traditional bases clog your deck forever once acquired
- Summoner ships cycle, allowing repeated base deployment
- Temporary bases scrap when destroyed—no discard pile clutter
- Creates "wave attack" playstyle: raise defenses, charge, rebuild
- Opponent must decide: attack bases (scraps them) or ignore (lets them persist)
