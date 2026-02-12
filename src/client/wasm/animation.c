/*
 * animation.c - Animation System Implementation for WASM Client
 *
 * Manages animation state, applies easing functions, and provides
 * particle effects for visual feedback.
 */

#include "animation.h"
#include "draw2d.h"
#include "theme.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

/* Global animation system */
static AnimationSystem g_anim = {0};

/* {{{ ease_apply
 * Apply an easing function to a normalized time value.
 */
float ease_apply(EasingType type, float t) {
    /* Clamp t to [0, 1] */
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    switch (type) {
        case EASE_LINEAR:
            return t;

        case EASE_IN_QUAD:
            return t * t;

        case EASE_OUT_QUAD:
            return t * (2.0f - t);

        case EASE_IN_OUT_QUAD:
            if (t < 0.5f) {
                return 2.0f * t * t;
            }
            return -1.0f + (4.0f - 2.0f * t) * t;

        case EASE_IN_CUBIC:
            return t * t * t;

        case EASE_OUT_CUBIC: {
            float f = t - 1.0f;
            return f * f * f + 1.0f;
        }

        case EASE_IN_OUT_CUBIC:
            if (t < 0.5f) {
                return 4.0f * t * t * t;
            } else {
                float f = 2.0f * t - 2.0f;
                return 0.5f * f * f * f + 1.0f;
            }

        case EASE_OUT_BACK: {
            /* Overshoot */
            float c1 = 1.70158f;
            float c3 = c1 + 1.0f;
            float f = t - 1.0f;
            return 1.0f + c3 * f * f * f + c1 * f * f;
        }

        case EASE_OUT_BOUNCE: {
            if (t < 1.0f / 2.75f) {
                return 7.5625f * t * t;
            } else if (t < 2.0f / 2.75f) {
                float f = t - 1.5f / 2.75f;
                return 7.5625f * f * f + 0.75f;
            } else if (t < 2.5f / 2.75f) {
                float f = t - 2.25f / 2.75f;
                return 7.5625f * f * f + 0.9375f;
            } else {
                float f = t - 2.625f / 2.75f;
                return 7.5625f * f * f + 0.984375f;
            }
        }

        default:
            return t;
    }
}
/* }}} */

/* {{{ find_free_animation
 * Find a free animation slot.
 */
static int find_free_animation(void) {
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        if (g_anim.animations[i].type == ANIM_NONE) {
            return i;
        }
    }
    return -1;
}
/* }}} */

/* {{{ anim_init */
void anim_init(void) {
    memset(&g_anim, 0, sizeof(g_anim));
    g_anim.time_scale = 1.0f;
    g_anim.paused = false;
}
/* }}} */

/* {{{ anim_cleanup */
void anim_cleanup(void) {
    memset(&g_anim, 0, sizeof(g_anim));
}
/* }}} */

/* {{{ update_single_animation
 * Update a single animation.
 */
static void update_single_animation(Animation* anim, float dt) {
    if (anim->type == ANIM_NONE || anim->complete) return;

    anim->elapsed += dt;
    float t = anim->elapsed / anim->duration;

    if (t >= 1.0f) {
        t = 1.0f;
        if (!anim->loop) {
            anim->complete = true;
        } else {
            anim->elapsed = 0.0f;
            t = 0.0f;
        }
    }

    float eased = ease_apply(anim->easing, t);

    switch (anim->type) {
        case ANIM_MOVE:
        case ANIM_SLIDE_IN:
        case ANIM_SLIDE_OUT:
            anim->current_x = anim->start_x + (anim->end_x - anim->start_x) * eased;
            anim->current_y = anim->start_y + (anim->end_y - anim->start_y) * eased;
            break;

        case ANIM_FADE_IN:
        case ANIM_FADE_OUT:
            anim->current_alpha = anim->start_alpha +
                                  (anim->end_alpha - anim->start_alpha) * eased;
            break;

        case ANIM_SCALE:
            anim->current_scale = anim->start_scale +
                                  (anim->end_scale - anim->start_scale) * eased;
            break;

        case ANIM_FLIP:
            /* Rotation from 0 to PI */
            anim->rotation = 3.14159f * eased;
            break;

        case ANIM_SHAKE: {
            /* Random offset that decreases over time */
            float intensity = anim->shake_intensity * (1.0f - t);
            anim->shake_offset_x = ((rand() % 1000) / 500.0f - 1.0f) * intensity;
            anim->shake_offset_y = ((rand() % 1000) / 500.0f - 1.0f) * intensity;
            break;
        }

        case ANIM_PULSE:
            /* Scale oscillates using sine */
            anim->current_scale = 1.0f + 0.1f * sinf(eased * 2.0f * 3.14159f);
            break;

        default:
            break;
    }

    /* Call completion callback */
    if (anim->complete && anim->on_complete) {
        anim->on_complete(anim->target_id, anim->user_data);
    }
}
/* }}} */

