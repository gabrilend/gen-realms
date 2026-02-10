# 3-004: Browser Canvas Renderer

## Current Behavior
No browser rendering exists.

## Intended Behavior
An HTML5 Canvas renderer for browser clients that:
- Renders game state visually
- Shows cards with placeholder art (until Phase 6)
- Displays player info, trade row, bases
- Shows narrative text panel
- Supports responsive layout
- Provides click targets for interaction

## Sub-Issues

This issue has been split into the following sub-issues:

| ID | Description | Status |
|----|-------------|--------|
| 3-004a | Canvas Infrastructure | pending |
| 3-004b | Card Rendering | pending |
| 3-004c | Game Zone Rendering | pending |
| 3-004d | Status and Narrative Panels | pending |

## Implementation Order

1. **3-004a** first - canvas setup and layout system
2. **3-004b** second - card rendering primitives
3. **3-004c** third - game zones use card rendering
4. **3-004d** last - UI panels complete the display

## Related Documents
- docs/04-architecture-c-server.md
- 3-003-wasm-build-configuration.md

## Dependencies
- 3-003: Wasm Build Configuration
- Modern browser with Canvas support

## Canvas Layout

```
┌────────────────────────────────────────────────────────────────┐
│ STATUS: Turn 12 | Authority 42 | d10:7 | Trade:3 Combat:5     │
├────────────────────────────────┬───────────────────────────────┤
│                                │                               │
│     YOUR BASES                 │      TRADE ROW                │
│     ┌──────┐                   │      ┌──────┐ ┌──────┐        │
│     │ Base │                   │      │Card1 │ │Card2 │ ...    │
│     └──────┘                   │      └──────┘ └──────┘        │
│                                │                               │
├────────────────────────────────┼───────────────────────────────┤
│                                │                               │
│     YOUR HAND                  │      NARRATIVE                │
│     ┌──────┐ ┌──────┐ ┌──────┐ │      The dire bear emerges   │
│     │Card1 │ │Card2 │ │Card3 │ │      from the Thornwood...   │
│     └──────┘ └──────┘ └──────┘ │                               │
│                                │                               │
└────────────────────────────────┴───────────────────────────────┘
```

## Acceptance Criteria
- [ ] Canvas renders game state
- [ ] Cards display with faction colors
- [ ] Narrative panel shows text
- [ ] Layout adapts to window size
- [ ] Click regions defined for interaction
