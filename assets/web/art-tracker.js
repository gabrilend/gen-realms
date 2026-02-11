/**
 * art-tracker.js - Card Art Regeneration Tracking for Symbeline Realms
 *
 * Tracks which cards need art regeneration based on needs_regen flag.
 * Cards are marked for regeneration on upgrade, first draw, or explicit
 * user request. Works with generation-queue.js and image-cache.js.
 */

(function() {
    'use strict';

    /* {{{ ArtTracker class */
    function ArtTracker() {
        this.pendingCards = new Map();  /* instance_id -> card object */
        this.listeners = [];            /* callbacks for state changes */
    }

    /* {{{ needsRegeneration
     * Check if a card needs art regeneration.
     */
    ArtTracker.prototype.needsRegeneration = function(card) {
        if (!card) return false;
        return card.needs_regen === true;
    };
    /* }}} */

    /* {{{ markForRegeneration
     * Mark a card as needing art regeneration.
     * Adds to pending queue and notifies listeners.
     */
    ArtTracker.prototype.markForRegeneration = function(card) {
        if (!card || !card.instance_id) {
            console.warn('ArtTracker: Cannot mark invalid card for regeneration');
            return false;
        }

        card.needs_regen = true;

        /* Ensure card has a seed for reproducible generation */
        if (!card.image_seed) {
            card.image_seed = this.generateSeed();
        }

        this.pendingCards.set(card.instance_id, card);
        this._notifyListeners('marked', card);

        return true;
    };
    /* }}} */

    /* {{{ markComplete
     * Mark a card's regeneration as complete.
     * Clears needs_regen flag and removes from pending queue.
     */
    ArtTracker.prototype.markComplete = function(card) {
        if (!card || !card.instance_id) {
            return false;
        }

        card.needs_regen = false;
        this.pendingCards.delete(card.instance_id);
        this._notifyListeners('complete', card);

        return true;
    };
    /* }}} */

    /* {{{ markFailed
     * Mark a card's regeneration as failed.
     * Keeps in pending queue for retry.
     */
    ArtTracker.prototype.markFailed = function(card, error) {
        if (!card || !card.instance_id) {
            return false;
        }

        this._notifyListeners('failed', card, error);
        return true;
    };
    /* }}} */

    /* {{{ getPendingCards
     * Get all cards currently pending regeneration.
     */
    ArtTracker.prototype.getPendingCards = function() {
        return Array.from(this.pendingCards.values());
    };
    /* }}} */

    /* {{{ getPendingCount
     * Get count of cards pending regeneration.
     */
    ArtTracker.prototype.getPendingCount = function() {
        return this.pendingCards.size;
    };
    /* }}} */

    /* {{{ isPending
     * Check if a specific card is pending regeneration.
     */
    ArtTracker.prototype.isPending = function(instanceId) {
        return this.pendingCards.has(instanceId);
    };
    /* }}} */

    /* {{{ cancelRegeneration
     * Cancel pending regeneration for a card.
     */
    ArtTracker.prototype.cancelRegeneration = function(instanceId) {
        if (this.pendingCards.has(instanceId)) {
            const card = this.pendingCards.get(instanceId);
            this.pendingCards.delete(instanceId);
            this._notifyListeners('cancelled', card);
            return true;
        }
        return false;
    };
    /* }}} */

    /* {{{ clearAll
     * Clear all pending regenerations.
     */
    ArtTracker.prototype.clearAll = function() {
        const count = this.pendingCards.size;
        this.pendingCards.clear();
        this._notifyListeners('cleared', null, { count: count });
        return count;
    };
    /* }}} */

    /* {{{ regenerateWithNewSeed
     * Mark card for regeneration with a new random seed.
     * Used when user explicitly requests different art.
     */
    ArtTracker.prototype.regenerateWithNewSeed = function(card) {
        if (!card || !card.instance_id) {
            return false;
        }

        card.image_seed = this.generateSeed();
        return this.markForRegeneration(card);
    };
    /* }}} */

    /* {{{ generateSeed
     * Generate a random seed for image generation.
     */
    ArtTracker.prototype.generateSeed = function() {
        return Math.floor(Math.random() * 1000000);
    };
    /* }}} */

    /* {{{ getOrCreateSeed
     * Get existing seed or create new one for a card.
     */
    ArtTracker.prototype.getOrCreateSeed = function(card) {
        if (!card) return 0;

        if (!card.image_seed) {
            card.image_seed = this.generateSeed();
        }
        return card.image_seed;
    };
    /* }}} */

    /* {{{ addListener
     * Add a listener for tracking state changes.
     * Callback receives (event, card, data).
     */
    ArtTracker.prototype.addListener = function(callback) {
        if (typeof callback === 'function') {
            this.listeners.push(callback);
        }
    };
    /* }}} */

    /* {{{ removeListener
     * Remove a previously added listener.
     */
    ArtTracker.prototype.removeListener = function(callback) {
        const index = this.listeners.indexOf(callback);
        if (index !== -1) {
            this.listeners.splice(index, 1);
            return true;
        }
        return false;
    };
    /* }}} */

    /* {{{ _notifyListeners
     * Internal: notify all listeners of state change.
     */
    ArtTracker.prototype._notifyListeners = function(event, card, data) {
        for (let i = 0; i < this.listeners.length; i++) {
            try {
                this.listeners[i](event, card, data);
            } catch (e) {
                console.error('ArtTracker listener error:', e);
            }
        }
    };
    /* }}} */

    /* {{{ getStats
     * Get statistics about tracking state.
     */
    ArtTracker.prototype.getStats = function() {
        return {
            pendingCount: this.pendingCards.size,
            listenerCount: this.listeners.length
        };
    };
    /* }}} */

    /* }}} */

    /* {{{ Event handlers for game state */

    /* {{{ onCardUpgraded
     * Handler for when a card receives an upgrade.
     * Marks card for regeneration to show upgrade visuals.
     */
    function onCardUpgraded(card) {
        if (window.artTracker) {
            window.artTracker.markForRegeneration(card);
        }
    }

    /* {{{ onCardDrawn
     * Handler for when a card is drawn from deck.
     * Marks for regeneration if no cached image exists.
     */
    function onCardDrawn(card, imageCache) {
        if (!window.artTracker) return;

        /* Only regenerate if no cached image */
        if (imageCache && imageCache.has(card.instance_id)) {
            return;
        }

        /* Check if needs_regen is already set from server */
        if (card.needs_regen) {
            window.artTracker.markForRegeneration(card);
        }
    }

    /* {{{ onRegenerateRequested
     * Handler for when user explicitly requests new art.
     */
    function onRegenerateRequested(card) {
        if (window.artTracker) {
            window.artTracker.regenerateWithNewSeed(card);
        }
    }

    /* {{{ onGameStateReceived
     * Handler for processing cards from server game state.
     * Checks each card's needs_regen flag.
     */
    function onGameStateReceived(gameState, imageCache) {
        if (!window.artTracker || !gameState) return;

        var cards = [];

        /* Collect all cards from game state */
        if (gameState.hand) cards = cards.concat(gameState.hand);
        if (gameState.deck) cards = cards.concat(gameState.deck);
        if (gameState.discard) cards = cards.concat(gameState.discard);
        if (gameState.inPlay) cards = cards.concat(gameState.inPlay);
        if (gameState.tradeRow) cards = cards.concat(gameState.tradeRow);

        /* Check each card */
        for (var i = 0; i < cards.length; i++) {
            var card = cards[i];
            if (card && card.needs_regen) {
                /* Only mark if not already cached */
                if (!imageCache || !imageCache.has(card.instance_id)) {
                    window.artTracker.markForRegeneration(card);
                }
            }
        }
    }

    /* }}} */

    /* {{{ Public API */
    window.artTracker = new ArtTracker();

    /* Export event handlers for integration */
    window.artTracker.handlers = {
        onCardUpgraded: onCardUpgraded,
        onCardDrawn: onCardDrawn,
        onRegenerateRequested: onRegenerateRequested,
        onGameStateReceived: onGameStateReceived
    };
    /* }}} */

})();
