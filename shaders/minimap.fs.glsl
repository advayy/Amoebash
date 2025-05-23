#version 330

// From vertex shader
in vec2 texcoord;
in vec3 position;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;

// Output color
layout(location = 0) out  vec4 color;

// Number of tiles in the texture - layers go from left being lowest (1) to right being highest
// Used for paralax mapping
uniform int map_width;
uniform int map_height;
uniform vec2 player_grid_position;
// if theres multiple frames you need to merge all three frames into one

uniform int map_array[400];
uniform int map_visited_array[400];

vec4 procColor() {
    vec4 fcolor = vec4(0.5, 0.0, 0.0, 0.25);
    
    int iX = int(texcoord.y * float(map_width));
    int iY = int(texcoord.x * float(map_height));
    int index = iX + iY * map_width;

    if (iX == 0 || iX == map_width - 1 || iY == 0 || iY == map_height - 1) {
        fcolor = vec4(0.75, 0.75, 0.75, 1.0);
    } else if (map_visited_array[index] == 0) {
        fcolor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        if (map_array[index] == 1) {
            fcolor = vec4(0.75, 0.75, 0.75, 1.0);
        } else if (map_array[index] == 2) {
            fcolor = vec4(1.0, 1.0, 0.0, 1.0);
        }
    }

    return fcolor;
}


void main()
{	
	vec4 fcolor = vec4(0.0, 0.0, 0.0, 0.5);

	// normalize the position, positions are in grid coordinates ranging from -map_width/2 to map_width/2, -map_height/2 to map_height/2
	vec2 normalized_player_position = vec2((player_grid_position.x) / float(map_width), (player_grid_position.y) / float(map_height));

	// calculate grid cell normalized width and height
	vec2 grid_cell_size = vec2(1.0 / float(map_width), 1.0 / float(map_height));	

	// if the texcoord is at the camera position, set the color to white
	if (texcoord.x >= normalized_player_position.x && texcoord.x < normalized_player_position.x + grid_cell_size.x) {
		if (texcoord.y >= normalized_player_position.y && texcoord.y < normalized_player_position.y + grid_cell_size.y) {
			fcolor = vec4(0.0, 1.0, 1.0, 1.0);
			color = fcolor;
			return;
		} 
	} 
	
	fcolor = procColor();
	color = fcolor;	
}
