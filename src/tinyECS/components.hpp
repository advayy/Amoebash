#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"
#include "../ext/json/json.hpp"

extern bool tutorial_mode;

enum BUFF_TYPE {
	TAIL = 0,
	MITOCHONDRIA = TAIL + 1,
	HEMOGLOBIN = MITOCHONDRIA + 1,
	GOLGI = HEMOGLOBIN + 1,
	CHLOROPLAST = GOLGI + 1,
	CELL_WALL = CHLOROPLAST + 1,
	AMINO_ACID = CELL_WALL + 1,
	LYSOSOME = AMINO_ACID + 1,
	CYTOPLASM = LYSOSOME + 1,
	PILLI = CYTOPLASM + 1,
	SPARE_NUCLEUS = PILLI + 1,
	VACUOLE = SPARE_NUCLEUS + 1,
	ENDOPLASMIC_RETICULUM = VACUOLE + 1,
	OCELOID = ENDOPLASMIC_RETICULUM + 1,
	SECRETOR = OCELOID + 1,
	ORANGE = SECRETOR + 1,
	PEROXISOMES = ORANGE + 1,
	MUTATION = PEROXISOMES + 1,
	FACEHUGGER = MUTATION + 1,
	BLACK_GOO = FACEHUGGER + 1,
   	INFO_BUFF0 = BLACK_GOO + 1,
	INFO_BUFF1 = INFO_BUFF0 + 1,
    INFO_BUFF2 = INFO_BUFF1 + 1,
    INFO_BUFF3 = INFO_BUFF2 + 1,
    INFO_BUFF4 = INFO_BUFF3 + 1,
    INFO_BUFF5 = INFO_BUFF4 + 1,
    INFO_BUFF6 = INFO_BUFF5 + 1,
    INFO_BUFF7 = INFO_BUFF6 + 1,
    INFO_BUFF8 = INFO_BUFF7 + 1,
    INFO_BUFF9 = INFO_BUFF8 + 1,
    INFO_BUFF10 = INFO_BUFF9 + 1,
    INFO_BUFF11 = INFO_BUFF10 + 1,
    INFO_BUFF12 = INFO_BUFF11 + 1,

    INFO_BOSS1 = INFO_BUFF12 + 1,
    INFO_BOSS2 = INFO_BOSS1 + 1,

	TOTAL_BUFF_COUNT = INFO_BOSS2 + 1,

	SLOT_INCREASE = TAIL - 2,
	INJECTION = TAIL - 1,
};

const int NUMBER_OF_BUFFS = (int)BUFF_TYPE::TOTAL_BUFF_COUNT;
// last enabled buff + 1
const int BUFFS_ENABLED = (int)BUFF_TYPE::SECRETOR + 1;

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
	std::unordered_map<BUFF_TYPE, int> buffsFromLastRun;
	std::vector<BUFF_TYPE> pickedInNucleus;
	int slots_unlocked = 1;
	int germoney_savings = 0;
};

struct Slot {
	int number = 0;
	bool filled = false;
};

struct ClickableBuff {
	BUFF_TYPE type;
	bool picked = false;
	float price = 0.0;
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
	float default_healing_timer = PLAYER_DEFAULT_HEALING_TIMER_MS;

	// Active cooldown timer and the default cooldown time
	int dash_count = DASH_RECHARGE_COUNT;
	int max_dash_count = DASH_RECHARGE_COUNT;
	
	int dash_cooldown_timer_ms = 0;
	int dash_cooldown_ms = PLAYER_DASH_COOLDOWN_MS;
	float dash_speed = PLAYER_DASH_SPEED;
	float dash_range = PLAYER_DASH_RANGE;

	float minimapViewRange = 3.0;
	float dashDecay = VELOCITY_DECAY_RATE;

	int sheilds = 0;

	float gun_projectile_damage = GUN_PROJECTILE_DAMAGE;
	int bulletsPerShot = 1;
	float angleConeRadius = 30;
	float bulletSpeed = GUN_PROJECTILE_SPEED;

	int extra_lives = 0;


