#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include <iostream>
#include <random>
#include <ctime>
#include <queue>
#include "animation_system.hpp"
#include "ui_system.hpp"

void initializeProgression(){
	auto entity = Entity();
	Progression& progressionState = registry.progressions.emplace(entity);
}

Entity createEnemy(RenderSystem* renderer, vec2 position)
{
	// reserve an entity
	auto entity = Entity();

	// invader
	Enemy& enemy = registry.enemies.emplace(entity);
	enemy.health = ENEMY_HEALTH;
	enemy.total_health = ENEMY_HEALTH;

	// store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// TODO A1: initialize the position, scale, and physics components
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 }; // FLAG
	motion.position = position;

	// resize, set scale to negative if you want to make it face the opposite way
	// motion.scale = vec2({ -INVADER_BB_WIDTH, INVADER_BB_WIDTH });
	motion.scale = vec2({ ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT });

	return entity;
}

Entity createRBCEnemy(RenderSystem* renderer, vec2 position)
{
	Entity entity = createEnemy(renderer, position);
	
	Entity hp_bar = createEnemyHPBar(entity, TEXTURE_ASSET_ID::ENEMY_HP_BAR);

	RBCEnemyAI& enemy_ai = registry.rbcEnemyAIs.emplace(entity);
	enemy_ai.patrolOrigin = position;
	enemy_ai.state = RBCEnemyState::FLOATING;

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::RBC_ENEMY,
			EFFECT_ASSET_ID::SPRITE_SHEET,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	Animation& a = registry.animations.emplace(entity);
	a.start_frame = 0;
	a.end_frame = 13;
	a.time_per_frame = 100.0f;
	a.loop = ANIM_LOOP_TYPES::LOOP;

	SpriteSheetImage& spriteSheet = registry.spriteSheetImages.emplace(entity);
	spriteSheet.total_frames = 14;
	spriteSheet.current_frame = 0;

	SpriteSize& sprite = registry.spritesSizes.emplace(entity);
	sprite.width = 32;
	sprite.height = 32;

	registry.motions.get(entity).velocity = {0.0f, -ENEMY_SPEED / 4};

	return entity;
}

Entity createSpikeEnemy(RenderSystem* renderer, vec2 position)
{
	Entity entity = createEnemy(renderer, position);

	Entity hp_bar = createEnemyHPBar(entity, TEXTURE_ASSET_ID::ENEMY_HP_BAR);

	SpikeEnemyAI& enemy_ai = registry.spikeEnemyAIs.emplace(entity);
	enemy_ai.patrolOrigin = position;
	enemy_ai.state = SpikeEnemyState::PATROLLING;

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::SPIKE_ENEMY,
			EFFECT_ASSET_ID::SPRITE_SHEET,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	Animation& a = registry.animations.emplace(entity);
	a.start_frame = 0;
	a.end_frame = 6;
	a.time_per_frame = 100.0f;
	a.loop = ANIM_LOOP_TYPES::PING_PONG;

	SpriteSheetImage& spriteSheet = registry.spriteSheetImages.emplace(entity);
	spriteSheet.total_frames = 13;
	spriteSheet.current_frame = 0;

	SpriteSize& sprite = registry.spritesSizes.emplace(entity);
	sprite.width = 32;
	sprite.height = 32;

	return entity;
}

Entity createBacteriophage(RenderSystem* renderer, vec2 position, int placement_index)
{
	Entity entity = createEnemy(renderer, position);
	
	Entity hp_bar = createEnemyHPBar(entity, TEXTURE_ASSET_ID::ENEMY_HP_BAR);

	BacteriophageAI& enemy_ai = registry.bacteriophageAIs.emplace(entity);
	enemy_ai.placement_index = placement_index;

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::BACTERIOPHAGE,
			EFFECT_ASSET_ID::SPRITE_SHEET,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	Animation& a = registry.animations.emplace(entity);
	a.start_frame = 0;
	a.end_frame = 9;
	a.time_per_frame = 100.0f;
	a.loop = ANIM_LOOP_TYPES::PING_PONG;

	SpriteSheetImage& spriteSheet = registry.spriteSheetImages.emplace(entity);
	spriteSheet.total_frames = 9;
	spriteSheet.current_frame = 0;

	SpriteSize& sprite = registry.spritesSizes.emplace(entity);
	sprite.width = 32;
	sprite.height = 32;

	return entity;
}

Entity createDenderite(RenderSystem* renderer, vec2 position)
{
	Entity entity = createEnemy(renderer, position);

	Entity hp_bar = createEnemyHPBar(entity, TEXTURE_ASSET_ID::ENEMY_HP_BAR);

	DenderiteAI& enemy_ai = registry.denderiteAIs.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::DENDERITE,
			EFFECT_ASSET_ID::SPRITE_SHEET,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	Motion& motion = registry.motions.get(entity);
	motion.scale = DENDERITE_SIZE;

	Animation& a = registry.animations.emplace(entity);
	a.start_frame = 0;
	a.end_frame = 6;
	a.time_per_frame = 100.0f;
	a.loop = ANIM_LOOP_TYPES::PING_PONG;

	SpriteSheetImage& spriteSheet = registry.spriteSheetImages.emplace(entity);
	spriteSheet.total_frames = 6;
	spriteSheet.current_frame = 0;

	SpriteSize& sprite = registry.spritesSizes.emplace(entity);
	sprite.width = 64;
	sprite.height = 64;

	return entity;
}

