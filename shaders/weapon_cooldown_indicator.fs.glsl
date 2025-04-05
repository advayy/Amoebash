#version 330

in vec2 fragTexCoord;
out vec4 outColor;

uniform sampler2D tex;
uniform float cooldown_ratio;

void main() {
    vec4 texColor = texture(tex, fragTexCoord);
    vec2 centeredCoord = fragTexCoord - vec2(0.5);

    float angle = atan(centeredCoord.y, centeredCoord.x);
    angle = angle + 3.1415926 / 2.0;
    if (angle < 0.0)
        angle += 2.0 * 3.1415926;
    
    float pct = cooldown_ratio;

    float angleLimit = pct * 2.0 * 3.1415926;

    if (length(centeredCoord) > 0.5 || angle < angleLimit) {
        discard;
    }

    outColor = vec4(1.0, 1.0, 1.0, 0.8 * texColor.a);
}