/* {{{ anim_update */
void anim_update(float dt) {
    if (g_anim.paused) return;

    dt *= g_anim.time_scale;

    /* Update animations */
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        Animation* anim = &g_anim.animations[i];
        if (anim->type != ANIM_NONE) {
            update_single_animation(anim, dt);

            /* Clean up completed non-looping animations */
            if (anim->complete && !anim->loop) {
                anim->type = ANIM_NONE;
            }
        }
    }

    /* Update particles */
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle* p = &g_anim.particles[i];
        if (p->active) {
            p->life -= dt;
            if (p->life <= 0.0f) {
                p->active = false;
                continue;
            }

            /* Apply velocity */
            p->x += p->vx * dt;
            p->y += p->vy * dt;

            /* Apply gravity */
            p->vy += 200.0f * dt;

            /* Fade out */
            float life_ratio = p->life / p->max_life;
            p->size = 4.0f * life_ratio;
        }
    }
}
/* }}} */

/* {{{ anim_create_move */
int anim_create_move(int target_id, float from_x, float from_y,
                     float to_x, float to_y, float duration, EasingType easing) {
    int handle = find_free_animation();
    if (handle < 0) return -1;

    Animation* anim = &g_anim.animations[handle];
    memset(anim, 0, sizeof(Animation));

    anim->type = ANIM_MOVE;
    anim->easing = easing;
    anim->target_id = target_id;
    anim->duration = duration;
    anim->start_x = from_x;
    anim->start_y = from_y;
    anim->end_x = to_x;
    anim->end_y = to_y;
    anim->current_x = from_x;
    anim->current_y = from_y;

    return handle;
}
/* }}} */

/* {{{ anim_create_fade */
int anim_create_fade(int target_id, bool fade_in, float duration) {
    int handle = find_free_animation();
    if (handle < 0) return -1;

    Animation* anim = &g_anim.animations[handle];
    memset(anim, 0, sizeof(Animation));

    anim->type = fade_in ? ANIM_FADE_IN : ANIM_FADE_OUT;
    anim->easing = EASE_LINEAR;
    anim->target_id = target_id;
    anim->duration = duration;
    anim->start_alpha = fade_in ? 0.0f : 1.0f;
    anim->end_alpha = fade_in ? 1.0f : 0.0f;
    anim->current_alpha = anim->start_alpha;

    return handle;
}
/* }}} */

/* {{{ anim_create_scale */
int anim_create_scale(int target_id, float from_scale, float to_scale,
                      float duration, EasingType easing) {
    int handle = find_free_animation();
    if (handle < 0) return -1;

    Animation* anim = &g_anim.animations[handle];
    memset(anim, 0, sizeof(Animation));

    anim->type = ANIM_SCALE;
    anim->easing = easing;
    anim->target_id = target_id;
    anim->duration = duration;
    anim->start_scale = from_scale;
    anim->end_scale = to_scale;
    anim->current_scale = from_scale;

    return handle;
}
/* }}} */

/* {{{ anim_create_flip */
int anim_create_flip(int target_id, float duration) {
    int handle = find_free_animation();
    if (handle < 0) return -1;

    Animation* anim = &g_anim.animations[handle];
    memset(anim, 0, sizeof(Animation));

    anim->type = ANIM_FLIP;
    anim->easing = EASE_IN_OUT_QUAD;
    anim->target_id = target_id;
    anim->duration = duration;
    anim->rotation = 0.0f;
    anim->current_scale = 1.0f;

    return handle;
}
/* }}} */

/* {{{ anim_create_shake */
int anim_create_shake(int target_id, float intensity, float duration) {
    int handle = find_free_animation();
    if (handle < 0) return -1;

    Animation* anim = &g_anim.animations[handle];
    memset(anim, 0, sizeof(Animation));

    anim->type = ANIM_SHAKE;
    anim->easing = EASE_LINEAR;
    anim->target_id = target_id;
    anim->duration = duration;
    anim->shake_intensity = intensity;

    return handle;
}
/* }}} */

