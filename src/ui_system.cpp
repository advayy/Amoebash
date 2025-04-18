#include "ui_system.hpp"
#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include <random>
#include <iostream>


Entity createMiniMap(RenderSystem *renderer, vec2 size)
{
	auto entity = Entity();

	// create motion component
	Motion &motion = registry.motions.emplace(entity);


	vec2 offset = {WINDOW_WIDTH_PX / 2 - 100, -WINDOW_HEIGHT_PX / 2 + 100};
	Camera &camera = registry.cameras.get(registry.cameras.entities[0]);

	motion.position = {camera.position.x + offset.x, camera.position.y + offset.y};
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
	MiniMap& m = registry.miniMaps.emplace(entity);
	m.visited = std::vector<std::vector<int>>(MAP_HEIGHT, std::vector<int>(MAP_WIDTH, 0));
	
	UIElement& u = registry.uiElements.emplace(entity);
	u.position = motion.position;
	u.scale = motion.scale;

	return entity;
}

// create start screen and buttons
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

// create shop screen and buttons
Entity createShopScreen()
{
	
	Entity shopScreenEntity = Entity();
	Shop& shop = registry.shops.emplace(shopScreenEntity);

	registry.renderRequests.insert(
		shopScreenEntity,
		{TEXTURE_ASSET_ID::START_SCREEN_BG,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});
	

	GameScreen &screen = registry.gameScreens.emplace(shopScreenEntity);
	screen.type = ScreenType::SHOP;

	Motion &motion = registry.motions.emplace(shopScreenEntity);
	motion.position = WORLD_ORIGIN;
	motion.scale = BACKGROUND_SCALE;

	Entity backButtonEntity = createBackButton();
	shop.buttons = std::vector{backButtonEntity};

	// MAKE 6 SHOP SLOTS
	float padding_plate = 150.0;
	vec2 s = {WORLD_ORIGIN.x - WINDOW_WIDTH_PX/4, WORLD_ORIGIN.y - WINDOW_HEIGHT_PX/4};

	for (int i = -1; i < 2; i ++) {
		for (int j = 0; j < 2; j++) {
			vec2 pos = {s.x + (i * padding_plate), s.y + (j * padding_plate)};
			registry.shops.emplace(createShopPlate(pos));
		}
	}
	
	// MAKE SHOP BOX
	registry.shops.emplace(createShopBox());

	// MAKE SHOPKEEPER
	registry.shops.emplace(createShopKeeper());

	return shopScreenEntity;
}

