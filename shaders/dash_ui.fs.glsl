#version 330

in vec2 texcoord;
out vec4 color;

uniform sampler2D dash_texture;
uniform float brightness;  // Controls dot visibility, not used yet

void main() {
    vec4 texColor = texture(dash_texture, texcoord);
    color = vec4(texColor.rgb, texColor.a);
}
