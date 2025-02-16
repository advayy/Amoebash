
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stdlib
#include <chrono>
#include <iostream>

// internal
#include "ai_system.hpp"
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "world_init.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	// global systems
	AISystem	  ai_system;
	WorldSystem   world_system;
	RenderSystem  renderer_system;
	PhysicsSystem physics_system;

	// initialize window
	GLFWwindow* window = world_system.create_window();
	if (!window) {
		// Time to read the error message
		std::cerr << "ERROR: Failed to create window.  Press any key to exit" << std::endl;
		getchar();
		return EXIT_FAILURE;
	}

	if (!world_system.start_and_load_sounds()) {
		std::cerr << "ERROR: Failed to start or load sounds." << std::endl;
	}

	// initialize the main systems
	renderer_system.init(window);
	world_system.init(&renderer_system);

	GameState& current_state = world_system.current_state;
	GameState& previous_state = world_system.previous_state;
	

	// variable timestep loop
	auto t = Clock::now();
	float stateTimer = 5000.f;
	
	while (!world_system.is_over()) {
		
		// processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// calculate elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;



		switch (current_state) {

			case GameState::INITIAL_CUTSCENE :
				stateTimer -= elapsed_ms;

				if (stateTimer <= 0.f) {
					current_state = GameState::START_SCREEN;
					stateTimer = 15000.f;
				}
				break;

			case GameState::START_SCREEN :
				renderer_system.drawStartScreen();
				break;


			case GameState::GAME_PLAY :
				// CK: be mindful of the order of your systems and rearrange this list only if necessary
				world_system.step(elapsed_ms);
				ai_system.step(elapsed_ms);
				physics_system.step(elapsed_ms);
				world_system.handle_collisions();

				renderer_system.draw();
				break;
			
			case GameState::PAUSE :
				renderer_system.draw();
				break;
			
			case GameState::SHOP :
				renderer_system.drawShopScreen();
				break;

			case GameState::INFO :
				// renderer_system.draw();
				renderer_system.drawInfoScreen();
				break;
			case GameState::GAME_OVER :
				renderer_system.drawGameOverScreen();
				break;
			
			case GameState::GAMEPLAY_CUTSCENE :
				stateTimer -= elapsed_ms;

				if (stateTimer <= 0.f) {
					stateTimer = 15000.f;

					current_state = previous_state;
					previous_state = GameState::GAMEPLAY_CUTSCENE;
				}
				break;

			default :
				break;
		}

		
	}

	return EXIT_SUCCESS;
}
