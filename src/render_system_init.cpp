// stdlib
#include <iostream>
#include <sstream>
#include <array>
#include <fstream>

// internal
#include "../ext/stb_image/stb_image.h"
#include "render_system.hpp"
#include "tinyECS/registry.hpp"

// fonts
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Render initialization
bool RenderSystem::init(GLFWwindow *window_arg)
{
	this->window = window_arg;

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // vsync

	// Load OpenGL function pointers
	const int is_fine = gl3w_init();
	assert(is_fine == 0);

	// Create a frame buffer
	frame_buffer = 0;
	glGenFramebuffers(1, &frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();

	// For some high DPI displays (ex. Retina Display on Macbooks)
	// https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value
	int frame_buffer_width_px, frame_buffer_height_px;
	glfwGetFramebufferSize(window, &frame_buffer_width_px, &frame_buffer_height_px); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	if (frame_buffer_width_px != WINDOW_WIDTH_PX)
	{
		printf("WARNING: retina display! https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value\n");
		printf("glfwGetFramebufferSize = %d,%d\n", frame_buffer_width_px, frame_buffer_height_px);
		printf("requested window width,height = %f,%f\n", WINDOW_WIDTH_PX, WINDOW_HEIGHT_PX);
	}

	// Hint: Ask your TA for how to setup pretty OpenGL error callbacks.
	// This can not be done in mac os, so do not enable
	// it unless you are on Linux or Windows. You will need to change the window creation
	// code to use OpenGL 4.3 (not suported on mac) and add additional .h and .cpp
	// glDebugMessageCallback((GLDEBUGPROC)errorCallback, nullptr);

	// We are not really using VAO's but without at least one bound we will crash in
	// some systems.
	glGenVertexArrays(1, &default_vao);
	glBindVertexArray(default_vao);
	gl_has_errors();

	initScreenTexture();
	initializeGlTextures();
	initializeGlEffects();
	initializeGlGeometryBuffers();

    // init font
    std::string font_filename = PROJECT_SOURCE_DIR + std::string("data/fonts/PixelifySans-Regular.ttf");
    unsigned int font_default_size = 48;
	fontInit(*window, font_filename, font_default_size);

	return true;
}

void RenderSystem::initializeGlTextures()
{
	glGenTextures((GLsizei)texture_gl_handles.size(), texture_gl_handles.data());

	for (uint i = 0; i < texture_paths.size(); i++)
	{
		const std::string &path = texture_paths[i];
		ivec2 &dimensions = texture_dimensions[i];

		stbi_uc *data;
		data = stbi_load(path.c_str(), &dimensions.x, &dimensions.y, NULL, 4);

		if (data == NULL)
		{
			const std::string message = "Could not load the file " + path + ".";
			fprintf(stderr, "%s", message.c_str());
			assert(false);
		}
		glBindTexture(GL_TEXTURE_2D, texture_gl_handles[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dimensions.x, dimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		gl_has_errors();
		stbi_image_free(data);
	}
	gl_has_errors();
}

void RenderSystem::initializeGlEffects()
{

	for (uint i = 0; i < effect_paths.size(); i++)
	{
		const std::string vertex_shader_name = effect_paths[i] + ".vs.glsl";
		const std::string fragment_shader_name = effect_paths[i] + ".fs.glsl";

		std::cout << "Loading shaders: " << vertex_shader_name << " and " << fragment_shader_name << std::endl;

		bool is_valid = loadEffectFromFile(vertex_shader_name, fragment_shader_name, effects[i]);
		assert(is_valid && (GLuint)effects[i] != 0);
	}
}

// One could merge the following two functions as a template function...
template <class T>
void RenderSystem::bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices)
{
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(uint)gid]);
	glBufferData(GL_ARRAY_BUFFER,
							 sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	gl_has_errors();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffers[(uint)gid]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
							 sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);
	gl_has_errors();
}

void RenderSystem::initializeGlMeshes()
{
	// No meshes rn ...
	// for (uint i = 0; i < mesh_paths.size(); i++)
	// {
	// 	// Initialize meshes
	// 	GEOMETRY_BUFFER_ID geom_index = mesh_paths[i].first;
	// 	std::string name = mesh_paths[i].second;
	// 	Mesh::loadFromOBJFile(name,
	// 		meshes[(int)geom_index].vertices,
	// 		meshes[(int)geom_index].vertex_indices,
	// 		meshes[(int)geom_index].original_size);

	// 	bindVBOandIBO(geom_index,
	// 		meshes[(int)geom_index].vertices,
	// 		meshes[(int)geom_index].vertex_indices);
	// }
}

