#include <iostream>
#include "ai_system.hpp"
#include "world_init.hpp"
#include "animation_system.hpp"

void AISystem::step(float elapsed_ms)
{
	Entity& player_entity = registry.players.entities[0];
	Motion& player_motion = registry.motions.get(player_entity);

	// iterate over all enemies to make them track player if detected
	for (uint j = 0; j < registry.enemies.entities.size(); j++) 
	{
		Entity enemyEntity = registry.enemies.entities[j];
		Motion& enemyMotion = registry.motions.get(enemyEntity);
		EnemyBehavior& enemyBehavior = registry.enemyBehaviors.get(enemyEntity);
		Animation& enemyAnimation = registry.animations.get(enemyEntity);

		vec2 diff = player_motion.position - enemyMotion.position;
		float distance = glm::length(diff);
		bool playerDetected = (distance < enemyBehavior.detectionRadius);
		vec2 toPlayer = diff;

		switch (enemyBehavior.state) 
		{
			case EnemyState::CHASING:
			{
				if (distance > 0.001f) 
				{
					vec2 direction = glm::normalize(toPlayer);
					enemyMotion.velocity = direction * ENEMY_SPEED_PER_MS;
				}
				break;
			}
			case EnemyState::PATROLLING: 
			{
				// define patrol boundaries based on stored origin and range.
				float leftBoundary = enemyBehavior.patrolOrigin.x - enemyBehavior.patrolRange;
				float rightBoundary = enemyBehavior.patrolOrigin.x + enemyBehavior.patrolRange;

				// if enemy goes past boundaries, reverse its patrol direction and clamp position.
				if (enemyMotion.position.x < leftBoundary || enemyMotion.position.x > rightBoundary) 
				{
					enemyBehavior.patrolForwards = !enemyBehavior.patrolForwards;
					enemyBehavior.patrolTime = 0.0f;
				}

				enemyBehavior.patrolTime += elapsed_ms;
				
				if (enemyBehavior.patrolForwards)
				{
					enemyMotion.position.x = lerp(leftBoundary, rightBoundary, enemyBehavior.patrolTime / ENEMY_PATROL_TIME_MS);
				}
				else
				{
					enemyMotion.position.x = lerp(rightBoundary, leftBoundary, enemyBehavior.patrolTime / ENEMY_PATROL_TIME_MS);
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
					enemyMotion.velocity = direction * ENEMY_SPEED_PER_MS;
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
}