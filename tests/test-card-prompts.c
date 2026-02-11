/*
 * test-card-prompts.c - Tests for Card Image Prompt Builder
 *
 * Validates prompt generation for different card types, faction styles,
 * art styles, and upgrade modifiers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/visual/02-card-prompts.h"

// {{{ Test Statistics
static int tests_run = 0;
static int tests_passed = 0;
// }}}

// {{{ Mock Data
static CardType mock_creature;
static CardType mock_base;
static CardType mock_action_card;
static CardInstance mock_instance;
static CardInstance mock_upgraded_instance;
static Effect mock_trade_effect;
static Effect mock_combat_effect;
static Effect mock_draw_effect;

static void setup_mocks(void) {
    // Mock creature card (Wilds faction ship)
    mock_creature.id = "dire_bear";
    mock_creature.name = "Dire Bear";
    mock_creature.flavor = "The forest's apex predator";
    mock_creature.cost = 4;
    mock_creature.faction = FACTION_WILDS;
    mock_creature.kind = CARD_KIND_SHIP;
    mock_creature.defense = 0;
    mock_creature.is_outpost = false;
    mock_combat_effect.type = EFFECT_COMBAT;
    mock_combat_effect.value = 4;
    mock_combat_effect.target_card_id = NULL;
    mock_creature.effects = &mock_combat_effect;
    mock_creature.effect_count = 1;
    mock_creature.ally_effects = NULL;
    mock_creature.ally_effect_count = 0;
    mock_creature.scrap_effects = NULL;
    mock_creature.scrap_effect_count = 0;
    mock_creature.spawns_id = NULL;

    // Mock base card (Kingdom faction)
    mock_base.id = "fortress_wall";
    mock_base.name = "Fortress Wall";
    mock_base.flavor = "Impenetrable stone defense";
    mock_base.cost = 5;
    mock_base.faction = FACTION_KINGDOM;
    mock_base.kind = CARD_KIND_BASE;
    mock_base.defense = 5;
    mock_base.is_outpost = true;
    mock_base.effects = NULL;
    mock_base.effect_count = 0;
    mock_base.ally_effects = NULL;
    mock_base.ally_effect_count = 0;
    mock_base.scrap_effects = NULL;
    mock_base.scrap_effect_count = 0;
    mock_base.spawns_id = NULL;

    // Mock action card (Merchant faction)
    mock_action_card.id = "trade_contract";
    mock_action_card.name = "Trade Contract";
    mock_action_card.flavor = "Seal the deal";
    mock_action_card.cost = 2;
    mock_action_card.faction = FACTION_MERCHANT;
    mock_action_card.kind = CARD_KIND_SHIP;
    mock_action_card.defense = 0;
    mock_action_card.is_outpost = false;
    mock_trade_effect.type = EFFECT_TRADE;
    mock_trade_effect.value = 3;
    mock_trade_effect.target_card_id = NULL;
    mock_action_card.effects = &mock_trade_effect;
    mock_action_card.effect_count = 1;
    mock_action_card.ally_effects = NULL;
    mock_action_card.ally_effect_count = 0;
    mock_action_card.scrap_effects = NULL;
    mock_action_card.scrap_effect_count = 0;
    mock_action_card.spawns_id = NULL;

    // Mock card instance (no upgrades)
    mock_instance.type = &mock_creature;
    mock_instance.instance_id = "inst_001";
    mock_instance.attack_bonus = 0;
    mock_instance.trade_bonus = 0;
    mock_instance.authority_bonus = 0;
    mock_instance.image_seed = 0;
    mock_instance.needs_regen = false;
    mock_instance.draw_effect_spent = false;
    mock_instance.placement = ZONE_NONE;
    mock_instance.deployed = false;
    mock_instance.damage_taken = 0;

    // Mock upgraded instance
    mock_upgraded_instance.type = &mock_creature;
    mock_upgraded_instance.instance_id = "inst_002";
    mock_upgraded_instance.attack_bonus = 2;
    mock_upgraded_instance.trade_bonus = 1;
    mock_upgraded_instance.authority_bonus = 0;
    mock_upgraded_instance.image_seed = 12345;
    mock_upgraded_instance.needs_regen = false;
    mock_upgraded_instance.draw_effect_spent = false;
    mock_upgraded_instance.placement = ZONE_NONE;
    mock_upgraded_instance.deployed = false;
    mock_upgraded_instance.damage_taken = 0;
}
// }}}

// {{{ test_config_default
static void test_config_default(void) {
    printf("  Testing default config...\\n");
    tests_run++;

    ImagePromptConfig config = prompt_config_default();

    assert(config.default_width == 512);
    assert(config.default_height == 768);
    assert(config.default_steps == 30);
    assert(config.default_cfg_scale > 7.0f && config.default_cfg_scale < 8.0f);
    assert(config.include_quality_tags == true);
    assert(config.include_artist_tags == false);

    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_faction_style_merchant
static void test_faction_style_merchant(void) {
    printf("  Testing Merchant faction style...\\n");
    tests_run++;

    const char* style = prompt_get_faction_style(FACTION_MERCHANT);

    assert(style != NULL);
    assert(strstr(style, "golden") != NULL);
    assert(strstr(style, "trade") != NULL || strstr(style, "coins") != NULL);

    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_faction_style_wilds
static void test_faction_style_wilds(void) {
    printf("  Testing Wilds faction style...\\n");
    tests_run++;

    const char* style = prompt_get_faction_style(FACTION_WILDS);

    assert(style != NULL);
    assert(strstr(style, "forest") != NULL || strstr(style, "primal") != NULL);
    assert(strstr(style, "beasts") != NULL || strstr(style, "creatures") != NULL);

    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_faction_style_kingdom
static void test_faction_style_kingdom(void) {
    printf("  Testing Kingdom faction style...\\n");
    tests_run++;

    const char* style = prompt_get_faction_style(FACTION_KINGDOM);

    assert(style != NULL);
    assert(strstr(style, "knight") != NULL || strstr(style, "castle") != NULL);
    assert(strstr(style, "noble") != NULL || strstr(style, "armor") != NULL);

    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_faction_style_artificer
static void test_faction_style_artificer(void) {
    printf("  Testing Artificer faction style...\\n");
    tests_run++;

    const char* style = prompt_get_faction_style(FACTION_ARTIFICER);

    assert(style != NULL);
    assert(strstr(style, "gear") != NULL || strstr(style, "construct") != NULL);
    assert(strstr(style, "machine") != NULL || strstr(style, "clockwork") != NULL);

    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_faction_style_neutral
static void test_faction_style_neutral(void) {
    printf("  Testing Neutral faction style...\\n");
    tests_run++;

    const char* style = prompt_get_faction_style(FACTION_NEUTRAL);

    assert(style != NULL);
    assert(strstr(style, "mystical") != NULL || strstr(style, "arcane") != NULL);

    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_faction_style_invalid
static void test_faction_style_invalid(void) {
    printf("  Testing invalid faction style fallback...\\n");
    tests_run++;

    const char* style = prompt_get_faction_style((Faction)99);

    // Should fall back to neutral
    assert(style != NULL);
    assert(style == prompt_get_faction_style(FACTION_NEUTRAL));

    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_art_style_painterly
static void test_art_style_painterly(void) {
    printf("  Testing painterly art style...\\n");
    tests_run++;

    const char* keywords = prompt_get_art_style_keywords(STYLE_PAINTERLY);

    assert(keywords != NULL);
    assert(strstr(keywords, "oil painting") != NULL);
    assert(strstr(keywords, "traditional") != NULL);

    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_art_style_detailed
static void test_art_style_detailed(void) {
    printf("  Testing detailed art style...\\n");
    tests_run++;

    const char* keywords = prompt_get_art_style_keywords(STYLE_DETAILED);

    assert(keywords != NULL);
    assert(strstr(keywords, "detailed") != NULL);
    assert(strstr(keywords, "realistic") != NULL || strstr(keywords, "intricate") != NULL);

    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_art_style_to_string
static void test_art_style_to_string(void) {
    printf("  Testing art style to string...\\n");
    tests_run++;

    assert(strcmp(art_style_to_string(STYLE_PAINTERLY), "painterly") == 0);
    assert(strcmp(art_style_to_string(STYLE_DETAILED), "detailed") == 0);
    assert(strcmp(art_style_to_string(STYLE_STYLIZED), "stylized") == 0);
    assert(strcmp(art_style_to_string(STYLE_ICON), "icon") == 0);

    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_art_style_from_string
static void test_art_style_from_string(void) {
    printf("  Testing art style from string...\\n");
    tests_run++;

    assert(art_style_from_string("painterly") == STYLE_PAINTERLY);
    assert(art_style_from_string("DETAILED") == STYLE_DETAILED);
    assert(art_style_from_string("Stylized") == STYLE_STYLIZED);
    assert(art_style_from_string("icon") == STYLE_ICON);
    assert(art_style_from_string("unknown") == STYLE_PAINTERLY);  // default
    assert(art_style_from_string(NULL) == STYLE_PAINTERLY);

    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_negative_prompt_base
static void test_negative_prompt_base(void) {
    printf("  Testing negative prompt base...\\n");
    tests_run++;

    const char* negative = prompt_get_negative_base();

    assert(negative != NULL);
    assert(strstr(negative, "blurry") != NULL);
    assert(strstr(negative, "watermark") != NULL);
    assert(strstr(negative, "low quality") != NULL);

    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_build_creature_basic
static void test_build_creature_basic(void) {
    printf("  Testing basic creature prompt build...\\n");
    tests_run++;

    ImagePrompt* prompt = prompt_build_creature(&mock_creature, STYLE_PAINTERLY, NULL);

    assert(prompt != NULL);
    assert(prompt->positive != NULL);
    assert(prompt->negative != NULL);
    assert(prompt->width == 512);
    assert(prompt->height == 768);

    // Should contain card name
    assert(strstr(prompt->positive, "Dire Bear") != NULL);

    // Should contain faction style (Wilds)
    assert(strstr(prompt->positive, "forest") != NULL ||
           strstr(prompt->positive, "primal") != NULL ||
           strstr(prompt->positive, "beasts") != NULL);

    // Should contain art style
    assert(strstr(prompt->positive, "oil painting") != NULL ||
           strstr(prompt->positive, "traditional") != NULL);

    prompt_free(prompt);
    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_build_creature_with_config
static void test_build_creature_with_config(void) {
    printf("  Testing creature prompt with custom config...\\n");
    tests_run++;

    ImagePromptConfig config = {
        .default_width = 1024,
        .default_height = 1024,
        .default_steps = 50,
        .default_cfg_scale = 10.0f,
        .include_quality_tags = false,
        .include_artist_tags = false
    };

    ImagePrompt* prompt = prompt_build_creature(&mock_creature, STYLE_DETAILED, &config);

    assert(prompt != NULL);
    assert(prompt->width == 1024);
    assert(prompt->height == 1024);
    assert(prompt->steps == 50);
    assert(prompt->cfg_scale > 9.9f && prompt->cfg_scale < 10.1f);

    // Quality tags should NOT be present when disabled
    assert(strstr(prompt->positive, "masterpiece") == NULL);

    prompt_free(prompt);
    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_build_base_basic
static void test_build_base_basic(void) {
    printf("  Testing basic base prompt build...\\n");
    tests_run++;

    ImagePrompt* prompt = prompt_build_base(&mock_base, STYLE_PAINTERLY, NULL);

    assert(prompt != NULL);
    assert(prompt->positive != NULL);
    assert(prompt->negative != NULL);

    // Bases should use landscape orientation (width > height)
    assert(prompt->width == 768);
    assert(prompt->height == 512);

    // Should contain card name
    assert(strstr(prompt->positive, "Fortress Wall") != NULL);

    // Should contain Kingdom faction style
    assert(strstr(prompt->positive, "knight") != NULL ||
           strstr(prompt->positive, "castle") != NULL ||
           strstr(prompt->positive, "armor") != NULL);

    // Should contain base-specific keywords
    assert(strstr(prompt->positive, "architectural") != NULL ||
           strstr(prompt->positive, "location") != NULL);

    prompt_free(prompt);
    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_build_base_outpost
static void test_build_base_outpost(void) {
    printf("  Testing outpost base prompt...\\n");
    tests_run++;

    ImagePrompt* prompt = prompt_build_base(&mock_base, STYLE_PAINTERLY, NULL);

    assert(prompt != NULL);

    // Outpost should have defensive visual modifiers
    assert(strstr(prompt->positive, "defensive") != NULL ||
           strstr(prompt->positive, "shield") != NULL ||
           strstr(prompt->positive, "fortified") != NULL);

    prompt_free(prompt);
    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_build_action_basic
static void test_build_action_basic(void) {
    printf("  Testing basic action prompt build...\\n");
    tests_run++;

    // Create an action-like card
    CardType action_card;
    action_card.id = "arcane_burst";
    action_card.name = "Arcane Burst";
    action_card.flavor = "Raw magical power";
    action_card.cost = 3;
    action_card.faction = FACTION_ARTIFICER;
    action_card.kind = CARD_KIND_SHIP;
    action_card.defense = 0;
    action_card.is_outpost = false;
    mock_draw_effect.type = EFFECT_DRAW;
    mock_draw_effect.value = 2;
    mock_draw_effect.target_card_id = NULL;
    action_card.effects = &mock_draw_effect;
    action_card.effect_count = 1;
    action_card.ally_effects = NULL;
    action_card.ally_effect_count = 0;
    action_card.scrap_effects = NULL;
    action_card.scrap_effect_count = 0;
    action_card.spawns_id = NULL;

    ImagePrompt* prompt = prompt_build_action(&action_card, STYLE_STYLIZED, NULL);

    assert(prompt != NULL);
    assert(prompt->positive != NULL);

    // Should contain card name
    assert(strstr(prompt->positive, "Arcane Burst") != NULL);

    // Should contain action keywords
    assert(strstr(prompt->positive, "magical") != NULL ||
           strstr(prompt->positive, "energy") != NULL ||
           strstr(prompt->positive, "dynamic") != NULL);

    // Should contain effect-specific keywords (draw = knowledge)
    assert(strstr(prompt->positive, "card") != NULL ||
           strstr(prompt->positive, "knowledge") != NULL ||
           strstr(prompt->positive, "swirling") != NULL);

    prompt_free(prompt);
    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_build_card_routes_creature
static void test_build_card_routes_creature(void) {
    printf("  Testing card routing to creature...\\n");
    tests_run++;

    ImagePrompt* prompt = prompt_build_card(&mock_creature, STYLE_PAINTERLY, NULL);

    assert(prompt != NULL);

    // Combat-focused ship should route to creature builder
    // Portrait orientation (width < height)
    assert(prompt->width < prompt->height);

    prompt_free(prompt);
    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_build_card_routes_base
static void test_build_card_routes_base(void) {
    printf("  Testing card routing to base...\\n");
    tests_run++;

    ImagePrompt* prompt = prompt_build_card(&mock_base, STYLE_PAINTERLY, NULL);

    assert(prompt != NULL);

    // Base should route to base builder
    // Landscape orientation (width > height)
    assert(prompt->width > prompt->height);

    prompt_free(prompt);
    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_build_with_instance_basic
static void test_build_with_instance_basic(void) {
    printf("  Testing instance prompt (no upgrades)...\\n");
    tests_run++;

    ImagePrompt* prompt = prompt_build_with_instance(&mock_instance, STYLE_PAINTERLY, NULL);

    assert(prompt != NULL);
    assert(prompt->positive != NULL);

    // Should contain card name
    assert(strstr(prompt->positive, "Dire Bear") != NULL);

    // Should NOT have upgrade modifiers
    assert(strstr(prompt->positive, "glowing weapon") == NULL);
    assert(strstr(prompt->positive, "golden glow") == NULL);

    prompt_free(prompt);
    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_build_with_instance_upgraded
static void test_build_with_instance_upgraded(void) {
    printf("  Testing instance prompt (with upgrades)...\\n");
    tests_run++;

    ImagePrompt* prompt = prompt_build_with_instance(&mock_upgraded_instance, STYLE_PAINTERLY, NULL);

    assert(prompt != NULL);
    assert(prompt->positive != NULL);

    // Should have attack upgrade modifiers
    assert(strstr(prompt->positive, "glowing weapon") != NULL ||
           strstr(prompt->positive, "enhanced power") != NULL ||
           strstr(prompt->positive, "fiery aura") != NULL);

    // Should have trade upgrade modifiers
    assert(strstr(prompt->positive, "golden glow") != NULL ||
           strstr(prompt->positive, "prosperous") != NULL ||
           strstr(prompt->positive, "jeweled") != NULL);

    // Should use instance seed
    assert(prompt->seed == 12345);

    prompt_free(prompt);
    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_null_safety_card
static void test_null_safety_card(void) {
    printf("  Testing NULL safety for card...\\n");
    tests_run++;

    assert(prompt_build_card(NULL, STYLE_PAINTERLY, NULL) == NULL);
    assert(prompt_build_creature(NULL, STYLE_PAINTERLY, NULL) == NULL);
    assert(prompt_build_base(NULL, STYLE_PAINTERLY, NULL) == NULL);
    assert(prompt_build_action(NULL, STYLE_PAINTERLY, NULL) == NULL);

    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_null_safety_instance
static void test_null_safety_instance(void) {
    printf("  Testing NULL safety for instance...\\n");
    tests_run++;

    assert(prompt_build_with_instance(NULL, STYLE_PAINTERLY, NULL) == NULL);

    CardInstance bad_instance;
    bad_instance.type = NULL;
    assert(prompt_build_with_instance(&bad_instance, STYLE_PAINTERLY, NULL) == NULL);

    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ test_prompt_free_null
static void test_prompt_free_null(void) {
    printf("  Testing prompt_free with NULL...\\n");
    tests_run++;

    // Should not crash
    prompt_free(NULL);

    tests_passed++;
    printf("    PASSED\\n");
}
// }}}

// {{{ main
int main(void) {
    printf("=== Card Image Prompt Builder Tests ===\\n\\n");

    setup_mocks();

    printf("Config Tests:\\n");
    test_config_default();

    printf("\\nFaction Style Tests:\\n");
    test_faction_style_merchant();
    test_faction_style_wilds();
    test_faction_style_kingdom();
    test_faction_style_artificer();
    test_faction_style_neutral();
    test_faction_style_invalid();

    printf("\\nArt Style Tests:\\n");
    test_art_style_painterly();
    test_art_style_detailed();
    test_art_style_to_string();
    test_art_style_from_string();
    test_negative_prompt_base();

    printf("\\nCreature Prompt Tests:\\n");
    test_build_creature_basic();
    test_build_creature_with_config();

    printf("\\nBase Prompt Tests:\\n");
    test_build_base_basic();
    test_build_base_outpost();

    printf("\\nAction Prompt Tests:\\n");
    test_build_action_basic();

    printf("\\nCard Routing Tests:\\n");
    test_build_card_routes_creature();
    test_build_card_routes_base();

    printf("\\nInstance Prompt Tests:\\n");
    test_build_with_instance_basic();
    test_build_with_instance_upgraded();

    printf("\\nSafety Tests:\\n");
    test_null_safety_card();
    test_null_safety_instance();
    test_prompt_free_null();

    printf("\\n=== Results: %d/%d tests passed ===\\n", tests_passed, tests_run);

    return tests_passed == tests_run ? 0 : 1;
}
// }}}