Entity createBoss(RenderSystem* renderer, vec2 position, BossState state, int bossStage)
{
	Entity entity = createEnemy(renderer, position);

	Motion& motion = registry.motions.get(entity);
	motion.scale = {BOSS_BB_WIDTH, BOSS_BB_HEIGHT};

	BossAI& enemy_ai = registry.bossAIs.emplace(entity);
	enemy_ai.state = state;
	enemy_ai.cool_down = 2000.f;
	enemy_ai.detectionRadius = BOSS_DETECTION_RADIUS;
	enemy_ai.projectile_size = BOSS_PROJECTILE;
	enemy_ai.stage = bossStage;

	Enemy& enemy = registry.enemies.get(entity);
	enemy.health = BOSS_HEALTH;
	enemy.total_health = BOSS_HEALTH;


	if (bossStage > 0) {
		motion.scale /= (2 * bossStage);
		enemy.health /= 2 * bossStage;
		enemy.total_health /= 2 * bossStage;
		enemy_ai.projectile_size /= 2 * bossStage;
		enemy_ai.detectionRadius = enemy_ai.detectionRadius * std::pow(0.75f, bossStage);
	}

	if (bossStage == 0) {
		Entity hp_bar = createEnemyHPBar(entity, TEXTURE_ASSET_ID::MITOSIS_BOSS_128_HP_BAR);
	} else {
		Entity hp_bar = createEnemyHPBar(entity, TEXTURE_ASSET_ID::MITOSIS_BOSS_16_HP_BAR);
	}

	TEXTURE_ASSET_ID texture = static_cast<TEXTURE_ASSET_ID>(static_cast<int>(TEXTURE_ASSET_ID::BOSS_STAGE_1) + bossStage);

	registry.renderRequests.insert(
		entity,
		{
			texture,
			EFFECT_ASSET_ID::SPRITE_SHEET,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	Animation& a = registry.animations.emplace(entity);
	a.start_frame = 0;
	a.end_frame = bossStage == 0 ? 7 : (bossStage == 1 ? 9 : 8);
	a.time_per_frame = 100.0f;
	a.loop = (bossStage <= 1) ? ANIM_LOOP_TYPES::LOOP : ANIM_LOOP_TYPES::PING_PONG;

	SpriteSheetImage& spriteSheet = registry.spriteSheetImages.emplace(entity);
	spriteSheet.total_frames = bossStage == 0 ? 7 : (bossStage == 1 ? 9 : 8);
	spriteSheet.current_frame = 0;

	SpriteSize& sprite = registry.spritesSizes.emplace(entity);
	sprite.width = motion.scale.x;
	sprite.height = motion.scale.y;

	enemy_ai.associatedArrow = createBossArrow(entity);

	return entity;
}

Entity createBossArrow(Entity Boss) {
	Entity arrow = Entity();

	BossArrow& bossArrow = registry.bossArrows.emplace(arrow);
	bossArrow.associatedBoss = Boss;
	registry.renderRequests.insert(
		arrow,
		{
			TEXTURE_ASSET_ID::BOSS_ARROW,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return arrow;
}

Entity createPlayer(RenderSystem *renderer, vec2 position)
{
	auto entity = Entity();

	// new tower
	auto &p = registry.players.emplace(entity);

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.0f; // A1-TD: CK: rotate to the left 180 degrees to fix orientation
	motion.velocity = {0.0f, 0.0f};
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({PLAYER_BB_WIDTH, PLAYER_BB_HEIGHT});

	// create an (empty) Tower component to be able to refer to all towers
	registry.deadlys.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::PLAYER,
		 EFFECT_ASSET_ID::SPRITE_SHEET,
		 GEOMETRY_BUFFER_ID::SPRITE});

	SpriteSheetImage &spriteSheet = registry.spriteSheetImages.emplace(entity);
	spriteSheet.total_frames = total_player_frames;

	Animation &a = registry.animations.emplace(entity);
	a.time_per_frame = MS_PER_S / total_player_frames;
	toggleDashAnimation(entity, false);

	SpriteSize &sprite = registry.spritesSizes.emplace(entity);
	sprite.width = 32;
	sprite.height = 32;

    createGun(renderer, position);

	return entity;
}

Entity 	createGunCooldown() {
	Entity e = Entity();	
	// camera position at window width and height - 50 each?
	vec2 pos = WEAPON_PILL_UI_POS;
	
	pos.x += WEAPON_PILL_UI_WIDTH/4;

	registry.renderRequests.insert(e,
		{
			TEXTURE_ASSET_ID::GUN_PROJECTILE,
			EFFECT_ASSET_ID::WEAPON_COOLDOWN_INDICATOR,
			GEOMETRY_BUFFER_ID::SPRITE
		});


	Motion& m = registry.motions.emplace(e);
	m.position = {pos.x, pos.y};
	m.scale = {32, 32};

	UIElement& u = registry.uiElements.emplace(e);
	u.position = {pos.x, pos.y};
	u.scale = {32, 32};
	return e;
}

Entity createGun(RenderSystem *renderer, vec2 position) {
    auto entity = Entity();

    registry.guns.emplace(entity);

    // Initialize the motion
    auto &motion = registry.motions.emplace(entity);
    motion.angle = 0.0f;
    motion.velocity = {0.0f, 0.0f};
    motion.position = position;

    motion.scale = vec2({GUN_SIZE, GUN_SIZE});

    registry.renderRequests.insert(
        entity,
        {TEXTURE_ASSET_ID::GUN,
         EFFECT_ASSET_ID::SPRITE_SHEET,
         GEOMETRY_BUFFER_ID::SPRITE
        }
    );

	Animation& a = registry.animations.emplace(entity);
	a.start_frame = 0;
	a.end_frame = 9;
	a.time_per_frame = 100.0f;
	a.loop = ANIM_LOOP_TYPES::LOOP;

	SpriteSheetImage& spriteSheet = registry.spriteSheetImages.emplace(entity);
	spriteSheet.total_frames = 9;
	spriteSheet.current_frame = 0;

	SpriteSize& sprite = registry.spritesSizes.emplace(entity);
	sprite.width = motion.scale.x;
	sprite.height = motion.scale.y;


    return entity;
}

Entity createProjectile(vec2 pos, vec2 size, vec2 velocity, float damage)
{
	auto entity = Entity();
	auto &p = registry.projectiles.emplace(entity);
	p.damage = damage;

	// Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	// registry.meshPtrs.emplace(entity, &mesh);

	// Create motion
	Motion &motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = velocity; // velocity;
	motion.position = pos;
	motion.scale = size;

	// registry.debugComponents.emplace(entity); // Causes it to not run kinda?
	registry.deadlys.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::PROJECTILE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	return entity;
}

Entity createBacteriophageProjectile(Entity& bacteriophage)
{
	Motion& motion = registry.motions.get(bacteriophage);
	vec2 direction = vec2(cosf((motion.angle - 90) * (M_PI / 180)), sinf((motion.angle - 90) * (M_PI / 180)));
	vec2 projectile_pos = motion.position + (motion.scale * direction);
	vec2 projectile_velocity = direction * PROJECTILE_SPEED;
	Entity projectile = createProjectile(projectile_pos, { PROJECTILE_BB_WIDTH, PROJECTILE_BB_HEIGHT }, projectile_velocity, PROJECTILE_DAMAGE);;
	registry.bacteriophageProjectiles.emplace(projectile);
	return projectile;
}

Entity createBossProjectile(vec2 position, vec2 size, vec2 velocity)
{
	Entity projectile = createProjectile(position, size, velocity);
	RenderRequest& render_request = registry.renderRequests.get(projectile);
	render_request.used_texture = TEXTURE_ASSET_ID::BOSS_PROJECTILE;
	Projectile& p = registry.projectiles.get(projectile);
	p.damage = BOSS_PROJECTILE_DAMAGE;
	registry.bossProjectiles.emplace(projectile);

	return projectile;
}

Entity createFinalBossProjectile(vec2 position, vec2 size, vec2 velocity, int phase)
{
	Entity projectile = createProjectile(position, size, velocity);
	RenderRequest& render_request = registry.renderRequests.get(projectile);
	render_request.used_texture = TEXTURE_ASSET_ID::BOSS_PROJECTILE;
	Projectile& p = registry.projectiles.get(projectile);
	p.damage = BOSS_PROJECTILE_DAMAGE;
	p.ms_until_despawn = 7500.f;

	registry.finalBossProjectiles.emplace(projectile);

	if (phase == 2) {
		registry.spiralProjectiles.emplace(projectile);
	} else if (phase == 3) {
		p.ms_until_despawn = 15000.f;
		Motion& motion = registry.motions.get(projectile);
		motion.velocity *= 2.f;

		registry.followingProjectiles.emplace(projectile);
		render_request.used_texture = TEXTURE_ASSET_ID::EYE_BALL_PROJECTILE;
		render_request.used_effect = EFFECT_ASSET_ID::SPRITE_SHEET;

		Animation& a = registry.animations.emplace(projectile);
		a.start_frame = 0;
		a.end_frame = 8;
		a.time_per_frame = 100.0f;
		a.loop = ANIM_LOOP_TYPES::LOOP;

		SpriteSheetImage& spriteSheet = registry.spriteSheetImages.emplace(projectile);
		spriteSheet.total_frames = 9;
		spriteSheet.current_frame = 0;

		SpriteSize& sprite = registry.spritesSizes.emplace(projectile);
		sprite.width = 32.f;
		sprite.height = 32.f;
	}

	return projectile;
}

Entity createBossMap(RenderSystem* renderer, vec2 size, std::pair<int, int>& playerPosition) {
	for (Entity& entity : registry.proceduralMaps.entities) {
        registry.remove_all_components_of(entity);
    }
    for (Entity& entity : registry.portals.entities) {
        registry.remove_all_components_of(entity);
    }

	auto entity = Entity();
	ProceduralMap& map = registry.proceduralMaps.emplace(entity);

	map.width = size.x;
	map.height = size.y;
	map.top = floor(WORLD_ORIGIN.y - size.y / 2);
	map.left = floor(WORLD_ORIGIN.x - size.x / 2);
	map.bottom = ceil(WORLD_ORIGIN.y + size.y / 2);
	map.right = ceil(WORLD_ORIGIN.x + size.x / 2);
	map.map.resize(map.height, std::vector<tileType>(map.width, tileType::EMPTY));

	for (int x = 0; x < map.width; x ++) {
		for (int y = 0; y < map.height; y++) {
			if (x == 0 || y == 0 || x == map.width - 1 || y == map.height - 1) {
				map.map[y][x] = tileType::WALL;
			} else {
				map.map[y][x] = tileType::EMPTY;
			}
		}
	}

	playerPosition.first = 18;
	playerPosition.second = 10;

    createBuff(gridCellToPosition({10,17}), INFO_BOSS1);

	return entity;
}

Entity createFinalBoss(RenderSystem* renderer, vec2 position) {
	Entity entity = createEnemy(renderer, position);

	Motion & motion = registry.motions.get(entity);
	motion.scale = {FINAL_BOSS_BB_WIDTH, FINAL_BOSS_BB_HEIGHT};
	
	FinalBossAI& enemy_ai = registry.finalBossAIs.emplace(entity);
	enemy_ai.detectionRadius = BOSS_DETECTION_RADIUS;
	
	Entity hp_bar = createEnemyHPBar(entity, TEXTURE_ASSET_ID::MITOSIS_BOSS_128_HP_BAR);
	if (registry.motions.has(hp_bar)) {
		Motion& hp_bar_motion = registry.motions.get(hp_bar);
		hp_bar_motion.position += vec2(0.f, + motion.scale.y);
	}

	Enemy& enemy = registry.enemies.get(entity);
	enemy.health = FINAL_BOSS_HEALTH;
	enemy.total_health = FINAL_BOSS_HEALTH;

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::FINAL_BOSS,
			EFFECT_ASSET_ID::SPRITE_SHEET,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	Animation& a = registry.animations.emplace(entity);
	a.start_frame = 8;
	a.end_frame = 10;
	a.time_per_frame = 100.0f;
	a.loop = ANIM_LOOP_TYPES::LOOP;

	SpriteSheetImage& spriteSheet = registry.spriteSheetImages.emplace(entity);
	spriteSheet.total_frames = 14;
	spriteSheet.current_frame = 0;

	SpriteSize& sprite = registry.spritesSizes.emplace(entity);
	sprite.width = motion.scale.x;
	sprite.height = motion.scale.y;

	enemy_ai.associatedArrow = createBossArrow(entity);
	return entity;
}

Entity createFinalBossMap(RenderSystem* renderer, vec2 size, std::pair<int, int>& playerPosition) {
	for (Entity& entity : registry.proceduralMaps.entities) {
		registry.remove_all_components_of(entity);
	}
	for (Entity& entity : registry.portals.entities) {
		registry.remove_all_components_of(entity);
	}
	auto entity = Entity();

	ProceduralMap& map = registry.proceduralMaps.emplace(entity);
	
	map.width = size.x;
	map.height = size.y;
	map.top = floor(WORLD_ORIGIN.y - size.y / 2);
	map.left = floor(WORLD_ORIGIN.x - size.x / 2);
	map.bottom = ceil(WORLD_ORIGIN.y + size.y / 2);
	map.right = ceil(WORLD_ORIGIN.x + size.x / 2);
	
	map.map.resize(map.height, std::vector<tileType>(map.width, tileType::EMPTY));
	
	std::vector<std::vector<int>> rawMap = {
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1},
		{1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
		{1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
		{1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
		{1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1},
		{1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
	};

	for (int i = 0; i < rawMap.size(); i++) {
		for (int j = 0; j < rawMap[i].size(); j++) {
			if (rawMap[i][j] == 1) {
				map.map[i][j] = tileType::WALL;
			} else {
				map.map[i][j] = tileType::EMPTY;
			}
		}
	}
	
	playerPosition.first = 18;
	playerPosition.second = 9;

    createBuff(gridCellToPosition({9,17}), INFO_BOSS2);

	return entity;
}


void updateMiniMap(vec2 playerPos) {

	float minimapViewRange = registry.players.get(registry.players.entities[0]).minimapViewRange;
	Entity e = registry.miniMaps.entities[0];
	MiniMap& m = registry.miniMaps.get(e);

	int top = std::max(0.f, playerPos.y - minimapViewRange);
	int bottom = std::min(20.f, playerPos.y + minimapViewRange);
	int left = std::max(0.f, playerPos.x - minimapViewRange);
	int right = std::min(20.f, playerPos.x + minimapViewRange);

	for(int i = left; i < right; i++) {
		for(int j = top; j < bottom; j++) {
			m.visited[i][j] = 1;
		}
	}
}

void emptyMiniMap() {
	MiniMap& m = registry.miniMaps.get(registry.miniMaps.entities[0]);
	for (int y = 0; y < MAP_HEIGHT; y++) {
		for(int x = 0; x< MAP_WIDTH; x++) {
			m.visited[y][x] = 0;
		}
	}
}

Entity createProceduralMap(RenderSystem* renderer, vec2 size, bool tutorial_on, std::pair<int, int>& playerPosition) {
    // print entering map
    // std::cout << "Entering createProceduralMap" << std::endl;

    for (Entity& entity : registry.proceduralMaps.entities) {
        registry.remove_all_components_of(entity);
    }
    for (Entity& entity : registry.portals.entities) {
        registry.remove_all_components_of(entity);
    }
	
	// std::cout << "Hello Creating Procedural Map" << std::endl;
	// std::cout << "Procedural Map, tutorial status: " << tutorial_on << std::endl;

    auto entity = Entity();
	ProceduralMap& map = registry.proceduralMaps.emplace(entity);

    // initalize map dimensions
	map.width = size.x;
	map.height = size.y;
	map.top = floor(WORLD_ORIGIN.y - size.y / 2);
	map.left = floor(WORLD_ORIGIN.x - size.x / 2);
	map.bottom = ceil(WORLD_ORIGIN.y + size.y / 2);
	map.right = ceil(WORLD_ORIGIN.x + size.x / 2);
	map.map.resize(map.height, std::vector<tileType>(map.width, tileType::EMPTY));

	if (tutorial_on) {
		std::vector<std::vector<int>> tutorial_map = {
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},		
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1},		
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,2,0,1,1},		
			{1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,INFO_BUFF12,0,0,1,1},		
			{1,1,1,1,0,0,0,0,0,0,INFO_BUFF11,0,0,0,0,0,1,1,1,1},		
			{1,1,0,0,0,0,INFO_BUFF10,0,1,0,0,0,1,1,1,1,1,1,1,1},		
			{1,1,INFO_BUFF9,0,0,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1},		
			{1,1,1,0,INFO_BUFF8,0,1,1,1,1,1,0,0,0,0,0,0,1,1,1},		
			{1,1,1,0,0,0,0,0,1,1,0,0,0,INFO_BUFF6,0,0,0,1,1,1},		
			{1,1,1,1,1,0,0,INFO_BUFF7,0,0,0,1,1,1,0,0,0,1,1,1},		
			{1,1,1,1,1,1,0,0,0,1,1,1,1,1,0,INFO_BUFF5,0,1,1,1},		
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1},		
			{1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1},		
			{1,1,1,1,1,1,1,1,1,1,1,0,0,INFO_BUFF4,0,0,1,1,1,1},		
			{1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1},		
			{1,1,1,1,1,1,1,1,1,0,INFO_BUFF3,0,0,0,1,1,1,1,1,1},		
			{1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1},		
			{1,0,INFO_BUFF0,0,INFO_BUFF1,0,0,INFO_BUFF2,0,0,0,0,1,1,1,1,1,1,1,1},		
			{1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1},		
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}		
		};
	
        for (int y = 0; y < tutorial_map.size(); y++) {
            for (int x = 0; x < tutorial_map.size(); x++) {
				if (tutorial_map[x][y] == 1) {
					map.map[y][x] = tileType::WALL;
				} else if (tutorial_map[x][y] == 2) {
					map.map[y][x] = tileType::PORTAL;
				} else if (tutorial_map[x][y] == 0) {
					map.map[y][x] = tileType::EMPTY;
                } else {
                    createBuff(gridCellToPosition({y,x}), static_cast<BUFF_TYPE>(tutorial_map[x][y]));
                    map.map[y][x] = tileType::EMPTY;
				}
			}
		}

        // Place 10 buffs at hardcoded positions along the player's path

	} else {
		// Initialize map to random walls / floors
        std::random_device rd;
		std::default_random_engine rng(rd());
		std::uniform_int_distribution<int> uniform_dist(0, 99);

        const int wallProbability = 40;
        do {
            for (int y = 0; y < map.height; ++y) {
                for (int x = 0; x < map.width; ++x) {
                    int random_value = uniform_dist(rng);
                    map.map[y][x] = (random_value < wallProbability ? tileType::WALL : tileType::EMPTY);
                }
            }
            
            // Call cellular automata algorithm
            map.map = applyCellularAutomataRules(map.map);
            
            // assign player to random empty tile
            std::pair<int, int> playerTile = getRandomEmptyTile(map.map);
            playerPosition.first = playerTile.first;
            playerPosition.second = playerTile.second;
            
            // assign portal to random empty tile
            std::pair<int, int> portalTile = getRandomEmptyTile(map.map);

            for (int x = 0; x < map.width; ++x) {
                map.map[0][x] = tileType::WALL;
                map.map[map.height - 1][x] = tileType::WALL;
            }
            for (int y = 0; y < map.height; ++y) {
                map.map[y][0] = tileType::WALL;
                map.map[y][map.width - 1] = tileType::WALL;
            }
            if (getDistance(map.map, playerTile, portalTile) < 15) {
                // std::cout << "PATH DOES NOT EXIST OR NOT ENOUGH DISTANCE, TRYING AGAIN." << std::endl;
                continue;
            }

            // std::cout << "PATH EXISTS AND IS GOOD DISTANCE!" << std::endl;
            map.map[portalTile.second][portalTile.first] = tileType::PORTAL;

            return entity;
        } while (true);
	}
    return entity;
}