Entity createShopPlate(vec2 pos) {
	Entity plate = Entity();

	registry.renderRequests.insert(
		plate,
		{TEXTURE_ASSET_ID::SHOP_PLATE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	Motion& m = registry.motions.emplace(plate);
	m.position = pos;
	m.scale = { SHOP_PLATE_SCALE.x, SHOP_PLATE_SCALE.y };

	return plate;
}

Entity createShopBox() {
	Entity box = Entity();
	// TEXTURE, MOTION
	registry.renderRequests.insert(
		box,
		{TEXTURE_ASSET_ID::PURCHASE_BOX,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE});

	Motion& m = registry.motions.emplace(box);
	m.position = {WORLD_ORIGIN.x - WINDOW_WIDTH_PX/4, WORLD_ORIGIN.y + WINDOW_HEIGHT_PX/5};
	m.scale = { PURCHASE_BOX_SCALE.x, PURCHASE_BOX_SCALE.y };

	return box;
}

Entity createShopKeeper() {
	Entity shopKeeper = Entity();

	// TEXTURE, ANIMATION, SPRITESHEET, SPRITESIZE, MOTION
	registry.renderRequests.insert(
		shopKeeper,
		{TEXTURE_ASSET_ID::SHOPKEEPER,
		 EFFECT_ASSET_ID::SPRITE_SHEET,
		 GEOMETRY_BUFFER_ID::SPRITE});

	Motion& m = registry.motions.emplace(shopKeeper);
	m.scale = {SHOPKEEPER_SIZE.x, SHOPKEEPER_SIZE.y};
	m.position = {WORLD_ORIGIN.x + WINDOW_WIDTH_PX/4, WORLD_ORIGIN.y};
		
	Animation &animation = registry.animations.emplace(shopKeeper);
	
	animation.time_per_frame = 100.0;
	animation.start_frame = 0;
	animation.end_frame = 0;
	animation.loop = ANIM_LOOP_TYPES::LOOP;
	 
	SpriteSheetImage &spriteSheet = registry.spriteSheetImages.emplace(shopKeeper);
	spriteSheet.total_frames = 3;
	 
	SpriteSize &sprite = registry.spritesSizes.emplace(shopKeeper);
	sprite.width = SHOPKEEPER_SIZE.x; 
	sprite.height = SHOPKEEPER_SIZE.y; 

	return shopKeeper;
}

Entity createClickableShopBuff(vec2 position, BUFF_TYPE buffType)
{

	if(buffType >= 0 && buffType < 15) {
		Entity e = createClickableBuffUI(position, buffType);
		registry.shops.emplace(e);
		return e;
	}

	TEXTURE_ASSET_ID selectedTexture = TEXTURE_ASSET_ID::BUFFS_SHEET;

	if(buffType == INJECTION) {
		// injection
		selectedTexture = TEXTURE_ASSET_ID::INJECTION;
	} else if (buffType == SLOT_INCREASE) {
		selectedTexture = TEXTURE_ASSET_ID::SLOT_INCREASE_BUFF;
	}


	Entity buff = Entity();

	ClickableBuff& clickable = registry.clickableBuffs.emplace(buff);

	clickable.picked = false;
	clickable.returnPosition = position;
	clickable.type = buffType;

	Motion &motion = registry.motions.emplace(buff);
	motion.position = position;

	motion.scale = {BUFF_WIDTH, BUFF_HEIGHT};

	registry.renderRequests.insert(buff,
								   {selectedTexture,
									EFFECT_ASSET_ID::TEXTURED,
									GEOMETRY_BUFFER_ID::SPRITE});
	
	registry.shops.emplace(buff);
	return buff;
}

// create info screen and buttons
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
	motion.position = WORLD_ORIGIN;
	motion.scale = BACKGROUND_SCALE;

	Entity backButtonEntity = createBackButton();
	info.buttons = std::vector{backButtonEntity};
	return infoScreenEntity;
}

// create game over screen with nucleus menu
Entity createGameOverScreen()
{
	return createNucleusMenuScreen();
}

// create nucleus menu nucleus
Entity createNucleusMenuNucleus() {
	Entity e = Entity();

	registry.renderRequests.insert(e,
	{
		TEXTURE_ASSET_ID::NUCLEUS_MENU,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE
	});
	Motion &motion = registry.motions.emplace(e);

	vec2 scale = {NUCLEUS_MENU_NUCLEUS_WIDTH, NUCLEUS_MENU_NUCLEUS_HEIGHT};

	Camera &camera = registry.cameras.get(registry.cameras.entities[0]);
	motion.position = {camera.position.x + (2*100), camera.position.y + (2*-20)};
	motion.scale = scale;

	return e;
}

// create nucleus menu slot for buff carry on
Entity createNucleusMenuSlot(vec2 position, int slotNumber){
	Entity e = Entity();

	Slot& s = registry.slots.emplace(e);
	s.number = slotNumber;

	registry.renderRequests.insert(e,
	{
		TEXTURE_ASSET_ID::NUCLEUS_MENU_SLOT,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE
	});

	Motion& m = registry.motions.emplace(e);

	m.position = position;
	m.scale = {NUCLEUS_MENU_SLOT_WIDTH, NUCLEUS_MENU_SLOT_HEIGHT};

	return e;
}

// create nucleus menu screen with slots and menu
Entity createNucleusMenuScreen() {
	// Add all buffs collected to the screen,
	// Add the number of cups
	// add the nucleus thing
	Progression p = registry.progressions.get(registry.progressions.entities[0]);

	Entity nucleus = createNucleusMenuNucleus();
	Entity nucleusMenuScreen = Entity();
	
	registry.overs.emplace(nucleus);
	Over& o = registry.overs.emplace(nucleusMenuScreen);

	// go through player/ game progression list of like buffs from last run... and place them using drawBuffUI?
	// center of nucleus 	
	
	Camera &camera_temp = registry.cameras.get(registry.cameras.entities[0]);
	vec2 origin_position = {camera_temp.position.x + (2*100), camera_temp.position.y + (2*-20)};
	vec2 screeenCentre = {camera_temp.position.x , camera_temp.position.y};	

	GameScreen &screen = registry.gameScreens.emplace(nucleusMenuScreen);

	screen.type = ScreenType::GAMEOVER;

	Motion &motion = registry.motions.emplace(nucleusMenuScreen);
	Camera &camera = registry.cameras.components[0];
	motion.position = camera.position;

	Entity proceedButtonEntity = createProceedButton(vec2({origin_position.x + 180, origin_position.y + NUCLEUS_MENU_NUCLEUS_HEIGHT}));
	o.buttons = {proceedButtonEntity};


	if(p.slots_unlocked == 1) { // upgrade 1...
		registry.overs.emplace(createNucleusMenuSlot(origin_position, 1));
	} else if (p.slots_unlocked == 4) {
		vec2 pos1 = {origin_position.x + NUCLEUS_MENU_SLOT_PADDING + NUCLEUS_MENU_SLOT_HEIGHT, origin_position.y};
		vec2 pos2 = {origin_position.x, origin_position.y + (NUCLEUS_MENU_SLOT_PADDING + NUCLEUS_MENU_SLOT_HEIGHT)};
		vec2 pos3 = {origin_position.x - (NUCLEUS_MENU_SLOT_PADDING + NUCLEUS_MENU_SLOT_HEIGHT), origin_position.y};
		vec2 pos4 = {origin_position.x, origin_position.y - (NUCLEUS_MENU_SLOT_PADDING + NUCLEUS_MENU_SLOT_HEIGHT)};
		
		registry.overs.emplace(createNucleusMenuSlot(pos4, 1)); // top
		registry.overs.emplace(createNucleusMenuSlot(pos2, 2)); // left m
		registry.overs.emplace(createNucleusMenuSlot(pos1, 3)); // right m
		registry.overs.emplace(createNucleusMenuSlot(pos3, 4)); // bottom
	} else if (p.slots_unlocked == 9) {
		
		vec2 pos1 = {origin_position.x - (NUCLEUS_MENU_SLOT_PADDING + NUCLEUS_MENU_SLOT_HEIGHT), origin_position.y - (NUCLEUS_MENU_SLOT_PADDING + NUCLEUS_MENU_SLOT_HEIGHT)};
		vec2 pos2 = {origin_position.x, origin_position.y - (NUCLEUS_MENU_SLOT_PADDING + NUCLEUS_MENU_SLOT_HEIGHT)};
		vec2 pos3 = {origin_position.x + (NUCLEUS_MENU_SLOT_PADDING + NUCLEUS_MENU_SLOT_HEIGHT), origin_position.y - (NUCLEUS_MENU_SLOT_PADDING + NUCLEUS_MENU_SLOT_HEIGHT)};
		vec2 pos4 = {origin_position.x - (NUCLEUS_MENU_SLOT_PADDING + NUCLEUS_MENU_SLOT_HEIGHT), origin_position.y};
		// origin
		vec2 pos5 = {origin_position.x + NUCLEUS_MENU_SLOT_PADDING + NUCLEUS_MENU_SLOT_HEIGHT, origin_position.y};
		vec2 pos6 = {origin_position.x - (NUCLEUS_MENU_SLOT_PADDING + NUCLEUS_MENU_SLOT_HEIGHT), origin_position.y + (NUCLEUS_MENU_SLOT_PADDING + NUCLEUS_MENU_SLOT_HEIGHT)};
		vec2 pos7 = {origin_position.x, origin_position.y + (NUCLEUS_MENU_SLOT_PADDING + NUCLEUS_MENU_SLOT_HEIGHT)};
		vec2 pos8 = {origin_position.x + NUCLEUS_MENU_SLOT_PADDING + NUCLEUS_MENU_SLOT_HEIGHT, origin_position.y + (NUCLEUS_MENU_SLOT_PADDING + NUCLEUS_MENU_SLOT_HEIGHT)};

		registry.overs.emplace(createNucleusMenuSlot(pos1, 1));
		registry.overs.emplace(createNucleusMenuSlot(pos2, 2));
		registry.overs.emplace(createNucleusMenuSlot(pos3, 3));
		registry.overs.emplace(createNucleusMenuSlot(pos4, 4));
		registry.overs.emplace(createNucleusMenuSlot(origin_position, 5));
		registry.overs.emplace(createNucleusMenuSlot(pos5, 5));
		registry.overs.emplace(createNucleusMenuSlot(pos6, 6));
		registry.overs.emplace(createNucleusMenuSlot(pos7, 7));
		registry.overs.emplace(createNucleusMenuSlot(pos8, 8));
	}

	// place each buff on the screen and make it "clickable" - with a clicked and a return to?
	// HOW MUCH CAN I PLACE ON SCREEN? 
	// START CORNER PADDING ?

	// CAN PLACE WITHIN THESE RANGES + PADDING TOP, BOTTOM, LEFT
	float screenTop = screeenCentre.y - WINDOW_HEIGHT_PX/2;
	float screenLeft = screeenCentre.x - WINDOW_WIDTH_PX/2;
	float screenBottom = screeenCentre.y + WINDOW_HEIGHT_PX/2;
	float rightMax = (camera.position.x + (2*100)) - NUCLEUS_MENU_NUCLEUS_WIDTH/2;
	float padding = 20;
	vec2 startPos = {screenLeft + padding + BUFF_WIDTH, screenTop + padding + BUFF_HEIGHT};
	vec2 currentPos = startPos;

    for (auto &buff : registry.players.components[0].buffsCollected) {
        if (buff.second == 0) continue;

        for (int i = 0; i < buff.second; i++) {
            registry.overs.emplace(createClickableBuffUI(currentPos, buff.first));
            currentPos.y += padding + BUFF_HEIGHT;

            if ((currentPos.y + padding + BUFF_HEIGHT) >= screenBottom) {
                startPos.x += padding + BUFF_WIDTH;
                currentPos = startPos;
            }
        }
    }

	return nucleusMenuScreen;
}

// create the buff UI for carry on
Entity createClickableBuffUI(vec2 position, BUFF_TYPE buffType)
{
	Entity buff = Entity();

	ClickableBuff& clickable = registry.clickableBuffs.emplace(buff);

	clickable.picked = false;
	clickable.returnPosition = position;
	clickable.type = buffType;


	Motion &motion = registry.motions.emplace(buff);
	motion.position = position;

	motion.scale = {BUFF_WIDTH, BUFF_HEIGHT};

	registry.renderRequests.insert(buff,
								   {TEXTURE_ASSET_ID::BUFFS_SHEET,
									EFFECT_ASSET_ID::SPRITE_SHEET,
									GEOMETRY_BUFFER_ID::SPRITE});

	SpriteSheetImage &spriteSheet = registry.spriteSheetImages.emplace(buff);
	spriteSheet.total_frames = 20;	 
	spriteSheet.current_frame = buffType;
								
	SpriteSize &sprite = registry.spritesSizes.emplace(buff);
	sprite.width = BUFF_WIDTH;
	sprite.height = BUFF_HEIGHT;
	
	// std::cout << "created clickable buff ui for " << buff << std::endl;

	return buff;
}

// create pause screen and buttons
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

	vec2 scale = {WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX};

	Camera &camera = registry.cameras.get(registry.cameras.entities[0]);
	motion.position = camera.position;
	motion.scale = scale;

	vec2 saveButtonPosition = camera.position + vec2(-BACK_BUTTON_SCALE.x/2.0f - WORK_SCALE_FACTOR * UI_MARGIN_X, -WINDOW_HEIGHT_PX/2.0f + WORK_SCALE_FACTOR* UI_MARGIN_Y);
	vec2 resumeButtonPosition = camera.position + vec2(BACK_BUTTON_SCALE.x/2.0f + WORK_SCALE_FACTOR * UI_MARGIN_X, -WINDOW_HEIGHT_PX/2.0f + WORK_SCALE_FACTOR* UI_MARGIN_Y);

	Entity saveButtonEntity = createSaveButton(saveButtonPosition);
	registry.uiElements.emplace(saveButtonEntity);
	Entity resumeButtonEntity = createResumeButton(resumeButtonPosition);
	registry.uiElements.emplace(resumeButtonEntity);

	
	ButtonType type = registry.buttons.get(saveButtonEntity).type;
	ButtonType resumeType = registry.buttons.get(resumeButtonEntity).type;

	Pause &saveButton = registry.pauses.emplace(saveButtonEntity);
	Pause &resumeButton = registry.pauses.emplace(resumeButtonEntity);
	

	return pauseScreenEntity;
}

