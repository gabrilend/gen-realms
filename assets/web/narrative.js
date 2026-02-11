/**
 * narrative.js - Narrative display panel for Symbeline Realms
 *
 * Manages the LLM-generated story text display with:
 * - Scrollable history buffer
 * - Auto-scroll on new entries
 * - Text styling (bold, italic, event types)
 * - Toggle visibility
 * - Copy-to-clipboard for sharing
 */

(function() {
    'use strict';

    /* {{{ Constants */
    const MAX_HISTORY = 50;  /* Maximum entries to keep */
    /* }}} */

    /* {{{ State */
    const state = {
        history: [],
        visible: true,
        scrollOffset: 0,       /* Lines scrolled up from bottom */
        userScrolled: false,   /* User manually scrolled */
        font: 'serif'
    };
    /* }}} */

    /* {{{ Entry types for styling */
    const EntryType = {
        NARRATIVE: 'narrative',   /* Story text from LLM */
        ACTION: 'action',         /* Player action description */
        ATTACK: 'attack',         /* Combat action */
        PURCHASE: 'purchase',     /* Card purchased */
        SYSTEM: 'system',         /* System message */
        TURN: 'turn'              /* Turn transition */
    };
    /* }}} */

    /* {{{ Colors for entry types */
    const entryColors = {
        [EntryType.NARRATIVE]: '#d4c4a8',  /* Parchment color */
        [EntryType.ACTION]: '#aabbcc',     /* Soft blue */
        [EntryType.ATTACK]: '#ff6b6b',     /* Red */
        [EntryType.PURCHASE]: '#ffd93d',   /* Gold */
        [EntryType.SYSTEM]: '#888888',     /* Gray */
        [EntryType.TURN]: '#88cc88'        /* Green */
    };
    /* }}} */

    /* {{{ parseMarkup
     * Parse simple markup in narrative text.
     * Supports: *bold*, _italic_, **bold**, __italic__
     */
    function parseMarkup(text) {
        /* Store original for plain text fallback */
        let parsed = text;

        /* Bold: **text** or *text* (when surrounded by spaces or edges) */
        parsed = parsed.replace(/\*\*([^*]+)\*\*/g, '<strong>$1</strong>');
        parsed = parsed.replace(/\*([^*]+)\*/g, '<em class="bold">$1</em>');

        /* Italic: __text__ or _text_ */
        parsed = parsed.replace(/__([^_]+)__/g, '<em>$1</em>');
        parsed = parsed.replace(/_([^_]+)_/g, '<em>$1</em>');

        return parsed;
    }
    /* }}} */

    /* {{{ stripMarkup
     * Remove markup for plain text output.
     */
    function stripMarkup(text) {
        return text
            .replace(/\*\*([^*]+)\*\*/g, '$1')
            .replace(/\*([^*]+)\*/g, '$1')
            .replace(/__([^_]+)__/g, '$1')
            .replace(/_([^_]+)_/g, '$1');
    }
    /* }}} */

    /* {{{ wordWrap
     * Wrap text to specified width (for canvas rendering).
     */
    function wordWrap(text, ctx, maxWidth) {
        const words = text.split(' ');
        const lines = [];
        let currentLine = '';

        words.forEach(function(word) {
            const testLine = currentLine ? currentLine + ' ' + word : word;
            const metrics = ctx.measureText(testLine);

            if (metrics.width > maxWidth && currentLine) {
                lines.push(currentLine);
                currentLine = word;
            } else {
                currentLine = testLine;
            }
        });

        if (currentLine) {
            lines.push(currentLine);
        }

        return lines;
    }
    /* }}} */

    /* {{{ Public API - window.narrative */
    window.narrative = {

        /* Expose entry types */
        Type: EntryType,

        /* {{{ add
         * Add a new narrative entry.
         * @param text The narrative text
         * @param type EntryType (default: NARRATIVE)
         */
        add: function(text, type) {
            type = type || EntryType.NARRATIVE;

            const entry = {
                text: text,
                type: type,
                timestamp: Date.now()
            };

            state.history.push(entry);

            /* Trim to max history */
            while (state.history.length > MAX_HISTORY) {
                state.history.shift();
            }

            /* Auto-scroll to bottom if user hasn't scrolled up */
            if (!state.userScrolled) {
                state.scrollOffset = 0;
            }
        },
        /* }}} */

        /* {{{ addAction
         * Shorthand for adding an action entry.
         */
        addAction: function(text) {
            this.add(text, EntryType.ACTION);
        },
        /* }}} */

        /* {{{ addAttack
         * Shorthand for adding an attack entry.
         */
        addAttack: function(text) {
            this.add(text, EntryType.ATTACK);
        },
        /* }}} */

        /* {{{ addPurchase
         * Shorthand for adding a purchase entry.
         */
        addPurchase: function(text) {
            this.add(text, EntryType.PURCHASE);
        },
        /* }}} */

        /* {{{ addSystem
         * Shorthand for adding a system message.
         */
        addSystem: function(text) {
            this.add(text, EntryType.SYSTEM);
        },
        /* }}} */

        /* {{{ addTurn
         * Shorthand for adding a turn marker.
         */
        addTurn: function(text) {
            this.add(text, EntryType.TURN);
        },
        /* }}} */

        /* {{{ clear
         * Clear all narrative history.
         */
        clear: function() {
            state.history = [];
            state.scrollOffset = 0;
            state.userScrolled = false;
        },
        /* }}} */

        /* {{{ getHistory
         * Get a copy of the history array.
         */
        getHistory: function() {
            return state.history.slice();
        },
        /* }}} */

        /* {{{ getVisibleHistory
         * Get entries visible in the current scroll position.
         * @param maxLines Maximum lines to return
         */
        getVisibleHistory: function(maxLines) {
            maxLines = maxLines || 10;
            const end = state.history.length - state.scrollOffset;
            const start = Math.max(0, end - maxLines);
            return state.history.slice(start, end);
        },
        /* }}} */

        /* {{{ scrollUp
         * Scroll up in history.
         */
        scrollUp: function(lines) {
            lines = lines || 1;
            state.scrollOffset = Math.min(
                state.scrollOffset + lines,
                state.history.length - 1
            );
            state.userScrolled = state.scrollOffset > 0;
        },
        /* }}} */

        /* {{{ scrollDown
         * Scroll down in history.
         */
        scrollDown: function(lines) {
            lines = lines || 1;
            state.scrollOffset = Math.max(0, state.scrollOffset - lines);
            state.userScrolled = state.scrollOffset > 0;
        },
        /* }}} */

        /* {{{ scrollToBottom
         * Scroll to the most recent entries.
         */
        scrollToBottom: function() {
            state.scrollOffset = 0;
            state.userScrolled = false;
        },
        /* }}} */

        /* {{{ isAtBottom */
        isAtBottom: function() {
            return state.scrollOffset === 0;
        },
        /* }}} */

        /* {{{ setVisible */
        setVisible: function(visible) {
            state.visible = !!visible;
        },
        /* }}} */

        /* {{{ isVisible */
        isVisible: function() {
            return state.visible;
        },
        /* }}} */

        /* {{{ toggle */
        toggle: function() {
            state.visible = !state.visible;
        },
        /* }}} */

        /* {{{ setFont */
        setFont: function(font) {
            state.font = font;
        },
        /* }}} */

        /* {{{ getFont */
        getFont: function() {
            return state.font;
        },
        /* }}} */

        /* {{{ loadFromPreferences */
        loadFromPreferences: function() {
            if (window.preferences) {
                const prefs = window.preferences.load();
                this.setFont(prefs.narrativeFont || 'serif');
            }
        },
        /* }}} */

        /* {{{ copyToClipboard
         * Copy narrative history to clipboard (plain text).
         */
        copyToClipboard: function() {
            const text = state.history.map(function(entry) {
                return stripMarkup(entry.text);
            }).join('\n\n');

            if (navigator.clipboard && navigator.clipboard.writeText) {
                navigator.clipboard.writeText(text).then(function() {
                    console.log('Narrative copied to clipboard');
                }).catch(function(err) {
                    console.error('Failed to copy:', err);
                });
            } else {
                /* Fallback for older browsers */
                const textarea = document.createElement('textarea');
                textarea.value = text;
                document.body.appendChild(textarea);
                textarea.select();
                try {
                    document.execCommand('copy');
                    console.log('Narrative copied to clipboard');
                } catch (err) {
                    console.error('Failed to copy:', err);
                }
                document.body.removeChild(textarea);
            }
        },
        /* }}} */

        /* {{{ getColor
         * Get the color for an entry type.
         */
        getColor: function(type) {
            return entryColors[type] || entryColors[EntryType.NARRATIVE];
        },
        /* }}} */

        /* {{{ render
         * Render narrative panel to canvas context.
         * @param ctx Canvas 2D context
         * @param region {x, y, w, h} panel region
         */
        render: function(ctx, region) {
            if (!state.visible) return;

            /* Background */
            ctx.fillStyle = 'rgba(20, 15, 10, 0.9)';
            ctx.fillRect(region.x, region.y, region.w, region.h);

            /* Border */
            ctx.strokeStyle = '#554433';
            ctx.lineWidth = 2;
            ctx.strokeRect(region.x, region.y, region.w, region.h);

            /* Title */
            ctx.fillStyle = '#d4af37';
            ctx.font = 'bold 12px sans-serif';
            ctx.textAlign = 'left';
            ctx.fillText('Chronicle', region.x + 10, region.y + 16);

            /* Scroll indicator if not at bottom */
            if (!this.isAtBottom()) {
                ctx.fillStyle = '#666';
                ctx.font = '10px sans-serif';
                ctx.textAlign = 'right';
                ctx.fillText('(scroll: ' + state.scrollOffset + ')', region.x + region.w - 10, region.y + 16);
            }

            /* Calculate visible area */
            const textY = region.y + 30;
            const textH = region.h - 40;
            const maxWidth = region.w - 20;
            const lineHeight = 16;
            const maxLines = Math.floor(textH / lineHeight);

            /* Get visible entries */
            const visible = this.getVisibleHistory(maxLines * 2);  /* Get extra for word wrap */

            /* Render entries from bottom up */
            let y = region.y + region.h - 20;
            const fontFamily = state.font === 'serif' ? 'Georgia, serif' :
                               state.font === 'monospace' ? 'monospace' : 'sans-serif';

            ctx.font = '13px ' + fontFamily;
            ctx.textAlign = 'left';

            for (let i = visible.length - 1; i >= 0 && y > textY; i--) {
                const entry = visible[i];
                const lines = wordWrap(stripMarkup(entry.text), ctx, maxWidth);

                /* Render lines in reverse (bottom up) */
                for (let j = lines.length - 1; j >= 0 && y > textY; j--) {
                    ctx.fillStyle = this.getColor(entry.type);
                    ctx.fillText(lines[j], region.x + 10, y);
                    y -= lineHeight;
                }

                /* Small gap between entries */
                y -= 4;
            }

            /* Scroll hints */
            if (state.history.length > maxLines) {
                ctx.fillStyle = '#666';
                ctx.font = '10px sans-serif';
                ctx.textAlign = 'center';

                if (state.scrollOffset > 0) {
                    ctx.fillText('\u25BC newer', region.x + region.w / 2, region.y + region.h - 5);
                }

                const canScrollUp = state.scrollOffset < state.history.length - maxLines;
                if (canScrollUp) {
                    ctx.fillText('\u25B2 older', region.x + region.w / 2, region.y + 26);
                }
            }
        },
        /* }}} */

        /* {{{ renderHTML
         * Render narrative to HTML element (for DOM-based display).
         * @param container DOM element to render into
         */
        renderHTML: function(container) {
            if (!container) return;
            if (!state.visible) {
                container.style.display = 'none';
                return;
            }

            container.style.display = 'block';
            container.innerHTML = '';

            state.history.forEach(function(entry) {
                const p = document.createElement('p');
                p.className = 'narrative-entry ' + entry.type;
                p.innerHTML = parseMarkup(entry.text);
                p.style.color = entryColors[entry.type];
                container.appendChild(p);
            });

            /* Auto-scroll to bottom if not user-scrolled */
            if (!state.userScrolled) {
                container.scrollTop = container.scrollHeight;
            }
        }
        /* }}} */

    };
    /* }}} */

})();
