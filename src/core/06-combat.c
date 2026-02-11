/* 06-combat.c - Combat resolution implementation
 *
 * Handles the combat system including target validation, outpost priority,
 * and base destruction. Combat power is accumulated during main phase and
 * spent on attacks.
 */

/* Enable POSIX functions */
#define _POSIX_C_SOURCE 200809L

#include "06-combat.h"
#include <stdlib.h>

/* ========================================================================== */
/*                           Target Discovery                                 */
/* ========================================================================== */

/* {{{ combat_get_valid_targets
 * Fills the output array with valid combat targets.
 * Returns the number of targets found.
 * Respects outpost priority: if outposts exist, only outposts are valid.
 */
int combat_get_valid_targets(Game* game, CombatTarget* out, int max) {
    if (!game || !out || max <= 0) {
        return 0;
    }

    Player* active = game_get_active_player(game);
    if (!active || active->combat <= 0) {
        return 0;  /* No combat available */
    }

    int count = 0;

    /* Check each opponent */
    for (int p = 0; p < game->player_count && count < max; p++) {
        if (p == game->active_player) {
            continue;  /* Skip self */
        }

        Player* opponent = game->players[p];
        if (!opponent || !player_is_alive(opponent)) {
            continue;
        }

        bool has_outpost = combat_has_outpost(game, p);

        /* Add bases as targets */
        for (int b = 0; b < opponent->deck->base_count && count < max; b++) {
            CardInstance* base = opponent->deck->bases[b];
            if (!base || !base->type) {
                continue;
            }

            bool is_outpost = base->type->is_outpost;

            /* If there are outposts, only outposts are valid */
            if (has_outpost && !is_outpost) {
                continue;
            }

            out[count].type = TARGET_BASE;
            out[count].player_index = p;
            out[count].base = base;
            out[count].defense_remaining = base->type->defense;
            out[count].is_outpost = is_outpost;
            count++;
        }

        /* Add player as target if no outposts */
        if (!has_outpost && count < max) {
            out[count].type = TARGET_PLAYER;
            out[count].player_index = p;
            out[count].base = NULL;
            out[count].defense_remaining = opponent->authority;
            out[count].is_outpost = false;
            count++;
        }
    }

    return count;
}
/* }}} */

/* {{{ combat_has_outpost
 * Returns true if the player has any outpost bases in play.
 */
bool combat_has_outpost(Game* game, int player_index) {
    if (!game || player_index < 0 || player_index >= game->player_count) {
        return false;
    }

    Player* player = game->players[player_index];
    if (!player || !player->deck) {
        return false;
    }

    for (int i = 0; i < player->deck->base_count; i++) {
        CardInstance* base = player->deck->bases[i];
        if (base && base->type && base->type->is_outpost) {
            return true;
        }
    }

    return false;
}
/* }}} */

/* {{{ combat_can_attack_player
 * Returns true if the player can be attacked directly.
 * False if they have outposts that must be destroyed first.
 */
bool combat_can_attack_player(Game* game, int player_index) {
    return !combat_has_outpost(game, player_index);
}
/* }}} */

/* ========================================================================== */
/*                           Combat Execution                                 */
/* ========================================================================== */

/* {{{ combat_attack_player
 * Deals damage to a player's authority.
 * Returns false if attack is invalid (has outposts, insufficient combat).
 */
bool combat_attack_player(Game* game, int player_index, int amount) {
    if (!game || player_index < 0 || player_index >= game->player_count) {
        return false;
    }
    if (player_index == game->active_player) {
        return false;  /* Can't attack self */
    }

    Player* attacker = game_get_active_player(game);
    Player* defender = game->players[player_index];

    if (!attacker || !defender) {
        return false;
    }
    if (amount <= 0 || amount > attacker->combat) {
        return false;  /* Invalid amount */
    }
    if (!combat_can_attack_player(game, player_index)) {
        return false;  /* Has outposts */
    }

    /* Spend combat and deal damage */
    attacker->combat -= amount;
    player_take_damage(defender, amount);

    /* Check for game end */
    if (!player_is_alive(defender)) {
        game->game_over = true;
        game->winner = game->active_player;
        game->phase = PHASE_GAME_OVER;
    }

    return true;
}
/* }}} */

/* {{{ combat_attack_base
 * Deals damage to a base. Destroys it if damage >= defense.
 * Returns false if attack is invalid.
 */
bool combat_attack_base(Game* game, int player_index, CardInstance* base, int amount) {
    if (!game || !base || player_index < 0 || player_index >= game->player_count) {
        return false;
    }
    if (player_index == game->active_player) {
        return false;  /* Can't attack own bases */
    }
    if (!base->type) {
        return false;
    }

    Player* attacker = game_get_active_player(game);
    Player* defender = game->players[player_index];

    if (!attacker || !defender) {
        return false;
    }
    if (amount <= 0 || amount > attacker->combat) {
        return false;
    }

    /* Check if base is actually in play */
    bool found = false;
    for (int i = 0; i < defender->deck->base_count; i++) {
        if (defender->deck->bases[i] == base) {
            found = true;
            break;
        }
    }
    if (!found) {
        return false;
    }

    /* Check outpost priority */
    if (!base->type->is_outpost && combat_has_outpost(game, player_index)) {
        return false;  /* Must destroy outposts first */
    }

    /* Spend combat */
    attacker->combat -= amount;

    /* Check if base is destroyed */
    if (amount >= base->type->defense) {
        combat_destroy_base(game, player_index, base);
    }
    /* Note: Bases don't take partial damage - either destroyed or not */

    return true;
}
/* }}} */

/* {{{ combat_destroy_base
 * Removes a base from play and puts it in the owner's discard pile.
 */
void combat_destroy_base(Game* game, int player_index, CardInstance* base) {
    if (!game || !base || player_index < 0 || player_index >= game->player_count) {
        return;
    }

    Player* owner = game->players[player_index];
    if (!owner || !owner->deck) {
        return;
    }

    /* Remove from bases and add to discard */
    CardInstance* removed = deck_remove_base(owner->deck, base);
    if (removed) {
        deck_add_to_discard(owner->deck, removed);
    }
}
/* }}} */

/* ========================================================================== */
/*                            Combat Queries                                  */
/* ========================================================================== */

/* {{{ combat_get_available
 * Returns the combat power available to the active player.
 */
int combat_get_available(Game* game) {
    if (!game) {
        return 0;
    }

    Player* player = game_get_active_player(game);
    if (!player) {
        return 0;
    }

    return player->combat;
}
/* }}} */

/* {{{ combat_get_base_defense
 * Returns the defense value of a base.
 */
int combat_get_base_defense(CardInstance* base) {
    if (!base || !base->type) {
        return 0;
    }
    return base->type->defense;
}
/* }}} */
