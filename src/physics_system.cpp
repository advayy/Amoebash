// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include "animation_system.hpp"
#include <iostream>

#include <glm/gtx/normalize_dot.hpp>
// include lerp
#include <glm/gtx/compatibility.hpp>

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion &motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return {abs(motion.scale.x), abs(motion.scale.y)};
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
bool collides(const Motion &motion1, const Motion &motion2)
{
	// object 1, set left right
	vec2 bb1 = get_bounding_box(motion1);
	vec2 bb2 = get_bounding_box(motion2);

	return glm::distance(motion1.position, motion2.position) < (bb1.x + bb2.x) / 2;
}

void PhysicsSystem::step(float elapsed_ms)
{
	// Move each entity that has motion (invaders, projectiles, and even towers [they have 0 for velocity])
	// based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.

	float step_seconds = elapsed_ms / 1000.f;
	// precompute boundaries for player motion
	float leftBound = MAP_LEFT * GRID_CELL_WIDTH_PX;
	float rightBound = MAP_RIGHT * GRID_CELL_WIDTH_PX - 1;
	float topBound = MAP_TOP * GRID_CELL_HEIGHT_PX;
	float bottomBound = MAP_BOTTOM * GRID_CELL_HEIGHT_PX - 1;

	Entity& player_entity = registry.players.entities[0];
	Motion& player_motion = registry.motions.get(player_entity);
	Player& player = registry.players.get(player_entity);

	// Handle player dashing
	// this needs to be done first since the collision resolution updates the player dash angle
	// so we need the updated velocities before we update player position
	for (uint i = 0; i < registry.dashes.size(); i++)// Is alwats a loop of 1 as we remove other dashes when we start a dash
	{
		Entity dash_entity = registry.dashes.entities[i];
		Dashing& dash = registry.dashes.get(dash_entity);
		Motion& motion = registry.motions.get(player_entity);

		dash.timer_ms -= elapsed_ms;

		if (dash.timer_ms <= 0)
		{
			registry.dashes.remove(dash_entity);
			motion.velocity = { 0, 0 };
			toggleDashAnimation(player_entity, false); // goes back to idle animation
		}
		else
		{
			// update player velocity
			motion.velocity = dash.velocity;

			// Gradually change the velocity
			dash.velocity *= VELOCITY_DECAY_RATE; // FLAG MIGHT NEED TO SWAP THIS ONE OUT TOO.
		}
	}

	auto &motion_registry = registry.motions;
	for (uint i = 0; i < motion_registry.size(); i++)
	{
		Entity entity = motion_registry.entities[i];
		Motion &motion = motion_registry.components[i];

		motion.position += motion.velocity * step_seconds;

		if (registry.keys.has(entity)) {
			float dampingFactor = 0.8f;
			motion.velocity *= dampingFactor;
			if (glm::length(motion.velocity) < 0.01f) {
				motion.velocity = vec2(0.0f, 0.0f);
			}
			if (motion.position.x >= rightBound + 1 || motion.position.x <= leftBound ||
				motion.position.y <= topBound || motion.position.y >= bottomBound + 1)  {
					// temporary hexagon motion
					
					
					
					motion.position.x = glm::clamp(motion.position.x, leftBound, rightBound);
					motion.position.y = glm::clamp(motion.position.y, topBound, bottomBound);
					
					if (motion.velocity != vec2(0.0f, 0.0f)) {
						motion.velocity = -1.f * motion.velocity;
					}
				}
		}

		if (registry.keys.has(entity)) 
		{
			float dampingFactor = 0.8f;
			motion.velocity *= dampingFactor;
			if (glm::length(motion.velocity) < 0.01f) {
				motion.velocity = vec2(0.0f, 0.0f);
			}
			if (motion.position.x >= rightBound + 1 || motion.position.x <= leftBound ||
				motion.position.y <= topBound || motion.position.y >= bottomBound + 1)  {
					// temporary hexagon motion
					
					
					
					motion.position.x = glm::clamp(motion.position.x, leftBound, rightBound);
					motion.position.y = glm::clamp(motion.position.y, topBound, bottomBound);
					
					if (motion.velocity != vec2(0.0f, 0.0f)) {
						motion.velocity = -1.f * motion.velocity;
					}
				}
		}

		if (registry.players.has(entity))
		{
			// player update: use cached boundaries
			if (motion.position.x >= rightBound + 1 || motion.position.x <= leftBound ||
				motion.position.y <= topBound || motion.position.y >= bottomBound + 1)
			{
				motion.velocity *= -0.5f;
				motion.position.x = glm::clamp(motion.position.x, leftBound, rightBound);
				motion.position.y = glm::clamp(motion.position.y, topBound, bottomBound);
			}
		}

		// buff drops and slides
		if (registry.buffs.has(entity))
		{
			float friction = 0.95f;
			motion.velocity *= friction;
			if (glm::length(motion.velocity) < 5.0f)
			{
				motion.velocity = {0, 0};
			}
		}
	}

	// iterate over all enemies
	for (uint j = 0; j < registry.enemies.entities.size(); j++)
	{
		Entity enemyEntity = registry.enemies.entities[j];
		Motion& enemyMotion = registry.motions.get(enemyEntity);
		EnemyBehavior& enemyBehavior = registry.enemyBehaviors.get(enemyEntity);
		Animation& enemyAnimation = registry.animations.get(enemyEntity);

		vec2 diff = player_motion.position - enemyMotion.position;
		float distance = glm::length(diff);
		bool playerDetected = (distance < (enemyBehavior.detectionRadius * registry.players.get(registry.players.entities[0]).detection_range)); // PLAYERS DETECTION RANGE BUFF?
		vec2 toPlayer = diff;

		switch (enemyBehavior.state)
		{
		case EnemyState::CHASING:
		{
			if (distance > 0.001f)
			{
				vec2 direction = glm::normalize(toPlayer);
				enemyMotion.velocity = direction * ENEMY_SPEED; // FLAG REMOVE CONSTANT LATER FOR ENEMIES PARTICULAR SPEED.
			}
			break;
		}
		case EnemyState::PATROLLING:
		{
			// define patrol boundaries based on stored origin and range.
			float leftBoundary = enemyBehavior.patrolOrigin.x - enemyBehavior.patrolRange;
			float rightBoundary = enemyBehavior.patrolOrigin.x + enemyBehavior.patrolRange;

			// if enemy goes past boundaries, reverse its patrol direction.
			if (enemyMotion.position.x < leftBoundary || enemyMotion.position.x > rightBoundary)
			{
				enemyBehavior.patrolForwards = !enemyBehavior.patrolForwards;
				enemyBehavior.patrolTime = 0.0f;
			}

			enemyBehavior.patrolTime += elapsed_ms;

			// M1 interpolation implementation
			if (enemyBehavior.patrolForwards)
			{
				enemyMotion.position.x = lerp(leftBoundary, rightBoundary, enemyBehavior.patrolTime / ENEMY_PATROL_TIME_MS);  // FLAG REMOVE CONSTANT LATER FOR ENEMIES PARTICULAR SPEED.
			}
			else
			{
				enemyMotion.position.x = lerp(rightBoundary, leftBoundary, enemyBehavior.patrolTime / ENEMY_PATROL_TIME_MS);  // FLAG REMOVE CONSTANT LATER FOR ENEMIES PARTICULAR SPEED.
			}

			// sse circular detection to transition to dash state.
			if (playerDetected)
			{
				changeAnimationFrames(enemyEntity, 7, 12);
				enemyBehavior.state = EnemyState::DASHING;
			}
			break;
		}
		case EnemyState::DASHING:
		{
			if (distance > 0.001f && playerDetected)
			{
				// dash toward the player
				vec2 direction = glm::normalize(toPlayer);
				enemyMotion.velocity = direction * ENEMY_SPEED;
			}
			else
			{
				changeAnimationFrames(enemyEntity, 0, 6);
				enemyBehavior.patrolOrigin = enemyMotion.position;
				enemyBehavior.state = EnemyState::PATROLLING;
				enemyBehavior.patrolTime = 0.0f;
				enemyMotion.velocity = { 0, 0 };
			}
			break;
		}
		}
	}
			
	// PLAYER DASH ACTION COOLDOWN
	player.dash_cooldown_timer_ms -= elapsed_ms;
	if (player.dash_cooldown_timer_ms <= 0)
	{
		if (player.dash_count < player.max_dash_count)
		{
			player.dash_count++;

			if (player.dash_count == player.max_dash_count)
			{
				player.dash_cooldown_timer_ms = 0;
			}
			else
			{
				player.dash_cooldown_timer_ms = player.dash_cooldown_ms;
			}
		}
	}

	// update player grid position
	player.grid_position = positionToGridCell(registry.motions.get(registry.players.entities[0]).position);

	// Collisions are always in this order: (Player | Projectiles | Chest, Key | Enemy | Wall | Buff)
	for (auto& e_entity : registry.enemies.entities)
	{
		// Handle enemy-projectile or enemy-player collisions
		Motion& e_motion = registry.motions.get(e_entity);
		
		for (auto& proj_entity : registry.projectiles.entities)
		{
			Motion& proj_motion = registry.motions.get(proj_entity);

			// ensure the projectile is the "first" entity
			if (detector.hasCollided(proj_motion, e_motion))
			{
				registry.collisions.emplace_with_duplicates(proj_entity, e_entity);
			}
		}

		// ensure the player is the "first" entity
		if (detector.hasCollided(player_motion, e_motion))
		{
			registry.collisions.emplace_with_duplicates(player_entity, e_entity);
		}
	}

	for (auto& buff_entity : registry.buffs.entities)
	{
		// Handle player-buff collisions
		Motion& buff_motion = registry.motions.get(buff_entity);

		if (detector.hasCollided(player_motion, buff_motion))
		{
			registry.collisions.emplace_with_duplicates(player_entity, buff_entity);
		}

		handleWallCollision(buff_entity);
	}

	for (auto& key_entity : registry.keys.entities)
	{
		// Handle player-key collisions
		Motion& key_motion = registry.motions.get(key_entity);

		if (detector.hasCollided(player_motion, key_motion))
		{
			registry.collisions.emplace_with_duplicates(player_entity, key_entity);
		}

		for (auto& chest_entity : registry.chests.entities)
		{
			// Handle chest-key collisions
			Motion& chest_motion = registry.motions.get(chest_entity);

			if (detector.hasCollided(chest_motion, key_motion))
			{
				registry.collisions.emplace_with_duplicates(chest_entity, key_entity);
			}
		}

		handleWallCollision(key_entity);
	}
	
	handleWallCollision(player_entity);
}

void PhysicsSystem::handleWallCollision(Entity& entity)
{
	Motion& motion = registry.motions.get(entity);

	std::vector<Entity> nearby_walls;

	for (auto& wall_entity : registry.walls.entities)
	{
		Motion& wall = registry.motions.get(wall_entity);
		float distance_to_motion = glm::distance(motion.position, wall.position);

		// skip the far away walls
		if (distance_to_motion > GRID_CELL_WIDTH_PX) continue;

		nearby_walls.push_back(wall_entity);
	}

	for (auto& wall_entity : nearby_walls)
	{
		if (registry.players.has(entity))
		{
			if (registry.dashes.components.size() > 0)
			{
				Dashing& dash = registry.dashes.components[0];
				auto collided = detector.checkAndHandlePlayerWallCollision(motion, dash.angle_deg, wall_entity);
				if (collided.first) detector.handleDashOnWallEdge(collided.second, dash);
			}
			else
			{
				detector.checkAndHandlePlayerWallCollision(motion, motion.angle, wall_entity);
			}
		}
		else
		{
			detector.checkAndHandleGeneralWallCollision(motion, wall_entity);
		}
	}
}