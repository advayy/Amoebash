#pragma once

#include <map>

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"

void initializeProgression();

Entity createEnemy(RenderSystem* renderer, vec2 position);
Entity createSpikeEnemy(RenderSystem* renderer, vec2 position);
Entity createRBCEnemy(RenderSystem* renderer, vec2 position);
Entity createBacteriophage(RenderSystem* renderer, vec2 position, int placement_index);
Entity createDenderite(RenderSystem* renderer, vec2 position, std::vector<ivec2> &path);
Entity createBoss(RenderSystem* renderer, vec2 position, BossState state = BossState::INITIAL, int bossStage = 0);
Entity createFinalBoss(RenderSystem* renderer, vec2 position);

Entity createPlayer(RenderSystem *renderer, vec2 position);
Entity createGun(RenderSystem *renderer, vec2 position);
Entity createKey(RenderSystem *renderer, vec2 position);
Entity createChest(RenderSystem *renderer, vec2 position);

// projectile
Entity createProjectile(vec2 pos, vec2 size, vec2 velocity, float damage = PROJECTILE_DAMAGE);
Entity createBacteriophageProjectile(Entity& bacteriophage);
Entity createBossProjectile(vec2 position, vec2 size, vec2 velocity);
Entity createFinalBossProjectile(vec2 position, vec2 size, vec2 velocity, int phase);
Entity createBossArrow(Entity Boss);

Entity createCamera();

Entity createProceduralMap(RenderSystem* renderer, vec2 size, bool tutorial_on, std::pair<int, int>& playerPosition);
Entity createBossMap(RenderSystem* renderer, vec2 size, std::pair<int, int>& playerPosition);
Entity createFinalBossMap(RenderSystem* renderer, vec2 size, std::pair<int, int>& playerPosition);

Entity addTile(vec2 gridCoord, TEXTURE_ASSET_ID texture_id, int total_frames);
Entity addParalaxTile(vec2 gridCoord);
Entity addWallTile(vec2 gridCoord);
Entity addPortalTile(vec2 gridCoord);

int countAdjacentWalls(const std::vector<int>& grid, int x, int y);
std::vector<std::vector<tileType>> applyCellularAutomataRules(const std::vector<std::vector<tileType>>& grid);
std::pair<int, int> getRandomEmptyTile(const std::vector<std::vector<tileType>>& grid);
int getDistance(const std::vector<std::vector<tileType>>& grid, std::pair<int,int> start, std::pair<int,int> end);

Entity createBuff(vec2 position);

void updateMiniMap(vec2 playerPos);
void emptyMiniMap();
    