#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"
#include "../ext/json/json.hpp"

extern bool tutorial_mode;

using json = nlohmann::json;

// asked gpt for this
namespace nlohmann {
    template <>
    struct adl_serializer<glm::vec2>
    {
        static void to_json(json& j, const glm::vec2& v) {
            j = json{{"x", v.x}, {"y", v.y}};
        }

        static void from_json(const json& j, glm::vec2& v) {
            j.at("x").get_to(v.x);
            j.at("y").get_to(v.y);
        }
    };
}

struct Progression {
	std::vector<int> buffsFromLastRun;
	std::vector<int> pickedInNucleus;
	int slots_unlocked = 9;
};


struct Slot {
	int number = 0;
	bool filled = false;
};

struct ClickableBuff {
	int type;
	bool picked = false;
	vec2 returnPosition = {0, 0};
	Entity slotEntity;
};


struct Player
{
	int current_health = PLAYER_DEFAULT_HEALTH;
	int max_health = PLAYER_DEFAULT_HEALTH;

	int speed = PLAYER_SPEED;
	int dash_damage = PLAYER_DASH_DAMAGE;
	float healing_rate = PLAYER_BASE_HEALING_RATE;
	float healing_timer_ms = PLAYER_DEFAULT_HEALING_TIMER_MS;
	
	// Active cooldown timer and the default cooldown time
	int dash_count = DASH_RECHARGE_COUNT;
	int max_dash_count = DASH_RECHARGE_COUNT;
	
	int dash_cooldown_timer_ms = 0;
	int dash_cooldown_ms = PLAYER_DASH_COOLDOWN_MS;
	float dash_speed = PLAYER_DASH_SPEED;
	float dash_range = PLAYER_DASH_RANGE;

	// Detection range for enemies
	float detection_range = 1.0f;

	float knockback_duration = 0.0f;

	vec2 grid_position = {0, 0};
	std::vector<int> buffsCollected;
};

struct Dashing
{
	vec2 velocity = { 0, 0 };
	float angle_deg = 0.0f;
	float timer_ms = 700.0f;
};

struct SpriteSize
{
	int width = 32;
	int height = 32;
};

struct Tile
{
	int grid_x = 0;
	int grid_y = 0;
};

struct Map
{
	// 2D array of numbers representing the map
	int width = 20; // This is in chunks of grid cells
	int height = 20;
	int top = 0;
	int left = 0;
	int bottom = 0;
	int right = 0;
};

enum class tileType {
	EMPTY = 0,
	WALL = 1,
    PORTAL = 2
};

struct ProceduralMap {
	// 2D array of numbers representing the map
	std::vector<std::vector<tileType>> map;

	int width = 20; // This is in chunks of grid cells
	int height = 20;
	int top=0;
	int left=0;
	int bottom=0;
	int right=0;
};

struct Portal {
    int grid_x = 0;
    int grid_y = 0;
};

struct MiniMap {
	std::vector<std::vector<int>> visited;
	int dummy = 0;
};

struct Camera
{
	vec2 position = {0, 0};
	bool initialized = false;
	vec2 grid_position = {0, 0};
};

// Invader
struct Enemy
{
	int health;
	int total_health;
};

// Projectile
struct Projectile 
{
	int damage;
	float ms_until_despawn = PROJECTILE_TTL_MS;
	bool from_enemy = true;
};

struct SpiralProjectile
{
};

struct FollowingProjectile
{
};

struct BacteriophageProjectile {
	int dummy = 0;
};

struct BossProjectile {};
struct FinalBossProjectile {};

// used for Entities that cause damage
struct Deadly
{
	int dummy = 0;
};

// Buff
struct Buff
{
	int type = 0; // Type of buff (0-19, corresponding to the sprite sheet)
	bool collected = false;
};

// All data relevant to the shape and motion of entities
struct Motion
{
	vec2 position = {0, 0};
	float angle = 0;
	vec2 velocity = {0, 0};
	vec2 scale = {10, 10};
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity &other) { this->other = other; };
};

