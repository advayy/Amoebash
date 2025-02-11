#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <string>
#include <tuple>
#include <vector>

// glfw (OpenGL)
#define NOMINMAX
#include <gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>				// vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3
using namespace glm;

#include "tinyECS/tiny_ecs.hpp"

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shader_path(const std::string& name) {return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name;};
inline std::string textures_path(const std::string& name) {return data_path() + "/textures/" + std::string(name);};
inline std::string audio_path(const std::string& name) {return data_path() + "/audio/" + std::string(name);};
inline std::string mesh_path(const std::string& name) {return data_path() + "/meshes/" + std::string(name);};

//
// game constants
//

// Required to Scale up the game beyond base... -> View as 1280x720 at 2x WORK_SCALE_FACTOR
const int WORK_SCALE_FACTOR = 2;

// Window dimensions
const int WINDOW_WIDTH_PX = 640 * WORK_SCALE_FACTOR;
const int WINDOW_HEIGHT_PX = 360 * WORK_SCALE_FACTOR;

// TILE DIMENSIONS/ GRID DIMENSIONS
const int TILE_SIZE = 128 * WORK_SCALE_FACTOR;

// USING TILE SIZE
const int GRID_CELL_WIDTH_PX = TILE_SIZE;
const int GRID_CELL_HEIGHT_PX = TILE_SIZE;
const int GRID_LINE_WIDTH_PX = 3;

// WINDOW GRID DIMENSIONS
const float WINDOW_GRID_WIDTH = WINDOW_WIDTH_PX / TILE_SIZE;
const float WINDOW_GRID_HEIGHT = WINDOW_HEIGHT_PX / TILE_SIZE;

// MAP stuff
const vec2 WORLD_ORIGIN = {0, 0};
const int CHUNK_DISTANCE = 1;

// These are hard coded to the dimensions of the entity's texture
const int PLAYER_SIZE = 32 * WORK_SCALE_FACTOR;
const int ENEMY_SIZE = 32 * WORK_SCALE_FACTOR;
const int LARGE_ENEMY_SIZE = 64 * WORK_SCALE_FACTOR;

// PLAYER BB
const float PLAYER_BB_WIDTH = (float)PLAYER_SIZE;
const float PLAYER_BB_HEIGHT = (float)PLAYER_SIZE;

// invaders are 32x32 px, but cells are 60x60
const float ENEMY_BB_WIDTH = (float)ENEMY_SIZE;
const float ENEMY_BB_HEIGHT = (float)ENEMY_SIZE;

// LARGE ENEMY
const float LARGE_ENEMY_BB_WIDTH = (float)LARGE_ENEMY_SIZE;
const float LARGE_ENEMY_BB_HEIGHT = (float)LARGE_ENEMY_SIZE;


// AMOEBA STATS
const int PLAYER_HEALTH = 100;
const int PLAYER_SPEED = 500;
const int PLAYER_DASH_RANGE = 200;
const int PLAYER_DASH_COOLDOWN_MS = 250;
const int PLAYER_DASH_DAMAGE = 20;

// ENEMY STATS
const int ENEMY_HEALTH = 50;
const int ENEMY_SPAWN_RATE_MS = 2 * 1000;

// OTHER CONSTANTS
const int PROJECTILE_DAMAGE = 10;
const float DASH_DURATION_MS = 500.0f;
const float VELOCITY_DECAY_RATE = 1.01f; // 0.95f;

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recommend making all components non-copyable by derving from ComponentNonCopyable
struct Transform {
	mat3 mat = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, { 0.f, 0.f, 1.f} }; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
};

bool gl_has_errors();
