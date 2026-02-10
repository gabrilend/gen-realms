# 3-010: Phase 3 Demo

## Current Behavior
Phase 2 demo has complete cards but no narrative.

## Intended Behavior
A demo that showcases the LLM Dungeon Master:
- Full narrative experience during gameplay
- DM-influenced trade row selections
- Story text for all major events
- Narrative history scrollback
- Game feels like a narrated story

## Suggested Implementation Steps

1. Update `issues/completed/demos/phase-3-demo.lua`
2. Integrate LLM client with game loop
3. Enable trade row DM selection
4. Enable event-driven narrative generation
5. Display narrative panel alongside game
6. Add narrative toggle (full/summary/off)
7. Add game transcript export
8. Create "story mode" option (slower, more narrative)
9. Include sample API configuration
10. Update run script with API key setup

## Related Documents
- 2-010-phase-2-demo.md
- All Phase 3 issues

## Dependencies
- All Phase 3 issues (3-001 through 3-009)

## Demo Experience

```
╔══════════════════════════════════════════════════════════════════╗
║  Symbeline Realms - Phase 3 Demo                   [DM Active]   ║
╠═══════════════════════════════════════════════════════════════════╣
║                                                                   ║
║  "As dawn breaks over the contested realm, two commanders        ║
║   prepare their forces. Lady Morgaine of the Thornwood           ║
║   surveys her domain, while Lord Theron counts his gold..."      ║
║                                                                   ║
╠═══════════════════════════════════════════════════════════════════╣
║  Your Hand:                    ║  Trade Row (DM Selected):       ║
║  [1] Guild Courier   +2T       ║  [A] Dire Bear (4g)             ║
║  [2] Wolf Scout      +2C       ║  [B] Trading Post (3g)          ║
║  [3] Village Scout   +1T       ║  [C] Knight Commander (4g)      ║
║                                ║  "The markets today favor       ║
║  Trade: 0  Combat: 0           ║   those who seek power..."      ║
╠═══════════════════════════════════════════════════════════════════╣
║  [P]lay [B]uy [A]ttack [E]nd   [S]croll narrative   [T]ranscript ║
╚══════════════════════════════════════════════════════════════════╝
```

## Acceptance Criteria
- [ ] LLM generates narrative during play
- [ ] Trade row influenced by DM
- [ ] Story feels continuous across turns
- [ ] Transcript can be exported
- [ ] Can play without API (cached/fallback)