struct Wall {
	int dummy = 0;
};

// Data structure for toggling debug mode
struct Debug
{
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
	int dummy = 0;
};

// used to hold grid line start and end positions
struct GridLine
{
	vec2 start_pos = {0, 0};
	vec2 end_pos = {10, 10}; // default to diagonal line
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
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex> &out_vertices, std::vector<uint16_t> &out_vertex_indices, vec2 &out_size);
	vec2 original_size = {1, 1};
	std::vector<ColoredVertex> vertices;
	std::vector<TexturedVertex> textured_vertices;
	std::vector<uint16_t> vertex_indices;
};

// Button Types
enum ButtonType
{
	STARTBUTTON = 0,
	SHOPBUTTON = STARTBUTTON + 1,
	INFOBUTTON = SHOPBUTTON + 1,
	BACKBUTTON = INFOBUTTON,
	SAVEBUTTON = BACKBUTTON + 1,
	PROCEED_BUTTON = SAVEBUTTON + 1,
	NONE = PROCEED_BUTTON + 1
};

// Coordinates and bounding box of start button on start screen
struct screenButton
{
	float w;
	float h;
	vec2 center;
	ButtonType type;
};

struct Logo 
{
	int dummy = 0;
};

enum ScreenType
{
	START = 0,
	GAMEPLAY = START + 1,
	INFO = GAMEPLAY + 1,
	SHOP = INFO + 1,
	NUCLEUS = SHOP + 1,
	GAMEOVER = NUCLEUS + 1,
	PAUSE = GAMEOVER + 1,
    NEXT_LEVEL = PAUSE + 1,
};

struct GameScreen
{
	ScreenType type;
	std::vector<screenButton> screenButtons;
	Entity logo;
};

struct Pause
{
	int dummy = 0;
};

struct Over
{
	int dummy = 0;
	std::vector<Entity> buttons;
};

struct Start
{
	std::vector<Entity> buttons;
	Entity logo;
};

struct Shop 
{
	std::vector<Entity> buttons;
};

struct Info
{
	std::vector<Entity> buttons;
};

struct GameplayCutScene
{
	int dummy = 0;
};

// UI Elements
struct UIElement // default / static ui
{
	vec2 position;
	vec2 scale;
};

struct HealthBar
{
	vec2 position;
	vec2 scale;
	int health;
};

struct DashRecharge
{
	int dummy = 0;
};

struct Key
{
	bool inserted = false;
};

struct Chest
{
	bool gotKey = false;
};


struct BuffUI
{
	int buffType;
};

struct InfoBox
{
	int dummy = 0;
};

struct Gun {
    float cooldown_timer_ms = 0.0f;
};