	// Detection range for enemies
	float detection_range = 1.0f;

	float knockback_duration = 0.0f;

	vec2 grid_position = {0, 0};
	// std::vector<int> buffsCollected;
    std::unordered_map<BUFF_TYPE, int> buffsCollected;

    int germoney_count = 0;
	float dangerFactor = DEFAULT_DANGER_LEVEL;
};

struct Text
{
	std::string text;
	vec3 color;
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

struct Thermometer {};

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
	BUFF_TYPE type = TAIL; // Type of buff (0-19, corresponding to the sprite sheet)
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
	float darken_screen_factor = -1.f;
	float vignette_screen_factor = 0.f;
    float vignette_timer_ms = 0.f;
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
	BACKBUTTON = INFOBUTTON + 1,
	SAVEBUTTON = BACKBUTTON + 1,
	PROCEEDBUTTON = SAVEBUTTON + 1,
	RESUMEBUTTON = PROCEEDBUTTON + 1,
	NONE = RESUMEBUTTON + 1
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
	int health;
	bool is_enemy_hp_bar = false;
	Entity owner = {}; 
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
	BUFF_TYPE buffType;
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

struct Effect {
    float death_timer_ms;
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
	GAME_LOGO = TILE + 1,
	BUTTON = GAME_LOGO + 1,
	PAUSE = BUTTON + 1,
	SHOP_BUTTON = PAUSE + 1,
	SHOP_BUTTON_ON_HOVER = SHOP_BUTTON + 1,
	NUCLEUS = SHOP_BUTTON_ON_HOVER + 1,
	NOSE = NUCLEUS + 1,
	CUTSCENEBACKGROUND = NOSE + 1,
	NOSEACCENT = CUTSCENEBACKGROUND + 1,
	ENTERINGNUCLEUS = NOSEACCENT + 1,
	NUCLEUS_UI = ENTERINGNUCLEUS + 1,
	HEALTH_BAR_UI = NUCLEUS_UI + 1,
	DASH_UI = HEALTH_BAR_UI + 1,
	GERMONEY_UI = DASH_UI + 1,
	WEAPON_PILL_UI = GERMONEY_UI + 1,
	INFO_BUTTON = WEAPON_PILL_UI + 1,
	INFO_BUTTON_ON_HOVER = INFO_BUTTON + 1,
	START_SCREEN_BG = INFO_BUTTON_ON_HOVER + 1,
	OUTLINE_BUTTON = START_SCREEN_BG + 1,
	BUFFS_SHEET = OUTLINE_BUTTON + 1,
	PORTAL = BUFFS_SHEET + 1,
	DEATH_PARTICLE = PORTAL + 1,
	PIXEL_PARTICLE = DEATH_PARTICLE + 1,
	GUN = PIXEL_PARTICLE + 1,
	GUN_STILL = GUN + 1,
	GUN_PROJECTILE = GUN_STILL + 1,
	BOSS_PROJECTILE = GUN_PROJECTILE + 1,
	NUCLEUS_MENU = BOSS_PROJECTILE + 1,
	NUCLEUS_MENU_SLOT = NUCLEUS_MENU + 1,
	BOSS_STAGE_1 = NUCLEUS_MENU_SLOT + 1,
	BOSS_STAGE_2 = BOSS_STAGE_1 + 1,
	BOSS_STAGE_3 = BOSS_STAGE_2 + 1,
	BOSS_STAGE_4 = BOSS_STAGE_3 + 1,
	START_BUTTON = BOSS_STAGE_4 + 1,
	START_BUTTON_ON_HOVER = START_BUTTON + 1,
	BACK_BUTTON = START_BUTTON_ON_HOVER + 1,
	BACK_BUTTON_ON_HOVER = BACK_BUTTON + 1,
	PROCEED_BUTTON = BACK_BUTTON_ON_HOVER + 1,
	PROCEED_BUTTON_ON_HOVER = PROCEED_BUTTON + 1,
	SAVE_BUTTON = PROCEED_BUTTON_ON_HOVER + 1,
	SAVE_BUTTON_ON_HOVER = SAVE_BUTTON + 1,
	RESUME_BUTTON = SAVE_BUTTON_ON_HOVER + 1,
	RESUME_BUTTON_ON_HOVER = RESUME_BUTTON + 1,
	BOSS_ARROW = RESUME_BUTTON_ON_HOVER + 1,
	WINSCREEN = BOSS_ARROW + 1,
	THERMOMETER = WINSCREEN + 1,
    INJECTION = THERMOMETER + 1,
	PURCHASE_BOX = INJECTION +1,
	SHOP_PLATE = PURCHASE_BOX + 1,
	SHOPKEEPER = SHOP_PLATE + 1,
	SLOT_INCREASE_BUFF = SHOPKEEPER + 1,
    ENEMY_HP_BAR = SLOT_INCREASE_BUFF + 1, 
	MITOSIS_BOSS_16_HP_BAR = ENEMY_HP_BAR + 1,
	MITOSIS_BOSS_128_HP_BAR = MITOSIS_BOSS_16_HP_BAR + 1,
	SPIKE_ENEMY_EXPLOSION_EFFECT = MITOSIS_BOSS_128_HP_BAR + 1,
	RBC_ENEMY_EXPLOSION_EFFECT = SPIKE_ENEMY_EXPLOSION_EFFECT + 1,
	BACTERIOPHAGE_ENEMY_PROJECTILE_EFFECT = RBC_ENEMY_EXPLOSION_EFFECT + 1,
	GUN_PROJECTILE_EFFECT = BACTERIOPHAGE_ENEMY_PROJECTILE_EFFECT + 1,	
	INFO_BUFF = GUN_PROJECTILE_EFFECT + 1,
	RED_TILES = INFO_BUFF + 1,
	RED_WALL = RED_TILES + 1,
	GREEN_TILES = RED_WALL + 1,
	GREEN_WALL = GREEN_TILES + 1,
	BLUE_TILES = GREEN_WALL + 1,
	BLUE_WALL = BLUE_TILES + 1,
	PURPLE_TILES = BLUE_WALL + 1,
	PURPLE_WALL = PURPLE_TILES + 1,
    BOSS_TILES = PURPLE_WALL + 1,
    BOSS_WALL = BOSS_TILES + 1,
    EYE_BALL_PROJECTILE = BOSS_WALL + 1,
	DENDERITE = EYE_BALL_PROJECTILE + 1,
	FINAL_BOSS = DENDERITE + 1,
	TEXTURE_COUNT = FINAL_BOSS + 1
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
    FONT = PARTICLE_EFFECT + 1,
    THERMOMETER_EFFECT = FONT + 1,
	WEAPON_COOLDOWN_INDICATOR = THERMOMETER_EFFECT + 1,
    EFFECT_COUNT = WEAPON_COOLDOWN_INDICATOR + 1,

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
	
