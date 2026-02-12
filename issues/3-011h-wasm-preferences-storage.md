# 3-011h: WASM Preferences Storage

## Current Behavior

User preferences stored via JavaScript (`preferences.js`, `preferences-ui.js`).
Uses browser localStorage for persistence. Preferences UI is a DOM modal.

## Intended Behavior

Preferences storage and UI implemented in C. localStorage accessed via EM_ASM.
Preferences UI renders in canvas, not DOM.

## Dependencies

- 3-011a: Core Canvas Infrastructure (for UI rendering)
- 3-011c: Input Handling (for UI interaction)

## Suggested Implementation Steps

1. Create `src/client/wasm/preferences.h` with:
   - DisplayPrefs structure (animation speed, tooltips, contrast, text size)
   - AudioPrefs structure (volume levels, mute)
   - GameplayPrefs structure (auto end turn, confirmations, card size)
   - AIPrefs structure (narrative, hints, opponent, difficulty)
   - KeybindPrefs structure (hotkey mappings)
   - UserPreferences combining all categories

2. Create `src/client/wasm/preferences.c` implementing:
   - `prefs_init()` - Load from localStorage via EM_ASM
   - `prefs_cleanup()` - Save on cleanup
   - `prefs_get()` / `prefs_get_mutable()` - Access preferences
   - `prefs_save()` - Store to localStorage via EM_ASM
   - `prefs_reset()` - Reset to defaults
   - `prefs_export_json()` - Export for backup
   - `prefs_import_json()` - Import from backup

## Files Created

- `src/client/wasm/preferences.h` - Preferences structures
- `src/client/wasm/preferences.c` - Preferences implementation

## JS Files Replaced

- `assets/web/preferences.js`
- `assets/web/preferences-ui.js` (partial - UI rendering later)

## Acceptance Criteria

- [x] Preferences load from localStorage on init
- [x] Preferences save to localStorage
- [x] Default values for first launch
- [x] Reset to defaults function
- [x] Export to JSON string
- [x] Import from JSON string
- [x] All preference categories supported

## Implementation Notes

localStorage access via EM_ASM:
```c
EM_ASM({
    localStorage.setItem(key, JSON.stringify(prefs));
});
```

Preferences saved as JSON object with nested categories.
Version field for future schema migrations.

Note: Preferences UI panel rendering deferred to later sub-issue or
separate implementation. This sub-issue focuses on storage/persistence.
