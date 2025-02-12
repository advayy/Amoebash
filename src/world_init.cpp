#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include <iostream>

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! TODO A1: implement grid lines as gridLines with renderRequests and colors
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Entity createGridLine(vec2 start_pos, vec2 end_pos)
{
	Entity entity = Entity();

	// TODO A1: create a gridLine component
	registry.gridLines.insert(entity, { start_pos, end_pos });

	// re-use the "DEBUG_LINE" renderRequest
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::LINE,
			GEOMETRY_BUFFER_ID::DEBUG_LINE
		}
	);
	
	// TODO A1: grid line color (choose your own color) // THIS IS A COMPONENT
	vec3 fcolor = { 0.0f, 0.5f, 0.5f }; 
    registry.colors.insert(entity, { fcolor });
	
	return entity;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! TODO A1: implement grid lines as gridLines with renderRequests and colors
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
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
	motion.velocity = { 50, 0 }; // FLAG
	motion.position = position;

	// resize, set scale to negative if you want to make it face the opposite way
	// motion.scale = vec2({ -INVADER_BB_WIDTH, INVADER_BB_WIDTH });
	motion.scale = vec2({ ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::ENEMY,
			EFFECT_ASSET_ID::ANIMATED_TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	Animation& a = registry.animations.emplace(entity);
	a.current_frame = 0;
	a.timer_ms = 250;
	a.default_frame_timer = 250;
	a.total_frames = 6;
	a.start_frame = 0;
	a.end_frame = 6;

	SpriteSize& sprite = registry.spritesSizes.emplace(entity);
	sprite.width = 32;
	sprite.height = 32;

	return entity;
}

Entity createPlayer(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// new tower
	auto& p = registry.players.emplace(entity);
	
	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.0f;	// A1-TD: CK: rotate to the left 180 degrees to fix orientation
	motion.velocity = { 0.0f, 0.0f };
	motion.position = position;

	// Setting initial values, scale is negative to make it face the opposite way
	motion.scale = vec2({ PLAYER_BB_WIDTH, PLAYER_BB_HEIGHT });

	// create an (empty) Tower component to be able to refer to all towers
	registry.deadlys.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::PLAYER,
			EFFECT_ASSET_ID::ANIMATED_TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	Animation& a = registry.animations.emplace(entity);
	a.current_frame = 0;
	a.timer_ms = 125;
	a.default_frame_timer = 125;
	a.total_frames = 9;
	a.start_frame = 0;
	a.end_frame = 3;

	SpriteSize& sprite = registry.spritesSizes.emplace(entity);
	sprite.width = 32;
	sprite.height = 32;

	return entity;
}

void animation(float elapsed_ms) {
	for (Entity& entity : registry.animations.entities) {
		Animation& a = registry.animations.get(entity);
		a.timer_ms -= elapsed_ms;

		if (a.timer_ms <= 0.0f) {
			a.timer_ms = a.default_frame_timer;

			if (a.current_frame == a.end_frame) {
				a.current_frame = a.start_frame;
			}
			else {
				a.current_frame = (a.current_frame + 1);
			}
		}
	}
}

void changeAnimationFrames(Entity entity, int start_frame, int end_frame) {
	Animation& a = registry.animations.get(entity);
	a.start_frame = start_frame;
	a.end_frame = end_frame;
	a.current_frame = start_frame;
}

// void changeAnimationFrames(Entity entity, int start_frame, int end_frame, int current_frame) {
// 	Animation& a = registry.animations.get(entity);
// 	a.start_frame = start_frame;
// 	a.end_frame = end_frame;
// 	a.current_frame = current_frame;
// }

Entity createProjectile(vec2 pos, vec2 size, vec2 velocity)
{
	auto entity = Entity();
	auto& p = registry.projectiles.emplace(entity);
	p.damage = PROJECTILE_DAMAGE;

	// Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	// registry.meshPtrs.emplace(entity, &mesh);

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = velocity;// velocity;
	motion.position = pos;
	motion.scale = size;

	// registry.debugComponents.emplace(entity); // Causes it to not run kinda?
	registry.deadlys.emplace(entity);
	
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::PROJECTILE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	return entity;
}

Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity,
		{
			// usage TEXTURE_COUNT when no texture is needed, i.e., an .obj or other vertices are used instead
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::LINE,
			GEOMETRY_BUFFER_ID::DEBUG_LINE
		}
	);

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}

void InitiatePlayerDash() {
	Player& player = registry.players.get(registry.players.entities[0]);
	Motion& player_motion = registry.motions.get(registry.players.entities[0]);
	
	if (isDashing())
	{
		// remove all other dashes - dash cancel...
		for (Entity& entity : registry.dashes.entities)
		{
			registry.remove_all_components_of(entity);
		}
	}

	
	Dashing& d = registry.dashes.emplace(Entity());
	d.angle = player_motion.angle;
	d.timer_ms = DASH_DURATION_MS;
	player.dash_cooldown_ms = PLAYER_DASH_COOLDOWN_MS;

	// Change animation frames
	changeAnimationFrames(registry.players.entities[0], 4, 7);
}