Biome currentBiome = Biome::RED; // Default to red biome

void setCurrentBiomeByLevel(unsigned int level) {
    currentBiome = getBiomeForLevel(level);
}

Biome getBiomeForLevel(unsigned int level) {
    // tutorial and level 1 are red
    if (level == 0 || level == 1) {
        return Biome::RED;
    }
    
    // cycle through biomes for other levels
    unsigned int biomeIndex = (level - 1) % static_cast<unsigned int>(Biome::BIOME_COUNT);
    return static_cast<Biome>(biomeIndex);
}

TEXTURE_ASSET_ID getTileTextureForBiome(Biome biome) {
    switch (biome) {
        case Biome::RED:
            return TEXTURE_ASSET_ID::RED_TILES;
        case Biome::GREEN:
            return TEXTURE_ASSET_ID::GREEN_TILES;
        case Biome::BLUE:
            return TEXTURE_ASSET_ID::BLUE_TILES;
		case Biome::PURPLE:
			return TEXTURE_ASSET_ID::PURPLE_TILES;
        case Biome::BOSS:
            return TEXTURE_ASSET_ID::BOSS_TILES;
        default:
            return TEXTURE_ASSET_ID::RED_TILES; 
    }
}

TEXTURE_ASSET_ID getWallTextureForBiome(Biome biome) {
    switch (biome) {
        case Biome::RED:
            return TEXTURE_ASSET_ID::RED_WALL;
        case Biome::GREEN:
            return TEXTURE_ASSET_ID::GREEN_WALL;
        case Biome::BLUE:
            return TEXTURE_ASSET_ID::BLUE_WALL;
		case Biome::PURPLE: 
			return TEXTURE_ASSET_ID::PURPLE_WALL;
        case Biome::BOSS:
            return TEXTURE_ASSET_ID::BOSS_WALL;
        default:
            return TEXTURE_ASSET_ID::RED_WALL; 
    }
}