struct BossArrow {
	Entity associatedBoss;
	bool draw = false;
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

enum class TEXTURE_ASSET_ID
{
	SPIKE_ENEMY = 0,
	RBC_ENEMY = SPIKE_ENEMY + 1,
	BACTERIOPHAGE = RBC_ENEMY + 1,
	PLAYER = BACTERIOPHAGE + 1,
	PROJECTILE = PLAYER + 1,
	TILE = PROJECTILE + 1,
	PARALAX_TILE = TILE + 1,
	GAME_LOGO = PARALAX_TILE + 1,
	GAMEOVER = GAME_LOGO + 1,
	BUTTON = GAMEOVER + 1,
	PAUSE = BUTTON + 1,
	SHOP_BUTTON = PAUSE + 1,
	NUCLEUS = SHOP_BUTTON + 1,
	SHOPSCREEN = NUCLEUS + 1,
	INFOSCREEN = SHOPSCREEN + 1,
	WALL_TILE = INFOSCREEN + 1,
	NOSE = WALL_TILE + 1,
	CUTSCENEBACKGROUND = NOSE + 1,
	NOSEACCENT = CUTSCENEBACKGROUND + 1,
	ENTERINGNUCLEUS = NOSEACCENT + 1,
	NUCLEUS_UI = ENTERINGNUCLEUS + 1,
	HEALTH_BAR_UI = NUCLEUS_UI + 1,
	DASH_UI = HEALTH_BAR_UI + 1,
	GERMONEY_UI = DASH_UI + 1,
	WEAPON_PILL_UI = GERMONEY_UI + 1,
	INFO_BUTTON = WEAPON_PILL_UI + 1,
	START_SCREEN_BG = INFO_BUTTON + 1,
	BACK_BUTTON = START_SCREEN_BG + 1,
	KEY = BACK_BUTTON + 1,
	BUFFS_SHEET = KEY + 1,
	PORTAL = BUFFS_SHEET + 1,
	MOUSE_CONTROL_INFO = PORTAL + 1,
	PAUSE_INFO = MOUSE_CONTROL_INFO + 1,
	DASH_INFO = PAUSE_INFO + 1,
	ENEMY_INFO = DASH_INFO + 1,
	RESTART_INFO = ENEMY_INFO + 1,
	LEAVE_TUTORIAL = RESTART_INFO + 1,
	CHEST = LEAVE_TUTORIAL + 1,
	PARTICLE = CHEST + 1,
	GUN = PARTICLE + 1,
	GUN_STILL = GUN + 1,
	GUN_PROJECTILE = GUN_STILL + 1,
	BOSS_PROJECTILE = GUN_PROJECTILE + 1,
	NUCLEUS_MENU = BOSS_PROJECTILE + 1,
	NUCLEUS_MENU_SLOT = NUCLEUS_MENU + 1,
	BOSS_STAGE_1 = NUCLEUS_MENU_SLOT + 1,
	BOSS_STAGE_2 = BOSS_STAGE_1 + 1,
	BOSS_STAGE_3 = BOSS_STAGE_2 + 1,
	BOSS_STAGE_4 = BOSS_STAGE_3 + 1,
	BOSS_ARROW = BOSS_STAGE_4 + 1,
	WINSCREEN = BOSS_ARROW + 1,
	EYE_BALL_PROJECTILE = WINSCREEN + 1,
	DENDERITE = EYE_BALL_PROJECTILE + 1,
	TEXTURE_COUNT = DENDERITE + 1
};

const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID
{
	COLOURED = 0,
	LINE = COLOURED + 1,
	TEXTURED = LINE + 1,
	VIGNETTE = TEXTURED + 1,
	SPRITE_SHEET = VIGNETTE + 1,
	TILE = SPRITE_SHEET + 1,
	MINI_MAP = TILE + 1,
	UI = MINI_MAP + 1,
	HEALTH_BAR = UI + 1,
	DASH_UI = HEALTH_BAR + 1,
	HEXAGON = DASH_UI + 1,
	PARTICLE_EFFECT = HEXAGON + 1,
	EFFECT_COUNT = PARTICLE_EFFECT + 1,

};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID
{
	SPRITE = 0,
	LINE = SPRITE + 1,
	DEBUG_LINE = LINE + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	HEXAGON = SCREEN_TRIANGLE + 1,
	GEOMETRY_COUNT = HEXAGON + 1
};

const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest
{
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
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
	int start_frame = 0;
	int end_frame = 0;
	float time_since_last_frame = 0.0f;
	float time_per_frame = 0.0f;
	ANIM_LOOP_TYPES loop = ANIM_LOOP_TYPES::NO_LOOP;
	bool forwards = true;
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

enum class PORTAL_FRAMES {
    FRAME_0 = 0,
    FRAME_1 = FRAME_0 + 1,
    FRAME_2 = FRAME_1 + 1,
    FRAME_3 = FRAME_2 + 1,
    FRAME_4 = FRAME_3 + 1,
    FRAME_5 = FRAME_4 + 1,
    FRAME_6 = FRAME_5 + 1,
    FRAME_7 = FRAME_6 + 1,
    FRAME_COUNT = FRAME_7 + 1
};
const int total_portal_frames = (int)PORTAL_FRAMES::FRAME_COUNT;

struct DamageCooldown
{
	uint last_damage_time;
};

struct EnemyAI
{
	bool patrolForwards = true;  // initial patrol direction
	float patrolSpeed = SPIKE_ENEMY_PATROL_SPEED_PER_MS;     // patrol speed 
	float detectionRadius = SPIKE_ENEMY_DETECTION_RADIUS; // radius in which enemy detects player
	vec2 patrolOrigin = { 0, 0 };     // origin of patrol
	float patrolRange = SPIKE_ENEMY_PATROL_RANGE;     // range of patrol
	float patrolTime = ENEMY_PATROL_TIME_MS / 2;
    float knockbackTimer = 0.f;
    float bombTimer = SPIKE_ENEMY_BOMB_TIMER;
};

enum class SpikeEnemyState
{
	CHASING = 0,
	PATROLLING = CHASING + 1,
	DASHING = PATROLLING + 1,
    KNOCKBACK = DASHING + 1
};

struct SpikeEnemyAI : EnemyAI
{
	SpikeEnemyState state = SpikeEnemyState::PATROLLING;
};

enum class RBCEnemyState
{
	FLOATING = 0,
	RUNAWAY = FLOATING + 1
};

struct RBCEnemyAI : EnemyAI
{
	RBCEnemyState state = RBCEnemyState::FLOATING;
};

enum class BacteriophageState
{
	CHASING = 0,
	PATROLLING = CHASING + 1
};

struct BacteriophageAI
{
	BacteriophageState state;
	float speed = ENEMY_SPEED;
	float detectionRadius = BACTERIOPHAGE_ENEMY_DETECTION_RADIUS; // radius in which enemy detects player
	float time_since_shoot_ms = 0.0f;
	bool can_shoot = false;
	int placement_index = 0;
};

enum class DenderiteState
{
	HUNT = 0,
	PIERCE = HUNT + 1,
	SHOOT = PIERCE + 1
};

struct DenderiteAI : EnemyAI
{
	// store whole path to follow 
	std::vector<ivec2> path;
	bool isCharging = false;
	float chargeTime = 100.0f;
	float chargeDuration = 500.0f;
	float shootCoolDown = 200.0f;
	int currentNodeIndex = 0;
	DenderiteState state = DenderiteState::HUNT;
};

enum class BossState
{
	INITIAL = 0,
	IDLE = INITIAL + 1,
	SHOOT_PARADE = IDLE + 1,
	RUMBLE = SHOOT_PARADE + 1,
	FLEE = RUMBLE + 1,
	NUM_STATES = FLEE + 1
};

enum class FinalBossState
{
	INITIAL = 0,
	SPAWN_1 = INITIAL + 1,
	SPIRAL_SHOOT_1 = SPAWN_1 + 1,
	TIRED = SPIRAL_SHOOT_1 + 1,
	SPAWN_2 = TIRED + 1,
};

struct BossAI : EnemyAI
{
	BossState state = BossState::INITIAL;
	int stage = 0;
	float cool_down;
	float shoot_cool_down;

	// RUMBLE-specific state
	float rumble_charge_time = 1500.f;  // time before rushing
	float rumble_duration = 1000.f;     // time spent rushing
	bool is_charging = true;
	vec2 projectile_size = BOSS_PROJECTILE;

	float flee_duration = 1000.f;    // Arbitrary duration in ms
	float flee_timer = 0.f;
	bool is_fleeing = false;

	Entity associatedArrow;
};

struct FinalBossAI : EnemyAI
{
	FinalBossState state = FinalBossState::INITIAL;
	unsigned int phase = 1;
	float cool_down = 20000.f;
	bool has_spawned = false;

	float shoot_cool_down = 300.f;
	float spiral_duration = 15000.f; 

	Entity associatedArrow;
};

enum class PARTICLE_TYPE 
{
    DEATH_PARTICLE = 0,
    // add more particle types here
    PARTICLE_TYPE_COUNT
};

enum class PARTICLE_STATE 
{
    BURST = 0,  
    FOLLOW = 1, 
    FADE = 2,   
};

// Particle component for particle system
struct Particle 
{
    PARTICLE_TYPE type;
    PARTICLE_STATE state = PARTICLE_STATE::BURST;
    float lifetime_ms = 2000.0f;
    float max_lifetime_ms = 2000.0f;
    float state_timer_ms = 0.0f;
    float speed_factor = 100.0f;
};

// MACROS for "to_json" and "from_json" on user-defined structs

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(vec2, x, y)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Player,
	current_health,
	max_health,
	speed,
	dash_damage,
	healing_rate,
	healing_timer_ms,
	dash_count,
	max_dash_count,
	dash_cooldown_timer_ms,
	dash_cooldown_ms,
	dash_speed,
	dash_range,
	detection_range,
	knockback_duration,
	grid_position,
	buffsCollected
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Dashing,
	velocity,
	angle_deg,
	timer_ms
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SpriteSize,
	width,
	height
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Tile,
	grid_x,
	grid_y
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Map,
	width,
	height,
	top,
	left,
	bottom,
	right
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ProceduralMap,
	map,
	width,
	height,
	top,
	left,
	bottom,
	right
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Portal,
	grid_x,
	grid_y
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MiniMap,
	dummy
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Camera,
	position,
	initialized,
	grid_position
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Enemy,
	health
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Projectile,
	damage,
	ms_until_despawn
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BacteriophageProjectile,
	dummy
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Deadly,
	dummy
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Buff,
	type,
	collected
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Motion,
	position,
	angle,
	velocity,
	scale
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Collision,
	other
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Wall,
	dummy
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Debug,
	in_debug_mode,
	in_freeze_mode
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ScreenState,
	darken_screen_factor,
	vignette_screen_factor
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DebugComponent,
	dummy
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GridLine,
	start_pos,
	end_pos
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DeathTimer,
	counter_ms
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VignetteTimer,
	counter_ms
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ColoredVertex,
	position,
	color
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TexturedVertex,
	position,
	texcoord
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Mesh,
	original_size,
	vertices,
	textured_vertices,
	vertex_indices
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(screenButton,
	w,
	h,
	center,
	type
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Logo,
	dummy
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GameScreen,
	type,
	screenButtons,
	logo
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Pause,
	dummy
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Over,
	dummy
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Start,
	buttons,
	logo
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Shop,
	buttons
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Info,
	buttons
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GameplayCutScene,
	dummy
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UIElement,
	position,
	scale
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HealthBar,
	position,
	scale,
	health
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DashRecharge,
	dummy
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Key,
	inserted
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Chest,
	gotKey
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BuffUI,
	buffType
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(InfoBox,
	dummy
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RenderRequest,
	used_texture,
	used_effect,
	used_geometry
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SpriteSheetImage,
	total_frames,
	current_frame
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Animation,
	start_frame,
	end_frame,
	time_since_last_frame,
	time_per_frame,
	loop,
	forwards
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DamageCooldown,
	last_damage_time
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EnemyAI,
	patrolForwards,
	patrolSpeed,
	detectionRadius,
	patrolOrigin,
	patrolRange,
	patrolTime
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SpikeEnemyAI,
	state
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RBCEnemyAI,
	state
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BacteriophageAI,
	state,
	speed,
	detectionRadius,
	time_since_shoot_ms,
	can_shoot,
	placement_index
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Particle,
	type,
	state,
	lifetime_ms,
	max_lifetime_ms,
	state_timer_ms,
	speed_factor
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Progression,
	buffsFromLastRun,
	pickedInNucleus,
	slots_unlocked
)