#include <SDL.h>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <iomanip>
#include <unordered_map>

// internal
#include "render_system.hpp"
#include "tinyECS/registry.hpp"
#include "world_system.hpp"
#include <sstream>
#include <iomanip>

void RenderSystem::updateFPS(float elapsed_ms)
{
	// skip all calculations if FPS display is not enabled
	if (!show_fps)
		return;

	// update frame time sum and count
	frame_time_sum += elapsed_ms;
	frame_count++;

	// update FPS calculation every second (1000ms)
	if (frame_time_sum >= 1000.0f)
	{
		current_fps = static_cast<float>(frame_count) / (frame_time_sum / 1000.0f);
		frame_time_sum = 0.0f;
		frame_count = 0;

		// update window title with FPS
		std::stringstream title;
		title << "Amoebash (Debug: ON, FPS: " << std::fixed << std::setprecision(1) << current_fps << ")";
		glfwSetWindowTitle(window, title.str().c_str());
	}
}

void RenderSystem::toggleFPSDisplay()
{
	show_fps = !show_fps;

	// reset window title when FPS display is turned off
	if (!show_fps)
	{
		glfwSetWindowTitle(window, "Amoebash");
	}
}

void RenderSystem::drawFPS()
{
	if (!show_fps)
		return;

	// keeping this code in case we want to render the FPS on the screen instead
	// of in the window title in the future.
}

