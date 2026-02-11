/**
 * battle-canvas.js - Battle Canvas Manager for Symbeline Realms
 *
 * Manages the full battle scene image with support for progressive
 * inpainting, region tracking, history/undo, and final export.
 * Works with ComfyUI for image generation.
 */

(function() {
    'use strict';

    /* {{{ Canvas regions */
    var REGION = {
        SKY: 'sky',
        P1_FORCES: 'p1_forces',
        CENTER: 'center',
        P2_FORCES: 'p2_forces',
        P1_BASE: 'p1_base',
        P2_BASE: 'p2_base'
    };

    /* Region bounds as percentage of canvas */
    var REGION_BOUNDS = {
        sky: { x: 0, y: 0, width: 100, height: 20 },
        p1_forces: { x: 0, y: 20, width: 35, height: 50 },
        center: { x: 30, y: 20, width: 40, height: 50 },
        p2_forces: { x: 65, y: 20, width: 35, height: 50 },
        p1_base: { x: 0, y: 70, width: 40, height: 30 },
        p2_base: { x: 60, y: 70, width: 40, height: 30 }
    };
    /* }}} */

    /* {{{ Default options */
    var DEFAULT_OPTIONS = {
        width: 512,
        height: 512,
        maxHistory: 50,
        backgroundColor: '#1a1a2e'
    };
    /* }}} */

    /* {{{ BattleCanvas class */
    function BattleCanvas(options) {
        options = options || {};

        this.width = options.width || DEFAULT_OPTIONS.width;
        this.height = options.height || DEFAULT_OPTIONS.height;
        this.maxHistory = options.maxHistory || DEFAULT_OPTIONS.maxHistory;
        this.backgroundColor = options.backgroundColor || DEFAULT_OPTIONS.backgroundColor;

        /* Canvas element */
        this.canvas = document.createElement('canvas');
        this.canvas.width = this.width;
        this.canvas.height = this.height;
        this.ctx = this.canvas.getContext('2d');

        /* Region state */
        this.regionFilled = {};
        for (var region in REGION) {
            this.regionFilled[REGION[region]] = false;
        }

        /* History for undo/replay */
        this.history = [];
        this.historyIndex = -1;

        /* Initialize with background */
        this._fillBackground();
    }

    /* {{{ _fillBackground
     * Internal: Fill canvas with background color.
     */
    BattleCanvas.prototype._fillBackground = function() {
        this.ctx.fillStyle = this.backgroundColor;
        this.ctx.fillRect(0, 0, this.width, this.height);
    };
    /* }}} */

    /* {{{ getRegionBounds
     * Get pixel bounds for a region.
     */
    BattleCanvas.prototype.getRegionBounds = function(region) {
        var pct = REGION_BOUNDS[region];
        if (!pct) return null;

        return {
            x: Math.floor(this.width * pct.x / 100),
            y: Math.floor(this.height * pct.y / 100),
            width: Math.floor(this.width * pct.width / 100),
            height: Math.floor(this.height * pct.height / 100)
        };
    };
    /* }}} */

    /* {{{ setRegion
     * Set a region with an image.
     * @param region - Region identifier from REGION
     * @param image - Image, Canvas, or ImageData
     * @param saveHistory - Whether to save state (default true)
     */
    BattleCanvas.prototype.setRegion = function(region, image, saveHistory) {
        var bounds = this.getRegionBounds(region);
        if (!bounds) {
            console.warn('BattleCanvas: Unknown region', region);
            return false;
        }

        try {
            /* Draw image to region */
            this.ctx.drawImage(
                image,
                bounds.x, bounds.y,
                bounds.width, bounds.height
            );

            this.regionFilled[region] = true;

            if (saveHistory !== false) {
                this._saveHistory();
            }

            return true;
        } catch (e) {
            console.error('BattleCanvas: Failed to set region:', e);
            return false;
        }
    };
    /* }}} */

    /* {{{ setRegionFromUrl
     * Load and set a region from image URL.
     * Returns Promise that resolves when complete.
     */
    BattleCanvas.prototype.setRegionFromUrl = function(region, url) {
        var self = this;

        return new Promise(function(resolve, reject) {
            var img = new Image();
            img.crossOrigin = 'anonymous';

            img.onload = function() {
                var success = self.setRegion(region, img);
                if (success) {
                    resolve();
                } else {
                    reject(new Error('Failed to draw image'));
                }
            };

            img.onerror = function() {
                reject(new Error('Failed to load image: ' + url));
            };

            img.src = url;
        });
    };
    /* }}} */

    /* {{{ setRegionFromBase64
     * Set a region from base64 image data.
     */
    BattleCanvas.prototype.setRegionFromBase64 = function(region, base64) {
        var self = this;

        return new Promise(function(resolve, reject) {
            var img = new Image();

            img.onload = function() {
                var success = self.setRegion(region, img);
                if (success) {
                    resolve();
                } else {
                    reject(new Error('Failed to draw image'));
                }
            };

            img.onerror = function() {
                reject(new Error('Failed to load base64 image'));
            };

            /* Ensure proper data URL format */
            if (base64.indexOf('data:') !== 0) {
                base64 = 'data:image/png;base64,' + base64;
            }
            img.src = base64;
        });
    };
    /* }}} */

    /* {{{ clearRegion
     * Clear a specific region.
     */
    BattleCanvas.prototype.clearRegion = function(region) {
        var bounds = this.getRegionBounds(region);
        if (!bounds) return false;

        this.ctx.fillStyle = this.backgroundColor;
        this.ctx.fillRect(bounds.x, bounds.y, bounds.width, bounds.height);
        this.regionFilled[region] = false;

        this._saveHistory();
        return true;
    };
    /* }}} */

    /* {{{ isRegionFilled
     * Check if a region has been filled.
     */
    BattleCanvas.prototype.isRegionFilled = function(region) {
        return this.regionFilled[region] === true;
    };
    /* }}} */

    /* {{{ getFilledRegions
     * Get list of filled region names.
     */
    BattleCanvas.prototype.getFilledRegions = function() {
        var filled = [];
        for (var region in this.regionFilled) {
            if (this.regionFilled[region]) {
                filled.push(region);
            }
        }
        return filled;
    };
    /* }}} */

    /* {{{ getUnfilledRegions
     * Get list of unfilled region names.
     */
    BattleCanvas.prototype.getUnfilledRegions = function() {
        var unfilled = [];
        for (var region in this.regionFilled) {
            if (!this.regionFilled[region]) {
                unfilled.push(region);
            }
        }
        return unfilled;
    };
    /* }}} */

    /* {{{ getMask
     * Generate inpainting mask for unfilled regions.
     * Returns a canvas with white for unfilled, black for filled.
     */
    BattleCanvas.prototype.getMask = function() {
        var maskCanvas = document.createElement('canvas');
        maskCanvas.width = this.width;
        maskCanvas.height = this.height;
        var ctx = maskCanvas.getContext('2d');

        /* Start with black (filled) */
        ctx.fillStyle = '#000000';
        ctx.fillRect(0, 0, this.width, this.height);

        /* Mark unfilled regions as white */
        ctx.fillStyle = '#FFFFFF';
        for (var region in this.regionFilled) {
            if (!this.regionFilled[region]) {
                var bounds = this.getRegionBounds(region);
                if (bounds) {
                    ctx.fillRect(bounds.x, bounds.y, bounds.width, bounds.height);
                }
            }
        }

        return maskCanvas;
    };
    /* }}} */

    /* {{{ getMaskForRegion
     * Generate inpainting mask for a specific region.
     */
    BattleCanvas.prototype.getMaskForRegion = function(region) {
        var bounds = this.getRegionBounds(region);
        if (!bounds) return null;

        var maskCanvas = document.createElement('canvas');
        maskCanvas.width = this.width;
        maskCanvas.height = this.height;
        var ctx = maskCanvas.getContext('2d');

        /* Black everywhere except the target region */
        ctx.fillStyle = '#000000';
        ctx.fillRect(0, 0, this.width, this.height);

        ctx.fillStyle = '#FFFFFF';
        ctx.fillRect(bounds.x, bounds.y, bounds.width, bounds.height);

        return maskCanvas;
    };
    /* }}} */

    /* {{{ _saveHistory
     * Internal: Save current state to history.
     */
    BattleCanvas.prototype._saveHistory = function() {
        /* Truncate future history if we're not at the end */
        if (this.historyIndex < this.history.length - 1) {
            this.history = this.history.slice(0, this.historyIndex + 1);
        }

        /* Save current state */
        var state = {
            imageData: this.ctx.getImageData(0, 0, this.width, this.height),
            regionFilled: Object.assign({}, this.regionFilled),
            timestamp: Date.now()
        };

        this.history.push(state);
        this.historyIndex = this.history.length - 1;

        /* Enforce max history */
        while (this.history.length > this.maxHistory) {
            this.history.shift();
            this.historyIndex--;
        }
    };
    /* }}} */

    /* {{{ undo
     * Undo to previous state.
     */
    BattleCanvas.prototype.undo = function() {
        if (this.historyIndex <= 0) return false;

        this.historyIndex--;
        var state = this.history[this.historyIndex];

        this.ctx.putImageData(state.imageData, 0, 0);
        this.regionFilled = Object.assign({}, state.regionFilled);

        return true;
    };
    /* }}} */

    /* {{{ redo
     * Redo to next state.
     */
    BattleCanvas.prototype.redo = function() {
        if (this.historyIndex >= this.history.length - 1) return false;

        this.historyIndex++;
        var state = this.history[this.historyIndex];

        this.ctx.putImageData(state.imageData, 0, 0);
        this.regionFilled = Object.assign({}, state.regionFilled);

        return true;
    };
    /* }}} */

    /* {{{ canUndo
     * Check if undo is available.
     */
    BattleCanvas.prototype.canUndo = function() {
        return this.historyIndex > 0;
    };
    /* }}} */

    /* {{{ canRedo
     * Check if redo is available.
     */
    BattleCanvas.prototype.canRedo = function() {
        return this.historyIndex < this.history.length - 1;
    };
    /* }}} */

    /* {{{ getHistoryCount
     * Get number of history states.
     */
    BattleCanvas.prototype.getHistoryCount = function() {
        return this.history.length;
    };
    /* }}} */

    /* {{{ clear
     * Clear entire canvas.
     */
    BattleCanvas.prototype.clear = function() {
        this._fillBackground();

        for (var region in this.regionFilled) {
            this.regionFilled[region] = false;
        }

        this._saveHistory();
    };
    /* }}} */

    /* {{{ reset
     * Reset canvas and history.
     */
    BattleCanvas.prototype.reset = function() {
        this._fillBackground();

        for (var region in this.regionFilled) {
            this.regionFilled[region] = false;
        }

        this.history = [];
        this.historyIndex = -1;
        this._saveHistory();
    };
    /* }}} */

    /* {{{ getCanvas
     * Get the underlying canvas element.
     */
    BattleCanvas.prototype.getCanvas = function() {
        return this.canvas;
    };
    /* }}} */

    /* {{{ getContext
     * Get the canvas 2D context.
     */
    BattleCanvas.prototype.getContext = function() {
        return this.ctx;
    };
    /* }}} */

    /* {{{ export
     * Export canvas as image data URL.
     * @param format - 'png' or 'jpeg' (default 'png')
     * @param quality - Quality for jpeg (0-1)
     */
    BattleCanvas.prototype.export = function(format, quality) {
        format = format || 'png';
        quality = quality || 0.9;

        var mimeType = format === 'jpeg' ? 'image/jpeg' : 'image/png';
        return this.canvas.toDataURL(mimeType, quality);
    };
    /* }}} */

    /* {{{ exportBlob
     * Export canvas as Blob.
     * Returns Promise that resolves to Blob.
     */
    BattleCanvas.prototype.exportBlob = function(format, quality) {
        var self = this;
        format = format || 'png';
        quality = quality || 0.9;

        var mimeType = format === 'jpeg' ? 'image/jpeg' : 'image/png';

        return new Promise(function(resolve) {
            self.canvas.toBlob(function(blob) {
                resolve(blob);
            }, mimeType, quality);
        });
    };
    /* }}} */

    /* {{{ renderTo
     * Render canvas to another canvas or context.
     */
    BattleCanvas.prototype.renderTo = function(target, x, y, width, height) {
        x = x || 0;
        y = y || 0;
        width = width || this.width;
        height = height || this.height;

        var ctx = target.getContext ? target.getContext('2d') : target;
        ctx.drawImage(this.canvas, x, y, width, height);
    };
    /* }}} */

    /* {{{ getStats
     * Get canvas statistics.
     */
    BattleCanvas.prototype.getStats = function() {
        return {
            width: this.width,
            height: this.height,
            filledRegions: this.getFilledRegions().length,
            totalRegions: Object.keys(REGION_BOUNDS).length,
            historyCount: this.history.length,
            historyIndex: this.historyIndex
        };
    };
    /* }}} */

    /* }}} */

    /* {{{ Public API */
    window.BattleCanvas = BattleCanvas;

    /* Export region constants */
    window.BattleCanvas.REGION = REGION;
    window.BattleCanvas.REGION_BOUNDS = REGION_BOUNDS;

    /* Create default instance */
    window.battleCanvas = new BattleCanvas();
    /* }}} */

})();
