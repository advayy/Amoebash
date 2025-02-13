#version 330

// From vertex shader
in vec2 texcoord;
in vec3 position;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;

// Output color
layout(location = 0) out  vec4 color;

// Number of tiles in the texture - layers go from left being lowest (1) to right being highest
// Used for paralax mapping
uniform int total_frames;
uniform int current_frame;
uniform int sprite_width;
uniform int sprite_height;
uniform vec2 camera_position;
// if theres multiple frames you need to merge all three frames into one

vec2 get_offset_texcoord(vec2 texcoord, int frame) {
	float sprite_width = 1.0 / float(total_frames);
	float frame_offset = float(frame) * sprite_width;
	return vec2(frame_offset + texcoord.x * sprite_width, texcoord.y);
}

vec2 apply_paralax(vec2 texcoord, int frame, vec2 camera_position) {
    //if this is the last frame use the original texcoord
	if(frame == total_frames - 1) {
		return texcoord;
	}

	// if(frame == 0) {
	// 	return texcoord;
	// }
	
	float sprite_width = 1.0 / float(total_frames);

	float depth = 1.0 - float(frame) / float(total_frames - 1);

    float offset_x = (camera_position.x / 2688.0 * depth);
    float offset_y = (camera_position.y / 2688.0 * depth);
	
	float frame_fac = frame;

	if (frame == 0) {
		frame_fac = 1.0;
	}

	float wrapped_x = frame * sprite_width + mod(texcoord.x + offset_x, (frame_fac) * sprite_width);
    float wrapped_y = mod(texcoord.y + offset_y, 1.0);

    return vec2(wrapped_x, wrapped_y);
}

void main()
{	

	vec4 fcolor = vec4(0.0, 0.0, 0.0, 0.0);

	for (int i = current_frame; i < total_frames; i++) {

		vec2 texcoord = get_offset_texcoord(texcoord, i);
		texcoord = apply_paralax(texcoord, i, camera_position);

		vec4 layer_color = texture(sampler0, texcoord);

		if(i == 0) {
			fcolor = layer_color;
		} else if (layer_color.a > 0.0) {
			fcolor = layer_color;
		}
	}	
	color = fcolor;
}