std::vector<TexturedVertex> RenderSystem::loadMeshVertices(const std::string &filename)
{
	// std::ifstream file(filename);
	// std::vector<TexturedVertex> meshVertices;
	// std::string line;

	// std::getline(file, line);

	// while (std::getline(file, line)) {
	// 	std::istringstream ss(line);
	// 	TexturedVertex vertex;

	// 	ss >> vertex.position.x >> vertex.position.y >> vertex.position.z
	// 		>> vertex.texcoord.x >> vertex.texcoord.y;

	// 	meshVertices.push_back(vertex);
	// }

	// return meshVertices;
	std::ifstream file(filename);
	std::vector<TexturedVertex> vertices;
	std::string line;

	if (!file.is_open())
	{
		std::cerr << "Error: Cannot open mesh file " << filename << std::endl;
		return vertices; // Return empty if file cannot be opened
	}

	bool reading_vertices = true; // Read vertices first

	while (std::getline(file, line))
	{
		// If we hit the index section, stop reading vertices
		if (line.find("# Triangulation indices") != std::string::npos)
		{
			break;
		}

		// Ignore empty lines or headers
		if (line.empty() || line.find("x y z u v") != std::string::npos)
		{
			continue;
		}

		std::istringstream ss(line);
		TexturedVertex vertex;

		// Read vertex position (x, y, z) and UV coordinates (u, v)
		if (ss >> vertex.position.x >> vertex.position.y >> vertex.position.z >> vertex.texcoord.x >> vertex.texcoord.y)
		{
			vertices.push_back(vertex);
		}
	}

	file.close();
	std::cout << "Loaded " << vertices.size() << " vertices from " << filename << std::endl;
	return vertices;
}