void RenderSystem::drawTexturedMesh(Entity entity,
																		const mat3 &projection)
{
	assert(registry.renderRequests.has(entity));
	const RenderRequest &render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	assert((render_request.used_effect == EFFECT_ASSET_ID::TEXTURED ||
					render_request.used_effect == EFFECT_ASSET_ID::SPRITE_SHEET ||
					render_request.used_effect == EFFECT_ASSET_ID::MINI_MAP ||
					render_request.used_effect == EFFECT_ASSET_ID::TILE ||
					render_request.used_effect == EFFECT_ASSET_ID::UI ||
					render_request.used_effect == EFFECT_ASSET_ID::HEALTH_BAR ||
					render_request.used_effect == EFFECT_ASSET_ID::DASH_UI ||
					render_request.used_effect == EFFECT_ASSET_ID::THERMOMETER_EFFECT) &&
				 "Type of render request not supported");

	setUpDefaultProgram(entity, render_request, program);

	if (render_request.used_effect == EFFECT_ASSET_ID::MINI_MAP)
	{
		GLint map_width_uloc = glGetUniformLocation(program, "map_width");
		GLint map_height_uloc = glGetUniformLocation(program, "map_height");
		GLint player_grid_position_uloc = glGetUniformLocation(program, "player_grid_position");

		glUniform1i(map_width_uloc, MAP_WIDTH);
		glUniform1i(map_height_uloc, MAP_HEIGHT);

		// get player position
		Player &player = registry.players.get(registry.players.entities[0]);
		glUniform2fv(player_grid_position_uloc, 1, (float *)&player.grid_position);

		// map array logic
		GLint map_array_uloc = glGetUniformLocation(program, "map_array");
		std::vector<std::vector<tileType>> map_array = registry.proceduralMaps.get(registry.proceduralMaps.entities[0]).map;
		std::vector<int> flat_array;
		flat_array.reserve(MAP_WIDTH * MAP_HEIGHT);

		for (const auto &row : map_array)
		{
			for (const auto &tile : row)
			{
				flat_array.push_back(static_cast<int>(tile));
			}
		}
		glUniform1iv(map_array_uloc, MAP_WIDTH * MAP_HEIGHT, flat_array.data());
		glUniform2fv(player_grid_position_uloc, 1, (float *)&player.grid_position);

		// PASS IN MINIMAP VISITED
		GLint map_visited_array_uloc = glGetUniformLocation(program, "map_visited_array");
		std::vector<std::vector<int>> map_visited_array = registry.miniMaps.get(registry.miniMaps.entities[0]).visited;
		std::vector<int> flat_visited_array;
		flat_visited_array.reserve(MAP_WIDTH * MAP_HEIGHT);

		for (const auto &row : map_visited_array)
		{
			for (const auto &visited : row)
			{
				flat_visited_array.push_back(static_cast<int>(visited));
			}
		}

		glUniform1iv(map_visited_array_uloc, MAP_WIDTH * MAP_HEIGHT, flat_visited_array.data());
	}

	if (render_request.used_effect == EFFECT_ASSET_ID::THERMOMETER_EFFECT) {
		// FEED THE VALUES FOR CURRENT DANGER LEVEL, AND MAX DANGER LEVEL
		// RANGES FROM GREEN TO PURPLE (Green->Yellow->Orange->Red->Pink->Purple)
		float maxDangerLevel = MAX_DANGER_LEVEL;
		float current_danger_level = registry.players.get(registry.players.entities[0]).dangerFactor;

		// ULOCS TO PASS 
		// uniform float max_danger;
		// uniform float current_danger;
		GLint max_danger_uloc = glGetUniformLocation(program, "max_danger");
		glUniform1f(max_danger_uloc, (float)maxDangerLevel);
		gl_has_errors();

		GLint current_danger_uloc = glGetUniformLocation(program, "current_danger");
		glUniform1f(current_danger_uloc, (float)current_danger_level);
		gl_has_errors();
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	if (render_request.used_effect == EFFECT_ASSET_ID::SPRITE_SHEET || render_request.used_effect == EFFECT_ASSET_ID::TILE)
	{
		setUpSpriteSheetTexture(entity, program);
	}

	if (render_request.used_effect == EFFECT_ASSET_ID::TILE)
	{
		// also take vec2 camera position
		Camera &camera = registry.cameras.get(registry.cameras.entities[0]);
		vec2 cameraPos = camera.position;
		GLint camera_position_uloc = glGetUniformLocation(program, "camera_position");
		glUniform2fv(camera_position_uloc, 1, (float *)&cameraPos);
	}

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);

	if (registry.motions.has(entity))
	{
		Motion &motion = registry.motions.get(entity);
		// Transformation code, see Rendering and Transformation in the template
		// specification for more info Incrementally updates transformation matrix,
		// thus ORDER IS IMPORTANT
		Transform transform;
		transform.translate(motion.position);
		transform.scale(motion.scale);
		transform.rotate(radians(motion.angle));

		// Setting uniform values to the currently bound program
		GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
		glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
		gl_has_errors();
	}

	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

void RenderSystem::setUpSpriteSheetTexture(Entity &entity, const GLuint program)
{
	// SpriteSheet UNIFORMS see :
	// 	int total_frames from SpriteSheetImage
	// 	int current_frame from SpriteSheetImage
	// 	int sprite size from sprite
	GLint total_frames_uloc = glGetUniformLocation(program, "total_frames");
	GLint current_frame_uloc = glGetUniformLocation(program, "current_frame");
	GLint sprite_width_uloc = glGetUniformLocation(program, "sprite_width");
	GLint sprite_height_uloc = glGetUniformLocation(program, "sprite_height");

	SpriteSheetImage &spriteSheet = registry.spriteSheetImages.get(entity);
	glUniform1i(total_frames_uloc, spriteSheet.total_frames);
	glUniform1i(current_frame_uloc, spriteSheet.current_frame);
	SpriteSize &sprite = registry.spritesSizes.get(entity);
	glUniform1i(sprite_width_uloc, sprite.width);
	glUniform1i(sprite_height_uloc, sprite.height);
}

void RenderSystem::setUpDefaultProgram(Entity &entity, const RenderRequest &render_request, const GLuint program)
{
	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	GLint in_position_loc = glGetAttribLocation(program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
	gl_has_errors();
	assert(in_texcoord_loc >= 0);

	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
												sizeof(TexturedVertex), (void *)0);
	gl_has_errors();

	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
					vec3)); // note the stride to skip the preceeding vertex position

	// Enabling and binding texture to slot 0
	glActiveTexture(GL_TEXTURE0);
	gl_has_errors();

	assert(registry.renderRequests.has(entity));
	GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

	glBindTexture(GL_TEXTURE_2D, texture_id);
	gl_has_errors();
}

