/**
 * symbeline.js - Browser client JavaScript wrapper for Symbeline Realms
 *
 * This script initializes the WebAssembly module and provides the JavaScript
 * interface that the C code calls into. It manages:
 *   - WebSocket connection to the game server
 *   - Canvas rendering context
 *   - Input event handling
 *   - Animation frame loop
 */

(function() {
    'use strict';

    /* {{{ Configuration */
    const CONFIG = {
        serverUrl: null,  /* Set via symbeline.connect() */
        canvasId: 'game-canvas',
        reconnectDelay: 3000,
        maxReconnectAttempts: 5
    };
    /* }}} */

    /* {{{ State */
    let wasmModule = null;
    let canvas = null;
    let ctx = null;
    let websocket = null;
    let reconnectAttempts = 0;
    let renderRequested = false;
    /* }}} */

    /* {{{ Public API - window.symbeline */
    window.symbeline = {
        /* Initialize the game client */
        init: async function(canvasId) {
            CONFIG.canvasId = canvasId || CONFIG.canvasId;

            /* Set up canvas */
            canvas = document.getElementById(CONFIG.canvasId);
            if (!canvas) {
                console.error('[Symbeline] Canvas not found:', CONFIG.canvasId);
                return false;
            }
            ctx = canvas.getContext('2d');
            resizeCanvas();

            /* Load Wasm module */
            try {
                wasmModule = await SymbelineRealms();
                wasmModule._client_init();
                console.log('[Symbeline] Wasm module loaded');
            } catch (e) {
                console.error('[Symbeline] Failed to load Wasm:', e);
                return false;
            }

            /* Set up event listeners */
            window.addEventListener('resize', resizeCanvas);
            canvas.addEventListener('click', handleClick);
            canvas.addEventListener('mousemove', handleMouseMove);
            document.addEventListener('keydown', handleKeyDown);

            /* Start render loop */
            requestAnimationFrame(renderLoop);

            return true;
        },

        /* Connect to game server */
        connect: function(serverUrl) {
            CONFIG.serverUrl = serverUrl;
            reconnectAttempts = 0;
            connectWebSocket();
        },

        /* Disconnect from server */
        disconnect: function() {
            if (websocket) {
                websocket.close();
                websocket = null;
            }
        },

        /* Send message to server (called from C) */
        sendMessage: function(json) {
            if (websocket && websocket.readyState === WebSocket.OPEN) {
                websocket.send(json);
            } else {
                console.warn('[Symbeline] Cannot send - not connected');
            }
        },

        /* Check connection status (called from C) */
        isConnected: function() {
            return websocket && websocket.readyState === WebSocket.OPEN;
        },

        /* Canvas drawing functions (called from C) */
        clearCanvas: function() {
            ctx.fillStyle = '#1a1a2e';
            ctx.fillRect(0, 0, canvas.width, canvas.height);
        },

        drawRect: function(x, y, w, h, color) {
            ctx.fillStyle = color;
            ctx.fillRect(x, y, w, h);
        },

        drawText: function(x, y, text) {
            ctx.fillStyle = '#e0e0e0';
            ctx.font = '16px monospace';
            ctx.fillText(text, x, y);
        },

        drawCard: function(x, y, w, h, cardJson) {
            /* Parse card data and render */
            try {
                const card = JSON.parse(cardJson);
                drawCardVisual(x, y, w, h, card);
            } catch (e) {
                /* Fallback: draw placeholder */
                ctx.strokeStyle = '#666';
                ctx.strokeRect(x, y, w, h);
                ctx.fillStyle = '#333';
                ctx.fillRect(x + 1, y + 1, w - 2, h - 2);
            }
        },

        /* Request a render frame (called from C) */
        requestRender: function() {
            renderRequested = true;
        },

        /* Cleanup */
        destroy: function() {
            this.disconnect();
            if (wasmModule) {
                wasmModule._client_cleanup();
            }
            window.removeEventListener('resize', resizeCanvas);
            canvas.removeEventListener('click', handleClick);
            canvas.removeEventListener('mousemove', handleMouseMove);
            document.removeEventListener('keydown', handleKeyDown);
        }
    };
    /* }}} */

    /* {{{ WebSocket management */
    function connectWebSocket() {
        if (!CONFIG.serverUrl) {
            console.warn('[Symbeline] No server URL configured');
            return;
        }

        websocket = new WebSocket(CONFIG.serverUrl);

        websocket.onopen = function() {
            console.log('[Symbeline] Connected to server');
            reconnectAttempts = 0;
        };

        websocket.onmessage = function(event) {
            if (wasmModule) {
                /* Pass message to C code */
                const ptr = wasmModule.stringToUTF8OnStack(event.data);
                wasmModule._client_handle_message(ptr);
            }
        };

        websocket.onclose = function() {
            console.log('[Symbeline] Disconnected from server');
            attemptReconnect();
        };

        websocket.onerror = function(error) {
            console.error('[Symbeline] WebSocket error:', error);
        };
    }

    function attemptReconnect() {
        if (reconnectAttempts >= CONFIG.maxReconnectAttempts) {
            console.error('[Symbeline] Max reconnect attempts reached');
            return;
        }

        reconnectAttempts++;
        console.log('[Symbeline] Reconnecting in', CONFIG.reconnectDelay, 'ms...');

        setTimeout(function() {
            if (!window.symbeline.isConnected()) {
                connectWebSocket();
            }
        }, CONFIG.reconnectDelay);
    }
    /* }}} */

    /* {{{ Canvas rendering */
    function resizeCanvas() {
        if (!canvas) return;

        /* Match canvas to container size */
        const container = canvas.parentElement;
        canvas.width = container.clientWidth;
        canvas.height = container.clientHeight;

        /* Trigger re-render */
        renderRequested = true;
    }

    function renderLoop() {
        if (renderRequested && wasmModule) {
            renderRequested = false;
            wasmModule._client_render_frame();
        }
        requestAnimationFrame(renderLoop);
    }

    function drawCardVisual(x, y, w, h, card) {
        /* Faction colors */
        const factionColors = {
            'merchant': '#d4a017',
            'wilds': '#2d7a2d',
            'kingdom': '#3366cc',
            'artificer': '#cc3333',
            'neutral': '#888888'
        };

        const color = factionColors[card.faction] || factionColors.neutral;

        /* Card background */
        ctx.fillStyle = '#222';
        ctx.fillRect(x, y, w, h);

        /* Faction border */
        ctx.strokeStyle = color;
        ctx.lineWidth = 2;
        ctx.strokeRect(x, y, w, h);

        /* Card name */
        ctx.fillStyle = color;
        ctx.font = 'bold 12px monospace';
        ctx.fillText(card.name || '???', x + 4, y + 16);

        /* Cost */
        if (card.cost !== undefined) {
            ctx.fillStyle = '#ffd700';
            ctx.fillText(card.cost.toString(), x + w - 16, y + 16);
        }

        /* Card type indicator */
        ctx.fillStyle = '#666';
        ctx.font = '10px monospace';
        ctx.fillText(card.type || 'ship', x + 4, y + h - 6);
    }
    /* }}} */

    /* {{{ Input handling */
    function handleClick(event) {
        const rect = canvas.getBoundingClientRect();
        const x = event.clientX - rect.left;
        const y = event.clientY - rect.top;

        /* TODO: Determine what was clicked and create action JSON */
        console.log('[Symbeline] Click at', x, y);
    }

    function handleMouseMove(event) {
        const rect = canvas.getBoundingClientRect();
        const x = event.clientX - rect.left;
        const y = event.clientY - rect.top;

        /* TODO: Hover effects */
    }

    function handleKeyDown(event) {
        /* Keyboard shortcuts */
        switch (event.key) {
            case 'Enter':
                /* End turn */
                break;
            case '1': case '2': case '3': case '4': case '5':
            case '6': case '7': case '8': case '9':
                /* Quick-select card */
                break;
            case 'Escape':
                /* Cancel selection */
                break;
        }
    }
    /* }}} */

})();
