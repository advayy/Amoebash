// Header
#include "world_system.hpp"
#include "world_init.hpp"
#include "common.hpp"

// json library
#include "../ext/json/json.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <map>
// include lerp
#include <glm/gtx/compatibility.hpp>

#include "physics_system.hpp"
#include "particle_system.hpp"
#include "animation_system.hpp"
#include "ui_system.hpp"


// json object from json library
using json = nlohmann::json;

// create the world
WorldSystem::WorldSystem() : level(0),
							 next_enemy_spawn(0),
							 enemy_spawn_rate_ms(ENEMY_SPAWN_RATE_MS)
{
	// seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem()
{
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (dash_sound != nullptr)
		Mix_FreeChunk(dash_sound);
	if (player_shoot_sound != nullptr)
		Mix_FreeChunk(player_shoot_sound);
	if (damage_sound != nullptr)
		Mix_FreeChunk(damage_sound);
	if (enemy_death_sound != nullptr)
		Mix_FreeChunk(enemy_death_sound);
	if (enemy_shoot_sound != nullptr)
		Mix_FreeChunk(enemy_shoot_sound);
	if (click_sound != nullptr)
		Mix_FreeChunk(click_sound);
	if (boss_background_music != nullptr)
		Mix_FreeMusic(boss_background_music);
	if (portal_sound != nullptr)
		Mix_FreeChunk(portal_sound);
		
	
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// toggle FPS display on/off
void WorldSystem::toggleFPSDisplay()
{
	renderer->toggleFPSDisplay();
}

// Debugging
namespace
{
	void glfw_err_cb(int error, const char *desc)
	{
		std::cerr << error << ": " << desc << std::endl;
	}
}

// call to close the window, wrapper around GLFW commands
void WorldSystem::close_window()
{
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow *WorldSystem::create_window()
{

	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit())
	{
		std::cerr << "ERROR: Failed to initialize GLFW in world_system.cpp" << std::endl;
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	// CK: setting GLFW_SCALE_TO_MONITOR to true will rescale window but then you must handle different scalings
	// glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_TRUE);		// GLFW 3.3+
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_FALSE); // GLFW 3.3+

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX, "Towers vs Invaders Assignment", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cerr << "ERROR: Failed to glfwCreateWindow in world_system.cpp" << std::endl;
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow *wnd, int _0, int _1, int _2, int _3)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow *wnd, double _0, double _1)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_move({_0, _1}); };
	auto mouse_button_pressed_redirect = [](GLFWwindow *wnd, int _button, int _action, int _mods)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_button_pressed(_button, _action, _mods); };

	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_pressed_redirect);

	return window;
}

// M3 Feature : Sound affects (at least 3)
bool WorldSystem::start_and_load_sounds()
{

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		fprintf(stderr, "Failed to initialize SDL Audio");
		return false;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
	{
		fprintf(stderr, "Failed to open audio device");
		return false;
	}


	Mix_VolumeMusic(64); // Background music is too loud so set volume to 50%

	background_music = Mix_LoadMUS(audio_path("theme_loop.wav").c_str());
	boss_background_music = Mix_LoadMUS(audio_path("boss_loop.wav").c_str());
	dash_sound = Mix_LoadWAV(audio_path("dash_1.wav").c_str());
	player_shoot_sound = Mix_LoadWAV(audio_path("dash_2.wav").c_str());
	damage_sound = Mix_LoadWAV(audio_path("damage.wav").c_str());
	enemy_shoot_sound = Mix_LoadWAV(audio_path("enemy_shoot.wav").c_str());
	enemy_death_sound = Mix_LoadWAV(audio_path("enemy_death.wav").c_str());
	click_sound = Mix_LoadWAV(audio_path("click_1.wav").c_str());
	portal_sound = Mix_LoadWAV(audio_path("portal.wav").c_str());


	if (background_music == nullptr || dash_sound == nullptr) // IDK why we do this anymore
	{
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
				audio_path("music.wav").c_str(),
				audio_path("dash_1.wav").c_str(),
				audio_path("dash_2.wav").c_str());
		return false;
	}

	return true;
}


void WorldSystem::init(RenderSystem *renderer_arg)
{
	// Either load progression or create a progression entity
	initializeProgression();

	this->renderer = renderer_arg;

	// // start playing background music indefinitely
	// // std::cout << "Starting music..." << std::endl;

	// Set all states to default
	restart_game();
	// set initial game state
	current_state = GameState::START_SCREEN_ANIMATION;
}


void WorldSystem::updateCamera(float elapsed_ms)
{
    Entity cameraEntity = registry.cameras.entities[0];
    Camera &camera = registry.cameras.get(cameraEntity);
    Motion &player_motion = registry.motions.get(registry.players.entities[0]);
    vec2 mouse_world_position = {game_mouse_pos_x, game_mouse_pos_y};

	if (!camera.initialized)
	{
		camera.position = player_motion.position; // Snap to initial position
		camera.initialized = true;
		return;
	}	

	float interpolationFactor = 0.05f;
	
	if (glm::length(player_motion.velocity) > 0.001f)
	{
		vec2 velocityUnitVector = glm::normalize(player_motion.velocity);
		float lerpRadius = CAMERA_POSITION_RADIUS;
	
		vec2 targetPosition = player_motion.position + velocityUnitVector * lerpRadius;
	
	
		camera.position = lerp(camera.position, targetPosition, interpolationFactor);
	} else {
		camera.position = lerp(camera.position, player_motion.position, interpolationFactor);
	}
	camera.grid_position = positionToGridCell(camera.position);
}

void WorldSystem::updateMouseCoords()
{
	Camera &camera = registry.cameras.get(registry.cameras.entities[0]);
	game_mouse_pos_x = device_mouse_pos_x + camera.position.x - WINDOW_WIDTH_PX * 0.5f;
	game_mouse_pos_y = device_mouse_pos_y + camera.position.y - WINDOW_HEIGHT_PX * 0.5f;
}

bool WorldSystem::updateBoss()
{
	std::vector<Entity> bosses_to_split;
    std::vector<Entity> bosses_to_remove;

    std::vector<Entity> bossEntities =  registry.bossAIs.entities;

	for (auto boss : bossEntities) 
	{

		if (!registry.enemies.has(boss) || !registry.motions.has(boss) || !registry.bossAIs.has(boss))
			continue;

		Enemy enemy = registry.enemies.get(boss);
		BossAI bossAI = registry.bossAIs.get(boss);
		Motion motion = registry.motions.get(boss);

		if (bossAI.stage == 3) continue;

		if (enemy.health < enemy.total_health / 2.f) {
			// bosses_to_split.push_back(boss);
			Motion originalMotion = registry.motions.get(boss);
			vec2 position = originalMotion.position;
			vec2 scale = originalMotion.scale;
			int stage = bossAI.stage;
			Entity arrow = bossAI.associatedArrow;


			vec2 smallScale = scale * 0.5f;
			vec2 offset = vec2(smallScale.x * 1.2f, 0.f);
			vec2 pos1 = originalMotion.position - offset;
			vec2 pos2 = originalMotion.position + offset;
			
			createBoss(renderer, pos1, BossState::IDLE, stage + 1);
			createBoss(renderer, pos2, BossState::IDLE, stage + 1);
			
			bosses_to_remove.push_back(boss);
			bosses_to_remove.push_back(arrow);
		}
	}

	for (int i = bosses_to_remove.size() - 1; i >= 0; --i) {
		Entity boss = bosses_to_remove[i];
		registry.remove_all_components_of(boss);
	}
	// terminal condition for the boss
	return registry.bossAIs.size() == 0;
}

void WorldSystem::updateBossArrows() {
	std::vector<Entity> removals;
	
	for (uint i = 0; i < registry.bossArrows.size(); i++) {
		Entity arrow = registry.bossArrows.entities[i];
		BossArrow& arrowComp = registry.bossArrows.get(arrow);
		if (!registry.bossAIs.has(arrowComp.associatedBoss) && !registry.finalBossAIs.has(arrowComp.associatedBoss)) {
			removals.push_back(arrow);
			continue;
		}

		Entity associatedBoss = arrowComp.associatedBoss;
		Motion& bossMotion = registry.motions.get(associatedBoss);

		Camera& camera = registry.cameras.get(registry.cameras.entities[0]);

		vec2 bossPos = bossMotion.position;
		vec2 cameraPos = camera.position;
		vec2 direction = glm::normalize(bossPos - cameraPos);
		float distance = glm::distance(bossPos, cameraPos);

		if (distance > WINDOW_WIDTH_PX / 2.f && distance > WINDOW_HEIGHT_PX / 2.f) {
			
			if (!registry.motions.has(arrow)) {
				registry.motions.emplace(arrow);
			}
			Motion& arrowMotion = registry.motions.get(arrow);
			vec2 center = {WINDOW_WIDTH_PX / 2.f, WINDOW_HEIGHT_PX / 2.f};
			
			float maxX = (WINDOW_WIDTH_PX / 2.f) - 20.f;
			float maxY = (WINDOW_HEIGHT_PX / 2.f) - 20.f;
			
			vec2 arrowOffset = direction * glm::min(maxX / std::abs(direction.x), maxY / std::abs(direction.y));
			
			arrowMotion.angle = atan2(direction.y, direction.x) * 180.f / M_PI + 90.f;
			arrowMotion.position = cameraPos + arrowOffset;
			arrowMotion.scale = {ARROW_WIDTH, ARROW_HEIGHT};
			arrowMotion.velocity = {0.f, 0.f};

			arrowComp.draw = true;
		} else {
			arrowComp.draw = false;
		}
	}

	uint size = removals.size();
	for (uint i = 0; i < size; i++) {
		registry.remove_all_components_of(removals[i]);
	}
}

