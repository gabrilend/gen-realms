/**
 * scene-composition.js - Scene Composition Rules for Symbeline Realms
 *
 * Governs element placement in the battle canvas with player-side
 * consistency, z-layer management, and visual balance.
 */

(function() {
    'use strict';

    /* {{{ Z-layer constants */
    var Z_LAYER = {
        BACKGROUND: 0,    /* Sky, distant elements */
        BASE: 1,          /* Bases and fortifications */
        FORCES: 2,        /* Creatures and units */
        ACTION: 3,        /* Combat effects, foreground */
        OVERLAY: 4        /* UI elements, highlights */
    };
    /* }}} */

    /* {{{ Placement zones (normalized 0-1 coordinates) */
    var ZONES = {
        sky: {
            x: 0, y: 0, width: 1.0, height: 0.2,
            zLayer: Z_LAYER.BACKGROUND,
            name: 'sky'
        },
        p1_base: {
            x: 0, y: 0.7, width: 0.35, height: 0.3,
            zLayer: Z_LAYER.BASE,
            name: 'p1_base'
        },
        p2_base: {
            x: 0.65, y: 0.7, width: 0.35, height: 0.3,
            zLayer: Z_LAYER.BASE,
            name: 'p2_base'
        },
        p1_forces: {
            x: 0.05, y: 0.2, width: 0.35, height: 0.5,
            zLayer: Z_LAYER.FORCES,
            name: 'p1_forces'
        },
        p2_forces: {
            x: 0.6, y: 0.2, width: 0.35, height: 0.5,
            zLayer: Z_LAYER.FORCES,
            name: 'p2_forces'
        },
        center: {
            x: 0.25, y: 0.15, width: 0.5, height: 0.55,
            zLayer: Z_LAYER.ACTION,
            name: 'center'
        }
    };
    /* }}} */

    /* {{{ Element types */
    var ELEMENT_TYPE = {
        BASE: 'base',
        CREATURE: 'creature',
        UNIT: 'unit',
        ATTACK: 'attack',
        EFFECT: 'effect',
        BACKGROUND: 'background'
    };
    /* }}} */

    /* {{{ SceneComposition class */
    function SceneComposition(options) {
        options = options || {};

        this.canvasWidth = options.width || 512;
        this.canvasHeight = options.height || 512;

        /* Track placed elements */
        this.elements = [];

        /* Density tracking per zone */
        this.zoneDensity = {};
        for (var zone in ZONES) {
            this.zoneDensity[zone] = 0;
        }
    }

    /* {{{ getZone
     * Get zone definition by name.
     */
    SceneComposition.prototype.getZone = function(zoneName) {
        return ZONES[zoneName] || null;
    };
    /* }}} */

    /* {{{ getZonePixels
     * Get zone bounds in pixel coordinates.
     */
    SceneComposition.prototype.getZonePixels = function(zoneName) {
        var zone = ZONES[zoneName];
        if (!zone) return null;

        return {
            x: Math.floor(zone.x * this.canvasWidth),
            y: Math.floor(zone.y * this.canvasHeight),
            width: Math.floor(zone.width * this.canvasWidth),
            height: Math.floor(zone.height * this.canvasHeight),
            zLayer: zone.zLayer,
            name: zone.name
        };
    };
    /* }}} */

    /* {{{ getZoneForPlayer
     * Get appropriate zone for a player's element.
     */
    SceneComposition.prototype.getZoneForPlayer = function(playerId, elementType) {
        var isPlayer1 = playerId === 1 || playerId === 'p1';

        switch (elementType) {
            case ELEMENT_TYPE.BASE:
                return isPlayer1 ? 'p1_base' : 'p2_base';

            case ELEMENT_TYPE.CREATURE:
            case ELEMENT_TYPE.UNIT:
                return isPlayer1 ? 'p1_forces' : 'p2_forces';

            case ELEMENT_TYPE.ATTACK:
            case ELEMENT_TYPE.EFFECT:
                return 'center';

            case ELEMENT_TYPE.BACKGROUND:
                return 'sky';

            default:
                return isPlayer1 ? 'p1_forces' : 'p2_forces';
        }
    };
    /* }}} */

    /* {{{ suggestPosition
     * Suggest a position within a zone, avoiding existing elements.
     */
    SceneComposition.prototype.suggestPosition = function(zoneName, elementSize) {
        var zone = this.getZonePixels(zoneName);
        if (!zone) return null;

        elementSize = elementSize || { width: 64, height: 64 };

        /* Find existing elements in this zone */
        var zoneElements = this.elements.filter(function(el) {
            return el.zone === zoneName;
        });

        /* Try to find non-overlapping position */
        var maxAttempts = 20;
        var bestPosition = null;
        var minOverlap = Infinity;

        for (var i = 0; i < maxAttempts; i++) {
            /* Random position within zone */
            var x = zone.x + Math.random() * (zone.width - elementSize.width);
            var y = zone.y + Math.random() * (zone.height - elementSize.height);

            var position = {
                x: Math.floor(x),
                y: Math.floor(y),
                width: elementSize.width,
                height: elementSize.height
            };

            /* Calculate overlap with existing elements */
            var totalOverlap = 0;
            for (var j = 0; j < zoneElements.length; j++) {
                totalOverlap += this._calculateOverlap(position, zoneElements[j]);
            }

            if (totalOverlap === 0) {
                return position;  /* Perfect fit */
            }

            if (totalOverlap < minOverlap) {
                minOverlap = totalOverlap;
                bestPosition = position;
            }
        }

        return bestPosition;
    };
    /* }}} */

    /* {{{ _calculateOverlap
     * Internal: Calculate overlap area between two rectangles.
     */
    SceneComposition.prototype._calculateOverlap = function(a, b) {
        var xOverlap = Math.max(0,
            Math.min(a.x + a.width, b.x + b.width) - Math.max(a.x, b.x));
        var yOverlap = Math.max(0,
            Math.min(a.y + a.height, b.y + b.height) - Math.max(a.y, b.y));

        return xOverlap * yOverlap;
    };
    /* }}} */

    /* {{{ checkOverlap
     * Check if two elements overlap.
     */
    SceneComposition.prototype.checkOverlap = function(elemA, elemB) {
        return this._calculateOverlap(elemA, elemB) > 0;
    };
    /* }}} */

    /* {{{ addElement
     * Register an element placement.
     */
    SceneComposition.prototype.addElement = function(element) {
        if (!element.zone || !element.x || !element.y) {
            console.warn('SceneComposition: Invalid element');
            return false;
        }

        /* Set z-layer from zone if not specified */
        if (element.zLayer === undefined) {
            var zone = ZONES[element.zone];
            element.zLayer = zone ? zone.zLayer : Z_LAYER.FORCES;
        }

        this.elements.push(element);
        this.zoneDensity[element.zone] = (this.zoneDensity[element.zone] || 0) + 1;

        return true;
    };
    /* }}} */

    /* {{{ removeElement
     * Remove an element by ID.
     */
    SceneComposition.prototype.removeElement = function(elementId) {
        for (var i = this.elements.length - 1; i >= 0; i--) {
            if (this.elements[i].id === elementId) {
                var removed = this.elements.splice(i, 1)[0];
                this.zoneDensity[removed.zone]--;
                return true;
            }
        }
        return false;
    };
    /* }}} */

    /* {{{ getElementsByZone
     * Get all elements in a zone.
     */
    SceneComposition.prototype.getElementsByZone = function(zoneName) {
        return this.elements.filter(function(el) {
            return el.zone === zoneName;
        });
    };
    /* }}} */

    /* {{{ getElementsByLayer
     * Get all elements at a z-layer.
     */
    SceneComposition.prototype.getElementsByLayer = function(zLayer) {
        return this.elements.filter(function(el) {
            return el.zLayer === zLayer;
        });
    };
    /* }}} */

    /* {{{ getSortedElements
     * Get elements sorted by z-layer for rendering.
     */
    SceneComposition.prototype.getSortedElements = function() {
        return this.elements.slice().sort(function(a, b) {
            return a.zLayer - b.zLayer;
        });
    };
    /* }}} */

    /* {{{ getZoneDensity
     * Get density (element count) for a zone.
     */
    SceneComposition.prototype.getZoneDensity = function(zoneName) {
        return this.zoneDensity[zoneName] || 0;
    };
    /* }}} */

    /* {{{ isZoneCrowded
     * Check if a zone is too crowded.
     */
    SceneComposition.prototype.isZoneCrowded = function(zoneName, maxDensity) {
        maxDensity = maxDensity || 5;
        return this.getZoneDensity(zoneName) >= maxDensity;
    };
    /* }}} */

    /* {{{ buildPromptForZone
     * Build composition-aware prompt additions for a zone.
     */
    SceneComposition.prototype.buildPromptForZone = function(zoneName, playerId) {
        var parts = [];
        var zone = ZONES[zoneName];

        if (!zone) return '';

        /* Position hints */
        if (zoneName === 'sky') {
            parts.push('background sky');
            parts.push('distant horizon');
        } else if (zoneName.indexOf('base') !== -1) {
            parts.push('fortification');
            parts.push('ground level');
            parts.push(playerId === 1 ? 'left side of scene' : 'right side of scene');
        } else if (zoneName.indexOf('forces') !== -1) {
            parts.push('mid-ground');
            parts.push(playerId === 1 ? 'left army' : 'right army');
        } else if (zoneName === 'center') {
            parts.push('center of action');
            parts.push('foreground');
            parts.push('dynamic combat');
        }

        /* Layer hints */
        switch (zone.zLayer) {
            case Z_LAYER.BACKGROUND:
                parts.push('atmospheric perspective');
                break;
            case Z_LAYER.BASE:
                parts.push('solid foundation');
                break;
            case Z_LAYER.FORCES:
                parts.push('detailed figures');
                break;
            case Z_LAYER.ACTION:
                parts.push('sharp focus');
                parts.push('dramatic lighting');
                break;
        }

        return parts.join(', ');
    };
    /* }}} */

    /* {{{ clear
     * Clear all tracked elements.
     */
    SceneComposition.prototype.clear = function() {
        this.elements = [];
        for (var zone in this.zoneDensity) {
            this.zoneDensity[zone] = 0;
        }
    };
    /* }}} */

    /* {{{ getStats
     * Get composition statistics.
     */
    SceneComposition.prototype.getStats = function() {
        return {
            totalElements: this.elements.length,
            zoneDensity: Object.assign({}, this.zoneDensity),
            canvasSize: { width: this.canvasWidth, height: this.canvasHeight }
        };
    };
    /* }}} */

    /* }}} */

    /* {{{ Public API */
    window.SceneComposition = SceneComposition;

    /* Export constants */
    window.SceneComposition.Z_LAYER = Z_LAYER;
    window.SceneComposition.ZONES = ZONES;
    window.SceneComposition.ELEMENT_TYPE = ELEMENT_TYPE;

    /* Create default instance */
    window.sceneComposition = new SceneComposition();
    /* }}} */

})();
