#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/tiny_ecs.hpp"

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count>  texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	// const std::vector<std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths = {
	// 	std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::CHICKEN, mesh_path("chicken.obj"))
	// 	// specify meshes of other assets here
	// };

	// Make sure these paths remain in sync with the associated enumerators (see TEXTURE_ASSET_ID).
	const std::array<std::string, texture_count> texture_paths = {
		textures_path("enemies/spike.png"),
		textures_path("amoeba/player_dash.png"),
		textures_path("projectiles/gold_bubble.png"),
		textures_path("tiles/test_tile.png"),
		textures_path("tiles/paralax_tile_1_128x.png"),
		textures_path("ui_art/amoebash_logo.png"),
		textures_path("ui_art/gameOver.png"),
		textures_path("ui_art/start.png"),
		textures_path("ui_art/pausescreen.png"),
		textures_path("ui_art/shop.png"),
		textures_path("ui_art/nucleus_full_size.png"),
		textures_path("ui_art/shopscreen.png"),
		textures_path("ui_art/infoscreen.png"),
		textures_path("tiles/wall_tile.png"),
		textures_path("transition_animations/noses_spritesheet.png"),
		textures_path("transition_animations/into_game_transition_sheet.png"),
		textures_path("transition_animations/nose_accent_spritesheet.png"),
		textures_path("transition_animations/nucleus_entering_nose_sheet.png")
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
		shader_path("minimap")
	};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(GLFWwindow* window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();

	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();

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

	mat3 createProjectionMatrix();

	Entity get_screen_state_entity() { return screen_state_entity; }

private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3& projection);
	void drawSpriteSheetTexturedMesh(Entity entity, const mat3& projection);
	void drawToScreen();

	void setUpDefaultProgram(Entity& entity, const RenderRequest& render_request, const GLuint program);
	void setUpSpriteSheetTexture(Entity& entity, const GLuint program);

	void drawScreenAndButtons(ScreenType screenType, const std::vector<ButtonType>& buttonTypes);



	// Window handle
	GLFWwindow* window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
