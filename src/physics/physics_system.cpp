// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>

void PhysicsSystem::step(float elapsed_ms)
{
	// Move each entity that has motion (invaders, projectiles, and even towers [they have 0 for velocity])
	// based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.

	Entity& player_entity = registry.players.entities[0];
	Motion& player_motion = registry.motions.get(player_entity);

	// move everything that needs to move first. deal with collisions / out of bounds later
	auto& motion_registry = registry.motions;
	for(uint i = 0; i< motion_registry.size(); i++)
	{
		Entity entity = motion_registry.entities[i];
		Motion& motion = motion_registry.components[i];
		motion.position += motion.velocity * elapsed_ms;
	}

	// update player grid position
	Player& player = registry.players.get(player_entity);
	player.grid_position = positionToGridCell(registry.motions.get(player_entity).position);

	// Handle enemy-projectile or enemy-player collisions
	for (auto& proj_entity : registry.projectiles.entities)
	{
		Motion& proj_motion = registry.motions.get(proj_entity);
		for (auto& e_entity : registry.enemies.entities)
		{
			// ensure the projectile or player is the "first" entity
			Motion& e_motion = registry.motions.get(e_entity);
			if (detector.hasCollided(proj_motion, e_motion))
			{
				registry.collisions.emplace_with_duplicates(proj_entity, e_entity);
			}
			else if (detector.hasCollided(player_motion, e_motion))
			{
				registry.collisions.emplace_with_duplicates(player_entity, e_entity);
			}
		}
	}

	// Handle player-wall collisions
	if (registry.velocities.has(player_entity))
	{
		Velocity& dash = registry.velocities.get(player_entity);

		for (auto& entity : registry.walls.entities)
		{
			Motion& motion = registry.motions.get(entity);
			bool collided = detector.checkAndHandleWallCollision(player_motion, dash, entity);
			if (collided) break;
		}

		player.dash_cooldown_ms -= elapsed_ms;

		if (dash.speed <= 0)
		{
			registry.velocities.remove(player_entity);
			player_motion.velocity = { 0, 0 };
			player.dash_cooldown_ms = 0;

			toggleDashAnimation(player_entity, false); // goes back to idle animation
		}
		else
		{
			// Compute new velocity for dash
			dash.speed += PLAYER_DASH_DECELERATION * elapsed_ms;
			float angle_radians = (dash.angle - 90) * (M_PI / 180.0f);
			// set new velocity components
			player_motion.velocity.x = dash.speed * cosf(angle_radians);
			player_motion.velocity.y = dash.speed * sinf(angle_radians);
		}
	}
}