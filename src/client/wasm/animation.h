/*
 * animation.h - Animation System for WASM Client
 *
 * Provides animation primitives for card movement, transitions,
 * and visual effects. Uses easing functions for smooth motion.
 */

#ifndef WASM_ANIMATION_H
#define WASM_ANIMATION_H

#include <stdbool.h>
#include <stdint.h>

/* {{{ Animation limits */
#define MAX_ANIMATIONS 32
#define MAX_PARTICLES 64
/* }}} */

/* {{{ EasingType
 * Easing functions for animation curves.
 */
typedef enum {
    EASE_LINEAR,
    EASE_IN_QUAD,
    EASE_OUT_QUAD,
    EASE_IN_OUT_QUAD,
    EASE_IN_CUBIC,
    EASE_OUT_CUBIC,
    EASE_IN_OUT_CUBIC,
    EASE_OUT_BACK,       /* Overshoot effect */
    EASE_OUT_BOUNCE      /* Bounce effect */
} EasingType;
/* }}} */

/* {{{ AnimationType
 * Types of animations supported.
 */
typedef enum {
    ANIM_NONE,
    ANIM_MOVE,           /* Move from A to B */
    ANIM_FADE_IN,        /* Fade in (alpha 0->1) */
    ANIM_FADE_OUT,       /* Fade out (alpha 1->0) */
    ANIM_SCALE,          /* Scale up/down */
    ANIM_FLIP,           /* Card flip (3D-ish) */
    ANIM_SHAKE,          /* Shake/vibrate */
    ANIM_PULSE,          /* Pulse/glow effect */
    ANIM_SLIDE_IN,       /* Slide from edge */
    ANIM_SLIDE_OUT       /* Slide to edge */
} AnimationType;
/* }}} */

/* {{{ Animation
 * Single animation instance.
 */
typedef struct {
    AnimationType type;
    EasingType easing;

    /* Target identification */
    int target_id;       /* Card/element ID being animated */
    int target_zone;     /* Zone the target is in */

    /* Timing */
    float duration;      /* Total duration in seconds */
    float elapsed;       /* Time elapsed */
    bool complete;       /* Animation finished */
    bool loop;           /* Loop animation */

    /* Position animation (ANIM_MOVE, ANIM_SLIDE_*) */
    float start_x, start_y;
    float end_x, end_y;
    float current_x, current_y;

    /* Alpha animation (ANIM_FADE_*) */
    float start_alpha;
    float end_alpha;
    float current_alpha;

    /* Scale animation (ANIM_SCALE) */
    float start_scale;
    float end_scale;
    float current_scale;

    /* Rotation/flip (ANIM_FLIP) */
    float rotation;      /* Current rotation (radians) */

    /* Shake effect (ANIM_SHAKE) */
    float shake_intensity;
    float shake_offset_x;
    float shake_offset_y;

    /* Callback when complete */
    void (*on_complete)(int target_id, void* user_data);
    void* user_data;
} Animation;
/* }}} */

/* {{{ Particle
 * Simple particle for effects.
 */
typedef struct {
    bool active;
    float x, y;
    float vx, vy;        /* Velocity */
    float life;          /* Remaining lifetime */
    float max_life;
    uint32_t color;
    float size;
} Particle;
/* }}} */

/* {{{ AnimationSystem
 * Global animation manager.
 */
typedef struct {
    Animation animations[MAX_ANIMATIONS];
    int animation_count;

    Particle particles[MAX_PARTICLES];
    int particle_count;

    float time_scale;    /* Speed multiplier (1.0 = normal) */
    bool paused;
} AnimationSystem;
/* }}} */

/* {{{ anim_init
 * Initialize the animation system.
 */
void anim_init(void);
/* }}} */

/* {{{ anim_cleanup
 * Clean up animation system.
 */
void anim_cleanup(void);
/* }}} */

/* {{{ anim_update
 * Update all animations by delta time.
 * @param dt - Delta time in seconds
 */
void anim_update(float dt);
/* }}} */

/* {{{ anim_create_move
 * Create a movement animation.
 * @param target_id - ID of target element
 * @param from_x, from_y - Starting position
 * @param to_x, to_y - Ending position
 * @param duration - Duration in seconds
 * @param easing - Easing function
 * @return Animation handle, or -1 on failure
 */
