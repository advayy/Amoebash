#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"

Entity createEnemy(RenderSystem* renderer, vec2 position);
Entity createPlayer(RenderSystem* renderer, vec2 position);

// projectile
Entity createProjectile(vec2 pos, vec2 size, vec2 velocity);

// grid lines to show tile positions
Entity createGridLine(vec2 start_pos, vec2 end_pos);

// debugging red lines
Entity createLine(vec2 position, vec2 size);

// Animation
void animation(float elapsed_ms);

void InitiatePlayerDash();
bool canDash();
bool isDashing();