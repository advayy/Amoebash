#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform float darken_screen_factor;
uniform float vignette_screen_factor;

in vec2 texcoord;

layout(location = 0) out vec4 color;

vec4 vignette(vec4 in_color) 
{
    vec2 centered_uv = texcoord - vec2(0.5, 0.5);
    float dist = length(centered_uv) * 1.5;

    float vignette_strength = smoothstep(0.5, 1.0, dist) * vignette_screen_factor; 

    vec3 red_tint = vec3(1.0, 0.2, 0.2);
    vec3 vignette_color = mix(in_color.rgb, red_tint, vignette_strength * 0.6);

    return vec4(vignette_color, in_color.a);
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