# 5-010: Phase 5 Demo

## Current Behavior
Phase 4 demo exists without LLM integration.

## Intended Behavior
A demonstration showing the complete LLM DM system:
- Narrative generation for all event types
- World state tracking
- Trade row selection by LLM
- Context window management
- Coherence through full game

## Suggested Implementation Steps

1. Create `run-phase5-demo.sh` in project root
2. Configure LLM endpoint (local or API)
3. Run complete game with narrative enabled
4. Display narrative alongside game state
5. Show context window usage
6. Verify coherence throughout game
7. Capture sample narratives

## Related Documents
- All Phase 5 issues (5-001 through 5-009)
- 4-010-phase-4-demo.md

## Dependencies
- All previous Phase 5 issues (5-001 through 5-009)
- Phase 4 complete (card content)

## Demo Output

```
=== SYMBELINE REALMS: PHASE 5 DEMO ===

Connecting to LLM endpoint: http://localhost:11434/v1
Model: llama3

--- GAME START ---

[NARRATIVE] The contested realm of Symbeline stretches
before two rival commanders. Lady Morgaine of the
Thornwood faces Lord Theron of the Golden Fleet.

--- TURN 1 ---
Lady Morgaine draws: Scout, Scout, Viper, Scout, Explorer

[NARRATIVE] Lady Morgaine surveys her modest forces -
scouts and a single viper, ready to begin the conquest.

Playing: Viper (+2 Combat)
[NARRATIVE] A venomous viper slithers forth from the
underbrush, its deadly fangs gleaming.

Trade Row Selection:
- LLM chose: Dire Bear (Wilds) for thematic coherence
[NARRATIVE] Word spreads through the Thornwood - a
legendary dire bear has been sighted near the markets.

--- TURN 12 ---

[WORLD STATE]
- Turn: 12
- Context tokens: 2847 / 4096
- Coherence score: 0.94

Lady Morgaine attacks for 7!
[NARRATIVE] With a thunderous roar, the combined might
of Lady Morgaine's wild host crashes against Lord
Theron's defenses. His authority crumbles by seven
points under the savage assault.

--- GAME OVER ---

Lady Morgaine wins with 12 authority remaining!

[NARRATIVE] As the dust settles over the battlefield of
Symbeline, Lady Morgaine stands triumphant. The beasts
of the Thornwood have proven their might, and a new
era begins under her wild dominion.

=== STATISTICS ===
Total LLM calls: 47
Cache hits: 12 (25.5%)
Average response time: 340ms
Context summarizations: 3
Coherence recoveries: 0

Demo complete.
```

## Acceptance Criteria
- [ ] LLM generates narratives for all events
- [ ] World state tracks correctly
- [ ] Trade row selection uses LLM
- [ ] Context stays within window
- [ ] Narrative coherent through game
- [ ] Demo runs to completion
