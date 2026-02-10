# 4-006: Sci-Fi to Fantasy Style Transfer

## Current Behavior
No style transformation capability.

## Intended Behavior
Prompt engineering to reinterpret sci-fi elements as fantasy:
- Spaceships -> Flying creatures or ships
- Lasers -> Magic bolts
- Space stations -> Floating castles
- Robots -> Golems
- Consistent fantasy aesthetic

## Suggested Implementation Steps

1. Create `src/visual/style-transfer.lua`
2. Define transformation vocabulary:
   ```lua
   local transforms = {
     spaceship = "dragon|flying ship|phoenix",
     laser = "lightning bolt|fire beam|arcane ray",
     robot = "golem|animated armor|construct",
     station = "floating castle|sky fortress|cloud citadel",
     alien = "demon|elemental|forest spirit"
   }
   ```
3. Create negative prompt list:
   ```lua
   local negatives = {
     "science fiction", "spaceship", "robot",
     "laser", "futuristic", "metal", "chrome",
     "space", "planet", "star destroyer"
   }
   ```
4. Implement `Style.transform_prompt(sci_fi_prompt)`
5. Implement `Style.get_negative_prompt()`
6. Implement `Style.get_style_keywords(faction)`
7. Add faction-specific style modifiers
8. Write tests for prompt transformation

## Related Documents
- notes/vision (fantasy reinterpretation)
- 4-004-inpainting-api-integration.md

## Dependencies
- 4-004: Inpainting API Integration

## Transformation Examples

```
Original (Star Realms):
"Trade Federation ship firing missiles at Blob creature"

Transformed (Symbeline):
"Merchant Guild flying galleon launching arcane missiles
 at writhing forest spirit, fantasy painting style,
 medieval, magical, oil painting"

Negative prompt:
"science fiction, spaceship, laser, futuristic, robot,
 chrome, metal plating, space station"
```

## Acceptance Criteria
- [ ] Sci-fi terms replaced with fantasy
- [ ] Consistent fantasy aesthetic
- [ ] Faction styles influence output
- [ ] Negative prompts suppress sci-fi
- [ ] Output looks fantasy, not sci-fi
