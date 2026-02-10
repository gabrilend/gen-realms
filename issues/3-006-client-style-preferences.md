# 3-006: Client Style Preferences

## Current Behavior
No client-side preference storage exists.

## Intended Behavior
A localStorage-based preference system that:
- Stores user style guide for image generation
- Stores UI preferences (colors, layout)
- Persists across sessions
- Provides defaults for new users
- Exports/imports for sharing

## Suggested Implementation Steps

1. Create `assets/web/preferences.js`
2. Define preference schema:
   ```javascript
   const defaultPrefs = {
       styleGuide: "dark fantasy, oil painting, dramatic lighting",
       negativePrompts: "cartoon, anime, bright colors",
       cardFrameStyle: "ornate gold",
       animationSpeed: 1.0,
       narrativeFont: "serif",
       showUpgradeBadges: true
   };
   ```
3. Implement `loadPreferences()` from localStorage
4. Implement `savePreferences(prefs)`
5. Create preferences UI panel:
   - Text area for style guide
   - Text area for negative prompts
   - Dropdown for card frame style
   - Slider for animation speed
6. Implement `exportPreferences()` → JSON download
7. Implement `importPreferences(file)` → JSON upload
8. Apply preferences to rendering
9. Send style guide with image requests (Phase 6)

## Related Documents
- docs/04-architecture-c-server.md (client preferences)
- 3-004-browser-canvas-renderer.md

## Dependencies
- 3-004: Browser Canvas Renderer

## Preferences UI

```html
<div id="preferences-panel">
  <h3>Style Preferences</h3>

  <label>Art Style Guide:</label>
  <textarea id="style-guide">dark fantasy, oil painting...</textarea>

  <label>Negative Prompts:</label>
  <textarea id="negative-prompts">cartoon, anime...</textarea>

  <label>Card Frame:</label>
  <select id="card-frame">
    <option value="ornate-gold">Ornate Gold</option>
    <option value="simple-black">Simple Black</option>
    <option value="faction-themed">Faction Themed</option>
  </select>

  <label>Animation Speed:</label>
  <input type="range" id="anim-speed" min="0.5" max="2.0" step="0.1">

  <button onclick="savePrefs()">Save</button>
  <button onclick="exportPrefs()">Export</button>
  <button onclick="importPrefs()">Import</button>
</div>
```

## Acceptance Criteria
- [ ] Preferences load from localStorage
- [ ] Preferences save correctly
- [ ] Default values work for new users
- [ ] UI allows editing all preferences
- [ ] Export/import works
- [ ] Style guide available for Phase 6
