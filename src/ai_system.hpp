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
};