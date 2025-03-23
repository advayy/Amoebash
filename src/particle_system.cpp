#include "particle_system.hpp"
#include "tinyECS/registry.hpp"
#include <iostream>
#include <random>

// defined in registry.cpp
extern ECSRegistry registry;

ParticleSystem::ParticleSystem()
{
    // creatge random number generator
    std::random_device rd;
    rng = std::default_random_engine(rd());
    uniform_dist = std::uniform_real_distribution<float>(0.0f, 1.0f);
}

void ParticleSystem::step(float elapsed_ms)
{
    // Iterate backwards to safely remove expired particles.
    for (int i = (int)registry.particles.size() - 1; i >= 0; i--)
    {
        Entity entity = registry.particles.entities[i];
        Particle& particle = registry.particles.components[i];

        // update lifetime
        particle.lifetime_ms -= elapsed_ms;

        // Remove expired particles
        if (particle.lifetime_ms <= 0)
        {
            registry.remove_all_components_of(entity);
            continue;
        }

        // update particle behavior based on type
        switch (particle.type)
        {
            case PARTICLE_TYPE::DEATH_PARTICLE:
            {
                Motion& motion = registry.motions.get(entity);

                if (particle.state == PARTICLE_STATE::BURST)
                {
                    motion.velocity *= 0.98f;
                    particle.state_timer_ms -= elapsed_ms;
                    if (particle.state_timer_ms <= 0)
                    {
                        particle.state = PARTICLE_STATE::FOLLOW;
                        particle.state_timer_ms = 1000.f;
                    }
                }
                else if (particle.state == PARTICLE_STATE::FOLLOW)
                {
                    if (!registry.players.entities.empty())
                    {
                        Entity player_entity = registry.players.entities[0];
                        Motion& player_motion = registry.motions.get(player_entity);
                        vec2 direction = player_motion.position - motion.position;
                        float distance = glm::length(direction);
                        if (distance > 0.1f)
                        {
                            direction = glm::normalize(direction);
                            float speed_factor = particle.speed_factor * (1.0f + (1.0f - particle.lifetime_ms / particle.max_lifetime_ms) * 200.0f);
                            motion.velocity += direction * speed_factor * (elapsed_ms / 1000.f);
                            float max_speed = 600.0f;
                            float current_speed = glm::length(motion.velocity);
                            if (current_speed > max_speed)
                            {
                                motion.velocity = glm::normalize(motion.velocity) * max_speed;
                            }
                        }
                    }
                    float life_ratio = particle.lifetime_ms / particle.max_lifetime_ms;
                    if (registry.colors.has(entity))
                    {
                        vec3& color = registry.colors.get(entity);
                        color = color * life_ratio * 2.0f;
                    }
                }
                break;
            }
            default:
                break;
        }
    }
}

void ParticleSystem::createParticles(PARTICLE_TYPE type, vec2 position, int count)
{
    switch (type)
    {
        case PARTICLE_TYPE::DEATH_PARTICLE:
            for (int i = 0; i < count; i++)
            {
                Entity particle = createDeathParticle(position);
                particlesByType[type].push_back(particle);
            }
            break;
            /// add more particles type heree

        case PARTICLE_TYPE::RIPPLE_PARTICLE:
        for (int i = 0; i < count; i++)
        {
            Entity particle = createRippleParticle(position);
            particlesByType[type].push_back(particle);
        }
        break;

        default:
            break;
    }
}

Entity ParticleSystem::createDeathParticle(vec2 position)
{
    Entity entity = Entity();

    // create motion component
    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.angle = 0.0f;

    // random burst direction in a circle
    float angle = uniform_dist(rng) * 2.0f * M_PI;
    float speed = 100.0f + uniform_dist(rng) * 150.0f;
    motion.velocity = {cos(angle) * speed, sin(angle) * speed};

    // random size variation
    float size_factor = 16.0f + uniform_dist(rng) * 10.0f;
    motion.scale = {size_factor/2, size_factor};

    // random color (temporary)
    // float r = 0.5f + uniform_dist(rng) * 0.2f;
    // float g = 0.7f + uniform_dist(rng) * 0.3f;
    // float b = 0.2f + uniform_dist(rng) * 0.2f;    
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    registry.colors.emplace(entity, vec3(r, g, b));

    // add  componentss
    Particle& particle = registry.particles.emplace(entity);
    particle.type = PARTICLE_TYPE::DEATH_PARTICLE;
    particle.lifetime_ms = 1000.0f + uniform_dist(rng) * 500.0f;
    particle.max_lifetime_ms = particle.lifetime_ms;
    particle.state = PARTICLE_STATE::BURST;
    particle.state_timer_ms = 300.0f + uniform_dist(rng) * 200.0f; // Time before following player
    particle.speed_factor = 50.0f + uniform_dist(rng) * 50.0f;

    registry.renderRequests.insert(
        entity,
        {TEXTURE_ASSET_ID::PARTICLE,
         EFFECT_ASSET_ID::TEXTURED,  
         GEOMETRY_BUFFER_ID::SPRITE});

    return entity;
}

Entity ParticleSystem::createRippleParticle(vec2 position)
{
    Entity entity = Entity();
    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.angle = 0.0f;

    float size_factor = 4.0f + uniform_dist(rng) * 2.0f;
    motion.scale = {size_factor, size_factor};
    registry.colors.emplace(entity, vec3(1.0f, 1.0f, 1.0f));
    
    Particle& particle = registry.particles.emplace(entity);
    particle.type = PARTICLE_TYPE::RIPPLE_PARTICLE;
    particle.lifetime_ms = 800.0f + uniform_dist(rng) * 200.0f;
    particle.max_lifetime_ms = particle.lifetime_ms;
    particle.state = PARTICLE_STATE::BURST;

    registry.renderRequests.insert(
        entity,
        {TEXTURE_ASSET_ID::PARTICLE,
         EFFECT_ASSET_ID::TEXTURED,
         GEOMETRY_BUFFER_ID::SPRITE});

    return entity;
}

void ParticleSystem::createPlayerRipples(Entity player_entity)
{
    if (!registry.motions.has(player_entity))
        return;
        
    Motion& player_motion = registry.motions.get(player_entity);
    vec2 velocity_direction = glm::normalize(player_motion.velocity);
    if (glm::length(velocity_direction) < 0.01f)
        return;
        
    vec2 perpendicular = vec2(-velocity_direction.y, velocity_direction.x);
    float offset_distance = player_motion.scale.x * 0.6f;
    vec2 left_position = player_motion.position - perpendicular * offset_distance;
    vec2 right_position = player_motion.position + perpendicular * offset_distance;
    float tail_offset = player_motion.scale.y * 0.5f;
    left_position -= velocity_direction * tail_offset;
    right_position -= velocity_direction * tail_offset;
    
    Entity left_particle = createRippleParticle(left_position);
    Entity right_particle = createRippleParticle(right_position);
    
    Motion& left_motion = registry.motions.get(left_particle);
    Motion& right_motion = registry.motions.get(right_particle);
    float ripple_speed = 50.0f + uniform_dist(rng) * 30.0f;
    left_motion.velocity = -perpendicular * ripple_speed;
    right_motion.velocity = perpendicular * ripple_speed;
    
    particlesByType[PARTICLE_TYPE::RIPPLE_PARTICLE].push_back(left_particle);
    particlesByType[PARTICLE_TYPE::RIPPLE_PARTICLE].push_back(right_particle);
}