void WorldSystem::spawnFourDenderitesOnMap() {
	// spawn four on the map where the tiles are empty, but do not spawn if no valid path exists
	ProceduralMap& map = registry.proceduralMaps.get(registry.proceduralMaps.entities[0]);
	if (map.map.size() == 0) return;

    for (int i = 0; i < 4; i++) {
        std::pair<int, int> denderitePosition = getRandomEmptyTile(map.map);
        vec2 denderiteWorldPosition = gridCellToPosition({denderitePosition.second, denderitePosition.first});
        createDenderite(renderer, denderiteWorldPosition);
    }
}

void WorldSystem::spawnEnemies(float elapsed_ms_since_last_update)
{
	// std::cout << "WS:spawn - f1" << std::endl;

	Motion &player_motion = registry.motions.get(registry.players.entities[0]);

	// spawn new invaders
	next_enemy_spawn -= elapsed_ms_since_last_update * current_speed;
	if (next_enemy_spawn < 0.f && !gameOver)
	{
		if (registry.enemies.entities.size() < MAX_ENEMIES_COUNT)
		{
			// reset timer
			next_enemy_spawn = (ENEMY_SPAWN_RATE_MS / 2) + uniform_dist(rng) * (ENEMY_SPAWN_RATE_MS / 2);

			// randomize position
			// randomize empty tile on mpa
			std::pair<int, int> enemyPosition = getRandomEmptyTile(registry.proceduralMaps.get(registry.proceduralMaps.entities[0]).map);

			int random_num = rand();
			if (random_num % 3 == 0)
			{
				createSpikeEnemy(renderer, gridCellToPosition({ enemyPosition.second, enemyPosition.first }));
			}
			else if (random_num % 3 == 1)
			{
				createRBCEnemy(renderer, gridCellToPosition({ enemyPosition.second, enemyPosition.first }));
			}
			else if (registry.bacteriophageAIs.entities.size() < MAX_BACTERIOPHAGE_COUNT)
			{
				while (glm::distance(player_motion.position, gridCellToPosition({ enemyPosition.second, enemyPosition.first })) < SPIKE_ENEMY_DETECTION_RADIUS)
				{
					// randomize empty tile on mpa
					enemyPosition = getRandomEmptyTile(registry.proceduralMaps.get(registry.proceduralMaps.entities[0]).map);
				}

				int index = (int)(uniform_dist(rng) * MAX_BACTERIOPHAGE_COUNT);
				while (bacteriophage_idx.find(index) != bacteriophage_idx.end())
				{
					index = (int)(uniform_dist(rng) * MAX_BACTERIOPHAGE_COUNT);
				}
				bacteriophage_idx[index] = 1;

				createBacteriophage(renderer, gridCellToPosition({ enemyPosition.second, enemyPosition.first }), index);
			}
		}
	}
	
}

void WorldSystem::handleProjectiles(float elapsed_ms_since_last_update)
{
	for (int i = 0; i < registry.projectiles.entities.size(); i++)
	{
		Projectile& projectile = registry.projectiles.get(registry.projectiles.entities[i]);
		projectile.ms_until_despawn -= elapsed_ms_since_last_update * current_speed;

		if (projectile.ms_until_despawn < 0.0f)
		{
			
			registry.remove_all_components_of(registry.projectiles.entities[i]);
		}
	}

	// spawn new projectiles
	next_projectile_ms -= elapsed_ms_since_last_update * current_speed;

	if (next_projectile_ms < 0.f && !gameOver)
	{
		next_projectile_ms = (PROJECTILE_SPAWN_RATE_MS / 2) + uniform_dist(rng) * (PROJECTILE_SPAWN_RATE_MS / 2);

		int shooting_enemy = 0;
		while (shooting_enemy < registry.bacteriophageAIs.entities.size())
		{
			BacteriophageAI& enemy_behavior = registry.bacteriophageAIs.get(registry.bacteriophageAIs.entities[shooting_enemy]);
			if (enemy_behavior.state != BacteriophageState::CHASING)
			{
				shooting_enemy++;
			}
			else break;
		}


		if (shooting_enemy < registry.bacteriophageAIs.entities.size())
		{
			Mix_PlayChannel(-1, enemy_shoot_sound, 0);
			createBacteriophageProjectile(registry.bacteriophageAIs.entities[shooting_enemy]);
		}
	}
}

bool WorldSystem::checkPortalCollision(){
	// Point the player to the mouse
	Motion &player_motion = registry.motions.get(registry.players.entities[0]);
    // check if player in portal tile
    if (registry.portals.entities.size() > 0 && registry.motions.has(registry.portals.entities.back())) {
        Motion &portal_motion = registry.motions.get(registry.portals.entities.back());
        vec2 portal_position = portal_motion.position;

        float distance = sqrt(pow(player_motion.position.x - portal_position.x, 2) + pow(player_motion.position.y - portal_position.y, 2));
        float portal_radius = TILE_SIZE / 4.0f;

        if (distance < portal_radius) {
            // go to black screen
			return true;
        }
    }
	return false;
}

void WorldSystem::updateDangerLevel(float elapsed_ms_since_last_update) {
    Player& p = registry.players.get(registry.players.entities[0]);

    static float dangerTimer = 0.f;
    dangerTimer += elapsed_ms_since_last_update;

    if (dangerTimer >= DANGER_INCREASE_INTERVAL) {
        dangerTimer = 0.f;
        p.dangerFactor = std::min(p.dangerFactor + DANGER_INCREASE_AMOUNT, MAX_DANGER_LEVEL);
    }
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update)
{
	Progression& prog = registry.progressions.get(registry.progressions.entities[0]);
	if (prog.pickedInNucleus.size() > 0) {
		for(int i = 0; i < prog.pickedInNucleus.size(); i++) {
			applyBuff(registry.players.get(registry.players.entities[0]), prog.pickedInNucleus[i]);
		}
		prog.pickedInNucleus.clear();	
	}


	// // std::cout << "Level : " << level << std::endl;
	updateDangerLevel(elapsed_ms_since_last_update);
	updateCamera(elapsed_ms_since_last_update);

	if (progress_map["tutorial_mode"] && registry.infoBoxes.size() == 0) {
		createInfoBoxes();
	}
	
    updateMouseCoords(); 
	updateHuds();

	handlePlayerMovement(elapsed_ms_since_last_update);
	handlePlayerHealth(elapsed_ms_since_last_update);

	if (!progress_map["tutorial_mode"] && level != BOSS_LEVEL && level != FINAL_BOSS_LEVEL) {
		spawnEnemies(elapsed_ms_since_last_update);
	} else if (level == BOSS_LEVEL) {
		if (!updateBoss()) {
			updateBossArrows();
		} else {
            if (registry.portals.size() == 0) {
                Player& player = registry.players.get(registry.players.entities[0]);
                Motion& player_motion = registry.motions.get(registry.players.entities[0]);
                vec2 grid_pos = positionToGridCell(player_motion.position);
                int x = grid_pos.x;
                int y = grid_pos.y;
    
                if (x <= 10) {
                    addPortalTile({x + 2, y});
                } else {
                    addPortalTile({x - 2, y});
                }
            }
		}
	} else if (level == FINAL_BOSS_LEVEL) {
		if (registry.finalBossAIs.size() != 0) {
			updateBossArrows();
		} else {
            if (registry.portals.size() == 0) {
                Player& player = registry.players.get(registry.players.entities[0]);
                Motion& player_motion = registry.motions.get(registry.players.entities[0]);
                vec2 grid_pos = positionToGridCell(player_motion.position);
                int x = grid_pos.x;
                int y = grid_pos.y;
    
                if (x <= 10) {
                    addPortalTile({x + 2, y});
                } else {
                    addPortalTile({x - 2, y});
                }
            }
		}
	}
	handleProjectiles(elapsed_ms_since_last_update);
	handleRippleEffect(elapsed_ms_since_last_update);

    tileProceduralMap();

	if (checkPortalCollision()) {
        if (level != FINAL_BOSS_LEVEL) {
            Entity screen_state_entity = renderer->get_screen_state_entity();
            ScreenState &screen = registry.screenStates.get(screen_state_entity);
            screen.darken_screen_factor = 1;
               darken_screen_timer = 0.0f;
            current_state = GameState::NEXT_LEVEL;
            Mix_PlayChannel(-1, portal_sound, 0);
            goToNextLevel();
            return true;
        } else {
            clearVignetteEffect();
			previous_state = current_state;
			current_state = GameState::VICTORY;
			stateTimer = WIN_CUTSCENE_DURATION_MS;
			createEndingWinScene();
        }
	}

    if (darken_screen_timer >= 0.0f) {
        darken_screen_timer += elapsed_ms_since_last_update;
        if (darken_screen_timer >= NEXT_LEVEL_BLACK_SCREEN_TIMER_MS) {
            Entity screen_state_entity = renderer->get_screen_state_entity();
            ScreenState &screen = registry.screenStates.get(screen_state_entity);
            screen.darken_screen_factor = -1;
            darken_screen_timer = -1.0f; // Stop the timer
        }
    }

    handleVignetteEffect(elapsed_ms_since_last_update);

	// step the particle system only when its needed
	// for optimaztion, we could only step the particles that are on screen
	particle_system.step(elapsed_ms_since_last_update);

    // update gun cooldown
    Gun &gun = registry.guns.get(registry.guns.entities[0]);
    if (gun.cooldown_timer_ms > 0.0f) {
        gun.cooldown_timer_ms -= elapsed_ms_since_last_update;
    }

    // update gun position to match player
    Motion &player_motion = registry.motions.get(registry.players.entities[0]);
    Motion &gun_motion = registry.motions.get(registry.guns.entities[0]);
    gun_motion.angle = 180.f + player_motion.angle;

    float angle_radians = glm::radians(player_motion.angle);
    vec2 offset = {cos(angle_radians) * (PLAYER_BB_WIDTH / 2), sin(angle_radians) * (PLAYER_BB_WIDTH / 2)};

    gun_motion.position = {player_motion.position[0] - offset.x, player_motion.position[1] - offset.y};
    gun_motion.velocity = player_motion.velocity;

	updateMiniMap(registry.players.get(registry.players.entities[0]).grid_position);
	return true;
}

