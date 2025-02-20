// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
bool collides(const Motion& motion1, const Motion& motion2)
{
	// object 1, set left right
	vec2 bb1 = get_bounding_box(motion1);
	vec2 bb2 = get_bounding_box(motion2);

	return false;
}

void PhysicsSystem::step(float elapsed_ms)
{
	// Move each entity that has motion (invaders, projectiles, and even towers [they have 0 for velocity])
	// based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.

	// MOVE ENTITIES
	Entity& player_entity = registry.players.entities[0];

	auto& motion_registry = registry.motions;
	for(uint i = 0; i< motion_registry.size(); i++)
	{
		Entity entity = motion_registry.entities[i];
		Motion& motion = motion_registry.components[i];
		motion.position += motion.velocity * elapsed_ms;
		// dont let player move out of bounds, knockback when they do that (reducing velocity)
		if (registry.players.has(entity)) {
			Player& player = registry.players.get(entity);
			if (motion.position.x >= MAP_RIGHT*GRID_CELL_WIDTH_PX || motion.position.x <= MAP_LEFT*GRID_CELL_WIDTH_PX || motion.position.y <= MAP_TOP*GRID_CELL_HEIGHT_PX || motion.position.y >= MAP_BOTTOM*GRID_CELL_HEIGHT_PX) {
				motion.velocity *= -0.5f;
				// clamp position
				motion.position.x = std::clamp(motion.position.x, MAP_LEFT*GRID_CELL_WIDTH_PX, (MAP_RIGHT)*GRID_CELL_WIDTH_PX - 1);
				motion.position.y = std::clamp(motion.position.y, MAP_TOP*GRID_CELL_HEIGHT_PX, (MAP_BOTTOM)*GRID_CELL_HEIGHT_PX - 1);
			}
		}

	}


	// Handle player dashing
	for (uint i = 0; i < registry.velocities.size(); i++)
	{
    	Player& player = registry.players.get(player_entity);
		Velocity& dash = registry.velocities.get(player_entity);
    	Motion& motion = registry.motions.get(player_entity);

    	player.dash_cooldown_ms -= elapsed_ms;

    	if (dash.speed <= 0)
  		{
   	    	registry.velocities.remove(player_entity);
    	    motion.velocity = { 0, 0 };
   	    	player.dash_cooldown_ms = 0;

			changeAnimationFrames(registry.players.entities[0], 0, 3); // sets current frame to start frame
    	}
    	else
    	{
        	// Compute new velocity for dash
			dash.speed += PLAYER_DASH_DECELERATION * elapsed_ms;
        	float angle_radians = (dash.angle - 90) * (M_PI / 180.0f);
			std::cout << dash.angle << std::endl;
			std::cout << angle_radians << std::endl;
        	// Apply velocity decay
    	    motion.velocity.x = dash.speed * cosf(angle_radians);
			motion.velocity.y = dash.speed * sinf(angle_radians);
    	}
	}

	// update player grid position
	Player& player = registry.players.get(player_entity);
	player.grid_position = positionToGridCell(registry.motions.get(player_entity).position);

	// Handle enemy-projectile or enemy-player collisions
	Motion& player_motion = registry.motions.get(player_entity);
	for (auto& proj_entity : registry.projectiles.entities)
	{
		Motion& proj_motion = registry.motions.get(proj_entity);

		for (auto& e_entity : registry.enemies.entities)
		{
			Motion& e_motion = registry.motions.get(e_entity);
			if (collides(proj_motion, e_motion))
			{
				registry.collisions.emplace_with_duplicates(proj_entity, e_entity);
			}
			else if (collides(player_motion, e_motion))
			{
				registry.collisions.emplace_with_duplicates(player_entity, e_entity);
			}
		}
	}

	// Handle player-wall collisions
	
	for (auto& entity : registry.walls.entities)
	{
		Motion& motion = registry.motions.get(entity);

		if (collides(player_motion, motion))
		{
			Velocity& player_velocity = registry.velocities.get(player_entity);
			player_velocity.angle *= -1;
			break;
		}
	}
}