// add components for cut scene animation
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

// removing all cutscrene components
void removeCutScene()
{
    std::vector<Entity> cutSceneList;

    for (auto e : registry.cutscenes.entities) {
        cutSceneList.push_back(e);
    }

    int size = cutSceneList.size();

    for(int i = 0; i < size; i++) {
        registry.remove_all_components_of(cutSceneList[i]);
    }
}

Entity createEndingWinScene() {
	Entity winScreenEntity = Entity();

	Motion& motion = registry.motions.emplace(winScreenEntity);
	motion.angle = 0.0f;
	motion.velocity = {0.0f, 0.0f};
	motion.position = registry.cameras.components[0].position;
	motion.scale = vec2({WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX});

	registry.renderRequests.insert(
		winScreenEntity,
		{TEXTURE_ASSET_ID::WINSCREEN,
		 EFFECT_ASSET_ID::SPRITE_SHEET,
		 GEOMETRY_BUFFER_ID::SPRITE}
	);

	Animation &animation = registry.animations.emplace(winScreenEntity);
	animation.time_per_frame = WIN_CUTSCENE_DURATION_MS / 4;
	animation.start_frame = 0;
	animation.end_frame = 4;
	animation.loop = ANIM_LOOP_TYPES::LOOP;

	SpriteSheetImage &spriteSheet = registry.spriteSheetImages.emplace(winScreenEntity);
	spriteSheet.total_frames = 4;

	SpriteSize &sprite = registry.spritesSizes.emplace(winScreenEntity);
	sprite.width = 128.f; 
	sprite.height = 72.f;  

	registry.cutscenes.emplace(winScreenEntity);

	return winScreenEntity;
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
	// std::cout << random_value << std::endl;
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

// remove Pause Screen and buttons
void removePauseScreen()
{
	if (registry.pauses.size() == 0)
		return;

	std::vector<Entity> removals;

	for (auto e : registry.pauses.entities) {
		removals.push_back(e);
	}

	int size = removals.size();

	for (int i = 0; i < size; i++) {
		registry.remove_all_components_of(removals[i]);
	}

	return;

}

// remove game over screen and related UI
void removeGameOverScreen()
{
	if (registry.overs.size() == 0)
		return;


	std::vector<Entity> toRemove;
	for(int i = 0; i < registry.overs.size(); i++) {
		toRemove.push_back(registry.overs.entities[i]);
	}

	for(int i = 0;  i < toRemove.size(); i++) {
		registry.remove_all_components_of(toRemove[i]);
	}
}

// remove start screen and related UI
void removeStartScreen()
{
	if (registry.starts.size() == 0)
		return;

	Entity start_entity = registry.starts.entities[0];
	Start &start = registry.starts.components[0];
	std::vector<Entity> buttons_to_remove = start.buttons;
	Entity logo = start.logo;

    int size = buttons_to_remove.size();

    for (int i = 0; i < size; i++) {
        registry.remove_all_components_of(buttons_to_remove[i]);
    }

	registry.remove_all_components_of(logo);
	registry.remove_all_components_of(start_entity);
}

// remove shop screen and related UI
void removeShopScreen()
{
	
	std::vector<Entity> to_remove;
	for(int i = 0; i < registry.shops.entities.size(); i++) {
		if(!registry.clickableBuffs.has(registry.shops.entities[i])) {
			to_remove.push_back(registry.shops.entities[i]);

			Entity shop_entity = registry.shops.entities[i];
			Shop &shop = registry.shops.components[i];
			if(shop.buttons.size() > 0) {
				// add the button entities to the list too
				to_remove.insert(to_remove.end(), shop.buttons.begin(), shop.buttons.end());
			}
		}
	}

	for(int i = 0; i < to_remove.size(); i++) {
		registry.remove_all_components_of(to_remove[i]);
	}
}

// remove info screen and related UI
void removeInfoScreen()
{
	if (registry.infos.size() == 0)
		return;
	
	Entity info_entity = registry.infos.entities[0];
	Info &info = registry.infos.components[0];
	std::vector<Entity> buttons_to_remove = info.buttons;
	
    int size = buttons_to_remove.size();

    for (int i = 0; i < size; i++) {
        registry.remove_all_components_of(buttons_to_remove[i]);
    }
    
	registry.remove_all_components_of(info_entity);
}

// Button Creater
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
						TEXTURE_ASSET_ID::START_BUTTON);
}

