# 2-008: Card Balance Validation

## Current Behavior
No balance analysis tools exist.

## Intended Behavior
A utility that analyzes the card database for balance:
- Calculate value-per-cost ratios
- Compare faction power curves
- Identify outliers (over/underpowered cards)
- Check ally bonus distribution
- Analyze scrap value vs play value
- Output balance report

## Suggested Implementation Steps

1. Create `tools/balance-validator.lua`
2. Define value metrics:
   ```lua
   local value_weights = {
     trade = 1.0,
     combat = 1.0,
     authority = 0.5,  -- less valuable than resources
     draw = 2.0,       -- card draw is powerful
   }
   ```
3. Implement `Balance.calculate_card_value(card)` - sum weighted effects
4. Implement `Balance.value_per_cost(card)` - efficiency ratio
5. Implement `Balance.faction_curve(faction)` - power by cost tier
6. Implement `Balance.find_outliers(cards)` - statistical analysis
7. Implement `Balance.generate_report(cards)` - formatted output
8. Add ally bonus analysis
9. Add scrap value analysis
10. Write tests with known imbalanced cards

## Related Documents
- docs/02-game-mechanics.md
- All faction design issues

## Dependencies
- 2-001: Card Template Format
- All faction card definitions

## Example Output

```
=== Balance Report ===

Faction Power Curves:
  Merchant: 1.8 / 2.1 / 2.3 / 2.5
  Wilds:    2.0 / 2.2 / 2.4 / 2.6
  Kingdom:  1.7 / 2.0 / 2.2 / 2.4
  Artificer: 1.9 / 2.1 / 2.3 / 2.5

Outliers (>2 std dev):
  [HIGH] primal_titan: 2.8 value/cost (avg: 2.2)
  [LOW] guild_courier: 1.4 value/cost (avg: 1.8)

Recommendations:
  - Consider reducing primal_titan effects
  - Consider buffing guild_courier or reducing cost
```

## Acceptance Criteria
- [ ] Utility runs on full card database
- [ ] Value calculations are reasonable
- [ ] Outliers correctly identified
- [ ] Report is human-readable
- [ ] Can inform card adjustments
