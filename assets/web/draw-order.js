/**
 * draw-order.js - Draw order selection UI for Symbeline Realms
 *
 * Provides a specialized UI for the draw order selection phase that
 * shows card backs and allows click-to-select ordering. Integrates
 * with the input handler for submitting the selection.
 */

(function() {
    'use strict';

    /* {{{ Constants */
    const CARD_WIDTH = 80;
    const CARD_HEIGHT = 112;
    const CARD_GAP = 20;
    /* }}} */

    /* {{{ State */
    const state = {
        active: false,
        cardCount: 0,
        selectedOrder: [],
        cardBounds: [],
        hoveredIndex: -1,
        onConfirm: null,
        onCancel: null
    };
    /* }}} */

    /* {{{ Public API - window.drawOrderUI */
    window.drawOrderUI = {

        /* {{{ show
         * Show the draw order selection UI.
         * @param cardCount Number of cards to order
         * @param onConfirm Callback with order array
         * @param onCancel Callback for cancel
         */
        show: function(cardCount, onConfirm, onCancel) {
            state.active = true;
            state.cardCount = cardCount;
            state.selectedOrder = [];
            state.cardBounds = [];
            state.hoveredIndex = -1;
            state.onConfirm = onConfirm;
            state.onCancel = onCancel;
        },
        /* }}} */

        /* {{{ hide */
        hide: function() {
            state.active = false;
            state.selectedOrder = [];
            state.cardBounds = [];
        },
        /* }}} */

        /* {{{ isActive */
        isActive: function() {
            return state.active;
        },
        /* }}} */

        /* {{{ handleClick
         * Handle click on the draw order UI.
         * @param x Click x coordinate
         * @param y Click y coordinate
         * @returns true if click was handled
         */
        handleClick: function(x, y) {
            if (!state.active) return false;

            /* Check card bounds */
            for (let i = 0; i < state.cardBounds.length; i++) {
                const bounds = state.cardBounds[i];
                if (x >= bounds.x && x < bounds.x + bounds.w &&
                    y >= bounds.y && y < bounds.y + bounds.h) {

                    /* Check if already selected */
                    const selIdx = state.selectedOrder.indexOf(i);
                    if (selIdx >= 0) {
                        /* Deselect - remove from order */
                        state.selectedOrder.splice(selIdx, 1);
                    } else {
                        /* Select - add to order */
                        state.selectedOrder.push(i);

                        /* Auto-confirm when all cards selected */
                        if (state.selectedOrder.length >= state.cardCount) {
                            this.confirm();
                        }
                    }
                    return true;
                }
            }

            return false;
        },
        /* }}} */

        /* {{{ handleMouseMove
         * Handle mouse move for hover effect.
         */
        handleMouseMove: function(x, y) {
            if (!state.active) return;

            state.hoveredIndex = -1;
            for (let i = 0; i < state.cardBounds.length; i++) {
                const bounds = state.cardBounds[i];
                if (x >= bounds.x && x < bounds.x + bounds.w &&
                    y >= bounds.y && y < bounds.y + bounds.h) {
                    state.hoveredIndex = i;
                    break;
                }
            }
        },
        /* }}} */

        /* {{{ confirm
         * Confirm the current selection.
         */
        confirm: function() {
            if (state.selectedOrder.length === 0) return;

            /* If not all cards selected, fill remaining with sequential order */
            const order = state.selectedOrder.slice();
            for (let i = 0; i < state.cardCount; i++) {
                if (!order.includes(i)) {
                    order.push(i);
                }
            }

            if (state.onConfirm) {
                state.onConfirm(order);
            }

            this.hide();
        },
        /* }}} */

        /* {{{ useDefault
         * Use default sequential order.
         */
        useDefault: function() {
            const order = [];
            for (let i = 0; i < state.cardCount; i++) {
                order.push(i);
            }

            if (state.onConfirm) {
                state.onConfirm(order);
            }

            this.hide();
        },
        /* }}} */

        /* {{{ cancel */
        cancel: function() {
            if (state.onCancel) {
                state.onCancel();
            }
            this.hide();
        },
        /* }}} */

        /* {{{ getSelectedOrder */
        getSelectedOrder: function() {
            return state.selectedOrder.slice();
        },
        /* }}} */

        /* {{{ render
         * Render the draw order selection UI.
         */
        render: function(ctx, canvasWidth, canvasHeight) {
            if (!state.active) return;

            /* Darken background */
            ctx.fillStyle = 'rgba(0, 0, 0, 0.8)';
            ctx.fillRect(0, 0, canvasWidth, canvasHeight);

            /* Calculate layout */
            const totalWidth = state.cardCount * CARD_WIDTH + (state.cardCount - 1) * CARD_GAP;
            const startX = (canvasWidth - totalWidth) / 2;
            const cardY = canvasHeight / 2 - CARD_HEIGHT / 2 - 30;

            /* Title */
            ctx.fillStyle = '#d4af37';
            ctx.font = 'bold 24px Georgia, serif';
            ctx.textAlign = 'center';
            ctx.fillText('Choose Your Draw Order', canvasWidth / 2, cardY - 60);

            /* Instructions */
            ctx.fillStyle = '#aaa';
            ctx.font = '14px sans-serif';
            ctx.fillText('Click cards in the order you want to draw them', canvasWidth / 2, cardY - 30);

            /* Clear card bounds */
            state.cardBounds = [];

            /* Draw cards */
            for (let i = 0; i < state.cardCount; i++) {
                const x = startX + i * (CARD_WIDTH + CARD_GAP);
                const y = cardY;

                state.cardBounds.push({ x: x, y: y, w: CARD_WIDTH, h: CARD_HEIGHT });

                /* Card back */
                this._renderCardBack(ctx, x, y, CARD_WIDTH, CARD_HEIGHT,
                    i, state.hoveredIndex === i);
            }

            /* Position labels */
            ctx.font = '12px sans-serif';
            ctx.fillStyle = '#888';
            ctx.textAlign = 'center';
            for (let i = 0; i < state.cardCount; i++) {
                const x = startX + i * (CARD_WIDTH + CARD_GAP) + CARD_WIDTH / 2;
                ctx.fillText('Position ' + (i + 1), x, cardY + CARD_HEIGHT + 20);
            }

            /* Selected order display */
            const selectedY = cardY + CARD_HEIGHT + 50;
            ctx.fillStyle = '#d4af37';
            ctx.font = '16px sans-serif';
            ctx.textAlign = 'center';

            if (state.selectedOrder.length > 0) {
                const orderStr = state.selectedOrder.map(function(i) {
                    return i + 1;
                }).join(' \u2192 ');
                ctx.fillText('Draw order: ' + orderStr, canvasWidth / 2, selectedY);
            } else {
                ctx.fillStyle = '#666';
                ctx.fillText('Click cards to set draw order', canvasWidth / 2, selectedY);
            }

            /* Buttons */
            const buttonY = selectedY + 40;
            const buttonWidth = 120;
            const buttonHeight = 36;
            const buttonGap = 20;

            /* Confirm button */
            const confirmX = canvasWidth / 2 - buttonWidth - buttonGap / 2;
            const canConfirm = state.selectedOrder.length > 0;

            ctx.fillStyle = canConfirm ? '#2d7a2d' : '#444';
            ctx.fillRect(confirmX, buttonY, buttonWidth, buttonHeight);
            ctx.strokeStyle = canConfirm ? '#4caf50' : '#666';
            ctx.lineWidth = 1;
            ctx.strokeRect(confirmX, buttonY, buttonWidth, buttonHeight);

            ctx.fillStyle = canConfirm ? '#fff' : '#888';
            ctx.font = '14px sans-serif';
            ctx.textAlign = 'center';
            ctx.fillText('Confirm', confirmX + buttonWidth / 2, buttonY + buttonHeight / 2 + 5);

            /* Default Order button */
            const defaultX = canvasWidth / 2 + buttonGap / 2;

            ctx.fillStyle = '#444';
            ctx.fillRect(defaultX, buttonY, buttonWidth, buttonHeight);
            ctx.strokeStyle = '#666';
            ctx.strokeRect(defaultX, buttonY, buttonWidth, buttonHeight);

            ctx.fillStyle = '#aaa';
            ctx.fillText('Default Order', defaultX + buttonWidth / 2, buttonY + buttonHeight / 2 + 5);

            /* Store button bounds for click handling */
            state.confirmBounds = { x: confirmX, y: buttonY, w: buttonWidth, h: buttonHeight };
            state.defaultBounds = { x: defaultX, y: buttonY, w: buttonWidth, h: buttonHeight };
        },
        /* }}} */

        /* {{{ _renderCardBack
         * Render a single card back.
         */
        _renderCardBack: function(ctx, x, y, w, h, index, hovered) {
            const selected = state.selectedOrder.includes(index);
            const selectionIndex = state.selectedOrder.indexOf(index);

            /* Lift selected/hovered cards */
            const lift = (selected || hovered) ? -8 : 0;
            const drawY = y + lift;

            /* Card shadow */
            ctx.fillStyle = 'rgba(0, 0, 0, 0.3)';
            ctx.fillRect(x + 3, drawY + 3, w, h);

            /* Card background */
            ctx.fillStyle = selected ? '#2a3a4a' : (hovered ? '#3a3a5a' : '#2a2a3a');
            ctx.fillRect(x, drawY, w, h);

            /* Card border */
            ctx.strokeStyle = selected ? '#d4af37' : (hovered ? '#8866cc' : '#555');
            ctx.lineWidth = selected ? 3 : 2;
            ctx.strokeRect(x, drawY, w, h);

            /* Card pattern (question mark) */
            ctx.fillStyle = '#444';
            ctx.font = 'bold 48px serif';
            ctx.textAlign = 'center';
            ctx.textBaseline = 'middle';
            ctx.fillText('?', x + w / 2, drawY + h / 2);
            ctx.textBaseline = 'alphabetic';

            /* Selection order badge */
            if (selected) {
                const badgeX = x + w - 18;
                const badgeY = drawY + 18;
                const badgeR = 14;

                /* Badge background */
                ctx.fillStyle = '#d4af37';
                ctx.beginPath();
                ctx.arc(badgeX, badgeY, badgeR, 0, Math.PI * 2);
                ctx.fill();

                /* Badge number */
                ctx.fillStyle = '#000';
                ctx.font = 'bold 14px sans-serif';
                ctx.textAlign = 'center';
                ctx.textBaseline = 'middle';
                ctx.fillText((selectionIndex + 1).toString(), badgeX, badgeY);
                ctx.textBaseline = 'alphabetic';
            }
        },
        /* }}} */

        /* {{{ handleButtonClick
         * Handle click on UI buttons.
         */
        handleButtonClick: function(x, y) {
            if (!state.active) return false;

            /* Check confirm button */
            if (state.confirmBounds &&
                x >= state.confirmBounds.x && x < state.confirmBounds.x + state.confirmBounds.w &&
                y >= state.confirmBounds.y && y < state.confirmBounds.y + state.confirmBounds.h) {
                if (state.selectedOrder.length > 0) {
                    this.confirm();
                    return true;
                }
            }

            /* Check default button */
            if (state.defaultBounds &&
                x >= state.defaultBounds.x && x < state.defaultBounds.x + state.defaultBounds.w &&
                y >= state.defaultBounds.y && y < state.defaultBounds.y + state.defaultBounds.h) {
                this.useDefault();
                return true;
            }

            return false;
        }
        /* }}} */

    };
    /* }}} */

})();