void WorldSystem::handleVignetteEffect(float elapsed_ms_since_last_update) {
    Entity screen_state_entity = renderer->get_screen_state_entity();
    ScreenState &screen = registry.screenStates.get(screen_state_entity);
    if (screen.vignette_screen_factor > 0) {
        screen.vignette_timer_ms -= elapsed_ms_since_last_update;
        if (screen.vignette_timer_ms <= 0) {
            screen.vignette_screen_factor -= elapsed_ms_since_last_update / 1000;
            if (screen.vignette_screen_factor < 0) {
                screen.vignette_screen_factor = 0;
            }
        }
	}
}

// Handle player health
void WorldSystem::handlePlayerHealth(float elapsed_ms)
{
	Player &player = registry.players.get(registry.players.entities[0]);
	HealthBar &healthBar = registry.healthBars.get(registry.healthBars.entities[0]);

	// handle regeneration
	// heal every second
	player.healing_timer_ms -= elapsed_ms;
	if (player.healing_timer_ms <= 0 && player.current_health < player.max_health)
	{
		player.healing_timer_ms = player.default_healing_timer;
		player.current_health += player.max_health * player.healing_rate;
		if (player.current_health > player.max_health)
		{
			player.current_health = player.max_health;
		}
	}

	if (player.current_health <= 0 && current_state != GameState::GAME_OVER)
	{
		triggerGameOver();
        clearVignetteEffect();
	}
}

void WorldSystem::triggerGameOver() {
	Player& player = registry.players.get(registry.players.entities[0]);
	Progression& p = registry.progressions.get(registry.progressions.entities[0]);
    p.buffsFromLastRun = player.buffsCollected;
    p.germoney_savings = player.germoney_count;
    previous_state = current_state;
    current_state = GameState::GAME_OVER;
    currentBiome = Biome::RED; 
    createGameOverScreen();

}

// Handle player movement
void WorldSystem::handlePlayerMovement(float elapsed_ms_since_last_update) {
	// if the player is not dashing, then have its velocity be base speed * by direction to mouse, if the mouse is outside deadzone
	
	Player &player = registry.players.get(registry.players.entities[0]);

	if (player.knockback_duration > 0.0f) {
		return;
	}


	Motion &player_motion = registry.motions.get(registry.players.entities[0]);
	player_motion.angle = atan2(game_mouse_pos_y - player_motion.position.y, game_mouse_pos_x - player_motion.position.x) * 180.0f / M_PI + 90.0f;

	if(registry.dashes.size() == 0) {
		Motion &player_motion = registry.motions.get(registry.players.entities[0]);

		vec2 direction = vec2(game_mouse_pos_x, game_mouse_pos_y) - player_motion.position;
		direction = normalize(direction);

		// If the mouse is outside the deadzone, move the player
		if(length(vec2(game_mouse_pos_x, game_mouse_pos_y) - player_motion.position) > MOUSE_TRACKING_DEADZONE) 
		{
			player_motion.velocity = {direction.x * player.speed, direction.y * player.speed};
		} 
		else
		{
			player_motion.velocity = {0, 0};
		}
	}
}

void WorldSystem::goToNextLevel()
{
    // print go to next level
    // std::cout << "Going to next level" << std::endl;

	current_speed = 1.f;
	level += 1;
	next_enemy_spawn = 0;
	enemy_spawn_rate_ms = ENEMY_SPAWN_RATE_MS;

	// update biome for the new level
	setCurrentBiomeByLevel(level); 
	
	initializedMap = false;
	currentTiles.clear();

    int enemySize = registry.enemies.entities.size();
    for (int i = 0; i < enemySize; i++) {
        registry.remove_all_components_of(registry.enemies.entities.back());
    }

    // remove all tiles
    int tile_size = registry.tiles.entities.size();
    for (int i = 0; i < tile_size; i++) {
        registry.remove_all_components_of(registry.tiles.entities.back());
    }

    // remove all buffs
    int buff_size = registry.buffs.entities.size();
    for (int i = 0; i < buff_size; i++) {
        registry.remove_all_components_of(registry.buffs.entities.back());
    }

	// remove boss arrows in case not removed
	int bossArrow_size = registry.bossArrows.size();
	for (int i = 0; i < bossArrow_size; i++) {
		registry.remove_all_components_of(registry.bossArrows.entities.back());
	}

	gameOver = false;
	std::pair<int, int> playerPosition;

	if (level != BOSS_LEVEL && level != FINAL_BOSS_LEVEL) {
		createProceduralMap(renderer, vec2(MAP_WIDTH, MAP_HEIGHT), progress_map["tutorial_mode"], playerPosition);
	} else if (level == BOSS_LEVEL) {
		createBossMap(renderer, vec2(MAP_WIDTH, MAP_HEIGHT), playerPosition);
		createBoss(renderer, gridCellToPosition({10, 10}));
	} else if (level == FINAL_BOSS_LEVEL) {
		createFinalBossMap(renderer, vec2(MAP_WIDTH, MAP_HEIGHT), playerPosition);
		createFinalBoss(renderer, gridCellToPosition({9, 2}));
	} 

	if (level == FINAL_BOSS_LEVEL - 1) {
		spawnFourDenderitesOnMap();
	}

	Player &player = registry.players.get(registry.players.entities[0]);
	Motion &playerMotion = registry.motions.get(registry.players.entities[0]);
	// Progression &prog = registry.progressions.get(registry.progressions.entities[0]);

	playerMotion.position = gridCellToPosition(vec2(playerPosition.second, playerPosition.first));

	// for(int i = 0; i < prog.pickedInNucleus.size(); i++) {
	// 	applyBuff(player, prog.pickedInNucleus[i]);
	// }
	Camera &camera = registry.cameras.get(registry.cameras.entities[0]);
	camera.position = playerMotion.position;
	bacteriophage_idx.clear();
    // print exiting
    // std::cout << "Exiting createProceduralMap" << std::endl;
	emptyMiniMap();
	return;
}