// first draw to an intermediate texture,
// apply the "vignette" texture, when requested
// then draw the intermediate texture
void RenderSystem::drawToScreen()
{
	// Setting shaders
	// get the vignette texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::VIGNETTE]);
	gl_has_errors();

	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
			GL_ELEMENT_ARRAY_BUFFER,
			index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
																																	 // indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();

	// add the "vignette" effect
	const GLuint vignette_program = effects[(GLuint)EFFECT_ASSET_ID::VIGNETTE];

	// set clock
	GLuint time_uloc = glGetUniformLocation(vignette_program, "time");
	GLuint dead_timer_uloc = glGetUniformLocation(vignette_program, "darken_screen_factor");
	GLuint vignette_timer_uloc = glGetUniformLocation(vignette_program, "vignette_screen_factor");

	glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));

	ScreenState &screen = registry.screenStates.get(screen_state_entity);
	// std::cout << "screen.darken_screen_factor: " << screen.darken_screen_factor << " entity id: " << screen_state_entity << std::endl;
	glUniform1f(dead_timer_uloc, screen.darken_screen_factor);
	glUniform1f(vignette_timer_uloc, screen.vignette_screen_factor);
	gl_has_errors();

	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(vignette_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();

	// Draw
	glDrawElements(
			GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
			nullptr); // one triangle = 3 vertices; nullptr indicates that there is
								// no offset from the bound index buffer
	gl_has_errors();
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw()
{
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();

	// clear backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);

	// white background
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
														// and alpha blending, one would have to sort
														// sprites back to front
	gl_has_errors();

	mat3 projection_2D = createProjectionMatrix();

	// Draw all tiles first
	drawInstancedTiles(projection_2D);

	// draw portal
	for (Entity entity : registry.portals.entities)
	{
		if (registry.renderRequests.has(entity))
			drawTexturedMesh(entity, projection_2D);
	}

	for (Entity entity : registry.renderRequests.entities)
	{
		// Skip entities that have a Particle component, Particles are drawn using instancing
		if (registry.particles.has(entity))
			continue;

		if (registry.tiles.has(entity))
			continue;

		// don't draw boss Arrows if not supposed to draw
		if (registry.bossArrows.has(entity))  {
			continue;
		}

		if (registry.keys.has(entity) || registry.chests.has(entity))
		{
			drawHexagon(entity, projection_2D);
		}
		else if ((registry.motions.has(entity) || !registry.spriteSheetImages.has(entity)) && !registry.tiles.has(entity) && !registry.gameScreens.has(entity) && !registry.miniMaps.has(entity) && !registry.portals.has(entity) && !registry.guns.has(entity))
		{
			drawTexturedMesh(entity, projection_2D);
		}
	}

	// draw gun
	for (Entity entity : registry.guns.entities)
	{
		if (registry.renderRequests.has(entity))
			drawTexturedMesh(entity, projection_2D);
	}

	// draw the mini map
	drawTexturedMesh(registry.miniMaps.entities[0], projection_2D);
	drawTexturedMesh(registry.thermometers.entities[0], projection_2D);

	// draw static ui elemments
	for (Entity entity : registry.uiElements.entities)
	{
		drawTexturedMesh(entity, projection_2D);
	}

	// draw the health bar
	for (Entity entity : registry.healthBars.entities)
	{
		drawHealthBar(entity, projection_2D);
	}
	for (Entity entity : registry.bossArrows.entities)
	{
		BossArrow &arrow = registry.bossArrows.get(entity);
		if(arrow.draw) {
			Motion& arrowMotion = registry.motions.get(entity);
			
			Motion& playerMotion = registry.motions.get(registry.players.entities[0]);
			drawTexturedMesh(entity, projection_2D);
		}
	}
	// draw dash charges
	drawDashRecharge(projection_2D);

	if (registry.pauses.size() != 0)
	{
		auto &pause = registry.pauses.entities[0];
		drawTexturedMesh(pause, projection_2D);
	}

	// INSTANCING: Draw instanced particles
	drawInstancedParticles();

	// draw framebuffer to screen
	// adding "vignette" effect when applied
	drawToScreen();

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

mat3 RenderSystem::createProjectionMatrix()
{
	Camera &camera = registry.cameras.get(registry.cameras.entities[0]);
	vec2 cameraPos = camera.position;

	float left = cameraPos.x - WINDOW_WIDTH_PX * 0.5f;
	float right = left + WINDOW_WIDTH_PX;
	float top = cameraPos.y - WINDOW_HEIGHT_PX * 0.5f;
	float bottom = top + WINDOW_HEIGHT_PX;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);

	return {
			{sx, 0.f, 0.f},
			{0.f, sy, 0.f},
			{tx, ty, 1.f}};
}

void RenderSystem::drawStartScreen()
{

	std::vector<ButtonType> buttons = {
			ButtonType::STARTBUTTON,
			ButtonType::SHOPBUTTON,
			ButtonType::INFOBUTTON};

	drawScreenAndButtons(ScreenType::START, buttons);
}

void RenderSystem::drawShopScreen()
{

	std::vector<ButtonType> buttons = {
			ButtonType::BACKBUTTON};

	drawScreenAndButtons(ScreenType::SHOP, buttons);
}

void RenderSystem::drawInfoScreen()
{

	std::vector<ButtonType> buttons = {
			ButtonType::BACKBUTTON};

	drawScreenAndButtons(ScreenType::INFO, buttons);
}

void RenderSystem::drawGameOverScreen()
{
	std::vector<ButtonType> buttons = {
			ButtonType::PROCEED_BUTTON}; // ACTS as a next button

	drawScreenAndButtons(ScreenType::GAMEOVER, buttons);
}

void RenderSystem::drawNextLevelScreen()
{
	std::vector<ButtonType> buttons = {};

	drawScreenAndButtons(ScreenType::NEXT_LEVEL, buttons);
}

void RenderSystem::drawScreenAndButtons(
		ScreenType screenType,
		const std::vector<ButtonType> &buttonTypes)
{

	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();

	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	gl_has_errors();

	mat3 projection_matrix = createProjectionMatrix();

	// draw background
	// draw logo
	// draw buttons

	if (screenType == ScreenType::START)
	{
		for (uint i = 0; i < registry.gameScreens.size(); i++)
		{
			const auto &screenComp = registry.gameScreens.components[i];
			if (screenComp.type == screenType)
			{
				Entity screenEntity = registry.gameScreens.entities[i];
				drawTexturedMesh(screenEntity, projection_matrix);
			}
		}
		for (uint i = 0; i < registry.starts.size(); i++)
		{
			Start &start = registry.starts.components[i];
			if (start.logo != Entity())
			{
				drawTexturedMesh(start.logo, projection_matrix);
			}
		}
	}

	if (screenType == ScreenType::GAMEOVER)
	{
		for (uint i = 0; i < registry.overs.entities.size(); i++)
		{
			Entity e = registry.overs.entities[i];
			if (registry.renderRequests.has(e))
			{
				drawTexturedMesh(e, projection_matrix);
			}
		}
	}

	if (buttonTypes.size() != 0)
	{

		for (uint i = 0; i < registry.buttons.components.size(); i++)
		{
			const screenButton &buttonComp = registry.buttons.components[i];

			for (auto bt : buttonTypes)
			{

				if (buttonComp.type == bt)
				{
					Entity buttonEntity = registry.buttons.entities[i];
					drawTexturedMesh(buttonEntity, projection_matrix);
					break;
				}
			}
		}
	}

	drawToScreen();
	glfwSwapBuffers(window);
	gl_has_errors();
}

void RenderSystem::drawCutScreneAnimation()
{

	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();

	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	gl_has_errors();

	mat3 projection_matrix = createProjectionMatrix();

	for (auto &entity : registry.cutscenes.entities)
	{
		drawTexturedMesh(entity, projection_matrix);
	}

	drawToScreen();
	glfwSwapBuffers(window);
	gl_has_errors();
}

void RenderSystem::drawUI(Entity entity, const mat3 &projection)
{
	Motion &motion = registry.motions.get(entity);
	Transform transform;
	transform.translate(motion.position);
	transform.scale(motion.scale);

	assert(registry.renderRequests.has(entity));
	const RenderRequest &render_request = registry.renderRequests.get(entity);

	GLuint program = effects[(GLuint)render_request.used_effect];
	glUseProgram(program);
	gl_has_errors();

	GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	GLint in_position_loc = glGetAttribLocation(program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)0);
	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)sizeof(vec3));

	glActiveTexture(GL_TEXTURE0);
	GLuint texture_id = texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];
	glBindTexture(GL_TEXTURE_2D, texture_id);
	gl_has_errors();

	GLuint transform_loc = glGetUniformLocation(program, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	gl_has_errors();

	GLuint projection_loc = glGetUniformLocation(program, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();

	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	GLsizei num_indices = size / sizeof(uint16_t);
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

void RenderSystem::drawUIElements()
{
	if (registry.uiElements.size() == 0)
		return;

	mat3 projection_2D = createProjectionMatrix();

	for (Entity entity : registry.uiElements.entities)
	{
		drawUI(entity, projection_2D);
	}

	drawToScreen();
	glfwSwapBuffers(window);
}

void RenderSystem::drawHexagon(Entity entity, const mat3 &projection)
{
	if (!registry.keys.has(entity) && !registry.chests.has(entity))
	{
		return;
	}

	if (!registry.renderRequests.has(entity))
	{
		return;
	}

	RenderRequest &render_request = registry.renderRequests.get(entity);
	GLuint program = effects[(GLuint)render_request.used_effect];
	glUseProgram(program);
	gl_has_errors();

	GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	GLint in_position_loc = glGetAttribLocation(program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");

	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)0);

	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)sizeof(vec3));
	gl_has_errors();

	glActiveTexture(GL_TEXTURE0);
	GLuint texture_id = texture_gl_handles[(GLuint)render_request.used_texture];
	glBindTexture(GL_TEXTURE_2D, texture_id);
	gl_has_errors();

	if (!registry.motions.has(entity))
	{
		return;
	}

	Motion &motion = registry.motions.get(entity);

	Transform transform;
	transform.translate(motion.position);
	transform.scale(motion.scale);

	GLuint transform_loc = glGetUniformLocation(program, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	gl_has_errors();

	GLuint projection_loc = glGetUniformLocation(program, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();

	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	GLsizei num_indices = size / sizeof(uint16_t);
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
}

void RenderSystem::drawHealthBar(Entity entity, const mat3 &projection)
{
	if (!registry.healthBars.has(entity))
		return;

	HealthBar &healthBar = registry.healthBars.get(entity);
	Motion &motion = registry.motions.get(entity);
	Player &player = registry.players.get(registry.players.entities[0]);

	Transform transform;
	transform.translate(motion.position);
	transform.scale(motion.scale);

	assert(registry.renderRequests.has(entity));
	const RenderRequest &render_request = registry.renderRequests.get(entity);

	GLuint program = effects[(GLuint)EFFECT_ASSET_ID::HEALTH_BAR];
	glUseProgram(program);
	gl_has_errors();

	GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	GLint in_position_loc = glGetAttribLocation(program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");

	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)0);
	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)sizeof(vec3));

	glActiveTexture(GL_TEXTURE0);
	GLuint texture_id = texture_gl_handles[(GLuint)TEXTURE_ASSET_ID::HEALTH_BAR_UI];
	glBindTexture(GL_TEXTURE_2D, texture_id);
	gl_has_errors();

	GLint max_health_loc = glGetUniformLocation(program, "max_health");
	glUniform1f(max_health_loc, player.max_health);
	gl_has_errors();

	GLint health_loc = glGetUniformLocation(program, "current_health");
	glUniform1f(health_loc, player.current_health);
	gl_has_errors();

	GLint health_texture_loc = glGetUniformLocation(program, "health_texture");
	glUniform1i(health_texture_loc, 0);
	gl_has_errors();

	GLint transform_loc = glGetUniformLocation(program, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	gl_has_errors();

	GLuint projection_loc = glGetUniformLocation(program, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();

	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	GLsizei num_indices = size / sizeof(uint16_t);
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

void RenderSystem::drawDashRecharge(const mat3 &projection)
{
	if (registry.dashes.size() == 0)
	{
		return;
	}

	for (Entity entity : registry.dashes.entities)
	{
		if (!registry.motions.has(entity))
		{
			continue;
		}

		if (!registry.renderRequests.has(entity))
		{
			continue;
		}

		Motion &motion = registry.motions.get(entity);
		Transform transform;
		transform.translate(motion.position);
		transform.scale(motion.scale);

		const RenderRequest &render_request = registry.renderRequests.get(entity);
		GLuint program = effects[(GLuint)render_request.used_effect];

		glUseProgram(program);
		gl_has_errors();

		GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
		GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		gl_has_errors();

		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)0);
		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)sizeof(vec3));

		glActiveTexture(GL_TEXTURE0);
		GLuint texture_id = texture_gl_handles[(GLuint)render_request.used_texture];
		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();

		GLint transform_loc = glGetUniformLocation(program, "transform");
		glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
		gl_has_errors();

		GLint projection_loc = glGetUniformLocation(program, "projection");
		glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
		gl_has_errors();

		GLint size = 0;
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
		GLsizei num_indices = size / sizeof(uint16_t);
		glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
		gl_has_errors();
	}
}

