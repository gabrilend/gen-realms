# 2-001: Card Template Format

## Current Behavior
Card data structure exists in code but no external card definition format.

## Intended Behavior
A standardized file format for defining cards that:
- Uses Lua tables (or JSON) for card definitions
- Supports all card properties from the card data structure
- Is human-readable and editable
- Can be validated against schema
- Supports comments/documentation within card files
- Organizes cards by faction/type

## Suggested Implementation Steps

1. Create `assets/cards/` directory structure:
   ```
   assets/cards/
     schema.lua
     starting/
     merchant/
     wilds/
     kingdom/
     artificer/
     neutral/
   ```
2. Define card schema in `schema.lua`:
   ```lua
   return {
     required = {"id", "name", "cost", "faction", "card_type", "effects"},
     optional = {"ally_effects", "scrap_effects", "defense", "is_outpost", "spawns", "art_ref", "flavor"}
   }
   ```
3. Create card loader in `src/card-loader.lua`
4. Implement `CardLoader.load_faction(faction_name)` - load all cards from folder
5. Implement `CardLoader.load_all()` - load complete card database
6. Implement `CardLoader.validate(card_data)` - check against schema
7. Create example card file with documentation
8. Write tests for load and validation

## Related Documents
- 1-001-card-data-structure.md
- docs/01-architecture-overview.md

## Dependencies
- 1-001: Card Data Structure (defines what cards contain)

## Acceptance Criteria
- [ ] Card files can be created as Lua tables
- [ ] Loader correctly reads all card files
- [ ] Invalid cards fail validation with clear error
- [ ] Example card demonstrates all properties
- [ ] Card database accessible at runtime
