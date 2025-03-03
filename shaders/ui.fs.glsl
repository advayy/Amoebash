#version 330

in vec2 texcoord;
uniform sampler2D sampler0;
layout(location = 0) out vec4 color;

void main() {
    color = texture(sampler0, texcoord);
    }
