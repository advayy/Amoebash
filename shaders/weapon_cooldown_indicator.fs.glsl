#version 330

in vec2 fragTexCoord;
out vec4 outColor;

uniform sampler2D tex;
uniform float cooldown_ratio;

void main() {
    vec4 texColor = texture(tex, fragTexCoord);
    
    float angle = atan(fragTexCoord.y - 0.5, fragTexCoord.x - 0.5);
    angle = angle < 0.0 ? angle + 2.0 * 3.1415926 : angle;
    float pct = 1.0 - cooldown_ratio;

    if (length(fragTexCoord - vec2(0.5)) > 0.5 || angle > pct * 2.0 * 3.1415926) {
        discard;
    }

    outColor = vec4(1.0, 1.0, 1.0, 0.8 * texColor.a);
}
