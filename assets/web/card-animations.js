/**
 * card-animations.js - Card movement animations for Symbeline Realms
 *
 * Canvas-based card animations for play, buy, draw, and scrap actions.
 * Uses the animation queue system and renders animated cards on top of
 * the regular game rendering.
 */

(function() {
    'use strict';

    /* {{{ State */
    const state = {
        activeAnimations: [],  /* Currently rendering animations */
        cardRenderer: null     /* Reference to card renderer for drawing */
    };
    /* }}} */

    /* {{{ Public API - window.cardAnimations */
    window.cardAnimations = {

        /* {{{ init
         * Initialize with reference to card renderer.
         */
        init: function(cardRenderer) {
            state.cardRenderer = cardRenderer || window.cardRenderer;
        },
        /* }}} */

        /* {{{ playCard
         * Animate card from hand to played area.
         * @param card The card object to animate
         * @param from {x, y, w, h} starting position
         * @param to {x, y, w, h} ending position
         * @param callback Called when animation completes
         */
        playCard: function(card, from, to, callback) {
            const animData = {
                card: card,
                fromX: from.x,
                fromY: from.y,
                fromW: from.w || 100,
                fromH: from.h || 140,
                toX: to.x,
                toY: to.y,
                toW: to.w || 100,
                toH: to.h || 140,
                progress: 0,
                type: 'play'
            };

            state.activeAnimations.push(animData);

            window.animation.queue({
                type: window.animation.Type.PLAY_CARD,
                update: function(progress) {
                    animData.progress = progress;
                },
                complete: function() {
                    /* Remove from active animations */
                    const idx = state.activeAnimations.indexOf(animData);
                    if (idx >= 0) {
                        state.activeAnimations.splice(idx, 1);
                    }
                    if (callback) callback();
                }
            });
        },
        /* }}} */

        /* {{{ buyCard
         * Animate card from trade row to discard pile (with arc).
         */
        buyCard: function(card, from, to, callback) {
            const animData = {
                card: card,
                fromX: from.x,
                fromY: from.y,
                fromW: from.w || 100,
                fromH: from.h || 140,
                toX: to.x,
                toY: to.y,
                toW: to.w || 80,
                toH: to.h || 110,
                progress: 0,
                type: 'buy'
            };

            state.activeAnimations.push(animData);

            window.animation.queue({
                type: window.animation.Type.BUY_CARD,
                update: function(progress) {
                    animData.progress = progress;
                },
                complete: function() {
                    const idx = state.activeAnimations.indexOf(animData);
                    if (idx >= 0) {
                        state.activeAnimations.splice(idx, 1);
                    }
                    if (callback) callback();
                }
            });
        },
        /* }}} */

        /* {{{ drawCard
         * Animate card from deck indicator to hand.
         */
        drawCard: function(card, from, to, callback) {
            const animData = {
                card: card,
                fromX: from.x,
                fromY: from.y,
                fromW: from.w || 60,
                fromH: from.h || 84,
                toX: to.x,
                toY: to.y,
                toW: to.w || 100,
                toH: to.h || 140,
                progress: 0,
                type: 'draw',
                showBack: true  /* Start with card back */
            };

            state.activeAnimations.push(animData);

            window.animation.queue({
                type: window.animation.Type.DRAW_CARD,
                update: function(progress) {
                    animData.progress = progress;
                    /* Flip at halfway point */
                    animData.showBack = progress < 0.5;
                },
                complete: function() {
                    const idx = state.activeAnimations.indexOf(animData);
                    if (idx >= 0) {
                        state.activeAnimations.splice(idx, 1);
                    }
                    if (callback) callback();
                }
            });
        },
        /* }}} */

        /* {{{ scrapCard
         * Animate card fading and shrinking away.
         */
        scrapCard: function(card, from, callback) {
            const animData = {
                card: card,
                fromX: from.x,
                fromY: from.y,
                fromW: from.w || 100,
                fromH: from.h || 140,
                progress: 0,
                type: 'scrap'
            };

            state.activeAnimations.push(animData);

            window.animation.queue({
                type: window.animation.Type.SCRAP_CARD,
                update: function(progress) {
                    animData.progress = progress;
                },
                complete: function() {
                    const idx = state.activeAnimations.indexOf(animData);
                    if (idx >= 0) {
                        state.activeAnimations.splice(idx, 1);
                    }
                    if (callback) callback();
                }
            });
        },
        /* }}} */

        /* {{{ flipCard
         * Animate card flip reveal.
         */
        flipCard: function(card, position, callback) {
            const animData = {
                card: card,
                fromX: position.x,
                fromY: position.y,
                fromW: position.w || 100,
                fromH: position.h || 140,
                progress: 0,
                type: 'flip',
                showBack: true
            };

            state.activeAnimations.push(animData);

            window.animation.queue({
                type: window.animation.Type.CARD_FLIP,
                update: function(progress) {
                    animData.progress = progress;
                    animData.showBack = progress < 0.5;
                },
                complete: function() {
                    const idx = state.activeAnimations.indexOf(animData);
                    if (idx >= 0) {
                        state.activeAnimations.splice(idx, 1);
                    }
                    if (callback) callback();
                }
            });
        },
        /* }}} */

        /* {{{ render
         * Render all active card animations.
         * Call this at the end of your render loop.
         */
        render: function(ctx) {
            if (!state.cardRenderer) {
                state.cardRenderer = window.cardRenderer;
            }

            state.activeAnimations.forEach(function(anim) {
                ctx.save();

                const p = anim.progress;

                switch (anim.type) {
                    case 'play':
                        this._renderPlayAnimation(ctx, anim, p);
                        break;

                    case 'buy':
                        this._renderBuyAnimation(ctx, anim, p);
                        break;

                    case 'draw':
                        this._renderDrawAnimation(ctx, anim, p);
                        break;

                    case 'scrap':
                        this._renderScrapAnimation(ctx, anim, p);
                        break;

                    case 'flip':
                        this._renderFlipAnimation(ctx, anim, p);
                        break;
                }

                ctx.restore();
            }, this);
        },
        /* }}} */

        /* {{{ _renderPlayAnimation
         * Render card moving from hand to play area.
         */
        _renderPlayAnimation: function(ctx, anim, p) {
            const x = window.animation.interpolate(anim.fromX, anim.toX, p);
            const y = window.animation.interpolate(anim.fromY, anim.toY, p);
            const w = window.animation.interpolate(anim.fromW, anim.toW, p);
            const h = window.animation.interpolate(anim.fromH, anim.toH, p);

            /* Slight lift in middle of animation */
            const lift = Math.sin(p * Math.PI) * -20;

            /* Scale bump at peak */
            const scaleBump = 1 + Math.sin(p * Math.PI) * 0.1;

            ctx.translate(x + w / 2, y + h / 2 + lift);
            ctx.scale(scaleBump, scaleBump);
            ctx.translate(-w / 2, -h / 2);

            if (state.cardRenderer) {
                state.cardRenderer.renderCard(ctx, anim.card, 0, 0, w, h);
            }
        },
        /* }}} */

        /* {{{ _renderBuyAnimation
         * Render card arcing from trade row to discard.
         */
        _renderBuyAnimation: function(ctx, anim, p) {
            const x = window.animation.interpolate(anim.fromX, anim.toX, p);
            const y = window.animation.interpolate(anim.fromY, anim.toY, p);
            const w = window.animation.interpolate(anim.fromW, anim.toW, p);
            const h = window.animation.interpolate(anim.fromH, anim.toH, p);

            /* Arc upward in middle */
            const arc = Math.sin(p * Math.PI) * -80;

            /* Fade slightly toward end */
            const alpha = p > 0.7 ? 1 - (p - 0.7) / 0.3 : 1;

            ctx.globalAlpha = alpha;
            ctx.translate(x + w / 2, y + h / 2 + arc);
            ctx.translate(-w / 2, -h / 2);

            if (state.cardRenderer) {
                state.cardRenderer.renderCard(ctx, anim.card, 0, 0, w, h);
            }
        },
        /* }}} */

        /* {{{ _renderDrawAnimation
         * Render card coming from deck with flip.
         */
        _renderDrawAnimation: function(ctx, anim, p) {
            const x = window.animation.interpolate(anim.fromX, anim.toX, p);
            const y = window.animation.interpolate(anim.fromY, anim.toY, p);
            const w = window.animation.interpolate(anim.fromW, anim.toW, p);
            const h = window.animation.interpolate(anim.fromH, anim.toH, p);

            /* Flip effect - scale X goes to 0 at midpoint */
            let scaleX;
            if (p < 0.5) {
                scaleX = 1 - p * 2;
            } else {
                scaleX = (p - 0.5) * 2;
            }

            ctx.translate(x + w / 2, y + h / 2);
            ctx.scale(scaleX, 1);
            ctx.translate(-w / 2, -h / 2);

            if (state.cardRenderer) {
                if (anim.showBack) {
                    state.cardRenderer.renderCardBack(ctx, 0, 0, w, h);
                } else {
                    state.cardRenderer.renderCard(ctx, anim.card, 0, 0, w, h);
                }
            }
        },
        /* }}} */

        /* {{{ _renderScrapAnimation
         * Render card shrinking and fading.
         */
        _renderScrapAnimation: function(ctx, anim, p) {
            const x = anim.fromX;
            const y = anim.fromY;
            const w = anim.fromW;
            const h = anim.fromH;

            /* Shrink to center */
            const scale = 1 - p;

            /* Fade out */
            const alpha = 1 - p;

            /* Slight upward drift */
            const drift = -p * 30;

            /* Spin slightly */
            const rotation = p * 0.5;

            ctx.globalAlpha = alpha;
            ctx.translate(x + w / 2, y + h / 2 + drift);
            ctx.rotate(rotation);
            ctx.scale(scale, scale);
            ctx.translate(-w / 2, -h / 2);

            if (state.cardRenderer) {
                state.cardRenderer.renderCard(ctx, anim.card, 0, 0, w, h);
            }
        },
        /* }}} */

        /* {{{ _renderFlipAnimation
         * Render card flip in place.
         */
        _renderFlipAnimation: function(ctx, anim, p) {
            const x = anim.fromX;
            const y = anim.fromY;
            const w = anim.fromW;
            const h = anim.fromH;

            /* Flip effect */
            let scaleX;
            if (p < 0.5) {
                scaleX = 1 - p * 2;
            } else {
                scaleX = (p - 0.5) * 2;
            }

            ctx.translate(x + w / 2, y + h / 2);
            ctx.scale(scaleX, 1);
            ctx.translate(-w / 2, -h / 2);

            if (state.cardRenderer) {
                if (anim.showBack) {
                    state.cardRenderer.renderCardBack(ctx, 0, 0, w, h);
                } else {
                    state.cardRenderer.renderCard(ctx, anim.card, 0, 0, w, h);
                }
            }
        },
        /* }}} */

        /* {{{ isAnimating
         * Check if any card animations are active.
         */
        isAnimating: function() {
            return state.activeAnimations.length > 0;
        },
        /* }}} */

        /* {{{ getActiveCount */
        getActiveCount: function() {
            return state.activeAnimations.length;
        },
        /* }}} */

        /* {{{ clear
         * Clear all active animations.
         */
        clear: function() {
            state.activeAnimations = [];
        }
        /* }}} */

    };
    /* }}} */

})();
