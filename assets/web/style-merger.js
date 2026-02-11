/**
 * style-merger.js - Style Guide Integration for Symbeline Realms
 *
 * Merges user style preferences from localStorage with card-specific
 * prompts to create complete generation requests. Handles faction styles,
 * upgrade visuals, and art style variants.
 */

(function() {
    'use strict';

    /* {{{ Faction style templates */
    var FACTION_STYLES = {
        merchant: 'golden accents, wealthy aesthetic, trading posts, coins, silk banners',
        wilds: 'forest environment, primal energy, natural elements, beasts, green tones',
        kingdom: 'noble bearing, castle architecture, knights, blue and silver, heraldry',
        artificer: 'mechanical elements, purple arcane energy, constructs, clockwork, crystals',
        neutral: 'mystical symbols, ethereal glow, ancient magic, arcane patterns'
    };
    /* }}} */

    /* {{{ Art style keywords */
    var ART_STYLES = {
        painterly: 'oil painting style, traditional fantasy art, visible brush strokes, rich colors',
        detailed: 'highly detailed, intricate rendering, sharp focus, realistic lighting',
        stylized: 'bold colors, graphic novel aesthetic, cel shaded, dynamic composition',
        icon: 'simplified iconic style, clean lines, symbolic representation, flat colors'
    };
    /* }}} */

    /* {{{ Upgrade visual modifiers */
    var UPGRADE_MODIFIERS = {
        attack: 'glowing weapon, fiery aura, enhanced power, battle-ready',
        trade: 'golden glow, prosperous appearance, jeweled ornaments, wealth symbols',
        authority: 'divine light, blessed aura, radiant halo, holy presence'
    };
    /* }}} */

    /* {{{ Card type keywords */
    var CARD_TYPE_KEYWORDS = {
        ship: 'character portrait, dynamic pose, heroic stance',
        base: 'architectural view, wide establishing shot, location artwork',
        unit: 'creature portrait, action pose, summoned being'
    };
    /* }}} */

    /* {{{ StyleMerger class */
    function StyleMerger() {
        this.cachedPrefs = null;
        this.lastPrefLoad = 0;
        this.prefCacheDuration = 5000; /* 5 seconds cache */
    }

    /* {{{ getPreferences
     * Get user preferences with caching.
     */
    StyleMerger.prototype.getPreferences = function() {
        var now = Date.now();

        /* Use cached prefs if fresh */
        if (this.cachedPrefs && (now - this.lastPrefLoad) < this.prefCacheDuration) {
            return this.cachedPrefs;
        }

        /* Load fresh prefs */
        if (window.preferences) {
            this.cachedPrefs = window.preferences.load();
        } else {
            /* Fallback defaults */
            this.cachedPrefs = {
                styleGuide: 'dark fantasy, oil painting, dramatic lighting',
                negativePrompts: 'cartoon, anime, bright colors, modern, photorealistic',
                artStyle: 'painterly'
            };
        }

        this.lastPrefLoad = now;
        return this.cachedPrefs;
    };
    /* }}} */

    /* {{{ clearPrefCache
     * Clear preference cache to force reload.
     */
    StyleMerger.prototype.clearPrefCache = function() {
        this.cachedPrefs = null;
        this.lastPrefLoad = 0;
    };
    /* }}} */

    /* {{{ getFactionStyle
     * Get style keywords for a faction.
     */
    StyleMerger.prototype.getFactionStyle = function(faction) {
        return FACTION_STYLES[faction] || FACTION_STYLES.neutral;
    };
    /* }}} */

    /* {{{ getArtStyle
     * Get keywords for an art style.
     */
    StyleMerger.prototype.getArtStyle = function(style) {
        return ART_STYLES[style] || ART_STYLES.painterly;
    };
    /* }}} */

    /* {{{ getCardTypeKeywords
     * Get keywords for a card type.
     */
    StyleMerger.prototype.getCardTypeKeywords = function(cardKind) {
        return CARD_TYPE_KEYWORDS[cardKind] || CARD_TYPE_KEYWORDS.ship;
    };
    /* }}} */

    /* {{{ getUpgradeModifiers
     * Get visual modifiers for card upgrades.
     */
    StyleMerger.prototype.getUpgradeModifiers = function(card) {
        var modifiers = [];

        if (card.attackBonus && card.attackBonus > 0) {
            modifiers.push(UPGRADE_MODIFIERS.attack);
        }
        if (card.tradeBonus && card.tradeBonus > 0) {
            modifiers.push(UPGRADE_MODIFIERS.trade);
        }
        if (card.authorityBonus && card.authorityBonus > 0) {
            modifiers.push(UPGRADE_MODIFIERS.authority);
        }

        return modifiers.join(', ');
    };
    /* }}} */

    /* {{{ buildCardPrompt
     * Build base prompt for a card (before user style merge).
     */
    StyleMerger.prototype.buildCardPrompt = function(card) {
        var parts = [];

        /* Card name as subject */
        if (card.name) {
            parts.push(card.name);
        }

        /* Card type keywords */
        if (card.kind) {
            parts.push(this.getCardTypeKeywords(card.kind));
        }

        /* Faction style */
        if (card.faction) {
            parts.push(this.getFactionStyle(card.faction));
        }

        /* Flavor text (if short) */
        if (card.flavor && card.flavor.length < 80) {
            parts.push(card.flavor);
        }

        /* Base description */
        parts.push('fantasy card art, detailed illustration');

        return {
            positive: parts.join(', '),
            negative: 'blurry, low quality, watermark, signature, text, deformed'
        };
    };
    /* }}} */

    /* {{{ mergeWithUserStyle
     * Merge card prompt with user style preferences.
     */
    StyleMerger.prototype.mergeWithUserStyle = function(cardPrompt, prefs) {
        prefs = prefs || this.getPreferences();

        var positive = cardPrompt.positive;
        var negative = cardPrompt.negative;

        /* Add user style guide */
        if (prefs.styleGuide) {
            positive += ', ' + prefs.styleGuide;
        }

        /* Add art style */
        if (prefs.artStyle) {
            positive += ', ' + this.getArtStyle(prefs.artStyle);
        }

        /* Add user negative prompts */
        if (prefs.negativePrompts) {
            negative += ', ' + prefs.negativePrompts;
        }

        return {
            positive: positive,
            negative: negative
        };
    };
    /* }}} */

    /* {{{ buildCompletePrompt
     * Build complete generation prompt for a card.
     * Combines card data, faction style, upgrades, and user preferences.
     */
    StyleMerger.prototype.buildCompletePrompt = function(card, options) {
        options = options || {};

        var prefs = options.preferences || this.getPreferences();

        /* Start with card base prompt */
        var prompt = this.buildCardPrompt(card);

        /* Add upgrade visuals if applicable */
        var upgrades = this.getUpgradeModifiers(card);
        if (upgrades) {
            prompt.positive += ', ' + upgrades;
        }

        /* Merge with user style */
        prompt = this.mergeWithUserStyle(prompt, prefs);

        /* Add quality tags */
        if (options.includeQuality !== false) {
            prompt.positive = 'masterpiece, best quality, ' + prompt.positive;
        }

        return {
            positive: prompt.positive,
            negative: prompt.negative,
            width: options.width || 512,
            height: options.height || 768,
            steps: options.steps || 30,
            cfg_scale: options.cfg_scale || 7.5,
            seed: card.image_seed || 0
        };
    };
    /* }}} */

    /* {{{ buildBatchPrompts
     * Build prompts for multiple cards efficiently.
     */
    StyleMerger.prototype.buildBatchPrompts = function(cards, options) {
        var self = this;
        options = options || {};

        /* Load prefs once for batch */
        var prefs = options.preferences || this.getPreferences();
        options.preferences = prefs;

        return cards.map(function(card) {
            return {
                card: card,
                prompt: self.buildCompletePrompt(card, options)
            };
        });
    };
    /* }}} */

    /* {{{ generatePromptHash
     * Generate a hash of prompt parameters for cache keying.
     */
    StyleMerger.prototype.generatePromptHash = function(prompt) {
        /* Simple string hash */
        var str = prompt.positive + '|' + prompt.negative + '|' + prompt.seed;
        var hash = 0;

        for (var i = 0; i < str.length; i++) {
            var char = str.charCodeAt(i);
            hash = ((hash << 5) - hash) + char;
            hash = hash & hash; /* Convert to 32bit integer */
        }

        return hash.toString(16);
    };
    /* }}} */

    /* }}} */

    /* {{{ Integration with art regeneration system */

    /* {{{ requestCardRegeneration
     * High-level function to request card art regeneration.
     * Builds prompt and adds to generation queue.
     */
    function requestCardRegeneration(card, gameState, options) {
        if (!card || !card.instance_id) {
            console.warn('StyleMerger: Cannot regenerate invalid card');
            return false;
        }

        options = options || {};

        /* Build complete prompt */
        var prompt = window.styleMerger.buildCompletePrompt(card, options);

        /* Determine priority from game state */
        var priority = window.generationQueue
            ? window.generationQueue.getPriorityForCard(card, gameState)
            : 10;

        /* Mark in tracker */
        if (window.artTracker) {
            window.artTracker.markForRegeneration(card);
        }

        /* Add to queue */
        if (window.generationQueue) {
            window.generationQueue.enqueue(card, prompt, priority);
        }

        return true;
    }

    /* {{{ processVisibleCards
     * Process all visible cards that need regeneration.
     */
    function processVisibleCards(gameState) {
        if (!gameState || !window.artTracker) return;

        var visibleCards = [];

        /* Collect visible cards */
        if (gameState.hand) visibleCards = visibleCards.concat(gameState.hand);
        if (gameState.tradeRow) visibleCards = visibleCards.concat(gameState.tradeRow);
        if (gameState.inPlay) visibleCards = visibleCards.concat(gameState.inPlay);

        /* Check each card */
        for (var i = 0; i < visibleCards.length; i++) {
            var card = visibleCards[i];

            if (window.artTracker.needsRegeneration(card)) {
                requestCardRegeneration(card, gameState);
            }
        }
    }

    /* }}} */

    /* {{{ Public API */
    window.styleMerger = new StyleMerger();

    /* Export integration functions */
    window.styleMerger.requestCardRegeneration = requestCardRegeneration;
    window.styleMerger.processVisibleCards = processVisibleCards;

    /* Export constants for reference */
    window.styleMerger.FACTION_STYLES = FACTION_STYLES;
    window.styleMerger.ART_STYLES = ART_STYLES;
    window.styleMerger.UPGRADE_MODIFIERS = UPGRADE_MODIFIERS;
    /* }}} */

})();
