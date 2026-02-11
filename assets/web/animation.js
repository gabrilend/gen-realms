/**
 * animation.js - Animation system for Symbeline Realms browser client
 *
 * Provides queued animations with easing functions for smooth card
 * movements, attacks, and UI feedback. Integrates with preferences
 * for speed control and accessibility (reduce motion).
 */

(function() {
    'use strict';

    /* {{{ Animation types */
    const AnimationType = {
        PLAY_CARD: 'play_card',       /* Card from hand to play area */
        BUY_CARD: 'buy_card',         /* Card from trade row to discard */
        DRAW_CARD: 'draw_card',       /* Card from deck to hand */
        ATTACK: 'attack',             /* Combat effect toward target */
        DAMAGE: 'damage',             /* Shake/flash on damage taken */
        AUTHORITY_CHANGE: 'authority_change',  /* Authority number change */
        SCRAP_CARD: 'scrap_card',     /* Card removal animation */
        TURN_CHANGE: 'turn_change',   /* Turn transition overlay */
        CARD_FLIP: 'card_flip'        /* Card reveal animation */
    };
    /* }}} */

    /* {{{ Animation configuration */
    const animationConfig = {
        [AnimationType.PLAY_CARD]: {
            duration: 300,
            easing: 'easeOut'
        },
        [AnimationType.BUY_CARD]: {
            duration: 400,
            easing: 'easeInOut'
        },
        [AnimationType.DRAW_CARD]: {
            duration: 250,
            easing: 'easeOut'
        },
        [AnimationType.ATTACK]: {
            duration: 500,
            easing: 'linear'
        },
        [AnimationType.DAMAGE]: {
            duration: 200,
            easing: 'shake'
        },
        [AnimationType.AUTHORITY_CHANGE]: {
            duration: 400,
            easing: 'easeOut'
        },
        [AnimationType.SCRAP_CARD]: {
            duration: 350,
            easing: 'fadeOut'
        },
        [AnimationType.TURN_CHANGE]: {
            duration: 800,
            easing: 'fadeInOut'
        },
        [AnimationType.CARD_FLIP]: {
            duration: 300,
            easing: 'easeInOut'
        }
    };
    /* }}} */

    /* {{{ Easing functions */
    const easings = {
        /* Linear - constant speed */
        linear: function(t) {
            return t;
        },

        /* Ease in - slow start, fast end */
        easeIn: function(t) {
            return t * t;
        },

        /* Ease out - fast start, slow end (landing) */
        easeOut: function(t) {
            return t * (2 - t);
        },

        /* Ease in-out - smooth acceleration/deceleration */
        easeInOut: function(t) {
            return t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t;
        },

        /* Shake - oscillating for damage feedback */
        shake: function(t) {
            return Math.sin(t * Math.PI * 4) * (1 - t);
        },

        /* Fade in-out - peak at middle (for overlays) */
        fadeInOut: function(t) {
            return t < 0.5 ? 2 * t : 2 * (1 - t);
        },

        /* Fade out - linear opacity decrease */
        fadeOut: function(t) {
            return 1 - t;
        },

        /* Bounce - slight overshoot and settle */
        bounce: function(t) {
            if (t < 0.5) {
                return 2 * t * t;
            } else {
                /* Slight overshoot then settle */
                const p = t - 0.5;
                return 0.5 + 0.5 * (1 - Math.pow(1 - 2 * p, 2) * Math.cos(p * Math.PI));
            }
        },

        /* Elastic - springy overshoot */
        elastic: function(t) {
            if (t === 0 || t === 1) return t;
            return Math.pow(2, -10 * t) * Math.sin((t - 0.075) * (2 * Math.PI) / 0.3) + 1;
        }
    };
    /* }}} */

    /* {{{ State */
    const state = {
        queue: [],
        isAnimating: false,
        currentAnimation: null,
        speed: 1.0,
        reduceMotion: false,
        paused: false
    };
    /* }}} */

    /* {{{ runAnimation
     * Execute a single animation with requestAnimationFrame.
     */
    function runAnimation(animation, callback) {
        const config = animationConfig[animation.type] || {
            duration: 300,
            easing: 'linear'
        };

        /* Apply speed multiplier (faster = shorter duration) */
        const duration = config.duration / state.speed;
        const easing = easings[config.easing] || easings.linear;
        const start = performance.now();

        state.currentAnimation = animation;

        /* {{{ frame */
        function frame(now) {
            if (state.paused) {
                /* If paused, keep requesting frames but don't advance */
                requestAnimationFrame(frame);
                return;
            }

            const elapsed = now - start;
            const t = Math.min(elapsed / duration, 1);
            const progress = easing(t);

            /* Call the animation's update function with progress (0-1) */
            if (animation.update) {
                animation.update(progress, t);
            }

            if (t < 1) {
                requestAnimationFrame(frame);
            } else {
                /* Animation complete */
                state.currentAnimation = null;

                if (animation.complete) {
                    animation.complete();
                }

                callback();
            }
        }
        /* }}} */

        requestAnimationFrame(frame);
    }
    /* }}} */

    /* {{{ processQueue
     * Process the next animation in the queue.
     */
    function processQueue() {
        if (state.queue.length === 0) {
            state.isAnimating = false;
            return;
        }

        state.isAnimating = true;
        const animation = state.queue.shift();

        /* If reduce motion is on, skip to completion */
        if (state.reduceMotion) {
            if (animation.update) {
                animation.update(1, 1);  /* Jump to end state */
            }
            if (animation.complete) {
                animation.complete();
            }
            /* Process next immediately */
            setTimeout(processQueue, 0);
            return;
        }

        runAnimation(animation, processQueue);
    }
    /* }}} */

    /* {{{ Public API - window.animation */
    window.animation = {

        /* Expose animation types for external use */
        Type: AnimationType,

        /* {{{ queue
         * Add an animation to the queue.
         * @param animation Object with:
         *   - type: AnimationType value
         *   - update(progress, rawT): called each frame with eased progress
         *   - complete(): called when animation finishes (optional)
         *   - data: arbitrary data for the animation (optional)
         */
        queue: function(animation) {
            if (!animation || !animation.type) {
                console.error('Animation requires a type');
                return;
            }

            state.queue.push(animation);

            if (!state.isAnimating) {
                processQueue();
            }
        },
        /* }}} */

        /* {{{ queueMultiple
         * Queue multiple animations at once.
         */
        queueMultiple: function(animations) {
            animations.forEach(function(anim) {
                this.queue(anim);
            }, this);
        },
        /* }}} */

        /* {{{ clear
         * Clear all pending animations (current one finishes).
         */
        clear: function() {
            state.queue = [];
        },
        /* }}} */

        /* {{{ skip
         * Skip current animation and all queued animations.
         */
        skip: function() {
            /* Complete current animation immediately */
            if (state.currentAnimation) {
                if (state.currentAnimation.update) {
                    state.currentAnimation.update(1, 1);
                }
                if (state.currentAnimation.complete) {
                    state.currentAnimation.complete();
                }
            }

            /* Complete all queued animations immediately */
            state.queue.forEach(function(anim) {
                if (anim.update) {
                    anim.update(1, 1);
                }
                if (anim.complete) {
                    anim.complete();
                }
            });

            state.queue = [];
            state.currentAnimation = null;
            state.isAnimating = false;
        },
        /* }}} */

        /* {{{ pause */
        pause: function() {
            state.paused = true;
        },
        /* }}} */

        /* {{{ resume */
        resume: function() {
            state.paused = false;
        },
        /* }}} */

        /* {{{ isPaused */
        isPaused: function() {
            return state.paused;
        },
        /* }}} */

        /* {{{ isAnimating */
        isAnimating: function() {
            return state.isAnimating;
        },
        /* }}} */

        /* {{{ getQueueLength */
        getQueueLength: function() {
            return state.queue.length;
        },
        /* }}} */

        /* {{{ setSpeed
         * Set animation speed multiplier.
         * @param speed Number (0.1 to 5.0)
         */
        setSpeed: function(speed) {
            state.speed = Math.max(0.1, Math.min(5.0, speed));
        },
        /* }}} */

        /* {{{ getSpeed */
        getSpeed: function() {
            return state.speed;
        },
        /* }}} */

        /* {{{ setReduceMotion
         * Enable/disable reduced motion mode (accessibility).
         */
        setReduceMotion: function(enabled) {
            state.reduceMotion = !!enabled;
        },
        /* }}} */

        /* {{{ getReduceMotion */
        getReduceMotion: function() {
            return state.reduceMotion;
        },
        /* }}} */

        /* {{{ loadFromPreferences
         * Load animation settings from preferences.
         */
        loadFromPreferences: function() {
            if (window.preferences) {
                const prefs = window.preferences.load();
                this.setSpeed(prefs.animationSpeed || 1.0);
                this.setReduceMotion(prefs.reduceMotion || false);
            }
        },
        /* }}} */

        /* {{{ getEasing
         * Get an easing function by name.
         */
        getEasing: function(name) {
            return easings[name] || easings.linear;
        },
        /* }}} */

        /* {{{ getConfig
         * Get animation configuration for a type.
         */
        getConfig: function(type) {
            return animationConfig[type] || { duration: 300, easing: 'linear' };
        },
        /* }}} */

        /* {{{ interpolate
         * Helper to interpolate between two values.
         */
        interpolate: function(start, end, progress) {
            return start + (end - start) * progress;
        },
        /* }}} */

        /* {{{ interpolatePoint
         * Helper to interpolate between two {x, y} points.
         */
        interpolatePoint: function(start, end, progress) {
            return {
                x: this.interpolate(start.x, end.x, progress),
                y: this.interpolate(start.y, end.y, progress)
            };
        },
        /* }}} */

        /* {{{ interpolateColor
         * Helper to interpolate between two RGB colors.
         * Colors should be objects with r, g, b (0-255).
         */
        interpolateColor: function(start, end, progress) {
            return {
                r: Math.round(this.interpolate(start.r, end.r, progress)),
                g: Math.round(this.interpolate(start.g, end.g, progress)),
                b: Math.round(this.interpolate(start.b, end.b, progress))
            };
        }
        /* }}} */

    };
    /* }}} */

})();
