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

    float frameIndex = frame;
    float parallaxFactor = float(frame)/float(total_frames - 1);

    vec2 adjustedTexCoord = texcoord + ((camera_position/3000) * parallaxFactor);

    float frameWidth = 1.0 / float(total_frames);  // Width of each frame in texture space
    float frameStart = frameIndex * frameWidth;   // Start of the current frame in texture space
    float frameEnd = frameStart + frameWidth;     // End of the current frame in texture space

    // Clamp the adjusted texture coordinates to the current frame's bounds
    adjustedTexCoord.x = mod(adjustedTexCoord.x, frameWidth) + frameStart;
    adjustedTexCoord.y = mod(adjustedTexCoord.y, 1.0);  // Keep y within [0, 1]

    return adjustedTexCoord;
}

void main()
{	
	if (total_frames == 1) {
		color = texture(sampler0, texcoord);
		return;
	}

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