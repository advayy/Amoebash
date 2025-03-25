#version 330

// From vertex shader
in vec2 texcoord;
in vec3 position;
flat in vec4 instanceTileParams;  // NEW: now contains per–tile data

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform vec2 camera_position;  // used for parallax

// Output color
layout(location = 0) out vec4 color;

vec2 get_offset_texcoord(vec2 tc, int frame) {
    float spr_width = 1.0 / float(int(instanceTileParams.x));  
    float frame_offset = float(frame) * spr_width;
    return vec2(frame_offset + tc.x * spr_width, tc.y);
}

vec2 apply_paralax(vec2 tc, int frame, vec2 cam_pos) {
    float frameIndex = frame;
    float parallaxFactor = (float(frame)) / float(int(instanceTileParams.x) - 1);
    vec2 adjustedTexCoord = tc + ((cam_pos/3000) * parallaxFactor);
    float frameWidth = 1.0 / float(int(instanceTileParams.x));
    float frameStart = frameIndex * frameWidth;
    float frameEnd = frameStart + frameWidth;
    adjustedTexCoord.x = mod(adjustedTexCoord.x, frameWidth) + frameStart;
    adjustedTexCoord.y = mod(adjustedTexCoord.y, 1.0);
    return adjustedTexCoord;
}

void main()
{	
    if (int(instanceTileParams.x) == 1) {  // total_frames == 1
        color = texture(sampler0, texcoord);
        return;
    }

    vec4 fcolor_accum = vec4(0.0);

    // Use instanceTileParams.y as current frame
    for (int i = int(instanceTileParams.y); i < int(instanceTileParams.x); i++) {
        vec2 off_tc = get_offset_texcoord(texcoord, i);
        if (i != int(instanceTileParams.x) - 1) {
            off_tc = apply_paralax(off_tc, i, camera_position);
        }
        vec4 layer_color = texture(sampler0, off_tc);
        if(i == 0)
            fcolor_accum = layer_color;
        else if (layer_color.a == 1.0)
            fcolor_accum = layer_color;
    }
	
    color = fcolor_accum;
}