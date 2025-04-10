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
const float CAMERA_POSITION_RADIUS = 100.f;

// These are hard coded to the dimensions of the entity's texture
const float PLAYER_SIZE = 32 * WORK_SCALE_FACTOR;
const float ENEMY_SIZE = 32 * WORK_SCALE_FACTOR;
const float LARGE_ENEMY_SIZE = 64 * WORK_SCALE_FACTOR;
const float PROJECTILE_SIZE = 8 * WORK_SCALE_FACTOR;
const float BOSS_PROJECTILE_SIZE = 32 * WORK_SCALE_FACTOR;
const float FINAL_BOSS_PROJECTILE_SIZE = 32 * WORK_SCALE_FACTOR;

// PLAYER BB
const float PLAYER_BB_WIDTH = (float)PLAYER_SIZE;
const float PLAYER_BB_HEIGHT = (float)PLAYER_SIZE;

// Hexagon stuff
const float HEXAGON_RADIUS = (float)PLAYER_SIZE;
// const vec2 HEXAGON_SPEED = {800, 800};

// invaders are 32x32 px, but cells are 60x60
const float ENEMY_BB_WIDTH = (float)ENEMY_SIZE;
const float ENEMY_BB_HEIGHT = (float)ENEMY_SIZE;
const float ENEMY_SPEED = 300; // reasonalby between 200-400

const float SPIKE_ENEMY_PATROL_SPEED_PER_MS = 150.f / MS_PER_S;
const float SPIKE_ENEMY_PATROL_RANGE = 100.0f;
const float ENEMY_SPEED_PER_MS = ENEMY_SPEED / MS_PER_S; // reasonalby between 200-400
const float ENEMY_PATROL_TIME_MS = 2 * SPIKE_ENEMY_PATROL_RANGE / SPIKE_ENEMY_PATROL_SPEED_PER_MS;
const float SPIKE_ENEMY_BOMB_DAMAGE = 40.f;
const float SPIKE_ENEMY_BOMB_TIMER = 750.f;
const float SPIKE_ENEMY_KNOCKBACK_DECAY = 0.9f;
const float SPIKE_ENEMY_KNOCKBACK_TIMER = 500.f;
const float SPIKE_ENEMY_KNOCKBACK_STRENGTH = 1750.f;

const float ENEMY_DAMAGE = 10;
const float SPIKE_ENEMY_DETECTION_RADIUS = 300.0f;
const float BACTERIOPHAGE_ENEMY_KEEP_AWAY_RADIUS = 150.f;
const float BACTERIOPHAGE_ENEMY_DETECTION_RADIUS = 500.0f;
const float BACTERIOPHAGE_ENEMY_SPEED = 150.0f;
const float NEXT_LEVEL_BLACK_SCREEN_TIMER_MS = 1000.0f;

// ENEMY STATS
const float ENEMY_HEALTH = 50;
const float ENEMY_SPAWN_RATE_MS = 1 * 1000;
const float MAX_BACTERIOPHAGE_COUNT = 8;
const float MAX_ENEMIES_COUNT = MAX_BACTERIOPHAGE_COUNT + 20;

const float BOSS_HEALTH = 200;
const float SMALLEST_BOSS_HEALTH = BOSS_HEALTH / 8.f;
const float FINAL_BOSS_HEALTH = 350.f;

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
const float PLAYER_DEFAULT_HEALING_TIMER_MS = 2500;

// PROJECTILE STATS
const float PROJECTILE_DAMAGE = 5;
const float PROJECTILE_SPAWN_RATE_MS = 2 * MS_PER_S;
const float PROJECTILE_SPEED = 300.0f;
const float PROJECTILE_TTL_MS = 4 * MS_PER_S;

// ARROW STATS
const float ARROW_WIDTH = 16 * WORK_SCALE_FACTOR;
const float ARROW_HEIGHT = 16 * WORK_SCALE_FACTOR;

// GUN and PROJECTILE
const float GUN_COOLDOWN_MS = 500.0f;
const float GUN_SIZE = 20 * WORK_SCALE_FACTOR;
const float GUN_UI_SIZE = 20 * WORK_SCALE_FACTOR;
const float GUN_PROJECTILE_DAMAGE = 15;
const float GUN_PROJECTILE_SPEED = 600.0f;

