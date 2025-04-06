#include "effect_system.hpp"
#include "tinyECS/registry.hpp"
#include <iostream>

extern ECSRegistry registry;

EffectSystem::EffectSystem()
{
}

void EffectSystem::step(float elapsed_ms)
{
    for (Entity e : registry.effects.entities) {
        Effect& effect = registry.effects.get(e);

        effect.death_timer_ms -= elapsed_ms;
        if (effect.death_timer_ms <= 0.f) {
            registry.remove_all_components_of(e);
        }
    }
}
