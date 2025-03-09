#version 330

// Input vertex
layout(location = 0) in vec2 in_position;      // Base vertex positions
layout(location = 1) in vec2 in_texcoord;      // Base texture coordinates
layout(location = 2) in vec2 in_offset;        // Instance position
layout(location = 3) in vec2 in_scale;         // Instance scale
layout(location = 4) in vec4 in_color;         // Instance color
layout(location = 5) in float in_rotation;     // Instance rotation

// Output--> fragment shader
out vec2 texcoord;
out vec4 color;

uniform mat3 projection;

void main() {
    // compute rotation matrix
    float c = cos(in_rotation);
    float s = sin(in_rotation);
    mat2 rot = mat2(c, -s, s, c);

    // transform vertex position based on instance data
    vec2 pos = rot * (in_position * in_scale) + in_offset;
    
    // transform position to clip space
    vec3 clip_position = projection * vec3(pos, 1.0);
    gl_Position = vec4(clip_position.xy, 0.0, 1.0);

    // pass color + texcoord to fragment shader
    color = in_color;
    texcoord = in_texcoord;
}