#version 330

in vec2 texcoord;
out vec4 fragColor;

uniform sampler2D tex;

void main()
{
    fragColor = texture(tex, texcoord);
}