void RenderSystem::initializeGlGeometryBuffers()
{
	// Vertex Buffer creation.
	glGenBuffers((GLsizei)vertex_buffers.size(), vertex_buffers.data());
	// Index Buffer creation.
	glGenBuffers((GLsizei)index_buffers.size(), index_buffers.data());

	// INSTANCING: generate the particle instance VBO for instanced rendering.
	glGenBuffers(1, &particle_instance_vbo);
	// INSTANCING: generate VBO for tile instancing
	glGenBuffers(1, &tile_instance_vbo);

	// Index and Vertex buffer data initialization.
	initializeGlMeshes();

	//////////////////////////
	// Initialize sprite
	// The position corresponds to the center of the texture.
	std::vector<TexturedVertex> textured_vertices(4);
	textured_vertices[0].position = {-1.f / 2, +1.f / 2, 0.f};
	textured_vertices[1].position = {+1.f / 2, +1.f / 2, 0.f};
	textured_vertices[2].position = {+1.f / 2, -1.f / 2, 0.f};
	textured_vertices[3].position = {-1.f / 2, -1.f / 2, 0.f};
	textured_vertices[0].texcoord = {0.f, 1.f};
	textured_vertices[1].texcoord = {1.f, 1.f};
	textured_vertices[2].texcoord = {1.f, 0.f};
	textured_vertices[3].texcoord = {0.f, 0.f};

	// Counterclockwise as it's the default OpenGL front winding direction.
	const std::vector<uint16_t> textured_indices = {0, 3, 1, 1, 3, 2};
	bindVBOandIBO(GEOMETRY_BUFFER_ID::SPRITE, textured_vertices, textured_indices);
	// NEW: Save the index count for instancing particles later.
	sprite_index_count = (GLsizei)textured_indices.size();

	/* LEGACY - not used, but code below still relies on it...*/
	////////////////////////
	// Initialize LINE
	std::vector<ColoredVertex> line_vertices;
	std::vector<uint16_t> line_indices;
	constexpr float z = -0.1f;
	constexpr int NUM_TRIANGLES = 62;

	for (int i = 0; i < NUM_TRIANGLES; i++)
	{
		const float t = float(i) * M_PI * 2.f / float(NUM_TRIANGLES - 1);
		line_vertices.push_back({});
		line_vertices.back().position = {0.5 * cos(t), 0.5 * sin(t), z};
		line_vertices.back().color = {0.8, 0.8, 0.8};
	}
	line_vertices.push_back({});
	line_vertices.back().position = {0, 0, 0};
	line_vertices.back().color = {1, 1, 1};
	for (int i = 0; i < NUM_TRIANGLES; i++)
	{
		line_indices.push_back((uint16_t)i);
		line_indices.push_back((uint16_t)((i + 1) % NUM_TRIANGLES));
		line_indices.push_back((uint16_t)NUM_TRIANGLES);
	}
	int geom_index = (int)GEOMETRY_BUFFER_ID::LINE;
	meshes[geom_index].vertices = line_vertices;
	meshes[geom_index].vertex_indices = line_indices;
	bindVBOandIBO(GEOMETRY_BUFFER_ID::LINE, meshes[geom_index].vertices, meshes[geom_index].vertex_indices);

	//////////////////////////////////
	// Initialize debug line
	std::vector<ColoredVertex> debug_line_vertices;
	std::vector<uint16_t> debug_line_indices;

	constexpr float depth = 0.5f;
	// constexpr vec3 red = { 0.8, 0.1, 0.1 };
	constexpr vec3 red = {1.0, 1.0, 1.0};

	// Corner points
	debug_line_vertices = {
			{{-0.5, -0.5, depth}, red},
			{{-0.5, 0.5, depth}, red},
			{{0.5, 0.5, depth}, red},
			{{0.5, -0.5, depth}, red},
	};

	// Two triangles
	debug_line_indices = {0, 1, 3, 1, 2, 3};

	geom_index = (int)GEOMETRY_BUFFER_ID::DEBUG_LINE;
	meshes[geom_index].vertices = debug_line_vertices;
	meshes[geom_index].vertex_indices = debug_line_indices;
	bindVBOandIBO(GEOMETRY_BUFFER_ID::DEBUG_LINE, debug_line_vertices, debug_line_indices);

	///////////////////////////////////////////////////////
	// Initialize screen triangle (yes, triangle, not quad; its more efficient).
	std::vector<vec3> screen_vertices(3);
	screen_vertices[0] = {-1, -6, 0.f};
	screen_vertices[1] = {6, -1, 0.f};
	screen_vertices[2] = {-1, 6, 0.f};

	///////////////////////////////////////////////////////
	// Initialize Occtagon Mesh
	// std::vector<ColoredVertex> hexagon_vertices;
	std::string mesh_file_name = "data/textures/meshes/key_flipped.txt";
	// std::string mesh_file_name = "data/textures/meshes/key.txt";
	std::vector<TexturedVertex> img_textured_vertices = loadMeshVertices(std::string(PROJECT_SOURCE_DIR) + mesh_file_name);

	std::vector<uint16_t> img_indices; // Store indices
	std::cout << img_textured_vertices.size() << std::endl;
	std::cout << "Should be loaded let the vertices" << std::endl;

	// Open the file to read indices directly
	std::ifstream file(std::string(PROJECT_SOURCE_DIR) + mesh_file_name);
	std::string line;
	bool reading_indices = false;

	while (std::getline(file, line))
	{
		if (line.find("# Triangulation indices") != std::string::npos)
		{
			reading_indices = true;
			continue; // Skip this line
		}

		if (reading_indices)
		{
			std::istringstream ss(line);
			uint16_t i1, i2, i3;
			if (ss >> i1 >> i2 >> i3)
			{
				img_indices.push_back(i1);
				img_indices.push_back(i2);
				img_indices.push_back(i3);
			}
		}
	}

	file.close(); // Close file after reading

	// std::cout << "Loaded " << img_indices.size() / 3 << " triangles.\n";

	// std::cout << "Loaded " << img_textured_vertices.size() << " vertices." << std::endl;
	// std::cout << "First 5 vertices:\n";
	// for (size_t i = 0; i < std::min(img_textured_vertices.size(), size_t(5)); i++) {
	// 	std::cout << "Pos: (" << img_textured_vertices[i].position.x << ", "
	// 			<< img_textured_vertices[i].position.y << ", "
	// 			<< img_textured_vertices[i].position.z << ") UV: ("
	// 			<< img_textured_vertices[i].texcoord.x << ", " << img_textured_vertices[i].texcoord.y << ")\n";
	// }

	// std::cout << "Loaded " << img_indices.size() / 3 << " triangles.\n";
	// std::cout << "First 5 triangle indices:\n";
	// for (size_t i = 0; i < std::min(img_indices.size(), size_t(15)); i += 3) {
	// 	std::cout << img_indices[i] << ", " << img_indices[i + 1] << ", " << img_indices[i + 2] << "\n";
	// }

	// Assign the loaded data to the mesh
	int img_geom_index = (int)GEOMETRY_BUFFER_ID::HEXAGON;
	meshes[img_geom_index].textured_vertices = img_textured_vertices;
	meshes[img_geom_index].vertex_indices = img_indices;

	bindVBOandIBO(GEOMETRY_BUFFER_ID::HEXAGON, meshes[img_geom_index].textured_vertices, meshes[img_geom_index].vertex_indices);

	// Counterclockwise as it's the default opengl front winding direction.
	const std::vector<uint16_t> screen_indices = {0, 1, 2};
	bindVBOandIBO(GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE, screen_vertices, screen_indices);
}