bool canDash() {
	Player& player = registry.players.get(registry.players.entities[0]);
	return player.dash_cooldown_ms <= 0;
}

bool isDashing() {
	return registry.dashes.size() > 0;
}

Entity createMap(RenderSystem* renderer, vec2 size) {
	auto entity = Entity();

	// Map
	// Note Size is in BLOCKS.... a block is a grid Square
	// MAP Must be even

	Map& map = registry.maps.emplace(entity);
	map.width = size.x;
	map.height = size.y;
	map.top = WORLD_ORIGIN.y - size.y / 2;
	map.left = WORLD_ORIGIN.x - size.x / 2;
	map.bottom = WORLD_ORIGIN.y + size.y / 2;
	map.right = WORLD_ORIGIN.x + size.x / 2;

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	return entity;
}

void tileMap() {
	// Chunk size is a GRID_CELL_WIDTH_PX and GRID_CELL_HEIGHT_PX
	// ADD TILES TO ALL POSITIONS within CHUNK_DISTANCE of the player
	// REMOVE TILES THAT ARE OUTSIDE OF CHUNK_DISTANCE of the player

	vec2 camera_pos = registry.cameras.get(registry.cameras.entities[0]).position;

	float cameraGrid_x = positionToGridCell(camera_pos).x;
	float cameraGrid_y = positionToGridCell(camera_pos).y;
	Map& map = registry.maps.get(registry.maps.entities[0]);

	// remove all tiles that arent in the chunk distance
	for (Entity& entity : registry.tiles.entities) {
		Tile& tile = registry.tiles.get(entity);
		vec2 tilePos = {tile.grid_x, tile.grid_y};
		vec2 cameraGrid = {cameraGrid_x, cameraGrid_y};
		if (abs(glm::distance(cameraGrid, tilePos)) > CHUNK_DISTANCE) {
			removeTile({tile.grid_x, tile.grid_y});
		}
	}

	for(int x = cameraGrid_x - (WINDOW_GRID_WIDTH/2 + 1); x < cameraGrid_x + (WINDOW_GRID_WIDTH/2 +1); x += 1) {
		for(int y = cameraGrid_y - (WINDOW_GRID_HEIGHT/2 +1); y < cameraGrid_y + (WINDOW_GRID_HEIGHT/2 + 1); y += 1) {
			vec2 gridCoord = {x, y};

			if (glm::distance(gridCoord, {cameraGrid_x, cameraGrid_y}) <= CHUNK_DISTANCE) {
				// std::cout << "CALL Add tile at: " << gridCoord.x << ", " << gridCoord.y << std::endl;
				addTile(gridCoord);
				}
		}
	}
}

Entity addTile(vec2 gridCoord) {
	for (Entity& entity : registry.tiles.entities) {
		Tile& tile = registry.tiles.get(entity);
		if (tile.grid_x == gridCoord.x && tile.grid_y == gridCoord.y) {
			return entity;
		}
	}

	Entity newTile = Entity();
	Tile& new_tile = registry.tiles.emplace(newTile);
	new_tile.grid_x = gridCoord.x;
	new_tile.grid_y = gridCoord.y;

	Motion& motion = registry.motions.emplace(newTile);
	motion.position = gridCellToPosition(gridCoord);
	motion.angle = 0.f;
	motion.velocity = {0, 0};
	motion.scale = {GRID_CELL_WIDTH_PX, GRID_CELL_HEIGHT_PX};

	registry.renderRequests.insert(
		newTile,
		{
			TEXTURE_ASSET_ID::TILE,
			EFFECT_ASSET_ID::TILE,
			GEOMETRY_BUFFER_ID::SPRITE
	});	
	return newTile;
}

void removeTile(vec2 gridCoord) {
	for (Entity& entity : registry.tiles.entities) {
		Tile& tile = registry.tiles.get(entity);
		if (tile.grid_x == gridCoord.x && tile.grid_y == gridCoord.y) {
			registry.remove_all_components_of(entity);
			registry.tiles.remove(entity);
			return;
		}
	}
}

vec2 positionToGridCell(vec2 position) {
	// map the players position to the closest grid cell
	vec2 gridCell = {0, 0};
	// Check which grid cell CONTAINS the players position
	gridCell.x = (position.x / GRID_CELL_WIDTH_PX);
	gridCell.y = floor(position.y / GRID_CELL_HEIGHT_PX);
	return gridCell;
}

vec2 gridCellToPosition(vec2 gridCell) {
	vec2 position = {0, 0};
    position.x = (gridCell.x * GRID_CELL_WIDTH_PX) + (GRID_CELL_WIDTH_PX / 2.0f);
    position.y = (gridCell.y * GRID_CELL_HEIGHT_PX) + (GRID_CELL_HEIGHT_PX / 2.0f);
	return position;
}


Entity createCamera() {
    Entity cameraEntity = Entity();
    Camera& camera = registry.cameras.emplace(cameraEntity);

    camera.position = WORLD_ORIGIN;

    return cameraEntity;
}
