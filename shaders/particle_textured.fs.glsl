#version 330

in vec2 texcoord;
out vec4 fragColor;
in float particle_alpha;

uniform sampler2D tex;

void main()
{
   vec4 texColor = texture(tex, texcoord);
    fragColor = vec4(texColor.rgb, texColor.a * particle_alpha);
}