Entity createShopButton()
{
	vec2 scale = SHOP_BUTTON_SCALE;
	vec2 position = SHOP_BUTTON_COORDINATES;

	Entity button = createButton(ButtonType::SHOPBUTTON,
						position,
						scale,
						TEXTURE_ASSET_ID::SHOP_BUTTON);

	// registry.shops.emplace(button);
	return button;
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

	// for (auto e : registry.buttons.entities) {
	// 		registry.buttons.remove(e);
	// }

	return createButton(ButtonType::BACKBUTTON,
						position,
						scale,
						TEXTURE_ASSET_ID::BACK_BUTTON);
}

Entity createProceedButton(vec2 position) {
	vec2 scale = PROCEED_BUTTON_SCALE;

	return createButton(ButtonType::PROCEEDBUTTON,
						position,
						scale,
						TEXTURE_ASSET_ID::PROCEED_BUTTON);
}

Entity createResumeButton(vec2 position) {
	vec2 scale = RESUME_BUTTON_SCALE;

	return createButton(ButtonType::RESUMEBUTTON,
						position,
						scale,
						TEXTURE_ASSET_ID::RESUME_BUTTON);
}

Entity createSaveButton(vec2 position) {
	vec2 scale = SAVE_BUTTON_SCALE;

	return createButton(ButtonType::SAVEBUTTON,
						position,
						scale,
						TEXTURE_ASSET_ID::SAVE_BUTTON);
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


// -M4 Feature: GAME BALANCING
Entity createThermometer() {
	Entity entity = Entity();

	Motion &motion = registry.motions.emplace(entity);
	motion.position = THERMOMETER_POS;
	motion.scale = {THERMOMETER_WIDTH, THERMOMETER_HEIGHT};
	
	UIElement& u = registry.uiElements.emplace(entity);
	u.position = motion.position;
	u.scale = motion.scale;

	Thermometer &t = registry.thermometers.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::THERMOMETER,
		 EFFECT_ASSET_ID::THERMOMETER_EFFECT,
		 GEOMETRY_BUFFER_ID::SPRITE});

	return entity;
}

