/**
 * zone-renderer.js - Game zone rendering for Symbeline Realms browser client
 *
 * Provides functions to render game zones (hand, trade row, bases, etc.)
 * and track click bounds for interactive elements.
 */

(function() {
    'use strict';

    /* {{{ Zone state */
    const state = {
        hoveredCard: null,      /* Card object currently hovered */
        selectedCards: [],      /* Array of selected card objects */
        clickBounds: [],        /* Array of { card, zone, x, y, w, h } */
        handOffset: 0,          /* Scroll offset for large hands */
        tradeRowOffset: 0       /* Scroll offset for trade row */
    };
    /* }}} */

    /* {{{ Constants */
    const CARD_SPACING = 10;
    const HOVER_LIFT = 20;      /* Pixels card lifts on hover */
    const WANDERER_COST = 2;    /* Explorer cost */
    /* }}} */

    /* {{{ Public API - window.zoneRenderer */
    window.zoneRenderer = {

        /* {{{ getState
         * Returns reference to zone state for external access.
         */
        getState: function() {
            return state;
        },
        /* }}} */

        /* {{{ clearClickBounds
         * Reset click bounds before each frame.
         */
        clearClickBounds: function() {
            state.clickBounds = [];
        },
        /* }}} */

        /* {{{ setHoveredCard
         * Set the currently hovered card.
         */
        setHoveredCard: function(card) {
            state.hoveredCard = card;
        },
        /* }}} */

        /* {{{ setSelectedCards
         * Set the selected cards array.
         */
        setSelectedCards: function(cards) {
            state.selectedCards = cards || [];
        },
        /* }}} */

        /* {{{ toggleCardSelection
         * Toggle a card's selection state.
         */
        toggleCardSelection: function(card) {
            const idx = state.selectedCards.indexOf(card);
            if (idx >= 0) {
                state.selectedCards.splice(idx, 1);
            } else {
                state.selectedCards.push(card);
            }
        },
        /* }}} */

        /* {{{ getCardAtPoint
         * Find card at given x,y coordinates.
         */
        getCardAtPoint: function(x, y) {
            /* Check in reverse order (top cards first) */
            for (let i = state.clickBounds.length - 1; i >= 0; i--) {
                const b = state.clickBounds[i];
                if (x >= b.x && x <= b.x + b.w && y >= b.y && y <= b.y + b.h) {
                    return { card: b.card, zone: b.zone, index: b.index };
                }
            }
            return null;
        },
        /* }}} */

        /* {{{ renderHand
         * Renders the player's hand with hover effects and centering.
         *
         * @param ctx - Canvas 2D context
         * @param hand - Array of card objects
         * @param region - { x, y, w, h } layout region
         */
        renderHand: function(ctx, hand, region) {
            if (!hand || hand.length === 0) return;

            const cardSize = window.cardRenderer.getCardSize();
            const totalWidth = hand.length * (cardSize.width + CARD_SPACING) - CARD_SPACING;
            let startX = region.x + (region.w - totalWidth) / 2;

            /* Clamp to region bounds */
            if (startX < region.x + 10) startX = region.x + 10;

            for (let i = 0; i < hand.length; i++) {
                const card = hand[i];
                const x = startX + i * (cardSize.width + CARD_SPACING);
                const baseY = region.y + 10;

                /* Hover effect: card lifts up */
                const hovered = state.hoveredCard === card;
                const selected = state.selectedCards.includes(card);
                const yOffset = hovered ? -HOVER_LIFT : 0;
                const y = baseY + yOffset;

                window.cardRenderer.renderCard(ctx, card, x, y, { hovered, selected });

                /* Store click bounds */
                state.clickBounds.push({
                    card: card,
                    zone: 'hand',
                    index: i,
                    x: x,
                    y: y,
                    w: cardSize.width,
                    h: cardSize.height
                });
            }

            /* Hand count label */
            ctx.fillStyle = '#666';
            ctx.font = '11px sans-serif';
            ctx.textAlign = 'right';
            ctx.fillText('Hand: ' + hand.length, region.x + region.w - 10, region.y + region.h - 8);
        },
        /* }}} */

        /* {{{ renderTradeRow
         * Renders the trade row with available cards for purchase.
         *
         * @param ctx - Canvas 2D context
         * @param tradeRow - Array of card objects (null for empty slots)
         * @param region - { x, y, w, h } layout region
         * @param playerTrade - Current trade available to player
         */
        renderTradeRow: function(ctx, tradeRow, region, playerTrade) {
            playerTrade = playerTrade || 0;

            /* Region background */
            ctx.fillStyle = 'rgba(0, 0, 0, 0.3)';
            ctx.fillRect(region.x, region.y, region.w, region.h);

            /* Header */
            ctx.fillStyle = '#d4af37';
            ctx.font = 'bold 14px sans-serif';
            ctx.textAlign = 'left';
            ctx.fillText('Trade Row', region.x + 10, region.y + 18);

            const cardSize = window.cardRenderer.getCardSize();
            const startY = region.y + 28;

            /* Draw trade row cards */
            if (tradeRow) {
                for (let i = 0; i < tradeRow.length; i++) {
                    const card = tradeRow[i];
                    const x = region.x + 10 + i * (cardSize.width + CARD_SPACING);
                    const y = startY;

                    if (card === null) {
                        /* Empty slot indicator */
                        ctx.strokeStyle = '#333';
                        ctx.setLineDash([4, 4]);
                        ctx.strokeRect(x, y, cardSize.width, cardSize.height);
                        ctx.setLineDash([]);
                        continue;
                    }

                    const canAfford = playerTrade >= (card.cost || 0);
                    const hovered = state.hoveredCard === card;
                    const dimmed = !canAfford;

                    window.cardRenderer.renderCard(ctx, card, x, y, {
                        hovered: hovered,
                        scale: dimmed ? 1 : 1  /* Could dim by reducing opacity */
                    });

                    /* Dim overlay if can't afford */
                    if (dimmed) {
                        ctx.fillStyle = 'rgba(0, 0, 0, 0.5)';
                        ctx.fillRect(x, y, cardSize.width, cardSize.height);
                    }

                    /* Store click bounds */
                    state.clickBounds.push({
                        card: card,
                        zone: 'tradeRow',
                        index: i,
                        x: x,
                        y: y,
                        w: cardSize.width,
                        h: cardSize.height
                    });
                }
            }

            /* Wanderer/Explorer slot */
            this.renderWandererSlot(ctx, region, playerTrade);
        },
        /* }}} */

        /* {{{ renderWandererSlot
         * Renders the always-available Explorer/Wanderer purchase option.
         */
        renderWandererSlot: function(ctx, region, playerTrade) {
            const cardSize = window.cardRenderer.getCardSize();
            const x = region.x + region.w - cardSize.width - 20;
            const y = region.y + 28;

            /* Wanderer card data */
            const wanderer = {
                name: 'Explorer',
                cost: WANDERER_COST,
                faction: 'neutral',
                kind: 'ship',
                effects: [{ type: 'trade', value: 2 }]
            };

            const canAfford = playerTrade >= WANDERER_COST;
            const hovered = state.hoveredCard === wanderer;

            window.cardRenderer.renderCard(ctx, wanderer, x, y, { hovered });

            if (!canAfford) {
                ctx.fillStyle = 'rgba(0, 0, 0, 0.5)';
                ctx.fillRect(x, y, cardSize.width, cardSize.height);
            }

            /* "Always Available" label */
            ctx.fillStyle = '#666';
            ctx.font = '9px sans-serif';
            ctx.textAlign = 'center';
            ctx.fillText('Always', x + cardSize.width / 2, y + cardSize.height + 12);

            state.clickBounds.push({
                card: wanderer,
                zone: 'wanderer',
                index: 0,
                x: x,
                y: y,
                w: cardSize.width,
                h: cardSize.height
            });
        },
        /* }}} */

        /* {{{ renderBases
         * Renders bases for player or opponent.
         *
         * @param ctx - Canvas 2D context
         * @param bases - Array of base card objects
         * @param region - { x, y, w, h } layout region
         * @param isOpponent - True if rendering opponent's bases
         */
        renderBases: function(ctx, bases, region, isOpponent) {
            /* Region background */
            ctx.fillStyle = 'rgba(0, 0, 0, 0.3)';
            ctx.fillRect(region.x, region.y, region.w, region.h);

            /* Label */
            const label = isOpponent ? "Opponent's Bases" : "Your Bases";
            ctx.fillStyle = isOpponent ? '#cc6666' : '#66cc66';
            ctx.font = 'bold 12px sans-serif';
            ctx.textAlign = 'left';
            ctx.fillText(label, region.x + 10, region.y + 16);

            if (!bases || bases.length === 0) {
                ctx.fillStyle = '#444';
                ctx.font = '11px sans-serif';
                ctx.fillText('No bases in play', region.x + 10, region.y + 40);
                return;
            }

            const cardSize = window.cardRenderer.getCardSize();
            const baseScale = 0.8;  /* Slightly smaller for bases */
            const scaledWidth = cardSize.width * baseScale;
            const scaledHeight = cardSize.height * baseScale;
            const verticalSpacing = scaledHeight * 0.6 + 10;

            for (let i = 0; i < bases.length; i++) {
                const base = bases[i];
                const x = region.x + 10;
                const y = region.y + 24 + i * verticalSpacing;

                const hovered = state.hoveredCard === base;
                const selected = state.selectedCards.includes(base);

                window.cardRenderer.renderCard(ctx, base, x, y, {
                    hovered,
                    selected,
                    scale: baseScale
                });

                /* Defense indicator next to base */
                if (base.defense !== undefined) {
                    const defX = x + scaledWidth + 8;
                    const defY = y + scaledHeight / 2;

                    ctx.fillStyle = base.isOutpost ? '#cc3333' : '#666';
                    ctx.beginPath();
                    ctx.arc(defX, defY, 12, 0, Math.PI * 2);
                    ctx.fill();

                    ctx.fillStyle = '#fff';
                    ctx.font = 'bold 11px sans-serif';
                    ctx.textAlign = 'center';
                    ctx.textBaseline = 'middle';
                    ctx.fillText(base.defense.toString(), defX, defY);
                    ctx.textBaseline = 'alphabetic';
                }

                /* Store click bounds */
                state.clickBounds.push({
                    card: base,
                    zone: isOpponent ? 'opponentBases' : 'playerBases',
                    index: i,
                    x: x,
                    y: y,
                    w: scaledWidth,
                    h: scaledHeight
                });
            }
        },
        /* }}} */

        /* {{{ renderPlayedCards
         * Renders cards played this turn in the center play area.
         *
         * @param ctx - Canvas 2D context
         * @param playedCards - Array of card objects played this turn
         * @param region - { x, y, w, h } layout region
         */
        renderPlayedCards: function(ctx, playedCards, region) {
            if (!playedCards || playedCards.length === 0) {
                /* Empty play area indicator */
                ctx.fillStyle = '#222';
                ctx.font = '14px sans-serif';
                ctx.textAlign = 'center';
                ctx.fillText('Play Area', region.x + region.w / 2, region.y + region.h / 2);
                return;
            }

            const cardSize = window.cardRenderer.getCardSize();
            const scale = 0.9;
            const scaledWidth = cardSize.width * scale;

            /* Fan out cards horizontally */
            const totalWidth = playedCards.length * (scaledWidth * 0.7);
            let startX = region.x + (region.w - totalWidth) / 2;
            const y = region.y + (region.h - cardSize.height * scale) / 2;

            for (let i = 0; i < playedCards.length; i++) {
                const card = playedCards[i];
                const x = startX + i * (scaledWidth * 0.7);
                const hovered = state.hoveredCard === card;

                window.cardRenderer.renderCard(ctx, card, x, y, {
                    hovered,
                    scale: scale
                });

                state.clickBounds.push({
                    card: card,
                    zone: 'playArea',
                    index: i,
                    x: x,
                    y: y,
                    w: scaledWidth,
                    h: cardSize.height * scale
                });
            }
        },
        /* }}} */

        /* {{{ renderDeckIndicator
         * Renders deck remaining count.
         *
         * @param ctx - Canvas 2D context
         * @param deckCount - Number of cards remaining in deck
         * @param x, y - Position to render
         */
        renderDeckIndicator: function(ctx, deckCount, x, y) {
            const size = 50;

            /* Deck shape (stacked cards) */
            ctx.fillStyle = '#1a1a2e';
            for (let i = 2; i >= 0; i--) {
                ctx.fillRect(x + i * 2, y + i * 2, size - 10, size * 1.4 - 10);
            }

            /* Border */
            ctx.strokeStyle = '#444';
            ctx.lineWidth = 2;
            ctx.strokeRect(x, y, size - 10, size * 1.4 - 10);

            /* Count */
            ctx.fillStyle = '#888';
            ctx.font = 'bold 16px sans-serif';
            ctx.textAlign = 'center';
            ctx.fillText(deckCount.toString(), x + (size - 10) / 2, y + size * 0.7);

            ctx.fillStyle = '#666';
            ctx.font = '9px sans-serif';
            ctx.fillText('DECK', x + (size - 10) / 2, y + size * 1.1);
        },
        /* }}} */

        /* {{{ renderDiscardIndicator
         * Renders discard pile count.
         *
         * @param ctx - Canvas 2D context
         * @param discardCount - Number of cards in discard
         * @param x, y - Position to render
         * @param topCard - Optional top card of discard to show
         */
        renderDiscardIndicator: function(ctx, discardCount, x, y, topCard) {
            const size = 50;

            if (discardCount === 0) {
                /* Empty discard placeholder */
                ctx.strokeStyle = '#333';
                ctx.setLineDash([4, 4]);
                ctx.strokeRect(x, y, size - 10, size * 1.4 - 10);
                ctx.setLineDash([]);

                ctx.fillStyle = '#444';
                ctx.font = '9px sans-serif';
                ctx.textAlign = 'center';
                ctx.fillText('DISCARD', x + (size - 10) / 2, y + size * 0.7);
                return;
            }

            /* Show mini version of top card or generic pile */
            if (topCard) {
                window.cardRenderer.renderCard(ctx, topCard, x, y, { scale: 0.4 });
            } else {
                ctx.fillStyle = '#2a2a2e';
                ctx.fillRect(x, y, size - 10, size * 1.4 - 10);
                ctx.strokeStyle = '#555';
                ctx.lineWidth = 1;
                ctx.strokeRect(x, y, size - 10, size * 1.4 - 10);
            }

            /* Count badge */
            ctx.fillStyle = '#666';
            ctx.beginPath();
            ctx.arc(x + size - 15, y + 10, 12, 0, Math.PI * 2);
            ctx.fill();

            ctx.fillStyle = '#fff';
            ctx.font = 'bold 10px sans-serif';
            ctx.textAlign = 'center';
            ctx.textBaseline = 'middle';
            ctx.fillText(discardCount.toString(), x + size - 15, y + 10);
            ctx.textBaseline = 'alphabetic';

            ctx.fillStyle = '#666';
            ctx.font = '9px sans-serif';
            ctx.fillText('DISCARD', x + (size - 10) / 2, y + size * 1.4);
        },
        /* }}} */

        /* {{{ renderResourceBar
         * Renders current turn resources (trade, combat).
         *
         * @param ctx - Canvas 2D context
         * @param trade - Current trade value
         * @param combat - Current combat value
         * @param region - { x, y, w, h } status region
         */
        renderResourceBar: function(ctx, trade, combat, region) {
            const y = region.y + region.h - 30;

            /* Trade */
            ctx.fillStyle = '#d4a017';
            ctx.font = 'bold 14px sans-serif';
            ctx.textAlign = 'left';
            ctx.fillText('Trade: ' + trade, region.x + 10, y);

            /* Combat */
            ctx.fillStyle = '#cc66cc';
            ctx.fillText('Combat: ' + combat, region.x + 100, y);
        }
        /* }}} */

    };
    /* }}} */

})();
