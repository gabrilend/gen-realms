/**
 * generation-queue.js - Image Generation Queue for Symbeline Realms
 *
 * Manages queued requests to ComfyUI for card art generation.
 * Implements priority-based ordering (visible cards first), concurrency
 * limits, and retry logic with exponential backoff.
 */

(function() {
    'use strict';

    /* {{{ Priority levels */
    var PRIORITY = {
        VISIBLE_HAND: 1,      /* Cards in player's hand - highest priority */
        VISIBLE_TRADE: 2,     /* Cards in trade row */
        VISIBLE_PLAY: 3,      /* Cards currently in play */
        OPPONENT_VISIBLE: 5,  /* Opponent's visible cards */
        BACKGROUND: 10        /* Preload for next turn - lowest priority */
    };
    /* }}} */

    /* {{{ Default options */
    var DEFAULT_OPTIONS = {
        maxConcurrent: 2,
        retryLimit: 3,
        retryDelay: 5000,     /* ms, base delay before retry */
        requestTimeout: 60000 /* ms, timeout for single request */
    };
    /* }}} */

    /* {{{ GenerationQueue class */
    function GenerationQueue(options) {
        options = options || {};

        this.queue = [];
        this.processing = false;
        this.maxConcurrent = options.maxConcurrent || DEFAULT_OPTIONS.maxConcurrent;
        this.activeCount = 0;
        this.retryLimit = options.retryLimit || DEFAULT_OPTIONS.retryLimit;
        this.retryDelay = options.retryDelay || DEFAULT_OPTIONS.retryDelay;
        this.requestTimeout = options.requestTimeout || DEFAULT_OPTIONS.requestTimeout;

        /* Callbacks */
        this.onProgress = null;   /* function(card, status, data) */
        this.onComplete = null;   /* function(card, imageData) */
        this.onError = null;      /* function(card, error) */

        /* Stats */
        this.totalProcessed = 0;
        this.totalFailed = 0;

        /* ComfyUI client reference (set externally) */
        this.comfyuiClient = null;
    }

    /* {{{ enqueue
     * Add a card to the generation queue with priority.
     */
    GenerationQueue.prototype.enqueue = function(card, prompt, priority) {
        if (!card || !card.instance_id) {
            console.warn('GenerationQueue: Cannot enqueue invalid card');
            return false;
        }

        /* Check if already in queue */
        for (var i = 0; i < this.queue.length; i++) {
            if (this.queue[i].card.instance_id === card.instance_id) {
                /* Update priority if higher (lower number) */
                if (priority < this.queue[i].priority) {
                    this.queue[i].priority = priority;
                    this._sortQueue();
                }
                return true;
            }
        }

        var request = {
            card: card,
            prompt: prompt,
            priority: priority || PRIORITY.BACKGROUND,
            retries: 0,
            enqueuedAt: Date.now()
        };

        /* Insert sorted by priority */
        var inserted = false;
        for (var j = 0; j < this.queue.length; j++) {
            if (this.queue[j].priority > priority) {
                this.queue.splice(j, 0, request);
                inserted = true;
                break;
            }
        }

        if (!inserted) {
            this.queue.push(request);
        }

        this._notifyProgress(card, 'queued', { position: this.queue.length });
        this._processQueue();

        return true;
    };
    /* }}} */

    /* {{{ cancel
     * Remove a card from the queue.
     */
    GenerationQueue.prototype.cancel = function(instanceId) {
        for (var i = 0; i < this.queue.length; i++) {
            if (this.queue[i].card.instance_id === instanceId) {
                var removed = this.queue.splice(i, 1)[0];
                this._notifyProgress(removed.card, 'cancelled');
                return true;
            }
        }
        return false;
    };
    /* }}} */

    /* {{{ cancelAll
     * Clear the entire queue.
     */
    GenerationQueue.prototype.cancelAll = function() {
        var count = this.queue.length;
        this.queue = [];
        return count;
    };
    /* }}} */

    /* {{{ getQueueLength
     * Get current queue length.
     */
    GenerationQueue.prototype.getQueueLength = function() {
        return this.queue.length;
    };
    /* }}} */

    /* {{{ getActiveCount
     * Get number of currently processing requests.
     */
    GenerationQueue.prototype.getActiveCount = function() {
        return this.activeCount;
    };
    /* }}} */

    /* {{{ isProcessing
     * Check if a specific card is currently being processed.
     */
    GenerationQueue.prototype.isProcessing = function(instanceId) {
        /* Check active requests */
        for (var i = 0; i < this.queue.length; i++) {
            if (this.queue[i].card.instance_id === instanceId &&
                this.queue[i].processing) {
                return true;
            }
        }
        return false;
    };
    /* }}} */

    /* {{{ getStats
     * Get queue statistics.
     */
    GenerationQueue.prototype.getStats = function() {
        return {
            queueLength: this.queue.length,
            activeCount: this.activeCount,
            maxConcurrent: this.maxConcurrent,
            totalProcessed: this.totalProcessed,
            totalFailed: this.totalFailed
        };
    };
    /* }}} */

    /* {{{ setPriority
     * Update priority for cards based on game state.
     */
    GenerationQueue.prototype.setPriority = function(instanceId, newPriority) {
        for (var i = 0; i < this.queue.length; i++) {
            if (this.queue[i].card.instance_id === instanceId) {
                this.queue[i].priority = newPriority;
                this._sortQueue();
                return true;
            }
        }
        return false;
    };
    /* }}} */

    /* {{{ getPriorityForCard
     * Determine priority based on card location in game state.
     */
    GenerationQueue.prototype.getPriorityForCard = function(card, gameState) {
        if (!gameState) {
            return PRIORITY.BACKGROUND;
        }

        /* Check hand */
        if (gameState.hand && this._containsCard(gameState.hand, card)) {
            return PRIORITY.VISIBLE_HAND;
        }

        /* Check trade row */
        if (gameState.tradeRow && this._containsCard(gameState.tradeRow, card)) {
            return PRIORITY.VISIBLE_TRADE;
        }

        /* Check in play */
        if (gameState.inPlay && this._containsCard(gameState.inPlay, card)) {
            return PRIORITY.VISIBLE_PLAY;
        }

        /* Check opponent visible */
        if (gameState.opponentInPlay && this._containsCard(gameState.opponentInPlay, card)) {
            return PRIORITY.OPPONENT_VISIBLE;
        }

        return PRIORITY.BACKGROUND;
    };
    /* }}} */

    /* {{{ _containsCard
     * Internal: Check if card array contains a specific card.
     */
    GenerationQueue.prototype._containsCard = function(cards, card) {
        for (var i = 0; i < cards.length; i++) {
            if (cards[i].instance_id === card.instance_id) {
                return true;
            }
        }
        return false;
    };
    /* }}} */

    /* {{{ _sortQueue
     * Internal: Sort queue by priority (ascending).
     */
    GenerationQueue.prototype._sortQueue = function() {
        this.queue.sort(function(a, b) {
            return a.priority - b.priority;
        });
    };
    /* }}} */

    /* {{{ _processQueue
     * Internal: Process pending requests up to concurrency limit.
     */
    GenerationQueue.prototype._processQueue = function() {
        var self = this;

        if (this.processing) return;
        this.processing = true;

        while (this.queue.length > 0 && this.activeCount < this.maxConcurrent) {
            var request = this._getNextRequest();
            if (!request) break;

            this.activeCount++;
            request.processing = true;

            this._processRequest(request)
                .then(function(result) {
                    self._handleSuccess(result.request, result.imageData);
                })
                .catch(function(error) {
                    self._handleFailure(error.request, error.error);
                })
                .finally(function() {
                    self.activeCount--;
                    self.processing = false;
                    self._processQueue();
                });
        }

        this.processing = false;
    };
    /* }}} */

    /* {{{ _getNextRequest
     * Internal: Get next non-processing request from queue.
     */
    GenerationQueue.prototype._getNextRequest = function() {
        for (var i = 0; i < this.queue.length; i++) {
            if (!this.queue[i].processing) {
                return this.queue[i];
            }
        }
        return null;
    };
    /* }}} */

    /* {{{ _processRequest
     * Internal: Process a single generation request.
     */
    GenerationQueue.prototype._processRequest = function(request) {
        var self = this;

        this._notifyProgress(request.card, 'generating');

        return new Promise(function(resolve, reject) {
            /* Check if ComfyUI client is available */
            if (!self.comfyuiClient) {
                reject({ request: request, error: new Error('ComfyUI client not configured') });
                return;
            }

            /* Create timeout */
            var timeoutId = setTimeout(function() {
                reject({ request: request, error: new Error('Request timeout') });
            }, self.requestTimeout);

            /* Make generation request */
            self.comfyuiClient.generate(request.prompt, {
                seed: request.card.image_seed
            }).then(function(imageData) {
                clearTimeout(timeoutId);
                resolve({ request: request, imageData: imageData });
            }).catch(function(error) {
                clearTimeout(timeoutId);
                reject({ request: request, error: error });
            });
        });
    };
    /* }}} */

    /* {{{ _handleSuccess
     * Internal: Handle successful generation.
     */
    GenerationQueue.prototype._handleSuccess = function(request, imageData) {
        /* Remove from queue */
        this._removeFromQueue(request.card.instance_id);

        /* Update stats */
        this.totalProcessed++;

        /* Store in cache */
        if (window.imageCache) {
            window.imageCache.set(request.card.instance_id, imageData, request.card.image_seed);
        }

        /* Mark complete in tracker */
        if (window.artTracker) {
            window.artTracker.markComplete(request.card);
        }

        /* Notify */
        this._notifyProgress(request.card, 'complete');
        if (this.onComplete) {
            this.onComplete(request.card, imageData);
        }
    };
    /* }}} */

    /* {{{ _handleFailure
     * Internal: Handle failed generation with retry.
     */
    GenerationQueue.prototype._handleFailure = function(request, error) {
        console.error('GenerationQueue: Generation failed:', error);

        request.processing = false;

        if (request.retries < this.retryLimit) {
            request.retries++;

            /* Exponential backoff */
            var delay = this.retryDelay * Math.pow(2, request.retries - 1);

            this._notifyProgress(request.card, 'retrying', {
                attempt: request.retries,
                delay: delay
            });

            var self = this;
            setTimeout(function() {
                self._processQueue();
            }, delay);
        } else {
            /* Max retries exceeded */
            this._removeFromQueue(request.card.instance_id);
            this.totalFailed++;

            /* Notify tracker of failure */
            if (window.artTracker) {
                window.artTracker.markFailed(request.card, error);
            }

            this._notifyProgress(request.card, 'failed', { error: error.message });
            if (this.onError) {
                this.onError(request.card, error);
            }
        }
    };
    /* }}} */

    /* {{{ _removeFromQueue
     * Internal: Remove request from queue by instance ID.
     */
    GenerationQueue.prototype._removeFromQueue = function(instanceId) {
        for (var i = 0; i < this.queue.length; i++) {
            if (this.queue[i].card.instance_id === instanceId) {
                this.queue.splice(i, 1);
                return true;
            }
        }
        return false;
    };
    /* }}} */

    /* {{{ _notifyProgress
     * Internal: Notify progress callback if set.
     */
    GenerationQueue.prototype._notifyProgress = function(card, status, data) {
        if (this.onProgress) {
            try {
                this.onProgress(card, status, data || {});
            } catch (e) {
                console.error('GenerationQueue: Progress callback error:', e);
            }
        }
    };
    /* }}} */

    /* }}} */

    /* {{{ Mock ComfyUI client for development */
    var mockComfyuiClient = {
        generate: function(prompt, options) {
            return new Promise(function(resolve) {
                /* Simulate generation delay */
                setTimeout(function() {
                    /* Return placeholder image data */
                    resolve('data:image/png;base64,PLACEHOLDER');
                }, 1000 + Math.random() * 2000);
            });
        }
    };
    /* }}} */

    /* {{{ Public API */
    window.generationQueue = new GenerationQueue();

    /* Export priority constants */
    window.generationQueue.PRIORITY = PRIORITY;

    /* Set mock client for development (replace with real client in production) */
    window.generationQueue.comfyuiClient = mockComfyuiClient;
    /* }}} */

})();
