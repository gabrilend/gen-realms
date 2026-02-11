/**
 * upgrade-viz.js - Upgrade Visualization for Symbeline Realms
 *
 * Provides visual effects for card upgrades including ally bonuses,
 * empowered states, scrap effects, and targeting indicators.
 * Uses canvas overlays and particle effects.
 */

(function() {
    'use strict';

    /* {{{ Upgrade types */
    var UPGRADE_TYPE = {
        ALLY_ACTIVE: 'ally_active',     /* Same-faction card played */
        EMPOWERED: 'empowered',          /* Permanent stat boost */
        SCRAPPED: 'scrapped',            /* Being removed from game */
        TARGETED: 'targeted',            /* Selected as target */
        COMBO: 'combo'                   /* Multiple upgrades active */
    };
    /* }}} */

    /* {{{ Overlay configurations */
    var OVERLAYS = {
        ally_active: {
            glowColor: '#FFD700',
            glowRadius: 10,
            particleType: 'sparkle',
            overlayOpacity: 0.3,
            pulseSpeed: 1.5
        },
        empowered: {
            glowColor: '#FF4444',
            glowRadius: 15,
            particleType: 'flame',
            overlayOpacity: 0.4,
            pulseSpeed: 2.0
        },
        scrapped: {
            glowColor: '#888888',
            glowRadius: 5,
            particleType: 'dust',
            overlayOpacity: 0.6,
            pulseSpeed: 0.5
        },
        targeted: {
            glowColor: '#FF0000',
            glowRadius: 8,
            particleType: 'target',
            overlayOpacity: 0.5,
            pulseSpeed: 3.0
        },
        combo: {
            glowColor: '#9933FF',
            glowRadius: 12,
            particleType: 'energy',
            overlayOpacity: 0.35,
            pulseSpeed: 2.5
        }
    };
    /* }}} */

    /* {{{ Faction glow colors */
    var FACTION_GLOW = {
        merchant: '#FFD700',
        wilds: '#33FF33',
        kingdom: '#3399FF',
        artificer: '#FF6666',
        neutral: '#CCCCCC'
    };
    /* }}} */

    /* {{{ Particle class */
    function Particle(x, y, type, faction) {
        this.x = x;
        this.y = y;
        this.type = type;
        this.faction = faction;
        this.age = 0;
        this.maxAge = 60 + Math.random() * 60;
        this.vx = (Math.random() - 0.5) * 2;
        this.vy = -1 - Math.random() * 2;
        this.size = 2 + Math.random() * 4;
        this.alpha = 1.0;
    }

    Particle.prototype.update = function() {
        this.age++;
        this.x += this.vx;
        this.y += this.vy;

        /* Type-specific behavior */
        switch (this.type) {
            case 'sparkle':
                this.vy += 0.02;  /* Slight float up */
                this.alpha = 1 - (this.age / this.maxAge);
                break;
            case 'flame':
                this.vy -= 0.05;  /* Rise faster */
                this.vx *= 0.98;  /* Slow horizontal */
                this.alpha = Math.max(0, 1 - (this.age / this.maxAge));
                this.size *= 0.99;
                break;
            case 'dust':
                this.vy += 0.03;  /* Fall slowly */
                this.vx *= 0.95;
                this.alpha = 1 - (this.age / this.maxAge);
                break;
            case 'target':
                /* Pulse size */
                this.size = 3 + Math.sin(this.age * 0.2) * 2;
                this.alpha = 0.8;
                break;
            case 'energy':
                /* Spiral motion */
                var angle = this.age * 0.1;
                this.vx = Math.cos(angle) * 1.5;
                this.vy = Math.sin(angle) * 1.5 - 0.5;
                this.alpha = 0.7 - (this.age / this.maxAge) * 0.5;
                break;
        }

        return this.age < this.maxAge;
    };

    Particle.prototype.render = function(ctx) {
        var color = FACTION_GLOW[this.faction] || '#FFFFFF';

        ctx.save();
        ctx.globalAlpha = this.alpha;

        switch (this.type) {
            case 'sparkle':
                /* Four-pointed star */
                ctx.fillStyle = color;
                ctx.beginPath();
                ctx.moveTo(this.x, this.y - this.size);
                ctx.lineTo(this.x + this.size * 0.3, this.y);
                ctx.lineTo(this.x, this.y + this.size);
                ctx.lineTo(this.x - this.size * 0.3, this.y);
                ctx.closePath();
                ctx.fill();
                break;

            case 'flame':
                /* Gradient circle */
                var grad = ctx.createRadialGradient(
                    this.x, this.y, 0,
                    this.x, this.y, this.size
                );
                grad.addColorStop(0, color);
                grad.addColorStop(1, 'transparent');
                ctx.fillStyle = grad;
                ctx.beginPath();
                ctx.arc(this.x, this.y, this.size, 0, Math.PI * 2);
                ctx.fill();
                break;

            case 'dust':
                /* Simple circle */
                ctx.fillStyle = '#888888';
                ctx.beginPath();
                ctx.arc(this.x, this.y, this.size * 0.5, 0, Math.PI * 2);
                ctx.fill();
                break;

            case 'target':
                /* Crosshair element */
                ctx.strokeStyle = '#FF0000';
                ctx.lineWidth = 1;
                ctx.beginPath();
                ctx.arc(this.x, this.y, this.size, 0, Math.PI * 2);
                ctx.stroke();
                break;

            case 'energy':
                /* Colored orb */
                ctx.fillStyle = color;
                ctx.beginPath();
                ctx.arc(this.x, this.y, this.size, 0, Math.PI * 2);
                ctx.fill();
                break;
        }

        ctx.restore();
    };
    /* }}} */

    /* {{{ UpgradeViz class */
    function UpgradeViz() {
        this.particles = [];
        this.activeEffects = new Map();  /* card_id -> [effects] */
        this.animationFrame = null;
        this.lastUpdate = 0;
    }

    /* {{{ addEffect
     * Add an upgrade effect to a card.
     */
    UpgradeViz.prototype.addEffect = function(cardId, effectType, bounds, faction) {
        if (!this.activeEffects.has(cardId)) {
            this.activeEffects.set(cardId, []);
        }

        var effects = this.activeEffects.get(cardId);

        /* Check if effect already exists */
        for (var i = 0; i < effects.length; i++) {
            if (effects[i].type === effectType) {
                return;  /* Already active */
            }
        }

        effects.push({
            type: effectType,
            bounds: bounds,
            faction: faction || 'neutral',
            startTime: Date.now(),
            phase: 0
        });

        /* Start animation loop if not running */
        this._startAnimation();
    };
    /* }}} */

    /* {{{ removeEffect
     * Remove an upgrade effect from a card.
     */
    UpgradeViz.prototype.removeEffect = function(cardId, effectType) {
        if (!this.activeEffects.has(cardId)) {
            return;
        }

        var effects = this.activeEffects.get(cardId);
        for (var i = effects.length - 1; i >= 0; i--) {
            if (effects[i].type === effectType) {
                effects.splice(i, 1);
            }
        }

        if (effects.length === 0) {
            this.activeEffects.delete(cardId);
        }
    };
    /* }}} */

    /* {{{ clearEffects
     * Clear all effects for a card.
     */
    UpgradeViz.prototype.clearEffects = function(cardId) {
        this.activeEffects.delete(cardId);
    };
    /* }}} */

    /* {{{ clearAll
     * Clear all active effects.
     */
    UpgradeViz.prototype.clearAll = function() {
        this.activeEffects.clear();
        this.particles = [];
    };
    /* }}} */

    /* {{{ renderGlow
     * Render glow effect around card bounds.
     */
    UpgradeViz.prototype.renderGlow = function(ctx, bounds, config, phase) {
        var x = bounds.x;
        var y = bounds.y;
        var w = bounds.width;
        var h = bounds.height;

        /* Pulsing intensity */
        var pulse = 0.5 + Math.sin(phase * config.pulseSpeed) * 0.5;
        var radius = config.glowRadius * (0.8 + pulse * 0.4);

        ctx.save();
        ctx.globalAlpha = config.overlayOpacity * pulse;

        /* Outer glow */
        ctx.shadowColor = config.glowColor;
        ctx.shadowBlur = radius;
        ctx.strokeStyle = config.glowColor;
        ctx.lineWidth = 3;

        /* Rounded rectangle */
        var r = 8;
        ctx.beginPath();
        ctx.moveTo(x + r, y);
        ctx.lineTo(x + w - r, y);
        ctx.quadraticCurveTo(x + w, y, x + w, y + r);
        ctx.lineTo(x + w, y + h - r);
        ctx.quadraticCurveTo(x + w, y + h, x + w - r, y + h);
        ctx.lineTo(x + r, y + h);
        ctx.quadraticCurveTo(x, y + h, x, y + h - r);
        ctx.lineTo(x, y + r);
        ctx.quadraticCurveTo(x, y, x + r, y);
        ctx.closePath();
        ctx.stroke();

        ctx.restore();
    };
    /* }}} */

    /* {{{ spawnParticles
     * Spawn particles for an effect.
     */
    UpgradeViz.prototype.spawnParticles = function(bounds, config, faction) {
        var count = 1 + Math.floor(Math.random() * 2);

        for (var i = 0; i < count; i++) {
            var x = bounds.x + Math.random() * bounds.width;
            var y = bounds.y + Math.random() * bounds.height;

            this.particles.push(new Particle(x, y, config.particleType, faction));
        }
    };
    /* }}} */

    /* {{{ render
     * Render all active effects and particles to a canvas context.
     */
    UpgradeViz.prototype.render = function(ctx) {
        var self = this;
        var now = Date.now();

        /* Render effects for each card */
        this.activeEffects.forEach(function(effects, cardId) {
            for (var i = 0; i < effects.length; i++) {
                var effect = effects[i];
                var config = OVERLAYS[effect.type];

                if (!config) continue;

                /* Calculate phase for animation */
                var elapsed = (now - effect.startTime) / 1000;
                effect.phase = elapsed;

                /* Render glow */
                self.renderGlow(ctx, effect.bounds, config, effect.phase);

                /* Spawn particles occasionally */
                if (Math.random() < 0.1) {
                    self.spawnParticles(effect.bounds, config, effect.faction);
                }
            }
        });

        /* Render and update particles */
        for (var j = this.particles.length - 1; j >= 0; j--) {
            var particle = this.particles[j];

            particle.render(ctx);

            if (!particle.update()) {
                this.particles.splice(j, 1);
            }
        }
    };
    /* }}} */

    /* {{{ _startAnimation
     * Internal: Start animation loop if needed.
     */
    UpgradeViz.prototype._startAnimation = function() {
        if (this.animationFrame) return;

        var self = this;

        function animate() {
            /* Check if any effects or particles remain */
            if (self.activeEffects.size === 0 && self.particles.length === 0) {
                self.animationFrame = null;
                return;
            }

            self.animationFrame = requestAnimationFrame(animate);
        }

        this.animationFrame = requestAnimationFrame(animate);
    };
    /* }}} */

    /* {{{ getEffectsForCard
     * Get active effects for a card.
     */
    UpgradeViz.prototype.getEffectsForCard = function(cardId) {
        return this.activeEffects.get(cardId) || [];
    };
    /* }}} */

    /* {{{ hasEffects
     * Check if a card has any active effects.
     */
    UpgradeViz.prototype.hasEffects = function(cardId) {
        return this.activeEffects.has(cardId) && this.activeEffects.get(cardId).length > 0;
    };
    /* }}} */

    /* }}} */

    /* {{{ Integration helpers */

    /* {{{ applyAllyBonus
     * Apply ally bonus visual when same-faction card played.
     */
    function applyAllyBonus(card, bounds) {
        if (window.upgradeViz && card) {
            window.upgradeViz.addEffect(
                card.instance_id,
                UPGRADE_TYPE.ALLY_ACTIVE,
                bounds,
                card.faction
            );
        }
    }

    /* {{{ applyEmpowered
     * Apply empowered visual for permanent stat boost.
     */
    function applyEmpowered(card, bounds) {
        if (window.upgradeViz && card) {
            window.upgradeViz.addEffect(
                card.instance_id,
                UPGRADE_TYPE.EMPOWERED,
                bounds,
                card.faction
            );
        }
    }

    /* {{{ applyScrapped
     * Apply scrap visual when card is being removed.
     */
    function applyScrapped(card, bounds) {
        if (window.upgradeViz && card) {
            window.upgradeViz.addEffect(
                card.instance_id,
                UPGRADE_TYPE.SCRAPPED,
                bounds,
                card.faction
            );
        }
    }

    /* {{{ applyTargeted
     * Apply targeted visual when card is selected as target.
     */
    function applyTargeted(card, bounds) {
        if (window.upgradeViz && card) {
            window.upgradeViz.addEffect(
                card.instance_id,
                UPGRADE_TYPE.TARGETED,
                bounds,
                card.faction
            );
        }
    }

    /* {{{ removeTargeted
     * Remove targeted visual.
     */
    function removeTargeted(card) {
        if (window.upgradeViz && card) {
            window.upgradeViz.removeEffect(card.instance_id, UPGRADE_TYPE.TARGETED);
        }
    }

    /* {{{ applyUpgradeEffects
     * Apply appropriate visuals based on card state.
     */
    function applyUpgradeEffects(card, bounds, gameState) {
        if (!card || !window.upgradeViz) return;

        /* Check for permanent upgrades */
        if (card.attackBonus || card.tradeBonus || card.authorityBonus) {
            applyEmpowered(card, bounds);
        }

        /* Check for ally bonus (same faction in play) */
        if (gameState && gameState.inPlay) {
            var hasAlly = false;
            for (var i = 0; i < gameState.inPlay.length; i++) {
                var other = gameState.inPlay[i];
                if (other.instance_id !== card.instance_id &&
                    other.faction === card.faction) {
                    hasAlly = true;
                    break;
                }
            }
            if (hasAlly) {
                applyAllyBonus(card, bounds);
            }
        }
    }

    /* }}} */

    /* {{{ Public API */
    window.upgradeViz = new UpgradeViz();

    /* Export upgrade types */
    window.upgradeViz.UPGRADE_TYPE = UPGRADE_TYPE;

    /* Export integration helpers */
    window.upgradeViz.applyAllyBonus = applyAllyBonus;
    window.upgradeViz.applyEmpowered = applyEmpowered;
    window.upgradeViz.applyScrapped = applyScrapped;
    window.upgradeViz.applyTargeted = applyTargeted;
    window.upgradeViz.removeTargeted = removeTargeted;
    window.upgradeViz.applyUpgradeEffects = applyUpgradeEffects;
    /* }}} */

})();
