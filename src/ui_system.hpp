#pragma once

#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"


class UISystem {
public:
	void removeInfoBoxes();
	void createInfoBoxes();
	Entity createMiniMap(RenderSystem* renderer, vec2 size);

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
	void createGermoneyText();

	Entity createBuffUI(vec2 position, BUFF_TYPE type, vec2 scale);
	Entity createRowBuffUI(BUFF_TYPE type);
	Entity createPopupBuffUI(vec2 position, BUFF_TYPE type);
	Entity createBuffPopup(BUFF_TYPE type);
	vec2 imageCoordToTextCoord(vec2 imageCoord);

	void cameraTrackUIElements();

	void step(float elapsed_ms_since_last_update);
	void updateThermometerText();
	void updateGermoneyText();
	void renderCollectedBuffs();
	// updates popup timers and removes if expired
	void updatePopups(float elapsed_ms_since_last_update);
	// removes popups from the registry based on if they satisfy the predicate provided
	void removePopups(std::function<bool(Entity&)> shouldRemove);

	Entity createText(std::string text, vec2 start_pos, vec3 color, float scale);
	vec2 getBuffSlot(int buffType);
	void removeBuffUI(int buffType);
	vec2 getBuffSlot_uiPos(int buffType);
	void findAndRemove(std::unordered_map<BUFF_TYPE, int>& map, int N);
	vec2 worldToScreen(vec2 world_pos);
private:

	Entity thermometer_text;
	void setThermometerText(Entity& entity) { this->thermometer_text = entity; };

	Entity germoney_text;
	void setGermoneyText(Entity& entity) { this->germoney_text = entity; };

	std::map<BUFF_TYPE, Entity> buff_to_ui_entity;
	std::map<BUFF_TYPE, Entity> buff_to_text_entity;

	const std::map<BUFF_TYPE, std::pair<std::string, std::string>> BUFF_TYPE_TO_TEXT =
	{
		{TAIL, {"Flagella", "Gives you a 5% speed boost"}},
		{MITOCHONDRIA, {"Mitochondria", "Reduces Dash cooldown by 5%"}},
		{HEMOGLOBIN, {"Hemoglobin", "Reduces enemy detection range by 5%"}},
		{GOLGI, {"Golgi Apparatus", "Increases your health by 10"}},
		{CHLOROPLAST, {"Chloroplast", "Increases your healing rate"}},
		{CELL_WALL, {"Cell Wall", "Negate the next time you take damage"}},
		{AMINO_ACID, {"Amino Acid", "Increases your Dash damage"}},
		{LYSOSOME, {"Lysosome", "Shoot 1 more projectile"}},
		{CYTOPLASM, {"Cytoplasm", "Increases your health by 10"}},
		{PILLI, {"Pilli", "Projectile Speed Bost"}},
		{SPARE_NUCLEUS, {"Spare Nucleus", "1+ Lives"}},
		{VACUOLE, {"Vacuole", "Heals some health instantly"}},
		{ENDOPLASMIC_RETICULUM, {"Endoplasmic Reticulum", "Not Implemented"}},
		{OVOID, {"Oceloid", "Increases mini-map view range"}},
		{SECRETOR, {"Secretor", "Increases dash drift"}},
		{UNNAMED, {"Pilli", "Not Implemented"}},
		{PEROXISOMES, {"Pilli", "Not Implemented"}},
		{MUTATION, {"Pilli", "Not Implemented"}},
		{FACEHUGGER, {"Pilli", "Not Implemented"}},
		{BLACK_GOO, {"Pilli", "Not Implemented"}},
	};
};

Entity createEnemyHPBar(Entity enemy, TEXTURE_ASSET_ID texture_id);
void removeEnemyHPBar(Entity enemy);
