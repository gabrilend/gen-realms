# 3-006b: Preferences UI Panel

## Current Behavior
No UI for editing preferences.

## Intended Behavior
A preferences panel that allows users to edit all settings:
- Text areas for style guide and negative prompts
- Dropdowns for style selections
- Sliders for numeric values
- Toggles for boolean options
- Save/reset buttons

## Suggested Implementation Steps

1. Create preferences panel HTML:
   ```html
   <!-- {{{ preferences panel -->
   <div id="preferences-panel" class="modal hidden">
       <div class="modal-content">
           <h3>Preferences</h3>

           <section class="pref-section">
               <h4>Art Generation</h4>
               <label>Style Guide:</label>
               <textarea id="pref-style-guide" rows="3"></textarea>

               <label>Negative Prompts:</label>
               <textarea id="pref-negative" rows="2"></textarea>
           </section>

           <section class="pref-section">
               <h4>Display</h4>
               <label>Card Frame Style:</label>
               <select id="pref-card-frame">
                   <option value="ornate-gold">Ornate Gold</option>
                   <option value="simple-black">Simple Black</option>
                   <option value="faction-themed">Faction Themed</option>
               </select>

               <label>Narrative Font:</label>
               <select id="pref-narrative-font">
                   <option value="serif">Serif</option>
                   <option value="sans-serif">Sans-Serif</option>
                   <option value="fantasy">Fantasy</option>
               </select>

               <label>Animation Speed:</label>
               <input type="range" id="pref-anim-speed"
                      min="0.5" max="2.0" step="0.1">
               <span id="pref-anim-speed-value">1.0x</span>
           </section>

           <section class="pref-section">
               <h4>Accessibility</h4>
               <label>
                   <input type="checkbox" id="pref-reduce-motion">
                   Reduce Motion
               </label>
               <label>
                   <input type="checkbox" id="pref-high-contrast">
                   High Contrast
               </label>
           </section>

           <div class="pref-buttons">
               <button onclick="savePrefsFromUI()">Save</button>
               <button onclick="resetPrefsUI()">Reset to Defaults</button>
               <button onclick="closePrefsPanel()">Cancel</button>
           </div>
       </div>
   </div>
   <!-- }}} -->
   ```

2. Implement `openPrefsPanel()`:
   ```javascript
   // {{{ open panel
   function openPrefsPanel() {
       const prefs = loadPreferences();
       populatePrefsUI(prefs);
       document.getElementById('preferences-panel').classList.remove('hidden');
   }
   // }}}
   ```

3. Implement `populatePrefsUI()`:
   ```javascript
   // {{{ populate
   function populatePrefsUI(prefs) {
       document.getElementById('pref-style-guide').value = prefs.styleGuide;
       document.getElementById('pref-negative').value = prefs.negativePrompts;
       document.getElementById('pref-card-frame').value = prefs.cardFrameStyle;
       document.getElementById('pref-narrative-font').value = prefs.narrativeFont;
       document.getElementById('pref-anim-speed').value = prefs.animationSpeed;
       document.getElementById('pref-anim-speed-value').textContent =
           prefs.animationSpeed + 'x';
       document.getElementById('pref-reduce-motion').checked = prefs.reduceMotion;
       document.getElementById('pref-high-contrast').checked = prefs.highContrast;
   }
   // }}}
   ```

4. Implement `savePrefsFromUI()`:
   ```javascript
   // {{{ save from ui
   function savePrefsFromUI() {
       const prefs = loadPreferences();
       prefs.styleGuide = document.getElementById('pref-style-guide').value;
       prefs.negativePrompts = document.getElementById('pref-negative').value;
       prefs.cardFrameStyle = document.getElementById('pref-card-frame').value;
       prefs.narrativeFont = document.getElementById('pref-narrative-font').value;
       prefs.animationSpeed = parseFloat(
           document.getElementById('pref-anim-speed').value);
       prefs.reduceMotion = document.getElementById('pref-reduce-motion').checked;
       prefs.highContrast = document.getElementById('pref-high-contrast').checked;

       savePreferences(prefs);
       applyPreferences(prefs);
       closePrefsPanel();
   }
   // }}}
   ```

5. Implement `applyPreferences()` to update UI immediately

6. Add slider value display updates

7. Write UI interaction tests

## Related Documents
- 3-006a-preferences-storage.md
- 3-006-client-style-preferences.md (parent issue)

## Dependencies
- 3-006a: Preferences Storage Schema

## Acceptance Criteria
- [ ] Panel opens and closes properly
- [ ] All preferences editable via UI
- [ ] Changes apply immediately on save
- [ ] Reset restores defaults
- [ ] Slider shows current value
