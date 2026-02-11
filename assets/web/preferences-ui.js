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
                    <button id="pref-export" class="btn-small">Export Settings</button>
                    <button id="pref-import" class="btn-small">Import Settings</button>
                </div>
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

        /* Export button */
        panel.querySelector('#pref-export').addEventListener('click', function() {
            window.preferencesUI.exportSettings();
        });

        /* Import button */
        panel.querySelector('#pref-import').addEventListener('click', function() {
            window.preferencesUI.importSettings();
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

        /* {{{ exportSettings
         * Export settings to clipboard or file.
         */
        exportSettings: function() {
            if (!window.preferences) {
                console.error('Preferences system not available');
                return;
            }

            const json = window.preferences.export();

            if (navigator.clipboard && navigator.clipboard.writeText) {
                navigator.clipboard.writeText(json).then(function() {
                    alert('Settings copied to clipboard');
                }).catch(function(err) {
                    console.error('Failed to copy:', err);
                    prompt('Copy these settings:', json);
                });
            } else {
                prompt('Copy these settings:', json);
            }
        },
        /* }}} */

        /* {{{ importSettings
         * Import settings from clipboard or prompt.
         */
        importSettings: function() {
            const json = prompt('Paste settings JSON:');
            if (!json) return;

            if (window.preferences && window.preferences.import(json)) {
                const prefs = window.preferences.load();
                populateFromPrefs(prefs);
                applyPreferences(prefs);
                alert('Settings imported successfully');
            } else {
                alert('Failed to import settings - invalid format');
            }
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