const float PROJECTILE_BB_WIDTH = (float)PROJECTILE_SIZE;
const float PROJECTILE_BB_HEIGHT = (float)PROJECTILE_SIZE;
const vec2 BOSS_PROJECTILE = {BOSS_PROJECTILE_SIZE, BOSS_PROJECTILE_SIZE};
const vec2 FINAL_BOSS_PROJECTILE = {FINAL_BOSS_PROJECTILE_SIZE, FINAL_BOSS_PROJECTILE_SIZE};
const float BOSS_PROJECTILE_SPEED = 300.f;
const float BOSS_BB_WIDTH = 128 * WORK_SCALE_FACTOR;
const float BOSS_BB_HEIGHT = 128 * WORK_SCALE_FACTOR;
const float FINAL_BOSS_BB_WIDTH = 256 * WORK_SCALE_FACTOR;
const float FINAL_BOSS_BB_HEIGHT = 256 * WORK_SCALE_FACTOR;
const float BOSS_DETECTION_RADIUS = SPIKE_ENEMY_DETECTION_RADIUS * 3.0f;
const float BOSS_RUMBLE_DAMAGE = 10.f * 2.f;
const float BOSS_PROJECTILE_DAMAGE = 5.f;
const unsigned int BOSS_LEVEL = 3;
// const unsigned int BOSS_LEVEL = 1;
const unsigned int FINAL_BOSS_LEVEL = 5;
// const unsigned int FINAL_BOSS_LEVEL = 1;


const vec2 DENDERITE_SIZE = {64 * WORK_SCALE_FACTOR, 64 * WORK_SCALE_FACTOR};
const float DENDERITE_RECALC_DURATION = 3000.f;
const float DENDERITE_PROJECTILE_SPEED = PROJECTILE_SPEED * 2.f;

// OTHER CONSTANTS
const float DASH_DURATION_MS = 500.0f;
const float VELOCITY_DECAY_RATE = 1.01f; // 0.95f;
const float MAX_VELOCITY_DECAY_RATE = 1.04f;
const float MAX_PROJECTILE_SPEED = 1500;
const float MIN_DETECTION_RANGE = 0.35f;

// FINAL BOSS STATE TIMING
const float FINAL_BOSS_BASE_COOLDOWN = 3000.f;
const float FINAL_BOSS_SHOOT_DURATION = 5000.f; // how long it shoots
const float FINAL_BOSS_TIRED_COOLDOWN = 10000; // how long you can attack boss

// FINAL BOSS SHOOTING COOLDOWNS
const float FINAL_BOSS_BASE_SHOOT_COOLDOWN = 750.f; // cooldown between shots
const float FINAL_BOSS_EYEBALL_SHOOT_COOLDOWN = 5000.f; // cooldown between shooting eyes
// LOGO
const float LOGO_WIDTH_PX = 383 * WORK_SCALE_FACTOR;
const float LOGO_HEIGHT_PX = 122 * WORK_SCALE_FACTOR;

// CUTSCENES
const float INTRO_CUTSCENE_DURATION_MS = 3 * MS_PER_S; // for animation after clicking start
const float BOOT_CUTSCENE_DURATION_MS = 2 * MS_PER_S;  // for logo movement at boot
const float GAMEPLAY_CUTSCENE_DURATION_MS = 3 * MS_PER_S;
const float STATE_TIMER_DEFAULT = 3 * MS_PER_S;
const float WIN_CUTSCENE_DURATION_MS = 3 * MS_PER_S;

// button positions, scales
const vec2 LOGO_POSITION_INITIAL = {-WINDOW_WIDTH_PX / 2.f, -WINDOW_HEIGHT_PX / 3.5f};
const vec2 LOGO_POSITION = LOGO_POSITION_INITIAL + vec2(WINDOW_WIDTH_PX / 2.f, 0.f);

