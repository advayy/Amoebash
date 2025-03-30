#include "ui_system.hpp"
#include "world_init.hpp"
#include "tinyECS/registry.hpp"
#include <random>
#include <iostream>

// create info boxes in tutorial mode
void createInfoBoxes() {

	TEXTURE_ASSET_ID baseTexture = TEXTURE_ASSET_ID::MOUSE_CONTROL_INFO;

	for (int i = 0; i < 6; i ++) {
		auto entity1 = Entity();
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

// remove info boxes in tutorial mode
void removeInfoBoxes() {
	std::vector<Entity> entityList;

	for (auto e : registry.infoBoxes.entities) {
		entityList.push_back(e);
	}

    int size = entityList.size();
    for(int i = 0; i < size; i++) {
        registry.remove_all_components_of(entityList[i]);
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
	MiniMap& m = registry.miniMaps.emplace(entity);
	m.visited = std::vector<std::vector<int>>(MAP_HEIGHT, std::vector<int>(MAP_WIDTH, 0));

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
	vec2 position = WORLD_ORIGIN;
	vec2 scale = BACKGROUND_SCALE;

	motion.position = position;
	motion.scale = scale;

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

	Entity nextButtonEntity = createNextButton(vec2({origin_position.x + 180, origin_position.y + NUCLEUS_MENU_NUCLEUS_HEIGHT}));
	o.buttons = {nextButtonEntity};


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

	for(int i = 0; i < p.buffsFromLastRun.size(); i++) {

		registry.overs.emplace(createClickableBuffUI(currentPos, p.buffsFromLastRun[i]));
		currentPos.y += padding + BUFF_HEIGHT;

		if((currentPos.y + padding + BUFF_HEIGHT) >= screenBottom) // forecast next position will fit otherwise shift start right and update current to start...
		{
			startPos.x += padding + BUFF_WIDTH;
			currentPos = startPos;
		}
	}

	return nucleusMenuScreen;
}

// create the buff UI for carry on
Entity createClickableBuffUI(vec2 position, int buffType)
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

	vec2 scale = {LOGO_WIDTH_PX, LOGO_HEIGHT_PX};

	Camera &camera = registry.cameras.get(registry.cameras.entities[0]);
	motion.position = camera.position;
	motion.scale = scale;

	vec2 saveButtonPosition = camera.position + vec2(0, 120.f);
	vec2 resumeButtonPosition = camera.position + vec2(0, 200.f);

	Entity saveButtonEntity = createButton(ButtonType::SAVEBUTTON, saveButtonPosition, BACK_BUTTON_SCALE, TEXTURE_ASSET_ID::SAVE_BUTTON);
	Entity resumeButtonEntity = createButton(ButtonType::RESUMEBUTTON, resumeButtonPosition, BACK_BUTTON_SCALE, TEXTURE_ASSET_ID::RESUME_BUTTON);

	
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
	if (registry.shops.size() == 0)
		return;
	
	Entity shop_entity = registry.shops.entities[0];
	Shop &shop = registry.shops.components[0];
	std::vector<Entity> buttons_to_remove = shop.buttons;
	
    int size = buttons_to_remove.size();

    for (int i = 0; i < size; i++) {
        registry.remove_all_components_of(buttons_to_remove[i]);
    }
	registry.remove_all_components_of(shop_entity);
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

Entity createNextButton(vec2 position) {
	vec2 scale = BACK_BUTTON_SCALE;

	return createButton(ButtonType::PROCEED_BUTTON,
						position,
						scale,
						TEXTURE_ASSET_ID::BACK_BUTTON);
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
	Player &player = registry.players.get(registry.players.entities[0]);
	vec2 playerPos = registry.motions.get(registry.players.entities[0]).position;

	for (int i = 0; i < DASH_RECHARGE_COUNT; i++)
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
}

Entity createBuffUI(vec2 position, int type)
{
	Entity buffUI = Entity();

	BuffUI &buff = registry.buffUIs.emplace(buffUI);
	buff.buffType = type;

	Motion &motion = registry.motions.emplace(buffUI);
	motion.position = position;
	motion.scale = {BUFF_UI_WIDTH, BUFF_UI_HEIGHT};

	registry.renderRequests.insert(buffUI,
								   {TEXTURE_ASSET_ID::BUFFS_SHEET,
									EFFECT_ASSET_ID::SPRITE_SHEET,
									GEOMETRY_BUFFER_ID::SPRITE});

	SpriteSheetImage &spriteSheet = registry.spriteSheetImages.emplace(buffUI);
	spriteSheet.total_frames = 20;	 
	spriteSheet.current_frame = type;
								
	SpriteSize &sprite = registry.spritesSizes.emplace(buffUI);
	sprite.width = BUFF_UI_WIDTH;
	sprite.height = BUFF_UI_HEIGHT;
	
	registry.uiElements.emplace(buffUI, UIElement{motion.position, motion.scale});
	
	return buffUI;
}

// Render collected buffs a certain amount per row that stacks
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

// HUD element update such has health etc.
void updateHuds()
{
	vec2 offset = {WINDOW_WIDTH_PX / 2 - 100, -WINDOW_HEIGHT_PX / 2 + 100};

	Entity minimapEntity = registry.miniMaps.entities[0];
	Motion &minimapMotion = registry.motions.get(minimapEntity);

	Camera &camera = registry.cameras.get(registry.cameras.entities[0]);
	minimapMotion.position = {camera.position.x + offset.x, camera.position.y + offset.y};

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

	if (!registry.healthBars.entities.empty())
	{
		HealthBar &healthBar = registry.healthBars.get(registry.healthBars.entities[0]);
		Motion &healthBarMotion = registry.motions.get(registry.healthBars.entities[0]);
		healthBarMotion.position = {camera.position.x + HEALTH_BAR_POS.x,
									camera.position.y + HEALTH_BAR_POS.y};

	}
}

