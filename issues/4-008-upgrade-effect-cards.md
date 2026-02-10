# 2-009: Card Art Placeholder System

## Current Behavior
No art reference system exists. Cards have no visual representation.

## Intended Behavior
A system that maps cards to visual references for future AI generation:
- Each card has art_ref pointing to reference image
- Placeholder images for development/testing
- Reference image database for inpainting prompts
- ASCII art fallback for TUI display
- Structure supports Phase 4 visual generation

## Suggested Implementation Steps

1. Create `assets/art/` directory structure:
   ```
   assets/art/
     placeholders/
     references/
     ascii/
   ```
2. Create placeholder image (simple colored rectangle per faction)
3. Define art reference schema:
   ```lua
   art_ref = {
     placeholder = "path/to/placeholder.png",
     reference = "path/to/reference.png",  -- for inpainting
     ascii = [[
       /\
      /  \
     /    \
     ]]
   }
   ```
4. Create `src/art-loader.lua` for art resolution
5. Implement `Art.get_display(card, mode)` - return appropriate art
6. Create faction-colored placeholder generator
7. Add ASCII art for starting cards
8. Document art contribution guidelines

## Related Documents
- docs/01-architecture-overview.md (visual generation)
- Phase 4 issues

## Dependencies
- 2-001: Card Template Format (art_ref field)

## Acceptance Criteria
- [ ] All cards have placeholder art mapping
- [ ] ASCII art displays in TUI
- [ ] Art loader resolves correct asset
- [ ] Structure ready for Phase 4 integration
- [ ] Placeholder distinguishes factions visually