void RenderSystem::drawBuffUI()
{
	if (registry.buffUIs.size() == 0)
		return;

	mat3 projection_2D = createProjectionMatrix();

	for (Entity entity : registry.buffUIs.entities)
	{
		drawUI(entity, projection_2D);
	}

	drawToScreen();
	glfwSwapBuffers(window);
}

// M3 Feature : Instance Rendering on Particles
// INSTANCING: Draw instanced particles
void RenderSystem::drawInstancedParticles()
{
    drawParticlesByTexture(TEXTURE_ASSET_ID::DEATH_PARTICLE);
    drawParticlesByTexture(TEXTURE_ASSET_ID::PIXEL_PARTICLE);
}

void RenderSystem::drawParticlesByTexture(TEXTURE_ASSET_ID texture_id)
{
	// for debugging purposes, check for errors
	while (glGetError() != GL_NO_ERROR)
	{ /* clear errors */
	}

    if (registry.particles.size() == 0)
        return;
    
    std::vector<mat3> instanceTransforms;
	std::vector<float> instanceAlphas;
	
    for (uint i = 0; i < registry.particles.size(); i++)
    {
        Entity entity = registry.particles.entities[i];
		
		if (registry.renderRequests.has(entity)) {
            RenderRequest& request = registry.renderRequests.get(entity);
            if (request.used_texture != texture_id) {
                continue;
            }
        } else {
            continue; 
        }

        Motion &motion = registry.motions.get(entity);
        Transform transform;
        transform.translate(motion.position);
        transform.scale(motion.scale);
        transform.rotate(radians(motion.angle));
        instanceTransforms.push_back(transform.mat);

		float alpha = 1.0f;
        if (registry.particles.has(entity)) {
            Particle& particle = registry.particles.get(entity);
            if (particle.type == PARTICLE_TYPE::RIPPLE_PARTICLE) {
                alpha = particle.lifetime_ms / particle.max_lifetime_ms;
            }
        }
        instanceAlphas.push_back(alpha);
    }
    
	// for debugging purposes
    // std::cout << "[Particle Debug] Instance transforms count: " << instanceTransforms.size() << std::endl;
		
    if (instanceTransforms.empty())
        return;
    
    // bind the default VAO
    glBindVertexArray(default_vao);
    
    // bind the sprite geometry (base VBO) for particles
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(uint)GEOMETRY_BUFFER_ID::SPRITE]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffers[(uint)GEOMETRY_BUFFER_ID::SPRITE]);
    // set base vertex attrib pointers expected by particle_textured.vs.glsl:
    glEnableVertexAttribArray(0); // in_position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)0);
    glEnableVertexAttribArray(1); // in_texcoord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)sizeof(vec3));
    
    //  bind the instance VBO and update it
    glBindBuffer(GL_ARRAY_BUFFER, particle_instance_vbo);
    glBufferData(GL_ARRAY_BUFFER, instanceTransforms.size() * sizeof(mat3),
                 instanceTransforms.data(), GL_DYNAMIC_DRAW);
    
    // setup instanced vertex attrib pointers for the mat3 (at locations 2, 3, and 4.)
    for (int i = 0; i < 3; i++) {
        GLuint attrib_location = 2 + i;
        glEnableVertexAttribArray(attrib_location);
        glVertexAttribPointer(attrib_location, 3, GL_FLOAT, GL_FALSE,
                              sizeof(mat3), (void*)(sizeof(vec3) * i));
        glVertexAttribDivisor(attrib_location, 1); // advance once per instance (super IMPORTANTT)
    }

    GLuint alpha_vbo;
    glGenBuffers(1, &alpha_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, alpha_vbo);
    glBufferData(GL_ARRAY_BUFFER, instanceAlphas.size() * sizeof(float),
                 instanceAlphas.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glVertexAttribDivisor(5, 1); 
    
    glActiveTexture(GL_TEXTURE0);
	GLuint gl_texture_id = texture_gl_handles[(uint)texture_id];
    glBindTexture(GL_TEXTURE_2D, gl_texture_id);
    
    // use the particle shader
    glUseProgram(effects[(uint)EFFECT_ASSET_ID::PARTICLE_EFFECT]);
    
    mat3 projection = createProjectionMatrix();
    GLuint proj_loc = glGetUniformLocation(effects[(uint)EFFECT_ASSET_ID::PARTICLE_EFFECT], "projection");
    glUniformMatrix3fv(proj_loc, 1, GL_FALSE, (float *)&projection);
    
    // ise the stored sprite_index_count
    GLsizei num_indices = sprite_index_count;
    
	// draw the instanced particles as a set
	glDrawElementsInstanced(GL_TRIANGLES, num_indices,
													GL_UNSIGNED_SHORT, nullptr, instanceTransforms.size());

	// disable instanced attributes
	for (int i = 0; i < 3; i++)
	{
		glDisableVertexAttribArray(2 + i);
	}

	// for debugging purposes, check for errors
	while (glGetError() != GL_NO_ERROR)
	{ /* clear any errors */
	}
}