RenderSystem::~RenderSystem()
{
	// Don't need to free gl resources since they last for as long as the program,
	// but it's polite to clean after yourself.
	glDeleteBuffers((GLsizei)vertex_buffers.size(), vertex_buffers.data());
	glDeleteBuffers((GLsizei)index_buffers.size(), index_buffers.data());
	glDeleteTextures((GLsizei)texture_gl_handles.size(), texture_gl_handles.data());
	glDeleteTextures(1, &off_screen_render_buffer_color);
	glDeleteRenderbuffers(1, &off_screen_render_buffer_depth);
	gl_has_errors();

	for (uint i = 0; i < effect_count; i++)
	{
		glDeleteProgram(effects[i]);
	}
	// delete allocated resources
	glDeleteFramebuffers(1, &frame_buffer);
	gl_has_errors();

	// remove all entities created by the render system
	while (registry.renderRequests.entities.size() > 0)
		registry.remove_all_components_of(registry.renderRequests.entities.back());
}

// Initialize the screen texture from a standard sprite
bool RenderSystem::initScreenTexture()
{
	// create a single entry
	registry.screenStates.emplace(screen_state_entity);

	int framebuffer_width, framebuffer_height;
	glfwGetFramebufferSize(const_cast<GLFWwindow *>(window), &framebuffer_width, &framebuffer_height); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	glGenTextures(1, &off_screen_render_buffer_color);
	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebuffer_width, framebuffer_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	gl_has_errors();

	glGenRenderbuffers(1, &off_screen_render_buffer_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, off_screen_render_buffer_depth);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, off_screen_render_buffer_color, 0);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, framebuffer_width, framebuffer_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, off_screen_render_buffer_depth);
	gl_has_errors();

	// To fix the white lines that appear
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	return true;
}

bool gl_compile_shader(GLuint shader)
{
	glCompileShader(shader);
	gl_has_errors();
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint log_len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
		std::vector<char> log(log_len);
		glGetShaderInfoLog(shader, log_len, &log_len, log.data());
		glDeleteShader(shader);

		gl_has_errors();

		fprintf(stderr, "GLSL: %s", log.data());
		return false;
	}

	return true;
}

bool loadEffectFromFile(
		const std::string &vs_path, const std::string &fs_path, GLuint &out_program)
{
	// Opening files
	std::ifstream vs_is(vs_path);
	std::ifstream fs_is(fs_path);
	if (!vs_is.good() || !fs_is.good())
	{
		fprintf(stderr, "Failed to load shader files %s, %s", vs_path.c_str(), fs_path.c_str());
		assert(false);
		return false;
	}

	// Reading sources
	std::stringstream vs_ss, fs_ss;
	vs_ss << vs_is.rdbuf();
	fs_ss << fs_is.rdbuf();
	std::string vs_str = vs_ss.str();
	std::string fs_str = fs_ss.str();
	const char *vs_src = vs_str.c_str();
	const char *fs_src = fs_str.c_str();
	GLsizei vs_len = (GLsizei)vs_str.size();
	GLsizei fs_len = (GLsizei)fs_str.size();

	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vs_src, &vs_len);
	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fs_src, &fs_len);
	gl_has_errors();

	// Compiling
	if (!gl_compile_shader(vertex))
	{
		fprintf(stderr, "Vertex compilation failed");
		assert(false);
		return false;
	}
	if (!gl_compile_shader(fragment))
	{
		fprintf(stderr, "Fragment compilation failed");
		assert(false);
		return false;
	}

	// Linking
	out_program = glCreateProgram();
	glAttachShader(out_program, vertex);
	glAttachShader(out_program, fragment);
	glLinkProgram(out_program);
	gl_has_errors();

	{
		GLint is_linked = GL_FALSE;
		glGetProgramiv(out_program, GL_LINK_STATUS, &is_linked);
		if (is_linked == GL_FALSE)
		{
			GLint log_len;
			glGetProgramiv(out_program, GL_INFO_LOG_LENGTH, &log_len);
			std::vector<char> log(log_len);
			glGetProgramInfoLog(out_program, log_len, &log_len, log.data());
			gl_has_errors();

			fprintf(stderr, "Link error: %s", log.data());
			assert(false);
			return false;
		}
	}

	// No need to carry this around. Keeping these objects is only useful if we recycle
	// the same shaders over and over, which we don't, so no need and this is simpler.
	glDetachShader(out_program, vertex);
	glDetachShader(out_program, fragment);
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	gl_has_errors();

	return true;
}

