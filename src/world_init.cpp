#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include <iostream>
#include <random>
#include <ctime>
#include <queue>
#include "animation_system.hpp"

Entity createEnemy(RenderSystem* renderer, vec2 position)
{
	// reserve an entity
	auto entity = Entity();

	// invader
	Enemy& enemy = registry.enemies.emplace(entity);
	enemy.health = ENEMY_HEALTH;

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

	return entity;
}

Entity createProjectile(vec2 pos, vec2 size, vec2 velocity)
{
	auto entity = Entity();
	auto &p = registry.projectiles.emplace(entity);
	p.damage = PROJECTILE_DAMAGE;

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
	Motion motion = registry.motions.get(bacteriophage);
	vec2 direction = vec2(cosf((motion.angle - 90) * (M_PI / 180)), sinf((motion.angle - 90) * (M_PI / 180)));
	vec2 projectile_pos = motion.position + (motion.scale * direction);
	vec2 projectile_velocity = direction * PROJECTILE_SPEED;
	Entity projectile = createProjectile(projectile_pos, { PROJECTILE_BB_WIDTH, PROJECTILE_BB_HEIGHT }, projectile_velocity);
	registry.bacteriophageProjectiles.emplace(projectile);
	return projectile;
}

Entity createProceduralMap(RenderSystem* renderer, vec2 size, bool tutorial_on, std::pair<int, int>& playerPosition) {
    // print entering map
    std::cout << "Entering createProceduralMap" << std::endl;

    for (Entity& entity : registry.proceduralMaps.entities) {
        registry.remove_all_components_of(entity);
    }
    for (Entity& entity : registry.portals.entities) {
        registry.remove_all_components_of(entity);
    }
	
	std::cout << "Hello Creating Procedural Map" << std::endl;
	std::cout << "Procedural Map, tutorial status: " << tutorial_on << std::endl;

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
		int hall_x = map.height / 2;

		for (int y = 0; y < map.height; y++) {
			map.map[y][hall_x] = tileType::EMPTY;
		}

		for (int x = 0; x < map.width; x++) {
			if (x < hall_x - 1 || x > hall_x) {
				for (int y = 0; y < map.height; y++) {
					map.map[y][x] = tileType::WALL;
				}
			}
		}
		std::cout << "Created InfoBoxes" << std::endl;

	} else {
		std::cout << "Should randomize map" << std::endl;
		std::srand(static_cast<unsigned>(std::time(0)));
	
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

            // print map
            for (int y = 0; y < map.height; ++y) {
                for (int x = 0; x < map.width; ++x) {
                    if (x == playerTile.first && y == playerTile.second) {
                        std::cout << "!!";
                    } else if (x == portalTile.first && y == portalTile.second) {
                        std::cout << "P";
                    } else {
                        std::cout << (map.map[y][x] == tileType::WALL ? "X" : ".");
                    }
                }
                std::cout << std::endl;
            }

            if (getDistance(map.map, playerTile, portalTile) < 15) {
                std::cout << "PATH DOES NOT EXIST OR NOT ENOUGH DISTANCE, TRYING AGAIN." << std::endl;
                continue;
            }

            std::cout << "PATH EXISTS AND IS GOOD DISTANCE!" << std::endl;
            map.map[portalTile.second][portalTile.first] = tileType::PORTAL;

            return entity;
        } while (true);
	}

    return entity;
}

Entity createKey(RenderSystem *renderer, vec2 position) 
{
	auto entity = Entity();

	auto &key = registry.keys.emplace(entity);

	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::HEXAGON);
	registry.meshPtrs.emplace(entity, &mesh);

	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.0f;
	motion.velocity = {0.0f, 0.0f};
	motion.position = position;
	motion.scale = {HEXAGON_RADIUS, HEXAGON_RADIUS};

	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::KEY,
		 EFFECT_ASSET_ID::HEXAGON,
		 GEOMETRY_BUFFER_ID::HEXAGON}
	);

	return entity;
}

Entity createChest(RenderSystem *renderer, vec2 position)
{
	auto entity = Entity();

	auto &chest = registry.chests.emplace(entity);

	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::HEXAGON);
	registry.meshPtrs.emplace(entity, &mesh);

	auto &motion = registry.motions.emplace(entity);
	motion.angle = 0.0f;
	motion.velocity = {0.0f, 0.0f};
	motion.position = position;
	motion.scale = {HEXAGON_RADIUS * 3.f, HEXAGON_RADIUS * 3.f};

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::CHEST,
			EFFECT_ASSET_ID::HEXAGON,
			GEOMETRY_BUFFER_ID::HEXAGON
		}
	);
 
	return entity;
}

Entity addParalaxTile(vec2 gridCoord)
{
	return addTile(gridCoord, TEXTURE_ASSET_ID::PARALAX_TILE, 3);
}

Entity addWallTile(vec2 gridCoord)
{
	auto tile = addTile(gridCoord, TEXTURE_ASSET_ID::WALL_TILE, 1);
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
	for (Entity &entity : registry.cameras.entities)
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

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
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

Entity createBuff(vec2 position)
{
	Entity entity = Entity();
	Motion &motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.scale = {BUFF_WIDTH, BUFF_HEIGHT};

	// Assign buff a random throwing direction
	float angle = (rand() % 360) * (M_PI / 180.0f);
	float speed = 100.0f + (rand() % 50);
	motion.velocity = {cos(angle) * speed, sin(angle) * speed};

	Buff &buff = registry.buffs.emplace(entity);

	// Currently only the first 5 buffs are active
	buff.type = rand() % NUMBER_OF_BUFFS;

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

	return entity;
}
