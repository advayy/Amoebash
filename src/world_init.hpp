#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"

Entity createEnemy(RenderSystem* renderer, vec2 position);
Entity createSpikeEnemy(RenderSystem* renderer, vec2 position);
Entity createBacteriophage(RenderSystem* renderer, vec2 position, int placement_index);
Entity createPlayer(RenderSystem* renderer, vec2 position);
void createButtons();

// projectile
Entity createProjectile(vec2 pos, vec2 size, vec2 velocity);
Entity createBacteriophageProjectile(Entity& bacteriophage);

// debugging red lines
Entity createLine(vec2 position, vec2 size);

// Animation
void toggleDashAnimation(Entity entity, bool is_dashing);

void initiatePlayerDash();
bool canDash();
bool isDashing();

Entity createCamera();

Entity createMap(RenderSystem* renderer, vec2 size);
Entity createMiniMap(RenderSystem* renderer, vec2 size);

void tileMap();
Entity addTile(vec2 gridCoord);
void removeTile(vec2 gridCoord);
Entity addWallTile(vec2 gridCoord);

vec2 positionToGridCell(vec2 position);
vec2 gridCellToPosition(vec2 gridCell);

Entity createStartScreen(vec2 position = {-WINDOW_WIDTH_PX / 2.f, 0.f});
Entity createShopScreen();
Entity createInfoScreen();
Entity createGameOverScreen();
Entity createPauseScreen();

void createGameplayCutScene();
Entity createNose();
Entity createCutSceneBackGround();
Entity createNoseAccent();
Entity createEnteringNucleus();

void removePauseScreen();
void removeGameOverScreen();
void removeStartScreen();
void removeCutScene();

Entity createButton(ButtonType type, vec2 position, vec2 scale, TEXTURE_ASSET_ID texture);
Entity createStartButton();
Entity createShopButton();
Entity createInfoButton();
