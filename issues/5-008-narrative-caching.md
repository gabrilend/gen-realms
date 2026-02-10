# 3-008: Narrative Display Formatting

## Current Behavior
Game displays only mechanical information.

## Intended Behavior
Present narrative text alongside game state:
- Story text appears in dedicated display area
- Mechanical actions also shown for clarity
- Text scrolls or pages for long narratives
- Color/styling distinguishes narrative from UI
- Option to toggle narrative verbosity

## Suggested Implementation Steps

1. Create `src/display/narrative.lua`
2. Define display layout:
   ```
   +------------------+------------------+
   |   Game State     |   Narrative      |
   |                  |                  |
   |  Hand: [cards]   |  "The dire bear  |
   |  Trade Row: ...  |   roars..."      |
   |                  |                  |
   |  Actions: ...    |  [more story...] |
   +------------------+------------------+
   ```
3. Implement `Narrative.add_text(text)` - queue narrative
4. Implement `Narrative.render(width, height)` - format for display
5. Implement text wrapping for long lines
6. Add scrollback buffer for history
7. Add color codes for different speakers/events
8. Implement verbosity levels (full, summary, off)
9. Integrate with main TUI renderer
10. Write tests for formatting edge cases

## Related Documents
- 3-007-event-hook-system.md
- 1-012-phase-1-demo.md

## Dependencies
- 3-007: Event Hook System (provides narrative text)

## Display Example

```
╔══════════════════════════════════════════════════════════════════╗
║  Symbeline Realms                              Turn 12           ║
╠═══════════════════════════════╦══════════════════════════════════╣
║  Lady Morgaine                ║                                  ║
║  Authority: 28    d10: 7      ║  From the depths of the          ║
║                               ║  Thornwood, Lady Morgaine        ║
║  Hand:                        ║  summons a dire bear of          ║
║  [1] Dire Bear     +5C        ║  ancient lineage. Its roar       ║
║  [2] Wolf Scout    +2C        ║  echoes across the battlefield.  ║
║  [3] Village Scout +1T        ║                                  ║
║                               ║  Lord Theron's merchants         ║
║  Trade: 1  Combat: 7          ║  scramble as the beast charges.  ║
╠═══════════════════════════════╩══════════════════════════════════╣
║  [P]lay [B]uy [A]ttack [E]nd turn                    [N] Narr.   ║
╚══════════════════════════════════════════════════════════════════╝
```

## Acceptance Criteria
- [ ] Narrative displays alongside game state
- [ ] Text wraps correctly
- [ ] Scrollback history accessible
- [ ] Verbosity levels work
- [ ] Display is visually clear