const vec2 START_BUTTON_SCALE = {128 * WORK_SCALE_FACTOR, 32 * WORK_SCALE_FACTOR};
const vec2 INFO_BUTTON_SCALE = {128 * WORK_SCALE_FACTOR, 32 * WORK_SCALE_FACTOR};
const vec2 SHOP_BUTTON_SCALE = {128 * WORK_SCALE_FACTOR, 32 * WORK_SCALE_FACTOR};
const vec2 BACK_BUTTON_SCALE = {128 * WORK_SCALE_FACTOR, 32 * WORK_SCALE_FACTOR};
const vec2 PROCEED_BUTTON_SCALE = {128 * WORK_SCALE_FACTOR, 32 * WORK_SCALE_FACTOR};
const vec2 RESUME_BUTTON_SCALE = {128 * WORK_SCALE_FACTOR, 32 * WORK_SCALE_FACTOR};
const vec2 SAVE_BUTTON_SCALE = {128 * WORK_SCALE_FACTOR, 32 * WORK_SCALE_FACTOR};

const float START_SCREEN_BUTTON_PADDING = 10 * WORK_SCALE_FACTOR;

const vec2 START_BUTTON_COORDINATES = {0.f, 0.f};
const vec2 SHOP_BUTTON_COORDINATES = {0.f, START_BUTTON_COORDINATES.y + START_BUTTON_SCALE.y + START_SCREEN_BUTTON_PADDING};
const vec2 INFO_BUTTON_COORDINATES = {0.f, SHOP_BUTTON_COORDINATES.y + START_BUTTON_SCALE.y + START_SCREEN_BUTTON_PADDING};
const vec2 BACK_BUTTON_COORDINATES = {-WINDOW_WIDTH_PX / 3.1f, WINDOW_HEIGHT_PX / 3.1f};
// BACKGROUND SCALE
const vec2 BACKGROUND_SCALE = vec2(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX);

// UI
const float UI_SCALE = 1.0f;
const float UI_MARGIN_X = 32;
const float UI_MARGIN_Y = 32;
const float UI_SPACING = 8;

// NUCLEUS MENU
const float NUCLEUS_MENU_NUCLEUS_WIDTH = 186 * WORK_SCALE_FACTOR;
const float NUCLEUS_MENU_NUCLEUS_HEIGHT = 148 * WORK_SCALE_FACTOR;
const float NUCLEUS_MENU_SLOT_WIDTH = 34 * WORK_SCALE_FACTOR;
const float NUCLEUS_MENU_SLOT_HEIGHT = 34 * WORK_SCALE_FACTOR;
const float NUCLEUS_MENU_SLOT_PADDING = 5 * WORK_SCALE_FACTOR;


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
	WINDOW_HEIGHT_PX / 2 - UI_MARGIN_Y - NUCLEUS_UI_HEIGHT / 2 - UI_SPACING -  HEALTH_BAR_HEIGHT / 2};

// dash recharge
const float DASH_WIDTH = 18 * UI_SCALE;
const float DASH_HEIGHT = 18 * UI_SCALE;
const float DASH_RADIUS = 40.0f * UI_SCALE;;

// const float DASH_RECHARGE_SPACING = (HEALTH_BAR_WIDTH - DASH_WIDTH) / (DASH_RECHARGE_COUNT - 1);
const float DASH_RECHARGE_SPACING = DASH_WIDTH + 8;
const vec2 DASH_RECHARGE_START_POS = {
	-WINDOW_WIDTH_PX / 2 + UI_MARGIN_X + NUCLEUS_UI_WIDTH + UI_SPACING + DASH_WIDTH / 2,
	WINDOW_HEIGHT_PX / 2 - UI_MARGIN_Y - NUCLEUS_UI_HEIGHT / 2}; // pos.y aligns w/ nuclues

// germoney
const float GERMONEY_UI_WIDTH = 48 * UI_SCALE;
const float GERMONEY_UI_HEIGHT = 22 * UI_SCALE;
const vec2 GERMONEY_UI_POS = {-WINDOW_WIDTH_PX / 2 + UI_MARGIN_X + NUCLEUS_UI_WIDTH + UI_SPACING + GERMONEY_UI_WIDTH / 2,
							  WINDOW_HEIGHT_PX / 2 - UI_MARGIN_Y - NUCLEUS_UI_HEIGHT / 2 + GERMONEY_UI_HEIGHT / 2};
