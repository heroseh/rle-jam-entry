#ifndef GAME_SHADER_H
#define GAME_SHADER_H

#define GAME_SHADER_ISLAND_GEN_DEBUG 0

// ===========================================
//
//
// Types
//
//
// ===========================================

#extension GL_EXT_buffer_reference : require
#define HERO_SHADER_RO_BUFFER_START(Name) layout(buffer_reference) uniform Name

HERO_SHADER_RO_BUFFER_START(GameShaderIslandGenDebug) {
	Mat4x4 mvp;
	float deep_sea_max;
	float sea_max;
	float ground_max;
	int show_height_map;
	int show_grid;
};

struct GameShaderIslandGenDebug {
	Mat4x4 mvp
	float deep_sea_max
	float sea_max
	float ground_max
	int show_height_map
	int show_grid
}

shader IslandGenDebug {
	ro_buffer(GameShaderIslandGenDebug) globals
	ro_image2d                          tile_height_map
}


#endif // GAME_SHADER_H

