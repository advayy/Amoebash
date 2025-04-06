#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/tiny_ecs.hpp"

// fonts
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include <glm/gtc/type_ptr.hpp>

// fonts
struct Character {
	unsigned int TextureID;  // ID handle of the glyph texture
	glm::ivec2   Size;       // Size of glyph
	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
	char character;
};

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem
{
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	// const std::vector<std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths = {
	// 	std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::CHICKEN, mesh_path("chicken.obj"))
	// 	// specify meshes of other assets here
	// };

	// M1 Feature - Basic integrated assets

	// Make sure these paths remain in sync with the associated enumerators (see TEXTURE_ASSET_ID).
	const std::array<std::string, texture_count> texture_paths = {
		textures_path("enemies/spike.png"),
		textures_path("enemies/rbc.png"),
		textures_path("enemies/bacteriophage.png"),
		textures_path("amoeba/player_dash.png"),
		textures_path("projectiles/bacteriophage_projectile.png"),
		textures_path("tiles/test_tile.png"),
		textures_path("tiles/parallax_tile_1_128x.png"),
		textures_path("ui_art/amoebash_logo.png"),
		textures_path("ui_art/gameOver.png"),
		textures_path("ui_art/button_outline.png"),
		textures_path("ui_art/pausescreen.png"),
		textures_path("ui_art/shop_button.png"),
		textures_path("ui_art/shop_button_on_hover.png"),
		textures_path("ui_art/nucleus_full_size.png"),
		textures_path("ui_art/shopscreen.png"),
		textures_path("ui_art/infoscreen.png"),
		textures_path("tiles/wall_tile.png"),
		textures_path("transition_animations/noses_spritesheet.png"),
		textures_path("transition_animations/into_game_transition_sheet.png"),
		textures_path("transition_animations/nose_accent_spritesheet.png"),
		textures_path("transition_animations/nucleus_entering_nose_sheet.png"),
		textures_path("ui_art/HUD_outlined_nucleus_full_size.png"),
		textures_path("ui_art/HUD_health_bar.png"),
		textures_path("ui_art/HUD_dash_component_clear.png"),
		textures_path("ui_art/HUD_germoney_hud.png"),
		textures_path("ui_art/HUD_weapons_pill.png"),
		textures_path("ui_art/info_button.png"),
		textures_path("ui_art/info_button_on_hover.png"),
		textures_path("ui_art/start_screen.png"),
		textures_path("ui_art/button_outline.png"),
		textures_path("projectiles/key.png"),
		textures_path("buffs/buffs_sheet.png"),
        textures_path("tiles/whirlpool_portal.png"),
		textures_path("tutorial/mouse_control.png"),
		textures_path("tutorial/pause_info.png"),
		textures_path("tutorial/dash_info.png"),
		textures_path("tutorial/enemy_info.png"),
		textures_path("tutorial/restart_info.png"),
		textures_path("tutorial/leave.png"),
		textures_path("projectiles/chest.png"),
		textures_path("effects/germoney.png"),
		textures_path("effects/pixel_particle.png"),
		textures_path("weapons/gun.png"),
		textures_path("weapons/pet_bacteriophage_still.png"),
		textures_path("weapons/gun_projectile.png"),
		textures_path("enemies/boss/boss_projectile.png"),
		textures_path("nucleus_menu/nucleus_menu_nucleus.png"),
		textures_path("nucleus_menu/nucleus_menu_slot_34x34.png"),
		textures_path("enemies/boss/mitosis_boss_128_transparent.png"),
		textures_path("enemies/boss/mitosis_boss_64_transparent.png"),
		textures_path("enemies/boss/mitosis_boss_32_transparent.png"),
		textures_path("enemies/boss/mitosis_boss_16_transparent.png"),
		textures_path("ui_art/start_button.png"),
		textures_path("ui_art/start_button_on_hover.png"),
		textures_path("ui_art/back_button.png"),
		textures_path("ui_art/back_button_on_hover.png"),
		textures_path("ui_art/proceed_button.png"), // exit button
		textures_path("ui_art/proceed_button_on_hover.png"), // exit button
		textures_path("ui_art/save_button.png"),
		textures_path("ui_art/save_button_on_hover.png"),
		textures_path("ui_art/resume_button.png"),
		textures_path("ui_art/resume_button_on_hover.png"),
		textures_path("enemies/boss/boss_arrow.png"),
		textures_path("ui_art/victory_cutscene.png"),
		textures_path("ui_art/thermometer_alone.png"),
		//textures_path("ui_art/circle.png"),
		textures_path("ui_art/enemy_hp_bar.png"),
		textures_path("ui_art/mitosis_boss_16_enemy_hp_bar.png"),
		textures_path("ui_art/mitosis_boss_128_enemy_hp_bar.png"),
	};
	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("line"),
		shader_path("textured"),
		shader_path("vignette"),
		shader_path("sprite_sheet_textured"),
		shader_path("tile"),
		shader_path("minimap"),
		shader_path("ui"),
		shader_path("health_bar"),
		shader_path("dash_ui"),
		shader_path("hexagon"),
		shader_path("particle_textured"),
        shader_path("font"),
		shader_path("thermometer"),
		shader_path("weapon_cooldown_indicator")
	};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(GLFWwindow *window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();

	GLuint getEffect(EFFECT_ASSET_ID id)
	{
		return effects[(int)id];
	}

	Mesh &getMesh(GEOMETRY_BUFFER_ID id)
	{
		return meshes[(int)id];
	};

	void initializeGlGeometryBuffers();
	std::vector<TexturedVertex> loadMeshVertices(const std::string &filename);

	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the vignette shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw();

	// Screen specific functionalities
	void drawStartScreen();
	void drawGameOverScreen();
	void drawShopScreen();
	void drawInfoScreen();
	void drawCutScreneAnimation();
	void drawNextLevelScreen();

	mat3 createProjectionMatrix();

	// FPS counter related methods
	void updateFPS(float elapsed_ms);
	void toggleFPSDisplay();
	void drawFPS();

	Entity get_screen_state_entity()
	{
		return screen_state_entity;
	};

	void drawUI(Entity entity, const mat3 &projection);
	void drawUIElements();
	void drawDashRecharge(const mat3 &projection);
	void drawHexagon(Entity entity, const mat3 &projection);
	void drawBuffUI();
	void drawGunCooldownIndicator(const vec2& camera_position, const mat3& projection);


	// INSTANCING: instanced particle drawing
	void drawInstancedParticles();
	void drawParticlesByTexture(TEXTURE_ASSET_ID texture_id);


	// INSTANCING: instanced tile drawing
	void drawInstancedTiles(const mat3 &projection);

private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3 &projection);
	void drawSpriteSheetTexturedMesh(Entity entity, const mat3 &projection);
	void drawToScreen();
	void drawToScreen(float darken_screen_factor);

	void setUpDefaultProgram(Entity &entity, const RenderRequest &render_request, const GLuint program);
	void setUpSpriteSheetTexture(Entity &entity, const GLuint program);

	void drawScreenAndButtons(ScreenType screenType, const std::vector<ButtonType> &buttonTypes);

	// Window handle
	GLFWwindow *window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;

	// FPS counter variables
	float frame_time_sum = 0.0f;
	int frame_count = 0;
	float current_fps = 0.0f;
	bool show_fps = true; // Start with FPS display enabled

	// INSTANCING: Particle effect shader
	GLuint particle_effect;

	// INSTANCING instance buffer to hold per-particle transform matrices
	GLuint particle_instance_vbo;

	// INSTANCING: Default VAO for rendering
	GLuint default_vao;

	// INSTANCING: Store the sprite index count
	GLsizei sprite_index_count;

	// INSTANCING: Instance VBO for tiles
	GLuint tile_instance_vbo;

    // freetype font rendering
    bool fontInit(GLFWwindow& window, const std::string& font_filename, unsigned int font_default_size);
    void drawText(Entity entity);
    
    // freetype font rendering
	std::map<char, Character> m_ftCharacters;
	GLuint m_font_shaderProgram;
	GLuint m_font_VAO;
	GLuint m_font_VBO;
};

bool loadEffectFromFile(
		const std::string &vs_path, const std::string &fs_path, GLuint &out_program);