Entity createHealthBar()
{
	Entity entity = Entity();

	Motion &motion = registry.motions.emplace(entity);
	motion.position = HEALTH_BAR_POS;
	motion.scale = {HEALTH_BAR_WIDTH, HEALTH_BAR_HEIGHT};

	UIElement& u = registry.uiElements.emplace(entity);
	u.position = motion.position;
	u.scale = motion.scale;

	HealthBar &healthBar = registry.healthBars.emplace(entity);
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
		Entity dash = Entity();

		Animation &a = registry.animations.emplace(dash);
		a.start_frame = 1;
		a.end_frame = 3;
		a.time_per_frame = 300.0f;
		a.loop = ANIM_LOOP_TYPES::PING_PONG;

		SpriteSheetImage &spriteSheet = registry.spriteSheetImages.emplace(dash);
		spriteSheet.total_frames = 3;
		spriteSheet.current_frame = 0;

		SpriteSize &sprite = registry.spritesSizes.emplace(dash);
		sprite.width = 19;
		sprite.height = 20;

		registry.renderRequests.insert(
			dash,
			{TEXTURE_ASSET_ID::DASH_UI,
			 EFFECT_ASSET_ID::SPRITE_SHEET,
			 GEOMETRY_BUFFER_ID::SPRITE});

		
	Motion &motion = registry.motions.emplace(dash);
	registry.dashRecharges.emplace(dash);
}

