#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;
out vec3 position;

// Application data
uniform mat3 transform;
uniform mat3 projection;

uniform int map_width;
uniform int map_height;
uniform vec2 camera_grid_position;

void main()
{
	texcoord = in_texcoord;
	position = in_position;
	vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}