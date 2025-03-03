#include <iostream>
#include "ai_system.hpp"
#include "world_init.hpp"
#include "animation_system.hpp"

// include lerp
#include <glm/gtx/compatibility.hpp>

bool AISystem::isPlayerInRadius(vec2 player, vec2 enemy, float& distance, vec2& direction, float detectionRadius)
{
	vec2 diff = player - enemy;
	distance = glm::length(diff);
	direction = glm::normalize(diff);
	return distance < detectionRadius;
}

void AISystem::step(float elapsed_ms)
{
	auto playerMotion = registry.motions.get(registry.players.entities[0]);

	for (auto& enemyEntity : registry.spikeEnemyAIs.entities)
	{
		SpikeEnemyAI& enemyBehavior = registry.spikeEnemyAIs.get(enemyEntity);
		Motion& enemyMotion = registry.motions.get(enemyEntity);
		Animation& enemyAnimation = registry.animations.get(enemyEntity);

		vec2 direction;
		float dist = 0;
		bool playerDetected = isPlayerInRadius(playerMotion.position, enemyMotion.position, dist, direction, enemyBehavior.detectionRadius);

		switch (enemyBehavior.state) 
		{
			case SpikeEnemyState::CHASING: 
			{
				if (dist > 0.001f) 
				{
					enemyMotion.velocity = direction * ENEMY_SPEED;
				}
				break;
			}
			case SpikeEnemyState::PATROLLING: 
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
					enemyBehavior.state = SpikeEnemyState::DASHING;
				}
				break;
			}
			case SpikeEnemyState::DASHING:
			{
				if (dist > 0.001f && playerDetected) 
				{
					// dash toward the player
					enemyMotion.velocity = direction * ENEMY_SPEED;
				}
				else 
				{
					changeAnimationFrames(enemyEntity, 0, 6);
					enemyBehavior.patrolOrigin = enemyMotion.position;
					enemyBehavior.state = SpikeEnemyState::PATROLLING;
					enemyBehavior.patrolTime = 0.0f;
					enemyMotion.velocity = { 0, 0 };
				}
				break;
			}
		}
	}

	for (auto& enemyEntity : registry.bacteriophageAIs.entities)
	{
		BacteriophageAI& enemyBehavior = registry.bacteriophageAIs.get(enemyEntity);
		Motion& enemyMotion = registry.motions.get(enemyEntity);

		vec2 directionToPlayer = glm::normalize(playerMotion.position - enemyMotion.position);

		float circleAngle = (2 * M_PI / MAX_BACTERIOPHAGE_COUNT) * enemyBehavior.placement_index;
		vec2 positionToReach = playerMotion.position + vec2(cosf(circleAngle) * ENEMY_DETECTION_RADIUS, sinf(circleAngle) * ENEMY_DETECTION_RADIUS);

		if (glm::distance(enemyMotion.position, positionToReach) > 1)
		{
			vec2 direction = glm::normalize(positionToReach - enemyMotion.position);
			enemyMotion.velocity = direction * ENEMY_SPEED;
		}
		else
		{
			enemyMotion.velocity = { 0, 0 };
		}

		enemyMotion.angle = tanf(directionToPlayer.y / directionToPlayer.x);
	}
}