// Reset the world state to its initial state
void WorldSystem::restart_game()
{
	// Debugging for memory/component leaks
	registry.list_all_components();
    
	// Reset the game speed
	current_speed = 1.f;
    
	if (progress_map["tutorial_mode"]) {
		level = 0;
	} else {
		level = 1;
	}
	next_enemy_spawn = 0;
	enemy_spawn_rate_ms = ENEMY_SPAWN_RATE_MS;

	bacteriophage_idx.clear();
    
	// FLAG
	gameOver = false;
	shopItemsPlaced = false;

	next_projectile_ms = 0;

	initializedMap = false;
	currentTiles.clear();


	// Not sure if we need to touch screen state here
	// ScreenState &screen = registry.screenStates.components[0];
	// screen.darken_screen_factor = -1; // FLAG doesnt seem to help

	registry.deathTimers.clear(); // this seems to work
	// Remove all entities that we created
	// All that have a motion, we could also iterate over all bug, eagles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// Remove all dashes
	while (registry.dashes.entities.size() > 0)
		registry.remove_all_components_of(registry.dashes.entities.back());

	while (registry.gameScreens.entities.size() > 0)
		registry.remove_all_components_of(registry.gameScreens.entities.back());

	while (registry.buttons.entities.size() > 0)
		registry.remove_all_components_of(registry.buttons.entities.back());

    // remove all tiles
    while (registry.tiles.entities.size() > 0)
        registry.remove_all_components_of(registry.tiles.entities.back());
	
	while (registry.shops.entities.size() > 0)
        registry.remove_all_components_of(registry.shops.entities.back());
	while (registry.overs.entities.size() > 0)
        registry.remove_all_components_of(registry.overs.entities.back());

	// debugging for memory/component leaks
	registry.list_all_components();
    
	// std::cout << "Creating Procedural Map, tutorial mode status :" << tutorial_mode << std::endl;

	std::pair<int, int> playerPosition;
	createProceduralMap(renderer, vec2(MAP_WIDTH, MAP_HEIGHT), progress_map["tutorial_mode"], playerPosition);
		
	if (progress_map["tutorial_mode"]) {
		createPlayer(renderer, gridCellToPosition({1, 10}));
		createSpikeEnemy(renderer, gridCellToPosition({12, 10}));
		createKey(renderer, gridCellToPosition({15, 10}));
		createChest(renderer, gridCellToPosition({18, 10}));
	} else {
		createPlayer(renderer, gridCellToPosition(vec2(playerPosition.second, playerPosition.first)));
	}

	createCamera();


	createStartScreen();


	stateTimer = BOOT_CUTSCENE_DURATION_MS;
	(renderer);

	createUIElement(NUCLEUS_UI_POS,
					vec2(NUCLEUS_UI_WIDTH, NUCLEUS_UI_HEIGHT),
					TEXTURE_ASSET_ID::NUCLEUS_UI,
					EFFECT_ASSET_ID::UI);
	createHealthBar();
	createThermometer();

	for (int i = 0; i < registry.players.get(registry.players.entities[0]).max_dash_count; i++) {
		createDashRecharge();
	}
	
	createUIElement(GERMONEY_UI_POS,
					vec2(GERMONEY_UI_WIDTH, GERMONEY_UI_HEIGHT),
					TEXTURE_ASSET_ID::GERMONEY_UI,
					EFFECT_ASSET_ID::UI);
	createUIElement(WEAPON_PILL_UI_POS,
					vec2(WEAPON_PILL_UI_WIDTH, WEAPON_PILL_UI_HEIGHT),
					TEXTURE_ASSET_ID::WEAPON_PILL_UI,
					EFFECT_ASSET_ID::UI);
    createUIElement(GUN_UI_POS,
                    vec2(GUN_UI_SIZE, GUN_UI_SIZE),
                    TEXTURE_ASSET_ID::GUN_STILL,
                    EFFECT_ASSET_ID::UI);

    Player &player = registry.players.get(registry.players.entities[0]);
    Progression &prog = registry.progressions.get(registry.progressions.entities[0]);
	
	// for(int i = 0; i < prog.pickedInNucleus.size(); i++) {
	// 	applyBuff(player, prog.pickedInNucleus[i]);
	// }
    // prog.pickedInNucleus.clear();

    player.germoney_count = prog.germoney_savings;
	
    createGunCooldown();

	createMiniMap(renderer, vec2(MAP_WIDTH, MAP_HEIGHT));
	emptyMiniMap();
}

