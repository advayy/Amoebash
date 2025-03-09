#version 330

// from vertex shader
in vec2 texcoord;
in vec4 color;

// output color
out vec4 FragColor;

// texture sampler
uniform sampler2D sampler0;

void main() {
    // sample texture and multiply by instance color
    vec4 tex_color = texture(sampler0, texcoord);
    FragColor = tex_color * color;
}