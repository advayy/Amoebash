#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include <iostream>
#include <random>
#include <ctime>
#include <queue>

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

void toggleDashAnimation(Entity entity, bool is_dashing)
{
	Animation &a = registry.animations.get(entity);
	SpriteSheetImage &s = registry.spriteSheetImages.get(entity);

	if (is_dashing)
	{
		a.start_frame = player_dash_start;
		a.end_frame = player_dash_end;
		a.loop = ANIM_LOOP_TYPES::LOOP;
	}
	else
	{
		a.start_frame = player_idle_start;
		a.end_frame = player_idle_end;
		a.loop = ANIM_LOOP_TYPES::PING_PONG;
	}

	s.current_frame = a.start_frame;
	a.forwards = true;
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


void initiatePlayerDash()
{
	Entity &player_e = registry.players.entities[0];
	Player &player = registry.players.get(player_e);
	Motion &player_motion = registry.motions.get(player_e);

	if (isDashing())
	{
		for (Entity& entity : registry.dashes.entities)
		{
			registry.dashes.remove(entity);
		}
	}

	player.dash_count--;

	Dashing &d = registry.dashes.emplace(Entity());
	d.angle_deg = player_motion.angle;
	float angle_radians = (d.angle_deg - 90) * (M_PI / 180.0f);
	d.velocity = {
		player.dash_speed * cosf(angle_radians),
		player.dash_speed * sinf(angle_radians)
	};
	d.timer_ms = DASH_DURATION_MS;
	player.dash_cooldown_timer_ms = player.dash_cooldown_ms;

	// Change animation frames
	toggleDashAnimation(player_e, true);
}

bool canDash()
{
	Player &player = registry.players.get(registry.players.entities[0]);
	return player.dash_count > 0; // PLAYER HAS 1 DASH SAVED ATLEAST
}

bool isDashing()
{
	return registry.dashes.size() > 0;
}

bool willMeshCollideSoon(const Entity& player, const Entity& hexagon, float predictionTime)
{
	Mesh& hexagonMesh = *registry.meshPtrs.get(hexagon);

	Motion& playerMotion = registry.motions.get(player);
	Motion& hexagonMotion = registry.motions.get(hexagon);

	vec2 playerFuturePos = playerMotion.position + playerMotion.velocity * predictionTime;
	vec2 hexagonFuturePos = hexagonMotion.position + hexagonMotion.velocity * predictionTime;

	vec2 diff = playerFuturePos - hexagonFuturePos;
	float distance = length(diff);

	// mesh based collision won't happen 
	if (distance > (playerMotion.scale.x / 2 + hexagonMotion.scale.x / 2))
	{
		return false;
	}

	// mesh based collision will happen

	vec2 playerCenter = playerFuturePos;
	float playerRadius = playerMotion.scale.x / 2;

	std::vector<vec2> hexagonWorldVertices = getWorldVertices(hexagonMesh.textured_vertices, hexagonFuturePos, hexagonMotion.scale);
	int numVertices = hexagonWorldVertices.size();

	bool inside = pointInPolygon(playerCenter, hexagonWorldVertices);

	// check the distance from circle (player) to edge
	for (int i = 0; i < numVertices; i++) {
		int next = (i + 1) % numVertices;

		vec2 A = hexagonWorldVertices[i];
		vec2 B = hexagonWorldVertices[next];

		vec2 AB = A - B;

		float t = dot(playerCenter - A, AB) / dot(AB, AB); // ratio of projections

		t = std::max(0.0f, std::min(1.0f, t)); // clamping

		vec2 projectionPoint = A + AB * t;
		
		float distance = length(playerCenter - projectionPoint);

		if (distance < playerRadius) {
			return true;
		}
	}

	return false;
}

bool pointInPolygon(const vec2& point, const std::vector<vec2> &polygon)
{
	bool inside = false;

	int numVertices = polygon.size();

	// using ray cast algorithm
	for (int i = 0, j = numVertices - 1; i < numVertices; j = i++) {
		const vec2& vi = polygon[i];
		const vec2& vj = polygon[j];

		if (((vi.y > point.y) != (vj.y > point.y)) &&
			(point.x < (vj.x - vi.x) * (point.y - vi.y) / (vj.y - vi.y) + vi.x))
		{
			inside = !inside;
		}
	}

	return inside;
}

std::vector<vec2> getWorldVertices(const std::vector<TexturedVertex>& vertices, const vec2 &position, const vec2 &scale) {
	std::vector<vec2> worldVertices;
	for (const auto& vertex : vertices) {
		vec2 worldVertex = {
			vertex.position.x * scale.x + position.x,
			vertex.position.y * scale.y + position.y
		};

		worldVertices.push_back(worldVertex);
	}
	return worldVertices;
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

void createInfoBoxes() {

	TEXTURE_ASSET_ID baseTexture = TEXTURE_ASSET_ID::MOUSE_CONTROL_INFO;

	for (int i = 0; i < 6; i ++) {
		auto entity1 = Entity();
	
		// Camera &camera = registry.cameras.get(registry.cameras.entities[0]);
		// Player &player = registry.players.get(registry.players.entities[0]);
		int x = (3  *  i)  + 1;
		int y = (i % 2 == 0) ? 11 : 8;
		vec2 infoPosition = gridCellToPosition({x, y});
	
		Motion& motion1 = registry.motions.emplace(entity1);
		motion1.position = infoPosition;

		motion1.scale = {128.f * WORK_SCALE_FACTOR * 3, 128.f * WORK_SCALE_FACTOR};
	
		InfoBox& info1 = registry.infoBoxes.emplace(entity1);
	
		registry.renderRequests.insert(
			entity1,
			{
				baseTexture,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			}
		);

		baseTexture = static_cast<TEXTURE_ASSET_ID>(static_cast<int>(baseTexture) + 1);
	}
}

void removeInfoBoxes() {
	for (auto e : registry.infoBoxes.entities) {
		registry.remove_all_components_of(e);
	}
	return;
}

Entity createMiniMap(RenderSystem *renderer, vec2 size)
{
	auto entity = Entity();

	// create motion component
	Motion &motion = registry.motions.emplace(entity);

	motion.position = {0, 0};
	motion.angle = 0.f;
	motion.velocity = {0, 0};
	motion.scale = {32 * 2 * WORK_SCALE_FACTOR, 32 * 2 * WORK_SCALE_FACTOR};

	// add render request
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::SPIKE_ENEMY,
		 EFFECT_ASSET_ID::MINI_MAP,
		 GEOMETRY_BUFFER_ID::SPRITE});

	// add entity to minimaps
	registry.miniMaps.emplace(entity);

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

void tileProceduralMap() {
	vec2 camera_pos = registry.cameras.get(registry.cameras.entities[0]).grid_position;

	float cameraGrid_x = camera_pos.x;
	float cameraGrid_y = camera_pos.y;

	ProceduralMap& map = registry.proceduralMaps.get(registry.proceduralMaps.entities[0]);

	std::map<int, std::map<int, int>> currentTiles;

	// remove all tiles that arent in the chunk distance
	for (Entity &entity : registry.tiles.entities)
	{

		Tile &tile = registry.tiles.get(entity);
		vec2 tilePos = {tile.grid_x, tile.grid_y};
		vec2 cameraGrid = {cameraGrid_x, cameraGrid_y};
		if (abs(glm::distance(cameraGrid, tilePos)) > CHUNK_DISTANCE)
		{
			registry.remove_all_components_of(entity);
		}
		else
		{
			// mark this tile as already drawn, so we don't create it again
			currentTiles[tile.grid_x][tile.grid_y] = 1;
		}
	}

	// setting map bounds
	int left = (cameraGrid_x - (WINDOW_GRID_WIDTH / 2) - CHUNK_DISTANCE / 2);	 // max((cameraGrid_x - (WINDOW_GRID_WIDTH/2 + CHUNK_DISTANCE/2)), (float) map.left);
	int right = (cameraGrid_x + (WINDOW_GRID_WIDTH / 2) + CHUNK_DISTANCE / 2);	 // min((cameraGrid_x + (WINDOW_GRID_WIDTH/2 +CHUNK_DISTANCE/2)), (float) map.right);
	int top = (cameraGrid_y - (WINDOW_GRID_HEIGHT / 2) - CHUNK_DISTANCE / 2);	 // max((cameraGrid_y - (WINDOW_GRID_HEIGHT/2 + CHUNK_DISTANCE/2)), (float) map.top);
	int bottom = (cameraGrid_y + (WINDOW_GRID_HEIGHT / 2) + CHUNK_DISTANCE / 2); // min((cameraGrid_y + (WINDOW_GRID_HEIGHT/2 + CHUNK_DISTANCE/2)), (float) map.bottom);

	for (int x = left; x < right; x += 1)
	{
		for (int y = top; y < bottom; y += 1)
		{
			vec2 gridCoord = {x, y};

			// check for already existing tiles, don't need to draw these again
			if (currentTiles.find(x) != currentTiles.end() && currentTiles[x].find(y) != currentTiles[x].end()) continue;

			if (x < map.left || x >= map.right || y < map.top || y >= map.bottom) {
				addWallTile(gridCoord);
			} else if (glm::distance(gridCoord, {cameraGrid_x, cameraGrid_y}) <= CHUNK_DISTANCE) {
				
				// print here
				// std::cout << "x: " << x << " y: " << y << std::endl;
				if (map.map[x][y] == tileType::EMPTY) 
				{
					// if its being tiled what tile to put
					addParalaxTile(gridCoord);
                } 
				else if (map.map[x][y] == tileType::PORTAL) 
				{
					addParalaxTile(gridCoord);
                    addPortalTile(gridCoord);
                } 
				else 
				{
					addWallTile(gridCoord);
				}
			}
		}
	}
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

    Entity newTile = Entity();
    Portal &portal = registry.portals.emplace(newTile);
    portal.grid_x = gridCoord.x;
    portal.grid_y = gridCoord.y;

    Motion &motion = registry.motions.emplace(newTile);
    motion.position = gridCellToPosition(gridCoord);
    motion.angle = 0.f;
    motion.velocity = {0, 0};
    motion.scale = {GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX};

    registry.renderRequests.insert(
        newTile,
        {TEXTURE_ASSET_ID::PORTAL,
         EFFECT_ASSET_ID::SPRITE_SHEET,
         GEOMETRY_BUFFER_ID::SPRITE}
    );

    SpriteSheetImage &spriteSheet = registry.spriteSheetImages.emplace(newTile);
    spriteSheet.total_frames = total_portal_frames;

    Animation &a = registry.animations.emplace(newTile);
    a.time_per_frame = MS_PER_S / total_portal_frames;
    a.loop = ANIM_LOOP_TYPES::LOOP;
    a.start_frame = 0;
    a.end_frame = total_portal_frames;

    SpriteSize &sprite = registry.spritesSizes.emplace(newTile);
    sprite.width = GRID_CELL_WIDTH_PX;
    sprite.height = GRID_CELL_HEIGHT_PX;

    return newTile;
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
			EFFECT_ASSET_ID::TILE,
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

vec2 positionToGridCell(vec2 position)
{
	// map the players position to the closest grid cell
	vec2 gridCell = {0, 0};
	// Check which grid cell CONTAINS the players position
	gridCell.x = floor(position.x / GRID_CELL_WIDTH_PX);
	gridCell.y = floor(position.y / GRID_CELL_HEIGHT_PX);
	return gridCell;
}

vec2 gridCellToPosition(vec2 gridCell)
{
	vec2 position = {0, 0};
	position.x = (gridCell.x * GRID_CELL_WIDTH_PX) + (GRID_CELL_WIDTH_PX / 2.0f);
	position.y = (gridCell.y * GRID_CELL_HEIGHT_PX) + (GRID_CELL_HEIGHT_PX / 2.0f);
	return position;
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

// Below are the components of the start Screen

Entity createStartScreen(vec2 position)
{
	Entity startScreenEntity = Entity();
	
	// render request for back ground
	registry.renderRequests.insert(
		startScreenEntity,
		{
			TEXTURE_ASSET_ID::START_SCREEN_BG,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	Motion& bg_motion = registry.motions.emplace(startScreenEntity);
	bg_motion.position = WORLD_ORIGIN;
	bg_motion.velocity = vec2(0.f,0.f);
	bg_motion.angle = 0.f;
	bg_motion.scale = BACKGROUND_SCALE;
	
	Start &start = registry.starts.emplace(startScreenEntity);
	GameScreen &screen = registry.gameScreens.emplace(startScreenEntity);
	screen.type = ScreenType::START;

	Entity startButtonEntity = createStartButton();
	Entity shopButtonEntity = createShopButton();
	Entity infoButtonEntity = createInfoButton();

	start.buttons = std::vector{startButtonEntity, shopButtonEntity, infoButtonEntity};
	
	Entity startScreenLogoEntity = Entity();
	// render request for logo
	registry.renderRequests.insert(
		startScreenLogoEntity,
		{TEXTURE_ASSET_ID::GAME_LOGO,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	Motion &logo_motion = registry.motions.emplace(startScreenLogoEntity);
	logo_motion.position = position;
	logo_motion.scale = {LOGO_WIDTH_PX, LOGO_HEIGHT_PX};
	logo_motion.velocity = {WINDOW_WIDTH_PX / 2.f / BOOT_CUTSCENE_DURATION_MS * MS_PER_S, 0.f};

	start.logo = startScreenLogoEntity;

	return startScreenEntity;
}

Entity createShopScreen()
{
	Entity shopScreenEntity = Entity();

	registry.renderRequests.insert(
		shopScreenEntity,
		{TEXTURE_ASSET_ID::START_SCREEN_BG,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});
	
	Shop& shop = registry.shops.emplace(shopScreenEntity);

	GameScreen &screen = registry.gameScreens.emplace(shopScreenEntity);
	screen.type = ScreenType::SHOP;

	Motion &motion = registry.motions.emplace(shopScreenEntity);
	vec2 position = WORLD_ORIGIN;
	vec2 scale = BACKGROUND_SCALE;

	motion.position = position;
	motion.scale = scale;

	Entity backButtonEntity = createBackButton();
	
	shop.buttons = std::vector{backButtonEntity};

	return shopScreenEntity;
}

Entity createInfoScreen()
{
	Entity infoScreenEntity = Entity();

	registry.renderRequests.insert(
		infoScreenEntity,
		{TEXTURE_ASSET_ID::START_SCREEN_BG,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	Info& info = registry.infos.emplace(infoScreenEntity);

	GameScreen &screen = registry.gameScreens.emplace(infoScreenEntity);
	screen.type = ScreenType::INFO;

	Motion &motion = registry.motions.emplace(infoScreenEntity);
	vec2 position = WORLD_ORIGIN;
	vec2 scale = BACKGROUND_SCALE;

	motion.position = position;
	motion.scale = scale;

	Entity backButtonEntity = createBackButton();

	info.buttons = std::vector{backButtonEntity};

	return infoScreenEntity;
}

Entity createGameOverScreen()
{
	Entity gameOverScreenEntity = Entity();

	registry.renderRequests.insert(
		gameOverScreenEntity,
		{TEXTURE_ASSET_ID::GAMEOVER,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	Over &over = registry.overs.emplace(gameOverScreenEntity);

	GameScreen &screen = registry.gameScreens.emplace(gameOverScreenEntity);
	screen.type = ScreenType::GAMEOVER;

	Motion &motion = registry.motions.emplace(gameOverScreenEntity);
	Camera &camera = registry.cameras.components[0];

	vec2 scale = {LOGO_WIDTH_PX, LOGO_HEIGHT_PX};
	motion.position = camera.position;
	motion.scale = scale;

	return gameOverScreenEntity;
}

Entity createPauseScreen()
{
	Entity pauseScreenEntity = Entity();

	registry.renderRequests.insert(
		pauseScreenEntity,
		{TEXTURE_ASSET_ID::PAUSE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	Pause &pause = registry.pauses.emplace(pauseScreenEntity);

	GameScreen &screen = registry.gameScreens.emplace(pauseScreenEntity);
	screen.type = ScreenType::PAUSE;

	Motion &motion = registry.motions.emplace(pauseScreenEntity);

	vec2 scale = {LOGO_WIDTH_PX, LOGO_HEIGHT_PX};

	Camera &camera = registry.cameras.get(registry.cameras.entities[0]);
	motion.position = camera.position;
	motion.scale = scale;
	return pauseScreenEntity;
}

void createGameplayCutScene()
{
	Entity backGround = createCutSceneBackGround();
	Entity nose = createNose();
	Entity noseAcceent = createNoseAccent();
	Entity nucleus = createEnteringNucleus();

	registry.cutscenes.emplace(backGround);
	registry.cutscenes.emplace(nose);
	registry.cutscenes.emplace(noseAcceent);
	registry.cutscenes.emplace(nucleus);
}

void removeCutScene()
{
	for (auto &e : registry.cutscenes.entities)
	{
		registry.remove_all_components_of(e);
	}
}

Entity createCutSceneBackGround()
{
	Entity backGroundEntity = Entity();

	Motion &motion = registry.motions.emplace(backGroundEntity);
	motion.angle = 0.0f;
	motion.velocity = {0.0f, 0.0f};
	motion.position = WORLD_ORIGIN;

	motion.scale = vec2({WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX});

	registry.renderRequests.insert(
		backGroundEntity,
		{TEXTURE_ASSET_ID::CUTSCENEBACKGROUND,
		 EFFECT_ASSET_ID::SPRITE_SHEET,
		 GEOMETRY_BUFFER_ID::SPRITE});

	Animation &animation = registry.animations.emplace(backGroundEntity);
	animation.time_per_frame = INTRO_CUTSCENE_DURATION_MS / 8;
	animation.start_frame = 0;
	animation.end_frame = 8;

	SpriteSheetImage &spriteSheet = registry.spriteSheetImages.emplace(backGroundEntity);
	spriteSheet.total_frames = 8;

	// not used at the moment
	SpriteSize &sprite = registry.spritesSizes.emplace(backGroundEntity);
	sprite.width = 128.f;
	sprite.height = 68.f;

	return backGroundEntity;
}

Entity createNose()
{
	Entity noseEntity = Entity();

	Motion &motion = registry.motions.emplace(noseEntity);
	motion.angle = 0.0f;
	motion.velocity = {0.0f, 0.0f};

	motion.scale = vec2({67.f * 5 * WORK_SCALE_FACTOR, 41.f * 5 * WORK_SCALE_FACTOR});

	motion.position = {(67.f / 2 - 3) * 5 * WORK_SCALE_FACTOR, (+0.5) * 5 * WORK_SCALE_FACTOR};

	registry.renderRequests.insert(
		noseEntity,
		{TEXTURE_ASSET_ID::NOSE,
		 EFFECT_ASSET_ID::SPRITE_SHEET,
		 GEOMETRY_BUFFER_ID::SPRITE});

	SpriteSheetImage &spriteSheet = registry.spriteSheetImages.emplace(noseEntity);

	spriteSheet.total_frames = 7;

	std::uniform_real_distribution<float> uniform_dist(0.0f, 1.0f);

    std::random_device rd;
    std::default_random_engine rng(rd());
	int random_value = static_cast<int>(uniform_dist(rng) * spriteSheet.total_frames);
	spriteSheet.current_frame = random_value;

	// not used at the moment
	SpriteSize &sprite = registry.spritesSizes.emplace(noseEntity);
	sprite.width = 67.f;
	sprite.height = 41.f;

	return noseEntity;
}

Entity createNoseAccent()
{
	Entity accentEntity = Entity();

	Motion &motion = registry.motions.emplace(accentEntity);
	motion.angle = 0.0f;
	motion.velocity = {0.0f, 0.0f};

	motion.scale = vec2({67.f * 5 * WORK_SCALE_FACTOR, 41.f * 5 * WORK_SCALE_FACTOR});

	motion.scale = vec2({67.f * 5 * WORK_SCALE_FACTOR, 41.f * 5 * WORK_SCALE_FACTOR});

	motion.position = {(67.f / 2 - 3) * 5 * WORK_SCALE_FACTOR, (+0.5) * 5 * WORK_SCALE_FACTOR};

	registry.renderRequests.insert(
		accentEntity,
		{TEXTURE_ASSET_ID::NOSEACCENT,
		 EFFECT_ASSET_ID::SPRITE_SHEET,
		 GEOMETRY_BUFFER_ID::SPRITE});

	SpriteSheetImage &spriteSheet = registry.spriteSheetImages.emplace(accentEntity);
	spriteSheet.total_frames = 5;

	std::uniform_real_distribution<float> uniform_dist(0.0f, 1.0f);

    std::random_device rd;
    std::default_random_engine rng(rd());
	int random_value = static_cast<int>(uniform_dist(rng) * spriteSheet.total_frames);
	std::cout << random_value << std::endl;
	spriteSheet.current_frame = random_value;

	// not used at the moment
	SpriteSize &sprite = registry.spritesSizes.emplace(accentEntity);
	sprite.width = 345.f;
	sprite.height = 210.f;

	return accentEntity;
}

Entity createEnteringNucleus()
{
	Entity nucleusEntity = Entity();

	Motion &motion = registry.motions.emplace(nucleusEntity);
	motion.angle = 0.0f;
	motion.velocity = {0.0f, 0.0f};
	motion.position = {0.f, 0.f};

	motion.scale = vec2({128.f * 5 * WORK_SCALE_FACTOR,
						 68.f * 5 * WORK_SCALE_FACTOR});

	registry.renderRequests.insert(
		nucleusEntity,
		{TEXTURE_ASSET_ID::ENTERINGNUCLEUS,
		 EFFECT_ASSET_ID::SPRITE_SHEET,
		 GEOMETRY_BUFFER_ID::SPRITE});

	Animation &animation = registry.animations.emplace(nucleusEntity);
	animation.time_per_frame = INTRO_CUTSCENE_DURATION_MS / 8;
	animation.start_frame = 0;
	animation.end_frame = 8;

	SpriteSheetImage &spriteSheet = registry.spriteSheetImages.emplace(nucleusEntity);
	spriteSheet.total_frames = 8;

	// not used at the moment
	SpriteSize &sprite = registry.spritesSizes.emplace(nucleusEntity);
	sprite.width = 345.f;
	sprite.height = 210.f;

	return nucleusEntity;
}

void removePauseScreen()
{
	if (registry.pauses.size() == 0)
		return;

	Entity pause = registry.pauses.entities[0];
	registry.remove_all_components_of(pause);
}

void removeGameOverScreen()
{
	if (registry.overs.size() == 0)
		return;

	Entity over = registry.overs.entities[0];
	registry.remove_all_components_of(over);
}

void removeStartScreen()
{
	if (registry.starts.size() == 0)
		return;

	Entity start_entity = registry.starts.entities[0];
	Start &start = registry.starts.components[0];
	std::vector<Entity> buttons_to_remove = start.buttons;
	Entity logo = start.logo;

	std::cout << "Button Size" << std::endl;
	std::cout << buttons_to_remove.size() << std::endl;
	for (auto &entity : buttons_to_remove)
	{
		registry.remove_all_components_of(entity);
	}
	registry.remove_all_components_of(logo);
	registry.remove_all_components_of(start_entity);
}

void removeShopScreen()
{
	if (registry.shops.size() == 0)
		return;
	
	Entity shop_entity = registry.shops.entities[0];
	Shop &shop = registry.shops.components[0];
	std::vector<Entity> buttons_to_remove = shop.buttons;
	std::cout << "Buttons: " << buttons_to_remove.size() << std::endl;
	for (auto &entity : buttons_to_remove)
	{
		registry.remove_all_components_of(entity);
	}

	registry.remove_all_components_of(shop_entity);
}

void removeInfoScreen()
{
	if (registry.infos.size() == 0)
		return;
	
	Entity info_entity = registry.infos.entities[0];
	Info &info = registry.infos.components[0];
	std::vector<Entity> buttons_to_remove = info.buttons;
	std::cout << "Buttons: " << buttons_to_remove.size() << std::endl;
	for (auto &entity : buttons_to_remove)
	{
		registry.remove_all_components_of(entity);
	}

	registry.remove_all_components_of(info_entity);
}

Entity createButton(ButtonType type, vec2 position, vec2 scale, TEXTURE_ASSET_ID texture)
{
	Entity buttonEntity = Entity();

	registry.renderRequests.insert(
		buttonEntity,
		{texture,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	Motion &motion = registry.motions.emplace(buttonEntity);

	motion.position = position;
	motion.scale = scale;

	screenButton &button = registry.buttons.emplace(buttonEntity);
	button.w = scale[0];
	button.h = scale[1];
	button.center = position + vec2{WINDOW_WIDTH_PX / 2.f, WINDOW_HEIGHT_PX / 2.f};
	button.type = type;

	return buttonEntity;
}

Entity createStartButton()
{
	vec2 position = START_BUTTON_COORDINATES;
	vec2 scale = START_BUTTON_SCALE;

	return createButton(ButtonType::STARTBUTTON,
						position,
						scale,
						TEXTURE_ASSET_ID::BUTTON);
}

Entity createShopButton()
{
	vec2 scale = SHOP_BUTTON_SCALE;
	vec2 position = SHOP_BUTTON_COORDINATES;

	return createButton(ButtonType::SHOPBUTTON,
						position,
						scale,
						TEXTURE_ASSET_ID::SHOP_BUTTON);
}

Entity createInfoButton()
{
	vec2 scale = INFO_BUTTON_SCALE; // currently same scale as shop button
	vec2 position = INFO_BUTTON_COORDINATES;

	return createButton(ButtonType::INFOBUTTON,
						position,
						scale,
						TEXTURE_ASSET_ID::INFO_BUTTON);
}

Entity createBackButton() {
	vec2 scale = BACK_BUTTON_SCALE;
	vec2 position = BACK_BUTTON_COORDINATES;

	return createButton(ButtonType::BACKBUTTON,
						position,
						scale,
						TEXTURE_ASSET_ID::BACK_BUTTON);
}


// Cellular Automata map generation functions

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


Entity createUIElement(vec2 position, vec2 scale, TEXTURE_ASSET_ID texture_id, EFFECT_ASSET_ID effect_id)
{
	Entity entity = Entity();

	Motion &motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.scale = scale;

	registry.uiElements.emplace(entity, UIElement{motion.position, motion.scale});

	registry.renderRequests.insert(
		entity,
		{texture_id,
		 effect_id,
		 GEOMETRY_BUFFER_ID::SPRITE});

	return entity;
}

Entity createHealthBar()
{
	Entity entity = Entity();

	Motion &motion = registry.motions.emplace(entity);
	motion.position = HEALTH_BAR_POS;
	motion.scale = {HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT};

	HealthBar &healthBar = registry.healthBars.emplace(entity);
	healthBar.position = motion.position;
	healthBar.scale = motion.scale;
	healthBar.health = registry.players.get(registry.players.entities[0]).current_health;

	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::HEALTH_BAR_UI,
		 EFFECT_ASSET_ID::HEALTH_BAR,
		 GEOMETRY_BUFFER_ID::SPRITE});

	return entity;
}

void createDashRecharge()
{
	for (int i = 0; i < DASH_RECHARGE_COUNT; i++)
	{
		Entity dot = Entity();
		Motion &motion = registry.motions.emplace(dot);

		motion.position = {DASH_RECHARGE_START_POS.x + (i * DASH_RECHARGE_SPACING), DASH_RECHARGE_START_POS.y};
		motion.scale = {DASH_WIDTH, DASH_HEIGHT};

		registry.renderRequests.insert(
			dot,
			{TEXTURE_ASSET_ID::DASH_UI,
			 EFFECT_ASSET_ID::DASH_UI,
			 GEOMETRY_BUFFER_ID::SPRITE});

		registry.dashRecharges.emplace(dot);
	}
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

Entity createBuffUI(vec2 position, int buffType)
{
	Entity buffUI = Entity();

	BuffUI &buff = registry.buffUIs.emplace(buffUI);
	buff.buffType = buffType;

	Motion &motion = registry.motions.emplace(buffUI);
	motion.position = position;
	motion.scale = {BUFF_UI_WIDTH, BUFF_UI_HEIGHT};

	registry.renderRequests.insert(buffUI,
								   {TEXTURE_ASSET_ID::BUFFS_SHEET,
									EFFECT_ASSET_ID::SPRITE_SHEET,
									GEOMETRY_BUFFER_ID::SPRITE});

	SpriteSheetImage &spriteSheet = registry.spriteSheetImages.emplace(buffUI);
	spriteSheet.total_frames = 20;	 
	spriteSheet.current_frame = buff.buffType;
								
	SpriteSize &sprite = registry.spritesSizes.emplace(buffUI);
	sprite.width = BUFF_UI_WIDTH;
	sprite.height = BUFF_UI_HEIGHT;
	
	registry.uiElements.emplace(buffUI, UIElement{motion.position, motion.scale});
	
	return buffUI;
}

void renderCollectedBuff(RenderSystem *renderer, int buffType)
{
	int numCollectedBuffs = registry.buffUIs.size();
	int buffsPerRow = BUFF_NUM / 2;
	vec2 position;
	if (numCollectedBuffs < buffsPerRow)
	{
		position = {BUFF_START_POS.x + numCollectedBuffs * BUFF_SPACING, BUFF_START_POS.y};
		Entity buffUI = createBuffUI(position, buffType);
	}
	else if (numCollectedBuffs >= buffsPerRow && numCollectedBuffs < BUFF_NUM)
	{
		position = {BUFF_START_POS.x + (numCollectedBuffs - buffsPerRow) * BUFF_SPACING,
					BUFF_START_POS.y - BUFF_SPACING};
		Entity buffUI = createBuffUI(position, buffType);
	}
}