#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;

// Output color
layout(location = 0) out  vec4 color;

uniform float max_danger;
uniform float current_danger;

void main()
{
	color = vec4(1.0, 1.0, 1.0, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y));
}
