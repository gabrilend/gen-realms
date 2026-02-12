/*
 * card-renderer.c - Card Rendering Implementation for WASM Client
 *
 * Renders cards with faction-specific styling, effect badges,
 * and visual states.
 */

#include "card-renderer.h"
#include "draw2d.h"
#include "theme.h"
#include <stdio.h>
#include <string.h>

/* Badge dimensions */
#define BADGE_SIZE 20
#define BADGE_SPACING 2

/* {{{ render_effect_badge
 * Render a small badge showing an effect value.
 */
static void render_effect_badge(int x, int y, int value, uint32_t color) {
    if (value <= 0) return;

    /* Background */
    draw_circle(x + BADGE_SIZE/2, y + BADGE_SIZE/2, BADGE_SIZE/2, THEME_BG_DARK);
    draw_circle_outline(x + BADGE_SIZE/2, y + BADGE_SIZE/2, BADGE_SIZE/2, color, 2);

    /* Value */
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", value);
    int text_w = draw_measure_text(buf, FONT_SIZE_SMALL);
    draw_text_ex(x + BADGE_SIZE/2 - text_w/2, y + BADGE_SIZE/2,
                 buf, color, FONT_SIZE_SMALL, TEXT_ALIGN_LEFT, TEXT_BASELINE_MIDDLE);
}
/* }}} */

/* {{{ render_upgrade_badge
 * Render a small badge showing an upgrade bonus.
 */
static void render_upgrade_badge(int x, int y, int value, uint32_t color) {
    if (value <= 0) return;

    /* Plus sign badge */
    draw_rect(x, y, BADGE_SIZE, BADGE_SIZE, color);

    char buf[8];
    snprintf(buf, sizeof(buf), "+%d", value);
    int text_w = draw_measure_text(buf, FONT_SIZE_SMALL);
    draw_text_ex(x + BADGE_SIZE/2 - text_w/2, y + BADGE_SIZE/2,
                 buf, THEME_TEXT_PRIMARY, FONT_SIZE_SMALL, TEXT_ALIGN_LEFT, TEXT_BASELINE_MIDDLE);
}
/* }}} */

/* {{{ card_render */
void card_render(int x, int y, int w, int h, const CardRenderData* data) {
    if (!data) return;

    /* Get faction color */
    uint32_t faction_color = theme_get_faction_color(data->faction);
    uint32_t faction_dark = theme_get_faction_color_dark(data->faction);

    /* Background color based on state */
    uint32_t bg_color = THEME_BG_CARD;
    uint32_t border_color = faction_color;
    int border_width = 2;

    switch (data->state) {
        case CARD_STATE_HOVER:
            bg_color = THEME_BG_HIGHLIGHT;
            border_width = 3;
            break;

        case CARD_STATE_SELECTED:
            border_color = THEME_TEXT_PRIMARY;
            border_width = 3;
            break;

        case CARD_STATE_DISABLED:
            draw_set_alpha(0.5f);
            break;

        case CARD_STATE_ALLY_ACTIVE:
            border_color = THEME_GLOW_ALLY;
            border_width = 3;
            break;

        case CARD_STATE_EMPOWERED:
            border_color = THEME_GLOW_EMPOWERED;
            border_width = 3;
            break;

        case CARD_STATE_TARGETED:
            border_color = THEME_GLOW_TARGETED;
            border_width = 3;
            break;

        default:
            break;
    }

    /* Card body */
    draw_rounded_rect(x, y, w, h, CARD_CORNER_RADIUS, bg_color);
    draw_rounded_rect_outline(x, y, w, h, CARD_CORNER_RADIUS, border_color, border_width);

    /* Faction stripe at top */
    draw_rect(x + 2, y + 2, w - 4, 4, faction_color);

    /* Cost badge (top left) */
    if (data->cost > 0) {
        draw_circle(x + 15, y + 20, 12, THEME_VALUE_TRADE);
        char cost_str[4];
        snprintf(cost_str, sizeof(cost_str), "%d", data->cost);
        draw_text_ex(x + 15, y + 20, cost_str, THEME_BG_DARK,
                     FONT_SIZE_NORMAL, TEXT_ALIGN_CENTER, TEXT_BASELINE_MIDDLE);
    }

    /* Card name */
    int name_y = y + 38;
    if (data->name) {
        /* Truncate long names */
        char name_buf[20];
        strncpy(name_buf, data->name, sizeof(name_buf) - 1);
        name_buf[sizeof(name_buf) - 1] = '\0';

        int name_w = draw_measure_text(name_buf, FONT_SIZE_SMALL);
        int name_x = x + (w - name_w) / 2;
        draw_text(name_x, name_y, name_buf, faction_color);
    }

    /* Card type indicator */
    const char* type_str = data->card_type;
    if (type_str) {
        draw_text_ex(x + w/2, y + h - 15, type_str, THEME_TEXT_DIM,
                     FONT_SIZE_SMALL, TEXT_ALIGN_CENTER, TEXT_BASELINE_MIDDLE);
    }

    /* Effect badges (right side) */
    int badge_x = x + w - BADGE_SIZE - 5;
    int badge_y = y + 55;

    if (data->trade_value > 0) {
        render_effect_badge(badge_x, badge_y, data->trade_value, THEME_VALUE_TRADE);
        badge_y += BADGE_SIZE + BADGE_SPACING;
    }

    if (data->combat_value > 0) {
        render_effect_badge(badge_x, badge_y, data->combat_value, THEME_VALUE_COMBAT);
        badge_y += BADGE_SIZE + BADGE_SPACING;
    }

    if (data->authority_value > 0) {
        render_effect_badge(badge_x, badge_y, data->authority_value, THEME_VALUE_AUTHORITY);
        badge_y += BADGE_SIZE + BADGE_SPACING;
    }

    if (data->draw_value > 0) {
        render_effect_badge(badge_x, badge_y, data->draw_value, THEME_VALUE_DRAW);
        badge_y += BADGE_SIZE + BADGE_SPACING;
    }

    /* Defense (for bases) */
    if (data->defense > 0) {
        draw_rect(x + 5, y + h - 30, 25, 18, faction_dark);
        char def_str[8];
        snprintf(def_str, sizeof(def_str), "%d", data->defense);
        draw_text_ex(x + 17, y + h - 21, def_str, THEME_TEXT_PRIMARY,
                     FONT_SIZE_NORMAL, TEXT_ALIGN_CENTER, TEXT_BASELINE_MIDDLE);

        /* Outpost indicator */
        if (data->is_outpost) {
            draw_circle(x + 5, y + h - 35, 4, THEME_STATUS_WARNING);
        }
    }

    /* Upgrade badges (left side) */
    int upgrade_y = y + 55;

    if (data->upgrade_attack > 0) {
        render_upgrade_badge(x + 5, upgrade_y, data->upgrade_attack, THEME_VALUE_COMBAT);
        upgrade_y += BADGE_SIZE + BADGE_SPACING;
    }

    if (data->upgrade_trade > 0) {
        render_upgrade_badge(x + 5, upgrade_y, data->upgrade_trade, THEME_VALUE_TRADE);
        upgrade_y += BADGE_SIZE + BADGE_SPACING;
    }

    if (data->upgrade_authority > 0) {
        render_upgrade_badge(x + 5, upgrade_y, data->upgrade_authority, THEME_VALUE_AUTHORITY);
        upgrade_y += BADGE_SIZE + BADGE_SPACING;
    }

    /* Reset alpha if disabled */
    if (data->state == CARD_STATE_DISABLED) {
        draw_reset_alpha();
    }
}
/* }}} */

