#version 330

in vec2 texcoord;
out vec4 color;

uniform sampler2D health_texture;
uniform float health;

void main() {
    vec4 base_color = texture(health_texture, texcoord);
    float grayscale = dot(base_color.rgb, vec3(0.299, 0.587, 0.114));
    base_color.rgb = vec3(grayscale);

    color = base_color;

    if (grayscale > 0.90) {
        vec4 bar_color;
        if (health >= 50.0) {
            bar_color = vec4(0.0, 1.0, 0.0, 1.0); // Green
        } else if (health >= 30.0) {
            bar_color = vec4(1.0, 1.0, 0.0, 1.0); // Yellow
        } else {
            bar_color = vec4(1.0, 0.0, 0.0, 1.0); // Red
        }

        float ratio = health / 100.0;
        if (texcoord.x < ratio) {
            color = bar_color;
        } else {
            color = vec4(0.0, 0.0, 0.0, 1.0);
        }
    }
}