int anim_create_move(int target_id, float from_x, float from_y,
                     float to_x, float to_y, float duration, EasingType easing);
/* }}} */

/* {{{ anim_create_fade
 * Create a fade animation.
 * @param target_id - ID of target element
 * @param fade_in - true for fade in, false for fade out
 * @param duration - Duration in seconds
 * @return Animation handle, or -1 on failure
 */
int anim_create_fade(int target_id, bool fade_in, float duration);
/* }}} */

/* {{{ anim_create_scale
 * Create a scale animation.
 * @param target_id - ID of target element
 * @param from_scale - Starting scale (1.0 = normal)
 * @param to_scale - Ending scale
 * @param duration - Duration in seconds
 * @param easing - Easing function
 * @return Animation handle, or -1 on failure
 */
int anim_create_scale(int target_id, float from_scale, float to_scale,
                      float duration, EasingType easing);
/* }}} */

/* {{{ anim_create_flip
 * Create a card flip animation.
 * @param target_id - ID of target card
 * @param duration - Duration in seconds
 * @return Animation handle, or -1 on failure
 */
int anim_create_flip(int target_id, float duration);
/* }}} */

/* {{{ anim_create_shake
 * Create a shake animation.
 * @param target_id - ID of target element
 * @param intensity - Shake intensity in pixels
 * @param duration - Duration in seconds
 * @return Animation handle, or -1 on failure
 */
int anim_create_shake(int target_id, float intensity, float duration);
/* }}} */

/* {{{ anim_create_pulse
 * Create a pulse/glow animation.
 * @param target_id - ID of target element
 * @param duration - Duration of one pulse cycle
 * @param loop - Whether to loop
 * @return Animation handle, or -1 on failure
 */
int anim_create_pulse(int target_id, float duration, bool loop);
/* }}} */

/* {{{ anim_set_callback
 * Set completion callback for an animation.
 * @param handle - Animation handle
 * @param callback - Function to call on completion
 * @param user_data - User data to pass to callback
 */
void anim_set_callback(int handle, void (*callback)(int, void*), void* user_data);
/* }}} */

/* {{{ anim_cancel
 * Cancel an animation.
 * @param handle - Animation handle
 */
void anim_cancel(int handle);
/* }}} */

/* {{{ anim_cancel_for_target
 * Cancel all animations for a target.
 * @param target_id - Target element ID
 */
void anim_cancel_for_target(int target_id);
/* }}} */

/* {{{ anim_is_active
 * Check if an animation is still running.
 * @param handle - Animation handle
 * @return true if animation is active
 */
bool anim_is_active(int handle);
/* }}} */

/* {{{ anim_get_position
 * Get current animated position for a target.
 * @param target_id - Target element ID
 * @param x, y - Output position (unchanged if no animation)
 * @return true if position was modified by animation
 */
bool anim_get_position(int target_id, float* x, float* y);
/* }}} */

/* {{{ anim_get_alpha
 * Get current animated alpha for a target.
 * @param target_id - Target element ID
 * @return Alpha value (1.0 if no animation)
 */
float anim_get_alpha(int target_id);
/* }}} */

/* {{{ anim_get_scale
 * Get current animated scale for a target.
 * @param target_id - Target element ID
 * @return Scale value (1.0 if no animation)
 */
float anim_get_scale(int target_id);
/* }}} */

/* {{{ anim_has_active
 * Check if any animations are currently active.
 * @return true if animations are running
 */
bool anim_has_active(void);
/* }}} */

/* {{{ anim_spawn_particles
 * Spawn particles at a position.
 * @param x, y - Spawn position
 * @param count - Number of particles
 * @param color - Particle color
 * @param spread - Velocity spread
 */
void anim_spawn_particles(float x, float y, int count, uint32_t color, float spread);
/* }}} */

/* {{{ anim_render_particles
 * Render all active particles.
 */
void anim_render_particles(void);
/* }}} */

/* {{{ Easing function utilities */
float ease_apply(EasingType type, float t);
/* }}} */

#endif /* WASM_ANIMATION_H */
