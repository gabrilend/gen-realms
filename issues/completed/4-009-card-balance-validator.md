# 4-009: Card Balance Validator

## Current Behavior

~~No balance analysis tools exist.~~

**COMPLETED 2026-02-12**: Balance validator implemented in C with full reporting.

## Intended Behavior

A utility that analyzes the card database for balance:
- Calculate value-per-cost ratios
- Compare faction power curves
- Identify outliers (over/underpowered cards)
- Check ally bonus distribution
- Analyze scrap value vs play value
- Output balance report

## Implementation

Created `src/tools/balance-validator.c` with:

**Value Weights:**
```c
ValueWeights w = {
    .trade = 1.0f,
    .combat = 1.0f,
    .authority = 0.5f,      /* Health is less valuable than resources */
    .draw = 2.0f,           /* Card draw is very powerful */
    .discard = 1.5f,        /* Disruption is strong */
    .scrap_trade = 0.5f,    /* Scrapping trade row is situational */
    .scrap_own = 1.0f,      /* Deck thinning is valuable */
    .acquire_free = 2.5f,   /* Free cards are strong */
    .spawn = 1.5f,          /* Spawning creates recurring value */
    .upgrade = 2.0f,        /* Permanent bonuses are very strong */
    .defense = 0.3f,        /* Defense per point */
    .outpost_bonus = 1.0f   /* Bonus for must-attack property */
};
```

**Features:**
- Loads all JSON card files recursively from assets/cards/
- Calculates base, ally, and scrap values separately
- Computes value-per-cost ratios for efficiency comparison
- Generates faction power curves by cost tier
- Statistical outlier detection (>2 std dev from mean)
- Human-readable report with recommendations
- Verbose mode (-v) shows all cards sorted by VPC

## Related Documents
- docs/02-game-mechanics.md
- All faction design issues

## Dependencies
- 4-001: Card JSON Schema (complete)
- All faction card definitions (complete)

## Actual Output

```
╔══════════════════════════════════════════════════════════════════╗
║                    SYMBELINE REALMS                              ║
║                  Card Balance Report                             ║
╚══════════════════════════════════════════════════════════════════╝

=== Summary ===

  Cards Analyzed:      79
  Global Avg VPC:      1.51
  Standard Deviation:  0.57
  VPC Range:           0.40 - 3.80
  High Outliers:       3 (>2.65 VPC)
  Low Outliers:        0 (<0.38 VPC)

=== Faction Summary ===

  Faction       Cards   Avg VPC  Ally Val  Scrap Val
  ------------  -----  --------  --------  ---------
  neutral           6      1.17      0.00      5.20
  merchant         17      1.47     21.33      2.00
  wilds            22      1.52     13.50      0.00
  kingdom          14      1.67     14.10      0.00
  artificer        14      1.54     13.65     10.20

=== High Outliers (Potentially Overpowered) ===

  [HIGH] Guild Courier         Cost: 1  VPC: 3.80  (avg: 1.51)
  [HIGH] Forest Sprite         Cost: 1  VPC: 3.30  (avg: 1.51)
  [HIGH] Royal Herald          Cost: 1  VPC: 3.70  (avg: 1.51)
```

**Analysis Notes:**
- 1-cost cards tend to be very efficient (high VPC) as expected
- Factions are well-balanced with VPC ranging 1.17-1.67
- Kingdom has slightly higher average VPC (1.67)
- Artificer has highest scrap value (10.20 total) as intended
- No underpowered cards detected

## Acceptance Criteria

- [x] Utility runs on full card database (79 cards loaded)
- [x] Value calculations are reasonable (weighted by effect type)
- [x] Outliers correctly identified (3 high outliers found)
- [x] Report is human-readable (formatted tables and sections)
- [x] Can inform card adjustments (recommendations generated)

## Files Created

- `src/tools/balance-validator.h` - Header with types and API
- `src/tools/balance-validator.c` - Implementation with main()
- `bin/balance-validator` - Compiled executable

## Usage

```bash
# Basic report
./bin/balance-validator

# Verbose (show all cards sorted)
./bin/balance-validator -v

# Custom cards directory
./bin/balance-validator /path/to/cards
```
