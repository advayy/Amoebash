#version 330

in vec2 frag_texcoord;
out vec4 out_color;

uniform sampler2D texture0;

void main() 
{
    out_color = texture(texture0, frag_texcoord);
}