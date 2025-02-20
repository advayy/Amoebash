
#include <SDL.h>
#include <glm/trigonometric.hpp>
#include <iostream>

// internal
#include "render_system.hpp"
#include "tinyECS/registry.hpp"
#include "world_system.hpp"

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
		render_request.used_effect == EFFECT_ASSET_ID::TILE) &&
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
		Player& player = registry.players.get(registry.players.entities[0]);

		glUniform2fv(player_grid_position_uloc, 1, (float*)&player.grid_position);
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
		Camera& camera = registry.cameras.get(registry.cameras.entities[0]);
		vec2 cameraPos = camera.position;
		GLint camera_position_uloc = glGetUniformLocation(program, "camera_position");
		glUniform2fv(camera_position_uloc, 1, (float*)&cameraPos);
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
		Motion& motion = registry.motions.get(entity);

		// Transformation code, see Rendering and Transformation in the template
		// specification for more info Incrementally updates transformation matrix,
		// thus ORDER IS IMPORTANT
		Transform transform;
		transform.translate(motion.position);
		transform.scale(motion.scale);
		transform.rotate(radians(motion.angle));

		// Setting uniform values to the currently bound program
		GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
		glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.mat);
		gl_has_errors();
	}

	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();


	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

void RenderSystem::setUpSpriteSheetTexture(Entity& entity, const GLuint program)
{
	// SpriteSheet UNIFORMS see :
		// 	int total_frames from SpriteSheetImage
		// 	int current_frame from SpriteSheetImage
		// 	int sprite size from sprite
	GLint total_frames_uloc = glGetUniformLocation(program, "total_frames");
	GLint current_frame_uloc = glGetUniformLocation(program, "current_frame");
	GLint sprite_width_uloc = glGetUniformLocation(program, "sprite_width");
	GLint sprite_height_uloc = glGetUniformLocation(program, "sprite_height");

	SpriteSheetImage& spriteSheet = registry.spriteSheetImages.get(entity);
	glUniform1i(total_frames_uloc, spriteSheet.total_frames);
	glUniform1i(current_frame_uloc, spriteSheet.current_frame);
	SpriteSize& sprite = registry.spritesSizes.get(entity);
	glUniform1i(sprite_width_uloc, sprite.width);
	glUniform1i(sprite_height_uloc, sprite.height);
}

void RenderSystem::setUpDefaultProgram(Entity& entity, const RenderRequest& render_request, const GLuint program)
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
		sizeof(TexturedVertex), (void*)0);
	gl_has_errors();

	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(
		in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
		(void*)sizeof(
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
	GLuint time_uloc       = glGetUniformLocation(vignette_program, "time");
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
	for (Entity entity : registry.tiles.entities)
	{
		drawTexturedMesh(entity, projection_2D);
	}

	for (Entity entity : registry.renderRequests.entities)
	{
		if ((registry.motions.has(entity) || !registry.spriteSheetImages.has(entity)) && !registry.tiles.has(entity) && !registry.gameScreens.has(entity) && !registry.miniMaps.has(entity))
		{
			drawTexturedMesh(entity, projection_2D);
		}
	}
	
	// draw the mini map
	drawTexturedMesh(registry.miniMaps.entities[0], projection_2D);
	
	if (registry.pauses.size() != 0) {
		auto& pause = registry.pauses.entities[0];
		drawTexturedMesh(pause, projection_2D);
	}


	// draw framebuffer to screen
	// adding "vignette" effect when applied
	drawToScreen();

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

mat3 RenderSystem::createProjectionMatrix() {
    Camera& camera = registry.cameras.get(registry.cameras.entities[0]);
    vec2 cameraPos = camera.position;
    
    float left   = cameraPos.x - WINDOW_WIDTH_PX * 0.5f;
    float right  = left + WINDOW_WIDTH_PX;
    float top    = cameraPos.y - WINDOW_HEIGHT_PX * 0.5f;
    float bottom = top + WINDOW_HEIGHT_PX;

    float sx = 2.f / (right - left);
    float sy = 2.f / (top - bottom);
    float tx = -(right + left)   / (right - left);
    float ty = -(top + bottom)   / (top - bottom);

	return {
		{ sx, 0.f, 0.f},
		{0.f,  sy, 0.f},
		{ tx,  ty, 1.f}
	};
}

void RenderSystem::drawStartScreen() {

    std::vector<ButtonType> buttons = {
        ButtonType::STARTBUTTON,
        ButtonType::SHOPBUTTON,
        ButtonType::INFOBUTTON
    };

    drawScreenAndButtons(ScreenType::START, buttons);
}


void RenderSystem::drawShopScreen() {

	std::vector<ButtonType> buttons = {
        ButtonType::SHOPBUTTON
    };

	drawScreenAndButtons(ScreenType::SHOP, buttons);
}


void RenderSystem::drawInfoScreen() {
	
	std::vector<ButtonType> buttons = {
		ButtonType::INFOBUTTON
	};

	drawScreenAndButtons(ScreenType::INFO, buttons);
}

void RenderSystem::drawGameOverScreen() {
	std::vector<ButtonType> buttons = {
		
	};

	drawScreenAndButtons(ScreenType::GAMEOVER, buttons);

}

void RenderSystem::drawScreenAndButtons(
    ScreenType screenType,
    const std::vector<ButtonType>& buttonTypes) {

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

    for (uint i = 0; i < registry.gameScreens.size(); i++) {
        const auto& screenComp = registry.gameScreens.components[i];
        if (screenComp.type == screenType) {
            Entity screenEntity = registry.gameScreens.entities[i];
            drawTexturedMesh(screenEntity, projection_matrix);
        }
    }

	if (buttonTypes.size() != 0) {
		for (uint i = 0; i < registry.buttons.components.size(); i++) {
			const screenButton& buttonComp = registry.buttons.components[i];

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

void RenderSystem::drawCutScreneAnimation() {

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

	for (auto& entity : registry.cutscenes.entities) {
		drawTexturedMesh(entity, projection_matrix);
	}
    
    drawToScreen();
    glfwSwapBuffers(window);
    gl_has_errors();
}