// weapon pill
const float WEAPON_PILL_UI_WIDTH = 118 * UI_SCALE;
const float WEAPON_PILL_UI_HEIGHT = 58 * UI_SCALE;
const vec2 WEAPON_PILL_UI_POS = {WINDOW_WIDTH_PX / 2 - UI_MARGIN_X - WEAPON_PILL_UI_WIDTH / 2,
								 WINDOW_HEIGHT_PX / 2 - UI_MARGIN_Y - NUCLEUS_UI_HEIGHT / 2}; // pos.y aligns w/ nuclues
const vec2 GUN_UI_POS = {WEAPON_PILL_UI_POS.x - WEAPON_PILL_UI_WIDTH / 4,
                         WEAPON_PILL_UI_POS.y};
const vec2 GUN_COOLDOWN_INDICATOR_SCALE = {20.f * UI_SCALE, 20.f * UI_SCALE};

const std::unordered_map<int, std::string> BUFF_TYPE_TO_NAME = {
    {-1, "Level Skip"},
    {-2, "Nucleus Slots"},
    {0, "Tail"},
    {1, "Mitochondria"},
    {2, "Hemoglobin"},
    {3, "Golgi"},
    {4, "Chloroplash"},
    {5, "Cell Wall"},
    {6, "Amino Acid"},
    {7, "Lysosyme"},
    {8, "CytoPlasm"},
    {9, "Virality"},
    {10, "Spare Nucleus"},
    {11, "Vacuole"},
    {12, "Endoplasmic Reticulum"},
    {13, "Oceloid"},
    {14, "Secretor"},
    {15, "Orange"},
    {16, "Peroxisomes"},
    {17, "Mutation"},
    {18, "Facehugger"},
    {19, "Black Goo"}
};

// -M4 Feature: GAME BALANCING
const float BUFF_DROP_CHANCE = 0.6f;
const float BUFF_DROP_FAIL_CHANCE = 1.0f - BUFF_DROP_CHANCE;

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

const float POPUP_BUFF_UI_WIDTH = 40.0f * UI_SCALE;
const float POPUP_BUFF_UI_HEIGHT = 40.0f * UI_SCALE;
const vec2 BUFF_POPUP_POS = { -WINDOW_WIDTH_PX / 8, WINDOW_HEIGHT_PX / 3.5 };
const float BUFF_POPUP_GAP = 10;
const float POPUP_DURATION = 5000;

// -M4 Feature: GAME BALANCING
// DANGER THERMOMETER 
const float DEFAULT_DANGER_LEVEL = 1.0f;
const float MAX_DANGER_LEVEL = 5.0f;
const float DANGER_INCREASE_INTERVAL = 30000.f; // 30 seconds
const float DANGER_INCREASE_AMOUNT = 0.1f;
const int THERMOMETER_WIDTH = 24 * WORK_SCALE_FACTOR;
const int THERMOMETER_HEIGHT = 104 * WORK_SCALE_FACTOR;
const vec2 THERMOMETER_POS = {20 -WINDOW_WIDTH_PX/2 +THERMOMETER_WIDTH/2, 20 -WINDOW_HEIGHT_PX/2 +THERMOMETER_HEIGHT/2};

// ENEMY HP BAR
const float ENEMY_HP_BAR_WIDTH = 34 * 1.5;
const float ENEMY_HP_BAR_HEIGHT = 10 * 1.5;

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

vec2 positionToGridCell(vec2 position);
vec2 gridCellToPosition(vec2 gridCell);

const vec2 SHOPKEEPER_SIZE = {274.f * WORK_SCALE_FACTOR, 203.f * WORK_SCALE_FACTOR};
const vec2 PURCHASE_BOX_SCALE = {172.f * WORK_SCALE_FACTOR, 72.f * WORK_SCALE_FACTOR};
const vec2 SHOP_PLATE_SCALE = {WORK_SCALE_FACTOR * 34.0, WORK_SCALE_FACTOR * 34.0};