/* {{{ card_render_back */
void card_render_back(int x, int y, int w, int h) {
    /* Dark card back with pattern */
    draw_rounded_rect(x, y, w, h, CARD_CORNER_RADIUS, THEME_BG_DARK);
    draw_rounded_rect_outline(x, y, w, h, CARD_CORNER_RADIUS, THEME_BORDER_NORMAL, 2);

    /* Simple pattern */
    int cx = x + w / 2;
    int cy = y + h / 2;

    draw_circle_outline(cx, cy, 20, THEME_BORDER_DIM, 2);
    draw_circle_outline(cx, cy, 30, THEME_BORDER_DIM, 1);

    /* Symbeline text */
    draw_text_ex(cx, cy, "S", THEME_BORDER_NORMAL, FONT_SIZE_TITLE,
                 TEXT_ALIGN_CENTER, TEXT_BASELINE_MIDDLE);
}
/* }}} */

/* {{{ card_render_empty_slot */
void card_render_empty_slot(int x, int y, int w, int h) {
    /* Dashed border rectangle */
    draw_rounded_rect_outline(x, y, w, h, CARD_CORNER_RADIUS, THEME_BORDER_DIM, 1);

    /* Dashed effect (draw short lines) */
    int dash_len = 8;
    int gap_len = 4;

    /* Top edge */
    for (int dx = CARD_CORNER_RADIUS; dx < w - CARD_CORNER_RADIUS; dx += dash_len + gap_len) {
        int len = dash_len;
        if (dx + len > w - CARD_CORNER_RADIUS) len = w - CARD_CORNER_RADIUS - dx;
        draw_line(x + dx, y, x + dx + len, y, THEME_BORDER_DIM, 1);
    }

    /* Plus sign in center */
    int cx = x + w / 2;
    int cy = y + h / 2;
    draw_line(cx - 10, cy, cx + 10, cy, THEME_BORDER_DIM, 2);
    draw_line(cx, cy - 10, cx, cy + 10, THEME_BORDER_DIM, 2);
}
/* }}} */

/* {{{ card_render_placeholder */
void card_render_placeholder(int x, int y, int w, int h, int cost) {
    card_render_empty_slot(x, y, w, h);

    if (cost >= 0) {
        /* Cost indicator */
        int cx = x + w / 2;
        int cy = y + h / 2 + 20;

        char buf[8];
        snprintf(buf, sizeof(buf), "%d", cost);
        draw_text_ex(cx, cy, buf, THEME_VALUE_TRADE, FONT_SIZE_LARGE,
                     TEXT_ALIGN_CENTER, TEXT_BASELINE_MIDDLE);
    }
}
/* }}} */
