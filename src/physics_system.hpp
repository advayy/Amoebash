#pragma once

#include "common.hpp"
#include "collisions/collision_detect.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/registry.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
public:

	void step(float elapsed_ms);

	PhysicsSystem()
	{
	}

private:
	/*
	* Handles wall collisions for the given entity
	*/
	void handleWallCollision(Entity& entity);

	// the collision detector to detect and handle collisions
	CollisionDetector detector;
};