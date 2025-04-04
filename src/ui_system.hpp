#pragma once

#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"

void removeInfoBoxes();
void createInfoBoxes();
Entity createMiniMap(RenderSystem *renderer, vec2 size);

Entity createStartScreen(vec2 position = LOGO_POSITION_INITIAL);
Entity createShopScreen();
Entity createInfoScreen();
Entity createNextButton(vec2 position);
Entity createGameOverScreen();
Entity createNucleusMenuNucleus();
Entity createNucleusMenuSlot(vec2 position, int slotNumber);
Entity createNucleusMenuScreen();
Entity createPauseScreen();
Entity createClickableBuffUI(vec2 position, int buffType);

void createGameplayCutScene();
Entity createEndingWinScene();
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

Entity createButton(ButtonType type, vec2 position, vec2 scale, TEXTURE_ASSET_ID texture);
Entity createStartButton();
Entity createShopButton();
Entity createInfoButton();
Entity createBackButton();

Entity createUIElement(vec2 position, vec2 scale, TEXTURE_ASSET_ID texture_id, EFFECT_ASSET_ID effect_id);
Entity createHealthBar();
Entity createThermometer();
void createDashRecharge();

Entity createBuffUI(vec2 position, int type);
void renderCollectedBuff(RenderSystem *renderer, int buffType);

void updateHuds();
vec2 getBuffSlot (int buffType);
void removeBuffUI(int buffType);
vec2 getBuffSlot_uiPos (int buffType);
void findAndRemove(std::unordered_map<int, int>& map, int N);
