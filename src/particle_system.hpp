#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"
#include <vector>
#include <unordered_map>
#include <random>

// Maximum number of particles that can be active at once
const size_t MAX_PARTICLES = 1000;

class ParticleSystem 
{
public:
    // unitialize the system
    ParticleSystem();
    ~ParticleSystem();

    // update particle positions, velocities, lifetimes, etfcc
    void step(float elapsed_ms);

    // create different types of particles
    void createParticles(PARTICLE_TYPE type, vec2 position, int count);

    // get the VAO (just for rendering)
    GLuint getVAO() const { return baseVAO; }

private:
    // helper function to create a death particle
    Entity createDeathParticle(vec2 position);

    // track spawned particles by type (for potential optimizations???)
    std::unordered_map<PARTICLE_TYPE, std::vector<Entity>> particlesByType;

    // random number generator for particle variations
    std::default_random_engine rng;
    std::uniform_real_distribution<float> uniform_dist;

    // instancing data
    GLuint baseVAO;          //  vertex array object
    GLuint baseVBO;          //  vertex buffer (positions)
    GLuint instanceVBO;      // instance data buffer
    GLuint instanceTexCoordVBO; // texture coordinates buffer
    
    // instance data arrays
    std::vector<vec2> instancePositions;
    std::vector<vec2> instanceScales;
    std::vector<vec4> instanceColors;
    std::vector<float> instanceRotations;

    void initializeBuffers();
    void updateInstanceData();
};