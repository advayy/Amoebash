#include "particle_system.hpp"
#include "tinyECS/registry.hpp"
#include <iostream>
#include <random>

// defined in registry.cpp
extern ECSRegistry registry;

ParticleSystem::ParticleSystem()
{
    // create random number generator
    std::random_device rd;
    rng = std::default_random_engine(rd());
    uniform_dist = std::uniform_real_distribution<float>(0.0f, 1.0f);

    initializeBuffers();
}

ParticleSystem::~ParticleSystem() {
    glDeleteBuffers(1, &baseVBO);
    glDeleteBuffers(1, &instanceVBO);
    glDeleteBuffers(1, &instanceTexCoordVBO);
    glDeleteVertexArrays(1, &baseVAO);
}

void ParticleSystem::initializeBuffers() {
    // create base quad vertices
    float quadVertices[] = {
        -0.5f, -0.5f,  // Bottom left
         0.5f, -0.5f,  // Bottom right
         0.5f,  0.5f,  // Top right
        -0.5f,  0.5f   // Top left
    };

    float texCoords[] = {
        0.0f, 0.0f,  // Bottom left
        1.0f, 0.0f,  // Bottom right
        1.0f, 1.0f,  // Top right
        0.0f, 1.0f   // Top left
    };

    // create and bind VAO first
    glGenVertexArrays(1, &baseVAO);
    glBindVertexArray(baseVAO);

    // det up base vertex positions (location = 0)
    glGenBuffers(1, &baseVBO);
    glBindBuffer(GL_ARRAY_BUFFER, baseVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    // det up texture coordinates (location = 1)
    glGenBuffers(1, &instanceTexCoordVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceTexCoordVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    // create instance data buffer
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

    // preellocate space for instance data
    size_t stride = sizeof(vec2) + sizeof(vec2) + sizeof(vec4) + sizeof(float); // position + scale + color + rotation
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * stride, nullptr, GL_DYNAMIC_DRAW);

    // Set up instance attributes with zero stride initially (will be updated in updateInstanceData)
    size_t offset = 0;

    // position (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)offset);
    glVertexAttribDivisor(2, 1);
    // scale (location = 3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(vec2) * MAX_PARTICLES));
    glVertexAttribDivisor(3, 1);
    // color (location = 4)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(vec2) * MAX_PARTICLES * 2));
    glVertexAttribDivisor(4, 1);
    // rotation (location = 5)
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(vec2) * MAX_PARTICLES * 2 + sizeof(vec4) * MAX_PARTICLES));
    glVertexAttribDivisor(5, 1);
    // Unbind VAO
    glBindVertexArray(0);
}

void ParticleSystem::updateInstanceData() {
    // clea previous instance data
    instancePositions.clear();
    instanceScales.clear();
    instanceColors.clear();
    instanceRotations.clear();

    // Gathr data  from all active particles
    for (uint i = 0; i < registry.particles.size(); i++) {
        Entity entity = registry.particles.entities[i];
        
        // Get motion dta
        Motion& motion = registry.motions.get(entity);
        instancePositions.push_back(motion.position);
        instanceScales.push_back(motion.scale);
        
        // Get color daa
        vec4 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec4(1.0f);
        instanceColors.push_back(color);
        
        // Get rotation        
        instanceRotations.push_back(motion.angle);
    }

    // update instance buffer w/ new data
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    size_t totalSize = sizeof(vec2) * instancePositions.size() + 
                      sizeof(vec2) * instanceScales.size() + 
                      sizeof(vec4) * instanceColors.size() + 
                      sizeof(float) * instanceRotations.size();
    
    glBufferData(GL_ARRAY_BUFFER, totalSize, nullptr, GL_DYNAMIC_DRAW);
    
    // upload data in separate chunks
    size_t offset = 0;
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vec2) * instancePositions.size(), instancePositions.data());
    offset += sizeof(vec2) * instancePositions.size();
    
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vec2) * instanceScales.size(), instanceScales.data());
    offset += sizeof(vec2) * instanceScales.size();
    
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(vec4) * instanceColors.size(), instanceColors.data());
    offset += sizeof(vec4) * instanceColors.size();
    
    glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(float) * instanceRotations.size(), instanceRotations.data());
}

void ParticleSystem::step(float elapsed_ms)
{
    // update existing particle behavior
    for (uint i = 0; i < registry.particles.size(); i++)
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
        Motion& motion = registry.motions.get(entity);
        
        switch (particle.type)
        {
            case PARTICLE_TYPE::DEATH_PARTICLE:
            {
                // during initial burst phase
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
                // during follow phase
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
                        vec4& color = registry.colors.get(entity);
                        color.a *= life_ratio * 2.0f;
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    // update instance data for rendering
    updateInstanceData();
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