// Compute collisions between entities. Collisions are always in this order: (Player | Projectiles, Enemy | Wall | Buff)
void WorldSystem::handle_collisions()
{
    std::vector<Entity> removals;

	for (auto& entity : registry.collisions.entities)
	{
		Collision& collision = registry.collisions.get(entity);
		Entity entity2 = collision.other;

		// if (registry.bacteriophageProjectiles.has(entity2) || registry.bossProjectiles.has(entity2) 
		// 	|| registry.finalBossProjectiles.has(entity2) || registry.denderiteProjectile.has(entity2))
		// {
		// 	if (registry.players.has(entity)) // HANDLE PROJECTILE/PLAYER COLLISION
		// 	{
		// 		Player& player = registry.players.get(entity);
		// 		Projectile& projectile = registry.projectiles.get(entity2);

		// 		// Player takes damage
		// 		damagePlayer(projectile.damage);

		// 		// remove projectile
        //         removals.push_back(entity2);
		// 	}
		// }

		if (registry.projectiles.has(entity2)) 
		{
			if (registry.players.has(entity))
			{
				Projectile& projectile = registry.projectiles.get(entity2);
				if (!projectile.from_enemy) continue;
				// Player takes damage
				damagePlayer(projectile.damage);

                Mix_PlayChannel(-1, damage_sound, 0);

				// remove projectile
                removals.push_back(entity2);
				// registry.remove_all_components_of(entity2);
			}
		}
		else if (registry.keys.has(entity2))
		{
			if (registry.players.has(entity))
			{
				float predictionTime = 0.001f; // 100 ms = 0.1s

				if (physics_system.willMeshCollideSoon(entity, entity2, predictionTime))
				{
					Motion& keyMotion = registry.motions.get(entity2);
					Motion& playerMotion = registry.motions.get(entity);

					if (glm::length(playerMotion.velocity) > 0.0f) 
					{
						keyMotion.velocity = playerMotion.velocity * 3.0f;
					}
					else 
					{
						keyMotion.velocity = vec2(0.0f, 0.0f);
					}
				}
			}
			else if (registry.chests.has(entity))
			{
				Motion& keyMotion = registry.motions.get(entity2);
				Motion& chestMotion = registry.motions.get(entity);

				Mesh& chestMesh = *registry.meshPtrs.get(entity);

				std::vector<vec2> chestWorldVertices = physics_system.getWorldVertices(chestMesh.textured_vertices, chestMotion.position, chestMotion.scale);

				if (physics_system.pointInPolygon(keyMotion.position, chestWorldVertices))
				{
                    removals.push_back(entity);
                    removals.push_back(entity2);
                
					if (progress_map["tutorial_mode"]) {
						current_state = GameState::NEXT_LEVEL;
						progress_map["tutorial_mode"] = false;
						removeInfoBoxes();
						goToNextLevel();
						emptyMiniMap();
					}
				}
			}
		}
		else if (registry.enemies.has(entity2))
		{
			Enemy& enemy = registry.enemies.get(entity2);
            Motion &enemy_motion = registry.motions.get(entity2);
			if (registry.projectiles.has(entity))
			{
				Projectile& projectile = registry.projectiles.get(entity);

				if (projectile.from_enemy) continue;
				if (enemy.health < 0) continue; // prevent multy buff drop

				// Invader takes damage

				enemy.health -= projectile.damage;

				// reflect projectile if hitting final boss in non-tired state
				if (registry.finalBossAIs.has(entity2)) {
					FinalBossAI & finalBossAI = registry.finalBossAIs.get(entity2);
					if (finalBossAI.state != FinalBossState::TIRED) {
						enemy.health += projectile.damage;
						Motion& projectile_motion = registry.motions.get(entity);
						vec2 direction = glm::normalize(projectile_motion.velocity);
						projectile_motion.velocity = -direction * GUN_PROJECTILE_SPEED;
						projectile.from_enemy = !projectile.from_enemy;
					} else {
						removals.push_back(entity);
					}
				} else {
					removals.push_back(entity);
				}

				// if invader health is below 0
				// remove invader and increase points
				// buff created
				if (enemy.health <= 0)
				{
					if (registry.bacteriophageAIs.has(entity2))
					{
						bacteriophage_idx.erase(registry.bacteriophageAIs.get(entity2).placement_index);
					}

					vec2 enemy_position = enemy_motion.position;

                    removeEnemyHPBar(entity2);
                    
					// level += 1; 
					Mix_PlayChannel(-1, enemy_death_sound, 0); // FLAG MORE SOUNDS
            
                    Player& player = registry.players.get(registry.players.entities[0]);
                    player.germoney_count += 1;

					if (level != FINAL_BOSS_LEVEL) {
						createBuff(vec2(enemy_position.x, enemy_position.y));
					}
					particle_system.createParticles(PARTICLE_TYPE::DEATH_PARTICLE, enemy_position, 15); 
                    removals.push_back(entity2);
				}
			}
			else if (registry.players.has(entity))
			{
                if (isDashing())
				{
                    if (registry.spikeEnemyAIs.has(entity2) && registry.spikeEnemyAIs.get(entity2).state != SpikeEnemyState::KNOCKBACK) {
                        Motion &player_motion = registry.motions.get(entity);
                        
                        vec2 direction_to_enemy = normalize(enemy_motion.position - player_motion.position);
                        
                        float player_angle_radians = glm::radians(player_motion.angle - 90.0f);
                        vec2 player_facing_direction = {cos(player_angle_radians), sin(player_angle_radians)};
                        
                        float dot_product = glm::dot(player_facing_direction, direction_to_enemy);
                        
                        if (dot_product > 0.001f) {
                            enemy.health -= PLAYER_DASH_DAMAGE;
                            
                            SpikeEnemyAI &enemy_ai = registry.spikeEnemyAIs.get(entity2);
                            
                            enemy_ai.state = SpikeEnemyState::KNOCKBACK;
                            enemy_ai.knockbackTimer = SPIKE_ENEMY_KNOCKBACK_TIMER;
                            
                            vec2 knockback_direction = normalize(enemy_motion.position - player_motion.position);
                            enemy_motion.velocity = knockback_direction * SPIKE_ENEMY_KNOCKBACK_STRENGTH;
                        }
                    } else {
						if (enemy.health < 0.f) continue; // prevent multy buff drop
                        enemy.health -= PLAYER_DASH_DAMAGE;
						Player& player = registry.players.get(entity);
						Motion& playerMotion = registry.motions.get(entity);

                        if (registry.bossAIs.has(entity2)) {
							
							for (auto e : registry.dashes.entities) {
								removals.push_back(e);
							}
							playerMotion.velocity = -1.f * glm::normalize(playerMotion.velocity) * PLAYER_DASH_SPEED;
							player.knockback_duration = 500.f;
						}

						if (registry.finalBossAIs.has(entity2)) {
							FinalBossAI & finalBossAI = registry.finalBossAIs.get(entity2);
							for (auto e : registry.dashes.entities) {
								removals.push_back(e);
							}
							// prevent damage in non-tired mode
							if (finalBossAI.state != FinalBossState::TIRED) {
								enemy.health += PLAYER_DASH_DAMAGE;
							}
							playerMotion.velocity = -1.f * glm::normalize(playerMotion.velocity) * PLAYER_DASH_SPEED;
							player.knockback_duration = 500.f;
						}
                    }
				}
				else
				{
					uint current_time = SDL_GetTicks();
					Player& player = registry.players.get(entity);
					// then apply damage.
					if (!registry.damageCooldowns.has(entity))
					{
						//  add the component and apply damage
						registry.damageCooldowns.insert(entity, { current_time });

						damagePlayer(1); // Why is this one
						
						Mix_PlayChannel(-1, damage_sound, 0);
					}
					else
					{
						// retrieve the cooldown component
						DamageCooldown& dc = registry.damageCooldowns.get(entity);
						if (current_time - dc.last_damage_time >= 500)
						{
							dc.last_damage_time = current_time;

							damagePlayer(1); // Why is this one

							Mix_PlayChannel(-1, damage_sound, 0);
						}
					}
				}
				
                if (enemy.health <= 0)
                {
                    if (registry.bacteriophageAIs.has(entity2))
                    {
                        bacteriophage_idx.erase(registry.bacteriophageAIs.get(entity2).placement_index);
                    }
                    
                    vec2 enemy_position = enemy_motion.position;

					
                    points += 1;
                    removals.push_back(entity2);
					removeEnemyHPBar(entity2);
                    Mix_PlayChannel(-1, enemy_death_sound, 0);
					
                    Player& player = registry.players.get(registry.players.entities[0]);
                    
					if (registry.bossAIs.has(entity2) || registry.finalBossAIs.has(entity2)) {
						player.germoney_count += 100;
					} else {
						player.germoney_count += 1;
					}

					if (level != FINAL_BOSS_LEVEL) {
						createBuff(vec2(enemy_position.x, enemy_position.y));
					}
                    particle_system.createParticles(PARTICLE_TYPE::DEATH_PARTICLE, enemy_position, 15);
                } 

				if (registry.bossAIs.has(entity2)) 
				{
					BossAI& bossAI = registry.bossAIs.get(entity2);

					if (bossAI.state == BossState::RUMBLE && registry.players.has(entity))
					{
						Motion& bossMotion = registry.motions.get(entity2);
						Motion& playerMotion = registry.motions.get(entity);

						Player& player = registry.players.get(entity);
						// need to check the rumble cool down

						if (!bossAI.is_charging || !bossAI.is_fleeing) {
							damagePlayer(BOSS_RUMBLE_DAMAGE);

                            Mix_PlayChannel(-1, damage_sound, 0);
						}

						if (player.knockback_duration > 0.f )
						{
							vec2 bossDirection = glm::length(bossMotion.velocity) > 0.0001f
							? glm::normalize(bossMotion.velocity)
							: vec2(1.f, 0.f); // default direction, rightwards
						
							vec2 knockBackDirection = bossDirection;
                            
                            // check if playermotion velocity is zero or very very low
                            playerMotion.velocity = glm::length(playerMotion.velocity) < 0.00001f ? vec2(0.1f, 0.0f) : playerMotion.velocity;
							playerMotion.velocity = knockBackDirection * 1000.f;
							bossMotion.velocity = {0.f, 0.f};						
						}
					}
				}
			
			}
		}
		else if (registry.buffs.has(entity2) && registry.players.has(entity))
		{
			collectBuff(entity, entity2);
            removals.push_back(entity2);
		}
	}

    int size = removals.size();

    for (int i = 0; i < size; i ++) {
        registry.remove_all_components_of(removals[i]);
    }
    
	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const
{
	return bool(glfwWindowShouldClose(window));
}

// on key callback
void WorldSystem::on_key(int key, int, int action, int mod)
{
    // if (action == GLFW_RELEASE && key == GLFW_KEY_N) {
    //     if (progress_map["tutorial_mode"]) {
    //         current_state = GameState::NEXT_LEVEL;
    //         progress_map["tutorial_mode"] = false;
    //         removeInfoBoxes();
    //         goToNextLevel();
    //         emptyMiniMap();
    //     } else {
    //         Entity screen_state_entity = renderer->get_screen_state_entity();
    //         ScreenState &screen = registry.screenStates.get(screen_state_entity);
    //         screen.darken_screen_factor = 1;
    //         darken_screen_timer = 0.0f;
    //         current_state = GameState::NEXT_LEVEL;
    //         Mix_PlayChannel(-1, portal_sound, 0);
    //         goToNextLevel();
    //     }
    // }

	// exit game w/ ESC
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE)
	{
		close_window();
    }

	// toggle FPS display with F key
	if (action == GLFW_RELEASE && key == GLFW_KEY_F)
	{
		toggleFPSDisplay();
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R)
	{
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		restart_game();

		previous_state = current_state;
		current_state = GameState::START_SCREEN_ANIMATION;
	}

	// Pausing Game
	if (action == GLFW_RELEASE && key == GLFW_KEY_SPACE)
	{
		if (current_state == GameState::GAME_PLAY)
		{
			current_state = GameState::PAUSE;

			// renderer.
			createPauseScreen();
			Mix_PauseMusic(); // Pause music
		}
		else if (current_state == GameState::PAUSE)
		{
			current_state = GameState::GAME_PLAY;
			removePauseScreen();
			Mix_ResumeMusic(); // Resume music
		}
	}

	// Debugging - not used in A1, but left intact for the debug lines
	if (key == GLFW_KEY_D)
	{
		if (action == GLFW_RELEASE)
		{
			if (debugging.in_debug_mode)
			{
				debugging.in_debug_mode = false;
			}
			else
			{
				debugging.in_debug_mode = true;
			}
		}
	}

	// using O key for gameover, for now
	if (key == GLFW_KEY_O)
	{
		if (action == GLFW_RELEASE)
		{
			if (current_state == GameState::GAME_PLAY)
			{
				Progression& p = registry.progressions.get(registry.progressions.entities[0]);

				p.buffsFromLastRun = registry.players.get(registry.players.entities[0]).buffsCollected;		
				previous_state = GameState::GAME_PLAY;
				current_state = GameState::GAME_OVER;
                triggerGameOver();
                clearVignetteEffect();
				createGameOverScreen();

				currentBiome = Biome::RED;
			}
		}
	}

	// Q for going to start screen
	if (key == GLFW_KEY_Q)
	{
		if (action == GLFW_RELEASE)
		{
			restart_game();
			current_state = GameState::START_SCREEN_ANIMATION;
		}
	}

    // S for shooting gun
    if (key == GLFW_KEY_S)
    {
        if (action == GLFW_RELEASE)
        {
            shootGun();
        }
    }

	if (key == GLFW_KEY_L && action == GLFW_RELEASE && current_state == GameState::START_SCREEN)
	{
		if (checkLoadFileExists()) {
			loadGame();
			loadProgress();
		}
	}

	
}

void WorldSystem::on_mouse_move(vec2 mouse_position)
{

	// record the current mouse position
	device_mouse_pos_x = mouse_position.x;
	device_mouse_pos_y = mouse_position.y;

	for (Entity button_entity : registry.buttons.entities) {
	screenButton &button = registry.buttons.get(button_entity);

	RenderRequest &request = registry.renderRequests.get(button_entity);
	if (button.type == ButtonType::STARTBUTTON && registry.renderRequests.has(button_entity)) {
		if (isButtonClicked(button)) {
			request.used_texture = TEXTURE_ASSET_ID::START_BUTTON_ON_HOVER;
		} else {
			request.used_texture = TEXTURE_ASSET_ID::START_BUTTON;
		}
	} else if (button.type == ButtonType::SHOPBUTTON && registry.renderRequests.has(button_entity)) {
		if (isButtonClicked(button)) {
			request.used_texture = TEXTURE_ASSET_ID::SHOP_BUTTON_ON_HOVER;
		} else {
			request.used_texture = TEXTURE_ASSET_ID::SHOP_BUTTON;
		}
	} else if (button.type == ButtonType::INFOBUTTON && registry.renderRequests.has(button_entity)) {
		if (isButtonClicked(button)) {
			request.used_texture = TEXTURE_ASSET_ID::INFO_BUTTON_ON_HOVER;
		} else {
			request.used_texture = TEXTURE_ASSET_ID::INFO_BUTTON;
		}
	} else if (button.type == ButtonType::BACKBUTTON && registry.renderRequests.has(button_entity)) {
		if (isButtonClicked(button)) {
			// std::cout << "Hovering over button type: " << static_cast<int>(button.type) << std::endl; //debug
			request.used_texture = TEXTURE_ASSET_ID::BACK_BUTTON_ON_HOVER;
		} else {
			request.used_texture = TEXTURE_ASSET_ID::BACK_BUTTON;
		}
	} else if (button.type == ButtonType::SAVEBUTTON && registry.renderRequests.has(button_entity)) {
		if (isButtonClicked(button)) {
			request.used_texture = TEXTURE_ASSET_ID::SAVE_BUTTON_ON_HOVER;
		} else {
			request.used_texture = TEXTURE_ASSET_ID::SAVE_BUTTON;
		}
	} else if (button.type == ButtonType::PROCEEDBUTTON && registry.renderRequests.has(button_entity)) {
		if (isButtonClicked(button)) {

			request.used_texture = TEXTURE_ASSET_ID::PROCEED_BUTTON_ON_HOVER;
		} else {
			request.used_texture = TEXTURE_ASSET_ID::PROCEED_BUTTON;
		}
	} else if (button.type == ButtonType::RESUMEBUTTON && registry.renderRequests.has(button_entity)) {
		if (isButtonClicked(button)) {
			request.used_texture = TEXTURE_ASSET_ID::RESUME_BUTTON_ON_HOVER;
		} else {
			request.used_texture = TEXTURE_ASSET_ID::RESUME_BUTTON;
		}
	}
	
	updateMouseCoords();
	}	
}


