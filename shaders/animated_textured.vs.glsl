#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;

// Application data
uniform mat3 transform;
uniform mat3 projection;

uniform int total_frames;
uniform int current_frame;

void main()
{
    // normalized texture coordinates
    float sprite_width = 1.0 / float(total_frames);
    float frame_offset = float(current_frame) * sprite_width;

    // Adjust texture coordinates to fit within one sprite's range
    texcoord = vec2(frame_offset + in_texcoord.x * sprite_width, in_texcoord.y);

    vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
    gl_Position = vec4(pos.xy, in_position.z, 1.0);
}