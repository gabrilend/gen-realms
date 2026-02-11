/* 01-card.c - Card and Effect implementation
 *
 * Implements memory management and utility functions for the card system.
 * Cards are the fundamental unit of gameplay - this module handles their
 * creation, modification (upgrades), and cleanup.
 */

/* Enable POSIX functions like strdup */
#define _POSIX_C_SOURCE 200809L

#include "01-card.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* Counter for generating unique instance IDs */
static uint32_t s_instance_counter = 0;
static bool s_rng_initialized = false;

/* ========================================================================== */
/*                              Effect Functions                              */
/* ========================================================================== */

/* {{{ effect_create
 * Allocates and initializes a single Effect. Caller owns the returned memory.
 */
Effect* effect_create(EffectType type, int value, const char* target) {
    Effect* effect = malloc(sizeof(Effect));
    if (!effect) {
        return NULL;
    }

    effect->type = type;
    effect->value = value;

    if (target) {
        effect->target_card_id = strdup(target);
    } else {
        effect->target_card_id = NULL;
    }

    return effect;
}
/* }}} */

/* {{{ effect_free
 * Frees an Effect and its associated strings.
 */
void effect_free(Effect* effect) {
    if (!effect) {
        return;
    }
    free(effect->target_card_id);
    free(effect);
}
/* }}} */

/* {{{ effect_array_create
 * Allocates an array of Effects. Returns NULL on failure.
 */
Effect* effect_array_create(int count) {
    if (count <= 0) {
        return NULL;
    }
    Effect* effects = calloc(count, sizeof(Effect));
    return effects;
}
/* }}} */

/* {{{ effect_array_free
 * Frees an array of Effects and their associated strings.
 */
void effect_array_free(Effect* effects, int count) {
    if (!effects) {
        return;
    }
    for (int i = 0; i < count; i++) {
        free(effects[i].target_card_id);
    }
    free(effects);
}
/* }}} */

/* ========================================================================== */
/*                             CardType Functions                             */
/* ========================================================================== */

/* {{{ card_type_create
 * Allocates and initializes a CardType with required fields.
 * Effects arrays start empty and must be added separately.
 */
CardType* card_type_create(const char* id, const char* name, int cost,
                           Faction faction, CardKind kind) {
    if (!id || !name) {
        return NULL;
    }

    CardType* type = calloc(1, sizeof(CardType));
    if (!type) {
        return NULL;
    }

    type->id = strdup(id);
    type->name = strdup(name);
    type->cost = cost;
    type->faction = faction;
    type->kind = kind;

    /* Defaults for optional fields */
    type->flavor = NULL;
    type->defense = 0;
    type->is_outpost = false;
    type->effects = NULL;
    type->effect_count = 0;
    type->ally_effects = NULL;
    type->ally_effect_count = 0;
    type->scrap_effects = NULL;
    type->scrap_effect_count = 0;
    type->spawns_id = NULL;

    return type;
}
/* }}} */

/* {{{ card_type_free
 * Frees a CardType and all associated memory.
 */
void card_type_free(CardType* type) {
    if (!type) {
        return;
    }

    free(type->id);
    free(type->name);
    free(type->flavor);
    free(type->spawns_id);

    effect_array_free(type->effects, type->effect_count);
    effect_array_free(type->ally_effects, type->ally_effect_count);
    effect_array_free(type->scrap_effects, type->scrap_effect_count);

    free(type);
}
/* }}} */

/* {{{ card_type_set_flavor
 * Sets the flavor text for a card type.
 */
void card_type_set_flavor(CardType* type, const char* flavor) {
    if (!type) {
        return;
    }
    free(type->flavor);
    type->flavor = flavor ? strdup(flavor) : NULL;
}
/* }}} */

/* {{{ card_type_set_base_stats
 * Sets defense and outpost flag for base-type cards.
 */
