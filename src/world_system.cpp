// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>

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
	std::cout << "Starting music..." << std::endl;
	Mix_PlayMusic(background_music, -1);

	// Set all states to default
    restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {

	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());


	// Point the player to the mouse
	Motion& player_motion = registry.motions.get(registry.players.entities[0]);
	player_motion.angle = atan2(mouse_pos_y - player_motion.position.y, mouse_pos_x- player_motion.position.x)  * 180.0f / M_PI + 90.0f;


	// Removing out of screen entities
	auto& motions_registry = registry.motions;

 	tileMap();


    // FLAG THIS WILL BREAK NOW
	
	
	// Remove entities that leave the screen on the left side // REMOVE PROJECTILES
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motions_registry.components.size()-1; i>=0; --i) {
	    Motion& motion = motions_registry.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f) {
			if(!registry.players.has(motions_registry.entities[i])) // don't remove the player
				registry.remove_all_components_of(motions_registry.entities[i]);
		}
	}

	// REMOVE INVADERS that leave screen on rightside and end the game // FLAG wont be required later imo...
	for (int i = (int)motions_registry.components.size()-1; i>=0; --i) {
	    Motion& motion = motions_registry.components[i];
		if (motion.position.x > WINDOW_WIDTH_PX) {
			if(registry.enemies.has(motions_registry.entities[i])) {
				registry.remove_all_components_of(motions_registry.entities[i]);
				
				
				// gameOver = true; // FLAG REMOVING CAUSE NOT NEEDED ATM
				// registry.deathTimers.insert(Entity(), { 1000.f });
				
				// //Set velocity for all entities to zero -> doesnt stop all projectiles that are gonna get created
				// for (int i = 0; i < motions_registry.components.size(); i++) {
				// 	motions_registry.components[i].velocity = {0, 0};
				// }
				
				// std::cout << "X---------------------------Game Over---------------------------X" << std::endl;
			}
		}
	}


	// spawn new invaders
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: limit them to cells on the far-left, except (0, 0)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	next_enemy_spawn -= elapsed_ms_since_last_update * current_speed;
	if (next_enemy_spawn < 0.f && !gameOver) {
		// reset timer
		next_enemy_spawn = (ENEMY_SPAWN_RATE_MS / 2) + uniform_dist(rng) * (ENEMY_SPAWN_RATE_MS / 2);

		// create invader with random initial position
		// FLAG NOT HOW SPAWNS SHOULD WORK

		float x = GRID_CELL_WIDTH_PX/2; // middle of a first cell
		float y = GRID_CELL_HEIGHT_PX/2 + (GRID_CELL_HEIGHT_PX * ((int) (uniform_dist(rng) * (WINDOW_HEIGHT_PX))/GRID_CELL_HEIGHT_PX));
		
		createEnemy(renderer, vec2(x, y)); // flag position wrong
	}
 	
	if(!gameOver) {
		animation(elapsed_ms_since_last_update);
	}

		

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: game over fade out // flag
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	assert(registry.screenStates.components.size() <= 1);
    ScreenState &screen = registry.screenStates.components[0];
	
	float v_counter_ms = 1000.f;
	// Vignette section
	for (Entity entity : registry.vignetteTimers.entities) {
		VignetteTimer& counter = registry.vignetteTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		
		if(counter.counter_ms < v_counter_ms){
			v_counter_ms = counter.counter_ms;
			if(counter.counter_ms <= 0){
				registry.vignetteTimers.remove(entity);
			}
		}
		screen.vignette_screen_factor = v_counter_ms / 1000;
	}

    float min_counter_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities) {

		// progress timer
		DeathTimer& counter = registry.deathTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if(counter.counter_ms < min_counter_ms){
		    min_counter_ms = counter.counter_ms;
		}
	}
	// reduce window brightness if any of the present chickens is dying
	screen.darken_screen_factor = 1 - min_counter_ms / 3000;

	// RESET DOESNT WORK
				// registry.vignetteTimers.remove(entity);

	// if(!gameOver && screen.vignette_screen_factor == 0){
	// 	if(registry.vignetteTimers.size() < 1) {
	// 		screen.vignette_screen_factor = -1; // reset
	// 	}
	// }

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

	// debugging for memory/component leaks
	registry.list_all_components();

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: create grid lines
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	int grid_line_width = GRID_LINE_WIDTH_PX;

	// create grid lines if they do not already exist
	if (grid_lines.size() == 0) {
		// vertical lines
		int cell_width = GRID_CELL_WIDTH_PX;
		for (int col = 0; col < WINDOW_WIDTH_PX/GRID_CELL_WIDTH_PX; col++) {
			// width of 2 to make the grid easier to see
			grid_lines.push_back(createGridLine(vec2(col * cell_width, 0), vec2(grid_line_width, 2 * WINDOW_HEIGHT_PX)));
		}

		// horizontal lines
		int cell_height = GRID_CELL_HEIGHT_PX;
		for (int col = 0; col < (WINDOW_HEIGHT_PX/GRID_CELL_HEIGHT_PX) +1; col++) { // FLAG KNOWN BUG HERE?
			// width of 2 to make the grid easier to see
			grid_lines.push_back(createGridLine(vec2(0, col * cell_height), vec2(2 * WINDOW_WIDTH_PX, grid_line_width)));
		}
	}

	createPlayer(renderer, gridCellToPosition(WORLD_ORIGIN));
	createMap(renderer, vec2(20, 20));
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

		if ((registry.players.has(entity1) && registry.enemies.has(entity2)) || (registry.players.has(entity2) && registry.enemies.has(entity1))) {
			
			// Set invader and projectile
			Entity player_entity = registry.players.has(entity1) ? entity1 : entity2;
			Entity enemy_entity = registry.enemies.has(entity1) ? entity1 : entity2;

			// We dont do this anymore FLAG
			// registry.remove_all_components_of(enemy_entity);
			// registry.remove_all_components_of(player_entity);
			Mix_PlayChannel(-1, dash_sound_2, 0);


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
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {

	// record the current mouse position
	mouse_pos_x = mouse_position.x;
	mouse_pos_y = mouse_position.y;
}

void WorldSystem::on_mouse_button_pressed(int button, int action, int mods) {

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: Handle mouse clicking for invader and tower placement.
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// on button press
	if (action == GLFW_PRESS && !gameOver) {

		vec2 tile = positionToGridCell(vec2(mouse_pos_x, mouse_pos_y));

		std::cout << "mouse position: " << mouse_pos_x << ", " << mouse_pos_y << std::endl;
		std::cout << "mouse tile position: " << tile.x << ", " << tile.y << std::endl;

		
		// CONTROLS


		if(button == GLFW_MOUSE_BUTTON_LEFT && canDash()){
			InitiatePlayerDash();
			Mix_PlayChannel(-1, dash_sound_1, 0);
		}
	}
}