Entity addParalaxTile(vec2 gridCoord)
{
    return addTile(gridCoord, getTileTextureForBiome(currentBiome), 3);
}

Entity addWallTile(vec2 gridCoord)
{
    auto tile = addTile(gridCoord, getWallTextureForBiome(currentBiome), 1);
    registry.walls.emplace(tile);
    return tile;
}

Entity addPortalTile(vec2 gridCoord) {
    for (Entity &entity : registry.portals.entities)
    {
        Portal &tile = registry.portals.get(entity);
        if (tile.grid_x == gridCoord.x && tile.grid_y == gridCoord.y)
        {
            return entity;
        }
    }

    auto tile = addTile(gridCoord, TEXTURE_ASSET_ID::PORTAL, total_portal_frames);

    Portal &portal = registry.portals.emplace(tile);
    portal.grid_x = gridCoord.x;
    portal.grid_y = gridCoord.y;

    Animation &a = registry.animations.emplace(tile);
    a.time_per_frame = MS_PER_S / total_portal_frames;
    a.loop = ANIM_LOOP_TYPES::LOOP;
    a.start_frame = 0;
    a.end_frame =
     total_portal_frames;

    return tile;
}

Entity addTile(vec2 gridCoord, TEXTURE_ASSET_ID texture_id, int total_frames)
{
	Entity newTile = Entity();

	Tile& new_tile = registry.tiles.emplace(newTile);
	new_tile.grid_x = gridCoord.x;
	new_tile.grid_y = gridCoord.y;

	Motion& motion = registry.motions.emplace(newTile);
	motion.position = gridCellToPosition(gridCoord);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.scale = { GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX };

	registry.renderRequests.insert(
		newTile,
		{
			texture_id,
			texture_id == TEXTURE_ASSET_ID::PORTAL ? EFFECT_ASSET_ID::SPRITE_SHEET : EFFECT_ASSET_ID::TILE,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	// Add spritesheet component to tile
	SpriteSheetImage& spriteSheet = registry.spriteSheetImages.emplace(newTile);
	spriteSheet.total_frames = total_frames;

	// Add sprite size component to tile
	SpriteSize& sprite = registry.spritesSizes.emplace(newTile);
	sprite.width = GRID_CELL_WIDTH_PX;
	sprite.height = GRID_CELL_HEIGHT_PX;

	return newTile;
}

Entity createCamera()
{
	// Remove all cameras
	std::vector<Entity> camerasToRemove;

	for (Entity &entity : registry.cameras.entities)
	{
		camerasToRemove.push_back(entity);
	}

	for (Entity &entity : camerasToRemove)
	{
		registry.remove_all_components_of(entity);
	}

	Entity cameraEntity = Entity();
	Camera &camera = registry.cameras.emplace(cameraEntity);

	camera.position = WORLD_ORIGIN;
	camera.initialized = false;
	camera.grid_position = WORLD_ORIGIN;

	return cameraEntity;
}

int countAdjacentWalls(const std::vector<std::vector<tileType>>& grid, int x, int y) {
    int height = grid.size();
    int width = grid[0].size();

    int count = 0;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            // Skip center cell
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx;
            int ny = y + dy;
            
            // Out of bounds
            if (nx < 0 || ny < 0 || nx >= width || ny >= height) {
                count++;
            } else {
                if (grid[ny][nx] == tileType::WALL) {
                    count++;
                }
            }
        }
    }
    return count;
}

