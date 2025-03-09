#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <string>
#include <tuple>
#include <vector>
#include <cmath>

// glfw (OpenGL)
#define NOMINMAX
#include <gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>			   // vec2
#include <glm/ext/vector_int2.hpp> // ivec2
#include <glm/vec3.hpp>			   // vec3
#include <glm/mat3x3.hpp>		   // mat3
using namespace glm;

#include "tinyECS/tiny_ecs.hpp"

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shader_path(const std::string &name) { return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name; };
inline std::string textures_path(const std::string &name) { return data_path() + "/textures/" + std::string(name); };
inline std::string audio_path(const std::string &name) { return data_path() + "/audio/" + std::string(name); };
inline std::string mesh_path(const std::string &name) { return data_path() + "/meshes/" + std::string(name); };

//
// game constants
//

// Required to Scale up the game beyond base... -> View as 1280x720 at 2x WORK_SCALE_FACTOR
const float WORK_SCALE_FACTOR = 2;

// Window dimensions
const float WINDOW_WIDTH_PX = 640 * WORK_SCALE_FACTOR;
const float WINDOW_HEIGHT_PX = 360 * WORK_SCALE_FACTOR;

const float MS_PER_S = 1000.f;

// TILE DIMENSIONS/ GRID DIMENSIONS
const float TILE_SIZE = 128 * WORK_SCALE_FACTOR;

// USING TILE SIZE
const float GRID_CELL_WIDTH_PX = TILE_SIZE;
const float GRID_CELL_HEIGHT_PX = TILE_SIZE;
const float GRID_LINE_WIDTH_PX = 3;

// WINDOW GRID DIMENSIONS
const float WINDOW_GRID_WIDTH = WINDOW_WIDTH_PX / TILE_SIZE;
const float WINDOW_GRID_HEIGHT = WINDOW_HEIGHT_PX / TILE_SIZE;

// MAP stuff
const float CHUNK_DISTANCE = 5.0;
const float MAP_WIDTH = 20;
const float MAP_HEIGHT = 20;
const vec2 WORLD_ORIGIN = {10, 10};

// Set map left right based on width and origin
const float MAP_LEFT = WORLD_ORIGIN.x - MAP_WIDTH / 2;
const float MAP_RIGHT = WORLD_ORIGIN.x + MAP_WIDTH / 2;
const float MAP_TOP = WORLD_ORIGIN.y - MAP_HEIGHT / 2;
const float MAP_BOTTOM = WORLD_ORIGIN.y + MAP_HEIGHT / 2;
const bool DEBUG_GRID = false;

// CAMERA
const vec2 DEADZONE_FACTOR = {0.50f, 0.50f};
const float CAMERA_TRACKING_DEADZONE = 100.0 * WORK_SCALE_FACTOR;
const float MOUSE_TRACKING_DEADZONE = 100.0 * WORK_SCALE_FACTOR;

// These are hard coded to the dimensions of the entity's texture
const float PLAYER_SIZE = 32 * WORK_SCALE_FACTOR;
const float ENEMY_SIZE = 32 * WORK_SCALE_FACTOR;
const float LARGE_ENEMY_SIZE = 64 * WORK_SCALE_FACTOR;

// PLAYER BB
const float PLAYER_BB_WIDTH = (float)PLAYER_SIZE;
const float PLAYER_BB_HEIGHT = (float)PLAYER_SIZE;

// invaders are 32x32 px, but cells are 60x60
const float ENEMY_BB_WIDTH = (float)ENEMY_SIZE;
const float ENEMY_BB_HEIGHT = (float)ENEMY_SIZE;
const float ENEMY_SPEED = 300; // reasonalby between 200-400
const float ENEMY_PATROL_SPEED_PER_MS = 150.f / MS_PER_S;
const float ENEMY_PATROL_RANGE = 100.0f;
const float ENEMY_SPEED_PER_MS = ENEMY_SPEED / MS_PER_S; // reasonalby between 200-400
const float ENEMY_PATROL_TIME_MS = 2 * ENEMY_PATROL_RANGE / ENEMY_PATROL_SPEED_PER_MS;
const float ENEMY_DAMAGE = 10;
// ENEMY STATS
const float ENEMY_HEALTH = 50;
const float ENEMY_SPAWN_RATE_MS = 1 * 1000;
const float MAX_ENEMIES_COUNT = 15;
const float ENEMY_DETECTION_RADIUS = 300.0f;

// LARGE ENEMY
const float LARGE_ENEMY_BB_WIDTH = (float)LARGE_ENEMY_SIZE;
const float LARGE_ENEMY_BB_HEIGHT = (float)LARGE_ENEMY_SIZE;

// BUFF MAP SIZE
const float BUFF_MAP_SIZE = 20 * WORK_SCALE_FACTOR;
const float BUFF_HUD_SIZE = 16 * WORK_SCALE_FACTOR; // DONT USE YET!!

// AMOEBA STATS
const float PLAYER_DEFAULT_HEALTH = 100;
const float PLAYER_SPEED = 200;
const float PLAYER_DASH_SPEED = 500;
const float PLAYER_DASH_RANGE = 200;
const float PLAYER_DASH_COOLDOWN_MS = 1000;
const float PLAYER_DASH_DAMAGE = 20;
const float DASH_RECHARGE_COUNT = 3;
const float DASH_RECHARGE_DELAY_MS = 1000;
const float PLAYER_BASE_HEALING_RATE = 0.0f;
const float PLAYER_DEFAULT_HEALING_TIMER_MS = 1000;


// OTHER CONSTANTS
const float PROJECTILE_DAMAGE = 10;
const float DASH_DURATION_MS = 500.0f;
const float VELOCITY_DECAY_RATE = 1.01f; // 0.95f;

