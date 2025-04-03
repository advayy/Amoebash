#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;

// Output color
layout(location = 0) out  vec4 color;

uniform float max_danger;
uniform float current_danger;

void main()
{
    // SIZE IS 2x {24, 104}, BOTTOM 2 and TOP 2 PIXELS ARE WHITE
    
    // If a texcoord is white make its alpha 0.7 and return, if a texcoord is transparent ie alpha =0 then return as it is, 
    // if a texcoord is green, then replace with red or clear.
    // The aount of red will be like a thermometer - ie current danger out of max danger sampled from the green pixels 
    vec4 texColor = texture(sampler0, texcoord);

    if (texColor.a == 0.0) {
        color = texColor;
        return;
    }

    if (texColor.rgb == vec3(1.0)) {
        color = vec4(1.0, 1.0, 1.0, 0.4);
        return;
    }

    float fillLevel = current_danger / max_danger;
    float fillLevel_inv = 1.0 - fillLevel;


    vec3 yellow = vec3(1.0, 1.0, 0.0);
    vec3 orange = vec3(1.0, 0.5, 0.0);
    vec3 red = vec3(1.0, 0.0, 0.0);
    vec3 purple = vec3(0.5, 0.0, 0.5);
    vec3 black = vec3(0.0, 0.0, 0.0);
   
    vec3 dangerColor;
    
    if (fillLevel < 0.25) {
        dangerColor = mix(yellow, orange, fillLevel / 0.25);
    } else if (fillLevel < 0.50) {
        dangerColor = mix(orange, red, (fillLevel - 0.25) / 0.25);
    } else if (fillLevel < 0.75) {
        dangerColor = mix(red, purple, (fillLevel - 0.50) / 0.25);
    } else {
        dangerColor = mix(purple, black, (fillLevel - 0.75) / 0.25);
    }


    if (texColor.g > 0.9) {
        if (texcoord.y >= fillLevel_inv) {
            color = vec4(dangerColor, 0.5);
        } else {
            color = vec4(0.0, 0.0, 0.0, 0.0);
        }
        return;
    }

    color = texColor;
}