std::vector<std::vector<tileType>> applyCellularAutomataRules(const std::vector<std::vector<tileType>>& grid) {
    int height = grid.size();
    int width = grid[0].size();

    std::vector<std::vector<tileType>> newGrid(height, std::vector<tileType>(width, tileType::EMPTY));
    
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int wallCount = countAdjacentWalls(grid, x, y);
            if (grid[y][x] == tileType::WALL) {
                // For wall, flip to floor if fewer than 4 neighboring walls
                newGrid[y][x] = (wallCount < 4) ? tileType::EMPTY : tileType::WALL;
            } else {
                // For a floor, flip to wall if 5 or more neighboring walls
                newGrid[y][x] = (wallCount >= 5) ? tileType::WALL : tileType::EMPTY;
            }
        }
    }
    return newGrid;
}

std::pair<int, int> getRandomEmptyTile(const std::vector<std::vector<tileType>>& grid) {
    std::vector<std::pair<int, int>> emptyTiles;

    int height = grid.size();
    int width = grid[0].size();

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            if (grid[y][x] == tileType::EMPTY) {
                emptyTiles.emplace_back(x, y);
            }
        }
    }

    std::random_device rd;
    std::default_random_engine rng(rd());
    std::uniform_int_distribution<int> uniform_dist(0, emptyTiles.size() - 1);
    return emptyTiles[uniform_dist(rng)];
}

