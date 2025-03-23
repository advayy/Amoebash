// Header
#include "world_system.hpp"
#include "world_init.hpp"
#include "common.hpp"

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

bool tutorial_mode = true;

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
	if (dash_sound_1 != nullptr)
		Mix_FreeChunk(dash_sound_1);
	if (dash_sound_2 != nullptr)
		Mix_FreeChunk(dash_sound_2);
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

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	dash_sound_1 = Mix_LoadWAV(audio_path("dash_1.wav").c_str());
	dash_sound_2 = Mix_LoadWAV(audio_path("dash_2.wav").c_str());

	if (background_music == nullptr || dash_sound_1 == nullptr || dash_sound_2 == nullptr)
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

	this->renderer = renderer_arg;

	// start playing background music indefinitely

	// std::cout << "Starting music..." << std::endl;
	Mix_PlayMusic(background_music, -1);

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


void WorldSystem::spawnEnemies(float elapsed_ms_since_last_update)
{
	// std::cout << "WS:spawn - f1" << std::endl;

	Motion &player_motion = registry.motions.get(registry.players.entities[0]);
	if (!tutorial_mode)
	{
		// std::cout << "WS:spawn - f2" << std::endl;

		// spawn new invaders
		next_enemy_spawn -= elapsed_ms_since_last_update * current_speed;
		if (next_enemy_spawn < 0.f && !gameOver)
		{
			// std::cout << "WS:spawn - f3" << std::endl;
			if (registry.enemies.entities.size() < MAX_ENEMIES_COUNT)
			{
				// std::cout << "WS:spawn - f4" << std::endl;
				// reset timer
				next_enemy_spawn = (ENEMY_SPAWN_RATE_MS / 2) + uniform_dist(rng) * (ENEMY_SPAWN_RATE_MS / 2);

				// randomize position
				// randomize empty tile on mpa
				std::pair<int, int> enemyPosition = getRandomEmptyTile(registry.proceduralMaps.get(registry.proceduralMaps.entities[0]).map);
				// std::cout << "WS:spawn - f5" << std::endl;

				int random_num = rand();
				if (random_num % 3 == 0)
				{
					// std::cout << "WS:spawn - f6" << std::endl;
					createSpikeEnemy(renderer, gridCellToPosition({ enemyPosition.second, enemyPosition.first }));
				}
				else if (random_num % 3 == 1)
				{
					// std::cout << "WS:spawn - f7" << std::endl;
					createRBCEnemy(renderer, gridCellToPosition({ enemyPosition.second, enemyPosition.first }));
				}
				else if (registry.bacteriophageAIs.entities.size() < MAX_BACTERIOPHAGE_COUNT)
				{
					// std::cout << "WS:spawn - f8" << std::endl;
					while (glm::distance(player_motion.position, gridCellToPosition({ enemyPosition.second, enemyPosition.first })) < SPIKE_ENEMY_DETECTION_RADIUS)
					{
						// std::cout << "WS:spawn - f9" << std::endl;
						// randomize empty tile on mpa
						enemyPosition = getRandomEmptyTile(registry.proceduralMaps.get(registry.proceduralMaps.entities[0]).map);
					}

					// std::cout << "WS:spawn - f10" << std::endl;
					int index = (int)(uniform_dist(rng) * MAX_BACTERIOPHAGE_COUNT);
					while (bacteriophage_idx.find(index) != bacteriophage_idx.end())
					{
						// std::cout << "WS:spawn - f11" << std::endl;
						index = (int)(uniform_dist(rng) * MAX_BACTERIOPHAGE_COUNT);
					}
					bacteriophage_idx[index] = 1;
					// std::cout << "WS:spawn - f12" << std::endl;
					createBacteriophage(renderer, gridCellToPosition({ enemyPosition.second, enemyPosition.first }), index);
				}
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

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update)
{

	// M1 Feature - Camera controls
	// std::cout << "WS:step - f1" << std::endl;

	updateCamera(elapsed_ms_since_last_update);
	// std::cout << "WS:step - f2" << std::endl;

	if (tutorial_mode && registry.infoBoxes.size() == 0) {
		createInfoBoxes();
	}
	// std::cout << "WS:step - f3" << std::endl;

	updateMouseCoords();
	// std::cout << "WS:step - f4" << std::endl;

	updateHuds();
	// std::cout << "WS:step - f5" << std::endl;

	handlePlayerMovement(elapsed_ms_since_last_update);
	// std::cout << "WS:step - f6" << std::endl;

	handlePlayerHealth(elapsed_ms_since_last_update);
	// std::cout << "WS:step - f7" << std::endl;

	spawnEnemies(elapsed_ms_since_last_update);
	// std::cout << "WS:step - f8" << std::endl;

	handleProjectiles(elapsed_ms_since_last_update);
	// std::cout << "WS:step - f9" << std::endl;

    tileProceduralMap();
	
	// std::cout << "WS:step - f10" << std::endl;

	if (checkPortalCollision()) {
        Entity screen_state_entity = renderer->get_screen_state_entity();
        ScreenState &screen = registry.screenStates.get(screen_state_entity);
        screen.darken_screen_factor = 1;
       	darken_screen_timer = 0.0f;
        current_state = GameState::NEXT_LEVEL;
		goToNextLevel();
		return true;
	}

	// IF PORTAL COLLISION THEN GO TO NEXT LEVEL...

	// std::cout << "WS:step - f11" << std::endl;

    // Update the darken screen timer
    if (darken_screen_timer >= 0.0f) {
        darken_screen_timer += elapsed_ms_since_last_update;
        if (darken_screen_timer >= 1000.0f) {
            Entity screen_state_entity = renderer->get_screen_state_entity();
            ScreenState &screen = registry.screenStates.get(screen_state_entity);
            screen.darken_screen_factor = -1;
            darken_screen_timer = -1.0f; // Stop the timer
        }
    }
	// std::cout << "WS:step - f12" << std::endl;

	// step the particle system only when its needed
	// for optimaztion, we could only step the particles that are on screen
	particle_system.step(elapsed_ms_since_last_update);
	// std::cout << "WS:step - f13" << std::endl;

	return true;
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
		player.healing_timer_ms = PLAYER_DEFAULT_HEALING_TIMER_MS;
		player.current_health += player.max_health * player.healing_rate;
		if (player.current_health > player.max_health)
		{
			player.current_health = player.max_health;
		}
	}

	if (player.current_health <= 0 && current_state != GameState::GAME_OVER)
	{
		previous_state = current_state;
		current_state = GameState::GAME_OVER;
		createGameOverScreen();
	}
}


// Handle player movement
void WorldSystem::handlePlayerMovement(float elapsed_ms_since_last_update) {
	// if the player is not dashing, then have its velocity be base speed * by direction to mouse, if the mouse is outside deadzone
	
	Player &player = registry.players.get(registry.players.entities[0]);
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
	
	initializedMap = false;
	currentTiles.clear();

    int enemySize = registry.enemies.entities.size();
    for (int i = 0; i < enemySize; i++) {
        registry.remove_all_components_of(registry.enemies.entities.back());
    }

	gameOver = false;

	std::pair<int, int> playerPosition;

	createProceduralMap(renderer, vec2(MAP_WIDTH, MAP_HEIGHT), tutorial_mode, playerPosition);

	Player &player = registry.players.get(registry.players.entities[0]);
	Motion &playerMotion = registry.motions.get(registry.players.entities[0]);
	playerMotion.position = gridCellToPosition(vec2(playerPosition.second, playerPosition.first));
	Camera &camera = registry.cameras.get(registry.cameras.entities[0]);
	camera.position = playerMotion.position;
	bacteriophage_idx.clear();
    // print exiting
    // std::cout << "Exiting createProceduralMap" << std::endl;
	return;
}


// Reset the world state to its initial state
void WorldSystem::restart_game()
{

	// std::cout << "Restarting..." << std::endl;
    // std::cout << "Level: " << level + 1 << std::endl;
    
	// Debugging for memory/component leaks
	registry.list_all_components();
    
	// Reset the game speed
	current_speed = 1.f;
    
	level += 1;
	next_enemy_spawn = 0;
	enemy_spawn_rate_ms = ENEMY_SPAWN_RATE_MS;

	bacteriophage_idx.clear();
    
	// FLAG
	gameOver = false;

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

	// debugging for memory/component leaks
	registry.list_all_components();
    
	// std::cout << "Creating Procedural Map, tutorial mode status :" << tutorial_mode << std::endl;

    std::pair<int, int> playerPosition;
	createProceduralMap(renderer, vec2(MAP_WIDTH, MAP_HEIGHT), tutorial_mode, playerPosition);

	if (tutorial_mode) {
		createPlayer(renderer, gridCellToPosition({0, 10}));
		createSpikeEnemy(renderer, gridCellToPosition({12, 10}));
		createKey(renderer, gridCellToPosition({16, 10}));
		createChest(renderer, gridCellToPosition({19, 10}));
	} else {
		createPlayer(renderer, gridCellToPosition(vec2(playerPosition.second, playerPosition.first)));
	}
	
	createMiniMap(renderer, vec2(MAP_WIDTH, MAP_HEIGHT));

	createCamera();


	createStartScreen();


	stateTimer = BOOT_CUTSCENE_DURATION_MS;
	(renderer);

	createUIElement(NUCLEUS_UI_POS,
					vec2(NUCLEUS_UI_WIDTH, NUCLEUS_UI_HEIGHT),
					TEXTURE_ASSET_ID::NUCLEUS_UI,
					EFFECT_ASSET_ID::UI);
	createHealthBar();
	createDashRecharge();
	createUIElement(GERMONEY_UI_POS,
					vec2(GERMONEY_UI_WIDTH, GERMONEY_UI_HEIGHT),
					TEXTURE_ASSET_ID::GERMONEY_UI,
					EFFECT_ASSET_ID::UI);
	createUIElement(WEAPON_PILL_UI_POS,
					vec2(WEAPON_PILL_UI_WIDTH, WEAPON_PILL_UI_HEIGHT),
					TEXTURE_ASSET_ID::WEAPON_PILL_UI,
					EFFECT_ASSET_ID::UI);
}

// Compute collisions between entities. Collisions are always in this order: (Player | Projectiles, Enemy | Wall | Buff)
void WorldSystem::handle_collisions()
{
	for (auto& entity : registry.collisions.entities)
	{
		Collision& collision = registry.collisions.get(entity);
		Entity entity2 = collision.other;

		if (registry.bacteriophageProjectiles.has(entity2))
		{
			if (registry.players.has(entity)) // HANDLE PROJECTILE/PLAYER COLLISION
			{
				Player& player = registry.players.get(entity);
				Projectile& projectile = registry.projectiles.get(entity2);

				// Player takes damage
				player.current_health -= projectile.damage;

				// remove projectile
				registry.remove_all_components_of(entity2);
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
					// std::cout << "Mesh collision imminent between player and hexagon" << std::endl;
				}
				else 
				{
					// std::cout << "No mesh collision predicted soon" << std::endl;
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
					// remove chest
					registry.remove_all_components_of(entity);
					registry.remove_all_components_of(entity2);

					if (tutorial_mode) {
						current_state = GameState::NEXT_LEVEL;
						tutorial_mode = false;
						removeInfoBoxes();
						goToNextLevel();
					}
				}
			}
		}
		else if (registry.enemies.has(entity2))
		{
			Enemy& enemy = registry.enemies.get(entity2);
			if (registry.projectiles.has(entity))
			{
				Projectile& projectile = registry.projectiles.get(entity);

				// Invader takes damage
				enemy.health -= projectile.damage;

				// remove projectile
				registry.remove_all_components_of(entity);

				// if invader health is below 0
				// remove invader and increase points
				// buff created
				if (enemy.health <= 0)
				{
					if (registry.bacteriophageAIs.has(entity2))
					{
						bacteriophage_idx.erase(registry.bacteriophageAIs.get(entity2).placement_index);
					}

					vec2 enemy_position = registry.motions.get(entity2).position;
					registry.remove_all_components_of(entity2);
					// level += 1;
					Mix_PlayChannel(-1, dash_sound_2, 0); // FLAG MORE SOUNDS

					createBuff(vec2(enemy_position.x, enemy_position.y));
					particle_system.createParticles(PARTICLE_TYPE::DEATH_PARTICLE, enemy_position, 15);
				}
			}
			else if (registry.players.has(entity))
			{
				if (isDashing())
				{
					enemy.health -= PLAYER_DASH_DAMAGE;

					if (enemy.health <= 0)
					{
						if (registry.bacteriophageAIs.has(entity2))
						{
							bacteriophage_idx.erase(registry.bacteriophageAIs.get(entity2).placement_index);
						}

						vec2 enemy_position = registry.motions.get(entity2).position;
						points += 1;
						registry.remove_all_components_of(entity2);

						createBuff(vec2(enemy_position.x, enemy_position.y));
						particle_system.createParticles(PARTICLE_TYPE::DEATH_PARTICLE, enemy_position, 15);
					}
				}
				else
				{
					// womp womp	game over or vignetted??
					uint current_time = SDL_GetTicks();

					// then apply damage.
					if (!registry.damageCooldowns.has(entity))
					{
						//  add the component and apply damage
						registry.damageCooldowns.insert(entity, { current_time });

						Player& player = registry.players.get(entity);
						player.current_health -= 1; // FLAG this is not the right kind of damage...
						Mix_PlayChannel(-1, dash_sound_2, 0);
					}
					else
					{
						// retrieve the cooldown component
						DamageCooldown& dc = registry.damageCooldowns.get(entity);
						if (current_time - dc.last_damage_time >= 500)
						{
							dc.last_damage_time = current_time;
							Player& player = registry.players.get(entity);
							player.current_health -= 1;
							Mix_PlayChannel(-1, dash_sound_2, 0);
						}
					}
				}
			}
		}
		else if (registry.buffs.has(entity2) && registry.players.has(entity))
		{
			collectBuff(entity, entity2);
		}
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
		}
		else if (current_state == GameState::PAUSE)
		{
			current_state = GameState::GAME_PLAY;
			removePauseScreen();
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
				previous_state = GameState::GAME_PLAY;
				current_state = GameState::GAME_OVER;
				createGameOverScreen();
			}
		}
	}

	// Q for going to start screen
	if (key == GLFW_KEY_Q)
	{
		if (action == GLFW_RELEASE)
		{
			restart_game();
			current_state = GameState::START_SCREEN;
		}
	}
}

