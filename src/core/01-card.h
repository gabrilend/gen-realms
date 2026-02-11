/* 01-card.h - Card and Effect type definitions
 *
 * Defines the foundational data structures for representing cards in
 * Symbeline Realms. Cards have a base type (CardType) shared across all
 * copies, and individual instances (CardInstance) that track upgrades
 * and per-copy state like art seeds.
 */

#ifndef SYMBELINE_CARD_H
#define SYMBELINE_CARD_H

#include <stdbool.h>
#include <stdint.h>

/* ========================================================================== */
/*                                 Enumerations                               */
/* ========================================================================== */

/* {{{ Faction
 * The five factions plus neutral. Cards belong to exactly one faction.
 * Ally abilities trigger when another card of the same faction is played.
 */
typedef enum {
    FACTION_NEUTRAL = 0,
    FACTION_MERCHANT,    /* Trade-focused, commerce and wealth */
    FACTION_WILDS,       /* Aggressive, swarming and beasts */
    FACTION_KINGDOM,     /* Defensive, authority gain and disruption */
    FACTION_ARTIFICER,   /* Scrap-focused, upgrades and manipulation */
    FACTION_COUNT        /* Sentinel for array sizing */
} Faction;
/* }}} */

/* {{{ CardKind
 * The three card kinds. Ships are played and discarded at end of turn.
 * Bases persist until destroyed. Units are spawned by bases.
 */
typedef enum {
    CARD_KIND_SHIP = 0,  /* Standard card, discarded at end of turn */
    CARD_KIND_BASE,      /* Persists in play until destroyed */
    CARD_KIND_UNIT       /* Spawned by bases, acts like ships */
} CardKind;
/* }}} */

/* {{{ BasePlacement
 * Where a base is placed when played. Replaces the outpost concept.
 * Frontier bases must all be destroyed before interior can be targeted.
 * This is chosen at play time, not defined on the card type.
 */
typedef enum {
    ZONE_NONE = 0,       /* Not a base / not yet placed */
    ZONE_FRONTIER,       /* Exposed position, attacked first */
    ZONE_INTERIOR        /* Protected position, attacked after frontier */
} BasePlacement;
/* }}} */

/* {{{ EffectType
 * All possible effect types. Effects are triggered when cards are played,
 * when ally conditions are met, or when cards are scrapped.
 */
typedef enum {
    /* Resource effects */
    EFFECT_TRADE = 0,       /* Add trade (coins) for this turn */
    EFFECT_COMBAT,          /* Add combat (attack) for this turn */
    EFFECT_AUTHORITY,       /* Gain authority (health) */

    /* Card manipulation */
    EFFECT_DRAW,            /* Draw cards from deck */
    EFFECT_DISCARD,         /* Force opponent to discard */
    EFFECT_SCRAP_TRADE_ROW, /* Scrap a card from trade row */
    EFFECT_SCRAP_HAND,      /* Scrap a card from own hand/discard */
    EFFECT_TOP_DECK,        /* Put a card from discard on top of deck */

    /* Deck flow effects */
    EFFECT_D10_UP,          /* Increment d10 (buy momentum) */
    EFFECT_D10_DOWN,        /* Decrement d10 (scrap momentum) */

    /* Special effects */
    EFFECT_DESTROY_BASE,    /* Target and destroy an opponent's base */
    EFFECT_COPY_SHIP,       /* Copy another ship's effects */
    EFFECT_ACQUIRE_FREE,    /* Acquire a card without paying */
    EFFECT_ACQUIRE_TOP,     /* Next acquired card goes to deck top */

    /* Upgrade effects (permanent modifications to cards) */
    EFFECT_UPGRADE_ATTACK,  /* Permanent +attack to a card */
    EFFECT_UPGRADE_TRADE,   /* Permanent +trade to a card */
    EFFECT_UPGRADE_AUTH,    /* Permanent +authority to a card */

    /* Spawning effects */
    EFFECT_SPAWN,           /* Spawn a unit into discard pile */

    EFFECT_TYPE_COUNT       /* Sentinel for validation */
} EffectType;
/* }}} */

/* ========================================================================== */
/*                                  Structures                                */
/* ========================================================================== */

