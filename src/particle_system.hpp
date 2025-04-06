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

    // Simple metrics reporting function
    void printPoolMetrics() const;

private:
    // helper function to create a death particle
    Entity createDeathParticle(vec2 position);

    // Get a particle from the pool or create a new one if pool is empty
    Entity getParticleFromPool(PARTICLE_TYPE type);
    void returnParticleToPool(Entity entity, PARTICLE_TYPE type);
    void resetParticle(Entity entity);

    // track spawned particles by type (for potential optimizations???)
    std::unordered_map<PARTICLE_TYPE, std::vector<Entity>> particlesByType;
    std::unordered_map<PARTICLE_TYPE, std::vector<Entity>> particlePools;

    // Simple metrics counters
    uint32_t total_particles_created = 0;
    uint32_t particles_reused = 0;

    // random number generator for particle variations
    std::default_random_engine rng;
    std::uniform_real_distribution<float> uniform_dist;
};