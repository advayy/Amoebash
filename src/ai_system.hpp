#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include "tinyECS/registry.hpp"

class AISystem
{
public:
	void step(float elapsed_ms);

private:
	bool isPlayerInRadius(vec2 player, vec2 enemy, float& distance, vec2& direction, float detectionRadius);

	SpikeEnemyState handleSpikeEnemyBehavior(Entity& enemyEntity, SpikeEnemyAI& enemyBehavior, float dist, vec2 direction, bool playerDetected, float elapsed_ms);
	RBCEnemyState handleRBCBehavior(Entity& enemyEntity, RBCEnemyAI& enemyBehavior, float dist, vec2 direction, bool playerDetected, float elapsed_ms);
	BacteriophageState handleBacteriophageBehavior(Entity& enemyEntity, BacteriophageAI& enemyBehavior, vec2 direction, bool playerDetected, float elapsed_ms, vec2 positionToReach, vec2 directionToPlayer);
	BossState handleBossBehaviour(Entity& enemyEntity, BossAI& enemyBehavior, float dist, vec2 direction, bool playerDetected, float elapsed_ms);
};