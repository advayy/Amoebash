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
            case PARTICLE_TYPE::RIPPLE_PARTICLE:
            {
                Motion& motion = registry.motions.get(entity);
                
                motion.position += motion.velocity * (elapsed_ms / 1000.f);
                
                float life_ratio = particle.lifetime_ms / particle.max_lifetime_ms;
                float start_size = 2.0f;
                float end_size = 4.0f;
                float current_size = start_size + (end_size - start_size) * (1.0f - life_ratio);
                motion.scale = {current_size, current_size};
                
                if (registry.colors.has(entity))
                {
                    vec3& color = registry.colors.get(entity);
                    color = color * life_ratio;
                }
                motion.velocity *= 0.97f;
                
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

Entity ParticleSystem::createRippleParticle(vec2 position, float lifetime_scale = 1.0f)
{
    Entity entity = Entity();
    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;

    Particle& particle = registry.particles.emplace(entity);
    particle.type = PARTICLE_TYPE::RIPPLE_PARTICLE;
particle.max_lifetime_ms = (3000.0f + uniform_dist(rng) * 200.0f) * lifetime_scale;
    particle.max_lifetime_ms = particle.lifetime_ms;
    particle.state = PARTICLE_STATE::FADE;

    registry.renderRequests.insert(
        entity,
        {TEXTURE_ASSET_ID::PIXEL_PARTICLE,
         EFFECT_ASSET_ID::TEXTURED,
         GEOMETRY_BUFFER_ID::SPRITE});

    return entity;
}

void ParticleSystem::createPlayerRipples(Entity player_entity)
{
    if (!registry.motions.has(player_entity))
        return;

    Motion& player_motion = registry.motions.get(player_entity);
    float player_speed = glm::length(player_motion.velocity);
    if (glm::length(player_motion.velocity) < 1.0f) return;

    vec2 velocity_direction = glm::normalize(player_motion.velocity);
    vec2 perpendicular = vec2(-velocity_direction.y, velocity_direction.x);

    float tail_offset = player_motion.scale.y * 0.6f;
    float side_offset = player_motion.scale.x * 0.3f;

    vec2 tail_center = player_motion.position - velocity_direction * tail_offset;

    float random_factor = 1.0f;
    vec2 randomness = vec2(
        (uniform_dist(rng) - 0.5f) * random_factor,
        (uniform_dist(rng) - 0.5f) * random_factor
    );

    vec2 left_position = tail_center - perpendicular * side_offset + randomness;
    vec2 right_position = tail_center + perpendicular * side_offset + randomness;

    float speed_scale = 2.0f;

    float left_distance = glm::length(left_position - tail_center);
    float right_distance = glm::length(right_position - tail_center);

    float left_speed = left_distance * speed_scale;
    float right_speed = right_distance * speed_scale;

    float angle_variation = 0.005f;
    float angle_offset_left = (uniform_dist(rng) - 0.5f) * angle_variation;
    float angle_offset_right = (uniform_dist(rng) - 0.5f) * angle_variation;

    vec2 left_base_dir = -perpendicular;
    vec2 right_base_dir = perpendicular;

    float cos_left = cos(angle_offset_left);
    float sin_left = sin(angle_offset_left);
    vec2 left_direction = vec2(
        left_base_dir.x * cos_left - left_base_dir.y * sin_left,
        left_base_dir.x * sin_left + left_base_dir.y * cos_left
    );

    float cos_right = cos(angle_offset_right);
    float sin_right = sin(angle_offset_right);
    vec2 right_direction = vec2(
        right_base_dir.x * cos_right - right_base_dir.y * sin_right,
        right_base_dir.x * sin_right + right_base_dir.y * cos_right
    );

    float lifetime_scale = glm::clamp(player_speed / 100.f, 0.2f, 1.0f);
    Entity left_particle = createRippleParticle(left_position, lifetime_scale);
    Entity right_particle = createRippleParticle(right_position, lifetime_scale);

    Motion& left_motion = registry.motions.get(left_particle);
    Motion& right_motion = registry.motions.get(right_particle);

    left_motion.velocity = left_direction * left_speed;
    right_motion.velocity = right_direction * right_speed;

    particlesByType[PARTICLE_TYPE::RIPPLE_PARTICLE].push_back(left_particle);
    particlesByType[PARTICLE_TYPE::RIPPLE_PARTICLE].push_back(right_particle);
}
