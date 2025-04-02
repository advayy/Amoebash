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
#include "animation_system.hpp"
#include "world_init.hpp"
#include "particle_system.hpp"
#include "ui_system.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	// global systems
	AISystem ai_system;
	AnimationSystem animation_system;
	WorldSystem world_system;
	RenderSystem renderer_system;
	PhysicsSystem physics_system;
    ParticleSystem particle_system;

	// initialize window
	GLFWwindow *window = world_system.create_window();
	if (!window)
	{
		// Time to read the error message
		std::cerr << "ERROR: Failed to create window.  Press any key to exit" << std::endl;
		getchar();
		return EXIT_FAILURE;
	}

	if (!world_system.start_and_load_sounds())
	{
		std::cerr << "ERROR: Failed to start or load sounds." << std::endl;
	}

	// initialize the main systems
	renderer_system.init(window);
	world_system.init(&renderer_system);

	GameState &current_state = world_system.current_state;
	GameState &previous_state = world_system.previous_state;

	// variable timestep loop
	auto t = Clock::now();

	float &stateTimer = world_system.stateTimer;

	while (!world_system.is_over())
	{

		// processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// calculate elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		// Update FPS counter
		renderer_system.updateFPS(elapsed_ms);

		switch (current_state)
		{

		case GameState::START_SCREEN_ANIMATION:
			stateTimer -= elapsed_ms;
			renderer_system.drawStartScreen();
			physics_system.step(elapsed_ms);
			if (stateTimer <= 0.f)
			{
				previous_state = GameState::START_SCREEN_ANIMATION;
				current_state = GameState::START_SCREEN;
				stateTimer = INTRO_CUTSCENE_DURATION_MS;
			}
			break;

		case GameState::START_SCREEN:
			Mix_PauseMusic();
			renderer_system.drawStartScreen();
			break;

		case GameState::GAME_PLAY:
			// CK: be mindful of the order of your systems and rearrange this list only if necessary
			world_system.step(elapsed_ms);
			ai_system.step(elapsed_ms);
			physics_system.step(elapsed_ms);
			world_system.handle_collisions();
            particle_system.step(elapsed_ms);
			animation_system.step(elapsed_ms);
			renderer_system.draw();
			renderer_system.drawUIElements();
			break;

		case GameState::PAUSE:
			renderer_system.draw();
			break;

		case GameState::SHOP:
			renderer_system.drawShopScreen();
			break;

		case GameState::INFO:
			renderer_system.drawInfoScreen();
			break;
		case GameState::GAME_OVER:
			Mix_PauseMusic();
			renderer_system.drawGameOverScreen();
			break;

		case GameState::GAMEPLAY_CUTSCENE: // intro cutscene
			Mix_PauseMusic();
			stateTimer -= elapsed_ms;
			renderer_system.drawCutScreneAnimation();
			animation_system.step(elapsed_ms);

            if (stateTimer <= 0.f) {
                stateTimer = BOOT_CUTSCENE_DURATION_MS;
				removeCutScene();
                previous_state = current_state;
                current_state = GameState::GAME_PLAY;
				world_system.startTheme();
            }
            break;

        case GameState::NEXT_LEVEL:
            stateTimer -= elapsed_ms * 5;
            renderer_system.drawNextLevelScreen();

            if (stateTimer <= 0.f) {
                stateTimer = BOOT_CUTSCENE_DURATION_MS;
                previous_state = current_state;
                current_state = GameState::GAME_PLAY;
            }
            break;
		
		case GameState::VICTORY:
			renderer_system.drawCutScreneAnimation();
			animation_system.step(elapsed_ms);
			break;

        default:
            break;
		}
	}

	return EXIT_SUCCESS;
}
