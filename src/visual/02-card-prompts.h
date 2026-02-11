/*
 * 02-card-prompts.h - Card Image Prompt Builder
 *
 * Generates ComfyUI-compatible prompts for card art generation.
 * Uses card metadata (name, faction, type) to build themed prompts
 * with faction-specific style guides and art style modifiers.
 */

#ifndef CARD_PROMPTS_H
#define CARD_PROMPTS_H

#include <stdbool.h>
#include "../core/01-card.h"

// {{{ ArtStyle
// Available art styles for card generation. Each style produces
// different visual aesthetics for the same card content.
typedef enum {
    STYLE_PAINTERLY = 0,  // Oil painting, traditional fantasy art
    STYLE_DETAILED,       // High detail, realistic rendering
    STYLE_STYLIZED,       // Bold colors, graphic novel aesthetic
    STYLE_ICON,           // Simplified, iconic representation
    STYLE_COUNT           // Sentinel for validation
} ArtStyle;
// }}}

// {{{ ImagePrompt
// Complete prompt configuration for ComfyUI image generation.
// Includes positive/negative prompts and generation parameters.
typedef struct {
    char* positive;       // Positive prompt describing desired image
    char* negative;       // Negative prompt for elements to avoid
    int width;            // Image width in pixels
    int height;           // Image height in pixels
    int steps;            // Number of diffusion steps
    float cfg_scale;      // Classifier-free guidance scale (1.0-20.0)
    uint32_t seed;        // Random seed (0 for random)
} ImagePrompt;
// }}}

// {{{ ImagePromptConfig
// Configuration for prompt generation defaults.
typedef struct {
    int default_width;
    int default_height;
    int default_steps;
    float default_cfg_scale;
    bool include_quality_tags;  // Add quality boosting keywords
    bool include_artist_tags;   // Add artist style references
} ImagePromptConfig;
// }}}

// {{{ prompt_config_default
// Returns default configuration for prompt generation.
ImagePromptConfig prompt_config_default(void);
// }}}

// {{{ prompt_build_card
// Builds a prompt for any card type. Selects appropriate sub-builder
// based on card kind (ship, base, unit).
// Returns NULL on failure. Caller must free with prompt_free().
ImagePrompt* prompt_build_card(CardType* card, ArtStyle style,
                                const ImagePromptConfig* config);
// }}}

// {{{ prompt_build_creature
// Builds a prompt specifically for creature-type cards (ships, units).
// Emphasizes the creature/character with action poses.
// Returns NULL on failure. Caller must free with prompt_free().
ImagePrompt* prompt_build_creature(CardType* card, ArtStyle style,
                                    const ImagePromptConfig* config);
// }}}

// {{{ prompt_build_base
// Builds a prompt for base cards. Emphasizes architecture, location,
// and environmental elements with the faction aesthetic.
// Returns NULL on failure. Caller must free with prompt_free().
ImagePrompt* prompt_build_base(CardType* card, ArtStyle style,
                                const ImagePromptConfig* config);
// }}}

// {{{ prompt_build_action
// Builds a prompt for action/effect visualization. Emphasizes magical
// energy, dynamic effects, and the card's mechanical effect.
// Returns NULL on failure. Caller must free with prompt_free().
ImagePrompt* prompt_build_action(CardType* card, ArtStyle style,
                                  const ImagePromptConfig* config);
// }}}

// {{{ prompt_build_with_instance
// Builds a prompt using a CardInstance for upgrade visualization.
// Upgraded cards show visual modifications (glowing weapons, enhanced
// armor, etc.) based on their bonuses.
// Returns NULL on failure. Caller must free with prompt_free().
ImagePrompt* prompt_build_with_instance(CardInstance* instance, ArtStyle style,
                                         const ImagePromptConfig* config);
// }}}

// {{{ prompt_free
// Frees all memory associated with an ImagePrompt.
// Safe to call with NULL.
void prompt_free(ImagePrompt* prompt);
// }}}

// {{{ prompt_get_faction_style
// Returns the style keywords for a faction. Used internally but
// exposed for testing and custom prompt building.
const char* prompt_get_faction_style(Faction faction);
// }}}

// {{{ prompt_get_art_style_keywords
// Returns the style keywords for an art style.
const char* prompt_get_art_style_keywords(ArtStyle style);
// }}}

// {{{ prompt_get_negative_base
// Returns the base negative prompt used for all generations.
const char* prompt_get_negative_base(void);
// }}}

// {{{ art_style_to_string
// Returns string representation of art style.
const char* art_style_to_string(ArtStyle style);
// }}}

// {{{ art_style_from_string
// Parses art style from string. Returns STYLE_PAINTERLY on failure.
ArtStyle art_style_from_string(const char* str);
// }}}

#endif /* CARD_PROMPTS_H */