int getDistance(const std::vector<std::vector<tileType>>& grid, std::pair<int,int> start, std::pair<int,int> end) {
    if (start.first < 0 || start.second < 0 || end.first < 0 || end.second < 0) return false;
    if (start.second >= (int)grid.size() || end.second >= (int)grid.size()) return false;
    if (start.first >= (int)grid[0].size() || end.first >= (int)grid[0].size()) return false;

    if (grid[start.second][start.first] != tileType::EMPTY ||
        grid[end.second][end.first] != tileType::EMPTY)
        return false;

    std::vector<std::vector<bool>> visited(grid.size(), std::vector<bool>(grid[0].size(), false));
    std::vector<std::vector<int>> distance(grid.size(), std::vector<int>(grid[0].size(), -1));

    std::queue<std::pair<int, int>> bfsQueue;
    bfsQueue.push(start);
    visited[start.second][start.first] = true;
    distance[start.second][start.first] = 0;

    std::vector<std::pair<int,int>> directions = {{0,1},{0,-1},{1,0},{-1,0}};
    while (!bfsQueue.empty()) {
        auto [cx, cy] = bfsQueue.front();
        bfsQueue.pop();

        if (cx == end.first && cy == end.second) return distance[cy][cx];

        for (auto [dx, dy] : directions) {
            int nx = cx + dx;
            int ny = cy + dy;

            if (nx >= 0 && ny >= 0 && ny < (int)grid.size() && nx < (int)grid[0].size()) {
                if (!visited[ny][nx] && grid[ny][nx] == tileType::EMPTY) {
                    visited[ny][nx] = true;
                    distance[ny][nx] = distance[cy][cx] + 1;
                    bfsQueue.push({nx, ny});
                }
            }
        }
    }

    return -1;
}

