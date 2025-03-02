#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"

struct Player
{
	int health = PLAYER_HEALTH;
	int speed = PLAYER_SPEED;
	int dash_cooldown_ms = 0;
	int dash_damage = PLAYER_DASH_DAMAGE;
	vec2 grid_position = { 0, 0 };
};

struct Dashing {
	float timer_ms = 700.0f;
	float angle    = 0.0;
	float speed_factor = 1.0;
};

struct SpriteSize {
	int width = 32;
	int height = 32;
};

struct Tile {
	int grid_x = 0;
	int grid_y = 0;
};

struct Map {
	// 2D array of numbers representing the map
	int width = 20; // This is in chunks of grid cells
	int height = 20;
	int top=0;
	int left=0;
	int bottom=0;
	int right=0;
};

struct MiniMap {
};

struct Camera {
    vec2 position = { 0, 0 };
	bool initialized = false;
	vec2 grid_position = { 0, 0 };
};

// Invader
struct Enemy {
	int health;
};

// Projectile
struct Projectile {
	int damage;
};

// used for Entities that cause damage
struct Deadly
{

};

// COMPONENTS FOR BUFFS?
struct Buff {
};


// All data relevant to the shape and motion of entities
struct Motion {
	vec2  position = { 0, 0 };
	float angle    = 0;
	vec2  velocity = { 0, 0 };
	vec2  scale    = { 10, 10 };
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity& other) { this->other = other; };
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	float darken_screen_factor = -1;
	float vignette_screen_factor = -1;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// used to hold grid line start and end positions
struct GridLine {
	vec2 start_pos = {  0,  0 };
	vec2 end_pos   = { 10, 10 };	// default to diagonal line
};

// A timer that will be associated to dying chicken
struct DeathTimer
{
	float counter_ms = 3000;
};

// A timer associated to vignette
struct VignetteTimer
{
	float counter_ms = 1000;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & chicken.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

// Button Types
enum ButtonType {
	STARTBUTTON = 0,
	SHOPBUTTON = STARTBUTTON + 1,
	INFOBUTTON = SHOPBUTTON + 1
};

// Coordinates and bounding box of start button on start screen
struct screenButton
{
	float w;
	float h;
	vec2 center;
	ButtonType type;
};

enum ScreenType {
	START = 0,
	GAMEPLAY = START + 1,
	INFO = GAMEPLAY + 1,
	SHOP = INFO + 1,
	NUCLEUS = SHOP + 1,
	GAMEOVER = NUCLEUS + 1,
	PAUSE = GAMEOVER + 1
};

struct GameScreen {
	ScreenType type;
	std::vector<screenButton> screenButtons;
};

struct Pause {

};

struct Over {

};

struct Start {
	std::vector<Entity> buttons;
};

struct GameplayCutScene {

};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
	ENEMY = 0,
	PLAYER = ENEMY + 1,
	PROJECTILE = PLAYER + 1,
	TILE = PROJECTILE + 1,
	PARALAX_TILE = TILE + 1,
	SCREEN = PARALAX_TILE + 1,
	GAMEOVER = SCREEN + 1,
	BUTTON = GAMEOVER + 1,
	PAUSE = BUTTON + 1,
	SHOPBUTTON = PAUSE + 1,
	NUCLEUS = SHOPBUTTON + 1,
	SHOPSCREEN = NUCLEUS + 1,
	INFOSCREEN = SHOPSCREEN + 1,
	WALL_TILE = INFOSCREEN + 1,
	NOSE = WALL_TILE + 1,
	CUTSCENEBACKGROUND = NOSE + 1,
	NOSEACCENT = CUTSCENEBACKGROUND + 1,
	ENTERINGNUCLEUS = NOSEACCENT + 1,
	RBC = ENTERINGNUCLEUS + 1,
	TEXTURE_COUNT = RBC + 1, 
};

const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	LINE = COLOURED + 1,
	TEXTURED = LINE + 1,
	VIGNETTE = TEXTURED + 1,
	SPRITE_SHEET = VIGNETTE + 1,
	TILE = SPRITE_SHEET + 1,
	MINI_MAP = TILE + 1,
	EFFECT_COUNT = MINI_MAP + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	SPRITE = 0,
	LINE = SPRITE + 1,
	DEBUG_LINE = LINE + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};

const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID   used_texture  = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID    used_effect   = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};

enum class ANIM_LOOP_TYPES 
{
	NO_LOOP = 0,
	LOOP = NO_LOOP + 1,
	PING_PONG = LOOP + 1
};

struct SpriteSheetImage 
{
	int total_frames = 0;
	int current_frame = 0;
};

// Animation frame
struct Animation
{
	int start_frame =				0;
	int end_frame =					0;
	float time_since_last_frame =	0.0f;
	float time_per_frame =			0.0f;
	ANIM_LOOP_TYPES loop =			ANIM_LOOP_TYPES::NO_LOOP;
	bool forwards =					true;
};

enum class PLAYER_FRAMES 
{
	FRAME_0 = 0,
	FRAME_1 = FRAME_0 + 1,
	FRAME_2 = FRAME_1 + 1,
	FRAME_3 = FRAME_2 + 1,
	FRAME_4 = FRAME_3 + 1,
	FRAME_5 = FRAME_4 + 1,
	FRAME_6 = FRAME_5 + 1,
	FRAME_7 = FRAME_6 + 1,
	FRAME_8 = FRAME_7 + 1,
	FRAME_COUNT = FRAME_8 + 1
};

const int player_idle_start = (int)PLAYER_FRAMES::FRAME_0;
const int player_idle_end = (int)PLAYER_FRAMES::FRAME_3;
const int player_dash_start = (int)PLAYER_FRAMES::FRAME_4;
const int player_dash_end = (int)PLAYER_FRAMES::FRAME_7;
const int total_player_frames = (int)PLAYER_FRAMES::FRAME_COUNT;

struct DamageCooldown {
	uint last_damage_time;
};

enum class EnemyState {
	CHASING = 0,
	PATROLLING = CHASING + 1,
	DASHING = PATROLLING + 1,
	RUNAWAY = DASHING + 1
};

struct EnemyBehavior {
	EnemyState state = EnemyState::PATROLLING;
	// float dashCooldown = 0.7f;       // cooldown before enemy can dash again
	bool patrolForwards = true;  // initial patrol direction
	float patrolSpeed = ENEMY_PATROL_SPEED_PER_MS;     // patrol speed 
	float detectionRadius = ENEMY_DETECTION_RADIUS; // radius in which enemy detects player
	vec2 patrolOrigin = { 0, 0 };     // origin of patrol
	float patrolRange = ENEMY_PATROL_RANGE;     // range of patrol
	float patrolTime = ENEMY_PATROL_TIME_MS / 2;
};

enum class EnemyTypes {
	SPIKE,
	RED_BLOOD_CELL
};

struct EnemyType {
	EnemyTypes type;
};


struct EnemyConfig {
    TEXTURE_ASSET_ID texture;
    int totalFrames;
    int spriteWidth;
    int spriteHeight;
};