/* {{{ Effect
 * A single effect that a card can produce. Effects have a type and a
 * numeric value (interpretation depends on type). Some effects target
 * cards, tracked by target_card_id when relevant.
 */
typedef struct {
    EffectType type;
    int value;              /* Amount for numeric effects */
    char* target_card_id;   /* For spawn/upgrade: which card type */
} Effect;
/* }}} */

/* {{{ CardType
 * The immutable definition of a card. All copies of "Dire Bear" point to
 * the same CardType. This struct is loaded from JSON and never modified
 * during gameplay.
 */
typedef struct {
    /* Identity */
    char* id;               /* Unique identifier, e.g., "dire_bear" */
    char* name;             /* Display name, e.g., "Dire Bear" */
    char* flavor;           /* Flavor text for display */

    /* Classification */
    int cost;               /* Trade required to buy from trade row */
    Faction faction;        /* Which faction this card belongs to */
    CardKind kind;          /* Ship, base, or unit */

    /* Base stats (for bases) */
    int defense;            /* Damage required to destroy (bases only) */
    bool is_outpost;        /* Must be destroyed before attacking player */

    /* Effects - triggered when card is played */
    Effect* effects;
    int effect_count;

    /* Ally effects - triggered if same-faction card already in play */
    Effect* ally_effects;
    int ally_effect_count;

    /* Scrap effects - triggered when card is removed from game */
    Effect* scrap_effects;
    int scrap_effect_count;

    /* Spawning (for bases that create units) */
    char* spawns_id;        /* Card type ID this base spawns, or NULL */
} CardType;
/* }}} */

/* {{{ CardInstance
 * A specific copy of a card in play. Each card drawn is a unique instance
 * that can be upgraded, has its own art seed, and tracks regeneration state.
 */
typedef struct {
    CardType* type;         /* Pointer to shared card definition */
    char* instance_id;      /* Unique ID for this specific copy */

    /* Permanent upgrades (applied by blacksmith, enchanter, etc.) */
    int attack_bonus;       /* Permanent +combat when played */
    int trade_bonus;        /* Permanent +trade when played */
    int authority_bonus;    /* Permanent +authority when played */

    /* Visual generation state */
    uint32_t image_seed;    /* Seed for reproducible art generation */
    bool needs_regen;       /* True if art should regenerate on shuffle */

    /* Runtime state */
    bool draw_effect_spent; /* True if auto-draw already triggered */

    /* Base-specific state (only used for CARD_KIND_BASE) */
    BasePlacement placement; /* Frontier or interior zone */
    bool deployed;           /* True after first full turn (effects active) */
    int damage_taken;        /* Damage accumulated (destroyed when >= defense) */
} CardInstance;
/* }}} */

/* ========================================================================== */
/*                              Function Prototypes                           */
/* ========================================================================== */

/* {{{ Effect functions */
Effect* effect_create(EffectType type, int value, const char* target);
void effect_free(Effect* effect);
Effect* effect_array_create(int count);
void effect_array_free(Effect* effects, int count);
/* }}} */

/* {{{ CardType functions */
CardType* card_type_create(const char* id, const char* name, int cost,
                           Faction faction, CardKind kind);
void card_type_free(CardType* type);
void card_type_set_flavor(CardType* type, const char* flavor);
void card_type_set_base_stats(CardType* type, int defense, bool is_outpost);
void card_type_set_spawns(CardType* type, const char* spawns_id);
/* }}} */

/* {{{ CardInstance functions */
CardInstance* card_instance_create(CardType* type);
void card_instance_free(CardInstance* instance);
char* card_instance_generate_id(void);
void card_instance_apply_upgrade(CardInstance* inst, EffectType upgrade_type,
                                  int value);
int card_instance_total_combat(CardInstance* inst);
int card_instance_total_trade(CardInstance* inst);
int card_instance_total_authority(CardInstance* inst);
void card_instance_mark_for_regen(CardInstance* inst);
void card_instance_clear_regen(CardInstance* inst);
/* }}} */

/* {{{ Utility functions */
const char* faction_to_string(Faction faction);
const char* card_kind_to_string(CardKind kind);
const char* effect_type_to_string(EffectType type);
const char* base_placement_to_string(BasePlacement placement);
const char* base_placement_art_modifier(BasePlacement placement);
/* }}} */

#endif /* SYMBELINE_CARD_H */
