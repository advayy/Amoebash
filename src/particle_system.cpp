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
    // handle all particles
    for (uint i = 0; i < registry.particles.size(); i++)
    {
        Entity entity = registry.particles.entities[i];
        Particle& particle = registry.particles.components[i];
        
        // update  lifetime
        particle.lifetime_ms -= elapsed_ms;
        
        // Remove expired particles
        if (particle.lifetime_ms <= 0)
        {
            registry.remove_all_components_of(entity);
            continue;
        }
        
        // update particle behavior stae based on type
        switch (particle.type)
        {
            case PARTICLE_TYPE::DEATH_PARTICLE:
            {
                Motion& motion = registry.motions.get(entity);
                
                // during initial burst phase
                if (particle.state == PARTICLE_STATE::BURST)
                {
                    // gradually slow down as particles expand outward
                    motion.velocity *= 0.98f;
                    
                    // transition to follow state after a delay
                    particle.state_timer_ms -= elapsed_ms;
                    if (particle.state_timer_ms <= 0)
                    {
                        particle.state = PARTICLE_STATE::FOLLOW;
                        particle.state_timer_ms = 1000.f; // time spent following
                    }
                }
                // dring follow phase
                else if (particle.state == PARTICLE_STATE::FOLLOW)
                {
                    // get player position if any
                    if (!registry.players.entities.empty())
                    {
                        Entity player_entity = registry.players.entities[0];
                        Motion& player_motion = registry.motions.get(player_entity);
                        
                        // calculate direction to player
                        vec2 direction = player_motion.position - motion.position;
                        float distance = glm::length(direction);
                        
                        // normalize direction and apply acceleration
                        if (distance > 0.1f)
                        {
                            direction = glm::normalize(direction);
                            float speed_factor = particle.speed_factor * (1.0f + (1.0f - particle.lifetime_ms / particle.max_lifetime_ms) * 200.0f);
                            motion.velocity += direction * speed_factor * (elapsed_ms / 1000.f);
                            
                            // cap velocity
                            float max_speed = 600.0f;
                            float current_speed = glm::length(motion.velocity);
                            if (current_speed > max_speed)
                            {
                                motion.velocity = glm::normalize(motion.velocity) * max_speed;
                            }
                        }
                    }
                    
                    // apply fading based on lifetime (temporary)
                    float life_ratio = particle.lifetime_ms / particle.max_lifetime_ms;
                    if (registry.colors.has(entity))
                    {
                        vec4& color = registry.colors.get(entity);
                        color.a *= life_ratio * 2.0f;
                    }
                }
                break;
            }
            case PARTICLE_TYPE::DASH_RIPPLE:
            {
                Motion& motion = registry.motions.get(entity);
                
                // Dash ripple uses EXPAND state to grow outward
                if (particle.state == PARTICLE_STATE::EXPAND)
                {
                    // Expand the ripple
                    float growth_rate = 70.0f;
                    motion.scale += vec2(growth_rate, growth_rate) * (elapsed_ms / 1000.f);
                    
                    // Fade out based on lifetime
                    float life_ratio = particle.lifetime_ms / particle.max_lifetime_ms;
                    if (registry.colors.has(entity))
                    {
                        vec4& color = registry.colors.get(entity);
                        color.a = life_ratio;
                    }
                }
                break;
            }
            /// add more type particles heree
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
        case PARTICLE_TYPE::DASH_RIPPLE:
            for (int i = 0; i < count; i++)
            {
                Entity particle = createDashRippleParticle(position);
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
    float speed = 350.0f + uniform_dist(rng) * 150.0f;
    motion.velocity = {cos(angle) * speed, sin(angle) * speed};
    
    // random size variation
    float size_factor = 5.0f + uniform_dist(rng) * 5.0f;
    motion.scale = {size_factor, size_factor};
     
    // add color component
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
    registry.colors.emplace(entity, vec4(r, g, b, a));
    
    // ad  componentss
    Particle& particle = registry.particles.emplace(entity);
    particle.type = PARTICLE_TYPE::DEATH_PARTICLE;
    particle.lifetime_ms = 1000.0f + uniform_dist(rng) * 500.0f;
    particle.max_lifetime_ms = particle.lifetime_ms;
    particle.state = PARTICLE_STATE::BURST;
    particle.state_timer_ms = 300.0f + uniform_dist(rng) * 200.0f; // time before following player
    particle.speed_factor = 400.0f + uniform_dist(rng) * 200.0f;
    
    registry.renderRequests.insert(
        entity,
        {TEXTURE_ASSET_ID::PARTICLE,
         EFFECT_ASSET_ID::TEXTURED,  
         GEOMETRY_BUFFER_ID::SPRITE});
    
    return entity;
}

Entity ParticleSystem::createDashRippleParticle(vec2 position)
{
    Entity entity = Entity();
    
    // create motion component
    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.angle = 0.0f;
    motion.velocity = {0.0f, 0.0f}; // Stationary ripple
    
    // Initial size
    float size_factor = 5.0f; // Initial ripple size
    motion.scale = {size_factor, size_factor};
    
    // Add color component - bluish color with transparency
    float r = 0.2f;
    float g = 0.6f;
    float b = 1.0f;
    float a = 0.2f; 
    registry.colors.emplace(entity, vec4(r, g, b, a));
    
    // Add particle component
    Particle& particle = registry.particles.emplace(entity);
    particle.type = PARTICLE_TYPE::DASH_RIPPLE;
    particle.lifetime_ms = 500.0f; // Short lifetime for quick effect
    particle.max_lifetime_ms = particle.lifetime_ms;
    particle.state = PARTICLE_STATE::EXPAND; // Use expand state
    
    // Add render request
    registry.renderRequests.insert(
        entity,
        {TEXTURE_ASSET_ID::RIPPLE_PARTICLE,
         EFFECT_ASSET_ID::TEXTURED,  
         GEOMETRY_BUFFER_ID::SPRITE});
    
    return entity;
}