#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;
layout(location = 2) in vec3 instance_row0;  // NEW: instance transform row 0
layout(location = 3) in vec3 instance_row1;  // NEW: instance transform row 1
layout(location = 4) in vec3 instance_row2;  // NEW: instance transform row 2
layout(location = 5) in vec4 tile_params;      // per-instance tile parameters

// Passed to fragment shader
out vec2 texcoord;
flat out vec4 instanceTileParams;

// Application data
uniform mat3 projection;
uniform vec2 camera_position;  // for parallax

void main()
{
    // Build per-instance transform from instance attributes
    mat3 transform = mat3(instance_row0, instance_row1, instance_row2);
    vec3 pos = projection * (transform * vec3(in_position.xy, 1.0));
    gl_Position = vec4(pos.xy, in_position.z, 1.0);
    texcoord = in_texcoord;
    instanceTileParams = tile_params;
}