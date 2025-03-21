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

SpikeEnemyState AISystem::handleSpikeEnemyBehavior(Entity& enemyEntity, SpikeEnemyAI& enemyBehavior, float dist, vec2 direction, bool playerDetected, float elapsed_ms)
{
	Motion& enemyMotion = registry.motions.get(enemyEntity);

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

			float t = enemyBehavior.patrolTime / ENEMY_PATROL_TIME_MS;
			enemyMotion.position.x = enemyBehavior.patrolForwards ? lerp(leftBoundary, rightBoundary, t) : lerp(rightBoundary, leftBoundary, t);
			
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

	return enemyBehavior.state;
}

RBCEnemyState AISystem::handleRBCBehavior(Entity& enemyEntity, RBCEnemyAI& enemyBehavior, float dist, vec2 direction, bool playerDetected, float elapsed_ms)
{
	Motion& enemyMotion = registry.motions.get(enemyEntity);

	switch(enemyBehavior.state) 
	{
		case RBCEnemyState::FLOATING: {
			enemyBehavior.patrolTime += elapsed_ms;
			if (enemyBehavior.patrolTime >= 3000.f) {
				std::random_device rd;
				std::default_random_engine rng(rd());
				std::uniform_real_distribution<float> uniform_dist(0.0f, 360 * 1.0f);
				enemyMotion.angle = uniform_dist(rng);
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
			}
			else {
				enemyBehavior.patrolOrigin = enemyMotion.position;
				enemyBehavior.state = RBCEnemyState::FLOATING;
				enemyBehavior.patrolTime = ENEMY_PATROL_TIME_MS / 2;
			}
			break;
		}

		default :
			enemyBehavior.state = RBCEnemyState::FLOATING;
			break;
	}
	
	return enemyBehavior.state;
}

BacteriophageState AISystem::handleBacteriophageBehavior(Entity& enemyEntity, BacteriophageAI& enemyBehavior, vec2 direction, bool playerDetected, float elapsed_ms, vec2 positionToReach, vec2 directionToPlayer)  {
	
	Motion& enemyMotion = registry.motions.get(enemyEntity);

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

	return enemyBehavior.state;
}

