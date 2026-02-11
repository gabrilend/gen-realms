/**
 * style-transfer.js - Style Transfer Prompts for Symbeline Realms
 *
 * Comprehensive style guide system for maintaining consistent fantasy
 * aesthetic across all generated images. Handles faction-specific visual
 * language, color palettes, reference images, and multi-faction blending.
 */

(function() {
    'use strict';

    /* {{{ Faction complete style definitions */
    var FACTION_STYLES = {
        merchant: {
            name: 'Trade Federation',
            keywords: 'golden accents, wealthy aesthetic, trade ships, coins, silk banners, opulent',
            negative: 'poor, rusted, broken, primitive, savage',
            colorPalette: ['#FFD700', '#4B0082', '#FFFFFF', '#1E3A5F'],
            colorNames: ['gold', 'deep purple', 'white', 'navy blue'],
            visualMotifs: ['ships', 'coins', 'banners', 'scales', 'treasure'],
            environmentHints: 'bustling port, trading dock, merchant vessel, treasury',
            lightingStyle: 'warm golden hour, lamplight, reflective surfaces',
            referenceImage: 'assets/styles/merchant_ref.png'
        },
        wilds: {
            name: 'The Wilds',
            keywords: 'forest environment, primal energy, beasts, nature, untamed, savage',
            negative: 'mechanical, urban, civilized, clean, artificial',
            colorPalette: ['#228B22', '#8B4513', '#FFD700', '#8B0000'],
            colorNames: ['forest green', 'earth brown', 'amber', 'blood red'],
            visualMotifs: ['beasts', 'forests', 'claws', 'fangs', 'vines'],
            environmentHints: 'ancient forest, wilderness, overgrown ruins, den',
            lightingStyle: 'dappled sunlight, mist, twilight, moonlight',
            referenceImage: 'assets/styles/wilds_ref.png'
        },
        kingdom: {
            name: 'The Kingdom',
            keywords: 'knights, castles, heraldry, armor, noble, chivalric',
            negative: 'barbaric, monstrous, chaotic, wild, savage',
            colorPalette: ['#C0C0C0', '#4169E1', '#DC143C', '#FFFFFF'],
            colorNames: ['silver', 'royal blue', 'crimson', 'white'],
            visualMotifs: ['knights', 'castles', 'swords', 'shields', 'banners'],
            environmentHints: 'castle walls, throne room, battlefield, tournament',
            lightingStyle: 'bright daylight, torchlit halls, dawn light',
            referenceImage: 'assets/styles/kingdom_ref.png'
        },
        artificer: {
            name: 'The Artificers',
            keywords: 'constructs, gears, enchanted, arcane machinery, magical tech',
            negative: 'organic, natural, primitive, wild, chaotic',
            colorPalette: ['#CD7F32', '#B87333', '#4169E1', '#9932CC'],
            colorNames: ['bronze', 'copper', 'blue energy', 'arcane purple'],
            visualMotifs: ['gears', 'constructs', 'runes', 'crystals', 'machines'],
            environmentHints: 'workshop, foundry, arcane laboratory, clocktower',
            lightingStyle: 'magical glow, furnace light, crystalline radiance',
            referenceImage: 'assets/styles/artificer_ref.png'
        },
        neutral: {
            name: 'Unaligned',
            keywords: 'mystical symbols, ethereal glow, ancient magic, arcane patterns',
            negative: 'faction-specific, allegiance, partisan',
            colorPalette: ['#808080', '#C0C0C0', '#4B0082', '#FFFFFF'],
            colorNames: ['grey', 'silver', 'indigo', 'white'],
            visualMotifs: ['symbols', 'portals', 'stars', 'mist', 'runes'],
            environmentHints: 'void, astral plane, ancient temple, nexus',
            lightingStyle: 'ethereal glow, starlight, ambient magic',
            referenceImage: 'assets/styles/neutral_ref.png'
        }
    };
    /* }}} */

    /* {{{ Base style configuration */
    var BASE_STYLE = {
        positive: 'fantasy art, medieval fantasy, magical, painterly, ' +
                  'detailed illustration, card game art, high quality, ' +
                  'masterpiece, best quality',
        negative: 'science fiction, modern, photograph, realistic photo, ' +
                  'blurry, low quality, text, watermark, signature, ' +
                  'deformed, ugly, bad anatomy, amateur'
    };
    /* }}} */

    /* {{{ Art style presets */
    var ART_STYLE_PRESETS = {
        painterly: {
            name: 'Painterly',
            keywords: 'oil painting style, traditional fantasy art, visible brush strokes, rich colors, classical technique',
            cfgScale: 7.5,
            steps: 30
        },
        detailed: {
            name: 'Highly Detailed',
            keywords: 'highly detailed, intricate rendering, sharp focus, realistic lighting, fine detail',
            cfgScale: 8.0,
            steps: 35
        },
        stylized: {
            name: 'Stylized',
            keywords: 'bold colors, graphic novel aesthetic, cel shaded, dynamic composition, vibrant',
            cfgScale: 7.0,
            steps: 28
        },
        icon: {
            name: 'Iconic',
            keywords: 'simplified iconic style, clean lines, symbolic representation, flat colors, bold silhouette',
            cfgScale: 6.5,
            steps: 25
        },
        cinematic: {
            name: 'Cinematic',
            keywords: 'cinematic composition, dramatic lighting, epic scale, movie poster quality',
            cfgScale: 8.5,
            steps: 35
        }
    };
    /* }}} */

    /* {{{ StyleTransfer class */
    function StyleTransfer() {
        this.referenceImages = {};
        this.loadedStyles = {};
    }

    /* {{{ getFactionStyle
     * Get complete style definition for a faction.
     */
    StyleTransfer.prototype.getFactionStyle = function(faction) {
        return FACTION_STYLES[faction] || FACTION_STYLES.neutral;
    };
    /* }}} */

    /* {{{ getColorPalette
     * Get color palette for a faction as hex values.
     */
    StyleTransfer.prototype.getColorPalette = function(faction) {
        var style = this.getFactionStyle(faction);
        return style.colorPalette || [];
    };
    /* }}} */

    /* {{{ getColorDescription
     * Get color palette as descriptive text for prompts.
     */
    StyleTransfer.prototype.getColorDescription = function(faction) {
        var style = this.getFactionStyle(faction);
        if (!style.colorNames) return '';
        return style.colorNames.join(', ') + ' color palette';
    };
    /* }}} */

    /* {{{ blendFactionStyles
     * Blend styles from multiple factions for multi-faction cards.
     * @param factions - Array of faction names
     * @param weights - Optional array of weights (defaults to equal)
     */
    StyleTransfer.prototype.blendFactionStyles = function(factions, weights) {
        if (!factions || factions.length === 0) {
            return this.getFactionStyle('neutral');
        }

        if (factions.length === 1) {
            return this.getFactionStyle(factions[0]);
        }

        /* Default to equal weights */
        if (!weights) {
            weights = factions.map(function() {
                return 1 / factions.length;
            });
        }

        /* Normalize weights */
        var totalWeight = weights.reduce(function(a, b) { return a + b; }, 0);
        weights = weights.map(function(w) { return w / totalWeight; });

        var blended = {
            name: 'Blended',
            keywords: [],
            negative: [],
            colorPalette: [],
            colorNames: [],
            visualMotifs: [],
            environmentHints: [],
            lightingStyle: []
        };

        /* Blend each property */
        for (var i = 0; i < factions.length; i++) {
            var style = this.getFactionStyle(factions[i]);
            var weight = weights[i];

            /* Higher weighted factions contribute more keywords */
            if (weight >= 0.3) {
                blended.keywords.push(style.keywords);
                blended.environmentHints.push(style.environmentHints);
            }

            /* Take visual motifs proportionally */
            var motifCount = Math.ceil(style.visualMotifs.length * weight);
            for (var j = 0; j < motifCount && j < style.visualMotifs.length; j++) {
                blended.visualMotifs.push(style.visualMotifs[j]);
            }

            /* Blend colors proportionally */
            var colorCount = Math.ceil(style.colorPalette.length * weight);
            for (var k = 0; k < colorCount && k < style.colorPalette.length; k++) {
                blended.colorPalette.push(style.colorPalette[k]);
                blended.colorNames.push(style.colorNames[k]);
            }

            /* Add lighting from dominant faction */
            if (weight === Math.max.apply(null, weights)) {
                blended.lightingStyle = style.lightingStyle;
            }

            /* Merge negatives */
            blended.negative.push(style.negative);
        }

        /* Convert arrays to strings */
        blended.keywords = blended.keywords.join(', ');
        blended.negative = this._mergeNegatives(blended.negative);
        blended.visualMotifs = this._uniqueArray(blended.visualMotifs);
        blended.environmentHints = blended.environmentHints.join(' or ');

        /* Remove duplicate colors */
        blended.colorPalette = this._uniqueArray(blended.colorPalette);
        blended.colorNames = this._uniqueArray(blended.colorNames);

        return blended;
    };
    /* }}} */

    /* {{{ _mergeNegatives
     * Internal: Merge negative prompts, removing duplicates.
     */
    StyleTransfer.prototype._mergeNegatives = function(negatives) {
        var allTerms = [];

        for (var i = 0; i < negatives.length; i++) {
            var terms = negatives[i].split(',').map(function(t) {
                return t.trim();
            });
            allTerms = allTerms.concat(terms);
        }

        return this._uniqueArray(allTerms).join(', ');
    };
    /* }}} */

    /* {{{ _uniqueArray
     * Internal: Remove duplicates from array.
     */
    StyleTransfer.prototype._uniqueArray = function(arr) {
        var seen = {};
        return arr.filter(function(item) {
            if (seen[item]) return false;
            seen[item] = true;
            return true;
        });
    };
    /* }}} */

    /* {{{ buildStylePrompt
     * Build complete style prompt for a card.
     * @param card - Card data object
     * @param options - Generation options
     */
    StyleTransfer.prototype.buildStylePrompt = function(card, options) {
        options = options || {};

        /* Determine factions */
        var factions = [];
        if (card.faction) {
            factions.push(card.faction);
        }
        if (card.secondaryFaction) {
            factions.push(card.secondaryFaction);
        }

        /* Get or blend faction style */
        var factionStyle = factions.length > 1
            ? this.blendFactionStyles(factions)
            : this.getFactionStyle(factions[0] || 'neutral');

        /* Get art style preset */
        var artStyle = ART_STYLE_PRESETS[options.artStyle] ||
                       ART_STYLE_PRESETS.painterly;

        /* Build positive prompt */
        var positive = [];

        /* Quality prefix */
        positive.push(BASE_STYLE.positive);

        /* Card subject */
        if (card.name) {
            positive.push(card.name);
        }

        /* Faction style */
        positive.push(factionStyle.keywords);

        /* Color description */
        if (factionStyle.colorNames) {
            positive.push(factionStyle.colorNames.slice(0, 2).join(' and ') + ' tones');
        }

        /* Environment hints if relevant */
        if (card.kind === 'base' && factionStyle.environmentHints) {
            positive.push(factionStyle.environmentHints.split(' or ')[0]);
        }

        /* Lighting */
        if (factionStyle.lightingStyle) {
            positive.push(factionStyle.lightingStyle.split(',')[0]);
        }

        /* Art style */
        positive.push(artStyle.keywords);

        /* Build negative prompt */
        var negative = [];
        negative.push(BASE_STYLE.negative);
        negative.push(factionStyle.negative);

        return {
            positive: positive.join(', '),
            negative: negative.join(', '),
            cfgScale: artStyle.cfgScale,
            steps: artStyle.steps
        };
    };
    /* }}} */

    /* {{{ validateStyleConsistency
     * Validate that a prompt maintains style consistency.
     * Returns warnings for potential issues.
     */
    StyleTransfer.prototype.validateStyleConsistency = function(prompt, expectedFaction) {
        var warnings = [];
        var factionStyle = this.getFactionStyle(expectedFaction);

        /* Check for conflicting faction terms */
        for (var faction in FACTION_STYLES) {
            if (faction === expectedFaction || faction === 'neutral') continue;

            var otherStyle = FACTION_STYLES[faction];
            var otherKeywords = otherStyle.keywords.split(',');

            for (var i = 0; i < otherKeywords.length; i++) {
                var keyword = otherKeywords[i].trim().toLowerCase();
                if (prompt.positive.toLowerCase().indexOf(keyword) !== -1) {
                    warnings.push({
                        type: 'faction_conflict',
                        message: 'Keyword "' + keyword + '" from ' + faction +
                                 ' found in ' + expectedFaction + ' card prompt'
                    });
                }
            }
        }

        /* Check for base style presence */
        var baseKeywords = ['fantasy', 'medieval', 'card game art'];
        var hasBase = false;
        for (var j = 0; j < baseKeywords.length; j++) {
            if (prompt.positive.toLowerCase().indexOf(baseKeywords[j]) !== -1) {
                hasBase = true;
                break;
            }
        }

        if (!hasBase) {
            warnings.push({
                type: 'missing_base_style',
                message: 'Prompt missing base fantasy style keywords'
            });
        }

        /* Check for faction identity */
        var factionKeywords = factionStyle.keywords.split(',');
        var hasFactionIdentity = false;
        for (var k = 0; k < factionKeywords.length; k++) {
            var fkeyword = factionKeywords[k].trim().toLowerCase();
            if (prompt.positive.toLowerCase().indexOf(fkeyword) !== -1) {
                hasFactionIdentity = true;
                break;
            }
        }

        if (!hasFactionIdentity) {
            warnings.push({
                type: 'missing_faction_identity',
                message: 'Prompt missing ' + expectedFaction + ' faction keywords'
            });
        }

        return {
            isValid: warnings.length === 0,
            warnings: warnings
        };
    };
    /* }}} */

    /* {{{ loadReferenceImage
     * Load a faction reference image for style reference.
     * Returns Promise.
     */
    StyleTransfer.prototype.loadReferenceImage = function(faction) {
        var self = this;
        var style = this.getFactionStyle(faction);

        if (!style.referenceImage) {
            return Promise.reject(new Error('No reference image for faction: ' + faction));
        }

        if (this.referenceImages[faction]) {
            return Promise.resolve(this.referenceImages[faction]);
        }

        return new Promise(function(resolve, reject) {
            var img = new Image();
            img.crossOrigin = 'anonymous';

            img.onload = function() {
                self.referenceImages[faction] = img;
                resolve(img);
            };

            img.onerror = function() {
                reject(new Error('Failed to load reference image: ' + style.referenceImage));
            };

            img.src = style.referenceImage;
        });
    };
    /* }}} */

    /* {{{ getReferenceImageBase64
     * Get reference image as base64 for API calls.
     */
    StyleTransfer.prototype.getReferenceImageBase64 = function(faction) {
        var img = this.referenceImages[faction];
        if (!img) return null;

        var canvas = document.createElement('canvas');
        canvas.width = img.width;
        canvas.height = img.height;

        var ctx = canvas.getContext('2d');
        ctx.drawImage(img, 0, 0);

        return canvas.toDataURL('image/png');
    };
    /* }}} */

    /* {{{ getArtStylePreset
     * Get art style preset by name.
     */
    StyleTransfer.prototype.getArtStylePreset = function(styleName) {
        return ART_STYLE_PRESETS[styleName] || ART_STYLE_PRESETS.painterly;
    };
    /* }}} */

    /* {{{ getAllArtStyles
     * Get list of all available art styles.
     */
    StyleTransfer.prototype.getAllArtStyles = function() {
        var styles = [];
        for (var key in ART_STYLE_PRESETS) {
            styles.push({
                id: key,
                name: ART_STYLE_PRESETS[key].name,
                keywords: ART_STYLE_PRESETS[key].keywords
            });
        }
        return styles;
    };
    /* }}} */

    /* {{{ getAllFactions
     * Get list of all factions with their color swatches.
     */
    StyleTransfer.prototype.getAllFactions = function() {
        var factions = [];
        for (var key in FACTION_STYLES) {
            factions.push({
                id: key,
                name: FACTION_STYLES[key].name,
                colorPalette: FACTION_STYLES[key].colorPalette,
                colorNames: FACTION_STYLES[key].colorNames
            });
        }
        return factions;
    };
    /* }}} */

    /* {{{ generateColorCSS
     * Generate CSS custom properties for faction colors.
     */
    StyleTransfer.prototype.generateColorCSS = function() {
        var css = ':root {\n';

        for (var faction in FACTION_STYLES) {
            var style = FACTION_STYLES[faction];
            var palette = style.colorPalette;

            css += '  /* ' + style.name + ' */\n';
            css += '  --faction-' + faction + '-primary: ' + palette[0] + ';\n';
            css += '  --faction-' + faction + '-secondary: ' + palette[1] + ';\n';
            css += '  --faction-' + faction + '-accent: ' + palette[2] + ';\n';
            css += '  --faction-' + faction + '-highlight: ' + palette[3] + ';\n';
        }

        css += '}\n';
        return css;
    };
    /* }}} */

    /* {{{ getStats
     * Get style transfer statistics.
     */
    StyleTransfer.prototype.getStats = function() {
        return {
            factionCount: Object.keys(FACTION_STYLES).length,
            artStyleCount: Object.keys(ART_STYLE_PRESETS).length,
            loadedReferences: Object.keys(this.referenceImages).length
        };
    };
    /* }}} */

    /* }}} */

    /* {{{ Public API */
    window.StyleTransfer = StyleTransfer;

    /* Export constants */
    window.StyleTransfer.FACTION_STYLES = FACTION_STYLES;
    window.StyleTransfer.BASE_STYLE = BASE_STYLE;
    window.StyleTransfer.ART_STYLE_PRESETS = ART_STYLE_PRESETS;

    /* Create default instance */
    window.styleTransfer = new StyleTransfer();
    /* }}} */

})();
