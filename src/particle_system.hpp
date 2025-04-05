#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include <vector>
#include <unordered_map>
#include <random>

class ParticleSystem 
{
public:
    // unitialize the system
    ParticleSystem();

    // update particle positions, velocities, lifetimes, etfcc
    void step(float elapsed_ms);

    // create different types of particles
    void createParticles(PARTICLE_TYPE type, vec2 position, int count);

    Entity createRippleParticle(vec2 position, float lifetime_scale);
    void createPlayerRipples(Entity player_entity);

private:
    // helper function to create a death particle
    Entity createDeathParticle(vec2 position);

    // track spawned particles by type (for potential optimizations???)
    std::unordered_map<PARTICLE_TYPE, std::vector<Entity>> particlesByType;

    // random number generator for particle variations
    std::default_random_engine rng;
    std::uniform_real_distribution<float> uniform_dist;
};