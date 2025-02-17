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
			EFFECT_ASSET_ID::SPRITE_SHEET,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	Animation& a = registry.animations.emplace(entity);
	a.timer_ms = 250;
	a.default_frame_timer = 250;
	a.start_frame = 0;
	a.end_frame = 6;

	SpriteSheetImage& spriteSheet = registry.spriteSheetImages.emplace(entity);
	spriteSheet.total_frames = 6;
	spriteSheet.current_frame = 0;

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
			EFFECT_ASSET_ID::SPRITE_SHEET,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	Animation& a = registry.animations.emplace(entity);
	a.timer_ms = 125;
	a.default_frame_timer = 125;
	a.start_frame = 0;
	a.end_frame = 3;

	SpriteSheetImage& spriteSheet = registry.spriteSheetImages.emplace(entity);
	spriteSheet.total_frames = 9;
	spriteSheet.current_frame = 0;

	SpriteSize& sprite = registry.spritesSizes.emplace(entity);
	sprite.width = 32;
	sprite.height = 32; 

	return entity;
}


void animation(float elapsed_ms) {
	for (Entity& entity : registry.animations.entities) {
		Animation& a = registry.animations.get(entity);
		a.timer_ms -= elapsed_ms;

		SpriteSheetImage& s = registry.spriteSheetImages.get(entity);

		if (a.timer_ms <= 0.0f) {
			a.timer_ms = a.default_frame_timer;

			if (s.current_frame == a.end_frame) {
				s.current_frame = a.start_frame;
			}
			else {
				s.current_frame = (s.current_frame + 1);
			}
		}
	}
}

void changeAnimationFrames(Entity entity, int start_frame, int end_frame) {
	Animation& a = registry.animations.get(entity);
	a.start_frame = start_frame;
	a.end_frame = end_frame;

	SpriteSheetImage& s = registry.spriteSheetImages.get(entity);
	s.current_frame = start_frame;
}

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
	
	// MAP doesnt need to be even as we ceil and floor

	Map& map = registry.maps.emplace(entity);
	map.width = size.x;
	map.height = size.y;
	map.top = floor(WORLD_ORIGIN.y - size.y / 2);
	map.left = floor(WORLD_ORIGIN.x - size.x / 2);
	map.bottom = ceil(WORLD_ORIGIN.y + size.y / 2);
	map.right = ceil(WORLD_ORIGIN.x + size.x / 2);

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	return entity;
}

Entity createMiniMap(RenderSystem* renderer, vec2 size) {
	auto entity = Entity();

	// create motion component
	Motion& motion = registry.motions.emplace(entity);
	
	motion.position = { 0, 0};
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.scale = { 32*2*WORK_SCALE_FACTOR, 32*2*WORK_SCALE_FACTOR };

	// add render request
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::ENEMY,
			EFFECT_ASSET_ID::MINI_MAP,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	// add entity to minimaps
	registry.miniMaps.emplace(entity);

	return entity;
}

