#version 330

in vec2 texcoord;
out vec4 color;

uniform sampler2D health_texture;

uniform float current_health;
uniform float max_health;

void main() {
    vec4 base_color = texture(health_texture, texcoord);

    color = base_color;
    float health = current_health / float(max_health);

    if (base_color.r > 0.98 && base_color.g > 0.98 && base_color.b > 0.98) {
        
        vec4 bar_color;

        if (health >= 0.5) {
            bar_color = vec4(0.0, 1.0, 0.0, 1.0); // Green
        } else if (health >= 0.3) {
            bar_color = vec4(1.0, 1.0, 0.0, 1.0); // Yellow
        } else {
            bar_color = vec4(1.0, 0.0, 0.0, 1.0); // Red
        }

        if (texcoord.x < health) {
            color = bar_color;
        } else {
            color = vec4(0.0, 0.0, 0.0, 1.0);
        }
    }
}
