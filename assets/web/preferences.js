/**
 * preferences.js - Client preferences storage for Symbeline Realms
 *
 * Manages user preferences using localStorage with versioning for
 * future migrations. Handles art generation style, UI settings,
 * and accessibility options.
 */

(function() {
    'use strict';

    /* {{{ Constants */
    const PREFS_VERSION = 1;
    const PREFS_KEY = 'symbeline_prefs';
    /* }}} */

    /* {{{ Default preferences */
    const defaultPrefs = {
        version: PREFS_VERSION,

        /* Art generation settings */
        styleGuide: "dark fantasy, oil painting, dramatic lighting",
        negativePrompts: "cartoon, anime, bright colors, modern, photorealistic",

        /* UI preferences */
        cardFrameStyle: "ornate-gold",  /* ornate-gold, simple-border, faction-colored */
        animationSpeed: 1.0,            /* 0.5 = slow, 1.0 = normal, 2.0 = fast */
        narrativeFont: "serif",         /* serif, sans-serif, monospace */
        showUpgradeBadges: true,        /* Show +/++/*** on upgraded cards */
        showFactionColors: true,        /* Color-code cards by faction */

        /* Accessibility */
        reduceMotion: false,            /* Skip or minimize animations */
        highContrast: false,            /* Increase text/background contrast */
        largeText: false                /* Increase font sizes */
    };
    /* }}} */

    /* {{{ migratePreferences
     * Migrate old preference versions to current version.
     */
    function migratePreferences(oldPrefs) {
        let prefs = { ...oldPrefs };

        /* Version 0 -> 1 migrations would go here */
        /* Example:
         * if (prefs.version < 1) {
         *     prefs.newField = defaultPrefs.newField;
         *     prefs.version = 1;
         * }
         */

        /* Ensure version is current after migration */
        prefs.version = PREFS_VERSION;

        return prefs;
    }
    /* }}} */

    /* {{{ validatePrefs
     * Ensure preference values are within valid ranges.
     */
    function validatePrefs(prefs) {
        const validated = { ...prefs };

        /* Clamp animation speed to reasonable range */
        if (typeof validated.animationSpeed !== 'number' ||
            validated.animationSpeed < 0.1 ||
            validated.animationSpeed > 5.0) {
            validated.animationSpeed = defaultPrefs.animationSpeed;
        }

        /* Validate card frame style */
        const validFrameStyles = ['ornate-gold', 'simple-border', 'faction-colored'];
        if (!validFrameStyles.includes(validated.cardFrameStyle)) {
            validated.cardFrameStyle = defaultPrefs.cardFrameStyle;
        }

        /* Validate font choice */
        const validFonts = ['serif', 'sans-serif', 'monospace'];
        if (!validFonts.includes(validated.narrativeFont)) {
            validated.narrativeFont = defaultPrefs.narrativeFont;
        }

        /* Ensure booleans are actually booleans */
        const booleanFields = [
            'showUpgradeBadges', 'showFactionColors',
            'reduceMotion', 'highContrast', 'largeText'
        ];
        booleanFields.forEach(function(field) {
            if (typeof validated[field] !== 'boolean') {
                validated[field] = defaultPrefs[field];
            }
        });

        return validated;
    }
    /* }}} */

    /* {{{ Public API - window.preferences */
    window.preferences = {

        /* {{{ load
         * Load preferences from localStorage, falling back to defaults.
         */
        load: function() {
            try {
                const stored = localStorage.getItem(PREFS_KEY);
                if (!stored) {
                    return { ...defaultPrefs };
                }

                const prefs = JSON.parse(stored);

                /* Version migration if needed */
                if (prefs.version < PREFS_VERSION) {
                    const migrated = migratePreferences(prefs);
                    this.save(migrated);
                    return migrated;
                }

                /* Merge with defaults for any missing keys, then validate */
                const merged = { ...defaultPrefs, ...prefs };
                return validatePrefs(merged);

            } catch (e) {
                console.error('Failed to load preferences:', e);
                return { ...defaultPrefs };
            }
        },
        /* }}} */

        /* {{{ save
         * Save preferences to localStorage.
         */
        save: function(prefs) {
            try {
                const toSave = validatePrefs({ ...prefs, version: PREFS_VERSION });
                localStorage.setItem(PREFS_KEY, JSON.stringify(toSave));
                return true;
            } catch (e) {
                console.error('Failed to save preferences:', e);
                return false;
            }
        },
        /* }}} */

        /* {{{ reset
         * Reset to default preferences.
         */
        reset: function() {
            try {
                localStorage.removeItem(PREFS_KEY);
            } catch (e) {
                console.error('Failed to clear preferences:', e);
            }
            return { ...defaultPrefs };
        },
        /* }}} */

        /* {{{ get
         * Get a single preference value.
         */
        get: function(key) {
            const prefs = this.load();
            return prefs.hasOwnProperty(key) ? prefs[key] : undefined;
        },
        /* }}} */

        /* {{{ set
         * Set a single preference value and save.
         */
        set: function(key, value) {
            const prefs = this.load();
            prefs[key] = value;
            return this.save(prefs);
        },
        /* }}} */

        /* {{{ getDefaults
         * Get a copy of the default preferences.
         */
        getDefaults: function() {
            return { ...defaultPrefs };
        },
        /* }}} */

        /* {{{ getVersion
         * Get the current preferences version.
         */
        getVersion: function() {
            return PREFS_VERSION;
        },
        /* }}} */

        /* {{{ export
         * Export preferences as a JSON string for sharing/backup.
         */
        export: function() {
            const prefs = this.load();
            return JSON.stringify(prefs, null, 2);
        },
        /* }}} */

        /* {{{ import
         * Import preferences from a JSON string.
         */
        import: function(jsonString) {
            try {
                const prefs = JSON.parse(jsonString);

                /* Basic validation - must be an object */
                if (typeof prefs !== 'object' || prefs === null) {
                    throw new Error('Invalid preferences format');
                }

                /* Migrate if older version */
                if (prefs.version && prefs.version < PREFS_VERSION) {
                    const migrated = migratePreferences(prefs);
                    return this.save(migrated);
                }

                /* Merge with defaults to fill any gaps */
                const merged = { ...defaultPrefs, ...prefs };
                return this.save(merged);

            } catch (e) {
                console.error('Failed to import preferences:', e);
                return false;
            }
        }
        /* }}} */

    };
    /* }}} */

})();
