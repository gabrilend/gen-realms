# 3-006a: Preferences Storage Schema

## Current Behavior
No preference storage exists.

## Intended Behavior
Define and implement the localStorage-based preference storage:
- Define preference schema with defaults
- Implement load/save functions
- Handle missing or corrupt data gracefully
- Version preferences for future migration

## Suggested Implementation Steps

1. Create `assets/web/preferences.js`:
   ```javascript
   // {{{ preference schema
   const PREFS_VERSION = 1;
   const PREFS_KEY = 'symbeline_prefs';

   const defaultPrefs = {
       version: PREFS_VERSION,
       // Art generation
       styleGuide: "dark fantasy, oil painting, dramatic lighting",
       negativePrompts: "cartoon, anime, bright colors, modern",
       // UI preferences
       cardFrameStyle: "ornate-gold",
       animationSpeed: 1.0,
       narrativeFont: "serif",
       showUpgradeBadges: true,
       // Accessibility
       reduceMotion: false,
       highContrast: false
   };
   // }}}
   ```

2. Implement `loadPreferences()`:
   ```javascript
   // {{{ load
   function loadPreferences() {
       try {
           const stored = localStorage.getItem(PREFS_KEY);
           if (!stored) return { ...defaultPrefs };

           const prefs = JSON.parse(stored);

           // Version migration if needed
           if (prefs.version < PREFS_VERSION) {
               return migratePreferences(prefs);
           }

           // Merge with defaults for any missing keys
           return { ...defaultPrefs, ...prefs };
       } catch (e) {
           console.error('Failed to load preferences:', e);
           return { ...defaultPrefs };
       }
   }
   // }}}
   ```

3. Implement `savePreferences()`:
   ```javascript
   // {{{ save
   function savePreferences(prefs) {
       try {
           prefs.version = PREFS_VERSION;
           localStorage.setItem(PREFS_KEY, JSON.stringify(prefs));
           return true;
       } catch (e) {
           console.error('Failed to save preferences:', e);
           return false;
       }
   }
   // }}}
   ```

4. Implement `resetPreferences()`:
   ```javascript
   // {{{ reset
   function resetPreferences() {
       localStorage.removeItem(PREFS_KEY);
       return { ...defaultPrefs };
   }
   // }}}
   ```

5. Implement `migratePreferences()` for version upgrades

6. Write storage tests

## Related Documents
- 3-006-client-style-preferences.md (parent issue)
- 3-004-browser-canvas-renderer.md

## Dependencies
- Browser localStorage API

## Preference Schema

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| styleGuide | string | "dark fantasy..." | Art generation style |
| negativePrompts | string | "cartoon, anime..." | Excluded elements |
| cardFrameStyle | string | "ornate-gold" | Card border style |
| animationSpeed | number | 1.0 | Animation multiplier |
| narrativeFont | string | "serif" | Story text font |
| showUpgradeBadges | bool | true | Show +/++ badges |
| reduceMotion | bool | false | Accessibility |
| highContrast | bool | false | Accessibility |

## Acceptance Criteria
- [ ] Default preferences returned for new users
- [ ] Preferences persist across page reloads
- [ ] Corrupt JSON doesn't crash app
- [ ] Version migration works
