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


    std::vector<vec2> getWorldVertices(const std::vector<TexturedVertex>& vertices, const vec2 &position, const vec2 &scale);
    bool pointInPolygon(const vec2& point, const std::vector<vec2> &polygon);
    bool willMeshCollideSoon(const Entity& player, const Entity& hexagon, float predictionTime);
private:
	/*
	* Handles wall collisions for the given entity
	*/
	void handleWallCollision(Entity& entity);

	// the collision detector to detect and handle collisions
	CollisionDetector detector;
};

