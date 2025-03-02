// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include "animation_system.hpp"
#include <iostream>

#include <glm/gtx/normalize_dot.hpp>
// include lerp
#include <glm/gtx/compatibility.hpp>

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

	// cache player's motion if players exist
	Motion* playerMotion = nullptr;
	if (!registry.players.entities.empty()) {
		playerMotion = &registry.motions.get(registry.players.entities[0]);
	}

	auto& motion_registry = registry.motions;
	for (uint i = 0; i < motion_registry.size(); i++) {
		Entity entity = motion_registry.entities[i];
		Motion& motion = motion_registry.components[i];

		motion.position += motion.velocity * step_seconds;

		if (registry.players.has(entity)) {
			// player update: use cached boundaries
			if (motion.position.x >= rightBound + 1 || motion.position.x <= leftBound ||
				motion.position.y <= topBound || motion.position.y >= bottomBound + 1) {
				motion.velocity *= -0.5f;
				motion.position.x = glm::clamp(motion.position.x, leftBound, rightBound);
				motion.position.y = glm::clamp(motion.position.y, topBound, bottomBound);
			}
		}


	}

	// enemy update – use cached playerMotion
	if (playerMotion) {
		// iterate over all enemies
		for (uint j = 0; j < registry.enemies.entities.size(); j++) {
			Entity enemyEntity = registry.enemies.entities[j];
			Motion& enemyMotion = registry.motions.get(enemyEntity);
			EnemyBehavior& enemyBehavior = registry.enemyBehaviors.get(enemyEntity);
			Animation& enemyAnimation = registry.animations.get(enemyEntity);

			EnemyType& type = registry.enemyTypes.get(enemyEntity);

			vec2 diff = playerMotion->position - enemyMotion.position;
			float distance = glm::length(diff);
			bool playerDetected = (distance < enemyBehavior.detectionRadius);
			vec2 toPlayer = diff;

			switch (enemyBehavior.state) {
			case EnemyState::CHASING: {
				if (distance > 0.001f) {
					vec2 direction = glm::normalize(toPlayer);
					enemyMotion.velocity = direction * ENEMY_SPEED;
				}
				break;
			}
			case EnemyState::PATROLLING: {
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
					enemyMotion.position.x = lerp(leftBoundary, rightBoundary, enemyBehavior.patrolTime / ENEMY_PATROL_TIME_MS);
				}
				else
				{
					enemyMotion.position.x = lerp(rightBoundary, leftBoundary, enemyBehavior.patrolTime / ENEMY_PATROL_TIME_MS);
				}

				// sse circular detection to transition to dash state.
				if (playerDetected) {
					if (type.type == EnemyTypes::SPIKE) {
						changeAnimationFrames(enemyEntity, 7, 12);
						enemyBehavior.state = EnemyState::DASHING;
					} else if (type.type == EnemyTypes::RED_BLOOD_CELL) {
						enemyBehavior.state = EnemyState::RUNAWAY;
						std::cout << "changing state to runaway" << std::endl;
					}
					
				}
				break;
			}
			case EnemyState::DASHING: {
				if (distance > 0.001f && playerDetected) {
					// dash toward the player
					vec2 direction = glm::normalize(toPlayer);
					enemyMotion.velocity = direction * ENEMY_SPEED;
				}
				else {
					changeAnimationFrames(enemyEntity, 0, 6);
					enemyBehavior.patrolOrigin = enemyMotion.position;
					enemyBehavior.state = EnemyState::PATROLLING;
					enemyBehavior.patrolTime = 0.0f;
					enemyMotion.velocity = { 0, 0 };
				}
				break;
			}

			case EnemyState::RUNAWAY: {
				std::cout << distance << std::endl; 
				std::cout << enemyBehavior.detectionRadius << std::endl;
				if (distance > 0.001f && playerDetected) {
					// run away from character
					vec2 direction = glm::normalize(toPlayer);
					enemyMotion.velocity = -direction * ENEMY_SPEED;
					std::cout << "I AM RUNNING AWAY FROM YOU" << std::endl;
				} else {
					enemyBehavior.patrolOrigin = enemyMotion.position;
					enemyBehavior.state = EnemyState::PATROLLING;
					enemyBehavior.patrolTime = 0.0f;
					enemyMotion.velocity = { 0, 0 };
				}
				break;
			}
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

			toggleDashAnimation(player_entity, false); // goes back to idle animation
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
	ComponentContainer<Motion>& motion_container = registry.motions;
	for (uint i = 0; i < motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];

		for (uint j = i + 1; j < motion_container.components.size(); j++)
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