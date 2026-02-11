/**
 * input-handler.js - Browser input handling for Symbeline Realms
 *
 * Handles mouse/touch input, translates to game actions, and manages
 * WebSocket communication with the server. Also provides draw order
 * selection UI and action confirmation dialogs.
 */

(function() {
    'use strict';

    /* {{{ State */
    const state = {
        websocket: null,
        connected: false,
        gamePhase: 'main',
        pendingAction: null,
        drawOrderSelection: [],
        drawOrderRequired: 0,
        errorMessage: null,
        errorTimeout: null,
        actionCallbacks: {}
    };
    /* }}} */

    /* {{{ Action types */
    const ACTION_TYPES = {
        PLAY_CARD: 'play_card',
        BUY_CARD: 'buy_card',
        BUY_WANDERER: 'buy_wanderer',
        ATTACK_PLAYER: 'attack_player',
        ATTACK_BASE: 'attack_base',
        SCRAP_CARD: 'scrap_card',
        ACTIVATE_BASE: 'activate_base',
        SET_DRAW_ORDER: 'set_draw_order',
        END_TURN: 'end_turn'
    };
    /* }}} */

    /* {{{ Public API - window.inputHandler */
    window.inputHandler = {

        /* {{{ init
         * Initialize input handling on the canvas.
         */
        init: function(canvas) {
            if (!canvas) return false;

            /* Mouse move for hover */
            canvas.addEventListener('mousemove', this.handleMouseMove.bind(this));

            /* Click for actions */
            canvas.addEventListener('click', this.handleClick.bind(this));

            /* Right click for dismiss/cancel */
            canvas.addEventListener('contextmenu', this.handleRightClick.bind(this));

            /* Touch support */
            canvas.addEventListener('touchstart', this.handleTouchStart.bind(this));
            canvas.addEventListener('touchend', this.handleTouchEnd.bind(this));

            /* Keyboard shortcuts */
            document.addEventListener('keydown', this.handleKeyDown.bind(this));

            return true;
        },
        /* }}} */

        /* {{{ connect
         * Connect to WebSocket server.
         */
        connect: function(url) {
            if (state.websocket) {
                state.websocket.close();
            }

            state.websocket = new WebSocket(url);

            state.websocket.onopen = function() {
                state.connected = true;
                console.log('Connected to server');
            };

            state.websocket.onclose = function() {
                state.connected = false;
                console.log('Disconnected from server');
            };

            state.websocket.onerror = function(err) {
                console.error('WebSocket error:', err);
                this.showError('Connection error');
            }.bind(this);

            state.websocket.onmessage = function(event) {
                this.handleServerMessage(event.data);
            }.bind(this);
        },
        /* }}} */

        /* {{{ disconnect */
        disconnect: function() {
            if (state.websocket) {
                state.websocket.close();
                state.websocket = null;
            }
            state.connected = false;
        },
        /* }}} */

        /* {{{ isConnected */
        isConnected: function() {
            return state.connected;
        },
        /* }}} */

        /* {{{ handleMouseMove
         * Update hover state based on mouse position.
         */
        handleMouseMove: function(e) {
            const rect = e.target.getBoundingClientRect();
            const x = e.clientX - rect.left;
            const y = e.clientY - rect.top;

            const hit = window.zoneRenderer.getCardAtPoint(x, y);
            window.zoneRenderer.setHoveredCard(hit ? hit.card : null);
        },
        /* }}} */

        /* {{{ handleClick
         * Process click and generate appropriate action.
         */
        handleClick: function(e) {
            const rect = e.target.getBoundingClientRect();
            const x = e.clientX - rect.left;
            const y = e.clientY - rect.top;

            const hit = window.zoneRenderer.getCardAtPoint(x, y);

            if (!hit) {
                /* Click on empty area - clear selection */
                window.zoneRenderer.setSelectedCards([]);
                return;
            }

            /* Handle based on zone */
            switch (hit.zone) {
                case 'hand':
                    this.handleHandClick(hit.card, hit.index);
                    break;

                case 'tradeRow':
                    this.handleTradeRowClick(hit.card, hit.index);
                    break;

                case 'wanderer':
                    this.handleWandererClick();
                    break;

                case 'playerBases':
                    this.handlePlayerBaseClick(hit.card, hit.index);
                    break;

                case 'opponentBases':
                    this.handleOpponentBaseClick(hit.card, hit.index);
                    break;

                case 'playArea':
                    this.handlePlayAreaClick(hit.card, hit.index);
                    break;
            }
        },
        /* }}} */

        /* {{{ handleRightClick
         * Handle right-click for cancel/dismiss.
         */
        handleRightClick: function(e) {
            e.preventDefault();

            /* Clear any pending action */
            state.pendingAction = null;
            window.zoneRenderer.setSelectedCards([]);

            /* In draw order mode, remove last selection */
            if (state.gamePhase === 'draw_order' && state.drawOrderSelection.length > 0) {
                state.drawOrderSelection.pop();
            }
        },
        /* }}} */

        /* {{{ handleTouchStart */
        handleTouchStart: function(e) {
            /* Store touch start for gesture detection */
            if (e.touches.length === 1) {
                e.target._touchStart = {
                    x: e.touches[0].clientX,
                    y: e.touches[0].clientY,
                    time: Date.now()
                };
            }
        },
        /* }}} */

        /* {{{ handleTouchEnd */
        handleTouchEnd: function(e) {
            if (!e.target._touchStart) return;

            const start = e.target._touchStart;
            const end = e.changedTouches[0];
            const dx = end.clientX - start.x;
            const dy = end.clientY - start.y;
            const dt = Date.now() - start.time;

            /* Tap detection (short, small movement) */
            if (dt < 300 && Math.abs(dx) < 10 && Math.abs(dy) < 10) {
                /* Simulate click */
                const rect = e.target.getBoundingClientRect();
                const hit = window.zoneRenderer.getCardAtPoint(
                    end.clientX - rect.left,
                    end.clientY - rect.top
                );

                if (hit) {
                    this.handleClick({
                        target: e.target,
                        clientX: end.clientX,
                        clientY: end.clientY
                    });
                }
            }

            delete e.target._touchStart;
        },
        /* }}} */

        /* {{{ handleKeyDown
         * Keyboard shortcuts.
         */
        handleKeyDown: function(e) {
            switch (e.key) {
                case 'Escape':
                    /* Cancel action */
                    state.pendingAction = null;
                    window.zoneRenderer.setSelectedCards([]);
                    break;

                case 'e':
                case 'E':
                    /* End turn */
                    if (!e.ctrlKey && !e.metaKey) {
                        this.sendAction(ACTION_TYPES.END_TURN, {});
                    }
                    break;

                case 'a':
                case 'A':
                    /* Attack player */
                    if (!e.ctrlKey && !e.metaKey) {
                        this.sendAction(ACTION_TYPES.ATTACK_PLAYER, {});
                    }
                    break;
            }
        },
        /* }}} */

        /* {{{ Zone click handlers */

        handleHandClick: function(card, index) {
            /* In main phase, clicking hand card plays it */
            if (state.gamePhase === 'main') {
                this.sendAction(ACTION_TYPES.PLAY_CARD, { card_index: index });
            }
        },

        handleTradeRowClick: function(card, index) {
            /* Buy card from trade row */
            if (state.gamePhase === 'main') {
                this.sendAction(ACTION_TYPES.BUY_CARD, { slot_index: index });
            }
        },

        handleWandererClick: function() {
            /* Buy wanderer/explorer */
            if (state.gamePhase === 'main') {
                this.sendAction(ACTION_TYPES.BUY_WANDERER, {});
            }
        },

        handlePlayerBaseClick: function(card, index) {
            /* Activate base ability */
            if (state.gamePhase === 'main') {
                this.sendAction(ACTION_TYPES.ACTIVATE_BASE, { base_index: index });
            }
        },

        handleOpponentBaseClick: function(card, index) {
            /* Attack opponent's base */
            if (state.gamePhase === 'main') {
                this.sendAction(ACTION_TYPES.ATTACK_BASE, { base_index: index });
            }
        },

        handlePlayAreaClick: function(card, index) {
            /* In draw order phase, add to selection */
            if (state.gamePhase === 'draw_order') {
                if (!state.drawOrderSelection.includes(index)) {
                    state.drawOrderSelection.push(index);

                    /* If we have enough, submit */
                    if (state.drawOrderSelection.length >= state.drawOrderRequired) {
                        this.sendAction(ACTION_TYPES.SET_DRAW_ORDER, {
                            order: state.drawOrderSelection
                        });
                        state.drawOrderSelection = [];
                    }
                }
            }
        },

        /* }}} */

        /* {{{ sendAction
         * Send action to server via WebSocket.
         */
        sendAction: function(actionType, data) {
            const message = {
                type: 'action',
                action: actionType,
                ...data
            };

            if (state.connected && state.websocket) {
                state.websocket.send(JSON.stringify(message));
            } else {
                /* Demo mode - just log */
                console.log('Action (demo):', message);

                /* Trigger callback if registered */
                if (state.actionCallbacks[actionType]) {
                    state.actionCallbacks[actionType](data);
                }
            }
        },
        /* }}} */

        /* {{{ handleServerMessage
         * Process messages from server.
         */
        handleServerMessage: function(data) {
            let message;
            try {
                message = JSON.parse(data);
            } catch (e) {
                console.error('Invalid JSON from server:', data);
                return;
            }

            switch (message.type) {
                case 'game_state':
                    /* Update game state */
                    if (message.phase) {
                        state.gamePhase = message.phase;
                    }
                    break;

                case 'error':
                    this.showError(message.message || 'Unknown error');
                    break;

                case 'draw_order':
                    /* Enter draw order selection mode */
                    state.gamePhase = 'draw_order';
                    state.drawOrderRequired = message.count || 5;
                    state.drawOrderSelection = [];
                    break;

                case 'action_result':
                    /* Action succeeded or failed */
                    if (message.success === false) {
                        this.showError(message.error || 'Action failed');
                    }
                    break;
            }
        },
        /* }}} */

        /* {{{ showError
         * Display error message overlay.
         */
        showError: function(message) {
            state.errorMessage = message;

            /* Clear after 3 seconds */
            if (state.errorTimeout) {
                clearTimeout(state.errorTimeout);
            }
            state.errorTimeout = setTimeout(function() {
                state.errorMessage = null;
            }, 3000);
        },
        /* }}} */

        /* {{{ getErrorMessage */
        getErrorMessage: function() {
            return state.errorMessage;
        },
        /* }}} */

        /* {{{ setGamePhase */
        setGamePhase: function(phase) {
            state.gamePhase = phase;
        },
        /* }}} */

        /* {{{ getGamePhase */
        getGamePhase: function() {
            return state.gamePhase;
        },
        /* }}} */

        /* {{{ getDrawOrderSelection */
        getDrawOrderSelection: function() {
            return state.drawOrderSelection.slice();
        },
        /* }}} */

        /* {{{ setDrawOrderRequired */
        setDrawOrderRequired: function(count) {
            state.drawOrderRequired = count;
        },
        /* }}} */

        /* {{{ onAction
         * Register callback for action (for demo/testing).
         */
        onAction: function(actionType, callback) {
            state.actionCallbacks[actionType] = callback;
        },
        /* }}} */

        /* {{{ renderError
         * Render error message overlay.
         */
        renderError: function(ctx, region) {
            if (!state.errorMessage) return;

            const text = state.errorMessage;
            ctx.font = 'bold 14px sans-serif';
            const metrics = ctx.measureText(text);
            const padding = 20;
            const w = metrics.width + padding * 2;
            const h = 40;
            const x = region.x + (region.w - w) / 2;
            const y = region.y + 60;

            /* Background */
            ctx.fillStyle = 'rgba(200, 50, 50, 0.9)';
            ctx.fillRect(x, y, w, h);

            /* Border */
            ctx.strokeStyle = '#ff6666';
            ctx.lineWidth = 2;
            ctx.strokeRect(x, y, w, h);

            /* Text */
            ctx.fillStyle = '#fff';
            ctx.textAlign = 'center';
            ctx.fillText(text, x + w / 2, y + h / 2 + 5);
        },
        /* }}} */

        /* {{{ renderDrawOrderUI
         * Render draw order selection UI.
         */
        renderDrawOrderUI: function(ctx, cards, region) {
            if (state.gamePhase !== 'draw_order') return;

            /* Header */
            ctx.fillStyle = 'rgba(0, 0, 0, 0.8)';
            ctx.fillRect(region.x, region.y, region.w, 50);

            ctx.fillStyle = '#d4af37';
            ctx.font = 'bold 16px sans-serif';
            ctx.textAlign = 'center';
            ctx.fillText('Select Draw Order', region.x + region.w / 2, region.y + 20);

            ctx.fillStyle = '#888';
            ctx.font = '12px sans-serif';
            ctx.fillText(
                'Selected: ' + state.drawOrderSelection.length + ' / ' + state.drawOrderRequired,
                region.x + region.w / 2,
                region.y + 40
            );

            /* Highlight selected cards */
            const selectedSet = new Set(state.drawOrderSelection);
            for (let i = 0; i < cards.length; i++) {
                if (selectedSet.has(i)) {
                    const card = cards[i];
                    if (card.bounds) {
                        ctx.strokeStyle = '#d4af37';
                        ctx.lineWidth = 3;
                        ctx.strokeRect(card.bounds.x - 2, card.bounds.y - 2,
                                       card.bounds.w + 4, card.bounds.h + 4);

                        /* Order number */
                        const orderNum = state.drawOrderSelection.indexOf(i) + 1;
                        ctx.fillStyle = '#d4af37';
                        ctx.beginPath();
                        ctx.arc(card.bounds.x + 10, card.bounds.y + 10, 12, 0, Math.PI * 2);
                        ctx.fill();

                        ctx.fillStyle = '#000';
                        ctx.font = 'bold 12px sans-serif';
                        ctx.textAlign = 'center';
                        ctx.textBaseline = 'middle';
                        ctx.fillText(orderNum.toString(), card.bounds.x + 10, card.bounds.y + 10);
                        ctx.textBaseline = 'alphabetic';
                    }
                }
            }
        },
        /* }}} */

        /* {{{ renderActionButtons
         * Render action buttons for current phase.
         */
        renderActionButtons: function(ctx, region, gameState) {
            const buttons = [];

            if (state.gamePhase === 'main') {
                buttons.push({
                    id: 'attack',
                    label: 'Attack',
                    enabled: (gameState.player.combat || 0) > 0,
                    color: '#cc6666'
                });
                buttons.push({
                    id: 'endTurn',
                    label: 'End Turn',
                    enabled: true,
                    color: '#666688'
                });
            }

            if (buttons.length > 0) {
                window.panelRenderer.renderActionButtons(ctx, buttons, region);
            }
        }
        /* }}} */

    };
    /* }}} */

})();
