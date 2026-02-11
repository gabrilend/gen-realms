/* 06-combat.h - Combat resolution type definitions
 *
 * Handles damage dealing to players and bases. Combat power accumulates
 * from played cards and can be spent to attack the opponent's authority
 * or destroy their bases. Outposts must be destroyed before attacking
 * the player directly.
 */

#ifndef SYMBELINE_COMBAT_H
#define SYMBELINE_COMBAT_H

#include "05-game.h"
#include <stdbool.h>

/* ========================================================================== */
/*                               Enumerations                                 */
/* ========================================================================== */

/* {{{ TargetType
 * The type of target for a combat action.
 */
typedef enum {
    TARGET_PLAYER,          /* Attack opponent's authority directly */
    TARGET_BASE             /* Attack a specific base */
} TargetType;
/* }}} */

/* ========================================================================== */
/*                                Structures                                  */
/* ========================================================================== */

/* {{{ CombatTarget
 * Describes a valid target for combat damage.
 */
typedef struct {
    TargetType type;
    int player_index;       /* Which player owns this target */
    CardInstance* base;     /* If TARGET_BASE: which base */
    int defense_remaining;  /* For bases: current HP */
    bool is_outpost;        /* For bases: must destroy first */
} CombatTarget;
/* }}} */

/* ========================================================================== */
/*                            Function Prototypes                             */
/* ========================================================================== */

/* {{{ Target discovery */
int combat_get_valid_targets(Game* game, CombatTarget* out, int max);
bool combat_has_outpost(Game* game, int player_index);
bool combat_can_attack_player(Game* game, int player_index);
/* }}} */

/* {{{ Combat execution */
bool combat_attack_player(Game* game, int player_index, int amount);
bool combat_attack_base(Game* game, int player_index, CardInstance* base, int amount);
void combat_destroy_base(Game* game, int player_index, CardInstance* base);
/* }}} */

/* {{{ Combat queries */
int combat_get_available(Game* game);
int combat_get_base_defense(CardInstance* base);
/* }}} */

#endif /* SYMBELINE_COMBAT_H */
