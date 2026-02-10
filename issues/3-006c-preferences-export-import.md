# 3-006c: Preferences Export/Import

## Current Behavior
No way to share or backup preferences.

## Intended Behavior
Allow users to export and import preference files:
- Export preferences as JSON download
- Import preferences from JSON file
- Validate imported data
- Merge with existing (don't overwrite all)

## Suggested Implementation Steps

1. Implement `exportPreferences()`:
   ```javascript
   // {{{ export
   function exportPreferences() {
       const prefs = loadPreferences();

       // Remove internal fields
       const exportable = { ...prefs };
       delete exportable.version;

       const blob = new Blob(
           [JSON.stringify(exportable, null, 2)],
           { type: 'application/json' }
       );
       const url = URL.createObjectURL(blob);

       const a = document.createElement('a');
       a.href = url;
       a.download = 'symbeline-preferences.json';
       a.click();

       URL.revokeObjectURL(url);
   }
   // }}}
   ```

2. Create file input element:
   ```html
   <!-- {{{ file input -->
   <input type="file" id="pref-import-input"
          accept=".json" style="display: none"
          onchange="handleImport(event)">
   <button onclick="document.getElementById('pref-import-input').click()">
       Import
   </button>
   <!-- }}} -->
   ```

3. Implement `handleImport()`:
   ```javascript
   // {{{ handle import
   function handleImport(event) {
       const file = event.target.files[0];
       if (!file) return;

       const reader = new FileReader();
       reader.onload = function(e) {
           try {
               const imported = JSON.parse(e.target.result);
               const validated = validateImportedPrefs(imported);
               if (validated) {
                   const current = loadPreferences();
                   const merged = { ...current, ...validated };
                   savePreferences(merged);
                   applyPreferences(merged);
                   populatePrefsUI(merged);
                   showMessage('Preferences imported successfully');
               }
           } catch (err) {
               showError('Invalid preferences file: ' + err.message);
           }
       };
       reader.readAsText(file);
   }
   // }}}
   ```

4. Implement `validateImportedPrefs()`:
   ```javascript
   // {{{ validate
   function validateImportedPrefs(data) {
       const validated = {};

       // Only accept known keys with correct types
       if (typeof data.styleGuide === 'string') {
           validated.styleGuide = data.styleGuide.substring(0, 500);
       }
       if (typeof data.negativePrompts === 'string') {
           validated.negativePrompts = data.negativePrompts.substring(0, 500);
       }
       if (typeof data.cardFrameStyle === 'string' &&
           ['ornate-gold', 'simple-black', 'faction-themed']
               .includes(data.cardFrameStyle)) {
           validated.cardFrameStyle = data.cardFrameStyle;
       }
       if (typeof data.animationSpeed === 'number' &&
           data.animationSpeed >= 0.5 && data.animationSpeed <= 2.0) {
           validated.animationSpeed = data.animationSpeed;
       }
       // ... validate other fields

       return Object.keys(validated).length > 0 ? validated : null;
   }
   // }}}
   ```

5. Add share URL generation (encode prefs in URL hash)

6. Add clipboard copy for sharing

7. Write import validation tests

## Related Documents
- 3-006a-preferences-storage.md
- 3-006b-preferences-ui.md
- 3-006-client-style-preferences.md (parent issue)

## Dependencies
- 3-006a: Preferences Storage Schema
- 3-006b: Preferences UI Panel

## File Format

```json
{
  "styleGuide": "dark fantasy, oil painting...",
  "negativePrompts": "cartoon, anime...",
  "cardFrameStyle": "ornate-gold",
  "animationSpeed": 1.0,
  "narrativeFont": "serif",
  "showUpgradeBadges": true,
  "reduceMotion": false,
  "highContrast": false
}
```

## Acceptance Criteria
- [ ] Export downloads valid JSON file
- [ ] Import reads and validates JSON
- [ ] Invalid data rejected with error message
- [ ] Partial imports merge with existing
- [ ] String lengths are bounded for security