void card_type_set_base_stats(CardType* type, int defense, bool is_outpost) {
    if (!type) {
        return;
    }
    type->defense = defense;
    type->is_outpost = is_outpost;
}
/* }}} */

/* {{{ card_type_set_spawns
 * Sets the card ID that this base spawns.
 */
void card_type_set_spawns(CardType* type, const char* spawns_id) {
    if (!type) {
        return;
    }
    free(type->spawns_id);
    type->spawns_id = spawns_id ? strdup(spawns_id) : NULL;
}
/* }}} */

/* ========================================================================== */
/*                           CardInstance Functions                           */
/* ========================================================================== */

/* {{{ card_instance_generate_id
 * Generates a unique instance ID. Format: "inst_XXXXXXXX" where X is hex.
 * Uses a counter combined with random bits for uniqueness.
 */
char* card_instance_generate_id(void) {
    if (!s_rng_initialized) {
        srand((unsigned int)time(NULL));
        s_rng_initialized = true;
    }

    /* Combine counter with random bits for collision resistance */
    uint32_t id_num = (s_instance_counter++ << 16) | (rand() & 0xFFFF);

    char* id = malloc(16);  /* "inst_" + 8 hex chars + null */
    if (id) {
        snprintf(id, 16, "inst_%08x", id_num);
    }
    return id;
}
/* }}} */

/* {{{ card_instance_create
 * Creates a new instance of a card type. Each instance has a unique ID
 * and can be independently upgraded.
 */
CardInstance* card_instance_create(CardType* type) {
    if (!type) {
        return NULL;
    }

    CardInstance* inst = calloc(1, sizeof(CardInstance));
    if (!inst) {
        return NULL;
    }

    inst->type = type;
    inst->instance_id = card_instance_generate_id();

    /* No upgrades initially */
    inst->attack_bonus = 0;
    inst->trade_bonus = 0;
    inst->authority_bonus = 0;

    /* Generate random seed for art */
    if (!s_rng_initialized) {
        srand((unsigned int)time(NULL));
        s_rng_initialized = true;
    }
    inst->image_seed = (uint32_t)rand();
    inst->needs_regen = true;  /* New cards need initial generation */

    inst->draw_effect_spent = false;

    return inst;
}
/* }}} */

/* {{{ card_instance_free
 * Frees a card instance. Does NOT free the underlying CardType.
 */
void card_instance_free(CardInstance* instance) {
    if (!instance) {
        return;
    }
    free(instance->instance_id);
    free(instance);
}
/* }}} */

/* {{{ card_instance_apply_upgrade
 * Applies a permanent upgrade to a card instance.
 * Only EFFECT_UPGRADE_* types are valid here.
 */
void card_instance_apply_upgrade(CardInstance* inst, EffectType upgrade_type,
                                  int value) {
    if (!inst) {
        return;
    }

    switch (upgrade_type) {
        case EFFECT_UPGRADE_ATTACK:
            inst->attack_bonus += value;
            inst->needs_regen = true;  /* Visual change needed */
            break;
        case EFFECT_UPGRADE_TRADE:
            inst->trade_bonus += value;
            inst->needs_regen = true;
            break;
        case EFFECT_UPGRADE_AUTH:
            inst->authority_bonus += value;
            inst->needs_regen = true;
            break;
        default:
            /* Not an upgrade effect, ignore */
            break;
    }
}
/* }}} */

/* {{{ card_instance_total_combat
 * Returns total combat value: base effects + upgrade bonus.
 * Scans the card's effects for EFFECT_COMBAT and sums with bonus.
 */
int card_instance_total_combat(CardInstance* inst) {
    if (!inst || !inst->type) {
        return 0;
    }

    int total = inst->attack_bonus;

    for (int i = 0; i < inst->type->effect_count; i++) {
        if (inst->type->effects[i].type == EFFECT_COMBAT) {
            total += inst->type->effects[i].value;
        }
    }

    return total;
}
/* }}} */

