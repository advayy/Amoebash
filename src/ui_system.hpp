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

Entity createUIElement(vec2 position, vec2 scale, TEXTURE_ASSET_ID texture_id, EFFECT_ASSET_ID effect_id);
Entity createHealthBar();
void createDashRecharge();

Entity createBuffUI(vec2 position, BUFF_TYPE type, vec2 scale);
void renderCollectedBuff(RenderSystem *renderer, BUFF_TYPE buffType, int index);
Entity createBuffPopup(BUFF_TYPE type);
vec2 imageCoordToTextCoord(vec2 imageCoord);

void updateHuds();
// updates popup timers and removes if expired
void updatePopups(float elapsed_ms_since_last_update);
// removes popups from the registry based on if they satisfy the predicate provided
void removePopups(std::function<bool(Entity&)> shouldRemove);

Entity createText(std::string text, vec2 start_pos, vec3 color, float scale);

const std::map<BUFF_TYPE, std::pair<std::string, std::string>> BUFF_TYPE_TO_TEXT =
{
	{TAIL, {"Tail", "Gives you a 5% speed boost"}},
	{MITOCHONDRIA, {"Mitochondria", "Reduces Dash cooldown by 5%"}},
	{HEMOGLOBIN, {"Hemoglobin", "Reduces enemy detection range by 5%"}},
	{GOLGI, {"Golgi Apparatus", "Increases your health by 10"}},
	{CHLOROPLAST, {"Chloroplast", "Increases your healing rate"}},
	{CELL_WALL, {"Cell Wall", "Not Implemented"}},
	{AMINO_ACID, {"Amino Acid", "Increases your Dash damage"}},
	{LYSOSOME, {"Lysosome", "Unimplemented"}},
	{CYTOPLASM, {"Cytoplasm", "Increases your health by 10"}},
	{PILLI, {"Pilli", "Not Implemented"}},
	{SPARE_NUCLEUS, {"Pilli", "Not Implemented"}},
	{VACUOLE, {"Pilli", "Not Implemented"}},
	{ENDOPLASMIC_RETICULUM, {"Pilli", "Not Implemented"}},
	{OVOID, {"Pilli", "Not Implemented"}},
	{SECRETOR, {"Pilli", "Not Implemented"}},
	{UNNAMED, {"Pilli", "Not Implemented"}},
	{PEROXISOMES, {"Pilli", "Not Implemented"}},
	{MUTATION, {"Pilli", "Not Implemented"}},
	{FACEHUGGER, {"Pilli", "Not Implemented"}},
	{BLACK_GOO, {"Pilli", "Not Implemented"}},
};