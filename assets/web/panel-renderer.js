/**
 * panel-renderer.js - Status and narrative panel rendering
 *
 * Provides rendering for the status bar and narrative panel in the
 * browser client. Status shows turn info, resources, and opponent summary.
 * Narrative displays LLM-generated story text with word wrapping.
 */

(function() {
    'use strict';

    /* {{{ Constants */
    const COLORS = {
        authority: '#44cccc',
        trade: '#d4af37',
        combat: '#cc66cc',
        d10: '#4a90d9',
        d4: '#9b59b6',
        opponent: '#cc6666',
        narrative: {
            bg: 'rgba(20, 15, 10, 0.9)',
            border: '#8b7355',
            title: '#d4c4a8',
            text: '#c4b4a0'
        }
    };
    /* }}} */

    /* {{{ Public API - window.panelRenderer */
    window.panelRenderer = {

        /* {{{ renderStatusBar
         * Renders the main status bar with turn info, resources, and opponent.
         *
         * @param ctx - Canvas 2D context
         * @param gameState - { turn, phase, player, opponent }
         * @param region - { x, y, w, h } layout region
         */
        renderStatusBar: function(ctx, gameState, region) {
            /* Background */
            ctx.fillStyle = 'rgba(0, 0, 0, 0.7)';
            ctx.fillRect(region.x, region.y, region.w, region.h);

            const y = region.y + region.h / 2 + 6;
            let x = region.x + 15;

            /* Turn number */
            ctx.fillStyle = '#fff';
            ctx.font = 'bold 16px sans-serif';
            ctx.textAlign = 'left';
            ctx.fillText('Turn ' + gameState.turn, x, y);
            x += 80;

            /* Phase */
            ctx.fillStyle = '#888';
            ctx.font = '12px sans-serif';
            ctx.fillText(this.formatPhase(gameState.phase), x, y);
            x += 100;

            /* Authority */
            this.renderAuthority(ctx, gameState.player.authority, x, region.y + 8);
            x += 90;

            /* Trade pool */
            this.renderPool(ctx, gameState.player.trade || 0, 'Trade', COLORS.trade, x, region.y + 8);
            x += 70;

            /* Combat pool */
            this.renderPool(ctx, gameState.player.combat || 0, 'Combat', COLORS.combat, x, region.y + 8);
            x += 80;

            /* d10/d4 tracker */
            this.renderDeckTracker(ctx, gameState.player.d10 || 0, gameState.player.d4 || 0, x, region.y + 8);
            x += 100;

            /* Opponent summary (right side) */
            this.renderOpponentSummary(ctx, gameState.opponent, region.x + region.w - 200, region.y + 8);
        },
        /* }}} */

        /* {{{ renderAuthority
         * Renders authority with heart icon.
         */
        renderAuthority: function(ctx, authority, x, y) {
            /* Heart icon */
            this.drawHeart(ctx, x, y + 18, 10);

            /* Value */
            ctx.fillStyle = COLORS.authority;
            ctx.font = 'bold 20px sans-serif';
            ctx.textAlign = 'left';
            ctx.fillText(authority.toString(), x + 20, y + 24);
        },
        /* }}} */

        /* {{{ renderPool
         * Renders a resource pool (trade or combat).
         */
        renderPool: function(ctx, value, label, color, x, y) {
            ctx.fillStyle = color;
            ctx.font = 'bold 18px sans-serif';
            ctx.textAlign = 'left';
            ctx.fillText(value.toString(), x, y + 22);

            ctx.fillStyle = '#666';
            ctx.font = '10px sans-serif';
            ctx.fillText(label, x, y + 34);
        },
        /* }}} */

        /* {{{ renderDeckTracker
         * Renders the d10/d4 deck tracker.
         */
        renderDeckTracker: function(ctx, d10, d4, x, y) {
            /* d10 */
            ctx.fillStyle = COLORS.d10;
            ctx.font = '14px sans-serif';
            ctx.textAlign = 'left';
            ctx.fillText('d10: ' + d10, x, y + 18);

            /* d4 */
            ctx.fillStyle = COLORS.d4;
            ctx.fillText('d4: ' + d4, x, y + 34);
        },
        /* }}} */

        /* {{{ renderOpponentSummary
         * Renders opponent info on the right side of status bar.
         */
        renderOpponentSummary: function(ctx, opponent, x, y) {
            if (!opponent) return;

            /* Label */
            ctx.fillStyle = '#888';
            ctx.font = '10px sans-serif';
            ctx.textAlign = 'left';
            ctx.fillText('OPPONENT', x, y + 12);

            /* Authority */
            ctx.fillStyle = COLORS.opponent;
            ctx.font = 'bold 16px sans-serif';
            ctx.fillText(opponent.authority + ' HP', x, y + 30);

            /* Hand count if known */
            if (opponent.handCount !== undefined) {
                ctx.fillStyle = '#666';
                ctx.font = '11px sans-serif';
                ctx.fillText(opponent.handCount + ' cards', x + 60, y + 30);
            }

            /* Base count if known */
            if (opponent.baseCount !== undefined && opponent.baseCount > 0) {
                ctx.fillText(opponent.baseCount + ' bases', x + 120, y + 30);
            }
        },
        /* }}} */

        /* {{{ renderNarrativePanel
         * Renders the narrative panel with LLM-generated story text.
         *
         * @param ctx - Canvas 2D context
         * @param narrativeHistory - Array of { text, timestamp } entries
         * @param region - { x, y, w, h } layout region
         * @param scrollOffset - Number of entries to skip from top
         */
        renderNarrativePanel: function(ctx, narrativeHistory, region, scrollOffset) {
            scrollOffset = scrollOffset || 0;

            /* Background */
            ctx.fillStyle = COLORS.narrative.bg;
            ctx.fillRect(region.x, region.y, region.w, region.h);

            /* Border */
            ctx.strokeStyle = COLORS.narrative.border;
            ctx.lineWidth = 2;
            ctx.strokeRect(region.x, region.y, region.w, region.h);

            /* Title */
            ctx.fillStyle = COLORS.narrative.title;
            ctx.font = 'italic 12px Georgia, serif';
            ctx.textAlign = 'left';
            ctx.fillText('The Tale Unfolds...', region.x + 10, region.y + 18);

            if (!narrativeHistory || narrativeHistory.length === 0) {
                ctx.fillStyle = '#666';
                ctx.font = 'italic 11px Georgia, serif';
                ctx.fillText('The story awaits...', region.x + 10, region.y + 45);
                return;
            }

            /* Calculate visible entries */
            const maxEntries = 5;
            const entries = narrativeHistory.slice(-(maxEntries + scrollOffset));
            const visibleEntries = entries.slice(scrollOffset, scrollOffset + maxEntries);

            /* Render entries with word wrapping */
            ctx.fillStyle = COLORS.narrative.text;
            ctx.font = '12px Georgia, serif';

            let y = region.y + 40;
            const maxWidth = region.w - 20;
            const lineHeight = 16;

            for (const entry of visibleEntries) {
                const text = typeof entry === 'string' ? entry : entry.text;
                const lines = this.wrapText(ctx, text, maxWidth);

                for (const line of lines) {
                    if (y > region.y + region.h - 10) break;
                    ctx.fillText(line, region.x + 10, y);
                    y += lineHeight;
                }

                y += 8; /* Gap between entries */
            }

            /* Scroll indicator */
            if (narrativeHistory.length > maxEntries) {
                const canScrollUp = scrollOffset > 0;
                const canScrollDown = narrativeHistory.length > scrollOffset + maxEntries;

                ctx.fillStyle = '#555';
                ctx.font = '10px sans-serif';
                ctx.textAlign = 'right';

                if (canScrollUp) {
                    ctx.fillText('▲', region.x + region.w - 10, region.y + 30);
                }
                if (canScrollDown) {
                    ctx.fillText('▼', region.x + region.w - 10, region.y + region.h - 10);
                }
            }
        },
        /* }}} */

        /* {{{ wrapText
         * Word-wrap text to fit within maxWidth.
         */
        wrapText: function(ctx, text, maxWidth) {
            if (!text) return [];

            const words = text.split(' ');
            const lines = [];
            let currentLine = '';

            for (const word of words) {
                const testLine = currentLine ? currentLine + ' ' + word : word;
                const metrics = ctx.measureText(testLine);

                if (metrics.width > maxWidth && currentLine) {
                    lines.push(currentLine);
                    currentLine = word;
                } else {
                    currentLine = testLine;
                }
            }

            if (currentLine) {
                lines.push(currentLine);
            }

            return lines;
        },
        /* }}} */

        /* {{{ drawHeart
         * Draw a heart icon at the given position.
         */
        drawHeart: function(ctx, x, y, size) {
            ctx.fillStyle = '#ff4444';
            ctx.beginPath();

            /* Heart shape using bezier curves */
            const w = size;
            const h = size;
            ctx.moveTo(x + w / 2, y + h);
            ctx.bezierCurveTo(x + w / 2, y + h * 0.7, x, y + h * 0.5, x, y + h * 0.3);
            ctx.bezierCurveTo(x, y, x + w / 4, y, x + w / 2, y + h * 0.3);
            ctx.bezierCurveTo(x + w * 0.75, y, x + w, y, x + w, y + h * 0.3);
            ctx.bezierCurveTo(x + w, y + h * 0.5, x + w / 2, y + h * 0.7, x + w / 2, y + h);
            ctx.fill();
        },
        /* }}} */

        /* {{{ formatPhase
         * Format phase name for display.
         */
        formatPhase: function(phase) {
            if (!phase) return 'UNKNOWN';

            const phases = {
                'main': 'MAIN PHASE',
                'combat': 'COMBAT',
                'buy': 'BUY PHASE',
                'draw_order': 'DRAW ORDER',
                'end': 'END TURN',
                'waiting': 'OPPONENT\'S TURN'
            };

            return phases[phase.toLowerCase()] || phase.toUpperCase();
        },
        /* }}} */

        /* {{{ renderActionButtons
         * Renders action buttons for mobile/touch support.
         *
         * @param ctx - Canvas 2D context
         * @param actions - Array of { id, label, enabled, color }
         * @param region - { x, y, w, h } region for buttons
         */
        renderActionButtons: function(ctx, actions, region) {
            if (!actions || actions.length === 0) return;

            const buttonWidth = 80;
            const buttonHeight = 30;
            const spacing = 10;
            const totalWidth = actions.length * (buttonWidth + spacing) - spacing;
            let x = region.x + (region.w - totalWidth) / 2;
            const y = region.y + (region.h - buttonHeight) / 2;

            for (const action of actions) {
                /* Button background */
                ctx.fillStyle = action.enabled ? (action.color || '#446688') : '#333';
                this.roundRect(ctx, x, y, buttonWidth, buttonHeight, 5);
                ctx.fill();

                /* Button border */
                ctx.strokeStyle = action.enabled ? '#6688aa' : '#444';
                ctx.lineWidth = 1;
                ctx.stroke();

                /* Button text */
                ctx.fillStyle = action.enabled ? '#fff' : '#666';
                ctx.font = 'bold 11px sans-serif';
                ctx.textAlign = 'center';
                ctx.fillText(action.label, x + buttonWidth / 2, y + buttonHeight / 2 + 4);

                x += buttonWidth + spacing;
            }
        },
        /* }}} */

        /* {{{ roundRect helper */
        roundRect: function(ctx, x, y, w, h, r) {
            ctx.beginPath();
            ctx.moveTo(x + r, y);
            ctx.lineTo(x + w - r, y);
            ctx.quadraticCurveTo(x + w, y, x + w, y + r);
            ctx.lineTo(x + w, y + h - r);
            ctx.quadraticCurveTo(x + w, y + h, x + w - r, y + h);
            ctx.lineTo(x + r, y + h);
            ctx.quadraticCurveTo(x, y + h, x, y + h - r);
            ctx.lineTo(x, y + r);
            ctx.quadraticCurveTo(x, y, x + r, y);
            ctx.closePath();
        }
        /* }}} */

    };
    /* }}} */

})();
