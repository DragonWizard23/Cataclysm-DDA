#pragma once
#ifndef CATA_SRC_WEAKPOINT_H
#define CATA_SRC_WEAKPOINT_H

#include <array>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "damage.h"
#include "optional.h"
#include "type_id.h"

class Character;
class Creature;
class JsonArray;
class JsonObject;
class JsonValue;

// Information about an attack on a weak point.
struct weakpoint_attack {
    enum class attack_type : int {
        NONE, // Unusual damage instances, such as falls, spells, and effects.
        MELEE_BASH, // Melee bludgeoning attacks
        MELEE_CUT, // Melee slashing attacks
        MELEE_STAB, // Melee piercing attacks
        PROJECTILE, // Ranged projectile attacks, including throwing weapons and guns

        NUM,
    };

    // The source of the attack.
    const Creature *source;
    // The target of the attack.
    const Creature *target;
    // The weapon used to make the attack.
    const item *weapon;
    // The type of the attack.
    attack_type type;
    // Whether the attack from a thrown object.
    bool is_thrown;
    // Whether the attack a critical hit.
    bool is_crit;
    // The Creature's skill in hitting weak points.
    float wp_skill;

    weakpoint_attack();
    // Returns the attack type of a melee attack.
    static attack_type type_of_melee_attack( const damage_instance &damage );
    // Compute and set the value of `wp_skill`.
    void compute_wp_skill();
};

// An effect that a weakpoint can cause.
struct weakpoint_effect {
    // The type of the effect.
    efftype_id effect;
    // The percent chance of causing the effect.
    float chance;
    // Whether the effect is permanent.
    bool permanent;
    // The range of the durations (in turns) of the effect.
    std::pair<int, int> duration;
    // The range of the intensities of the effect.
    std::pair<int, int> intensity;
    // The range of damage, as a percentage of max health, required to the effect.
    std::pair<float, float> damage_required;
    // The message to print, if the player causes the effect.
    std::string message;

    weakpoint_effect();
    // Maybe apply an effect to the target.
    void apply_to( Creature &target, int total_damage, const weakpoint_attack &attack ) const;
    void load( const JsonObject &jo );
};

struct weakpoint_difficulty {
    std::array<float, static_cast<int>( weakpoint_attack::attack_type::NUM )> difficulty;

    explicit weakpoint_difficulty( float default_value );
    float of( const weakpoint_attack &attack ) const;
    void load( const JsonObject &jo );
};

struct weakpoint_family {
    // ID of the family. Equal to the proficiency, if not provided.
    std::string id;
    // Name of proficiency corresponding to the family.
    proficiency_id proficiency;
    // The skill bonus for having the proficiency.
    cata::optional<float> bonus;
    // The skill penalty for not having the proficiency.
    cata::optional<float> penalty;

    float modifier( const Character &attacker ) const;
    void load( const JsonValue &jsin );
};

struct weakpoint_families {
    // List of weakpoint families
    std::vector<weakpoint_family> families;

    // Practice all weak point families for the given duration. Returns true if a proficiency was learned.
    bool practice( Character &learner, const time_duration &amount ) const;
    bool practice_hit( Character &learner ) const;
    bool practice_kill( Character &learner ) const;
    bool practice_dissect( Character &learner ) const;
    float modifier( const Character &attacker ) const;

    void clear();
    void load( const JsonArray &ja );
    void remove( const JsonArray &ja );
};

struct weakpoint {
    // ID of the weakpoint. Equal to the name, if not provided.
    std::string id;
    // Name of the weakpoint. Can be empty.
    std::string name;
    // Percent chance of hitting the weakpoint. Can be increased by skill.
    float coverage = 100.0f;
    // Multiplier for existing armor values. Defaults to 1.
    std::array<float, static_cast<int>( damage_type::NUM )> armor_mult;
    // Flat penalty to armor values. Applied after the multiplier.
    std::array<float, static_cast<int>( damage_type::NUM )> armor_penalty;
    // Damage multipliers. Applied after armor.
    std::array<float, static_cast<int>( damage_type::NUM )> damage_mult;
    // Critical damage multiplers. Applied after armor instead of damage_mult, if the attack is a crit.
    std::array<float, static_cast<int>( damage_type::NUM )>crit_mult;
    // A list of required effects.
    std::vector<efftype_id> required_effects;
    // A list of effects that may trigger by hitting this weak point.
    std::vector<weakpoint_effect> effects;
    // Constant coverage multipliers, depending on the attack type.
    weakpoint_difficulty coverage_mult;
    // Difficulty gates, varying by the attack type.
    weakpoint_difficulty difficulty;

    weakpoint();
    // Apply the armor multipliers and offsets to a set of resistances.
    void apply_to( resistances &resistances ) const;
    // Apply the damage multiplers to a set of damage values.
    void apply_to( damage_instance &damage, bool is_crit ) const;
    void apply_effects( Creature &target, int total_damage, const weakpoint_attack &attack ) const;
    // Return the change of the creature hitting the weakpoint.
    float hit_chance( const weakpoint_attack &attack ) const;
    void load( const JsonObject &jo );
};

struct weakpoints {
    // List of weakpoints. Each weakpoint should have a unique id.
    std::vector<weakpoint> weakpoint_list;
    // Default weakpoint to return.
    weakpoint default_weakpoint;

    // Selects a weakpoint to hit.
    const weakpoint *select_weakpoint( const weakpoint_attack &attack ) const;

    void clear();
    void load( const JsonArray &ja );
    void remove( const JsonArray &ja );
};

#endif // CATA_SRC_WEAKPOINT_H