void WorldSystem::on_mouse_move(vec2 mouse_position)
{

	// record the current mouse position
	device_mouse_pos_x = mouse_position.x;
	device_mouse_pos_y = mouse_position.y;
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
			if (button == GLFW_MOUSE_BUTTON_LEFT)
			{
				if (canDash())
				{
					initiatePlayerDash();
					Mix_PlayChannel(-1, dash_sound_1, 0);
				}
			}
		}
		else if (current_state == GameState::START_SCREEN && button == GLFW_MOUSE_BUTTON_LEFT)
		{
			ButtonType clickedButton = getClickedButton();

			if (clickedButton == ButtonType::SHOPBUTTON) 
			{
				previous_state = current_state;
				current_state = GameState::SHOP;
				removeStartScreen();
				createShopScreen();
			}
			else if (clickedButton == ButtonType::INFOBUTTON) 
			{
				previous_state = current_state;
				current_state = GameState::INFO;
				removeStartScreen();
				createInfoScreen();
			}
			else if (clickedButton == ButtonType::STARTBUTTON) 
			{
				previous_state = current_state;
				current_state = GameState::GAMEPLAY_CUTSCENE;
				removeStartScreen();
				createGameplayCutScene();
			}
		}
		else if (current_state == GameState::SHOP && button == GLFW_MOUSE_BUTTON_LEFT)
		{
			if (getClickedButton() == ButtonType::BACKBUTTON)
			{
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
				removeInfoScreen();
				createStartScreen(LOGO_POSITION);
				GameState temp = current_state;
				current_state = previous_state;
				previous_state = temp;
			}
		}
		// gameover state -> start screen state
		else if (current_state == GameState::GAME_OVER && button == GLFW_MOUSE_BUTTON_LEFT) 
		{
			previous_state = current_state;
			current_state = GameState::START_SCREEN_ANIMATION;

			removeGameOverScreen();
			restart_game();
		}
	}
}