Entity createEnemyHPBar(Entity enemy, TEXTURE_ASSET_ID texture_id) {
    for (Entity hp : registry.healthBars.entities) {
        if (registry.healthBars.get(hp).is_enemy_hp_bar &&
            registry.healthBars.get(hp).owner == enemy) {
            return hp;
        }
    }

    Entity hp = Entity();

    Motion& motion = registry.motions.emplace(hp);
	Motion& enemy_motion = registry.motions.get(enemy);

	float scale_factor = enemy_motion.scale.x * 0.65f / ENEMY_HP_BAR_WIDTH;
    motion.scale = vec2(ENEMY_HP_BAR_WIDTH * scale_factor, ENEMY_HP_BAR_HEIGHT * scale_factor); 
	motion.position = enemy_motion.position + vec2(0.f, enemy_motion.scale.y / 1.5f);
    
    HealthBar& healthBar = registry.healthBars.emplace(hp);
    healthBar.is_enemy_hp_bar = true;
	healthBar.owner = enemy; 

    registry.renderRequests.insert(hp, {
        texture_id,
        EFFECT_ASSET_ID::HEALTH_BAR,
        GEOMETRY_BUFFER_ID::SPRITE
    });

    return hp;
}

void removeEnemyHPBar(Entity enemy) {
    std::vector<Entity> toRemove;
    for (Entity e : registry.healthBars.entities) {
        HealthBar& hb = registry.healthBars.get(e);
        if (hb.is_enemy_hp_bar && hb.owner == enemy) {
            registry.remove_all_components_of(e);
            break;
        }
    }
}


Entity createBuffUI(vec2 position, BUFF_TYPE type, vec2 scale)
{
	Entity buffUI = Entity();

	BuffUI &buff = registry.buffUIs.emplace(buffUI);
	buff.buffType = type;

	Motion &motion = registry.motions.emplace(buffUI);
	motion.position = position;
	motion.scale = scale;

    if (type == INFO_BOSS1) {
        TEXTURE_ASSET_ID texture = static_cast<TEXTURE_ASSET_ID>(static_cast<int>(TEXTURE_ASSET_ID::BOSS_STAGE_1));

        registry.renderRequests.insert(
            buffUI,
            {
                texture,
                EFFECT_ASSET_ID::SPRITE_SHEET,
                GEOMETRY_BUFFER_ID::SPRITE
            }
        );

        Animation& a = registry.animations.emplace(buffUI);
        a.start_frame = 0;
        a.end_frame = 7;
        a.time_per_frame = 100.0f;
        a.loop = ANIM_LOOP_TYPES::LOOP;

        SpriteSheetImage& spriteSheet = registry.spriteSheetImages.emplace(buffUI);
        spriteSheet.total_frames = 7;
        spriteSheet.current_frame = 0;

        SpriteSize& sprite = registry.spritesSizes.emplace(buffUI);
        sprite.width = motion.scale.x;
        sprite.height = motion.scale.y;
    } else if (type == INFO_BOSS2) {
        registry.renderRequests.insert(
            buffUI,
            {
                TEXTURE_ASSET_ID::FINAL_BOSS,
                EFFECT_ASSET_ID::SPRITE_SHEET,
                GEOMETRY_BUFFER_ID::SPRITE
            }
        );

        Animation& a = registry.animations.emplace(buffUI);
        a.start_frame = 0;
        a.end_frame = 10;
        a.time_per_frame = 100.0f;
        a.loop = ANIM_LOOP_TYPES::PING_PONG;

        SpriteSheetImage& spriteSheet = registry.spriteSheetImages.emplace(buffUI);
        spriteSheet.total_frames = 14;
        spriteSheet.current_frame = 0;

        SpriteSize& sprite = registry.spritesSizes.emplace(buffUI);
        sprite.width = motion.scale.x;
        sprite.height = motion.scale.y;
    } else if (type > BLACK_GOO) {
        registry.renderRequests.insert(
            buffUI,
            {
                TEXTURE_ASSET_ID::INFO_BUFF,
                EFFECT_ASSET_ID::TEXTURED,
                GEOMETRY_BUFFER_ID::SPRITE
            }
        );
    } else {
        registry.renderRequests.insert(buffUI,
                                       {TEXTURE_ASSET_ID::BUFFS_SHEET,
                                        EFFECT_ASSET_ID::SPRITE_SHEET,
                                        GEOMETRY_BUFFER_ID::SPRITE});
    
        SpriteSheetImage &spriteSheet = registry.spriteSheetImages.emplace(buffUI);
        spriteSheet.total_frames = 20;	 
        spriteSheet.current_frame = type;
                                    
        SpriteSize &sprite = registry.spritesSizes.emplace(buffUI);
        sprite.width = scale.x;
        sprite.height = scale.y;
    }

	
	registry.uiElements.emplace(buffUI, UIElement{motion.position, motion.scale});
	
	return buffUI;
}

Entity createRowBuffUI(vec2 position, BUFF_TYPE type)
{
	return createBuffUI(position, type, { BUFF_UI_WIDTH, BUFF_UI_HEIGHT });
}

Entity createPopupBuffUI(vec2 position, BUFF_TYPE type)
{
	return createBuffUI(position, type, { POPUP_BUFF_UI_WIDTH, POPUP_BUFF_UI_HEIGHT });
}

