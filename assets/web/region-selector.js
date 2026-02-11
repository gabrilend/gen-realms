/**
 * region-selector.js - Inpainting Region Selection for Symbeline Realms
 *
 * Maps game events to canvas regions and manages a priority queue
 * for progressive scene generation. Works with battle-canvas.js
 * for inpainting mask generation.
 */

(function() {
    'use strict';

    /* {{{ Event types */
    var EVENT_TYPE = {
        GAME_START: 'game_start',
        TURN_START: 'turn_start',
        CARD_PLAYED: 'card_played',
        BASE_PLAYED: 'base_played',
        ATTACK_PLAYER: 'attack_player',
        ATTACK_BASE: 'attack_base',
        BASE_DESTROYED: 'base_destroyed',
        GAME_OVER: 'game_over'
    };
    /* }}} */

    /* {{{ Region constants (from battle-canvas.js) */
    var REGION = {
        SKY: 'sky',
        P1_FORCES: 'p1_forces',
        CENTER: 'center',
        P2_FORCES: 'p2_forces',
        P1_BASE: 'p1_base',
        P2_BASE: 'p2_base'
    };
    /* }}} */

    /* {{{ Event to region mapping with base priority */
    var EVENT_REGION_MAP = {
        game_start: { region: REGION.SKY, priority: 100 },
        turn_start: { region: null, priority: 0 },  /* No region */
        card_played: { region: REGION.P1_FORCES, priority: 60 },
        base_played: { region: REGION.P1_BASE, priority: 80 },
        attack_player: { region: REGION.CENTER, priority: 90 },
        attack_base: { region: REGION.CENTER, priority: 85 },
        base_destroyed: { region: REGION.CENTER, priority: 95 },
        game_over: { region: REGION.CENTER, priority: 100 }
    };
    /* }}} */

    /* {{{ Region generation order (dependencies) */
    var REGION_ORDER = [
        REGION.SKY,         /* Background first */
        REGION.P1_BASE,     /* Bases second */
        REGION.P2_BASE,
        REGION.P1_FORCES,   /* Forces third */
        REGION.P2_FORCES,
        REGION.CENTER       /* Center last (overlaps others) */
    ];
    /* }}} */

    /* {{{ RegionSelector class */
    function RegionSelector(battleCanvas) {
        this.canvas = battleCanvas || window.battleCanvas;
        this.queue = [];
        this.regionPending = {};

        for (var region in REGION) {
            this.regionPending[REGION[region]] = false;
        }

        /* Callbacks */
        this.onTaskQueued = null;
        this.onTaskStarted = null;
        this.onTaskCompleted = null;
    }

    /* {{{ queueEvent
     * Queue a game event for region generation.
     */
    RegionSelector.prototype.queueEvent = function(eventType, eventData) {
        var mapping = EVENT_REGION_MAP[eventType];
        if (!mapping || !mapping.region) {
            return false;  /* Event doesn't trigger region generation */
        }

        eventData = eventData || {};

        /* Determine actual region based on event data */
        var region = this._determineRegion(eventType, eventData);
        if (!region) return false;

        /* Check if region already filled or pending */
        if (this.canvas && this.canvas.isRegionFilled(region)) {
            return false;
        }

        if (this.regionPending[region]) {
            /* Update priority if higher */
            this._updatePriority(region, mapping.priority);
            return true;
        }

        /* Create task */
        var task = {
            region: region,
            priority: this._calculatePriority(mapping.priority, region),
            eventType: eventType,
            eventData: eventData,
            promptContext: this._buildPromptContext(eventType, eventData),
            queuedAt: Date.now()
        };

        this._insertTask(task);
        this.regionPending[region] = true;

        if (this.onTaskQueued) {
            this.onTaskQueued(task);
        }

        return true;
    };
    /* }}} */

    /* {{{ _determineRegion
     * Internal: Determine which region based on event type and data.
     */
    RegionSelector.prototype._determineRegion = function(eventType, eventData) {
        var isPlayer1 = eventData.player === 1 || eventData.playerId === 1;
        var baseRegion = EVENT_REGION_MAP[eventType].region;

        switch (eventType) {
            case EVENT_TYPE.CARD_PLAYED:
                return isPlayer1 ? REGION.P1_FORCES : REGION.P2_FORCES;

            case EVENT_TYPE.BASE_PLAYED:
                return isPlayer1 ? REGION.P1_BASE : REGION.P2_BASE;

            case EVENT_TYPE.BASE_DESTROYED:
                /* Use target's base region */
                return eventData.targetPlayer === 1 ? REGION.P1_BASE : REGION.P2_BASE;

            default:
                return baseRegion;
        }
    };
    /* }}} */

    /* {{{ _calculatePriority
     * Internal: Calculate final priority considering dependencies.
     */
    RegionSelector.prototype._calculatePriority = function(basePriority, region) {
        var orderIndex = REGION_ORDER.indexOf(region);

        /* Lower index = should generate first = higher priority */
        var orderBonus = (REGION_ORDER.length - orderIndex) * 5;

        /* Bonus for unfilled prerequisite regions */
        var prereqPenalty = 0;
        for (var i = 0; i < orderIndex; i++) {
            var prereqRegion = REGION_ORDER[i];
            if (this.canvas && !this.canvas.isRegionFilled(prereqRegion)) {
                prereqPenalty += 10;
            }
        }

        return basePriority + orderBonus - prereqPenalty;
    };
    /* }}} */

    /* {{{ _buildPromptContext
     * Internal: Build prompt context from event data.
     */
    RegionSelector.prototype._buildPromptContext = function(eventType, eventData) {
        var context = [];

        switch (eventType) {
            case EVENT_TYPE.GAME_START:
                context.push('battle scene beginning');
                context.push('dramatic sky');
                break;

            case EVENT_TYPE.CARD_PLAYED:
                if (eventData.card) {
                    context.push(eventData.card.name + ' deployed');
                    if (eventData.card.faction) {
                        context.push(eventData.card.faction + ' forces');
                    }
                }
                break;

            case EVENT_TYPE.BASE_PLAYED:
                if (eventData.card) {
                    context.push(eventData.card.name + ' established');
                    context.push('fortification');
                }
                break;

            case EVENT_TYPE.ATTACK_PLAYER:
                context.push('combat in progress');
                context.push('attack animation');
                if (eventData.damage) {
                    context.push(eventData.damage + ' damage');
                }
                break;

            case EVENT_TYPE.BASE_DESTROYED:
                context.push('destruction');
                context.push('rubble and fire');
                break;

            case EVENT_TYPE.GAME_OVER:
                context.push('victory scene');
                if (eventData.winner) {
                    context.push(eventData.winner + ' triumphant');
                }
                break;
        }

        return context.join(', ');
    };
    /* }}} */

    /* {{{ _insertTask
     * Internal: Insert task in priority order (highest first).
     */
    RegionSelector.prototype._insertTask = function(task) {
        var inserted = false;

        for (var i = 0; i < this.queue.length; i++) {
            if (task.priority > this.queue[i].priority) {
                this.queue.splice(i, 0, task);
                inserted = true;
                break;
            }
        }

        if (!inserted) {
            this.queue.push(task);
        }
    };
    /* }}} */

    /* {{{ _updatePriority
     * Internal: Update priority for existing task if higher.
     */
    RegionSelector.prototype._updatePriority = function(region, newPriority) {
        for (var i = 0; i < this.queue.length; i++) {
            if (this.queue[i].region === region) {
                var calculated = this._calculatePriority(newPriority, region);
                if (calculated > this.queue[i].priority) {
                    this.queue[i].priority = calculated;
                    /* Re-sort queue */
                    var task = this.queue.splice(i, 1)[0];
                    this._insertTask(task);
                }
                return;
            }
        }
    };
    /* }}} */

    /* {{{ getNext
     * Get next region task from queue.
     */
    RegionSelector.prototype.getNext = function() {
        if (this.queue.length === 0) {
            return null;
        }

        var task = this.queue.shift();
        this.regionPending[task.region] = false;

        if (this.onTaskStarted) {
            this.onTaskStarted(task);
        }

        return task;
    };
    /* }}} */

    /* {{{ peek
     * Peek at next task without removing.
     */
    RegionSelector.prototype.peek = function() {
        return this.queue.length > 0 ? this.queue[0] : null;
    };
    /* }}} */

    /* {{{ markComplete
     * Mark a region as completed.
     */
    RegionSelector.prototype.markComplete = function(region) {
        this.regionPending[region] = false;

        if (this.onTaskCompleted) {
            this.onTaskCompleted({ region: region });
        }

        /* Re-calculate priorities for remaining tasks */
        this._recalculatePriorities();
    };
    /* }}} */

    /* {{{ _recalculatePriorities
     * Internal: Recalculate all task priorities after completion.
     */
    RegionSelector.prototype._recalculatePriorities = function() {
        for (var i = 0; i < this.queue.length; i++) {
            var task = this.queue[i];
            var basePriority = EVENT_REGION_MAP[task.eventType]
                ? EVENT_REGION_MAP[task.eventType].priority
                : 50;
            task.priority = this._calculatePriority(basePriority, task.region);
        }

        /* Re-sort */
        this.queue.sort(function(a, b) {
            return b.priority - a.priority;
        });
    };
    /* }}} */

    /* {{{ getMask
     * Generate inpainting mask for next region.
     */
    RegionSelector.prototype.getMask = function() {
        var task = this.peek();
        if (!task || !this.canvas) {
            return null;
        }

        return this.canvas.getMaskForRegion(task.region);
    };
    /* }}} */

    /* {{{ getQueueLength
     * Get number of pending tasks.
     */
    RegionSelector.prototype.getQueueLength = function() {
        return this.queue.length;
    };
    /* }}} */

    /* {{{ clear
     * Clear all pending tasks.
     */
    RegionSelector.prototype.clear = function() {
        this.queue = [];
        for (var region in this.regionPending) {
            this.regionPending[region] = false;
        }
    };
    /* }}} */

    /* {{{ queueAllUnfilled
     * Queue all unfilled regions with appropriate priorities.
     */
    RegionSelector.prototype.queueAllUnfilled = function() {
        if (!this.canvas) return;

        var unfilled = this.canvas.getUnfilledRegions();

        for (var i = 0; i < unfilled.length; i++) {
            var region = unfilled[i];

            /* Find default priority based on region */
            var priority = 50;  /* Default */
            if (region === REGION.SKY) priority = 100;
            else if (region === REGION.CENTER) priority = 90;
            else if (region === REGION.P1_BASE || region === REGION.P2_BASE) priority = 80;
            else priority = 60;

            var task = {
                region: region,
                priority: this._calculatePriority(priority, region),
                eventType: 'fill',
                eventData: {},
                promptContext: 'fantasy battle scene, ' + region.replace('_', ' '),
                queuedAt: Date.now()
            };

            if (!this.regionPending[region]) {
                this._insertTask(task);
                this.regionPending[region] = true;
            }
        }
    };
    /* }}} */

    /* {{{ getStats
     * Get selector statistics.
     */
    RegionSelector.prototype.getStats = function() {
        var pendingCount = 0;
        for (var region in this.regionPending) {
            if (this.regionPending[region]) pendingCount++;
        }

        return {
            queueLength: this.queue.length,
            pendingCount: pendingCount,
            filledCount: this.canvas ? this.canvas.getFilledRegions().length : 0
        };
    };
    /* }}} */

    /* }}} */

    /* {{{ Public API */
    window.RegionSelector = RegionSelector;

    /* Export constants */
    window.RegionSelector.EVENT_TYPE = EVENT_TYPE;
    window.RegionSelector.REGION = REGION;
    window.RegionSelector.REGION_ORDER = REGION_ORDER;

    /* Create default instance */
    window.regionSelector = new RegionSelector();
    /* }}} */

})();
