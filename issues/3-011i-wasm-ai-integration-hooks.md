# 3-011i: WASM AI Integration Hooks

## Current Behavior

AI integration (narrative generation, hints, opponent AI) handled by JavaScript
modules calling server API. Streaming narrative responses update DOM directly.

## Intended Behavior

AI integration hooks in C that communicate with server via WebSocket. Callbacks
for narrative streaming, hints, and opponent actions. All rendering through
canvas, not DOM.

## Dependencies

- 3-011g: WebSocket Communication (provides ws_send, message handling)

## Suggested Implementation Steps

1. Create `src/client/wasm/ai-hooks.h` with:
   - AIRequestType enum (narrative, hint, opponent_action, card_description)
   - AIResponseStatus enum (pending, streaming, complete, error, cancelled)
   - NarrativeContext structure (event, card, faction, value, target)
   - AIHint structure (action, target, reason, confidence)
   - AIOpponentAction structure (action, target, reasoning, think_time)
   - Callback types for responses

2. Create `src/client/wasm/ai-hooks.c` implementing:
   - `ai_init()` - Initialize with callbacks
   - `ai_request_narrative()` - Request narrative for event
   - `ai_request_hint()` - Request play suggestion
   - `ai_request_opponent_action()` - Get AI opponent's action
   - `ai_cancel_request()` / `ai_cancel_all()` - Cancel pending
   - `ai_get_status()` - Check request status
   - `ai_is_busy()` - Any requests pending
   - `ai_set_enabled()` - Toggle AI features
   - Response handlers for each type

## Files Created

- `src/client/wasm/ai-hooks.h` - AI integration API
- `src/client/wasm/ai-hooks.c` - AI integration implementation

## JS Files Replaced

- AI portions of various JS files
- `assets/web/art-tracker.js` (partial - core hooks)
- `assets/web/generation-queue.js` (partial - core hooks)

## Acceptance Criteria

- [x] Narrative requests sent to server
- [x] Streaming narrative updates handled
- [x] Hint requests supported
- [x] Opponent action requests supported
- [x] Request cancellation works
- [x] Status tracking for pending requests
- [x] Feature toggles (enable/disable each AI type)

## Implementation Notes

AI requests sent as JSON over WebSocket:
```json
{
  "type": "ai_request",
  "request_type": "narrative",
  "request_id": 42,
  "context": { ... }
}
```

Streaming responses include delta flag:
```json
{
  "type": "ai_response",
  "request_id": 42,
  "text": "...",
  "delta": true,
  "complete": false
}
```

Note: Image generation (art-tracker, generation-queue, image-cache) planned
for server-side with browser just displaying received images. Simplifies
WASM client significantly.