	// Simple position tracking for collision detection
	float previousPositionX = 0.0f;
	bool hasPreviousPosition = false;
	
	float bombTimer = SPIKE_ENEMY_BOMB_TIMER;
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

	float timeSinceLastRecalc = 0.f;
    float recalcTimeThreshold = DENDERITE_RECALC_DURATION;
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
	float cool_down = FINAL_BOSS_BASE_COOLDOWN;
	bool has_spawned = false;

	float shoot_cool_down = FINAL_BOSS_BASE_SHOOT_COOLDOWN;
	float spiral_duration = FINAL_BOSS_SHOOT_DURATION; 

	Entity associatedArrow;
};

enum class PARTICLE_TYPE 
{
    DEATH_PARTICLE = 0,
    // add more particle types here
	RIPPLE_PARTICLE = 1,
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

//biomes
enum class Biome {
  RED = 0,
  GREEN = RED + 1,
  BLUE = GREEN + 1,
  PURPLE = BLUE + 1,
  BOSS = PURPLE + 1,
  BIOME_COUNT = BOSS + 1
};

struct PopupWithImage {
	Entity text;
	Entity description;
	Entity image;
	float duration;

	PopupWithImage(const Entity& text, const Entity& description, const Entity& image, float duration)
		: text(text), description(description), image(image), duration(duration) {}
};

struct PopupElement {};

const std::map<BUFF_TYPE, std::pair<std::string, std::string>> BUFF_TYPE_TO_TEXT = {
	{TAIL, {"Flagella", "Increases movement speed"}},
	{MITOCHONDRIA, {"Mitochondria", "Gives you 1 more dash bubble"}},
	{HEMOGLOBIN, {"Hemoglobin", "Reduces enemy detection range"}},
	{GOLGI, {"Golgi Apparatus", "Increases dash recharge rate"}},
	{CHLOROPLAST, {"Chloroplast", "Enables healing"}},
	{CELL_WALL, {"Cell Wall", "Negate the next time you take damage"}},
	{AMINO_ACID, {"Amino Acid", "Increases your Dash damage"}},
	{LYSOSOME, {"Lysosome", "Shoot 1 more projectile"}},
	{CYTOPLASM, {"Cytoplasm", "Increases your overall health"}},
	{PILLI, {"Pilli", "Projectile Speed Bost"}},
	{SPARE_NUCLEUS, {"Spare Nucleus", "1+ Lives"}},
	{VACUOLE, {"Vacuole", "Heals some health instantly"}},
	{ENDOPLASMIC_RETICULUM, {"Endoplasmic Reticulum", "Increase healing rate"}},
	{OCELOID, {"Oceloid", "Increases mini-map view range"}},
	{SECRETOR, {"Secretor", "Increases dash drift"}},
	{ORANGE, {"Orange", "Reduce bullet spread"}},
	{PEROXISOMES, {"Peroxisomes", "Not Implemented"}},
	{MUTATION, {"Mutation", "Not Implemented"}},
	{FACEHUGGER, {"Facehugger", "Not Implemented"}},
	{BLACK_GOO, {"Black Goo", "Not Implemented"}},
	{INFO_BUFF0, {"Story", "You're the nucleus of a brain eating Amoeba, and its time to get to work"}},
    {INFO_BUFF1, {"Moving", "Use mouse to move around"}},
    {INFO_BUFF2, {"Dashing", "Left-click to dash, the bubbles by you are your dash count"}},
    {INFO_BUFF3, {"Pause", "Use 'Space' to pause the game"}},
    {INFO_BUFF4, {"Save/Load", "Save on pause, use 'L' to load game in the main menu"}},
    {INFO_BUFF6, {"Attack", "Dash into enemies to damage them"}},
    {INFO_BUFF7, {"Shooting", "Use 'S', or right-click to shoot enemies from your pet bacteriophage"}},
    {INFO_BUFF5, {"Buffs", "Collect buffs by killing enemies, buffs improve your abilities"}},
    {INFO_BUFF8, {"Nucleus Menu", "Select buffs to take into your next run when you die"}},
    {INFO_BUFF9, {"Germoney", "Kill enemies to collect Germoney, use it in the shop"}},
    {INFO_BUFF10, {"Immunoresponse meter", "The thermometer on the top left shows the danger level"}},
    {INFO_BUFF11, {"Game", "Five levels and Two bosses await"}},
    {INFO_BUFF12, {"Portal", "Find and enter portals to go to the next level"}},
    {INFO_BOSS1, {"Mitosis", "Kill the bosses before they kill you"}},
    {INFO_BOSS2, {"Brain", "attack when its not electrified!, and after defeating the dendrites!"}},
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
	default_healing_timer,
	dash_count,
	max_dash_count,
	dash_cooldown_timer_ms,
	dash_cooldown_ms,
	dash_speed,
	dash_range,
	minimapViewRange,
	dashDecay,
	sheilds,
	gun_projectile_damage,
	bulletsPerShot,
	angleConeRadius,
	bulletSpeed,
	extra_lives,
	detection_range,
	knockback_duration,
	grid_position,
	buffsCollected,
	germoney_count,
	dangerFactor
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
	health,
	is_enemy_hp_bar,
	owner
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
	slots_unlocked,
	germoney_savings
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Effect,
	death_timer_ms
)