void tileMap() {
	// Chunk size is a GRID_CELL_WIDTH_PX and GRID_CELL_HEIGHT_PX
	// ADD TILES TO ALL POSITIONS within CHUNK_DISTANCE of the player
	// REMOVE TILES THAT ARE OUTSIDE OF CHUNK_DISTANCE of the player

	vec2 camera_pos = registry.cameras.get(registry.cameras.entities[0]).grid_position;

	float cameraGrid_x = camera_pos.x;
	float cameraGrid_y = camera_pos.y;

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


	//setting map bounds

	int left = (cameraGrid_x - (WINDOW_GRID_WIDTH/2 + CHUNK_DISTANCE/2)); //max((cameraGrid_x - (WINDOW_GRID_WIDTH/2 + CHUNK_DISTANCE/2)), (float) map.left);
	int right =  (cameraGrid_x + (WINDOW_GRID_WIDTH/2 +CHUNK_DISTANCE/2)); // min((cameraGrid_x + (WINDOW_GRID_WIDTH/2 +CHUNK_DISTANCE/2)), (float) map.right);
	int top = (cameraGrid_y - (WINDOW_GRID_HEIGHT/2 + CHUNK_DISTANCE/2)) ; //max((cameraGrid_y - (WINDOW_GRID_HEIGHT/2 + CHUNK_DISTANCE/2)), (float) map.top);
	int bottom = (cameraGrid_y + (WINDOW_GRID_HEIGHT/2 + CHUNK_DISTANCE/2)); //min((cameraGrid_y + (WINDOW_GRID_HEIGHT/2 + CHUNK_DISTANCE/2)), (float) map.bottom);

	for(int x = left; x < right; x += 1) {
		for(int y = top; y < bottom; y += 1) {
			vec2 gridCoord = {x, y};

			// if gridcoord is past the map bounds, plant a wall tile
			if (x < map.left || x > map.right || y < map.top || y > map.bottom) {
				addWallTile(gridCoord);
			}
			if (glm::distance(gridCoord, {cameraGrid_x, cameraGrid_y}) <= CHUNK_DISTANCE) {
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
			TEXTURE_ASSET_ID::PARALAX_TILE,
			EFFECT_ASSET_ID::TILE,
			GEOMETRY_BUFFER_ID::SPRITE
	});	

	// Add spritesheet component to tile
	SpriteSheetImage& spriteSheet = registry.spriteSheetImages.emplace(newTile);
	spriteSheet.total_frames = 3;
	spriteSheet.current_frame = 0;

	// Add sprite size component to tile
	SpriteSize& sprite = registry.spritesSizes.emplace(newTile);
	sprite.width = GRID_CELL_WIDTH_PX;
	sprite.height = GRID_CELL_HEIGHT_PX;

	return newTile;
}

Entity addWallTile(vec2 gridCoord) {
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
			TEXTURE_ASSET_ID::WALL_TILE,
			EFFECT_ASSET_ID::TILE,
			GEOMETRY_BUFFER_ID::SPRITE
	});	

	// Add spritesheet component to tile
	SpriteSheetImage& spriteSheet = registry.spriteSheetImages.emplace(newTile);
	spriteSheet.total_frames = 1;
	spriteSheet.current_frame = 0;

	// Add sprite size component to tile
	SpriteSize& sprite = registry.spritesSizes.emplace(newTile);
	sprite.width = GRID_CELL_WIDTH_PX;
	sprite.height = GRID_CELL_HEIGHT_PX;

	return newTile;
}

void removeTile(vec2 gridCoord) {
	for (Entity& entity : registry.tiles.entities) {
		Tile& tile = registry.tiles.get(entity);
		if (tile.grid_x == gridCoord.x && tile.grid_y == gridCoord.y) {
			registry.remove_all_components_of(entity);
			return;
		}
	}
}

vec2 positionToGridCell(vec2 position) {
	// map the players position to the closest grid cell
	vec2 gridCell = {0, 0};
	// Check which grid cell CONTAINS the players position
	gridCell.x = floor(position.x / GRID_CELL_WIDTH_PX);
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
	// Remove all cameras
	for (Entity& entity : registry.cameras.entities) {
		registry.remove_all_components_of(entity);
	}


    Entity cameraEntity = Entity();
    Camera& camera = registry.cameras.emplace(cameraEntity);

    camera.position = WORLD_ORIGIN;
	camera.initialized = false;
	camera.grid_position = WORLD_ORIGIN;

    return cameraEntity;
}


// Below are the components of the start Screen