bool WorldSystem::isButtonClicked(screenButton &button)
{
	float button_x = button.center[0];
	float button_y = button.center[1];

	float x_distance = std::abs(button_x - device_mouse_pos_x);
	float y_distance = std::abs(button_y - device_mouse_pos_y);

	bool res = (x_distance < button.w / 2.f) && (y_distance < button.h / 2.f);

	// std::cout << device_mouse_pos_x << " " << device_mouse_pos_y << std::endl;
	// std::cout << game_mouse_pos_x << " " << game_mouse_pos_y << std::endl;
	// std::cout << button_x << " " << button_y << std::endl;
	// std::cout << "button: " << res << std::endl;

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

	switch (buff.type)
	{
	case 0: // Tail
		player.speed *= 1.05f;
		std::cout << "Collected Tail: Player Speed increased by 5%" << std::endl;
		break;

	case 1: // Mitochondria
		player.dash_cooldown_ms *= 0.95f;
		std::cout << "Collected Mitochondria: Dash cooldown decreased by 5%" << std::endl;
		break;

	case 2: // Hemoglobin
		player.detection_range *= 0.95f;
		std::cout << "Collected Hemoglobin: Enemies Detection range decreased by 5%" << std::endl;
		break;

	case 3: // Golgi Apparatus Buff (need to be implemented)
		
		std::cout << "Collected Golgi Body: need to be implemented" << std::endl;
		break;

	case 4: // Chloroplast
		player.healing_rate += 0.05;
	std::cout << "Collected Chloroplast: Healing increased by 5% " << std::endl;
		break;
	default:
		std::cerr << "Unknown buff type: " << buff.type << std::endl;
		break;
	}

	buff.collected = true;
	renderCollectedBuff(renderer, buff.type);
	registry.remove_all_components_of(buff_entity);

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
				
				// print here
				// std::cout << "x: " << x << " y: " << y << std::endl;
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

	initializedMap=true;
}