// Structure for tile instance data (put here so it is visible to the shader for later changes)
struct TileInstance
{
	mat3 transform;
	vec4 params; // x: total_frames, y: current_frame, z: sprite_width, w: sprite_height
};

void RenderSystem::drawInstancedTiles(const mat3 &projection)
{

	// group tile instances by texture used.
	std::unordered_map<GLuint, std::vector<TileInstance>> groups;

	for (Entity entity : registry.tiles.entities)
	{
		if (!registry.motions.has(entity) ||
				!registry.renderRequests.has(entity) ||
				!registry.spriteSheetImages.has(entity) ||
				!registry.spritesSizes.has(entity))
			continue;
		RenderRequest &req = registry.renderRequests.get(entity);
		if (req.used_effect != EFFECT_ASSET_ID::TILE)
			continue;
		//build instance transform from motion.
		Motion &motion = registry.motions.get(entity);
		Transform transform;
		transform.translate(motion.position);
		transform.scale(motion.scale);
		transform.rotate(radians(motion.angle));

		SpriteSheetImage &spriteSheet = registry.spriteSheetImages.get(entity);
		SpriteSize &sprite = registry.spritesSizes.get(entity);

		TileInstance instance;
		instance.transform = transform.mat;
		instance.params = {float(spriteSheet.total_frames),
											 float(spriteSheet.current_frame),
											 float(sprite.width),
											 float(sprite.height)};
		GLuint texture = texture_gl_handles[(uint)req.used_texture];
		groups[texture].push_back(instance);
	}

	// for each texture group, render the batch 
	for (auto &group : groups)
	{
		GLuint texture = group.first;
		std::vector<TileInstance> &instances = group.second;
		if (instances.empty())
			continue;

		GLuint program = effects[(uint)EFFECT_ASSET_ID::TILE];
		glUseProgram(program);
		gl_has_errors();

		// bind common geometry buffer
		GLuint vbo = vertex_buffers[(uint)GEOMETRY_BUFFER_ID::SPRITE];
		GLuint ibo = index_buffers[(uint)GEOMETRY_BUFFER_ID::SPRITE];
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

		// aet up base vertex attribute pointers
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)0);
		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)sizeof(vec3));

		// bindd and update tile instance VB
		glBindBuffer(GL_ARRAY_BUFFER, tile_instance_vbo);
		glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(TileInstance), instances.data(), GL_DYNAMIC_DRAW);

		// aetup instance attributes for the matrix (locations 2, 3, 4)
		for (int i = 0; i < 3; i++)
		{
			GLuint attrib_location = 2 + i;
			glEnableVertexAttribArray(attrib_location);
			glVertexAttribPointer(attrib_location, 3, GL_FLOAT, GL_FALSE, sizeof(TileInstance), (void *)(sizeof(vec3) * i));
			glVertexAttribDivisor(attrib_location, 1); // one per instance
		}
		// aetup instance attribute for tile parameters at location 5
		GLuint tile_params_loc = 5;
		glEnableVertexAttribArray(tile_params_loc);
		glVertexAttribPointer(tile_params_loc, 4, GL_FLOAT, GL_FALSE, sizeof(TileInstance), (void *)sizeof(mat3));
		glVertexAttribDivisor(tile_params_loc, 1);

		// set shader uniforms.
		glUniformMatrix3fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, (float *)&projection);
		Camera &camera = registry.cameras.get(registry.cameras.entities[0]);
		glUniform2fv(glGetUniformLocation(program, "camera_position"), 1, (float *)&camera.position);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(glGetUniformLocation(program, "sampler0"), 0);

		// draw instanced
		GLint iboSize = 0;
		glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &iboSize);
		GLsizei num_indices = iboSize / sizeof(uint16_t);
		glDrawElementsInstanced(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr, instances.size());
		gl_has_errors();

		// dsable instanced attributes
		for (int i = 2; i <= 5; i++)
		{
			glDisableVertexAttribArray(i);
		}
	}
}