ButtonType WorldSystem::getClickedButton()
{
	for (auto& button : registry.buttons.components) {
		if (isButtonClicked(button)) {
			return button.type;
		}
	}

	return ButtonType::NONE;
}

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods)
{

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: Handle mouse clicking for invader and tower placement.
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// on button press
	if (gameOver) return;
	else if (action == GLFW_RELEASE)
	{
		vec2 tile = positionToGridCell(vec2(game_mouse_pos_x, game_mouse_pos_y));

		if (current_state == GameState::GAME_PLAY)
		{
            if (button == GLFW_MOUSE_BUTTON_LEFT && (mods & GLFW_MOD_SHIFT))
            {
                shootGun();
            }
			else if (button == GLFW_MOUSE_BUTTON_LEFT)
			{
				if (canDash())
				{
					initiatePlayerDash();
					Mix_PlayChannel(-1, dash_sound, 0);
				}
			}
            else if (button == GLFW_MOUSE_BUTTON_RIGHT)
            {
                shootGun();
            }
		}
		else if (current_state == GameState::START_SCREEN && button == GLFW_MOUSE_BUTTON_LEFT)
		{
			
			ButtonType clickedButton = getClickedButton();

			if (clickedButton == ButtonType::SHOPBUTTON) 
			{
				Mix_PlayChannel(-1, click_sound, 0);
				previous_state = current_state;
				current_state = GameState::SHOP;
				removeStartScreen();
				createShopScreen();
				if (!shopItemsPlaced) {
					placeBuffsOnShopScreen();
					shopItemsPlaced = true;
				}
			}
			else if (clickedButton == ButtonType::INFOBUTTON) 
			{
				Mix_PlayChannel(-1, click_sound, 0);
				previous_state = current_state;
				current_state = GameState::INFO;
				removeStartScreen();
				createInfoScreen();
			}
			else if (clickedButton == ButtonType::STARTBUTTON) 
			{
				Mix_PlayChannel(-1, click_sound, 0);
				previous_state = current_state;
				current_state = GameState::GAMEPLAY_CUTSCENE;

                // assign player germoney count progression gemoney savings
                Progression& p = registry.progressions.get(registry.progressions.entities[0]);
                Player& player = registry.players.get(registry.players.entities[0]);
				player.germoney_count += p.germoney_savings;

				removeStartScreen();
				createGameplayCutScene();
			}
		}
		else if (current_state == GameState::SHOP && button == GLFW_MOUSE_BUTTON_LEFT)
		{

			// SHOPPING LOGIC HERE
			// if they clicked on a buff, buy it and depending on the type, alter progression, player buffs or game level,
			// and deduct the buff price from the players progression savings
			Entity e;
			if(isClickableBuffClicked(&e)) {
				// GET BUFF TYPE AND PRICE
				ClickableBuff& c = registry.clickableBuffs.get(e);
				Progression& p = registry.progressions.get(registry.progressions.entities[0]);

				if(c.price <= p.germoney_savings) {
					// MONEY SOUND {s}

					// BUY AND APPLY
					p.germoney_savings -= c.price;
					
					if(c.type == -1) {
						level += 1; // hope this works xx
					} else if (c.type == -2) {
						if (p.slots_unlocked == 1) {
							p.slots_unlocked = 4;
						} else {
							p.slots_unlocked = 9;
						}
					} else {
						registry.progressions.get(registry.progressions.entities[0]).pickedInNucleus.push_back(c.type);
					}

					// remove buff
					registry.remove_all_components_of(e);
				} else {
					// WOMP WOMP SOUND {s}
				}
			}

			if (getClickedButton() == ButtonType::BACKBUTTON)
			{
				Mix_PlayChannel(-1, click_sound, 0);
				removeShopScreen();
				createStartScreen(LOGO_POSITION);
				GameState temp = current_state;
				current_state = previous_state;
				previous_state = temp;
			}
		}
		else if (current_state == GameState::INFO && button == GLFW_MOUSE_BUTTON_LEFT)
		{
			if (getClickedButton() == ButtonType::BACKBUTTON)
			{
				Mix_PlayChannel(-1, click_sound, 0);
				removeInfoScreen();
				createStartScreen(LOGO_POSITION);
				GameState temp = current_state;
				current_state = previous_state;
				previous_state = temp;
			}
		}
		// gameover state -> start screen state // FLAG this should be done with the button on the screen
		else if (current_state == GameState::GAME_OVER && button == GLFW_MOUSE_BUTTON_LEFT) 
		{
			Entity e;

			if (getClickedButton() == ButtonType::PROCEEDBUTTON)
			{
				Mix_PlayChannel(-1, click_sound, 0);
				previous_state = current_state;
				current_state = GameState::START_SCREEN_ANIMATION;
				moveSelectedBuffsToProgression();
				removeGameOverScreen();
				restart_game();

			}
			else if (isClickableBuffClicked(&e)) {
				Mix_PlayChannel(-1, click_sound, 0);
				handleClickableBuff(e);
			}
		}
		else if (current_state == GameState::PAUSE && button == GLFW_MOUSE_BUTTON_LEFT)
		{
			if (getClickedButton() == ButtonType::SAVEBUTTON)
			{
				Mix_PlayChannel(-1, click_sound, 0);
				if (level < BOSS_LEVEL && !progress_map["tutorial_mode"]) {
					saveGame();
					saveProgress();
				}
			}
			else if (getClickedButton() == ButtonType::RESUMEBUTTON){
				Mix_PlayChannel(-1, click_sound, 0);
				current_state = GameState::GAME_PLAY;
				removePauseScreen();
				Mix_ResumeMusic(); // Resume music
			}
		}

		else if (current_state == GameState::VICTORY && button == GLFW_MOUSE_BUTTON_LEFT)
		{
			previous_state = current_state;
			current_state = GameState::START_SCREEN_ANIMATION;
			removeCutScene();
			restart_game();
		}
	}
}

void WorldSystem::shootGun() {
    Gun &gun = registry.guns.get(registry.guns.entities[0]);
    Motion &gun_motion = registry.motions.get(registry.guns.entities[0]);
    Player &player = registry.players.get(registry.players.entities[0]);

	float base_angle_rad = glm::radians(180.f + gun_motion.angle);

	if (gun.cooldown_timer_ms <= 0.0f) {
        gun.cooldown_timer_ms = GUN_COOLDOWN_MS; // Reset cooldown
		Mix_PlayChannel(-1, player_shoot_sound, 0);

		for(int i = 0; i < player.bulletsPerShot; i++) {
			float offset_deg = (player.bulletsPerShot > 1)
			? -player.angleConeRadius/2 + (2.f * player.angleConeRadius/2 * i / (player.bulletsPerShot - 1))
			: 0.f;
			float angle_rad = base_angle_rad + glm::radians(offset_deg);

			vec2 velocity = {
				cos(angle_rad) * player.bulletSpeed,
				sin(angle_rad) * player.bulletSpeed
			};
			velocity = {velocity.y, -velocity.x};
			
			Entity projectiles = createProjectile(gun_motion.position, {PROJECTILE_SIZE, PROJECTILE_SIZE}, velocity, player.gun_projectile_damage);

			RenderRequest& render_request = registry.renderRequests.get(projectiles);
			render_request.used_texture = TEXTURE_ASSET_ID::GUN_PROJECTILE;

			Projectile &projectile = registry.projectiles.get(projectiles);
			projectile.from_enemy = false;
		}
	}
}

void WorldSystem::moveSelectedBuffsToProgression() {
	Progression& p = registry.progressions.get(registry.progressions.entities[0]);
	p.pickedInNucleus.clear();
	p.buffsFromLastRun.clear();

	for(int i = 0; i < registry.clickableBuffs.entities.size(); i++) {
		ClickableBuff& c = registry.clickableBuffs.get(registry.clickableBuffs.entities[i]);
		if(c.picked == true) {
			p.pickedInNucleus.push_back(c.type);
		}
	}

}

bool WorldSystem::isClickableBuffClicked(Entity* return_e) {
	float mouse_x = game_mouse_pos_x; 
    float mouse_y = game_mouse_pos_y;

	Camera& camera = registry.cameras.components[0];
	vec2 camera_pos = camera.position;	
	vec2 m_pos = {mouse_x, mouse_y};
	

	for(int i = 0; i < registry.clickableBuffs.entities.size(); i++) {
		ClickableBuff& c = registry.clickableBuffs.get(registry.clickableBuffs.entities[i]);
		Motion& c_motion = registry.motions.get(registry.clickableBuffs.entities[i]); // GUARANTEED TO HAVE A POSITION

		vec2 c_pos = c_motion.position;

		if(mouseBuffIntersect(m_pos , c_pos)) {
			*return_e = registry.clickableBuffs.entities[i];
			return true;
		}
	}
	return false;
}

