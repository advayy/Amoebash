#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform float darken_screen_factor;
uniform float vignette_screen_factor;

in vec2 texcoord;

layout(location = 0) out vec4 color;

vec4 vignette(vec4 in_color) 
{
	return in_color;
}

// darken the screen, i.e., fade to black
vec4 fade_color(vec4 in_color) 
{
	// return in_color;
    if (darken_screen_factor <= 0.0) {
        return in_color;
    }
    return mix(in_color, vec4(0.0, 0.0, 0.0, 1.0), darken_screen_factor);
}

void main()
{
    vec4 in_color = texture(screen_texture, texcoord);
	in_color = vignette(in_color);
    color = fade_color(in_color);
}