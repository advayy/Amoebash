#include <iostream>
#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"


class AnimationSystem
{
public:
	// step the animation engine forward by elapsed_ms milliseconds
	void step(float elapsed_ms);

	AnimationSystem() {}
};

// animation utils
void changeAnimationFrames(Entity entity, int start_frame, int end_frame);