/* {{{ anim_create_pulse */
int anim_create_pulse(int target_id, float duration, bool loop) {
    int handle = find_free_animation();
    if (handle < 0) return -1;

    Animation* anim = &g_anim.animations[handle];
    memset(anim, 0, sizeof(Animation));

    anim->type = ANIM_PULSE;
    anim->easing = EASE_LINEAR;
    anim->target_id = target_id;
    anim->duration = duration;
    anim->loop = loop;
    anim->current_scale = 1.0f;

    return handle;
}
/* }}} */

/* {{{ anim_set_callback */
void anim_set_callback(int handle, void (*callback)(int, void*), void* user_data) {
    if (handle < 0 || handle >= MAX_ANIMATIONS) return;

    g_anim.animations[handle].on_complete = callback;
    g_anim.animations[handle].user_data = user_data;
}
/* }}} */

/* {{{ anim_cancel */
void anim_cancel(int handle) {
    if (handle < 0 || handle >= MAX_ANIMATIONS) return;
    g_anim.animations[handle].type = ANIM_NONE;
}
/* }}} */

/* {{{ anim_cancel_for_target */
void anim_cancel_for_target(int target_id) {
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        if (g_anim.animations[i].target_id == target_id) {
            g_anim.animations[i].type = ANIM_NONE;
        }
    }
}
/* }}} */

/* {{{ anim_is_active */
bool anim_is_active(int handle) {
    if (handle < 0 || handle >= MAX_ANIMATIONS) return false;
    return g_anim.animations[handle].type != ANIM_NONE &&
           !g_anim.animations[handle].complete;
}
/* }}} */

/* {{{ anim_get_position */
bool anim_get_position(int target_id, float* x, float* y) {
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        Animation* anim = &g_anim.animations[i];
        if (anim->target_id == target_id && anim->type != ANIM_NONE) {
            switch (anim->type) {
                case ANIM_MOVE:
                case ANIM_SLIDE_IN:
                case ANIM_SLIDE_OUT:
                    if (x) *x = anim->current_x;
                    if (y) *y = anim->current_y;
                    return true;

                case ANIM_SHAKE:
                    if (x) *x += anim->shake_offset_x;
                    if (y) *y += anim->shake_offset_y;
                    return true;

                default:
                    break;
            }
        }
    }
    return false;
}
/* }}} */

/* {{{ anim_get_alpha */
float anim_get_alpha(int target_id) {
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        Animation* anim = &g_anim.animations[i];
        if (anim->target_id == target_id &&
            (anim->type == ANIM_FADE_IN || anim->type == ANIM_FADE_OUT)) {
            return anim->current_alpha;
        }
    }
    return 1.0f;
}
/* }}} */

/* {{{ anim_get_scale */
float anim_get_scale(int target_id) {
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        Animation* anim = &g_anim.animations[i];
        if (anim->target_id == target_id &&
            (anim->type == ANIM_SCALE || anim->type == ANIM_PULSE)) {
            return anim->current_scale;
        }
    }
    return 1.0f;
}
/* }}} */

/* {{{ anim_has_active */
bool anim_has_active(void) {
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        if (g_anim.animations[i].type != ANIM_NONE &&
            !g_anim.animations[i].complete) {
            return true;
        }
    }
    return false;
}
/* }}} */

/* {{{ anim_spawn_particles */
void anim_spawn_particles(float x, float y, int count, uint32_t color, float spread) {
    for (int i = 0; i < count; i++) {
        /* Find free particle */
        int idx = -1;
        for (int j = 0; j < MAX_PARTICLES; j++) {
            if (!g_anim.particles[j].active) {
                idx = j;
                break;
            }
        }
        if (idx < 0) break;

        Particle* p = &g_anim.particles[idx];
        p->active = true;
        p->x = x;
        p->y = y;

        /* Random velocity in a spread */
        float angle = ((rand() % 1000) / 1000.0f) * 2.0f * 3.14159f;
        float speed = ((rand() % 1000) / 1000.0f) * spread;
        p->vx = cosf(angle) * speed;
        p->vy = sinf(angle) * speed - spread * 0.5f; /* Bias upward */

        p->life = 0.5f + ((rand() % 1000) / 2000.0f);
        p->max_life = p->life;
        p->color = color;
        p->size = 4.0f;
    }
}
/* }}} */

/* {{{ anim_render_particles */
void anim_render_particles(void) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle* p = &g_anim.particles[i];
        if (p->active && p->size > 0.5f) {
            /* Alpha based on remaining life */
            float alpha = p->life / p->max_life;
            draw_set_alpha(alpha);
            draw_circle((int)p->x, (int)p->y, (int)p->size, p->color);
        }
    }
    draw_reset_alpha();
}
/* }}} */
