#pragma once

#include "common.hpp"
#include "collision_detect.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
private:
	// the collision detector to detect and handle collisions
	CollisionDetector detector;
public:
	// step the physics engine forward by elapsed_ms milliseconds
	void step(float elapsed_ms);

	PhysicsSystem()
	{
	}
};