Entity createStartScreen() {
	Entity startScreenEntity = Entity();

	registry.renderRequests.insert(
		startScreenEntity,
		{
			TEXTURE_ASSET_ID::SCREEN,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	GameScreen& screen = registry.gameScreens.emplace(startScreenEntity);
	screen.type = ScreenType::START;


	Motion& motion = registry.motions.emplace(startScreenEntity);
	vec2 position = {0.f, 0.f};
	vec2 scale = {LOGO_WIDTH_PX, LOGO_HEIGHT_PX};
	motion.position = position;
	motion.scale = scale;

	Entity startButtonEntity = createStartButton();
    Entity shopButtonEntity  = createShopButton();
    Entity infoButtonEntity  = createInfoButton();

	return startScreenEntity;
}

Entity createShopScreen() {
	Entity shopScreenEntity = Entity();

	registry.renderRequests.insert(
			shopScreenEntity,
			{
				TEXTURE_ASSET_ID::SHOPSCREEN,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			}
	);

	GameScreen& screen = registry.gameScreens.emplace(shopScreenEntity);
	screen.type = ScreenType::SHOP;

	Motion& motion = registry.motions.emplace(shopScreenEntity);
	vec2 position = {0.f, 0.f};
	vec2 scale = {WINDOW_WIDTH_PX / 3.f, WINDOW_HEIGHT_PX / 3.f};

	motion.position = position;
	motion.scale = scale;

	return shopScreenEntity;
}

Entity createInfoScreen() {
	Entity infoScreenEntity = Entity();

	registry.renderRequests.insert(
			infoScreenEntity,
			{
				TEXTURE_ASSET_ID::INFOSCREEN,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			}
	);

	GameScreen& screen = registry.gameScreens.emplace(infoScreenEntity);
	screen.type = ScreenType::INFO;

	Motion& motion = registry.motions.emplace(infoScreenEntity);
	vec2 position = {0.f, 0.f};
	vec2 scale = {WINDOW_WIDTH_PX / 3.f, WINDOW_HEIGHT_PX / 3.f};

	motion.position = position;
	motion.scale = scale;

	return infoScreenEntity;
}

Entity createGameOverScreen() {
	Entity gameOverScreenEntity = Entity();

	registry.renderRequests.insert(
		gameOverScreenEntity,
		{
				TEXTURE_ASSET_ID::GAMEOVER,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
		}
	);

	Over& over = registry.overs.emplace(gameOverScreenEntity);

	GameScreen& screen = registry.gameScreens.emplace(gameOverScreenEntity);
	screen.type = ScreenType::GAMEOVER;

	Motion& motion = registry.motions.emplace(gameOverScreenEntity);
	Camera& camera = registry.cameras.components[0];

	vec2 scale = {WINDOW_WIDTH_PX / 3.f, WINDOW_HEIGHT_PX / 3.f};
	motion.position = camera.position;
	motion.scale = scale;

	return gameOverScreenEntity;
}

Entity createPauseScreen() {
	Entity pauseScreenEntity = Entity();

	registry.renderRequests.insert(
			pauseScreenEntity,
			{
				TEXTURE_ASSET_ID::PAUSE,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			}
		);

	Pause& pause = registry.pauses.emplace(pauseScreenEntity);

	GameScreen& screen = registry.gameScreens.emplace(pauseScreenEntity);
	screen.type = ScreenType::PAUSE;
		
	Motion& motion = registry.motions.emplace(pauseScreenEntity);
	
	vec2 scale = {WINDOW_WIDTH_PX / 3.f, WINDOW_HEIGHT_PX / 3.f};


	Camera& camera = registry.cameras.get(registry.cameras.entities[0]);
	motion.position = camera.position;
	motion.scale = scale;
	return pauseScreenEntity;
}

void removePauseScreen() {
	if (registry.pauses.size() == 0) return;

	Entity pause = registry.pauses.entities[0];
	registry.remove_all_components_of(pause);
}

void removeGameOverScreen() {
	if (registry.overs.size() == 0) return;

	Entity over = registry.overs.entities[0];
	registry.remove_all_components_of(over);
}

Entity createButton(ButtonType type, vec2 position, vec2 scale, TEXTURE_ASSET_ID texture) {
	Entity buttonEntity = Entity();

	registry.renderRequests.insert(
			buttonEntity,
			{
				texture,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE
			}
		);

	Motion& motion = registry.motions.emplace(buttonEntity);
	
	motion.position = position;
	motion.scale = scale;

	screenButton& button = registry.buttons.emplace(buttonEntity);
	button.w = scale[0];
	button.h = scale[1];
	button.center = position + vec2{WINDOW_WIDTH_PX / 2.f, WINDOW_HEIGHT_PX /2.f};
	button.type = type;

	return buttonEntity;
}

Entity createStartButton() {
	vec2 position = { 0.f, WINDOW_HEIGHT_PX / 4.5f };
    vec2 scale    = { WINDOW_WIDTH_PX / 7.f, WINDOW_HEIGHT_PX / 7.f };
    
	return createButton(ButtonType::STARTBUTTON, 
						position, 
						scale, 
						TEXTURE_ASSET_ID::BUTTON);
}

Entity createShopButton() {
	vec2 scale    = { WINDOW_WIDTH_PX / 20.f, WINDOW_HEIGHT_PX / 20.f * 1.78f };
    vec2 position = { scale.x - WINDOW_WIDTH_PX / 2.f,
                      scale.y - WINDOW_HEIGHT_PX / 2.f };
    
	return createButton(ButtonType::SHOPBUTTON, 
						position, 
						scale, 
						TEXTURE_ASSET_ID::SHOPBUTTON);
}

Entity createInfoButton() {
    vec2 scale    = { WINDOW_WIDTH_PX / 20.f, WINDOW_HEIGHT_PX / 20.f * 1.78f };
    vec2 position = { scale.x - WINDOW_WIDTH_PX / 2.f,
                      scale.y + WINDOW_HEIGHT_PX / 3.f };

    return createButton(ButtonType::INFOBUTTON, 
						position, 
						scale, 
						TEXTURE_ASSET_ID::NUCLEUS);
}