/**
 * effects.js - Visual effects for Symbeline Realms
 *
 * Canvas-based visual effects for attacks, damage, authority changes,
 * and turn transitions. Uses the animation queue system.
 */

(function() {
    'use strict';

    /* {{{ State */
    const state = {
        activeEffects: [],
        damageNumbers: [],
        screenFlash: null,
        turnOverlay: null
    };
    /* }}} */

    /* {{{ Public API - window.effects */
    window.effects = {

        /* {{{ attackPlayer
         * Flash screen edge to indicate attack on player.
         * @param damage Amount of damage
         * @param targetSide 'left' or 'right' (or 'top' for opponent)
         * @param callback Called when complete
         */
        attackPlayer: function(damage, targetSide, callback) {
            state.screenFlash = {
                side: targetSide || 'top',
                progress: 0,
                color: '#ff4444'
            };

            window.animation.queue({
                type: window.animation.Type.ATTACK,
                update: function(progress) {
                    state.screenFlash.progress = progress;
                },
                complete: function() {
                    state.screenFlash = null;
                    /* Show damage number */
                    this.showDamage(damage, targetSide === 'top' ? 'opponent' : 'player');
                    if (callback) callback();
                }.bind(this)
            });
        },
        /* }}} */

        /* {{{ attackBase
         * Shake and highlight a base being attacked.
         * @param baseRect {x, y, w, h} position of the base
         * @param damage Amount of damage
         * @param callback Called when complete
         */
        attackBase: function(baseRect, damage, callback) {
            const effect = {
                type: 'baseShake',
                rect: baseRect,
                progress: 0
            };

            state.activeEffects.push(effect);

            window.animation.queue({
                type: window.animation.Type.DAMAGE,
                update: function(progress) {
                    effect.progress = progress;
                },
                complete: function() {
                    const idx = state.activeEffects.indexOf(effect);
                    if (idx >= 0) {
                        state.activeEffects.splice(idx, 1);
                    }
                    /* Show damage at base location */
                    this.showDamageAt(damage, baseRect.x + baseRect.w / 2, baseRect.y);
                    if (callback) callback();
                }.bind(this)
            });
        },
        /* }}} */

        /* {{{ showDamage
         * Show floating damage number.
         * @param damage Amount
         * @param target 'player' or 'opponent'
         */
        showDamage: function(damage, target) {
            const layout = window.canvasLayout ? window.canvasLayout.getLayout() : null;
            let x, y;

            if (target === 'opponent' && layout) {
                x = layout.status.x + layout.status.w / 2;
                y = layout.opponentBases ? layout.opponentBases.y + 50 : 100;
            } else if (layout) {
                x = layout.status.x + 100;
                y = layout.status.y + 30;
            } else {
                x = 400;
                y = target === 'opponent' ? 100 : 500;
            }

            this.showDamageAt(damage, x, y);
        },
        /* }}} */

        /* {{{ showDamageAt
         * Show floating damage number at specific position.
         */
        showDamageAt: function(damage, x, y) {
            const damageNum = {
                value: damage,
                x: x,
                y: y,
                startY: y,
                progress: 0
            };

            state.damageNumbers.push(damageNum);

            window.animation.queue({
                type: window.animation.Type.DAMAGE,
                update: function(progress) {
                    damageNum.progress = progress;
                    damageNum.y = damageNum.startY - progress * 60;
                },
                complete: function() {
                    const idx = state.damageNumbers.indexOf(damageNum);
                    if (idx >= 0) {
                        state.damageNumbers.splice(idx, 1);
                    }
                }
            });
        },
        /* }}} */

        /* {{{ showHealing
         * Show floating healing number.
         */
        showHealing: function(amount, target) {
            const layout = window.canvasLayout ? window.canvasLayout.getLayout() : null;
            let x, y;

            if (target === 'player' && layout) {
                x = layout.status.x + 100;
                y = layout.status.y + 30;
            } else {
                x = 400;
                y = target === 'opponent' ? 100 : 500;
            }

            const healNum = {
                value: amount,
                x: x,
                y: y,
                startY: y,
                progress: 0,
                isHealing: true
            };

            state.damageNumbers.push(healNum);

            window.animation.queue({
                type: window.animation.Type.AUTHORITY_CHANGE,
                update: function(progress) {
                    healNum.progress = progress;
                    healNum.y = healNum.startY - progress * 40;
                },
                complete: function() {
                    const idx = state.damageNumbers.indexOf(healNum);
                    if (idx >= 0) {
                        state.damageNumbers.splice(idx, 1);
                    }
                }
            });
        },
        /* }}} */

        /* {{{ authorityChange
         * Animate authority number changing.
         * @param from Starting value
         * @param to Ending value
         * @param target 'player' or 'opponent'
         * @param callback Called when complete
         */
        authorityChange: function(from, to, target, callback) {
            const effect = {
                type: 'authorityTick',
                from: from,
                to: to,
                current: from,
                target: target,
                progress: 0
            };

            state.activeEffects.push(effect);

            window.animation.queue({
                type: window.animation.Type.AUTHORITY_CHANGE,
                update: function(progress) {
                    effect.progress = progress;
                    effect.current = Math.round(from + (to - from) * progress);
                },
                complete: function() {
                    const idx = state.activeEffects.indexOf(effect);
                    if (idx >= 0) {
                        state.activeEffects.splice(idx, 1);
                    }
                    if (callback) callback();
                }
            });
        },
        /* }}} */

        /* {{{ turnChange
         * Show turn change overlay.
         * @param isYourTurn Boolean
         * @param turnNumber Optional turn number
         * @param callback Called when complete
         */
        turnChange: function(isYourTurn, turnNumber, callback) {
            state.turnOverlay = {
                text: isYourTurn ? 'Your Turn' : "Opponent's Turn",
                subtext: turnNumber ? 'Turn ' + turnNumber : null,
                progress: 0
            };

            window.animation.queue({
                type: window.animation.Type.TURN_CHANGE,
                update: function(progress) {
                    state.turnOverlay.progress = progress;
                },
                complete: function() {
                    state.turnOverlay = null;
                    if (callback) callback();
                }
            });
        },
        /* }}} */

        /* {{{ tradeGain
         * Visual feedback for gaining trade.
         */
        tradeGain: function(amount) {
            const effect = {
                type: 'resourceGain',
                resource: 'trade',
                amount: amount,
                progress: 0
            };

            state.activeEffects.push(effect);

            window.animation.queue({
                type: window.animation.Type.AUTHORITY_CHANGE,
                update: function(progress) {
                    effect.progress = progress;
                },
                complete: function() {
                    const idx = state.activeEffects.indexOf(effect);
                    if (idx >= 0) {
                        state.activeEffects.splice(idx, 1);
                    }
                }
            });
        },
        /* }}} */

        /* {{{ combatGain
         * Visual feedback for gaining combat.
         */
        combatGain: function(amount) {
            const effect = {
                type: 'resourceGain',
                resource: 'combat',
                amount: amount,
                progress: 0
            };

            state.activeEffects.push(effect);

            window.animation.queue({
                type: window.animation.Type.AUTHORITY_CHANGE,
                update: function(progress) {
                    effect.progress = progress;
                },
                complete: function() {
                    const idx = state.activeEffects.indexOf(effect);
                    if (idx >= 0) {
                        state.activeEffects.splice(idx, 1);
                    }
                }
            });
        },
        /* }}} */

        /* {{{ render
         * Render all active effects.
         * Call at end of render loop, after card animations.
         */
        render: function(ctx, canvasWidth, canvasHeight) {
            /* Screen flash */
            if (state.screenFlash) {
                this._renderScreenFlash(ctx, canvasWidth, canvasHeight);
            }

            /* Base shake effects */
            state.activeEffects.forEach(function(effect) {
                if (effect.type === 'baseShake') {
                    this._renderBaseShake(ctx, effect);
                } else if (effect.type === 'resourceGain') {
                    this._renderResourceGain(ctx, effect);
                }
            }, this);

            /* Damage/healing numbers */
            state.damageNumbers.forEach(function(num) {
                this._renderDamageNumber(ctx, num);
            }, this);

            /* Turn overlay (render last, on top) */
            if (state.turnOverlay) {
                this._renderTurnOverlay(ctx, canvasWidth, canvasHeight);
            }
        },
        /* }}} */

        /* {{{ _renderScreenFlash */
        _renderScreenFlash: function(ctx, canvasWidth, canvasHeight) {
            const flash = state.screenFlash;
            const alpha = Math.sin(flash.progress * Math.PI) * 0.4;

            ctx.save();

            const gradient = ctx.createLinearGradient(0, 0, 0, canvasHeight);

            if (flash.side === 'top') {
                /* Flash from top (opponent attack) */
                gradient.addColorStop(0, 'rgba(255, 68, 68, ' + alpha + ')');
                gradient.addColorStop(0.3, 'rgba(255, 68, 68, 0)');
                ctx.fillStyle = gradient;
                ctx.fillRect(0, 0, canvasWidth, canvasHeight * 0.3);
            } else if (flash.side === 'bottom') {
                /* Flash from bottom (player damage) */
                gradient.addColorStop(0.7, 'rgba(255, 68, 68, 0)');
                gradient.addColorStop(1, 'rgba(255, 68, 68, ' + alpha + ')');
                ctx.fillStyle = gradient;
                ctx.fillRect(0, canvasHeight * 0.7, canvasWidth, canvasHeight * 0.3);
            }

            ctx.restore();
        },
        /* }}} */

        /* {{{ _renderBaseShake */
        _renderBaseShake: function(ctx, effect) {
            const rect = effect.rect;
            const shakeX = Math.sin(effect.progress * Math.PI * 6) * 5 * (1 - effect.progress);

            ctx.save();

            /* Red border highlight */
            ctx.strokeStyle = 'rgba(255, 68, 68, ' + (1 - effect.progress) + ')';
            ctx.lineWidth = 3;
            ctx.strokeRect(
                rect.x + shakeX - 2,
                rect.y - 2,
                rect.w + 4,
                rect.h + 4
            );

            ctx.restore();
        },
        /* }}} */

        /* {{{ _renderDamageNumber */
        _renderDamageNumber: function(ctx, num) {
            const alpha = 1 - num.progress;
            const scale = 1 + num.progress * 0.3;

            ctx.save();

            ctx.translate(num.x, num.y);
            ctx.scale(scale, scale);

            /* Shadow */
            ctx.fillStyle = 'rgba(0, 0, 0, ' + alpha * 0.8 + ')';
            ctx.font = 'bold 28px sans-serif';
            ctx.textAlign = 'center';
            ctx.fillText((num.isHealing ? '+' : '-') + num.value, 2, 2);

            /* Number */
            ctx.fillStyle = num.isHealing
                ? 'rgba(68, 255, 68, ' + alpha + ')'
                : 'rgba(255, 68, 68, ' + alpha + ')';
            ctx.fillText((num.isHealing ? '+' : '-') + num.value, 0, 0);

            ctx.restore();
        },
        /* }}} */

        /* {{{ _renderResourceGain */
        _renderResourceGain: function(ctx, effect) {
            const layout = window.canvasLayout ? window.canvasLayout.getLayout() : null;
            if (!layout) return;

            const alpha = Math.sin(effect.progress * Math.PI);
            let x, y, color, symbol;

            if (effect.resource === 'trade') {
                x = layout.status.x + layout.status.w / 2 - 50;
                y = layout.status.y + 20;
                color = '#ffd700';
                symbol = '+' + effect.amount + ' \u26C1';
            } else if (effect.resource === 'combat') {
                x = layout.status.x + layout.status.w / 2 + 50;
                y = layout.status.y + 20;
                color = '#ff4444';
                symbol = '+' + effect.amount + ' \u2694';
            }

            ctx.save();

            ctx.globalAlpha = alpha;
            ctx.fillStyle = color;
            ctx.font = 'bold 18px sans-serif';
            ctx.textAlign = 'center';
            ctx.fillText(symbol, x, y - effect.progress * 20);

            ctx.restore();
        },
        /* }}} */

        /* {{{ _renderTurnOverlay */
        _renderTurnOverlay: function(ctx, canvasWidth, canvasHeight) {
            const overlay = state.turnOverlay;
            /* Fade in first half, fade out second half */
            const alpha = overlay.progress < 0.5
                ? overlay.progress * 2
                : (1 - overlay.progress) * 2;

            ctx.save();

            /* Darken background */
            ctx.fillStyle = 'rgba(0, 0, 0, ' + alpha * 0.5 + ')';
            ctx.fillRect(0, 0, canvasWidth, canvasHeight);

            /* Main text */
            ctx.fillStyle = 'rgba(212, 175, 55, ' + alpha + ')';  /* Gold */
            ctx.font = 'bold 48px Georgia, serif';
            ctx.textAlign = 'center';
            ctx.textBaseline = 'middle';

            /* Text shadow */
            ctx.shadowColor = 'rgba(0, 0, 0, ' + alpha + ')';
            ctx.shadowBlur = 10;
            ctx.shadowOffsetX = 3;
            ctx.shadowOffsetY = 3;

            ctx.fillText(overlay.text, canvasWidth / 2, canvasHeight / 2);

            /* Subtext (turn number) */
            if (overlay.subtext) {
                ctx.font = '24px Georgia, serif';
                ctx.fillStyle = 'rgba(180, 150, 80, ' + alpha + ')';
                ctx.fillText(overlay.subtext, canvasWidth / 2, canvasHeight / 2 + 40);
            }

            ctx.restore();
        },
        /* }}} */

        /* {{{ isAnimating */
        isAnimating: function() {
            return state.activeEffects.length > 0 ||
                   state.damageNumbers.length > 0 ||
                   state.screenFlash !== null ||
                   state.turnOverlay !== null;
        },
        /* }}} */

        /* {{{ clear
         * Clear all active effects.
         */
        clear: function() {
            state.activeEffects = [];
            state.damageNumbers = [];
            state.screenFlash = null;
            state.turnOverlay = null;
        }
        /* }}} */

    };
    /* }}} */

})();