// Render collected buffs a certain amount per row that stacks
void renderCollectedBuff(RenderSystem *renderer, BUFF_TYPE buffType)
{
    auto &collectedBuffs = registry.players.get(registry.players.entities[0]).buffsCollected;
    if (collectedBuffs[buffType] == 0) {
        return;
    }

    int numCollectedBuffs = 0;
    for (auto& buff : collectedBuffs) {
        if (buff.second > 0) {
            numCollectedBuffs++;
        }
    }

	int freeSlot = registry.buffUIs.size() - registry.popupElements.size(); // Assume that we will never leave a buff on there with a gap when removing.
	int buffsPerRow = BUFF_NUM / 2;
	vec2 position;
	
    bool found = false;
    for (auto& b : registry.buffUIs.entities) {
        if (registry.buffUIs.get(b).buffType == buffType) {
            found = true;
            break;
        }
    }

    if (!found) {
        if (freeSlot < buffsPerRow) // if the free slot id is larger than collected buffs 
        {
            position = {BUFF_START_POS.x + freeSlot * BUFF_SPACING, BUFF_START_POS.y};
            Entity buffUI = createRowBuffUI(position, buffType);
        }
        else if (freeSlot >= buffsPerRow && freeSlot < BUFF_NUM)
        {
            position = {BUFF_START_POS.x + (freeSlot - buffsPerRow) * BUFF_SPACING,
                        BUFF_START_POS.y - BUFF_SPACING};
            Entity buffUI = createRowBuffUI(position, buffType);
        }
    }

}

vec2 getBuffSlot (BUFF_TYPE buffType) {
	vec2 position = {0, 0};

	for(int i = 0; i < registry.buffUIs.size(); i++) {
		// Get the buffUI, if the type is correct, return its position
		BuffUI& b = registry.buffUIs.get(registry.buffUIs.entities[i]);

		if(b.buffType == buffType) {
			position = registry.motions.get(registry.buffUIs.entities[i]).position;
			break;
		}
	}
	return position;
}

vec2 getBuffSlot_uiPos (BUFF_TYPE buffType) {
	vec2 position = {0, 0};

	for(int i = 0; i < registry.buffUIs.size(); i++) {
		// Get the buffUI, if the type is correct, return its position
		BuffUI& b = registry.buffUIs.get(registry.buffUIs.entities[i]);

		if(b.buffType == buffType) {
			position = registry.uiElements.get(registry.buffUIs.entities[i]).position;
			break;
		}
	}
	return position;
}

void removeBuffUI(BUFF_TYPE buffType) {
	// After you find the position to remove, then first remove the buff, and for all motions that are greater than it, move them back + think of the wrap around case
	// this ensures that the buff no and slot no are always paired.

    auto& collectedBuffs = registry.players.get(registry.players.entities[0]).buffsCollected;
    if (collectedBuffs[buffType] > 1) {
        collectedBuffs[buffType]--;
        return;
    }
	
	vec2 removeBuffPosition = getBuffSlot(buffType);
	vec2 uiPos_removeBuffPosition = getBuffSlot_uiPos(buffType);
	bool isRemovedOnFirstLevel = true; // Assumed its on first / lower bar..

	if(uiPos_removeBuffPosition.y == (BUFF_START_POS.y - BUFF_SPACING)) { // not on first level...
		isRemovedOnFirstLevel = false;
	}

	Entity toRemove;

	for(auto e: registry.buffUIs.entities) {
		Motion& m = registry.motions.get(e);

		if(m.position.x == removeBuffPosition.x && m.position.y == removeBuffPosition.y) {
			toRemove = e;
			break;
		}
	}

	registry.remove_all_components_of(toRemove);

	vec2 pos = removeBuffPosition;
	
	for(auto e: registry.buffUIs.entities) {
		Motion& m = registry.motions.get(e);
		
		// DETERMINE WHETHER TO MOVE? 
		// - if Y is the same and x new is greater than x saved
		// - if Y new is less then the other
		if(m.position.y == pos.y && m.position.x > pos.x) {
			// needs to be shifted back
		} else if (m.position.y < pos.y) {
			// needs to shift back
		} else {
			// skip this buffUI
			continue;
		}
		
		// CASES When moving
		// IS FIRST LEVEL SHIFT BACK (2)
		// IS SECOND LEVEL AND WOULD SHIFT TO LOWER X THEN SHIFT DOWN TO MAX
		// IS SECOND LEVEL AND WOULDNT, THEN SHIFT BACK (2)

		UIElement& ui_e = registry.uiElements.get(e);
		
		if(isRemovedOnFirstLevel && ui_e.position.x == BUFF_START_POS.x) {
			int buffsPerRow = BUFF_NUM / 2;
			ui_e.position.x = BUFF_START_POS.x + ((buffsPerRow - 1) * BUFF_SPACING); // Move back by one space - not sure about the -1
			ui_e.position.y = BUFF_START_POS.y;
		} else {
			ui_e.position.x -= BUFF_SPACING;
		}

		// for some reason we need to also change its position as a UI element 	registry.uiElements.emplace(buffUI, UIElement{motion.position, motion.scale});
	}

	// Remove the buff from the player's collected buffs
	findAndRemove(registry.players.get(registry.players.entities[0]).buffsCollected, buffType);
}


