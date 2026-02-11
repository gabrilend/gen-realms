/* 06-combat.c - Combat resolution implementation
 *
 * Handles the combat system including target validation, frontier/interior
 * priority, and base destruction. Combat power is accumulated during main
 * phase and spent on attacks.
 *
 * Attack priority: All frontier bases must be destroyed before interior
 * bases can be targeted. All bases must be destroyed before the player
 * can be attacked directly.
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
 * Respects zone priority: frontier bases first, then interior, then player.
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

        bool has_frontier = deck_has_frontier_bases(opponent->deck);
        bool has_any_bases = deck_total_base_count(opponent->deck) > 0;

        /* Priority: Frontier bases first */
        if (has_frontier) {
            for (int b = 0; b < opponent->deck->frontier_base_count && count < max; b++) {
                CardInstance* base = opponent->deck->frontier_bases[b];
                if (!base || !base->type) {
                    continue;
                }

                out[count].type = TARGET_BASE;
                out[count].player_index = p;
                out[count].base = base;
                out[count].defense_remaining = base->type->defense - base->damage_taken;
                out[count].is_outpost = true;  /* Frontier = must attack first */
                count++;
            }
        }
        /* If no frontier, interior bases are valid */
        else if (has_any_bases) {
            for (int b = 0; b < opponent->deck->interior_base_count && count < max; b++) {
                CardInstance* base = opponent->deck->interior_bases[b];
                if (!base || !base->type) {
                    continue;
                }

                out[count].type = TARGET_BASE;
                out[count].player_index = p;
                out[count].base = base;
                out[count].defense_remaining = base->type->defense - base->damage_taken;
                out[count].is_outpost = false;  /* Interior = protected */
                count++;
            }
        }
        /* If no bases at all, player is valid target */
        else if (count < max) {
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
 * Returns true if the player has any frontier bases in play.
 * NOTE: "outpost" terminology retained for API compatibility, but now
 * refers to frontier bases which must be destroyed first.
 */
bool combat_has_outpost(Game* game, int player_index) {
    if (!game || player_index < 0 || player_index >= game->player_count) {
        return false;
    }

    Player* player = game->players[player_index];
    if (!player || !player->deck) {
        return false;
    }

    return deck_has_frontier_bases(player->deck);
}
/* }}} */

/* {{{ combat_can_attack_player
 * Returns true if the player can be attacked directly.
 * False if they have any bases that must be destroyed first.
 */
bool combat_can_attack_player(Game* game, int player_index) {
    if (!game || player_index < 0 || player_index >= game->player_count) {
        return false;
    }

    Player* player = game->players[player_index];
    if (!player || !player->deck) {
        return false;
    }

    /* Player can only be attacked if they have no bases at all */
    return deck_total_base_count(player->deck) == 0;
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
 * Deals damage to a base. Destroys it if total damage >= defense.
 * Returns false if attack is invalid.
 *
 * Zone priority: Frontier bases must be destroyed before interior bases.
 * Bases now accumulate damage across multiple attacks.
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

    /* Check if base is in frontier zone */
    bool in_frontier = false;
    for (int i = 0; i < defender->deck->frontier_base_count; i++) {
        if (defender->deck->frontier_bases[i] == base) {
            in_frontier = true;
            break;
        }
    }

    /* Check if base is in interior zone */
    bool in_interior = false;
    if (!in_frontier) {
        for (int i = 0; i < defender->deck->interior_base_count; i++) {
            if (defender->deck->interior_bases[i] == base) {
                in_interior = true;
                break;
            }
        }
    }

    /* Base must be in play */
    if (!in_frontier && !in_interior) {
        return false;
    }

    /* Check zone priority: can't attack interior if frontier exists */
    if (in_interior && deck_has_frontier_bases(defender->deck)) {
        return false;  /* Must destroy frontier bases first */
    }

    /* Spend combat */
    attacker->combat -= amount;

    /* Accumulate damage on the base */
    base->damage_taken += amount;

    /* Check if base is destroyed */
    if (base->damage_taken >= base->type->defense) {
        combat_destroy_base(game, player_index, base);
    }

    return true;
}
/* }}} */

/* {{{ combat_destroy_base
 * Removes a base from play and puts it in the owner's discard pile.
 * Resets damage and placement state on the card.
 */
void combat_destroy_base(Game* game, int player_index, CardInstance* base) {
    if (!game || !base || player_index < 0 || player_index >= game->player_count) {
        return;
    }

    Player* owner = game->players[player_index];
    if (!owner || !owner->deck) {
        return;
    }

    /* Remove from bases (deck_remove_base checks both zones) */
    CardInstance* removed = deck_remove_base(owner->deck, base);
    if (removed) {
        /* Reset base state for when it's replayed */
        removed->damage_taken = 0;
        removed->deployed = false;
        removed->placement = ZONE_NONE;

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
 * Returns the remaining defense value of a base (total defense minus damage).
 */
int combat_get_base_defense(CardInstance* base) {
    if (!base || !base->type) {
        return 0;
    }
    int remaining = base->type->defense - base->damage_taken;
    return remaining > 0 ? remaining : 0;
}
/* }}} */