// LOGO
const float LOGO_WIDTH_PX = 383 * WORK_SCALE_FACTOR;
const float LOGO_HEIGHT_PX = 122 * WORK_SCALE_FACTOR;

// CUTSCENES
const float INTRO_CUTSCENE_DURATION_MS = 3 * MS_PER_S; // for animation after clicking start
const float BOOT_CUTSCENE_DURATION_MS = 3 * MS_PER_S;  // for logo movement at boot
const float GAMEPLAY_CUTSCENE_DURATION_MS = 3 * MS_PER_S;
const float STATE_TIMER_DEFAULT = 3 * MS_PER_S;

// button positions, scales
const vec2 START_BUTTON_COORDINATES = {0.f, WINDOW_HEIGHT_PX / 4.5f};
const vec2 START_BUTTON_SCALE = {WINDOW_WIDTH_PX / 7.f, WINDOW_HEIGHT_PX / 7.f};

const vec2 SHOP_INFO_BUTTON_SCALE = {WINDOW_WIDTH_PX / 20.f, WINDOW_HEIGHT_PX / 20.f * 1.78f};

// UI
const float UI_SCALE = 1;
const float UI_MARGIN_X = 32;
const float UI_MARGIN_Y = 32;
const float UI_SPACING = 16;

// nuclues
const float NUCLEUS_UI_WIDTH = 62 * UI_SCALE;
const float NUCLEUS_UI_HEIGHT = 68 * UI_SCALE;
const vec2 NUCLEUS_UI_POS = {-WINDOW_WIDTH_PX / 2 + UI_MARGIN_X + NUCLEUS_UI_WIDTH / 2,
							 WINDOW_HEIGHT_PX / 2 - UI_MARGIN_Y - NUCLEUS_UI_HEIGHT / 2};

// health bar
const float HEALTH_BAR_WIDTH = 107 * UI_SCALE;
const float HEALTH_BAR_HEIGHT = 15 * UI_SCALE;
const vec2 HEALTH_BAR_POS = {
	-WINDOW_WIDTH_PX / 2 + UI_MARGIN_X + NUCLEUS_UI_WIDTH + UI_SPACING + HEALTH_BAR_WIDTH / 2,
	WINDOW_HEIGHT_PX / 2 - UI_MARGIN_Y - NUCLEUS_UI_HEIGHT / 2 - UI_SPACING - HEALTH_BAR_HEIGHT / 2};

// dash recharge
const float DASH_WIDTH = 18 * UI_SCALE;
const float DASH_HEIGHT = 21.6 * UI_SCALE;

// const float DASH_RECHARGE_SPACING = (HEALTH_BAR_WIDTH - DASH_WIDTH) / (DASH_RECHARGE_COUNT - 1);
const float DASH_RECHARGE_SPACING = DASH_WIDTH + 8;
const vec2 DASH_RECHARGE_START_POS = {
	-WINDOW_WIDTH_PX / 2 + UI_MARGIN_X + NUCLEUS_UI_WIDTH + UI_SPACING + DASH_WIDTH / 2,
	WINDOW_HEIGHT_PX / 2 - UI_MARGIN_Y - NUCLEUS_UI_HEIGHT / 2}; // pos.y aligns w/ nuclues

// germoney
const float GERMONEY_UI_WIDTH = 48 * UI_SCALE;
const float GERMONEY_UI_HEIGHT = 22 * UI_SCALE;
const vec2 GERMONEY_UI_POS = {-WINDOW_WIDTH_PX / 2 + UI_MARGIN_X + NUCLEUS_UI_WIDTH + UI_SPACING + GERMONEY_UI_WIDTH / 2,
							  WINDOW_HEIGHT_PX / 2 - UI_MARGIN_Y - NUCLEUS_UI_HEIGHT / 2 + UI_SPACING + GERMONEY_UI_HEIGHT / 2};
// weapon pill
const float WEAPON_PILL_UI_WIDTH = 118 * UI_SCALE;
const float WEAPON_PILL_UI_HEIGHT = 58 * UI_SCALE;
const vec2 WEAPON_PILL_UI_POS = {WINDOW_WIDTH_PX / 2 - UI_MARGIN_X - WEAPON_PILL_UI_WIDTH / 2,
								 WINDOW_HEIGHT_PX / 2 - UI_MARGIN_Y - NUCLEUS_UI_HEIGHT / 2}; // pos.y aligns w/ nuclues

// 
const int NUMBER_OF_BUFFS = 5;
const float BUFF_WIDTH = 20.0f * WORK_SCALE_FACTOR;
const float BUFF_HEIGHT = 20.0f * WORK_SCALE_FACTOR;
const float BUFF_UI_WIDTH = 20.0f * UI_SCALE;
const float BUFF_UI_HEIGHT = 20.0f * UI_SCALE;
const float BUFF_SPACING = 12 + BUFF_UI_WIDTH;
const int BUFF_NUM = (static_cast<int>(std::floor(
						  (WINDOW_WIDTH_PX - UI_MARGIN_X * 2 - NUCLEUS_UI_WIDTH - HEALTH_BAR_WIDTH - WEAPON_PILL_UI_WIDTH - UI_SPACING) / BUFF_SPACING)) /
					  2) *
					 2;
const vec2 BUFF_START_POS = {
	-(BUFF_NUM / 2) / 2 * BUFF_SPACING,
	WINDOW_HEIGHT_PX / 2 - UI_MARGIN_Y - NUCLEUS_UI_HEIGHT / 2 + UI_SPACING + GERMONEY_UI_HEIGHT / 2};

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recommend making all components non-copyable by derving from ComponentNonCopyable
struct Transform
{
	mat3 mat = {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}}; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
};

bool gl_has_errors();
