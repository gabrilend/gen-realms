/**
 * canvas.js - Canvas layout and rendering infrastructure for Symbeline Realms
 *
 * Manages the HTML5 canvas layout, defines game zones, and provides
 * rendering utilities. This module is independent of the Wasm game logic
 * and can be used for pure-JavaScript rendering or as a backend for Wasm.
 */

(function() {
    'use strict';

    /* {{{ Configuration */
    const CANVAS_CONFIG = {
        minWidth: 800,
        minHeight: 600,
        targetFps: 60,
        backgroundColor: '#1a1a2e',
        gridColor: '#2a2a4e',
        showDebugGrid: false
    };
    /* }}} */

    /* {{{ Faction colors */
    const FACTION_COLORS = {
        merchant: { primary: '#d4a017', secondary: '#8b6914', bg: 'rgba(212, 160, 23, 0.1)' },
        wilds: { primary: '#2d7a2d', secondary: '#1d4d1d', bg: 'rgba(45, 122, 45, 0.1)' },
        kingdom: { primary: '#3366cc', secondary: '#224488', bg: 'rgba(51, 102, 204, 0.1)' },
        artificer: { primary: '#cc3333', secondary: '#882222', bg: 'rgba(204, 51, 51, 0.1)' },
        neutral: { primary: '#888888', secondary: '#555555', bg: 'rgba(136, 136, 136, 0.1)' }
    };

    const VALUE_COLORS = {
        authority: '#44cccc',
        combat: '#cc66cc',
        trade: '#d4a017'
    };
    /* }}} */

    /* {{{ Layout regions
     * Defines the proportional areas of the game screen.
     * Updated on resize via recalculateLayout().
     */
    const layout = {
        /* Status bar at top */
        status: { x: 0, y: 0, w: 0, h: 0 },

        /* Player's hand (bottom center) */
        hand: { x: 0, y: 0, w: 0, h: 0 },

        /* Trade row (top center) */
        tradeRow: { x: 0, y: 0, w: 0, h: 0 },

        /* Player's bases (bottom left) */
        playerBases: { x: 0, y: 0, w: 0, h: 0 },

        /* Opponent's bases (top left) */
        opponentBases: { x: 0, y: 0, w: 0, h: 0 },

        /* Narrative panel (right side) */
        narrative: { x: 0, y: 0, w: 0, h: 0 },

        /* Play area (center) */
        playArea: { x: 0, y: 0, w: 0, h: 0 }
    };

    /* Card dimensions (calculated based on layout) */
    const cardSize = { w: 100, h: 140 };
    const cardSpacing = 10;
    /* }}} */

    /* {{{ Canvas state */
    let canvas = null;
    let ctx = null;
    let lastFrameTime = 0;
    let frameCount = 0;
    let fps = 0;
    let fpsUpdateTime = 0;
    /* }}} */

    /* {{{ Public API - window.canvasLayout */
    window.canvasLayout = {
        /* Initialize canvas and layout system */
        init: function(canvasId) {
            canvas = document.getElementById(canvasId);
            if (!canvas) {
                console.error('[Canvas] Canvas element not found:', canvasId);
                return false;
            }

            ctx = canvas.getContext('2d', { alpha: false });
            if (!ctx) {
                console.error('[Canvas] Failed to get 2D context');
                return false;
            }

            /* Set up resize handler */
            window.addEventListener('resize', this.handleResize.bind(this));
            this.handleResize();

            console.log('[Canvas] Initialized', canvas.width + 'x' + canvas.height);
            return true;
        },

        /* Get canvas context */
        getContext: function() {
            return ctx;
        },

        /* Get layout regions */
        getLayout: function() {
            return layout;
        },

        /* Get faction colors */
        getFactionColor: function(faction, type) {
            const colors = FACTION_COLORS[faction] || FACTION_COLORS.neutral;
            return colors[type || 'primary'];
        },

        /* Get value colors */
        getValueColor: function(valueType) {
            return VALUE_COLORS[valueType] || '#ffffff';
        },

        /* Handle window resize */
        handleResize: function() {
            if (!canvas) return;

            const container = canvas.parentElement;
            canvas.width = Math.max(container.clientWidth, CANVAS_CONFIG.minWidth);
            canvas.height = Math.max(container.clientHeight, CANVAS_CONFIG.minHeight);

            recalculateLayout();
        },

        /* Clear canvas with background color */
        clear: function() {
            if (!ctx) return;
            ctx.fillStyle = CANVAS_CONFIG.backgroundColor;
            ctx.fillRect(0, 0, canvas.width, canvas.height);
        },

        /* Draw debug grid (useful for layout testing) */
        drawDebugGrid: function() {
            if (!ctx || !CANVAS_CONFIG.showDebugGrid) return;

            ctx.strokeStyle = CANVAS_CONFIG.gridColor;
            ctx.lineWidth = 1;

            /* Vertical lines every 50px */
            for (let x = 0; x < canvas.width; x += 50) {
                ctx.beginPath();
                ctx.moveTo(x, 0);
                ctx.lineTo(x, canvas.height);
                ctx.stroke();
            }

            /* Horizontal lines every 50px */
            for (let y = 0; y < canvas.height; y += 50) {
                ctx.beginPath();
                ctx.moveTo(0, y);
                ctx.lineTo(canvas.width, y);
                ctx.stroke();
            }
        },

        /* Draw layout region borders (debug) */
        drawLayoutRegions: function() {
            if (!ctx) return;

            ctx.strokeStyle = '#444';
            ctx.lineWidth = 1;
            ctx.setLineDash([5, 5]);

            for (const name in layout) {
                const region = layout[name];
                ctx.strokeRect(region.x, region.y, region.w, region.h);

                ctx.fillStyle = '#666';
                ctx.font = '10px monospace';
                ctx.fillText(name, region.x + 4, region.y + 12);
            }

            ctx.setLineDash([]);
        },

        /* Draw a region border with title */
        drawRegionBorder: function(regionName, title, color) {
            if (!ctx) return;

            const region = layout[regionName];
            if (!region) return;

            ctx.strokeStyle = color || '#444';
            ctx.lineWidth = 1;
            ctx.strokeRect(region.x, region.y, region.w, region.h);

            if (title) {
                ctx.fillStyle = color || '#666';
                ctx.font = 'bold 12px monospace';
                const textWidth = ctx.measureText(title).width;
                const titleX = region.x + (region.w - textWidth) / 2;
                ctx.fillText(title, titleX, region.y + 14);
            }
        },

        /* Get card size */
        getCardSize: function() {
            return { ...cardSize };
        },

        /* Calculate FPS */
        updateFps: function(time) {
            frameCount++;

            if (time - fpsUpdateTime >= 1000) {
                fps = frameCount;
                frameCount = 0;
                fpsUpdateTime = time;
            }

            return fps;
        },

        /* Get current FPS */
        getFps: function() {
            return fps;
        },

        /* Start render loop */
        startRenderLoop: function(renderCallback) {
            function loop(time) {
                const dt = time - lastFrameTime;
                lastFrameTime = time;

                window.canvasLayout.updateFps(time);

                if (typeof renderCallback === 'function') {
                    renderCallback(ctx, dt, time);
                }

                requestAnimationFrame(loop);
            }

            requestAnimationFrame(loop);
        }
    };
    /* }}} */

    /* {{{ recalculateLayout
     * Calculate proportional layout regions based on canvas size.
     * Layout adapts to different screen sizes while maintaining usability.
     */
    function recalculateLayout() {
        if (!canvas) return;

        const w = canvas.width;
        const h = canvas.height;

        /* Padding from edges */
        const padding = 10;

        /* Status bar height */
        const statusHeight = 40;

        /* Narrative panel width (right side) */
        const narrativeWidth = Math.min(300, w * 0.25);

        /* Main game area (excluding narrative) */
        const gameWidth = w - narrativeWidth - padding * 2;
        const gameHeight = h - statusHeight - padding * 2;

        /* Hand area height (bottom) */
        const handHeight = cardSize.h + padding * 2;

        /* Bases area width (sides) */
        const basesWidth = cardSize.w * 2 + cardSpacing + padding;

        /* Status bar */
        layout.status = {
            x: padding,
            y: padding,
            w: w - padding * 2,
            h: statusHeight
        };

        /* Trade row (top center) */
        layout.tradeRow = {
            x: basesWidth + padding,
            y: statusHeight + padding * 2,
            w: gameWidth - basesWidth * 2,
            h: cardSize.h + padding * 2
        };

        /* Player's hand (bottom center) */
        layout.hand = {
            x: basesWidth + padding,
            y: h - handHeight - padding,
            w: gameWidth - basesWidth * 2,
            h: handHeight
        };

        /* Player's bases (bottom left) */
        layout.playerBases = {
            x: padding,
            y: h - handHeight - padding,
            w: basesWidth,
            h: handHeight
        };

        /* Opponent's bases (top left) */
        layout.opponentBases = {
            x: padding,
            y: statusHeight + padding * 2,
            w: basesWidth,
            h: cardSize.h + padding * 2
        };

        /* Play area (center) */
        const playTop = layout.tradeRow.y + layout.tradeRow.h + padding;
        const playBottom = layout.hand.y - padding;
        layout.playArea = {
            x: padding,
            y: playTop,
            w: gameWidth,
            h: playBottom - playTop
        };

        /* Narrative panel (right side) */
        layout.narrative = {
            x: w - narrativeWidth - padding,
            y: statusHeight + padding * 2,
            w: narrativeWidth,
            h: h - statusHeight - padding * 3
        };

        /* Adjust card size for small screens */
        if (w < 1000) {
            cardSize.w = 80;
            cardSize.h = 112;
        } else {
            cardSize.w = 100;
            cardSize.h = 140;
        }
    }
    /* }}} */

})();
