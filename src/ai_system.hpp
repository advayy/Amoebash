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

	SpikeEnemyState handleDefaultEnemyBehavior(Entity& enemyEntity, EnemyAI& enemyBehavior, int state, vec2 direction, float dist, bool playerDetected, float elapsed_ms);
};