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


	auto& motion_registry = registry.motions;
	for(uint i = 0; i< motion_registry.size(); i++)
	{
		Entity entity = motion_registry.entities[i];
		if (registry.buttons.has(entity) || registry.gameScreens.has(entity)) {
			continue;
		} 
		
		Motion& motion = motion_registry.components[i];
		float step_seconds = elapsed_ms / 1000.f;
		motion.position += motion.velocity * step_seconds;
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
	for (uint i = 0; i < registry.dashes.size(); i++)
	{
    	Entity player_entity = registry.players.entities[0];
    	Entity dash_entity = registry.dashes.entities[i];

    	Player& player = registry.players.get(player_entity);
    	Dashing& dash = registry.dashes.components[i];
    	Motion& motion = registry.motions.get(player_entity);

    	dash.timer_ms -= elapsed_ms;
    	player.dash_cooldown_ms -= elapsed_ms;

    	if (dash.timer_ms <= 0)
  		{
   	    	registry.dashes.remove(dash_entity);
    	    motion.velocity = { 0, 0 };
   	    	player.dash_cooldown_ms = 0;

			changeAnimationFrames(registry.players.entities[0], 0, 3); // sets current frame to start frame
    	}
    	else
    	{
        	// Compute base velocity for dash
        	float angle_radians = (dash.angle - 90) * (M_PI / 180.0f);
        	vec2 base_velocity = {
        	    PLAYER_SPEED * cosf(angle_radians), 
    	        PLAYER_SPEED * sinf(angle_radians)
	        };

        	// Apply velocity decay
    	    motion.velocity.x = base_velocity.x * dash.speed_factor;
	        motion.velocity.y = base_velocity.y * dash.speed_factor;

        	// Gradually reduce speed factor
        	dash.speed_factor *= VELOCITY_DECAY_RATE;
    	}
	}

	// update player grid position
	Player& player = registry.players.get(registry.players.entities[0]);
	player.grid_position = positionToGridCell(registry.motions.get(registry.players.entities[0]).position);


	// Handle collisions
    ComponentContainer<Motion> &motion_container = registry.motions;
	for(uint i = 0; i < motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];
		
		for(uint j = i+1; j < motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];
			if (collides(motion_i, motion_j))
			{
				Entity entity_j = motion_container.entities[j];
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
			}
		}
	}
}