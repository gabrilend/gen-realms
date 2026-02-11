/**
 * card-renderer.js - Card rendering for Symbeline Realms browser client
 *
 * Provides functions to render cards on the HTML5 canvas with faction colors,
 * effects, upgrade badges, and interaction states (hover, selected).
 */

(function() {
    'use strict';

    /* {{{ Card dimensions */
    const CARD = {
        WIDTH: 100,
        HEIGHT: 140,
        RADIUS: 8,
        PADDING: 6,
        ART_HEIGHT: 60,
        NAME_SIZE: 12,
        COST_SIZE: 14,
        EFFECT_SIZE: 10
    };
    /* }}} */

    /* {{{ Faction colors (matches canvas.js) */
    const FACTION_COLORS = {
        merchant: {
            primary: '#d4a017',
            secondary: '#8b6914',
            bg: '#2a2210',
            text: '#ffe066'
        },
        wilds: {
            primary: '#2d7a2d',
            secondary: '#1d4d1d',
            bg: '#0f200f',
            text: '#66ff66'
        },
        kingdom: {
            primary: '#3366cc',
            secondary: '#224488',
            bg: '#101830',
            text: '#6699ff'
        },
        artificer: {
            primary: '#cc3333',
            secondary: '#882222',
            bg: '#200f0f',
            text: '#ff6666'
        },
        neutral: {
            primary: '#888888',
            secondary: '#555555',
            bg: '#1a1a1a',
            text: '#cccccc'
        }
    };
    /* }}} */

    /* {{{ Effect icons (Unicode symbols) */
    const EFFECT_ICONS = {
        trade: '\u{1F4B0}',      /* 💰 */
        combat: '\u2694',        /* ⚔ */
        authority: '\u2764',     /* ❤ */
        draw: '\u{1F0CF}',       /* 🃏 */
        discard: '\u274C',       /* ❌ */
        scrap: '\u2672',         /* ♲ */
        destroy: '\u{1F4A5}'     /* 💥 */
    };

    /* Simpler ASCII fallback for effect display */
    const EFFECT_SYMBOLS = {
        trade: 'T',
        combat: 'C',
        authority: 'A',
        draw: 'D',
        discard: '-',
        scrap: 'X',
        destroy: '!'
    };
    /* }}} */

    /* {{{ Public API - window.cardRenderer */
    window.cardRenderer = {

        /* Get faction colors */
        getFactionColors: function(faction) {
            return FACTION_COLORS[faction] || FACTION_COLORS.neutral;
        },

        /* Get card dimensions */
        getCardSize: function() {
            return { width: CARD.WIDTH, height: CARD.HEIGHT };
        },

        /* {{{ renderCard
         * Main card rendering function.
         *
         * @param ctx - Canvas 2D context
         * @param card - Card data object { name, cost, faction, kind, effects, ... }
         * @param x, y - Top-left position
         * @param options - { selected, hovered, faceDown, scale }
         */
        renderCard: function(ctx, card, x, y, options) {
            options = options || {};
            const scale = options.scale || 1;
            const w = CARD.WIDTH * scale;
            const h = CARD.HEIGHT * scale;
            const r = CARD.RADIUS * scale;

            const colors = this.getFactionColors(card.faction);

            ctx.save();

            /* Hover glow effect */
            if (options.hovered) {
                ctx.shadowColor = colors.primary;
                ctx.shadowBlur = 15;
            }

            /* Selected highlight */
            if (options.selected) {
                ctx.shadowColor = '#ffffff';
                ctx.shadowBlur = 20;
            }

            /* Card background */
            ctx.fillStyle = colors.bg;
            this.roundRect(ctx, x, y, w, h, r);
            ctx.fill();

            /* Card border */
            ctx.strokeStyle = options.selected ? '#ffffff' : colors.primary;
            ctx.lineWidth = options.selected ? 3 : 2;
            ctx.stroke();

            ctx.shadowBlur = 0;

            if (options.faceDown) {
                this.renderCardBack(ctx, x, y, w, h, r, scale);
                ctx.restore();
                return;
            }

            /* Art placeholder area */
            const artY = y + 24 * scale;
            const artH = CARD.ART_HEIGHT * scale;
            ctx.fillStyle = '#1a1a2e';
            ctx.fillRect(x + CARD.PADDING * scale, artY,
                         w - CARD.PADDING * 2 * scale, artH);

            /* Art placeholder icon (faction symbol) */
            ctx.fillStyle = colors.secondary;
            ctx.font = (32 * scale) + 'px sans-serif';
            ctx.textAlign = 'center';
            ctx.fillText(this.getFactionSymbol(card.faction),
                         x + w / 2, artY + artH / 2 + 10 * scale);

            /* Card name */
            ctx.fillStyle = colors.text;
            ctx.font = 'bold ' + (CARD.NAME_SIZE * scale) + 'px sans-serif';
            ctx.textAlign = 'center';
            const name = this.truncateName(card.name, 12);
            ctx.fillText(name, x + w / 2, y + 16 * scale);

            /* Cost badge */
            if (card.cost !== undefined && card.cost >= 0) {
                this.renderCost(ctx, card.cost, x + w - 18 * scale, y + 8 * scale, scale);
            }

            /* Card kind indicator */
            ctx.fillStyle = '#666';
            ctx.font = (9 * scale) + 'px sans-serif';
            ctx.textAlign = 'left';
            const kindText = card.kind === 'base' ? 'BASE' :
                             card.kind === 'unit' ? 'UNIT' : 'SHIP';
            ctx.fillText(kindText, x + CARD.PADDING * scale, y + h - 6 * scale);

            /* Effects */
            this.renderEffects(ctx, card.effects, x + CARD.PADDING * scale,
                              y + artY - y + artH + 12 * scale, w - CARD.PADDING * 2 * scale, scale);

            /* Ally effects indicator */
            if (card.allyEffects && card.allyEffects.length > 0) {
                ctx.fillStyle = colors.primary;
                ctx.font = 'bold ' + (8 * scale) + 'px sans-serif';
                ctx.fillText('ALLY', x + CARD.PADDING * scale, y + h - 18 * scale);
            }

            /* Upgrade badges */
            if (card.attackBonus || card.tradeBonus || card.authorityBonus) {
                this.renderUpgradeBadges(ctx, card, x, y, w, scale);
            }

            /* Defense (for bases) */
            if (card.kind === 'base' && card.defense !== undefined) {
                this.renderDefense(ctx, card.defense, card.isOutpost,
                                   x + w - 12 * scale, y + h - 12 * scale, scale);
            }

            ctx.restore();
        },
        /* }}} */

        /* {{{ renderCost
         * Renders the card cost in a gold coin badge.
         */
        renderCost: function(ctx, cost, x, y, scale) {
            scale = scale || 1;
            const r = 10 * scale;

            /* Coin background */
            ctx.beginPath();
            ctx.arc(x, y, r, 0, Math.PI * 2);
            ctx.fillStyle = '#d4a017';
            ctx.fill();
            ctx.strokeStyle = '#8b6914';
            ctx.lineWidth = 1;
            ctx.stroke();

            /* Cost number */
            ctx.fillStyle = '#000';
            ctx.font = 'bold ' + (10 * scale) + 'px sans-serif';
            ctx.textAlign = 'center';
            ctx.textBaseline = 'middle';
            ctx.fillText(cost.toString(), x, y);
            ctx.textBaseline = 'alphabetic';
        },
        /* }}} */

        /* {{{ renderEffects
         * Renders effect summary with icons.
         */
        renderEffects: function(ctx, effects, x, y, maxWidth, scale) {
            if (!effects || effects.length === 0) return;

            scale = scale || 1;
            ctx.font = (CARD.EFFECT_SIZE * scale) + 'px monospace';
            ctx.textAlign = 'left';

            let offsetX = 0;
            const spacing = 4 * scale;

            for (let i = 0; i < effects.length && offsetX < maxWidth; i++) {
                const effect = effects[i];
                const text = this.formatEffect(effect);
                const color = this.getEffectColor(effect.type);

                ctx.fillStyle = color;
                ctx.fillText(text, x + offsetX, y);
                offsetX += ctx.measureText(text).width + spacing;
            }
        },
        /* }}} */

        /* {{{ renderUpgradeBadges
         * Renders permanent upgrade indicators.
         */
        renderUpgradeBadges: function(ctx, card, x, y, w, scale) {
            scale = scale || 1;
            const badgeSize = 12 * scale;
            let badgeX = x + w - badgeSize - 4 * scale;
            const badgeY = y + 26 * scale;

            if (card.attackBonus > 0) {
                ctx.fillStyle = '#cc66cc';
                ctx.beginPath();
                ctx.arc(badgeX, badgeY, badgeSize / 2, 0, Math.PI * 2);
                ctx.fill();
                ctx.fillStyle = '#fff';
                ctx.font = 'bold ' + (8 * scale) + 'px sans-serif';
                ctx.textAlign = 'center';
                ctx.textBaseline = 'middle';
                ctx.fillText('+' + card.attackBonus, badgeX, badgeY);
                badgeX -= badgeSize + 2 * scale;
            }

            if (card.tradeBonus > 0) {
                ctx.fillStyle = '#d4a017';
                ctx.beginPath();
                ctx.arc(badgeX, badgeY, badgeSize / 2, 0, Math.PI * 2);
                ctx.fill();
                ctx.fillStyle = '#000';
                ctx.font = 'bold ' + (8 * scale) + 'px sans-serif';
                ctx.textAlign = 'center';
                ctx.textBaseline = 'middle';
                ctx.fillText('+' + card.tradeBonus, badgeX, badgeY);
            }

            ctx.textBaseline = 'alphabetic';
        },
        /* }}} */

        /* {{{ renderDefense
         * Renders base defense value with outpost indicator.
         */
        renderDefense: function(ctx, defense, isOutpost, x, y, scale) {
            scale = scale || 1;
            const size = 14 * scale;

            /* Shield shape for outpost, circle for regular */
            if (isOutpost) {
                ctx.fillStyle = '#cc3333';
                ctx.beginPath();
                ctx.moveTo(x, y - size);
                ctx.lineTo(x + size, y - size / 2);
                ctx.lineTo(x + size, y + size / 2);
                ctx.lineTo(x, y + size);
                ctx.lineTo(x - size, y + size / 2);
                ctx.lineTo(x - size, y - size / 2);
                ctx.closePath();
                ctx.fill();
            } else {
                ctx.fillStyle = '#666';
                ctx.beginPath();
                ctx.arc(x, y, size / 2, 0, Math.PI * 2);
                ctx.fill();
            }

            ctx.fillStyle = '#fff';
            ctx.font = 'bold ' + (10 * scale) + 'px sans-serif';
            ctx.textAlign = 'center';
            ctx.textBaseline = 'middle';
            ctx.fillText(defense.toString(), x, y);
            ctx.textBaseline = 'alphabetic';
        },
        /* }}} */

        /* {{{ renderCardBack
         * Renders the back of a face-down card.
         */
        renderCardBack: function(ctx, x, y, w, h, r, scale) {
            scale = scale || 1;

            /* Pattern background */
            ctx.fillStyle = '#1a1a2e';
            this.roundRect(ctx, x + 4 * scale, y + 4 * scale,
                           w - 8 * scale, h - 8 * scale, r / 2);
            ctx.fill();

            /* Decorative pattern */
            ctx.strokeStyle = '#333';
            ctx.lineWidth = 1;
            for (let i = 0; i < 5; i++) {
                const offset = (i * 10 + 10) * scale;
                ctx.beginPath();
                ctx.moveTo(x + offset, y + 10 * scale);
                ctx.lineTo(x + 10 * scale, y + offset);
                ctx.stroke();
            }

            /* Center symbol */
            ctx.fillStyle = '#444';
            ctx.font = (24 * scale) + 'px sans-serif';
            ctx.textAlign = 'center';
            ctx.fillText('★', x + w / 2, y + h / 2 + 8 * scale);
        },
        /* }}} */

        /* {{{ Utility functions */

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
        },

        getFactionSymbol: function(faction) {
            switch (faction) {
                case 'merchant': return '⚖';
                case 'wilds': return '🌿';
                case 'kingdom': return '👑';
                case 'artificer': return '⚙';
                default: return '◆';
            }
        },

        truncateName: function(name, maxLen) {
            if (!name) return '???';
            if (name.length <= maxLen) return name;
            return name.substring(0, maxLen - 2) + '..';
        },

        formatEffect: function(effect) {
            if (!effect) return '';
            const val = effect.value || 0;
            switch (effect.type) {
                case 'trade': return '+' + val + 'T';
                case 'combat': return '+' + val + 'C';
                case 'authority': return '+' + val + 'A';
                case 'draw': return 'D' + val;
                case 'discard': return '-' + val;
                case 'destroy_base': return '!';
                case 'd10_up': return 'd+' + val;
                case 'd10_down': return 'd-' + val;
                default: return effect.type ? effect.type[0].toUpperCase() : '?';
            }
        },

        getEffectColor: function(type) {
            switch (type) {
                case 'trade': return '#d4a017';
                case 'combat': return '#cc66cc';
                case 'authority': return '#44cccc';
                case 'draw': return '#66ff66';
                case 'discard': return '#ff6666';
                default: return '#888888';
            }
        }

        /* }}} */
    };
    /* }}} */

})();