void WorldSystem::handleClickableBuff(Entity e) { // FOR NUCLEUS MENU 
	// Find a free slot if there is one availibe
	// move buff to slot if its not already in a slot, if it is move it to return position
	Entity s;

	ClickableBuff& c = registry.clickableBuffs.get(e);
	Motion& c_m = registry.motions.get(e);
	
	if(c.picked) {
		// move it back
		// std::cout << "c current pos" << c_m.position.x << ", " << c_m.position.y << std::endl;
		// std::cout << "c return pos" << c.returnPosition.x << ", " << c.returnPosition.y << std::endl;


		c_m.position = c.returnPosition;
		c.picked = false;
		
		// UNFILL THE RESPECTIVE SLOT...
		Entity slot_to_remove = c.slotEntity;
		Slot& slot = registry.slots.get(slot_to_remove);
		slot.filled = false;

	} else {
		if(isFreeSlot()) {
			s = getFreeSlot();
		} else {
			// no free slots so do nothing
			return;
		}
	
		Motion& s_pos = registry.motions.get(s);	
		Slot& slot = registry.slots.get(s);
		c.slotEntity = s;
		c_m.position = s_pos.position;
		c.picked = true;
		slot.filled = true;
	}
}

bool WorldSystem::isFreeSlot(){
	
	for(int i = 0; i < registry.slots.size(); i++) {
		if(registry.slots.get(registry.slots.entities[i]).filled == false) {
			return true;
		}
	}
	return false;
}

Entity WorldSystem::getFreeSlot(){
	for(int i = 0; i < registry.slots.size(); i++) {
		if(registry.slots.get(registry.slots.entities[i]).filled == false) {
			return registry.slots.entities[i];
		}
	}
}

bool WorldSystem::mouseBuffIntersect(vec2 mouse_pos, vec2 c_pos) {
	float c_top = c_pos.y - BUFF_HEIGHT/2;
	float c_bottom = c_pos.y + BUFF_HEIGHT/2;

	if (mouse_pos.y >= c_top && mouse_pos.y <= c_bottom){ // Y match
		float c_l = c_pos.x - BUFF_WIDTH/2;
		float c_r = c_pos.x + BUFF_WIDTH/2;
		if(mouse_pos.x >= c_l && mouse_pos.x <= c_r) {
			return true;
		}
	}

	return false;
}

bool WorldSystem::isButtonClicked(screenButton &button)
{
    float button_x = button.center[0];
    float button_y = button.center[1];

    float prenormalized_x = button_x - device_mouse_pos_x;
    float prenormalized_y = button_y - device_mouse_pos_y;

    if (current_state == GameState::GAME_PLAY || current_state == GameState::PAUSE || current_state == GameState::GAME_OVER) {
        Camera& camera = registry.cameras.components[0];
        vec2 camera_pos = camera.position;

        prenormalized_x -= camera_pos.x;
        prenormalized_y -= camera_pos.y;
    }

    float x_distance = std::abs(prenormalized_x);
    float y_distance = std::abs(prenormalized_y);
    
    bool res = (x_distance < button.w / 2.f) && (y_distance < button.h / 2.f);

    return res;
}


void WorldSystem::collectBuff(Entity player_entity, Entity buff_entity)
{

	Player &player = registry.players.get(player_entity);
	if (!registry.buffs.has(buff_entity))
	{
		return;
	}

	Buff &buff = registry.buffs.get(buff_entity);
	buff.collected = true;

	applyBuff(player, buff.type);
}

void WorldSystem::applyBuff(Player& player, int buff_type)
{
	bool skipUIRender = false;

	switch (buff_type)
	{
	case 0: // Tail
		player.speed += player.speed * 0.05f;
		break;

	case 1: // Mitochondria is the powerhouse of the cell
		player.max_dash_count ++;
		createDashRecharge();
		break;

	case 2: // Hemoglobin
		player.detection_range -= player.dash_cooldown_ms * 0.05f;
		break;

	case 3: // Golgi Apparatus Buff (need to be implemented)
		player.dash_cooldown_ms = player.dash_cooldown_ms * 0.95;
		break;

	case 4: // Chloroplast
		player.healing_rate += 0.03;
		break;
	case 5: // Plant Cell Wall
		player.sheilds += 1;
		break;
	case 6: // Amino Acid
		//	Increase Player Damage
		player.dash_damage += 0.05;
		break;
	case 7: // Lysosyme
		// Shoot multiple projectiles?
		player.bulletsPerShot++;
		break;
	case 8: // CytoPlasm
		player.max_health += 10;
		break;
	case 9: // Pilli OR Virality
		//	increases bullet area cone // projectile speed instead?
		player.bulletSpeed += 200;
		break;
	case 10: // Spare Nucleus
		player.extra_lives++;
		break;
	case 11: // Vacuole
		//	 - doesnt render in the ui... - Heals hp -------------------------------> POPUP CALL HERE
		player.current_health += 50;
		skipUIRender = true;
		break;
	case 12: // Endoplasmic Reticulum 
		player.default_healing_timer -= 250;
		break;
	case 13: // oceloid cell
		player.minimapViewRange++;
		break;
	case 14: // Secretor cell
		player.dashDecay += 0.005; // SUPER OP
		break;
	case 15: // IDK some weird orange and pink shit	
		player.angleConeRadius += 30;
		break;
	case 16: // Peroxisomes
		//	Removes a nerf ...?
		break;
	case 17: // Mutation
		//	Some nerf?
		break;
	case 18: // Facehugger
		//	Some Poison?
		break;
	case 19: // Black Goo  - temp turn screen dark?
		//	Some nerf?
		break;


		default:
		std::cerr << "Unknown buff type: " << buff_type << std::endl;
		break;
	}
    
	if(!skipUIRender) {
        player.buffsCollected[buff_type] += 1;
		renderCollectedBuff(renderer, buff_type);
	}
}


void WorldSystem::initiatePlayerDash()
{
	Entity &player_e = registry.players.entities[0];
	Player &player = registry.players.get(player_e);
	Motion &player_motion = registry.motions.get(player_e);

	if (isDashing())
	{
		for (Entity& entity : registry.dashes.entities)
		{
			registry.dashes.remove(entity);
		}
	}

	player.dash_count--;

	Dashing &d = registry.dashes.emplace(Entity());
	d.angle_deg = player_motion.angle;
	float angle_radians = (d.angle_deg - 90) * (M_PI / 180.0f);
	d.velocity = {
		player.dash_speed * cosf(angle_radians),
		player.dash_speed * sinf(angle_radians)
	};
	d.timer_ms = DASH_DURATION_MS;
	player.dash_cooldown_timer_ms = player.dash_cooldown_ms;

	// Change animation frames
	toggleDashAnimation(player_e, true);

	particle_system.createParticles(PARTICLE_TYPE::RIPPLE_PARTICLE, player_motion.position, 4);
}

bool WorldSystem::canDash()
{
	Player &player = registry.players.get(registry.players.entities[0]);
	return player.dash_count > 0; // PLAYER HAS 1 DASH SAVED ATLEAST
}

bool WorldSystem::isDashing()
{
	return registry.dashes.size() > 0;
}

void WorldSystem::tileProceduralMap() {
	if(initializedMap) {
		return;
	}

	vec2 camera_pos = registry.cameras.get(registry.cameras.entities[0]).grid_position;

	float cameraGrid_x = camera_pos.x;
	float cameraGrid_y = camera_pos.y;

	ProceduralMap& map = registry.proceduralMaps.get(registry.proceduralMaps.entities[0]);

	// setting map bounds
	int left = (cameraGrid_x - (WINDOW_GRID_WIDTH / 2) - CHUNK_DISTANCE / 2);	 
	int right = (cameraGrid_x + (WINDOW_GRID_WIDTH / 2) + CHUNK_DISTANCE / 2);	 
	int top = (cameraGrid_y - (WINDOW_GRID_HEIGHT / 2) - CHUNK_DISTANCE / 2);	
	int bottom = (cameraGrid_y + (WINDOW_GRID_HEIGHT / 2) + CHUNK_DISTANCE / 2);

	left = -3;
	right = 23;
	top = -3;
	bottom = 23;


	for (int x = left; x < right; x += 1)
	{
		for (int y = top; y < bottom; y += 1)
		{
			vec2 gridCoord = {x, y};

			// check for already existing tiles, don't need to draw these again
			if (currentTiles.find(x) != currentTiles.end() && currentTiles[x].find(y) != currentTiles[x].end()) continue;

			if (x < map.left || x >= map.right || y < map.top || y >= map.bottom) {
				currentTiles[x][y] = 1;
				addWallTile(gridCoord);
			} else { // if (glm::distance(gridCoord, {cameraGrid_x, cameraGrid_y}) <= CHUNK_DISTANCE)
				if (map.map[x][y] == tileType::EMPTY) 
				{
					// if its being tiled what tile to put
					currentTiles[x][y] = 1;
					addParalaxTile(gridCoord);
                } 
				else if (map.map[x][y] == tileType::PORTAL) 
				{
					currentTiles[x][y] = 1;
					addParalaxTile(gridCoord);
                    addPortalTile(gridCoord);
                } 
				else 
				{
					currentTiles[x][y] = 1;
					addWallTile(gridCoord);
				}
			}
		}
	}

	initializedMap = true;
}

