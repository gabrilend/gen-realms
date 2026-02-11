/**
 * preferences-ui.js - Preferences panel UI for Symbeline Realms
 *
 * Creates and manages a modal preferences panel for editing all game
 * settings. Integrates with preferences.js for storage.
 */

(function() {
    'use strict';

    /* {{{ State */
    const state = {
        panel: null,
        isOpen: false
    };
    /* }}} */

    /* {{{ createPanel
     * Create the preferences panel DOM elements.
     */
    function createPanel() {
        if (state.panel) return state.panel;

        const panel = document.createElement('div');
        panel.id = 'preferences-panel';
        panel.className = 'modal hidden';
        panel.innerHTML = `
            <!-- {{{ preferences modal -->
            <div class="modal-backdrop"></div>
            <div class="modal-content">
                <h3>Preferences</h3>

                <section class="pref-section">
                    <h4>Art Generation</h4>
                    <label for="pref-style-guide">Style Guide:</label>
                    <textarea id="pref-style-guide" rows="2" placeholder="e.g., dark fantasy, oil painting..."></textarea>

                    <label for="pref-negative">Negative Prompts:</label>
                    <textarea id="pref-negative" rows="2" placeholder="e.g., cartoon, modern, anime..."></textarea>
                </section>

                <section class="pref-section">
                    <h4>Display</h4>
                    <div class="pref-row">
                        <label for="pref-card-frame">Card Frame:</label>
                        <select id="pref-card-frame">
                            <option value="ornate-gold">Ornate Gold</option>
                            <option value="simple-border">Simple Border</option>
                            <option value="faction-colored">Faction Colored</option>
                        </select>
                    </div>

                    <div class="pref-row">
                        <label for="pref-narrative-font">Narrative Font:</label>
                        <select id="pref-narrative-font">
                            <option value="serif">Serif (Georgia)</option>
                            <option value="sans-serif">Sans-Serif</option>
                            <option value="monospace">Monospace</option>
                        </select>
                    </div>

                    <div class="pref-row">
                        <label for="pref-anim-speed">Animation Speed:</label>
                        <input type="range" id="pref-anim-speed"
                               min="0.5" max="2.0" step="0.1" value="1.0">
                        <span id="pref-anim-speed-value">1.0x</span>
                    </div>

                    <div class="pref-row checkbox-row">
                        <label>
                            <input type="checkbox" id="pref-show-badges">
                            Show upgrade badges (+, ++)
                        </label>
                    </div>

                    <div class="pref-row checkbox-row">
                        <label>
                            <input type="checkbox" id="pref-faction-colors">
                            Show faction colors
                        </label>
                    </div>
                </section>

                <section class="pref-section">
                    <h4>Accessibility</h4>
                    <div class="pref-row checkbox-row">
                        <label>
                            <input type="checkbox" id="pref-reduce-motion">
                            Reduce motion (skip animations)
                        </label>
                    </div>

                    <div class="pref-row checkbox-row">
                        <label>
                            <input type="checkbox" id="pref-high-contrast">
                            High contrast mode
                        </label>
                    </div>

                    <div class="pref-row checkbox-row">
                        <label>
                            <input type="checkbox" id="pref-large-text">
                            Larger text
                        </label>
                    </div>
                </section>

                <div class="pref-buttons">
                    <button id="pref-save" class="btn-primary">Save</button>
                    <button id="pref-reset" class="btn-secondary">Reset to Defaults</button>
                    <button id="pref-cancel" class="btn-secondary">Cancel</button>
                </div>

                <div class="pref-footer">
                    <button id="pref-export-file" class="btn-small">Download File</button>
                    <button id="pref-export-clipboard" class="btn-small">Copy to Clipboard</button>
                    <button id="pref-import-file" class="btn-small">Import File</button>
                    <input type="file" id="pref-import-input" accept=".json" style="display:none">
                </div>

                <div id="pref-message" class="pref-message hidden"></div>
            </div>
            <!-- }}} -->
        `;

        document.body.appendChild(panel);
        state.panel = panel;

        /* Attach event listeners */
        setupEventListeners(panel);

        return panel;
    }
    /* }}} */

    /* {{{ setupEventListeners */
    function setupEventListeners(panel) {
        /* Close on backdrop click */
        panel.querySelector('.modal-backdrop').addEventListener('click', function() {
            window.preferencesUI.close();
        });

        /* Animation speed slider value display */
        const speedSlider = panel.querySelector('#pref-anim-speed');
        const speedValue = panel.querySelector('#pref-anim-speed-value');
        speedSlider.addEventListener('input', function() {
            speedValue.textContent = this.value + 'x';
        });

        /* Save button */
        panel.querySelector('#pref-save').addEventListener('click', function() {
            window.preferencesUI.save();
        });

        /* Reset button */
        panel.querySelector('#pref-reset').addEventListener('click', function() {
            window.preferencesUI.reset();
        });

        /* Cancel button */
        panel.querySelector('#pref-cancel').addEventListener('click', function() {
            window.preferencesUI.close();
        });

        /* Export to file button */
        panel.querySelector('#pref-export-file').addEventListener('click', function() {
            window.preferencesUI.exportToFile();
        });

        /* Export to clipboard button */
        panel.querySelector('#pref-export-clipboard').addEventListener('click', function() {
            window.preferencesUI.exportToClipboard();
        });

        /* Import from file button */
        panel.querySelector('#pref-import-file').addEventListener('click', function() {
            panel.querySelector('#pref-import-input').click();
        });

        /* File input change handler */
        panel.querySelector('#pref-import-input').addEventListener('change', function(e) {
            window.preferencesUI.importFromFile(e);
        });

        /* Close on Escape key */
        document.addEventListener('keydown', function(e) {
            if (e.key === 'Escape' && state.isOpen) {
                window.preferencesUI.close();
            }
        });
    }
    /* }}} */

    /* {{{ populateFromPrefs */
    function populateFromPrefs(prefs) {
        const panel = state.panel;
        if (!panel) return;

        panel.querySelector('#pref-style-guide').value = prefs.styleGuide || '';
        panel.querySelector('#pref-negative').value = prefs.negativePrompts || '';
        panel.querySelector('#pref-card-frame').value = prefs.cardFrameStyle || 'ornate-gold';
        panel.querySelector('#pref-narrative-font').value = prefs.narrativeFont || 'serif';
        panel.querySelector('#pref-anim-speed').value = prefs.animationSpeed || 1.0;
        panel.querySelector('#pref-anim-speed-value').textContent =
            (prefs.animationSpeed || 1.0) + 'x';
        panel.querySelector('#pref-show-badges').checked = prefs.showUpgradeBadges !== false;
        panel.querySelector('#pref-faction-colors').checked = prefs.showFactionColors !== false;
        panel.querySelector('#pref-reduce-motion').checked = prefs.reduceMotion || false;
        panel.querySelector('#pref-high-contrast').checked = prefs.highContrast || false;
        panel.querySelector('#pref-large-text').checked = prefs.largeText || false;
    }
    /* }}} */

    /* {{{ readFromUI */
    function readFromUI() {
        const panel = state.panel;
        if (!panel) return {};

        return {
            styleGuide: panel.querySelector('#pref-style-guide').value,
            negativePrompts: panel.querySelector('#pref-negative').value,
            cardFrameStyle: panel.querySelector('#pref-card-frame').value,
            narrativeFont: panel.querySelector('#pref-narrative-font').value,
            animationSpeed: parseFloat(panel.querySelector('#pref-anim-speed').value),
            showUpgradeBadges: panel.querySelector('#pref-show-badges').checked,
            showFactionColors: panel.querySelector('#pref-faction-colors').checked,
            reduceMotion: panel.querySelector('#pref-reduce-motion').checked,
            highContrast: panel.querySelector('#pref-high-contrast').checked,
            largeText: panel.querySelector('#pref-large-text').checked
        };
    }
    /* }}} */

    /* {{{ applyPreferences
     * Apply preferences to active game systems.
     */
    function applyPreferences(prefs) {
        /* Update animation system */
        if (window.animation) {
            window.animation.setSpeed(prefs.animationSpeed || 1.0);
            window.animation.setReduceMotion(prefs.reduceMotion || false);
        }

        /* Update narrative font */
        if (window.narrative) {
            window.narrative.setFont(prefs.narrativeFont || 'serif');
        }

        /* Apply high contrast mode */
        if (prefs.highContrast) {
            document.body.classList.add('high-contrast');
        } else {
            document.body.classList.remove('high-contrast');
        }

        /* Apply large text mode */
        if (prefs.largeText) {
            document.body.classList.add('large-text');
        } else {
            document.body.classList.remove('large-text');
        }
    }
    /* }}} */

    /* {{{ Public API - window.preferencesUI */
    window.preferencesUI = {

        /* {{{ open
         * Open the preferences panel.
         */
        open: function() {
            const panel = createPanel();
            const prefs = window.preferences ? window.preferences.load() : {};
            populateFromPrefs(prefs);
            panel.classList.remove('hidden');
            state.isOpen = true;
        },
        /* }}} */

        /* {{{ close
         * Close the preferences panel.
         */
        close: function() {
            if (state.panel) {
                state.panel.classList.add('hidden');
            }
            state.isOpen = false;
        },
        /* }}} */

        /* {{{ isOpen */
        isOpen: function() {
            return state.isOpen;
        },
        /* }}} */

        /* {{{ save
         * Save preferences from UI and close panel.
         */
        save: function() {
            const newPrefs = readFromUI();

            if (window.preferences) {
                window.preferences.save(newPrefs);
            }

            applyPreferences(newPrefs);
            this.close();

            console.log('Preferences saved');
        },
        /* }}} */

        /* {{{ reset
         * Reset preferences to defaults.
         */
        reset: function() {
            if (window.preferences) {
                const defaults = window.preferences.reset();
                populateFromPrefs(defaults);
                applyPreferences(defaults);
            }
        },
        /* }}} */

        /* {{{ showMessage
         * Show a temporary message in the panel.
         */
        showMessage: function(text, isError) {
            const msgEl = state.panel ? state.panel.querySelector('#pref-message') : null;
            if (!msgEl) return;

            msgEl.textContent = text;
            msgEl.className = 'pref-message' + (isError ? ' error' : ' success');
            msgEl.classList.remove('hidden');

            /* Auto-hide after 3 seconds */
            setTimeout(function() {
                msgEl.classList.add('hidden');
            }, 3000);
        },
        /* }}} */

        /* {{{ exportToFile
         * Export settings as downloadable JSON file.
         */
        exportToFile: function() {
            if (!window.preferences) {
                this.showMessage('Preferences system not available', true);
                return;
            }

            const prefs = window.preferences.load();

            /* Remove internal version field for cleaner export */
            const exportable = { ...prefs };
            delete exportable.version;

            const json = JSON.stringify(exportable, null, 2);
            const blob = new Blob([json], { type: 'application/json' });
            const url = URL.createObjectURL(blob);

            const a = document.createElement('a');
            a.href = url;
            a.download = 'symbeline-preferences.json';
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);

            URL.revokeObjectURL(url);
            this.showMessage('Settings downloaded');
        },
        /* }}} */

        /* {{{ exportToClipboard
         * Export settings to clipboard.
         */
        exportToClipboard: function() {
            if (!window.preferences) {
                this.showMessage('Preferences system not available', true);
                return;
            }

            const json = window.preferences.export();

            if (navigator.clipboard && navigator.clipboard.writeText) {
                navigator.clipboard.writeText(json).then(function() {
                    window.preferencesUI.showMessage('Settings copied to clipboard');
                }).catch(function(err) {
                    console.error('Failed to copy:', err);
                    window.preferencesUI.showMessage('Copy failed - check console', true);
                });
            } else {
                /* Fallback for older browsers */
                const textarea = document.createElement('textarea');
                textarea.value = json;
                document.body.appendChild(textarea);
                textarea.select();
                try {
                    document.execCommand('copy');
                    this.showMessage('Settings copied to clipboard');
                } catch (err) {
                    this.showMessage('Copy failed', true);
                }
                document.body.removeChild(textarea);
            }
        },
        /* }}} */

        /* {{{ importFromFile
         * Import settings from file input.
         */
        importFromFile: function(event) {
            const file = event.target.files[0];
            if (!file) return;

            const reader = new FileReader();
            reader.onload = function(e) {
                try {
                    const imported = JSON.parse(e.target.result);
                    const validated = window.preferencesUI.validateImportedPrefs(imported);

                    if (validated) {
                        /* Merge with current preferences */
                        const current = window.preferences ? window.preferences.load() : {};
                        const merged = { ...current, ...validated };

                        if (window.preferences) {
                            window.preferences.save(merged);
                        }

                        populateFromPrefs(merged);
                        applyPreferences(merged);
                        window.preferencesUI.showMessage('Settings imported successfully');
                    } else {
                        window.preferencesUI.showMessage('No valid settings found in file', true);
                    }
                } catch (err) {
                    console.error('Import error:', err);
                    window.preferencesUI.showMessage('Invalid JSON file: ' + err.message, true);
                }
            };

            reader.onerror = function() {
                window.preferencesUI.showMessage('Failed to read file', true);
            };

            reader.readAsText(file);

            /* Reset file input so same file can be selected again */
            event.target.value = '';
        },
        /* }}} */

        /* {{{ validateImportedPrefs
         * Validate and sanitize imported preferences.
         */
        validateImportedPrefs: function(data) {
            if (typeof data !== 'object' || data === null) {
                return null;
            }

            const validated = {};
            const MAX_STRING_LENGTH = 500;

            /* String fields with length limit */
            if (typeof data.styleGuide === 'string') {
                validated.styleGuide = data.styleGuide.substring(0, MAX_STRING_LENGTH);
            }
            if (typeof data.negativePrompts === 'string') {
                validated.negativePrompts = data.negativePrompts.substring(0, MAX_STRING_LENGTH);
            }

            /* Enum fields */
            const validFrameStyles = ['ornate-gold', 'simple-border', 'faction-colored'];
            if (validFrameStyles.includes(data.cardFrameStyle)) {
                validated.cardFrameStyle = data.cardFrameStyle;
            }

            const validFonts = ['serif', 'sans-serif', 'monospace'];
            if (validFonts.includes(data.narrativeFont)) {
                validated.narrativeFont = data.narrativeFont;
            }

            /* Numeric fields with range */
            if (typeof data.animationSpeed === 'number' &&
                data.animationSpeed >= 0.5 && data.animationSpeed <= 2.0) {
                validated.animationSpeed = data.animationSpeed;
            }

            /* Boolean fields */
            const booleanFields = [
                'showUpgradeBadges', 'showFactionColors',
                'reduceMotion', 'highContrast', 'largeText'
            ];
            booleanFields.forEach(function(field) {
                if (typeof data[field] === 'boolean') {
                    validated[field] = data[field];
                }
            });

            return Object.keys(validated).length > 0 ? validated : null;
        },
        /* }}} */

        /* {{{ toggle */
        toggle: function() {
            if (state.isOpen) {
                this.close();
            } else {
                this.open();
            }
        }
        /* }}} */

    };
    /* }}} */

})();
