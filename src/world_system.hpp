#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>
#include <map>
#include <memory>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"
#include "particle_system.hpp"
#include "physics_system.hpp"

class ParticleSystem;

enum class GameState
{
	START_SCREEN_ANIMATION,
	START_SCREEN,
	GAME_PLAY,
	GAME_OVER,
	PAUSE,
	SHOP,
	INFO,
	GAMEPLAY_CUTSCENE,
    NEXT_LEVEL,
	VICTORY
};

// Container for all our entities and game logic.
// Individual rendering / updates are deferred to the update() methods.
class WorldSystem
{
public:
	WorldSystem();

	// creates main window
	GLFWwindow *create_window();

	// starts and loads music and sound effects
	bool start_and_load_sounds();

	// call to close the window
	void close_window();

	// starts the game
	void init(RenderSystem *renderer);

	// releases all associated resources
	~WorldSystem();

	// steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	// check for collisions generated by the physics system
	void handle_collisions();

	// should the game be over ?
	bool is_over() const;

	// for game saving / loading
	void saveGame();
	void loadGame();
	void saveProgress();
	void loadProgress();

	bool checkLoadFileExists();

	GameState current_state = GameState::START_SCREEN_ANIMATION;
	GameState previous_state = GameState::START_SCREEN_ANIMATION;

	float stateTimer = BOOT_CUTSCENE_DURATION_MS;
	void collectBuff(Entity player_entity, Entity buff_entity);

	void handlePlayerHealth(float elapsed_ms);

	std::map<std::string, bool> progress_map = {
		{"tutorial_mode", true}
	};

    void initiatePlayerDash();
    bool canDash();
    bool isDashing();

	void spawnEnemies(float elapsed_ms_since_last_update);
	void handleProjectiles(float elapsed_ms_since_last_update);
	bool checkPortalCollision();
	void spawnFourDenderitesOnMap();
	void startTheme();
	void triggerGameOver();


private:
	bool gameOver = false;
	bool shopItemsPlaced = false;
	bool shopScreenCreated = false;
	bool firstEnemySpawned = false;

	float device_mouse_pos_x = 0.0f;
	float device_mouse_pos_y = 0.0f;

	float game_mouse_pos_x = 0.0f;
	float game_mouse_pos_y = 0.0f;

	// input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_mouse_button_pressed(int button, int action, int mods);

    void shootGun();

	// to get the clicked button
	ButtonType getClickedButton();
	// to check if button was clicked
	bool isButtonClicked(screenButton& button);

	// restart level
	void restart_game();
	void goToNextLevel();

	void updateCamera(float elapsed_ms);
	void updateMouseCoords();
	bool updateBoss();	
	void updateBossArrows();

	void handlePlayerMovement(float elapsed_ms_since_last_update);

    void handleVignetteEffect(float elapsed_ms_since_last_update);
	void handleRippleEffect(float elapsed_ms_since_last_update);

	// OpenGL window handle
	GLFWwindow *window;

	int next_enemy_spawn;
	int enemy_spawn_rate_ms; // see default value in common.hpp
	int next_projectile_ms;
	std::map<int, int> bacteriophage_idx;

	unsigned int points;

	unsigned int level = 0;

    // black screen for next level timer
    float darken_screen_timer = -1.0f;

	// Game state
	RenderSystem* renderer;
	float current_speed;

	// particle
	ParticleSystem particle_system;

    // physics 
    PhysicsSystem physics_system;

	// grid
	std::vector<Entity> grid_lines;

	// music references
	Mix_Music *background_music;
	Mix_Music *boss_background_music;
	Mix_Chunk *dash_sound;
	Mix_Chunk *player_shoot_sound;
	Mix_Chunk *damage_sound;
	Mix_Chunk *enemy_shoot_sound;
	Mix_Chunk *enemy_death_sound;
	Mix_Chunk *click_sound;
	Mix_Chunk *portal_sound;
	Mix_Chunk *buy_sound;
	Mix_Chunk *buff_pickup;

	// debugging (fps etc..)
	void toggleFPSDisplay();
	bool debug = false;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

    CollisionSystem detector;

	std::map<int, std::map<int, int>> currentTiles;
	bool initializedMap = false;
    void tileProceduralMap();

	bool isClickableBuffClicked(Entity* return_e);
	bool mouseBuffIntersect(vec2 mouse_pos, vec2 c_pos);
	void handleClickableBuff(Entity e);
	Entity getFreeSlot();
	bool isFreeSlot();
	void moveSelectedBuffsToProgression();
	void applyBuff(Player& player, BUFF_TYPE buff_type);

	void updateDangerLevel(float elapsed_ms_since_last_update);
	void placeBuffsOnShopScreen();
	void switchMusicThemeToBoss();
	void switchMusicBossToTheme();
};