void WorldSystem::handleRippleEffect(float elapsed_ms)
{
        
    Entity player_entity = registry.players.entities[0];
    Motion& player_motion = registry.motions.get(player_entity);
    
    static float ripple_timer = 0.0f;
    ripple_timer += elapsed_ms;
    
    if (ripple_timer >= 5.0f) {
        particle_system.createPlayerRipples(player_entity);
        ripple_timer = 0.0f;
    }
}
// M3 Feature JSON Saving and Loading
// On click of the save button in Pause Screen we save the progress so far.
// Such as Player Status, Position and Map along with the buffs and projectiles

void WorldSystem::saveGame() {
	std::cout << "Saving Game!" << std::endl;
	
	json gameData;
	
	// save map data
	Entity proceduralMapEntity = registry.proceduralMaps.entities[0];
	ProceduralMap& map = registry.proceduralMaps.get(proceduralMapEntity);
	
	gameData["map"] = json(map);

	// save player status + position
	Entity playerEntity = registry.players.entities[0];
	Player& player = registry.players.get(playerEntity);
	Motion& playerMotion = registry.motions.get(playerEntity);
	
	gameData["player"]["playerStatus"] = json(player);
	gameData["player"]["motion"] = json(playerMotion);

	// save buff status
	std::vector<BuffUI> buffs = registry.buffUIs.components;

	gameData["buffs"] = json(buffs);

	// save progress
	Entity progressEntity = registry.progressions.entities[0];
	Progression& prog = registry.progressions.get(progressEntity);
	gameData["progress"] = json(prog);

	// save projectiles
	std::vector<Entity> projectileEntities = registry.projectiles.entities;
	
	json motionsArray = json::array();
	// make json array to contain motions for projectiles
	for (auto e : projectileEntities) {
		Motion& projectileMotion = registry.motions.get(e);
		
		json motionJson = json(projectileMotion);

		motionsArray.push_back(motionJson);
	}

	gameData["projectiles"] = motionsArray;

	std::string filename = std::string(PROJECT_SOURCE_DIR) + "/data/save/world_status.json";

	std::ofstream o(filename);

	if (!o.is_open()) {
		std::cerr << "Could not open file for writing JSON" << std::endl;
		return;
	}	

	o << gameData.dump(4) << std::endl;

	std::cout << "Done Saving!" << std::endl;
}

// checker to see if the file exists
bool WorldSystem::checkLoadFileExists() {
	std::string filename = std::string(PROJECT_SOURCE_DIR) + "/data/save/world_status.json";
	std::ifstream f(filename.c_str());
	return f.good();
}

// load the game from the file, after initialization we over-write the info we want to load to the game
// happens on press L in the game start screen
void WorldSystem::loadGame() {
	std::cout << "Loading Game!" << std::endl;

	std::string filename = std::string(PROJECT_SOURCE_DIR) + "/data/save/world_status.json";
	std::ifstream i(filename);

	if (!i.is_open()) {
		std::cerr << "Could not open file for reading JSON" << std::endl;
		return;
	}

	json gameData;

	i >> gameData;


	if (registry.players.entities.size() > 0) {
		std::cout << "Player Exists Fine" << std::endl;
	}
	
	// get player
	Entity playerEntity = registry.players.entities[0];
	Player& player = registry.players.get(playerEntity);

	if (registry.motions.has(playerEntity)) {
		std::cout << "Player Motion Exists Fine" << std::endl;
	}

	// load player motion & status
	Motion& playerMotion = registry.motions.get(playerEntity);
	player = gameData["player"]["playerStatus"].get<Player>();
	std::cout << "gemoney count: " << player.germoney_count << std::endl;
	playerMotion = gameData["player"]["motion"].get<Motion>();

	if (registry.proceduralMaps.entities.size() > 0) {
		std::cout << "Procedural Map Exists Fine" << std::endl;
	}

	// load map
	Entity mapEntity = registry.proceduralMaps.entities[0];
	ProceduralMap& map = registry.proceduralMaps.get(mapEntity);
	map = gameData["map"].get<ProceduralMap>();

	if (registry.buffUIs.entities.size() == 0) {
		std::cout << "Buff UI should be empty" << std::endl;
	}

	// load buffs 
	std::vector<BuffUI> buffs = gameData["buffs"].get<std::vector<BuffUI>>();

	for (auto buff : buffs) {
		renderCollectedBuff(renderer, buff.buffType);
	}

	// load projectiles
	std::vector<Motion> projectiles = gameData["projectiles"].get<std::vector<Motion>>();

	for (auto projectile : projectiles) {
		createProjectile(projectile.position, projectile.scale ,projectile.velocity);
	}

	// check progression exists
	if (registry.progressions.entities.size() > 0) {
		std::cout << "Progression Exists Fine" << std::endl;
	}

	// load progression
	Entity progressEntity = registry.progressions.entities[0];
	Progression& prog = registry.progressions.get(progressEntity);
	prog = gameData["progress"].get<Progression>();
	std::cout << "Done Loading!" << std::endl;
}

// save progress along with the status
// we keep track of the current level + if tutorial is done
// cannot be saved for boss stage
void WorldSystem::saveProgress() {
	std::string progress_filename = std::string(PROJECT_SOURCE_DIR) + "/data/save/progress.json";

	std::ofstream p(progress_filename);

	if (!p.is_open()) {
		std::cerr << "Could not open file for writing JSON (Progress)" << std::endl;
		return;
	}

	json progressData;

	progressData["progress"] = json(progress_map);
	progressData["levels"] = json(level);

	p << progressData.dump(4) << std::endl;

	std::cout << "Done Saving Progress!" << std::endl;
}

// load the json and overwrite the saved variables
void WorldSystem::loadProgress() {
	std::string progress_filename = std::string(PROJECT_SOURCE_DIR) + "/data/save/progress.json";

	std::ifstream p(progress_filename);

	removeInfoBoxes();

	if (registry.keys.size() != 0) {
		registry.remove_all_components_of(registry.keys.entities[0]);
	}

	if (registry.chests.size() != 0) {
		registry.remove_all_components_of(registry.chests.entities[0]);
	}

	if (!p.is_open()) {
		std::cerr << "Could not open file for reaidng JSON (Progress)" << std::endl;
		return;
	}

	json progressData;

	p >> progressData;

	progress_map = progressData["progress"].get<std::map<std::string, bool>>();

	level = progressData["levels"].get<int>();
}

void WorldSystem::startTheme() {
	Mix_PlayMusic(background_music, -1);
}

void WorldSystem::placeBuffsOnShopScreen() {
	bool offerSlotBoost = false;
	int unlocked = 0;

	if(registry.progressions.get(registry.progressions.entities[0]).slots_unlocked < 9) {
		offerSlotBoost = true;
		unlocked = registry.progressions.get(registry.progressions.entities[0]).slots_unlocked;
	}

	int placed = 0;

	for(int i = 0; i < registry.shops.entities.size(); i++) {
		// Get the texture asset id, then check if its a plate, if so then get its position and put a clickable there.

		if(!registry.renderRequests.has(registry.shops.entities[i])) {
			continue;
		}

		RenderRequest& r = registry.renderRequests.get(registry.shops.entities[i]);

		if(r.used_texture != TEXTURE_ASSET_ID::SHOP_PLATE) 
		{
			continue;
		}

		vec2 position = registry.motions.get(registry.shops.entities[i]).position;

		if(placed == 0) {
			ClickableBuff& c = registry.clickableBuffs.get(createClickableShopBuff(position, -1));
			c.price = 1000.0;
		} else {
			if(offerSlotBoost) {
				ClickableBuff& c = registry.clickableBuffs.get(createClickableShopBuff(position, -2));
				c.price = 200.0 * unlocked;	// dynamic pricing
				offerSlotBoost = false; // OFFERED NOW.
			} else {
				// GET RANDOM TYPE
				// GET PRICE PER TYPE
				int buffType = getRandomBuffType();
				if(buffType == 11) {
					buffType = 10;
				}

				std::vector<int> commonBuffs = {0, 1, 2, 3, 5, 6, 11};
				std::vector<int> rareBuffs = {4, 8, 9, 12};
				std::vector<int> eliteBuffs = {7, 10, 13, 14};

				ClickableBuff& c = registry.clickableBuffs.get(createClickableShopBuff(position, buffType));
				c.price = 1000;
				
				if (std::find(commonBuffs.begin(), commonBuffs.end(), buffType) != commonBuffs.end()) {
					c.price = 50.0f;
				} else if (std::find(rareBuffs.begin(), rareBuffs.end(), buffType) != rareBuffs.end()) {
					c.price = 100.0f;
				} else if (std::find(eliteBuffs.begin(), eliteBuffs.end(), buffType) != eliteBuffs.end()) {
					c.price = 200.0f;
				}
			}
		}

		placed++;
	}
}
