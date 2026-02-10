# 6-010: Phase 6 Demo

## Current Behavior
Phase 5 demo exists with LLM narrative but no visuals.

## Intended Behavior
A demonstration showing the complete visual generation system:
- ComfyUI integration working
- Card art generated dynamically
- Battle canvas with progressive inpainting
- Style-consistent faction visuals
- Image caching reducing generation time

## Suggested Implementation Steps

1. Create `run-phase6-demo.sh` in project root
2. Configure ComfyUI endpoint (local server)
3. Run game with visual generation enabled
4. Display generated card art
5. Show battle canvas building progressively
6. Export final battle scene
7. Display cache statistics
8. Verify style consistency

## Related Documents
- All Phase 6 issues (6-001 through 6-009)
- 5-010-phase-5-demo.md

## Dependencies
- All previous Phase 6 issues (6-001 through 6-009)
- Phase 5 complete (LLM narrative)
- ComfyUI running locally

## Demo Output

```
=== SYMBELINE REALMS: PHASE 6 DEMO ===

Connecting to ComfyUI: http://localhost:8188
Model: sd_xl_base_1.0

--- INITIALIZATION ---

Generating card art...
[1/5] Dire Bear (Wilds) - 3.2s
[2/5] Trading Post (Merchant) - 2.8s
[3/5] Knight Commander (Kingdom) - 3.1s
[4/5] Construct Sentinel (Artificer) - 2.9s
[5/5] Wandering Merchant (Neutral) - 2.7s

Card art generation complete.

--- GAME START ---

Building battle canvas (512x512)...

[CANVAS] Generating SKY region...
  Prompt: "Fantasy battlefield sky, dramatic clouds,
           two armies facing, medieval, magical, painterly"
  Time: 4.2s
  Status: Complete ✓

[CANVAS] Generating P1_BASE region...
  Prompt: "Forest stronghold, wooden palisade,
           primal totems, Wilds faction, fantasy art"
  Time: 3.8s
  Status: Complete ✓

--- TURN 3 ---

Lady Morgaine plays: Dire Bear

[CANVAS] Generating P1_FORCES region...
  Prompt: "Massive dire bear, forest creature,
           primal rage, charging left-to-right,
           fantasy card art, Wilds faction"
  Time: 3.5s
  Status: Complete ✓

[UPGRADE] Ally bonus active - applying golden glow overlay

--- TURN 7 ---

Attack for 5 damage!

[CANVAS] Generating CENTER region...
  Prompt: "Battle clash, bear attacking knight,
           magical energy, combat action, dynamic pose"
  Time: 4.1s
  Status: Complete ✓

--- GAME OVER ---

Lady Morgaine wins!

[CANVAS] Exporting final battle scene...
  Path: output/images/game_20240115/frame_final.png
  Size: 512x512
  Status: Exported ✓

=== STATISTICS ===

ComfyUI Calls: 12
Cache Hits: 3 (25%)
Average Generation Time: 3.4s
Total Canvas Regions: 6
Style Consistency: High (all fantasy)
Final Image Size: 847 KB

Opening battle scene in viewer...

Demo complete.
```

## Visual Output

The demo should produce:
1. Individual card art images (256x256 each)
2. Progressive canvas snapshots (frame_001.png, frame_002.png, ...)
3. Final composite battle scene (512x512)
4. Cache directory with reusable generations

## Acceptance Criteria
- [ ] ComfyUI generates images successfully
- [ ] Card art matches faction styles
- [ ] Battle canvas builds progressively
- [ ] Cache reduces redundant generation
- [ ] Final scene exports correctly
- [ ] Demo runs to completion
