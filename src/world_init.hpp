#pragma once

#include <map>

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"

Entity createEnemy(RenderSystem* renderer, vec2 position);
Entity createSpikeEnemy(RenderSystem* renderer, vec2 position);
Entity createRBCEnemy(RenderSystem* renderer, vec2 position);
Entity createBacteriophage(RenderSystem* renderer, vec2 position, int placement_index);
Entity createPlayer(RenderSystem *renderer, vec2 position);
Entity createKey(RenderSystem *renderer, vec2 position);
Entity createChest(RenderSystem *renderer, vec2 position);
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
bool willMeshCollideSoon(const Entity& player, const Entity& hexagon, float predictionTime);
bool pointInPolygon(const vec2& point, const std::vector<vec2> &polygon);
std::vector<vec2> getWorldVertices(const std::vector<TexturedVertex>& vertices, const vec2 &position, const vec2 &scale);


Entity createCamera();

Entity createProceduralMap(RenderSystem* renderer, vec2 size, bool tutorial_on, std::pair<int, int>& playerPosition);
void tileProceduralMap();

Entity createMiniMap(RenderSystem *renderer, vec2 size);

Entity addTile(vec2 gridCoord, TEXTURE_ASSET_ID texture_id, int total_frames);
Entity addParalaxTile(vec2 gridCoord);
Entity addWallTile(vec2 gridCoord);
Entity addPortalTile(vec2 gridCoord);

vec2 positionToGridCell(vec2 position);
vec2 gridCellToPosition(vec2 gridCell);

Entity createStartScreen(vec2 position = LOGO_POSITION_INITIAL);
Entity createShopScreen();
Entity createInfoScreen();
Entity createGameOverScreen();
Entity createPauseScreen();
void createInfoBoxes();

void createGameplayCutScene();
Entity createNose();
Entity createCutSceneBackGround();
Entity createNoseAccent();
Entity createEnteringNucleus();

void removePauseScreen();
void removeGameOverScreen();
void removeStartScreen();
void removeShopScreen();
void removeInfoScreen();
void removeCutScene();
void removeInfoBoxes();

Entity createButton(ButtonType type, vec2 position, vec2 scale, TEXTURE_ASSET_ID texture);
Entity createStartButton();
Entity createShopButton();
Entity createInfoButton();
Entity createBackButton();

int countAdjacentWalls(const std::vector<int>& grid, int x, int y);
std::vector<std::vector<tileType>> applyCellularAutomataRules(const std::vector<std::vector<tileType>>& grid);
std::pair<int, int> getRandomEmptyTile(const std::vector<std::vector<tileType>>& grid);
int getDistance(const std::vector<std::vector<tileType>>& grid, std::pair<int,int> start, std::pair<int,int> end);

Entity createUIElement(vec2 position, vec2 scale, TEXTURE_ASSET_ID texture_id, EFFECT_ASSET_ID effect_id);
Entity createHealthBar();
void createDashRecharge();
Entity createBuff(vec2 position);
Entity createBuffUI(vec2 position, int buffType);
void renderCollectedBuff(RenderSystem *renderer, int buffType);
