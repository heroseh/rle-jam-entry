#ifndef _GAME_SHADER_SHARED_H_
#define _GAME_SHADER_SHARED_H_

#include "../../deps/hero/core_glsl_shared.h"

#define GAME_ISLAND_AXIS_TILES_COUNT      1024
#define GAME_ISLAND_AXIS_CELLS_COUNT      32
#define GAME_ISLAND_AXIS_CELL_TILES_COUNT 32

#define GameTile U32

#define GAME_TILE_HEIGHT_BITS_SHIFT 0
#define GAME_TILE_HEIGHT_BITS_SIZE  8
#define GAME_TILE_HEIGHT(tile) hero_bitfield_extract_u32(tile, GAME_TILE_HEIGHT_BITS_SHIFT, GAME_TILE_HEIGHT_BITS_SIZE)
#define GAME_TILE_HEIGHT_SET(tile_ptr, value) hero_bitfield_insert_u32(tile_ptr, value, GAME_TILE_HEIGHT_BITS_SHIFT, GAME_TILE_HEIGHT_BITS_SIZE)

#endif // _GAME_SHADER_SHARED_H_