BossState AISystem::handleBossBehaviour(Entity& enemyEntity, BossAI& enemyBehavior, float dist, vec2 direction, bool playerDetected, float elapsed_ms)
{
	Motion &enemyMotion = registry.motions.get(enemyEntity);

	
	Enemy& enemy = registry.enemies.get(enemyEntity);
	if (enemyBehavior.state != BossState::INITIAL)
		enemy.health *= 0.9999999f;

	std::cout << enemy.health << " " << enemy.total_health << std::endl;

	switch (enemyBehavior.state)
	{
		case BossState::INITIAL:
		{
			if(playerDetected)
			{

				int next = 1 + (std::rand() % ((int)BossState::NUM_STATES - 1));
				enemyBehavior.state = static_cast<BossState>(next);
				
				if (enemyBehavior.state == BossState::SHOOT_PARADE)
				{
					enemyBehavior.shoot_cool_down = 0.f;
					enemyBehavior.cool_down = 3000.f;
				}
				else if (enemyBehavior.state == BossState::RUMBLE)
				{
					enemyBehavior.rumble_charge_time = 1500.f;
					enemyBehavior.rumble_duration = 1000.f;
					enemyBehavior.is_charging = true;
				}
			}
			break;
		}
		case BossState::IDLE: 
		{
			if (enemyMotion.angle != 0.f) {
				const float smoothing_factor = 0.1f;

				enemyMotion.angle = glm::lerp(enemyMotion.angle, 0.f, smoothing_factor);

				if (std::fabs(enemyMotion.angle) < 0.1f) {
					enemyMotion.angle = 0.f;
				}
			}

			enemyBehavior.cool_down -= elapsed_ms;
			if(enemyBehavior.cool_down < 0.f)
			{

				int next = 1 + (std::rand() % ((int)BossState::NUM_STATES - 1));
				enemyBehavior.state = static_cast<BossState>(next);
				
				if (enemyBehavior.state == BossState::SHOOT_PARADE)
				{
					enemyBehavior.shoot_cool_down = 0.f;
					enemyBehavior.cool_down = 3000.f;
				}
				else if (enemyBehavior.state == BossState::RUMBLE)
				{
					enemyBehavior.rumble_charge_time = 1500.f;
					enemyBehavior.rumble_duration = 1000.f;
					enemyBehavior.is_charging = true;
				}
			}		
			break;
		}

		case BossState::SHOOT_PARADE:
		{
			enemyBehavior.cool_down -= elapsed_ms;
			enemyBehavior.shoot_cool_down -= elapsed_ms;
			
			if (enemyBehavior.shoot_cool_down < 0.f) {
				for (int angleDeg = 0; angleDeg < 360; angleDeg += 15)
				{
					float angleRad = glm::radians((float)angleDeg);
					vec2 dir = { cosf(angleRad), sinf(angleRad) };
					vec2 velocity = dir * PROJECTILE_SPEED * 3.f;
					vec2 spawnPos = enemyMotion.position + dir * enemyMotion.scale.x / 3.f;

					createBossProjectile(spawnPos, {PROJECTILE_BB_WIDTH * 3.f, PROJECTILE_BB_HEIGHT * 3.f}, velocity);

				}

				enemyBehavior.shoot_cool_down = 500.f;	
			}

			if (enemyBehavior.cool_down < 0.f) {
				enemyBehavior.state = BossState::IDLE;
				enemyBehavior.cool_down = 10000.f;
			}	

			break;
		}

		case BossState::RUMBLE:
		{
			if (enemyBehavior.is_charging) {
				enemyBehavior.rumble_charge_time -= elapsed_ms;
				enemyMotion.velocity = { 0.f, 0.f };

				enemyMotion.angle = atan2(direction.y, direction.x) * (180.f / M_PI) + 90.f;

				if (enemyBehavior.rumble_charge_time <= 0.f) {
					vec2 locked_direction = glm::normalize(direction);
					enemyMotion.velocity = locked_direction * ENEMY_SPEED * 4.f;

					enemyBehavior.is_charging = false;
					enemyBehavior.rumble_duration = 1000.f;
				}
			}
			else {
				enemyBehavior.rumble_duration -= elapsed_ms;

				if (enemyBehavior.rumble_duration <= 0.f) {
					
					enemyMotion.velocity = { 0.f, 0.f };
					enemyBehavior.state = BossState::IDLE;
					enemyBehavior.cool_down = 5000.f;
					enemyBehavior.rumble_charge_time = 1500.f;
					enemyBehavior.is_charging = true;
				}
			}
			break;
		}

		default:
			break;
	}

	// std::cout << static_cast<int>(enemyBehavior.state) << std::endl;
	
	return enemyBehavior.state;
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

		enemyBehavior.state = handleSpikeEnemyBehavior(enemyEntity, enemyBehavior, dist, direction, playerDetected, elapsed_ms);	
	}
	
	for (auto& enemyEntity : registry.rbcEnemyAIs.entities)
	{
		RBCEnemyAI& enemyBehavior = registry.rbcEnemyAIs.get(enemyEntity);
		Motion& enemyMotion = registry.motions.get(enemyEntity);
		Animation& enemyAnimation = registry.animations.get(enemyEntity);

		vec2 direction;
		float dist = 0;
		bool playerDetected = isPlayerInRadius(playerMotion.position, enemyMotion.position, dist, direction, enemyBehavior.detectionRadius);

		enemyBehavior.state = handleRBCBehavior(enemyEntity, enemyBehavior, dist, direction, playerDetected, elapsed_ms);

		EnemyAI castedEnemyBehavior = (EnemyAI)enemyBehavior;
		enemyBehavior.patrolForwards = castedEnemyBehavior.patrolForwards;
		enemyBehavior.patrolTime = castedEnemyBehavior.patrolTime;
		enemyBehavior.patrolOrigin = castedEnemyBehavior.patrolOrigin;
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

		enemyBehavior.state = handleBacteriophageBehavior(enemyEntity, enemyBehavior, direction, playerDetected, elapsed_ms, positionToReach, directionToPlayer);
	}

	for (auto& enemyEntity : registry.bossAIs.entities)
	{
		BossAI& enemyBehavior = registry.bossAIs.get(enemyEntity);
		Motion& enemyMotion = registry.motions.get(enemyEntity);

		vec2 direction;
		float dist = 0;
		bool playerDetected = isPlayerInRadius(playerMotion.position, enemyMotion.position, dist, direction, enemyBehavior.detectionRadius);

		enemyBehavior.state = handleBossBehaviour(enemyEntity, enemyBehavior, dist, direction, playerDetected, elapsed_ms);
	}
}
