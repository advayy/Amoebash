#include <random>
#include <iostream>
#include "ai_system.hpp"
#include "world_init.hpp"
#include "animation_system.hpp"
// include lerp
#include <glm/gtx/compatibility.hpp>

bool AISystem::isPlayerInRadius(vec2 player, vec2 enemy, float& distance, vec2& direction, float detectionRadius)
{
	/* Measure distance to player and check if within detection radius */
	vec2 diff = player - enemy;
	distance = glm::length(diff);
	direction = glm::normalize(diff);
	return distance < detectionRadius * registry.players.get(registry.players.entities[0]).detection_range;
}

SpikeEnemyState AISystem::handleSpikeEnemyBehavior(Entity &enemyEntity, SpikeEnemyAI &enemyBehavior, float dist, vec2 direction, bool playerDetected, float elapsed_ms)
{
	Motion &enemyMotion = registry.motions.get(enemyEntity);

	switch (enemyBehavior.state)
	{
	case SpikeEnemyState::CHASING:
	{
		// adjust velocity towards player
		if (dist > 0.001f)
		{
			enemyMotion.velocity = direction * ENEMY_SPEED;
		}
		break;
	}
	case SpikeEnemyState::PATROLLING:
	{
			// get current X position for comparison later
		float currentX = enemyMotion.position.x;
		
		// define patrol boundaries based on stored origin and range
		float leftBoundary = enemyBehavior.patrolOrigin.x - enemyBehavior.patrolRange;
		float rightBoundary = enemyBehavior.patrolOrigin.x + enemyBehavior.patrolRange;
		
		// set velocity based on patrol direction
		float targetVelocityX = enemyBehavior.patrolForwards ? 
			SPIKE_ENEMY_PATROL_SPEED_PER_MS * 1000 : 
			-SPIKE_ENEMY_PATROL_SPEED_PER_MS * 1000;
			
		enemyMotion.velocity.x = targetVelocityX;
		enemyMotion.velocity.y = 0;

		// check if enemy has not moved since the last frame
		if (enemyBehavior.hasPreviousPosition) {
			float distanceMoved = abs(currentX - enemyBehavior.previousPositionX);
			
			// if almost no movement occurred but we should be moving
			if (distanceMoved < 0.1f && abs(targetVelocityX) > 50.0f) {
				// Wall collision - reverse direction
				enemyBehavior.patrolForwards = !enemyBehavior.patrolForwards;
				
				// Apply immediate velocity change in the opposite direction
				enemyMotion.velocity.x = -enemyMotion.velocity.x;
				
				// debug
				// std::cout << "Wall collision detected! Reversing direction." << std::endl;
			}
		}
		
		// save current position for next frame comparison
		enemyBehavior.previousPositionX = currentX;
		enemyBehavior.hasPreviousPosition = true;
		
		// check normal patrol boundary conditions
		if (enemyMotion.position.x >= rightBoundary) {
			enemyBehavior.patrolForwards = false;
		} else if (enemyMotion.position.x <= leftBoundary) {
			enemyBehavior.patrolForwards = true;
		}
		
		// Switch to dashing if player detected
		if (playerDetected) {
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
			if (dist <= 25.f)
			{
				// if overlapping with player deal damage
                enemyBehavior.bombTimer -= elapsed_ms;
                if (enemyBehavior.bombTimer <= 0)
                {
                    Enemy& enemy = registry.enemies.get(enemyEntity);
                    enemy.health = 0;
                    Player &player = registry.players.get(registry.players.entities[0]);
					damagePlayer(SPIKE_ENEMY_BOMB_DAMAGE);
				}
			}
		}
		else
		{
			// change animation frames and reset patrol state
			changeAnimationFrames(enemyEntity, 0, 6);
			enemyBehavior.patrolOrigin = enemyMotion.position;
			enemyMotion.velocity = {0, 0};
			enemyBehavior.bombTimer = SPIKE_ENEMY_BOMB_TIMER;

			return SpikeEnemyState::PATROLLING;
		}
		break;
	}
	case SpikeEnemyState::KNOCKBACK:
	{
		// apply knockback velocity
		enemyBehavior.knockbackTimer -= elapsed_ms;
		if (enemyBehavior.knockbackTimer <= 0)
		{
			changeAnimationFrames(enemyEntity, 0, 6);
			enemyBehavior.patrolOrigin = enemyMotion.position;
			enemyMotion.velocity = {0, 0};
			return SpikeEnemyState::DASHING;
		}
		else
		{
			// decay knockback velocity
			enemyMotion.velocity *= SPIKE_ENEMY_KNOCKBACK_DECAY;
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
	
			// while patrolling randomly flip direction
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

			// if player is detected, transition to chasing state
			if (playerDetected) {
				enemyBehavior.state = RBCEnemyState::RUNAWAY;
			}

			break;
		}

		case RBCEnemyState::RUNAWAY: {
			if (dist > 0.001f && playerDetected) {
				// run away from character by adjusting direction directly opposite from player
				float new_angle = atan2(direction.y, direction.x);
				new_angle *= (360.f / (2 * M_PI)) + 90.f;
				if (new_angle < 0) {
					new_angle += 360;
				}
				enemyMotion.angle = new_angle;
				enemyMotion.velocity = -direction * ENEMY_SPEED;
			}
			else {
				// if player is no longer detected, transition back to floating state
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
		// if player is detected, transition to chasing state with smooth transition
		if (playerDetected)
		{
			enemyBehavior.state = BacteriophageState::CHASING;
			
			// Start with a half-speed movement for smoother transition
			vec2 moveDirection = glm::normalize(positionToReach - enemyMotion.position);
			enemyMotion.velocity = moveDirection * (ENEMY_SPEED * 0.5f);
		}
		else
		{
			// add slight floating motion even when idleto make it more alive and avoid jitter
			float time = static_cast<float>(glfwGetTime() * 0.5f);
			enemyMotion.velocity.x = sin(time + enemyBehavior.placement_index) * 10.0f;
			enemyMotion.velocity.y = cos(time * 1.3f + enemyBehavior.placement_index) * 10.0f;
		}
		break;
	}
	case BacteriophageState::CHASING:
	{
		// if player is no longer detected, decelerate smoothly
		if (!playerDetected)
		{
			enemyBehavior.state = BacteriophageState::PATROLLING;
			enemyMotion.velocity *= 0.5f; // Gradual slow down instead of abrupt stop
		}
		else
		{
	
			float distanceToTarget = glm::distance(enemyMotion.position, positionToReach);
			
			// mmooth rotation towards target 
			float targetAngle = atan2(directionToPlayer.y, directionToPlayer.x) * (180.0f / M_PI) + 90;
			float currentAngle = enemyMotion.angle;
			
			if (targetAngle - currentAngle > 180.0f) targetAngle -= 360.0f;
			if (targetAngle - currentAngle < -180.0f) targetAngle += 360.0f;
			
			// Interpolate toward plauer
			enemyMotion.angle = glm::mix(currentAngle, targetAngle, 0.1f);
			
			// Ddynamic speed based on distance to target (drifting ishh)
			if (distanceToTarget > 150.0f) {
				// Far away - move at full speed
				vec2 moveDirection = glm::normalize(positionToReach - enemyMotion.position);
				enemyMotion.velocity = moveDirection * ENEMY_SPEED;
			}
			else if (distanceToTarget > 30.0f) {
				// Medium distance - scale speed down proportionally
				float speedFactor = glm::min(distanceToTarget / 150.0f, 1.0f);
				vec2 moveDirection = glm::normalize(positionToReach - enemyMotion.position);
				enemyMotion.velocity = moveDirection * (ENEMY_SPEED * speedFactor);
			}
			else if (distanceToTarget > 5.0f) {
				// Close - move very slowly to reduce jitter
				vec2 moveDirection = glm::normalize(positionToReach - enemyMotion.position);
				enemyMotion.velocity = moveDirection * (distanceToTarget * 3.0f);
			}
			else {
				// SUPERR close - maintain minimal movement >>proprotional to players delta
				vec2 oldPos = enemyMotion.position;
				vec2 idealPos = positionToReach;
				enemyMotion.velocity = (idealPos - oldPos) * 2.0f;
			}
		}
		break;
	}
	}

	return enemyBehavior.state;
}

BossState AISystem::handleBossBehaviour(Entity& enemyEntity, BossAI& enemyBehavior, float dist, vec2 direction, bool playerDetected, float elapsed_ms)
{

	if (!registry.motions.has(enemyEntity) || !registry.enemies.has(enemyEntity)) {
		return BossState::INITIAL;
	}

	Motion &enemyMotion = registry.motions.get(enemyEntity);

	Enemy& enemy = registry.enemies.get(enemyEntity);

	switch (enemyBehavior.state)
	{
		// initaially do noting, if player detected go to any random state and set cool_down
		case BossState::INITIAL:
		{
			if(playerDetected)
			{
				std::vector<BossState> possibleStates = { BossState::SHOOT_PARADE, BossState::RUMBLE };
				BossState next_state = possibleStates[std::rand() % possibleStates.size()];

				enemyBehavior.state = next_state;
				
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
		// basically initial, but with the option of running way when health is low and doesn't need to wait for player to be detected
		case BossState::IDLE: 
		{

			// reset angle to 0
			if (enemyMotion.angle != 0.f) {
				const float smoothing_factor = 0.1f;

				enemyMotion.angle = glm::lerp(enemyMotion.angle, 0.f, smoothing_factor);

				if (std::fabs(enemyMotion.angle) < 0.1f) {
					enemyMotion.angle = 0.f;
				}
			}

			// when cool_down reaches < 0, go to a random state
			enemyBehavior.cool_down -= elapsed_ms;
			if(enemyBehavior.cool_down < 0.f)
			{
				float health_ratio = enemy.health / enemy.total_health;
				bool can_flee = (health_ratio < 0.65f);

				std::vector<BossState> possibleStates = { BossState::SHOOT_PARADE, BossState::RUMBLE };
				if (can_flee)
					possibleStates.push_back(BossState::FLEE);

				BossState next_state = possibleStates[std::rand() % possibleStates.size()];
				enemyBehavior.state = next_state;
				
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
				else if (enemyBehavior.state == BossState::FLEE)
				{
					enemyBehavior.flee_timer = 500.f;
					enemyBehavior.is_fleeing = true;
				}
			}		
			break;
		}

		case BossState::SHOOT_PARADE:
		{
			// shoot for a small duration and go to cool_down
			enemyBehavior.cool_down -= elapsed_ms;
			enemyBehavior.shoot_cool_down -= elapsed_ms;
			
			if (enemyBehavior.shoot_cool_down < 0.f) {
				for (int angleDeg = 0; angleDeg < 360; angleDeg += 30)
				{
					float angleRad = glm::radians((float)angleDeg);
					vec2 dir = { cosf(angleRad), sinf(angleRad) };
					vec2 velocity = dir * PROJECTILE_SPEED * 3.f;
					vec2 spawnPos = enemyMotion.position + dir * enemyMotion.scale.x / 3.f;

					createBossProjectile(spawnPos, enemyBehavior.projectile_size, velocity);
				}

				enemyBehavior.shoot_cool_down = 500.f;	
			}

			if (enemyBehavior.cool_down < 0.f) {
				enemyBehavior.state = BossState::IDLE;
				enemyBehavior.cool_down = 3000.f;
			}	

			break;
		}

		case BossState::RUMBLE:
		{
			// charge dash and rotate towards player to dash
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
					enemyBehavior.cool_down = 3000.f;
					enemyBehavior.rumble_charge_time = 1500.f;
					enemyBehavior.is_charging = true;
				}
			}
			break;
		}

		// run away from player in the opposite direction
		case BossState::FLEE :
		{
			if (enemyBehavior.is_fleeing) {
				enemyBehavior.flee_timer -= elapsed_ms;

				vec2 flee_dir = -glm::normalize(direction);
				enemyMotion.velocity = flee_dir * ENEMY_SPEED * 5.f;

				if (enemyBehavior.flee_timer < 0.f) {
					enemyMotion.velocity = { 0.f, 0.f };
					enemyBehavior.state = BossState::IDLE;
					enemyBehavior.cool_down = 3000.f;
					enemyBehavior.is_fleeing = false;
				}
			}
			break;
		}

		default:
			break;
	}
	
	return enemyBehavior.state;
}

FinalBossState AISystem::handleFinalBossBehaviour(Entity& enemyEntity, FinalBossAI& enemyBehavior, float dist, vec2 direction, bool playerDetected, float elapsed_ms)
{
	Motion& enemyMotion = registry.motions.get(enemyEntity);
	Enemy& enemy = registry.enemies.get(enemyEntity);

	switch (enemyBehavior.state)
	{
		case FinalBossState::INITIAL:
		{
			if (playerDetected)
			{
				enemyBehavior.state = FinalBossState::SPAWN_1;
				enemyBehavior.cool_down = FINAL_BOSS_BASE_COOLDOWN;
			}
			break;
		}

		case FinalBossState::SPAWN_1:
		{
			if (!enemyBehavior.has_spawned) {
				// spawn
				ProceduralMap& map = registry.proceduralMaps.components[0];
				std::vector<std::vector<tileType>> rawMap = map.map;

				int width = rawMap[0].size(); 
				int height = rawMap.size();   

				int contourThickness = 2;
				if (enemyBehavior.phase == 2) contourThickness = 3;
				else if (enemyBehavior.phase == 3) contourThickness = 4;

				for (int row = 0; row < height; ++row) {
					for (int col = 0; col < width; ++col) {
						bool isOnContour = (
							row < contourThickness || row >= height - contourThickness ||
							col < contourThickness || col >= width - contourThickness
						);

						if (isOnContour && rawMap[row][col] == tileType::EMPTY) {
							float spawnChance = 0.4f;
							if (enemyBehavior.phase == 2) spawnChance = 0.6f;
							else if (enemyBehavior.phase == 3) spawnChance = 0.85f;

							if ((float)rand() / RAND_MAX < spawnChance) {
								createDenderite(nullptr, gridCellToPosition({ col, row }));
							}
						}
					}
				}
			
				enemyBehavior.has_spawned = true;
			} else {
				if (registry.denderiteAIs.size() == 0) {
					enemyBehavior.state = FinalBossState::SPIRAL_SHOOT_1; // Just shooting
					enemyBehavior.shoot_cool_down = FINAL_BOSS_BASE_SHOOT_COOLDOWN;
					enemyBehavior.cool_down = FINAL_BOSS_BASE_COOLDOWN;
					enemyBehavior.has_spawned = false;
				}
			}
			break;
		}

		case FinalBossState::SPIRAL_SHOOT_1: {
			enemyBehavior.spiral_duration -= elapsed_ms;
			enemyBehavior.shoot_cool_down -= elapsed_ms;

			if (enemyBehavior.shoot_cool_down <= 0.f) {

				if (enemyBehavior.phase == 1 || enemyBehavior.phase == 2) {				
					int angleStep = 30;
					int totalBullets = 360 / angleStep;
					float baseAngle = static_cast<float>(rand() % 360) + static_cast<float>(rand() % 45); // randomized patterns
					
					for (int i = 0; i < totalBullets; ++i)
					{
						float angleDeg = baseAngle + i * angleStep;
						float angleRad = glm::radians(angleDeg);
					
						vec2 dir = { cosf(angleRad), sinf(angleRad) };
						vec2 velocity = dir * PROJECTILE_SPEED * 2.f;

						Motion& refreshedMotion = registry.motions.get(enemyEntity); 
						vec2 spawnPos = refreshedMotion.position + dir * refreshedMotion.scale.x / 3.f;

						createFinalBossProjectile(spawnPos, FINAL_BOSS_PROJECTILE, velocity, enemyBehavior.phase);
					
						if (enemyBehavior.phase == 2)
							Entity spiralProjectile = createFinalBossProjectile(spawnPos, FINAL_BOSS_PROJECTILE, velocity, enemyBehavior.phase);
						enemyBehavior.shoot_cool_down = FINAL_BOSS_BASE_SHOOT_COOLDOWN;
					}
				} else {
					Motion& refreshedMotion = registry.motions.get(enemyEntity); 
					vec2 dir = direction;
					vec2 velocity = dir * PROJECTILE_SPEED * 0.5f;

					Entity eyeBallProjectile_1 = createFinalBossProjectile(refreshedMotion.position - 50.f, FINAL_BOSS_PROJECTILE, velocity, 3);
					Entity eyeBallProjectile_2 = createFinalBossProjectile(refreshedMotion.position + 50.f, FINAL_BOSS_PROJECTILE, velocity, 3);
					enemyBehavior.shoot_cool_down = FINAL_BOSS_EYEBALL_SHOOT_COOLDOWN;
				}
			}

			if (enemyBehavior.spiral_duration <= 0.f) {
				enemyBehavior.state = FinalBossState::TIRED;
				changeAnimationFrames(enemyEntity, 1, 8);

				enemyBehavior.spiral_duration = FINAL_BOSS_SHOOT_DURATION;
				enemyBehavior.shoot_cool_down = FINAL_BOSS_BASE_SHOOT_COOLDOWN;
				enemyBehavior.cool_down = FINAL_BOSS_TIRED_COOLDOWN;
			}

			break;
		}

		case FinalBossState::TIRED: {

			if (enemy.health <= 2/3.f * enemy.total_health && enemyBehavior.phase == 1) {
				enemyBehavior.phase = 2;
				enemyBehavior.state = FinalBossState::SPAWN_1;
				enemyBehavior.cool_down = FINAL_BOSS_BASE_COOLDOWN;
				changeAnimationFrames(enemyEntity, 9, 11);
			} else if (enemy.health <= 1/3.f * enemy.total_health && enemyBehavior.phase == 2) {
				enemyBehavior.phase = 3;
				enemyBehavior.state = FinalBossState::SPAWN_1;
				enemyBehavior.cool_down = FINAL_BOSS_BASE_COOLDOWN;
				changeAnimationFrames(enemyEntity, 9, 11);
			} else {
				enemyBehavior.cool_down -= elapsed_ms;
				if (enemyBehavior.cool_down <= 0.f) {
					enemyBehavior.state = FinalBossState::SPAWN_1;
					enemyBehavior.shoot_cool_down = FINAL_BOSS_BASE_SHOOT_COOLDOWN;
					enemyBehavior.cool_down = FINAL_BOSS_BASE_COOLDOWN;
					changeAnimationFrames(enemyEntity, 9, 11);
				}
			}
			break;
		} 
	}

	return enemyBehavior.state;
}

DenderiteState AISystem::handleDenderiteBehavior(Entity& enemyEntity, DenderiteAI& enemyBehavior, float dist, vec2 direction, bool playerDetected, float elapsed_ms)
{
	Motion& enemyMotion = registry.motions.get(enemyEntity);
	Enemy& enemy = registry.enemies.get(enemyEntity);

	switch (enemyBehavior.state) {

		case DenderiteState::HUNT:
		{
			if (playerDetected) {
				enemyBehavior.state = DenderiteState::PIERCE;
				enemyBehavior.chargeTime = 2000.f;
				enemyBehavior.isCharging = true;
			}
			break;
		}

		case DenderiteState::PIERCE:
		{
			const float pierceSpeed = 1500.f;

			if (enemyBehavior.isCharging) {
				enemyBehavior.chargeTime -= elapsed_ms;
				enemyMotion.velocity = vec2(0.f);

				enemyMotion.angle = atan2(direction.y, direction.x) * (180.f / M_PI) + 90.f;

				if (enemyBehavior.chargeTime <= 0.f) {
					vec2 locked_direction = glm::normalize(direction);
					enemyMotion.velocity = locked_direction * pierceSpeed;
					
					enemyBehavior.isCharging = false;
					enemyBehavior.chargeTime = 0.f;
					enemyBehavior.shootCoolDown = 1000.f;
				}
			} else {
				enemyBehavior.chargeDuration -= elapsed_ms;
				if (enemyBehavior.chargeDuration <= 0.f) {
					enemyMotion.velocity = { 0.f, 0.f };
					enemyBehavior.state = DenderiteState::SHOOT;
				}
			}
			break;
		}

		case DenderiteState::SHOOT:
		{
			if (enemyMotion.angle != 0.f) {
				const float smoothing_factor = 0.1f;

				enemyMotion.angle = glm::lerp(enemyMotion.angle, 0.f, smoothing_factor);

				if (std::fabs(enemyMotion.angle) < 0.1f) {
					enemyMotion.angle = 0.f;
				}
			}

			enemyMotion.velocity = {0.f, 0.f};

			enemyBehavior.shootCoolDown -= elapsed_ms;
			if (enemyBehavior.shootCoolDown <= 0.f) {
				createProjectile(enemyMotion.position, {PROJECTILE_SIZE, PROJECTILE_SIZE}, direction * DENDERITE_PROJECTILE_SPEED, PROJECTILE_DAMAGE);
				enemyBehavior.shootCoolDown = 1000.0f; 
			}
			break;
		}
	}

	return enemyBehavior.state;
}

// handle AI behavior for all enemies with according parameters
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

	for (auto& enemyEntity : registry.finalBossAIs.entities)
	{
		FinalBossAI& enemyBehavior = registry.finalBossAIs.get(enemyEntity);
		Motion& enemyMotion = registry.motions.get(enemyEntity);

		vec2 direction;
		float dist = 0;
		bool playerDetected = isPlayerInRadius(playerMotion.position, enemyMotion.position, dist, direction, enemyBehavior.detectionRadius);

		enemyBehavior.state = handleFinalBossBehaviour(enemyEntity, enemyBehavior, dist, direction, playerDetected, elapsed_ms);
	}

	for (auto& enemyEntity : registry.denderiteAIs.entities)
	{
		DenderiteAI& enemyBehavior = registry.denderiteAIs.get(enemyEntity);
		Motion& enemyMotion = registry.motions.get(enemyEntity);

		vec2 direction;
		float dist = 0;
		bool playerDetected = isPlayerInRadius(playerMotion.position, enemyMotion.position, dist, direction, enemyBehavior.detectionRadius);

		enemyBehavior.state = handleDenderiteBehavior(enemyEntity, enemyBehavior, dist, direction, playerDetected, elapsed_ms);
	}
}
