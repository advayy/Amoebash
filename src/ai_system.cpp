#include <random>
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

SpikeEnemyState AISystem::handleDefaultEnemyBehavior(Entity& enemyEntity, EnemyAI& enemyBehavior, int state, vec2 direction, float dist, bool playerDetected, float elapsed_ms)
{
	Motion& enemyMotion = registry.motions.get(enemyEntity);
	switch ((SpikeEnemyState)state)
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
			return SpikeEnemyState::DASHING;
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
			enemyBehavior.patrolTime = 0.0f;
			enemyMotion.velocity = { 0, 0 };
			return SpikeEnemyState::PATROLLING;
		}
		break;
	}
	}

	return (SpikeEnemyState)state;
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

		enemyBehavior.state = handleDefaultEnemyBehavior(enemyEntity, enemyBehavior, (int)enemyBehavior.state, direction, dist, playerDetected, elapsed_ms);
	}

	for (auto& enemyEntity : registry.rbcEnemyAIs.entities)
	{
		RBCEnemyAI& enemyBehavior = registry.rbcEnemyAIs.get(enemyEntity);
		Motion& enemyMotion = registry.motions.get(enemyEntity);
		Animation& enemyAnimation = registry.animations.get(enemyEntity);

		vec2 direction;
		float dist = 0;
		bool playerDetected = isPlayerInRadius(playerMotion.position, enemyMotion.position, dist, direction, enemyBehavior.detectionRadius);

		EnemyAI castedEnemyBehavior = (EnemyAI)enemyBehavior;
		handleDefaultEnemyBehavior(enemyEntity, castedEnemyBehavior, (int)enemyBehavior.state, direction, dist, playerDetected, elapsed_ms);
		enemyBehavior.patrolForwards = castedEnemyBehavior.patrolForwards;
		enemyBehavior.patrolTime = castedEnemyBehavior.patrolTime;
		enemyBehavior.patrolOrigin = castedEnemyBehavior.patrolOrigin;

		switch (enemyBehavior.state)
		{
		case RBCEnemyState::CHASING:
		case RBCEnemyState::PATROLLING:
		case RBCEnemyState::DASHING:
		{
			enemyBehavior.state = (RBCEnemyState)(int)handleDefaultEnemyBehavior(enemyEntity, enemyBehavior, (int)enemyBehavior.state, direction, dist, playerDetected, elapsed_ms);
			break;
		}
		// Just for RBC's
		case RBCEnemyState::FLOATING: {
			enemyBehavior.patrolTime += elapsed_ms;
			if (enemyBehavior.patrolTime >= 3000.f) {
				std::random_device rd;
				std::default_random_engine rng(rd());
				std::uniform_real_distribution<float> uniform_dist(0.0f, 360 * 1.0f);
				enemyMotion.angle = uniform_dist(rng);
				std::cout << enemyMotion.angle << std::endl;
				enemyBehavior.patrolTime = 0.f;


				enemyMotion.velocity.x = ENEMY_SPEED * cos(enemyMotion.angle) / 4;
				enemyMotion.velocity.y = ENEMY_SPEED * sin(enemyMotion.angle) / 4;
			}


			if (playerDetected) {
				enemyBehavior.state = RBCEnemyState::RUNAWAY;
			}

			break;
		}

		case RBCEnemyState::RUNAWAY: {
			if (dist > 0.001f && playerDetected) {
				// run away from character
				float new_angle = atan2(direction.y, direction.x);
				new_angle *= (360.f / (2 * M_PI)) + 90.f;
				if (new_angle < 0) {
					new_angle += 360;
				}
				enemyMotion.angle = new_angle;
				enemyMotion.velocity = -direction * ENEMY_SPEED;
				std::cout << "I AM RUNNING AWAY FROM YOU" << std::endl;
			}
			else {
				enemyBehavior.patrolOrigin = enemyMotion.position;
				enemyBehavior.state = RBCEnemyState::FLOATING;
				enemyBehavior.patrolTime = ENEMY_PATROL_TIME_MS / 2;
			}
			break;
		}
		}
	}

	for (auto& enemyEntity : registry.bacteriophageAIs.entities)
	{
		BacteriophageAI& enemyBehavior = registry.bacteriophageAIs.get(enemyEntity);
		Motion& enemyMotion = registry.motions.get(enemyEntity);

		vec2 direction;
		float dist = 0;
		bool playerDetected = isPlayerInRadius(playerMotion.position, enemyMotion.position, dist, direction, enemyBehavior.detectionRadius);
		
		vec2 directionToPlayer = glm::normalize(playerMotion.position - enemyMotion.position);
		float circleAngle = (2 * M_PI / MAX_BACTERIOPHAGE_COUNT) * enemyBehavior.placement_index;
		vec2 positionToReach = playerMotion.position + vec2(cosf(circleAngle) * BACTERIOPHAGE_ENEMY_KEEP_AWAY_RADIUS, sinf(circleAngle) * SPIKE_ENEMY_DETECTION_RADIUS);

		switch (enemyBehavior.state)
		{
		case BacteriophageState::PATROLLING:
		{
			if (playerDetected)
			{
				enemyBehavior.state = BacteriophageState::CHASING;
				enemyMotion.angle = atan2(directionToPlayer.y, directionToPlayer.x) * (180.0f / M_PI) + 90;
				vec2 direction = glm::normalize(positionToReach - enemyMotion.position);
				enemyMotion.velocity = direction * ENEMY_SPEED;
			}
			else
			{
				enemyMotion.velocity = { 0, 0 };
			}
			break;
		}
		case BacteriophageState::CHASING:
			if (!playerDetected)
			{
				enemyBehavior.state = BacteriophageState::PATROLLING;
				enemyMotion.angle = 0.0f;
				enemyMotion.velocity = { 0, 0 };
			}
			else if (glm::distance(enemyMotion.position, positionToReach) > 1)
			{
				vec2 direction = glm::normalize(positionToReach - enemyMotion.position);
				enemyMotion.angle = atan2(directionToPlayer.y, directionToPlayer.x) * (180.0f / M_PI) + 90;
				enemyMotion.velocity = direction * ENEMY_SPEED;
			}
			else
			{
				enemyMotion.velocity = { 0, 0 };
				enemyMotion.angle = atan2(directionToPlayer.y, directionToPlayer.x) * (180.0f / M_PI) + 90;
			}
		}
	}
}