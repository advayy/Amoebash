// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
// include lerp
#include <glm/gtx/compatibility.hpp>

#include "physics_system.hpp"

// create the world
WorldSystem::WorldSystem() :
	points(0),
	next_enemy_spawn(0),
	enemy_spawn_rate_ms(ENEMY_SPAWN_RATE_MS)
{
	// seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
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

// Debugging
namespace {
	void glfw_err_cb(int error, const char *desc) {
		std::cerr << error << ": " << desc << std::endl;
	}
}

// call to close the window, wrapper around GLFW commands
void WorldSystem::close_window() {
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {

	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
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
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_FALSE);		// GLFW 3.3+

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX, "Towers vs Invaders Assignment", nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "ERROR: Failed to glfwCreateWindow in world_system.cpp" << std::endl;
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto mouse_button_pressed_redirect = [](GLFWwindow* wnd, int _button, int _action, int _mods) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_button_pressed(_button, _action, _mods); };
	
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_pressed_redirect);

	return window;
}

bool WorldSystem::start_and_load_sounds() {
	
	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return false;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return false;
	}

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	dash_sound_1 = Mix_LoadWAV(audio_path("dash_1.wav").c_str());
	dash_sound_2 = Mix_LoadWAV(audio_path("dash_2.wav").c_str());

	if (background_music == nullptr || dash_sound_1 == nullptr || dash_sound_2 == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music.wav").c_str(),
			audio_path("dash_1.wav").c_str(),
			audio_path("dash_2.wav").c_str());
		return false;
	}

	return true;
}

void WorldSystem::init(RenderSystem* renderer_arg) {

	this->renderer = renderer_arg;

	// start playing background music indefinitely

	// std::cout << "Starting music..." << std::endl;
	// Mix_PlayMusic(background_music, -1);

	

	// Set all states to default
    restart_game();
	// set initial game state
	current_state = GameState::START_SCREEN_ANIMATION;
}

void WorldSystem::updateCamera(float elapsed_ms) {
	// Get camera entity
	Entity cameraEntity = registry.cameras.entities[0];
    Camera& camera = registry.cameras.get(cameraEntity);

    Motion& player_motion = registry.motions.get(registry.players.entities[0]);

    // Initialize camera position if not already initialized
    if (!camera.initialized) {
        camera.position = player_motion.position; // Snap to player
        camera.initialized = true; // Mark as initialized
        return; // Skip interpolation for this frame
    }

    // Define deadzone size (75% of the screen width and height)
    float deadzoneWidth = WINDOW_WIDTH_PX * DEADZONE_FACTOR.x;
    float deadzoneHeight = WINDOW_HEIGHT_PX * DEADZONE_FACTOR.y;

    // Calculate deadzone boundaries relative to the camera's position
    float deadzoneLeft = camera.position.x - deadzoneWidth / 2 - 10.0f; // Buffer of 10 pixels
    float deadzoneRight = camera.position.x + deadzoneWidth / 2 + 10.0f;
    float deadzoneTop = camera.position.y - deadzoneHeight / 2 - 10.0f;
    float deadzoneBottom = camera.position.y + deadzoneHeight / 2 + 10.0f;

    // Check if the player is outside the deadzone
    bool playerOutsideDeadzone =
        player_motion.position.x < deadzoneLeft ||
        player_motion.position.x > deadzoneRight ||
        player_motion.position.y < deadzoneTop ||
        player_motion.position.y > deadzoneBottom;

    // Only move the camera if the player is outside the deadzone
    if (playerOutsideDeadzone) {
        // Define interpolation factor (0 = no movement, 1 = instant snap)
        float interpolationFactor = 0.1f; // Lower value for smoother movement

        // Convert elapsed_ms to seconds for smoother interpolation
        float deltaTime = elapsed_ms / 1000.0f;

        // Calculate interpolated camera position
        vec2 targetPosition = player_motion.position; // Target is the player's position
		// M1 interpolation implementation
        camera.position = lerp(camera.position, targetPosition, interpolationFactor * deltaTime);

        // Optional: Clamp camera speed
        vec2 cameraMovement = targetPosition - camera.position;
        float maxSpeed = 500.0f * deltaTime; // Adjust max speed as needed
        if (length(cameraMovement) > maxSpeed) {
            cameraMovement = normalize(cameraMovement) * maxSpeed;
        }
        camera.position += cameraMovement;
    }

	// Update camera grid position
	camera.grid_position = positionToGridCell(camera.position);
}