/* {{{ card_instance_total_trade
 * Returns total trade value: base effects + upgrade bonus.
 */
int card_instance_total_trade(CardInstance* inst) {
    if (!inst || !inst->type) {
        return 0;
    }

    int total = inst->trade_bonus;

    for (int i = 0; i < inst->type->effect_count; i++) {
        if (inst->type->effects[i].type == EFFECT_TRADE) {
            total += inst->type->effects[i].value;
        }
    }

    return total;
}
/* }}} */

/* {{{ card_instance_total_authority
 * Returns total authority gain: base effects + upgrade bonus.
 */
int card_instance_total_authority(CardInstance* inst) {
    if (!inst || !inst->type) {
        return 0;
    }

    int total = inst->authority_bonus;

    for (int i = 0; i < inst->type->effect_count; i++) {
        if (inst->type->effects[i].type == EFFECT_AUTHORITY) {
            total += inst->type->effects[i].value;
        }
    }

    return total;
}
/* }}} */

/* {{{ card_instance_mark_for_regen
 * Marks this card instance for art regeneration on next shuffle.
 */
void card_instance_mark_for_regen(CardInstance* inst) {
    if (inst) {
        inst->needs_regen = true;
    }
}
/* }}} */

/* {{{ card_instance_clear_regen
 * Clears regeneration flag after new art has been generated.
 */
void card_instance_clear_regen(CardInstance* inst) {
    if (inst) {
        inst->needs_regen = false;
    }
}
/* }}} */

/* ========================================================================== */
/*                             Utility Functions                              */
/* ========================================================================== */

/* {{{ faction_to_string
 * Returns human-readable faction name.
 */
const char* faction_to_string(Faction faction) {
    switch (faction) {
        case FACTION_NEUTRAL:   return "Neutral";
        case FACTION_MERCHANT:  return "Merchant Guilds";
        case FACTION_WILDS:     return "The Wilds";
        case FACTION_KINGDOM:   return "High Kingdom";
        case FACTION_ARTIFICER: return "Artificer Order";
        default:                return "Unknown";
    }
}
/* }}} */

/* {{{ card_kind_to_string
 * Returns human-readable card kind name.
 */
const char* card_kind_to_string(CardKind kind) {
    switch (kind) {
        case CARD_KIND_SHIP: return "Ship";
        case CARD_KIND_BASE: return "Base";
        case CARD_KIND_UNIT: return "Unit";
        default:             return "Unknown";
    }
}
/* }}} */

/* {{{ effect_type_to_string
 * Returns human-readable effect type name.
 */
const char* effect_type_to_string(EffectType type) {
    switch (type) {
        case EFFECT_TRADE:          return "Trade";
        case EFFECT_COMBAT:         return "Combat";
        case EFFECT_AUTHORITY:      return "Authority";
        case EFFECT_DRAW:           return "Draw";
        case EFFECT_DISCARD:        return "Discard";
        case EFFECT_SCRAP_TRADE_ROW: return "Scrap Trade Row";
        case EFFECT_SCRAP_HAND:     return "Scrap Hand/Discard";
        case EFFECT_TOP_DECK:       return "Top Deck";
        case EFFECT_D10_UP:         return "D10 Up";
        case EFFECT_D10_DOWN:       return "D10 Down";
        case EFFECT_DESTROY_BASE:   return "Destroy Base";
        case EFFECT_COPY_SHIP:      return "Copy Ship";
        case EFFECT_ACQUIRE_FREE:   return "Acquire Free";
        case EFFECT_UPGRADE_ATTACK: return "Upgrade Attack";
        case EFFECT_UPGRADE_TRADE:  return "Upgrade Trade";
        case EFFECT_UPGRADE_AUTH:   return "Upgrade Authority";
        case EFFECT_SPAWN:          return "Spawn";
        default:                    return "Unknown";
    }
}
/* }}} */