void findAndRemove(std::unordered_map<BUFF_TYPE, int>& map, BUFF_TYPE N) {
    auto it = map.find(N);
    if (it != map.end()) {
        if (it->second == 0) return;
        it->second--;
    }
}


// HUD element update such has health etc.
void updateHuds()
{
	Camera &camera = registry.cameras.get(registry.cameras.entities[0]);

	if (!registry.uiElements.entities.empty())
	{
		for (Entity entity : registry.uiElements.entities)
		{
			if (!registry.motions.has(entity))
				continue;
			Motion &uiMotion = registry.motions.get(entity);
			UIElement &uiElement = registry.uiElements.get(entity);
			uiMotion.position = {camera.position.x + uiElement.position.x, camera.position.y + uiElement.position.y};
		}

	}

	if (!registry.healthBars.entities.empty()) {
		std::vector<Entity> removals;
		for (Entity health_bar : registry.healthBars.entities) {
			if (!registry.motions.has(health_bar) || !registry.healthBars.has(health_bar))
				continue;

			Motion& bar_motion = registry.motions.get(health_bar);
			HealthBar& hb = registry.healthBars.get(health_bar);

			if (hb.is_enemy_hp_bar) {
				Entity enemy = hb.owner;

				if (!registry.enemies.has(enemy) || !registry.motions.has(enemy)) {
					removals.push_back(health_bar);
					// registry.remove_all_components_of(health_bar);
					continue;
				}	
					Motion& enemy_motion = registry.motions.get(enemy);
					bar_motion.position = enemy_motion.position + vec2(0.f, -enemy_motion.scale.y / 1.5f);
					hb.health = registry.enemies.get(enemy).health;
			} else {
				if (!registry.cameras.entities.empty()) {
					Camera& camera = registry.cameras.get(registry.cameras.entities[0]);
					bar_motion.position = {
						camera.position.x + HEALTH_BAR_POS.x,
						camera.position.y + HEALTH_BAR_POS.y
					};
				}
			}
		}


		int size = removals.size();
		for (int i = 0; i < size; i++) {
			registry.remove_all_components_of(removals[i]);
		}
	}


	// if (!registry.thermometers.entities.empty()) {
	// 	Thermometer& t = registry.thermometers.get(registry.thermometers.entities[0]);
	// 	Motion& m = registry.motions.get(registry.thermometers.entities[0]);
	// 	m.position = {
	// 		camera.position.x + THERMOMETER_POS.x,
	// 		camera.position.y + THERMOMETER_POS.y
	// 	};
	// }

}

void updatePopups(float elapsed_ms_since_last_update)
{
	removePopups([&](Entity& entity)
		{
			PopupWithImage& popup = registry.imagePopups.get(entity);
			popup.duration -= elapsed_ms_since_last_update;
			return popup.duration < 0;
		});
}

void removePopups(std::function<bool(Entity&)> shouldRemove)
{
	std::vector<Entity> removals;

	for (auto& entity : registry.imagePopups.entities)
	{
		if (shouldRemove(entity))
		{
			PopupWithImage& popup = registry.imagePopups.get(entity);
			registry.remove_all_components_of(popup.text);
			registry.remove_all_components_of(popup.description);
			registry.remove_all_components_of(popup.image);
			removals.push_back(entity);
		}
	}

	for (int i = 0; i < removals.size(); i++)
	{
		registry.remove_all_components_of(removals[i]);
	}
}

Entity createText(std::string text, vec2 start_pos, vec3 color, float scale)
{
	Entity entity = Entity();

	registry.texts.insert(entity, { text, color });
	Motion& motion = registry.motions.emplace(entity);
	motion.position = start_pos;
	motion.scale = { scale, scale };

	return entity;
}

Entity createBuffPopup(BUFF_TYPE type)
{
	removePopups([](Entity& entity) { return true;});

	Entity buffPopup = Entity();

	const Entity& buffImage = createPopupBuffUI(BUFF_POPUP_POS + vec2(BUFF_POPUP_GAP, BUFF_POPUP_GAP), type);
	registry.popupElements.emplace(buffImage);
	Motion& buffImageMotion = registry.motions.get(buffImage);

	auto buff_test = BUFF_TYPE_TO_TEXT.at(type);

	registry.imagePopups.insert(
		buffPopup,
		PopupWithImage(
			createText(buff_test.first, imageCoordToTextCoord(buffImageMotion.position) + vec2(buffImageMotion.scale.x, 0) + vec2(BUFF_POPUP_GAP, 0), { 1.0f, 1.0f, 1.0f }, 0.5),
			createText(buff_test.second, imageCoordToTextCoord(buffImageMotion.position) + vec2(buffImageMotion.scale.x, 0) + vec2(BUFF_POPUP_GAP, -25), { 1.0f, 1.0f, 1.0f }, 0.3),
			buffImage,
			POPUP_DURATION
		)
	);

	return buffPopup;
}

vec2 imageCoordToTextCoord(vec2 imageCoord)
{
	return vec2(imageCoord.x, -imageCoord.y) + vec2(WINDOW_WIDTH_PX / 2, WINDOW_HEIGHT_PX / 2);
}