Entity createBuff(vec2 position, BUFF_TYPE buffType)
{
	Entity entity = Entity();
	Motion &motion = registry.motions.emplace(entity);
	motion.scale = {BUFF_WIDTH, BUFF_HEIGHT};

	Entity player_entity = registry.players.entities[0];

    Motion &player_motion = registry.motions.get(player_entity);

	vec2 player_direction = (glm::length(player_motion.velocity) > 0.0f) 
	? glm::normalize(player_motion.velocity) 
	: vec2(1.0f, 0.0f); 

	// Assign buff a random throwing direction perpendicular to the player's direction within +/-30 deg
    vec2 perp_right = vec2(-player_direction.y, player_direction.x);
    vec2 perp_left = vec2(player_direction.y, -player_direction.x);
    vec2 perp_base = (rand() % 2 == 0) ? perp_right : perp_left;
	float random_angle_offset = ((rand() % 60) - 30) * (M_PI / 180.0f);
    float rotation_cos = cos(random_angle_offset);
    float rotation_sin = sin(random_angle_offset);
    vec2 angle = vec2(
        perp_base.x * rotation_cos - perp_base.y * rotation_sin,
        perp_base.x * rotation_sin + perp_base.y * rotation_cos
    );


	motion.position = vec2(position.x + angle.x * 80.0f, position.y + angle.y * 80.0f);

	float speed = 100.0f + (rand() % 50);
	motion.velocity = angle * speed;

	Buff &buff = registry.buffs.emplace(entity);

	// Currently only the first 15 buffs are active
    if (buffType > BLACK_GOO) {
        buff.type = buffType;
        registry.renderRequests.insert(
            entity,
            {
                TEXTURE_ASSET_ID::INFO_BUFF,
                EFFECT_ASSET_ID::TEXTURED,
                GEOMETRY_BUFFER_ID::SPRITE
            }
        );
    } else {
        buff.type = getRandomBuffType();
        registry.renderRequests.insert(
            entity,
            {TEXTURE_ASSET_ID::BUFFS_SHEET,
             EFFECT_ASSET_ID::SPRITE_SHEET,
             GEOMETRY_BUFFER_ID::SPRITE});
    
        SpriteSheetImage &spriteSheet = registry.spriteSheetImages.emplace(entity);
        spriteSheet.total_frames = 20;	 
        spriteSheet.current_frame = buff.type;
    
        SpriteSize &sprite = registry.spritesSizes.emplace(entity);
        sprite.width = 20;
        sprite.height = 20;
    }


	return entity;
}

