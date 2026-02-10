# 3-009: Narrative Display

## Current Behavior
No dedicated narrative display in clients.

## Intended Behavior
A scrollable narrative panel that:
- Shows LLM-generated story text
- Maintains history for scrollback
- Auto-scrolls on new text
- Supports text styling (emphasis, speaker)
- Works in both terminal and browser

## Suggested Implementation Steps

1. **Terminal version** in `src/client/01-terminal.c`:
   - Dedicate right panel to narrative
   - Implement scroll with up/down keys
   - Store last N narrative entries
   - Word wrap for panel width

2. **Browser version** in `assets/web/narrative.js`:
   - Create scrollable div overlay
   - Style with serif font (fantasy feel)
   - Parse simple markup (bold, italic)
   - Auto-scroll to bottom on new entry
   ```javascript
   function addNarrative(text) {
       const entry = document.createElement('p');
       entry.innerHTML = parseMarkup(text);
       narrativePanel.appendChild(entry);
       narrativePanel.scrollTop = narrativePanel.scrollHeight;
   }
   ```

3. Store narrative history (last 50 entries)
4. Add toggle to show/hide narrative panel
5. Add copy-to-clipboard for sharing story
6. Style different event types (attack, purchase, etc.)

## Related Documents
- docs/04-architecture-c-server.md
- 3-001-terminal-renderer.md
- 3-004-browser-canvas-renderer.md

## Dependencies
- 3-001: Terminal Renderer (for terminal version)
- 3-004: Browser Canvas Renderer (for browser version)
- 2-005: Protocol (narrative messages)

## Narrative Styling

```css
.narrative-panel {
    font-family: 'Garamond', serif;
    background: rgba(20, 15, 10, 0.9);
    color: #d4c4a8;
    padding: 1em;
    overflow-y: auto;
}

.narrative-entry {
    margin-bottom: 0.5em;
    line-height: 1.4;
}

.narrative-entry.attack {
    color: #ff6b6b;
}

.narrative-entry.purchase {
    color: #ffd93d;
}
```

## Acceptance Criteria
- [ ] Terminal narrative panel works
- [ ] Browser narrative panel scrolls
- [ ] History preserved for scrollback
- [ ] New text auto-scrolls
- [ ] Styling distinguishes event types
- [ ] Toggle to hide/show works