void WorldSystem::updateHuds() {

	vec2 offset = {WINDOW_WIDTH_PX/2 - 100, -WINDOW_HEIGHT_PX/2 + 100};
	
	Entity minimapEntity = registry.miniMaps.entities[0];
	Motion& minimapMotion = registry.motions.get(minimapEntity);

	Camera& camera = registry.cameras.get(registry.cameras.entities[0]);
	minimapMotion.position = { camera.position.x + offset.x, camera.position.y + offset.y};
}

void WorldSystem::updateMouseCoords() {
    Camera& camera = registry.cameras.get(registry.cameras.entities[0]);
    game_mouse_pos_x = device_mouse_pos_x + camera.position.x - WINDOW_WIDTH_PX * 0.5f;
    game_mouse_pos_y = device_mouse_pos_y + camera.position.y - WINDOW_HEIGHT_PX * 0.5f;
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {

	// M1 Feature - Camera controls
    updateCamera(elapsed_ms_since_last_update);
    updateMouseCoords();
	updateHuds();

	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Point the player to the mouse
	Motion& player_motion = registry.motions.get(registry.players.entities[0]);
	player_motion.angle = atan2(game_mouse_pos_y - player_motion.position.y, game_mouse_pos_x - player_motion.position.x)  * 180.0f / M_PI + 90.0f;


	// spawn new invaders
	next_enemy_spawn -= elapsed_ms_since_last_update * current_speed;
	if (next_enemy_spawn < 0.f && !gameOver) {
		if (registry.enemies.entities.size() < MAX_ENEMIES_COUNT) {
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


	tileMap();

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {

	std::cout << "Restarting..." << std::endl;

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Reset the game speed
	current_speed = 1.f;

	points = 0;
	next_enemy_spawn = 0;
	enemy_spawn_rate_ms = ENEMY_SPAWN_RATE_MS;

	//FLAG
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
	
	while (registry.buttons.entities.size() > 0 )
		registry.remove_all_components_of(registry.buttons.entities.back());

	// debugging for memory/component leaks
	registry.list_all_components();

	createPlayer(renderer, gridCellToPosition(WORLD_ORIGIN));
	createMap(renderer, vec2(MAP_WIDTH, MAP_HEIGHT));
	createMiniMap(renderer, vec2(MAP_WIDTH, MAP_HEIGHT));

    createCamera();

	// screens
	if (previous_state == GameState::GAME_OVER) {
		createStartScreen(WORLD_ORIGIN);
	} else {
		createStartScreen();
	}
	createShopScreen();
	createInfoScreen();

	// timer settings depending on previous state
	if (current_state == GameState::START_SCREEN_ANIMATION) {
		stateTimer = BOOT_CUTSCENE_DURATION_MS;
	} else {
		stateTimer = INTRO_CUTSCENE_DURATION_MS;
	}
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: Loop over all collisions detected by the physics system
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	ComponentContainer<Collision>& collision_container = registry.collisions;
	for (uint i = 0; i < collision_container.components.size(); i++) {
		
		Collision& collision = collision_container.components[i];
		Entity& entity1 = collision_container.entities[i];
		Entity& entity2 = collision.other;

		// should be deprecated no?
		// if 1 is a projectile and 2 is an invader or if 1 is an invader and 2 is a projectile
		if ((registry.projectiles.has(entity1) && registry.enemies.has(entity2)) || (registry.projectiles.has(entity2) && registry.enemies.has(entity1))) {
			
			// Set invader and projectile
			Entity projectile_entity = registry.projectiles.has(entity1) ? entity1 : entity2;
			Entity enemy_entity = registry.enemies.has(entity1) ? entity1 : entity2;

			Enemy& enemy = registry.enemies.get(registry.enemies.has(entity1) ? entity1 : entity2);
			Projectile& projectile = registry.projectiles.get(registry.projectiles.has(entity1) ? entity1 : entity2);
			
			// Invader takes damage
			enemy.health -= projectile.damage;

			// remove projectile
			registry.remove_all_components_of(projectile_entity);

			// if invader health is below 0, remove invader and increase points
			if(enemy.health <= 0){
				registry.remove_all_components_of(enemy_entity);
				points += 1;
 				Mix_PlayChannel(-1, dash_sound_2, 0); // FLAG MORE SOUNDS 
			}
		}

		// player-enemy collision
		if ((registry.players.has(entity1) && registry.enemies.has(entity2)) ||
			(registry.players.has(entity2) && registry.enemies.has(entity1))) {
			
			Entity player_entity = registry.players.has(entity1) ? entity1 : entity2;
			Entity enemy_entity = registry.enemies.has(entity1) ? entity1 : entity2;



			if (isDashing()) {
				// remove invader
				registry.remove_all_components_of(enemy_entity);
				Mix_PlayChannel(-1, dash_sound_2, 0);
			} else {
				// womp womp	game over or vignetted??
				uint current_time = SDL_GetTicks();
			
				// then apply damage.
				if (!registry.damageCooldowns.has(player_entity)) {
					//  add the component and apply damage
					registry.damageCooldowns.insert(player_entity, { current_time });
				
					Player& player = registry.players.get(player_entity);
					player.health -= 1;
					Mix_PlayChannel(-1, dash_sound_2, 0);
				} else {
					// retrieve the cooldown component
					DamageCooldown& dc = registry.damageCooldowns.get(player_entity);
					if (current_time - dc.last_damage_time >= 500) {
						dc.last_damage_time = current_time;
						Player& player = registry.players.get(player_entity);
						player.health -= 1;
						Mix_PlayChannel(-1, dash_sound_2, 0);
					}
				}
				// registry.remove_all_components_of(player_entity);
				// registry.vignetteTimers.insert(Entity(), { 1000.f });

				// Mix_PlayChannel(-1, dash_sound_2, 0);
			}

			// We dont do this anymore FLAG
			// registry.remove_all_components_of(enemy_entity);
			// registry.remove_all_components_of(player_entity);
			// Mix_PlayChannel(-1, dash_sound_2, 0);


			// Trigger vignette shader
			// ensure it stays dark for a second before returning to normal
			// registry.vignetteTimers.insert(Entity(), { 1000.f });

			// ScreenState &screen = registry.screenStates.components[1];
			// screen.darken_screen_factor = 1;
			// registry.deathTimers.insert(tower_entity, { 1000.f });
		}
	}
	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// on key callback
void WorldSystem::on_key(int key, int, int action, int mod) {

	// exit game w/ ESC
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		close_window();
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

        restart_game();

		previous_state = current_state;
		current_state = GameState::START_SCREEN;

		Entity startScreen = registry.starts.entities[0];
		Motion& startScreenMotion = registry.motions.get(startScreen);
		startScreenMotion.velocity = {0.f, 0.f};
		startScreenMotion.position = {0.f, 0.f};
	}

	// Pausing Game
	if (action == GLFW_RELEASE && key == GLFW_KEY_SPACE) {
		if (current_state == GameState::GAME_PLAY) {
			current_state = GameState::PAUSE;

			// renderer.
			createPauseScreen();

		} else if (current_state == GameState::PAUSE) {
			current_state = GameState::GAME_PLAY;
			removePauseScreen();
		}
	}

	// Debugging - not used in A1, but left intact for the debug lines
	if (key == GLFW_KEY_D) {
		if (action == GLFW_RELEASE) {
			if (debugging.in_debug_mode) {
				debugging.in_debug_mode = false;
			}
			else {
				debugging.in_debug_mode = true;
			}
		}
	}

	// using O key for gameover, for now
	if (key == GLFW_KEY_O) {
		if (action == GLFW_RELEASE) {
			if (current_state == GameState::GAME_PLAY) {
				previous_state = GameState::GAME_PLAY;
				current_state = GameState::GAME_OVER;
				createGameOverScreen();
			}
		}
	}

	// Q for going to start screen
	if (key == GLFW_KEY_Q) {
		if (action == GLFW_RELEASE) {
			restart_game();
			current_state = GameState::START_SCREEN;
		}
	}
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {

	// record the current mouse position
	device_mouse_pos_x = mouse_position.x;
	device_mouse_pos_y = mouse_position.y;
}

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods) {

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: Handle mouse clicking for invader and tower placement.
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// on button press
	if (action == GLFW_RELEASE && !gameOver) {

		vec2 tile = positionToGridCell(vec2(game_mouse_pos_x, game_mouse_pos_y));

				
		if(button == GLFW_MOUSE_BUTTON_LEFT && canDash() && current_state == GameState::GAME_PLAY)
		{
			initiatePlayerDash();
			Mix_PlayChannel(-1, dash_sound_1, 0);
		}
		else if (button == GLFW_MOUSE_BUTTON_LEFT && current_state == GameState::START_SCREEN) 
		{
			
			screenButton* startButton = nullptr; 

			for (auto& button : registry.buttons.components) {
				if (button.type == ButtonType::STARTBUTTON) {
					startButton = &button;  
					break;
				}
			} 

			if (!startButton) {
				return;
			} 

			// Find shop button
			screenButton* shopButton = nullptr; 
			for (auto& button : registry.buttons.components) {
				if (button.type == ButtonType::SHOPBUTTON) {
					shopButton = &button; 
					break;  
				}
			}

			if (!shopButton) {
				return;
			} 

			screenButton* nucleusButton = nullptr; 
			for (auto& button : registry.buttons.components) {
				if (button.type == ButtonType::INFOBUTTON) {
					nucleusButton = &button; 
					break;  
				}
			}

			if (!nucleusButton) {
				return;
			} 


			bool on_button_shop = buttonClick(*shopButton);
			bool on_button_nucleus = buttonClick(*nucleusButton);
			bool on_button = buttonClick(*startButton);
			





			if (on_button_shop) {
				previous_state = current_state;
				current_state = GameState::SHOP;

			} else if (on_button_nucleus) {
				previous_state = current_state;
				current_state = GameState::INFO;

			} else if (on_button) {
				previous_state = current_state;
				current_state = GameState::GAMEPLAY_CUTSCENE;
				// need to add render request for cutscene animations
				removeStartScreen();
				createGameplayCutScene();
			}
		}

		// gameplay -> shop
		else if(button == GLFW_MOUSE_BUTTON_LEFT && current_state == GameState::GAME_PLAY) {
			
			// Find shop button
			screenButton* shopButton = nullptr; 
			for (auto& button : registry.buttons.components) {
				if (button.type == ButtonType::SHOPBUTTON) {
					shopButton = &button; 
					break;  
				}
			}

			if (!shopButton) {
				return;
			} 

			screenButton* nucleusButton = nullptr; 
			for (auto& button : registry.buttons.components) {
				if (button.type == ButtonType::INFOBUTTON) {
					nucleusButton = &button; 
					break;  
				}
			}

			if (!nucleusButton) {
				return;
			} 


			bool on_button_shop = buttonClick(*shopButton);
			bool on_button_nucleus = buttonClick(*nucleusButton);
			

			if (on_button_shop) {
				previous_state = current_state;
				current_state = GameState::SHOP;
			} else if (on_button_nucleus) {
				previous_state = current_state;
				current_state = GameState::INFO;
			}
		}

		else if (button == GLFW_MOUSE_BUTTON_LEFT && current_state == GameState::SHOP) {
			// Find shop button
			screenButton* shopButton = nullptr; 
			for (auto& button : registry.buttons.components) {
				if (button.type == ButtonType::SHOPBUTTON) {
					shopButton = &button; 
					break;  
				}
			}

			if (!shopButton) {
				return;
			} 

			bool on_button_shop = buttonClick(*shopButton);
			if (on_button_shop) {
				GameState temp = current_state;
				current_state = previous_state;
				previous_state = temp;
			}

		} 
		else if (button == GLFW_MOUSE_BUTTON_LEFT && current_state == GameState::INFO) {
			screenButton* nucleusButton = nullptr; 
			for (auto& button : registry.buttons.components) {
				if (button.type == ButtonType::INFOBUTTON) {
					nucleusButton = &button; 
					break;  
				}
			}

			if (!nucleusButton) {
				return;
			} 

			bool on_button_shop = buttonClick(*nucleusButton);
			if (on_button_shop) {
				GameState temp = current_state;
				current_state = previous_state;
				previous_state = temp;
			}
		}

		// gameover state -> start screen state
		else if(button == GLFW_MOUSE_BUTTON_LEFT && current_state == GameState::GAME_OVER) {
			previous_state = current_state;
			current_state = GameState::START_SCREEN;

			removeGameOverScreen();
			restart_game();
		}
	}
}

bool WorldSystem::buttonClick(screenButton& button) {
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
