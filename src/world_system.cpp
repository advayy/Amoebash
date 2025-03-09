// Header
#include "world_system.hpp"
#include "world_init.hpp"
#include "common.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
// include lerp
#include <glm/gtx/compatibility.hpp>

#include "physics_system.hpp"

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
	// Mix_PlayMusic(background_music, -1);

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
    camera.position = lerp(camera.position, player_motion.position, interpolationFactor);
	camera.grid_position = positionToGridCell(camera.position);
}



void WorldSystem::updateHuds()
{

	vec2 offset = {WINDOW_WIDTH_PX / 2 - 100, -WINDOW_HEIGHT_PX / 2 + 100};

	Entity minimapEntity = registry.miniMaps.entities[0];
	Motion &minimapMotion = registry.motions.get(minimapEntity);

	Camera &camera = registry.cameras.get(registry.cameras.entities[0]);
	minimapMotion.position = {camera.position.x + offset.x, camera.position.y + offset.y};

	if (!registry.uiElements.entities.empty())
	{
		for (Entity entity : registry.uiElements.entities)
		{
			if (!registry.motions.has(entity))
				continue;
			Motion &uiMotion = registry.motions.get(entity);
			UIElement &uiElement = registry.uiElements.get(entity);
			uiMotion.position = {camera.position.x + uiElement.position.x, camera.position.y + uiElement.position.y};
		}
	}

	if (!registry.healthBars.entities.empty())
	{
		HealthBar &healthBar = registry.healthBars.get(registry.healthBars.entities[0]);
		Motion &healthBarMotion = registry.motions.get(registry.healthBars.entities[0]);
		healthBarMotion.position = {camera.position.x + HEALTH_BAR_POS.x,
									camera.position.y + HEALTH_BAR_POS.y};

	}

	if (registry.dashRecharges.size() > 0)
	{
		Player &player = registry.players.get(registry.players.entities[0]);
		vec2 firstDotPosition = {camera.position.x + DASH_RECHARGE_START_POS.x, camera.position.y + DASH_RECHARGE_START_POS.y};

		int i = 0;
		for (Entity entity : registry.dashRecharges.entities)
		{
			if (!registry.motions.has(entity))
				continue;

			Motion &motion = registry.motions.get(entity);
			motion.position = {firstDotPosition.x + (i * DASH_RECHARGE_SPACING), firstDotPosition.y};

			if (i >= player.dash_count)
				motion.scale = {0, 0};
			else
				motion.scale = {DASH_WIDTH, DASH_HEIGHT};
			i++;
		}
	}
}