Entity createBuffWithChanceToFail(vec2 pos) {
	
	float chance = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    if (chance < BUFF_DROP_FAIL_CHANCE) {
        return {};
    }

	return createBuff(pos);
}

void applyVignetteEffect() {
    registry.screenStates.components[0].vignette_screen_factor = .5f;
}

void clearVignetteEffect() {
	std::cout << "clearing vignette effect" << std::endl;
    registry.screenStates.components[0].vignette_screen_factor = 0.f;
	std::cout << "cleared vignette effect" << std::endl;
	std::cout << registry.screenStates.components[0].vignette_screen_factor << std::endl;
}


// -M4 Feature: GAME BALANCING
BUFF_TYPE getRandomBuffType() {
	std::vector<BUFF_TYPE> commonBuffs = { TAIL, MITOCHONDRIA, HEMOGLOBIN, GOLGI, CELL_WALL, AMINO_ACID, VACUOLE };
	std::vector<BUFF_TYPE> rareBuffs = { CHLOROPLAST, CYTOPLASM, PILLI, ENDOPLASMIC_RETICULUM };
	std::vector<BUFF_TYPE> eliteBuffs = { LYSOSOME, SPARE_NUCLEUS, OCELOID, SECRETOR };

    int categoryRoll = rand() % 100;

    std::vector<BUFF_TYPE>* selectedCategory;

    if (categoryRoll < 50) {  // 50% chance for common
        selectedCategory = &commonBuffs;
    } else if (categoryRoll < 80) { // 30% chance for rare
        selectedCategory = &rareBuffs;
    } else { // 20% chance for elite
        selectedCategory = &eliteBuffs;
    }

    int index = rand() % selectedCategory->size();
    return (*selectedCategory)[index];
}

void damagePlayer(float damageAmount) {
	Player& player = registry.players.get(registry.players.entities[0]);
	
	if (player.current_health <= 0) return;

	if(player.sheilds > 0) {
		player.sheilds --;
		removeBuffUI(CELL_WALL); // PLANT CELL WALL/ SHEILD
	} else {
		player.current_health -= damageAmount * player.dangerFactor;
        applyVignetteEffect();

		if (player.current_health <= 0) {
			if (player.extra_lives > 0) {
				player.current_health = player.max_health/2;
				player.extra_lives--;
				removeBuffUI(SPARE_NUCLEUS);
			} else {
				// game over
			}
		} else {
			// normally took damage
		}
	}
}

Entity createEffect(TEXTURE_ASSET_ID texture, vec2 position, vec2 scale, int total_frames) {
    Entity entity = Entity();

    Motion& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = scale;

    registry.renderRequests.insert(
        entity,
        { texture, 
		EFFECT_ASSET_ID::SPRITE_SHEET, 
		GEOMETRY_BUFFER_ID::SPRITE});

    Animation& animation = registry.animations.emplace(entity);
    animation.start_frame = 0;
    animation.end_frame = total_frames - 1;
    animation.time_per_frame = 50.f;
    animation.loop = ANIM_LOOP_TYPES::NO_LOOP;

    SpriteSheetImage& spriteSheet = registry.spriteSheetImages.emplace(entity);
    spriteSheet.total_frames = total_frames;
    spriteSheet.current_frame = 0;

    SpriteSize& sprite = registry.spritesSizes.emplace(entity);

	Effect& effect = registry.effects.emplace(entity);
    effect.death_timer_ms = animation.time_per_frame * total_frames;


    return entity;
}
