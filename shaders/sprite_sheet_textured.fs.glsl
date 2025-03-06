#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec4 fcolor;

// Output color
layout(location = 0) out  vec4 color;

uniform int total_frames;
uniform int current_frame;
uniform int sprite_width;
uniform int sprite_height;

void main()
{
	vec4 texture_color = texture(sampler0, vec2(texcoord.x, texcoord.y));
	color = vec4(fcolor.rgb, fcolor.a) * texture_color;
}