void WorldSystem::updateMouseCoords()
{
	Camera &camera = registry.cameras.get(registry.cameras.entities[0]);
	game_mouse_pos_x = device_mouse_pos_x + camera.position.x - WINDOW_WIDTH_PX * 0.5f;
	game_mouse_pos_y = device_mouse_pos_y + camera.position.y - WINDOW_HEIGHT_PX * 0.5f;
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update)
{

	// M1 Feature - Camera controls
	updateCamera(elapsed_ms_since_last_update);

	if (tutorial_mode && registry.infoBoxes.size() == 0) {
		createInfoBoxes();
	}

	updateMouseCoords();
	updateHuds();

	// Updating window title with points - disabled for now
	// std::stringstream title_ss;
	// title_ss << "Points: " << points;
	// glfwSetWindowTitle(window, title_ss.str().c_str());

	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Point the player to the mouse
	Motion &player_motion = registry.motions.get(registry.players.entities[0]);
	player_motion.angle = atan2(game_mouse_pos_y - player_motion.position.y, game_mouse_pos_x - player_motion.position.x) * 180.0f / M_PI + 90.0f;


	if (!tutorial_mode)
	{
		// spawn new invaders
		next_enemy_spawn -= elapsed_ms_since_last_update * current_speed;
		if (next_enemy_spawn < 0.f && !gameOver)
		{
			if (registry.enemies.entities.size() < MAX_ENEMIES_COUNT)
			{
				// reset timer
				next_enemy_spawn = (ENEMY_SPAWN_RATE_MS / 2) + uniform_dist(rng) * (ENEMY_SPAWN_RATE_MS / 2);
	
				// randomize position
				int map_w = MAP_RIGHT - MAP_LEFT;
				int map_h = MAP_BOTTOM - MAP_TOP;
				int randomXCell = MAP_LEFT + (int)(uniform_dist(rng) * map_w);
				int randomYCell = MAP_TOP + (int)(uniform_dist(rng) * map_h);
				vec2 enemyPosition = gridCellToPosition({(float)randomXCell, (float)randomYCell});
	
				createEnemy(renderer, enemyPosition);
	
				// Optional debug output for spawning enemies
				// std::cout << "TOTAL ENEMIES: " << registry.enemies.entities.size() << std::endl;
			}
		}
	}

    tileProceduralMap();

    // check if player in portal tile
    if (registry.portals.entities.size() > 0 && registry.motions.has(registry.portals.entities.back())) {
        Motion &portal_motion = registry.motions.get(registry.portals.entities.back());
        vec2 portal_position = portal_motion.position;

        float distance = sqrt(pow(player_motion.position.x - portal_position.x, 2) + pow(player_motion.position.y - portal_position.y, 2));
        float portal_radius = TILE_SIZE / 4.0f;

        if (distance < portal_radius) {
            // go to black screen
            Entity screen_state_entity = renderer->get_screen_state_entity();
            ScreenState &screen = registry.screenStates.get(screen_state_entity);
            screen.darken_screen_factor = 1;
            darken_screen_timer = 0.0f;

            current_state = GameState::NEXT_LEVEL;
			if (tutorial_mode) {
				tutorial_mode = false;
				removeInfoBoxes();
			}
            restart_game();
            removeStartScreen(); // removing buttons that are added again

        }
    }

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

	handlePlayerMovement(elapsed_ms_since_last_update);
	handlePlayerHealth(elapsed_ms_since_last_update);

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

	if(registry.dashes.size() == 0) {
		Motion &player_motion = registry.motions.get(registry.players.entities[0]);

		vec2 direction = vec2(game_mouse_pos_x, game_mouse_pos_y) - player_motion.position;
		direction = normalize(direction);

		// If the mouse is outside the deadzone, move the player
		if(length(vec2(game_mouse_pos_x, game_mouse_pos_y) - player_motion.position) > MOUSE_TRACKING_DEADZONE) {
			player_motion.velocity = {direction.x * player.speed, direction.y * player.speed};
		} else {
			player_motion.velocity = {0, 0};
		}
	}

	// step the particle system only when its needed
	// for optimaztion, we could only step the particles that are on screen
	particle_system.step(elapsed_ms_since_last_update);

	return true;

}