bool is_shader_error(unsigned int shader, const std::string& shader_name) {
    GLint fs_compile_result;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &fs_compile_result);
    if (fs_compile_result != GL_TRUE) {
        std::cerr << "ERROR: shader compiler error for shader: " << shader_name << std::endl;

        char errBuff[1024];
        int bufLen = 0;
        glGetShaderInfoLog(shader, 1024, &bufLen, errBuff);
        std::cout << "Shader error info: " << errBuff << std::endl;
        assert(bufLen == 0);
        return true;
    }
    else {
        std::cout << "No error with shader: " << shader_name << std::endl;
        return false;
    }
}

std::string readShaderFile(const std::string& filename)
{
	std::cout << "Loading shader filename: " << filename << std::endl;

	std::ifstream ifs(filename);

	if (!ifs.good())
	{
		std::cerr << "ERROR: invalid filename loading shader from file: " << filename << std::endl;
		return "";
	}

	std::ostringstream oss;
	oss << ifs.rdbuf();
	std::cout << oss.str() << std::endl;
	return oss.str();
}

bool RenderSystem::fontInit(GLFWwindow& window, const std::string& font_filename, unsigned int font_default_size) {
    // read in our shader files
    std::string fontVertexShaderSource = readShaderFile(PROJECT_SOURCE_DIR + std::string("shaders/font.vs.glsl"));
    std::string fontFragmentShaderSource = readShaderFile(PROJECT_SOURCE_DIR + std::string("shaders/font.fs.glsl"));
    const char* fontVertexShaderSource_c = fontVertexShaderSource.c_str();
    const char* fontFragmentShaderSource_c = fontFragmentShaderSource.c_str();

    // enable blending or you will just get solid boxes instead of text
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // font buffer setup
    glGenVertexArrays(1, &m_font_VAO);
    glGenBuffers(1, &m_font_VBO);

    // font vertex shader
    unsigned int font_vertexShader;
    font_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(font_vertexShader, 1, &fontVertexShaderSource_c, NULL);
    glCompileShader(font_vertexShader);

    // simple example of error checking
    if (is_shader_error(font_vertexShader, std::string("font_vertexShader"))) {
        assert("ERROR: font vertex shader error");
    }

    // font fragement shader
    unsigned int font_fragmentShader;
    font_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(font_fragmentShader, 1, &fontFragmentShaderSource_c, NULL);
    glCompileShader(font_fragmentShader);

    // simple example of error checking
    if (is_shader_error(font_fragmentShader, std::string("font_fragmentShader"))) {
        assert("ERROR: font fragment shader error");
    }

    // font shader program
    m_font_shaderProgram = glCreateProgram();
    glAttachShader(m_font_shaderProgram, font_vertexShader);
    glAttachShader(m_font_shaderProgram, font_fragmentShader);
    glLinkProgram(m_font_shaderProgram);

    // apply projection matrix for font
    glUseProgram(m_font_shaderProgram);
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(WINDOW_WIDTH_PX), 0.0f, static_cast<float>(WINDOW_HEIGHT_PX));
    GLint project_location = glGetUniformLocation(m_font_shaderProgram, "projection");
    assert(project_location > -1);
    // std::cout << "project_location: " << project_location << std::endl;
    glUniformMatrix4fv(project_location, 1, GL_FALSE, glm::value_ptr(projection));

    // clean up shaders
    glDeleteShader(font_vertexShader);
    glDeleteShader(font_fragmentShader);

    // init FreeType fonts
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return false;
    }

    FT_Face face;
    if (FT_New_Face(ft, font_filename.c_str(), 0, &face))
    {
        std::cerr << "ERROR::FREETYPE: Failed to load font: " << font_filename << std::endl;
        return false;
    }

    // extract a default size
    FT_Set_Pixel_Sizes(face, 0, font_default_size);

    // disable byte-alignment restriction in OpenGL
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // load each of the chars - note only first 128 ASCII chars
    for (unsigned char c = (unsigned char)0; c < (unsigned char)128; c++)
    {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "ERROR::FREETYTPE: Failed to load Glyph " << c << std::endl;
            continue;
        }

        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        // std::cout << "texture: " << c << " = " << texture << std::endl;

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x),
            (char)c
        };
        m_ftCharacters.insert(std::pair<char, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // clean up
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // bind buffers
    glBindVertexArray(m_font_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_font_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    // release buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}