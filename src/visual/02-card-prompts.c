/*
 * 02-card-prompts.c - Card Image Prompt Builder Implementation
 *
 * Builds ComfyUI-compatible prompts for card art. Each faction has a
 * distinct visual style, and cards are rendered with appropriate themes
 * based on their type (creature, base, action).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "02-card-prompts.h"

// {{{ Constants
#define MAX_PROMPT_LENGTH 2048
#define DEFAULT_WIDTH 512
#define DEFAULT_HEIGHT 768
#define DEFAULT_STEPS 30
#define DEFAULT_CFG_SCALE 7.5f
// }}}

// {{{ Faction Style Templates
// Visual themes for each faction. These keywords define the aesthetic
// that ComfyUI will use when generating card art.
static const char* FACTION_STYLES[] = {
    [FACTION_NEUTRAL] = "mystical, arcane symbols, ethereal glow, ancient magic",
    [FACTION_MERCHANT] = "golden banners, coins, trade ships, wealthy, opulent, silk robes, marketplace",
    [FACTION_WILDS] = "forest creatures, primal, nature, beasts, untamed, tribal, feral, moss and vines",
    [FACTION_KINGDOM] = "knights, castles, heraldry, armor, noble, royal banners, stone fortifications",
    [FACTION_ARTIFICER] = "constructs, gears, enchanted machines, arcane tech, crystal energy, clockwork"
};
// }}}

// {{{ Art Style Keywords
// Style modifiers that affect the overall visual aesthetic.
static const char* STYLE_KEYWORDS[] = {
    [STYLE_PAINTERLY] = "oil painting, traditional fantasy art, brush strokes, rich colors, classical",
    [STYLE_DETAILED] = "highly detailed, realistic, intricate, photorealistic lighting, sharp focus",
    [STYLE_STYLIZED] = "bold colors, graphic novel, cel shaded, dynamic composition, vibrant",
    [STYLE_ICON] = "simplified, iconic, clean lines, symbolic, flat colors, logo style"
};
// }}}

// {{{ Art Style Names
static const char* STYLE_NAMES[] = {
    [STYLE_PAINTERLY] = "painterly",
    [STYLE_DETAILED] = "detailed",
    [STYLE_STYLIZED] = "stylized",
    [STYLE_ICON] = "icon"
};
// }}}

// {{{ Negative Prompt Base
// Elements to avoid in all card art. Prevents common generation artifacts
// and maintains consistent fantasy aesthetic.
static const char* NEGATIVE_PROMPT_BASE =
    "blurry, low quality, watermark, signature, text, logo, "
    "modern elements, photography, 3d render, anime, cartoon, "
    "deformed, ugly, duplicate, morbid, mutilated, poorly drawn, "
    "extra limbs, bad anatomy, bad proportions, disfigured, "
    "out of frame, cropped, worst quality, jpeg artifacts";
// }}}

// {{{ Quality Tags
// Tags added when include_quality_tags is enabled in config.
static const char* QUALITY_TAGS =
    "masterpiece, best quality, highly detailed, professional";
// }}}

// {{{ Card Type Keywords
// Additional keywords based on card kind for appropriate framing.
static const char* creature_keywords = "character portrait, dynamic pose, heroic stance";
static const char* base_keywords = "architectural, wide shot, establishing view, location";
static const char* action_keywords = "magical energy, dynamic effects, spell casting, motion blur";
// }}}

// {{{ Helper: safe_strdup
static char* safe_strdup(const char* str) {
    if (str == NULL) {
        return NULL;
    }
    return strdup(str);
}
// }}}

// {{{ Helper: build_positive_prompt
// Assembles the positive prompt from components.
static char* build_positive_prompt(const char* name, const char* faction_style,
                                    const char* card_type_keywords,
                                    const char* style_keywords,
                                    const char* flavor,
                                    bool include_quality) {
    char buffer[MAX_PROMPT_LENGTH];
    int offset = 0;

    // Start with quality tags if enabled
    if (include_quality) {
        offset += snprintf(buffer + offset, MAX_PROMPT_LENGTH - offset,
                           "%s, ", QUALITY_TAGS);
    }

    // Add card name as the subject
    offset += snprintf(buffer + offset, MAX_PROMPT_LENGTH - offset,
                       "%s, ", name ? name : "fantasy card");

    // Add faction style
    offset += snprintf(buffer + offset, MAX_PROMPT_LENGTH - offset,
                       "%s, ", faction_style ? faction_style : "fantasy");

    // Add card type keywords
    offset += snprintf(buffer + offset, MAX_PROMPT_LENGTH - offset,
                       "%s, ", card_type_keywords ? card_type_keywords : "");

    // Add flavor text elements if present and short enough
    if (flavor != NULL && strlen(flavor) < 100) {
        offset += snprintf(buffer + offset, MAX_PROMPT_LENGTH - offset,
                           "%s, ", flavor);
    }

    // Add art style
    offset += snprintf(buffer + offset, MAX_PROMPT_LENGTH - offset,
                       "%s, ", style_keywords ? style_keywords : "fantasy art");

    // Common fantasy card art footer
    offset += snprintf(buffer + offset, MAX_PROMPT_LENGTH - offset,
                       "fantasy card art, detailed illustration");

    return strdup(buffer);
}
// }}}

// {{{ Helper: create_prompt_struct
// Creates and initializes an ImagePrompt with default values.
static ImagePrompt* create_prompt_struct(const ImagePromptConfig* config) {
    ImagePrompt* prompt = malloc(sizeof(ImagePrompt));
    if (prompt == NULL) {
        return NULL;
    }

    prompt->positive = NULL;
    prompt->negative = NULL;
    prompt->width = config ? config->default_width : DEFAULT_WIDTH;
    prompt->height = config ? config->default_height : DEFAULT_HEIGHT;
    prompt->steps = config ? config->default_steps : DEFAULT_STEPS;
    prompt->cfg_scale = config ? config->default_cfg_scale : DEFAULT_CFG_SCALE;
    prompt->seed = 0;  // 0 means random

    return prompt;
}
// }}}

// {{{ Helper: add_upgrade_modifiers
// Adds visual modifiers for upgraded cards.
static void add_upgrade_modifiers(char* buffer, size_t size,
                                   int attack_bonus, int trade_bonus,
                                   int authority_bonus) {
    size_t offset = strlen(buffer);

    if (attack_bonus > 0) {
        offset += snprintf(buffer + offset, size - offset,
                           ", glowing weapon, enhanced power, fiery aura");
    }
    if (trade_bonus > 0) {
        offset += snprintf(buffer + offset, size - offset,
                           ", golden glow, prosperous, jeweled ornaments");
    }
    if (authority_bonus > 0) {
        offset += snprintf(buffer + offset, size - offset,
                           ", divine light, blessed, radiant halo");
    }
}
// }}}

// {{{ prompt_config_default
ImagePromptConfig prompt_config_default(void) {
    ImagePromptConfig config = {
        .default_width = DEFAULT_WIDTH,
        .default_height = DEFAULT_HEIGHT,
        .default_steps = DEFAULT_STEPS,
        .default_cfg_scale = DEFAULT_CFG_SCALE,
        .include_quality_tags = true,
        .include_artist_tags = false
    };
    return config;
}
// }}}

// {{{ prompt_free
void prompt_free(ImagePrompt* prompt) {
    if (prompt == NULL) {
        return;
    }
    free(prompt->positive);
    free(prompt->negative);
    free(prompt);
}
// }}}

// {{{ prompt_get_faction_style
const char* prompt_get_faction_style(Faction faction) {
    if (faction < 0 || faction >= FACTION_COUNT) {
        return FACTION_STYLES[FACTION_NEUTRAL];
    }
    return FACTION_STYLES[faction];
}
// }}}

// {{{ prompt_get_art_style_keywords
const char* prompt_get_art_style_keywords(ArtStyle style) {
    if (style < 0 || style >= STYLE_COUNT) {
        return STYLE_KEYWORDS[STYLE_PAINTERLY];
    }
    return STYLE_KEYWORDS[style];
}
// }}}

// {{{ prompt_get_negative_base
const char* prompt_get_negative_base(void) {
    return NEGATIVE_PROMPT_BASE;
}
// }}}

// {{{ art_style_to_string
const char* art_style_to_string(ArtStyle style) {
    if (style < 0 || style >= STYLE_COUNT) {
        return "unknown";
    }
    return STYLE_NAMES[style];
}
// }}}

// {{{ art_style_from_string
ArtStyle art_style_from_string(const char* str) {
    if (str == NULL) {
        return STYLE_PAINTERLY;
    }

    for (int i = 0; i < STYLE_COUNT; i++) {
        if (strcasecmp(str, STYLE_NAMES[i]) == 0) {
            return (ArtStyle)i;
        }
    }

    return STYLE_PAINTERLY;
}
// }}}

// {{{ prompt_build_creature
ImagePrompt* prompt_build_creature(CardType* card, ArtStyle style,
                                    const ImagePromptConfig* config) {
    if (card == NULL) {
        return NULL;
    }

    ImagePromptConfig cfg = config ? *config : prompt_config_default();
    ImagePrompt* prompt = create_prompt_struct(&cfg);
    if (prompt == NULL) {
        return NULL;
    }

    const char* faction_style = prompt_get_faction_style(card->faction);
    const char* style_keywords = prompt_get_art_style_keywords(style);

    prompt->positive = build_positive_prompt(
        card->name,
        faction_style,
        creature_keywords,
        style_keywords,
        card->flavor,
        cfg.include_quality_tags
    );

    prompt->negative = safe_strdup(NEGATIVE_PROMPT_BASE);

    if (prompt->positive == NULL || prompt->negative == NULL) {
        prompt_free(prompt);
        return NULL;
    }

    return prompt;
}
// }}}

// {{{ prompt_build_base
ImagePrompt* prompt_build_base(CardType* card, ArtStyle style,
                                const ImagePromptConfig* config) {
    if (card == NULL) {
        return NULL;
    }

    ImagePromptConfig cfg = config ? *config : prompt_config_default();

    // Bases use landscape orientation
    int temp = cfg.default_width;
    cfg.default_width = cfg.default_height;
    cfg.default_height = temp;

    ImagePrompt* prompt = create_prompt_struct(&cfg);
    if (prompt == NULL) {
        return NULL;
    }

    const char* faction_style = prompt_get_faction_style(card->faction);
    const char* style_keywords = prompt_get_art_style_keywords(style);

    // Add defense indicator for bases
    char base_extras[256];
    if (card->defense > 0) {
        snprintf(base_extras, sizeof(base_extras),
                 "%s, fortified structure, defense %d",
                 base_keywords, card->defense);
    } else {
        snprintf(base_extras, sizeof(base_extras), "%s", base_keywords);
    }

    // Add outpost visual modifier
    if (card->is_outpost) {
        strncat(base_extras, ", defensive position, shield barrier",
                sizeof(base_extras) - strlen(base_extras) - 1);
    }

    prompt->positive = build_positive_prompt(
        card->name,
        faction_style,
        base_extras,
        style_keywords,
        card->flavor,
        cfg.include_quality_tags
    );

    prompt->negative = safe_strdup(NEGATIVE_PROMPT_BASE);

    if (prompt->positive == NULL || prompt->negative == NULL) {
        prompt_free(prompt);
        return NULL;
    }

    return prompt;
}
// }}}

// {{{ prompt_build_action
ImagePrompt* prompt_build_action(CardType* card, ArtStyle style,
                                  const ImagePromptConfig* config) {
    if (card == NULL) {
        return NULL;
    }

    ImagePromptConfig cfg = config ? *config : prompt_config_default();
    ImagePrompt* prompt = create_prompt_struct(&cfg);
    if (prompt == NULL) {
        return NULL;
    }

    const char* faction_style = prompt_get_faction_style(card->faction);
    const char* style_keywords = prompt_get_art_style_keywords(style);

    // Build effect-based keywords from the card's primary effect
    char effect_keywords[256];
    snprintf(effect_keywords, sizeof(effect_keywords), "%s", action_keywords);

    if (card->effects != NULL && card->effect_count > 0) {
        // Add visual hints based on primary effect type
        switch (card->effects[0].type) {
            case EFFECT_TRADE:
                strncat(effect_keywords, ", golden coins, wealth flow",
                        sizeof(effect_keywords) - strlen(effect_keywords) - 1);
                break;
            case EFFECT_COMBAT:
                strncat(effect_keywords, ", destructive energy, weapon strike",
                        sizeof(effect_keywords) - strlen(effect_keywords) - 1);
                break;
            case EFFECT_AUTHORITY:
                strncat(effect_keywords, ", healing light, protective aura",
                        sizeof(effect_keywords) - strlen(effect_keywords) - 1);
                break;
            case EFFECT_DRAW:
                strncat(effect_keywords, ", swirling cards, mystic knowledge",
                        sizeof(effect_keywords) - strlen(effect_keywords) - 1);
                break;
            case EFFECT_DESTROY_BASE:
                strncat(effect_keywords, ", explosive force, crumbling walls",
                        sizeof(effect_keywords) - strlen(effect_keywords) - 1);
                break;
            default:
                break;
        }
    }

    prompt->positive = build_positive_prompt(
        card->name,
        faction_style,
        effect_keywords,
        style_keywords,
        card->flavor,
        cfg.include_quality_tags
    );

    prompt->negative = safe_strdup(NEGATIVE_PROMPT_BASE);

    if (prompt->positive == NULL || prompt->negative == NULL) {
        prompt_free(prompt);
        return NULL;
    }

    return prompt;
}
// }}}

// {{{ prompt_build_card
ImagePrompt* prompt_build_card(CardType* card, ArtStyle style,
                                const ImagePromptConfig* config) {
    if (card == NULL) {
        return NULL;
    }

    // Route to appropriate builder based on card kind
    switch (card->kind) {
        case CARD_KIND_BASE:
            return prompt_build_base(card, style, config);

        case CARD_KIND_SHIP:
        case CARD_KIND_UNIT:
            // For ships and units, check if it has effects that suggest
            // it's more of an action card than a creature
            if (card->effects != NULL && card->effect_count > 0) {
                EffectType primary = card->effects[0].type;
                // Action-oriented effects get action treatment
                if (primary == EFFECT_DRAW || primary == EFFECT_DISCARD ||
                    primary == EFFECT_SCRAP_TRADE_ROW || primary == EFFECT_SCRAP_HAND ||
                    primary == EFFECT_DESTROY_BASE || primary == EFFECT_COPY_SHIP) {
                    return prompt_build_action(card, style, config);
                }
            }
            // Default to creature treatment
            return prompt_build_creature(card, style, config);

        default:
            // Unknown kind, try creature
            return prompt_build_creature(card, style, config);
    }
}
// }}}

// {{{ prompt_build_with_instance
ImagePrompt* prompt_build_with_instance(CardInstance* instance, ArtStyle style,
                                         const ImagePromptConfig* config) {
    if (instance == NULL || instance->type == NULL) {
        return NULL;
    }

    // Build base prompt from card type
    ImagePrompt* prompt = prompt_build_card(instance->type, style, config);
    if (prompt == NULL) {
        return NULL;
    }

    // Apply upgrade visual modifiers if any bonuses exist
    int total_bonus = instance->attack_bonus + instance->trade_bonus +
                      instance->authority_bonus;
    if (total_bonus > 0) {
        // Create expanded positive prompt with upgrade modifiers
        char expanded[MAX_PROMPT_LENGTH];
        strncpy(expanded, prompt->positive, MAX_PROMPT_LENGTH - 1);
        expanded[MAX_PROMPT_LENGTH - 1] = '\0';

        add_upgrade_modifiers(expanded, MAX_PROMPT_LENGTH,
                              instance->attack_bonus,
                              instance->trade_bonus,
                              instance->authority_bonus);

        free(prompt->positive);
        prompt->positive = strdup(expanded);

        if (prompt->positive == NULL) {
            prompt_free(prompt);
            return NULL;
        }
    }

    // Use instance seed if available
    if (instance->image_seed != 0) {
        prompt->seed = instance->image_seed;
    }

    return prompt;
}
// }}}