// Reset the world state to its initial state
void WorldSystem::restart_game()
{

	std::cout << "Restarting..." << std::endl;
    std::cout << "Level: " << level + 1 << std::endl;
    
	// Debugging for memory/component leaks
	registry.list_all_components();
    
	// Reset the game speed
	current_speed = 1.f;
    
	level += 1;
	next_enemy_spawn = 0;
	enemy_spawn_rate_ms = ENEMY_SPAWN_RATE_MS;
    
	// FLAG
	gameOver = false;

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
    
	std::cout << "Creating Procedural Map, tutorial mode status :" << tutorial_mode << std::endl;

    std::pair<int, int> playerPosition;
	createProceduralMap(renderer, vec2(MAP_WIDTH, MAP_HEIGHT), tutorial_mode, playerPosition);

	if (tutorial_mode) {
		createPlayer(renderer, gridCellToPosition({0, 10}));
		createEnemy(renderer, gridCellToPosition({12, 10}));
	} else {
		createPlayer(renderer, gridCellToPosition(vec2(playerPosition.second, playerPosition.first)));
	}
	
	createMiniMap(renderer, vec2(MAP_WIDTH, MAP_HEIGHT));

	createCamera();

	// screens
	if (previous_state == GameState::GAME_OVER)
	{
		createStartScreen(WORLD_ORIGIN);
	}
	else
	{
		createStartScreen();
	}
	createShopScreen();
	createInfoScreen();

	// timer settings depending on previous state
	if (current_state == GameState::START_SCREEN_ANIMATION)
	{
		stateTimer = BOOT_CUTSCENE_DURATION_MS;
	}
	else
	{
		stateTimer = INTRO_CUTSCENE_DURATION_MS;
	}

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

// Compute collisions between entities
void WorldSystem::handle_collisions()
{

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: Loop over all collisions detected by the physics system
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	ComponentContainer<Collision> &collision_container = registry.collisions;
	for (uint i = 0; i < collision_container.components.size(); i++)
	{

		Collision &collision = collision_container.components[i];
		Entity &entity1 = collision_container.entities[i];
		Entity &entity2 = collision.other;

		// should be deprecated no?
		// if 1 is a projectile and 2 is an invader or if 1 is an invader and 2 is a projectile
		if ((registry.projectiles.has(entity1) && registry.enemies.has(entity2)) || (registry.projectiles.has(entity2) && registry.enemies.has(entity1)))
		{

			// Set invader and projectile
			Entity projectile_entity = registry.projectiles.has(entity1) ? entity1 : entity2;
			Entity enemy_entity = registry.enemies.has(entity1) ? entity1 : entity2;

			Enemy &enemy = registry.enemies.get(registry.enemies.has(entity1) ? entity1 : entity2);
			Projectile &projectile = registry.projectiles.get(registry.projectiles.has(entity1) ? entity1 : entity2);

			// Invader takes damage
			enemy.health -= projectile.damage;

			// remove projectile
			registry.remove_all_components_of(projectile_entity);

			// if invader health is below 0
			// remove invader and increase points
			// buff created
			if (enemy.health <= 0)
			{
				// Get the enemy position before removing it
				Motion& enemyMotion = registry.motions.get(enemy_entity);
				vec2 enemyPosition = enemyMotion.position;
				
				// Remove the enemy entity
				registry.remove_all_components_of(enemy_entity);
				points += 1;
				Mix_PlayChannel(-1, dash_sound_2, 0);
				
				// Create death particles at the enemy's position
				particle_system.createParticles(PARTICLE_TYPE::DEATH_PARTICLE, enemyPosition, 15);
        
				// level += 1;
				Mix_PlayChannel(-1, dash_sound_2, 0); // FLAG MORE SOUNDS

				createBuff(vec2(enemy_position.x + 60, enemy_position.y + 60));
			}
		}

		if (registry.players.has(entity1) && registry.buffs.has(entity2))
		{
			collectBuff(entity1, entity2);
		}
		else if (registry.players.has(entity2) && registry.buffs.has(entity1))
		{
			collectBuff(entity2, entity1);
		}

		// player-enemy collision
		if ((registry.players.has(entity1) && registry.enemies.has(entity2)) ||
			(registry.players.has(entity2) && registry.enemies.has(entity1)))
		{

			Entity player_entity = registry.players.has(entity1) ? entity1 : entity2;
			Entity enemy_entity = registry.enemies.has(entity1) ? entity1 : entity2;
			Enemy &enemy = registry.enemies.get(enemy_entity);

			/*
			if (isDashing())
			{
					// Get the enemy position before removing it
				Motion& enemyMotion = registry.motions.get(enemy_entity);
				vec2 enemyPosition = enemyMotion.position;
				
				// remove invader
				registry.remove_all_components_of(enemy_entity);
				Mix_PlayChannel(-1, dash_sound_2, 0);
				
				// Create death particles at the enemy's position
				particle_system.createParticles(PARTICLE_TYPE::DEATH_PARTICLE, enemyPosition, 15);
			}
			*/
			// Kill enemy when dashing?
			if (isDashing())
			{
				enemy.health -= PLAYER_DASH_DAMAGE;

				if (enemy.health <= 0)
				{
					vec2 enemy_position = registry.motions.get(enemy_entity).position;
					points += 1;
					registry.remove_all_components_of(enemy_entity);
					
					createBuff(vec2(enemy_position.x + 60, enemy_position.y + 60));
				}
			}
			else
			{
				// womp womp	game over or vignetted??
				uint current_time = SDL_GetTicks();

				// then apply damage.
				if (!registry.damageCooldowns.has(player_entity))
				{
					//  add the component and apply damage
					registry.damageCooldowns.insert(player_entity, {current_time});

					Player &player = registry.players.get(player_entity);
					player.current_health -= 1; // FLAG this is not the right kind of damage...
					Mix_PlayChannel(-1, dash_sound_2, 0);
				}
				else
				{
					// retrieve the cooldown component
					DamageCooldown &dc = registry.damageCooldowns.get(player_entity);
					if (current_time - dc.last_damage_time >= 500)
					{
						dc.last_damage_time = current_time;
						Player &player = registry.players.get(player_entity);
						player.current_health -= 1;
						Mix_PlayChannel(-1, dash_sound_2, 0);
					}
				}
			}
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
		current_state = GameState::START_SCREEN;

		Entity startScreen = registry.starts.entities[0];
		Motion &startScreenMotion = registry.motions.get(startScreen);
		startScreenMotion.velocity = {0.f, 0.f};
		startScreenMotion.position = {0.f, 0.f};
        level = 1;
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

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods)
{

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: Handle mouse clicking for invader and tower placement.
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// on button press
	if (action == GLFW_RELEASE && !gameOver)
	{

		vec2 tile = positionToGridCell(vec2(game_mouse_pos_x, game_mouse_pos_y));

		if (button == GLFW_MOUSE_BUTTON_LEFT && canDash() && current_state == GameState::GAME_PLAY)
		{
			initiatePlayerDash();
			Mix_PlayChannel(-1, dash_sound_1, 0);
		}
		else if (button == GLFW_MOUSE_BUTTON_LEFT && current_state == GameState::START_SCREEN)
		{

			screenButton *startButton = nullptr;

			for (auto &button : registry.buttons.components)
			{
				if (button.type == ButtonType::STARTBUTTON)
				{
					startButton = &button;
					break;
				}
			}

			if (!startButton)
			{
				return;
			}

			// Find shop button
			screenButton *shopButton = nullptr;
			for (auto &button : registry.buttons.components)
			{
				if (button.type == ButtonType::SHOPBUTTON)
				{
					shopButton = &button;
					break;
				}
			}

			if (!shopButton)
			{
				return;
			}

			screenButton *nucleusButton = nullptr;
			for (auto &button : registry.buttons.components)
			{
				if (button.type == ButtonType::INFOBUTTON)
				{
					nucleusButton = &button;
					break;
				}
			}

			if (!nucleusButton)
			{
				return;
			}

			bool on_button_shop = buttonClick(*shopButton);
			bool on_button_nucleus = buttonClick(*nucleusButton);
			bool on_button = buttonClick(*startButton);

			if (on_button_shop)
			{
				previous_state = current_state;
				current_state = GameState::SHOP;
			}
			else if (on_button_nucleus)
			{
				previous_state = current_state;
				current_state = GameState::INFO;
			}
			else if (on_button)
			{
				previous_state = current_state;
				current_state = GameState::GAMEPLAY_CUTSCENE;
				// need to add render request for cutscene animations
				removeStartScreen();
				createGameplayCutScene();
			}
		}

		// gameplay -> shop
		else if (button == GLFW_MOUSE_BUTTON_LEFT && current_state == GameState::GAME_PLAY)
		{

			// Find shop button
			screenButton *shopButton = nullptr;
			for (auto &button : registry.buttons.components)
			{
				if (button.type == ButtonType::SHOPBUTTON)
				{
					shopButton = &button;
					break;
				}
			}

			if (!shopButton)
			{
				return;
			}

			screenButton *nucleusButton = nullptr;
			for (auto &button : registry.buttons.components)
			{
				if (button.type == ButtonType::INFOBUTTON)
				{
					nucleusButton = &button;
					break;
				}
			}

			if (!nucleusButton)
			{
				return;
			}

			bool on_button_shop = buttonClick(*shopButton);
			bool on_button_nucleus = buttonClick(*nucleusButton);

			if (on_button_shop)
			{
				previous_state = current_state;
				current_state = GameState::SHOP;
			}
			else if (on_button_nucleus)
			{
				previous_state = current_state;
				current_state = GameState::INFO;
			}
		}

		else if (button == GLFW_MOUSE_BUTTON_LEFT && current_state == GameState::SHOP)
		{
			// Find shop button
			screenButton *shopButton = nullptr;
			for (auto &button : registry.buttons.components)
			{
				if (button.type == ButtonType::SHOPBUTTON)
				{
					shopButton = &button;
					break;
				}
			}

			if (!shopButton)
			{
				return;
			}

			bool on_button_shop = buttonClick(*shopButton);
			if (on_button_shop)
			{
				GameState temp = current_state;
				current_state = previous_state;
				previous_state = temp;
			}
		}
		else if (button == GLFW_MOUSE_BUTTON_LEFT && current_state == GameState::INFO)
		{
			screenButton *nucleusButton = nullptr;
			for (auto &button : registry.buttons.components)
			{
				if (button.type == ButtonType::INFOBUTTON)
				{
					nucleusButton = &button;
					break;
				}
			}

			if (!nucleusButton)
			{
				return;
			}

			bool on_button_shop = buttonClick(*nucleusButton);
			if (on_button_shop)
			{
				GameState temp = current_state;
				current_state = previous_state;
				previous_state = temp;
			}
		}

		// gameover state -> start screen state
		else if (button == GLFW_MOUSE_BUTTON_LEFT && current_state == GameState::GAME_OVER)
		{
			previous_state = current_state;
			current_state = GameState::START_SCREEN;

			removeGameOverScreen();
			restart_game();
		}
	}
}

bool WorldSystem::buttonClick(screenButton &button)
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
	buff.collected = true;

	registry.remove_all_components_of(buff_entity);

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

	renderCollectedBuff(renderer, buff.type);
}
