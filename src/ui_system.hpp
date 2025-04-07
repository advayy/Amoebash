#pragma once

#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"

void removeInfoBoxes();
void createInfoBoxes();
Entity createMiniMap(RenderSystem *renderer, vec2 size);

Entity createStartScreen(vec2 position = LOGO_POSITION_INITIAL);
Entity createShopScreen();
Entity createInfoScreen();
Entity createGameOverScreen();
Entity createNucleusMenuNucleus();
Entity createNucleusMenuSlot(vec2 position, int slotNumber);
Entity createNucleusMenuScreen();
Entity createPauseScreen();
Entity createClickableBuffUI(vec2 position, BUFF_TYPE buffType);

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
Entity createProceedButton(vec2 position);
Entity createResumeButton(vec2 position);
Entity createSaveButton(vec2 position);
Entity createUIElement(vec2 position, vec2 scale, TEXTURE_ASSET_ID texture_id, EFFECT_ASSET_ID effect_id);
Entity createHealthBar();
Entity createThermometer();
void createDashRecharge();
Entity createEnemyHPBar(Entity enemy, TEXTURE_ASSET_ID texture_id);
void removeEnemyHPBar(Entity enemy);

Entity createBuffUI(vec2 position, BUFF_TYPE type, vec2 scale);
Entity createRowBuffUI(vec2 position, BUFF_TYPE type);
Entity createPopupBuffUI(vec2 position, BUFF_TYPE type);
void renderCollectedBuff(RenderSystem *renderer, BUFF_TYPE buffType);

Entity createText(std::string text, vec2 start_pos, vec3 color, float scale);

void updateHuds();
vec2 getBuffSlot (BUFF_TYPE buffType);
void removeBuffUI(BUFF_TYPE buffType);
vec2 getBuffSlot_uiPos (BUFF_TYPE buffType);
void findAndRemove(std::unordered_map<BUFF_TYPE, int>& map, BUFF_TYPE N);

Entity createShopKeeper();
Entity createShopBox();
Entity createShopPlate(vec2 pos);
Entity createClickableShopBuff(vec2 position, BUFF_TYPE buffType);

void updatePopups(float elapsed_ms_since_last_update);
void removePopups(std::function<bool(Entity&)> shouldRemove);
Entity createBuffPopup(BUFF_TYPE type);
vec2 imageCoordToTextCoord(vec2 imageCoord);