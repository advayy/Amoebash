#version 330

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texcoord;
// Instanced transform: occupies attribute locations 2,3,4
layout(location = 2) in mat3 instanceTransform;
layout(location = 5) in float instanceAlpha;

uniform mat3 projection;

out vec2 texcoord;
out float particle_alpha; 

void main()
{
    texcoord = in_texcoord;
    particle_alpha = instanceAlpha;
    vec3 pos = projection * (instanceTransform * vec3(in_position.xy, 1.0));
    gl_Position = vec4(pos.xy, in_position.z, 1.0);
}
