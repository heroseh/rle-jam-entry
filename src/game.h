#ifndef _RLE_H_
#define _RLE_H_

#include <deps/hero/core.h>
#include <deps/hero/window.h>
#include <deps/hero/gfx.h>
#include <deps/hero/ui.h>

#include <deps/hero/core.c>
#include <deps/hero/window.c>
#include <deps/hero/gfx.c>
#include <deps/hero/ui.c>

#include <deps/FastNoiseLite.h>

#include "shaders/island_gen_debug_shared.h"
#include "shaders/play_shared.h"

#define GAME_WINDOW_TITLE "RLE"
#define GAME_WINDOW_WIDTH 1280
#define GAME_WINDOW_HEIGHT 720

// ===========================================
//
//
// UI
//
//
// ===========================================

void game_ui_value_adjuster_f(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, HeroString name, float* value, float step);
void game_ui_value_adjuster_u(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, HeroString name, U32* value, int step);

// ===========================================
//
//
// Island Generator
//
//
// ===========================================

typedef struct GameIslandGenConfig GameIslandGenConfig;
struct GameIslandGenConfig {
	U32 seed;
	F32 frequency;
	U32 fractal_type;
	U32 octaves;
	F32 lacunarity;
	F32 deep_sea_max;
	F32 sea_max;
	F32 ground_max;
	F32 falloff_map_intensity;
};

enum {
	GAME_ISLAND_GEN_CONFIG_SIMPLE,
	GAME_ISLAND_GEN_CONFIG_NOT_SO_SIMPLE,
	GAME_ISLAND_GEN_CONFIG_NOT_SIMPLE,
	GAME_ISLAND_GEN_CONFIG_COUNT,
};

extern GameIslandGenConfig game_island_gen_configs[GAME_ISLAND_GEN_CONFIG_COUNT];

void game_island_gen(GameIslandGenConfig* config, F32* tile_height_map_out);

// ===========================================
//
//
// Scene
//
//
// ===========================================

typedef U8 GameSceneType;
enum {
	GAME_SCENE_TYPE_ISLAND_GEN_DEBUG,
	GAME_SCENE_TYPE_PLAY,
};

#define HERO_STACK_ELMT_TYPE GameSceneType
#include "deps/hero/stack_gen.inl"

typedef U8 GameSceneState;
enum {
	GAME_SCENE_STATE_UNLOADED,
	GAME_SCENE_STATE_LOADING,
	GAME_SCENE_STATE_PAUSED,
	GAME_SCENE_STATE_RUNNING,
};

typedef struct GameScene GameScene;

typedef HeroResult (*GameSceneLoadFn)(GameScene* scene);
typedef HeroResult (*GameSceneUnloadFn)(GameScene* scene);
typedef HeroResult (*GameSceneUpdateFn)(GameScene* scene, GameScene** replace_with_scene_out);
typedef HeroResult (*GameSceneUpdateRenderDataFn)(GameScene* scene);

struct GameScene {
	GameSceneType               type;
	GameSceneState              state;
	GameSceneLoadFn             load_fn;
	GameSceneUnloadFn           unload_fn;
	GameSceneUpdateFn           update_fn;
	GameSceneUpdateRenderDataFn update_render_data_fn;
};

static inline HeroResult game_scene_load(GameScene* scene) {
	HERO_ASSERT(scene->state == GAME_SCENE_STATE_UNLOADED, "can only load a scene that is unloaded");
	scene->state = GAME_SCENE_STATE_LOADING;
	return scene->load_fn(scene);
}

static inline HeroResult game_scene_unload(GameScene* scene) {
	HERO_ASSERT(scene->state != GAME_SCENE_STATE_UNLOADED, "can only unload a scene that is loaded");
	HeroResult result = scene->unload_fn(scene);
	if (result < 0) {
		return result;
	}

	scene->state = GAME_SCENE_STATE_UNLOADED;
	return HERO_SUCCESS;
}

static inline HeroResult game_scene_update(GameScene* scene, GameScene** replace_with_scene_out) {
	HERO_ASSERT(scene->state != GAME_SCENE_STATE_UNLOADED, "can only update a scene that is loaded");
	return scene->update_fn(scene, replace_with_scene_out);
}

static inline HeroResult game_scene_update_render_data(GameScene* scene) {
	HERO_ASSERT(scene->state != GAME_SCENE_STATE_UNLOADED, "can only render a scene that is loaded");
	return scene->update_render_data_fn(scene);
}

// ===========================================
//
//
// Scene: Island Generator Debug
//
//
// ===========================================

typedef struct GameSceneIslandGenDebugRenderData GameSceneIslandGenDebugRenderData;
struct GameSceneIslandGenDebugRenderData {
	F32* tile_height_map;
	F32 deep_sea_max;
	F32 sea_max;
	F32 ground_max;
	bool show_grid;
	bool show_height_map;
	bool has_been_updated;
};

typedef struct GameSceneIslandGenDebug GameSceneIslandGenDebug;
struct GameSceneIslandGenDebug {
	GameScene scene;
	GameIslandGenConfig config;
	U32 config_idx;
	U32 seed;
	GameSceneIslandGenDebugRenderData render_data;
	HeroUIImageAtlasId colored_height_map_image_atlas_id;
};

HeroResult game_scene_island_gen_debug_init(GameScene** scene_out);

// ===========================================
//
//
// Scene: Play
//
//
// ===========================================

typedef U8 GameMoveTargetMode;
enum {
	GAME_MOVE_TARGET_MODE_ENTITY,
	GAME_MOVE_TARGET_MODE_DIRECTION,
	GAME_MOVE_TARGET_MODE_TILE,
	GAME_MOVE_TARGET_MODE_SELF,
};

typedef U8 GameMoveCombatMode;
enum {
	GAME_MOVE_COMBAT_MODE_CQC,
	GAME_MOVE_COMBAT_MODE_RANGED,
};

typedef struct GameMove GameMove;
struct GameMove {
	HeroString         name;
	U8                 damage;
	U8                 range_radius;
	F32                spread_angle;
	U8                 splash_radius;
	GameMoveTargetMode target_mode;
	GameMoveCombatMode combat_mode;
};

enum {
	GAME_MOVE_SHOOT,
	GAME_MOVE_SHOOT_SPREAD,
	GAME_MOVE_WIDE_SPIN,
	GAME_MOVE_CANNON,

	GAME_MOVE_COUNT,
};

static GameMove game_moves[] = {
	[GAME_MOVE_SHOOT] = {
		.name =         hero_string_lit("shoot"),
		.damage =       22,
		.range_radius = 5,
		.splash_radius = 1.f,
		.target_mode =  GAME_MOVE_TARGET_MODE_ENTITY,
		.combat_mode =  GAME_MOVE_COMBAT_MODE_RANGED,
	},
	[GAME_MOVE_SHOOT_SPREAD] = {
		.name =         hero_string_lit("shoot spread"),
		.damage =       22,
		.range_radius = 4,
		.spread_angle = HERO_DEGREES_TO_RADIANS(90.f),
		.target_mode =  GAME_MOVE_TARGET_MODE_DIRECTION,
		.combat_mode =  GAME_MOVE_COMBAT_MODE_RANGED,
	},
	[GAME_MOVE_WIDE_SPIN] = {
		.name =         hero_string_lit("wide spin"),
		.damage =       22,
		.range_radius = 3,
		.target_mode =  GAME_MOVE_TARGET_MODE_SELF,
		.combat_mode =  GAME_MOVE_COMBAT_MODE_RANGED,
	},
	[GAME_MOVE_CANNON] = {
		.name =         hero_string_lit("cannon"),
		.damage =       22,
		.range_radius = 6,
		.target_mode =  GAME_MOVE_TARGET_MODE_TILE,
		.combat_mode =  GAME_MOVE_COMBAT_MODE_RANGED,
		.splash_radius = 3.f,
	},
};

typedef U8 GameDirection;
enum {
	GAME_DIRECTION_NONE,
	GAME_DIRECTION_UP,
	GAME_DIRECTION_DOWN,
	GAME_DIRECTION_LEFT,
	GAME_DIRECTION_RIGHT,

	GAME_DIRECTION_COUNT,
};

extern Vec3 game_direction_vectors[GAME_DIRECTION_COUNT];

typedef U8 GameInputFlags;
typedef U8 GameInput;
enum {
	GAME_INPUT_UP,
	GAME_INPUT_DOWN,
	GAME_INPUT_LEFT,
	GAME_INPUT_RIGHT,
	GAME_INPUT_A,
	GAME_INPUT_B,
	GAME_INPUT_X,
	GAME_INPUT_Y,

	GAME_INPUT_COUNT,
};

typedef struct GameInputState GameInputState;
struct GameInputState {
	GameInputFlags is_pressed;
	GameInputFlags has_been_pressed;
	GameInputFlags has_been_released;
};

HeroScanCode game_input_default_keyboard_map[GAME_INPUT_COUNT] = {
	[GAME_INPUT_UP] = HERO_KEY_CODE_W,
	[GAME_INPUT_DOWN] = HERO_KEY_CODE_S,
	[GAME_INPUT_LEFT] = HERO_KEY_CODE_A,
	[GAME_INPUT_RIGHT] = HERO_KEY_CODE_D,
	[GAME_INPUT_A] = HERO_KEY_CODE_J,
	[GAME_INPUT_B] = HERO_KEY_CODE_K,
	[GAME_INPUT_X] = HERO_KEY_CODE_U,
	[GAME_INPUT_Y] = HERO_KEY_CODE_I,
};

typedef U8 GamePlayerActionState;
enum {
	GAME_PLAYER_ACTION_STATE_MOVING,
	GAME_PLAYER_ACTION_STATE_INTERACT_DIALOG,
	GAME_PLAYER_ACTION_STATE_MOVE_DIALOG,
	GAME_PLAYER_ACTION_STATE_MOVE_TARGET_ENTITY,
	GAME_PLAYER_ACTION_STATE_MOVE_TARGET_TILE,
	GAME_PLAYER_ACTION_STATE_MOVE_ANIMATE,
	GAME_PLAYER_ACTION_STATE_ITEM,
	GAME_PLAYER_ACTION_STATE_ABILITY_DIALOG,
	GAME_PLAYER_ACTION_STATE_ABILITY_TARGET_ENTITY,
	GAME_PLAYER_ACTION_STATE_ABILITY_TARGET_DIRECTION,
};

typedef U8 GameEntityType;
enum {
	GAME_ENTITY_TYPE_HUMAN,

	GAME_ENTITY_TYPE_COUNT,
};

typedef U32 GameEntityFlags;
enum {
	GAME_ENTITY_FLAGS_AUTO_Y_FROM_HEIGHT_MAP = 0x1,
	GAME_ENTITY_FLAGS_BILLBOARD =              0x2,
	GAME_ENTITY_FLAGS_PLAYER =                 0x4,
	GAME_ENTITY_FLAGS_STOP_ON_NEXT_TILE =      0x8,
	GAME_ENTITY_FLAGS_INVALID_RANGE =         0x10,
	GAME_ENTITY_FLAGS_VALID_SPLASH =          0x20,
};

typedef struct GameEntity GameEntity;
struct GameEntity {
	HeroObjectHeader header;
	GameEntityType   type;
	GameEntityFlags  flags;
	Vec3             position;
	GameDirection    direction;
	F32              stop_on_next_tile_start;
	U32              step_idx;
	U32              steps_per_move;
	U32              health;
	U32              animate_og_health;
	char             health_string[10]; // TODO: use a temporary frame lifetime allocator
	char             name[16];
};

typedef struct GameRenderEntityBillboard GameRenderEntityBillboard;
struct GameRenderEntityBillboard {
	GameEntityType   type;
	Vec3             position;
	U8               ascii;
	HeroColor        color;
	HeroColor        tint;
};

#define HERO_STACK_ELMT_TYPE GameRenderEntityBillboard
#include "deps/hero/stack_gen.inl"

HERO_TYPEDEF_OBJECT_ID(GameEntityId);
#define HERO_OBJECT_ID_TYPE GameEntityId
#define HERO_OBJECT_TYPE GameEntity
#include "deps/hero/object_pool_gen.inl"

typedef struct GameScenePlayRenderData GameScenePlayRenderData;
struct GameScenePlayRenderData {
	F32* tile_height_map;
	F32* voxel_height_map;
	U32* terrain_tile_tint_map;
	Vec3 camera_forward;
	Vec3 camera_position;
	Quat camera_rotation;
	HeroStack(GameRenderEntityBillboard) entity_billboards;
};

typedef U32 GameScenePlayFlags;
enum {
	GAME_SCENE_PLAY_GLOW = 0x1,
};

typedef struct GameCamera GameCamera;
struct GameCamera {
	GameEntityId target_entity_id;
	Vec3 target_position;

	Vec3 forward;
	Vec3 position;
	Quat rotation;
	F32  yaw;
	F32  pitch;
};

typedef U8 GameEntityTurnType;
enum {
	GAME_ENTITY_TURN_TYPE_NONE,
	GAME_ENTITY_TURN_TYPE_MOVE_TILE,
	GAME_ENTITY_TURN_TYPE_USE_MOVE,
	GAME_ENTITY_TURN_TYPE_USE_ABILITY,
	GAME_ENTITY_TURN_TYPE_USE_ITEM,
};

typedef struct GameEntityTurn GameEntityTurn;
struct GameEntityTurn {
	GameEntityTurnType type;
	union {
		struct {
			GameDirection direction;
		} move_tile;
		struct {
			GameMove* move;
			union {
				GameEntityId target_entity_id;
				Vec2 target_direction;
				struct {
					U32 x;
					U32 y;
				} target_tile;
			} data;
		} use_move;
	} data;
};

typedef struct GameEntityFilterArgs GameEntityFilterArgs;
struct GameEntityFilterArgs {
	U32 origin_tile_x;
	U32 origin_tile_y;
	U32 radius;

	Vec2 aim_direction;
	F32 spread_angle;

	U32 splash_origin_tile_x;
	U32 splash_origin_tile_y;
	F32 splash_radius;

	GameEntityId ignore_entity_id;

	F32 _squared_radius;
	F32 _half_angle_cos;
	F32 _squared_half_angle_cos;
	F32 _squared_splash_radius;
	F32 _prev_squared_distance;
	bool _valid;
	bool _splash;
};

typedef struct GameScenePlay GameScenePlay;
struct GameScenePlay {
	GameScene scene;
	GameScenePlayFlags flags;
	HeroObjectPool(GameEntity) entity_pool;
	GameEntityId player_entity_id;
	GameEntityId enemy_entity_id;
	GameCamera camera;
	F32* tile_height_map;
	F32* voxel_height_map;
	U32* terrain_tile_tint_map;
	GameScenePlayRenderData render_data;
	HeroUIImageAtlasId ui_image_atlas_id;
	F32 npc_move_time;
	Mat4x4 mvp;
	GamePlayerActionState player_action_state;
	GameEntityId player_target_entity_id;
	F32 player_move_animate_time;
	U32 player_selected_move_idx;
	Vec2 player_move_aim_direction;
	U32 player_target_tile_x;
	U32 player_target_tile_y;
	F32 glow_intensity;
	GameEntityTurn player_entity_turn;
};

HeroResult game_scene_play_init(GameScene** scene_out);

HeroResult game_scene_play_entity_add(GameScenePlay* scene, GameEntityType type, GameEntity** entity_out, GameEntityId* id_out);
HeroResult game_scene_play_entity_get(GameScenePlay* scene, GameEntityId id, GameEntity** out);
HeroResult game_scene_play_entity_update(GameScenePlay* scene, GameEntityId id, GameEntity* entity);
HeroResult game_scene_play_player_handle_input(GameScenePlay* scene, GameInputState state);
HeroResult game_scene_play_entity_handle_input(GameScenePlay* scene, GameEntityId id, GameInputState state);
U8 game_scene_play_entity_ascii(GameEntity* entity);
HeroResult game_scene_play_entity_find_closest_in_range(GameScenePlay* scene, GameEntityFilterArgs* args, GameEntityId* id_out);
HeroResult game_scene_play_entity_iter_next_in_range(GameScenePlay* scene, GameEntityId* id_mut, GameEntityFilterArgs* args, bool backwards);
void game_scene_play_entity_apply_damage(GameScenePlay* scene, GameEntityId target_id);
void game_scene_play_camera_set_target_tile(GameScenePlay* scene, U32 tile_x, U32 tile_y);
void game_scene_play_camera_set_target_entity(GameScenePlay* scene, GameEntityId entity_id);
void game_scene_play_camera_update(GameScenePlay* scene);
void game_scene_play_update_mvp(GameScenePlay* scene);
Vec2 game_scene_play_world_to_screen_space_point(GameScenePlay* scene, Vec3 position);

void game_scene_play_entity_filter_start(GameScenePlay* scene, GameEntityFilterArgs* args);
void game_scene_play_entity_filter_check(GameScenePlay* scene, GameEntityId entity_id, GameEntity* entity, GameEntityFilterArgs* args);

// ===========================================
//
//
// Logic
//
//
// ===========================================

void game_logic_init(void);
void game_logic_update(void);

// ===========================================
//
//
// Graphics: Common
//
//
// ===========================================

HeroResult game_gfx_shader_init(const char* name, HeroShaderType type, HeroShaderId* id_out);
HeroResult game_gfx_swapchain_frame_buffers_reinit(HeroSwapchain* swapchain);

// ===========================================
//
//
// Graphics Scene: Island Generator Debug
//
//
// ===========================================

typedef struct GameGfxIslandGenDebug GameGfxIslandGenDebug;
struct GameGfxIslandGenDebug {
	GameSceneIslandGenDebugRenderData scene_render_data;

	HeroVertexLayoutId     vertex_layout_id;
	HeroBufferId           vertex_buffer_id;
	HeroBufferId           index_buffer_id;
	HeroBufferId           uniform_buffer_id;
	HeroShaderId           shader_id;
	HeroDescriptorPoolId   descriptor_pool_id;
	HeroShaderGlobalsId    shader_globals_id;
	HeroPipelineId         pipeline_id;
	HeroMaterialId         material_id;
	HeroRenderPassLayoutId render_pass_layout_id;
	HeroRenderPassId       render_pass_id;
	HeroFrameBufferId      frame_buffer_id;
	HeroImageId            attachment_image_id;
	HeroImageId            tile_height_map_image_id;
};

HeroResult game_gfx_island_gen_debug_init();
HeroResult game_gfx_island_gen_debug_deinit();
HeroResult game_gfx_island_gen_debug_render(HeroCommandRecorder* command_recorder);

// ===========================================
//
//
// Graphics Scene: Play
//
//
// ===========================================

typedef struct GameGfxPlay GameGfxPlay;
struct GameGfxPlay {
	GameScenePlayRenderData scene_render_data;

	HeroShaderId           voxel_raytrace_shader_id;
	HeroShaderGlobalsId    voxel_raytrace_shader_globals_id;
	HeroVertexLayoutId     terrain_tile_vertex_layout_id;
	HeroBufferId           terrain_tile_vertex_buffer_id;
	HeroBufferId           terrain_tile_index_buffer_id;
	HeroShaderId           terrain_tile_shader_id;
	HeroShaderGlobalsId    terrain_tile_shader_globals_id;
	HeroPipelineId         terrain_tile_pipeline_id;
	HeroMaterialId         terrain_tile_material_id;
	HeroImageId            terrain_tile_tint_image_id;
	HeroImageId            voxel_height_image_id;
	HeroVertexLayoutId     model_vertex_layout_id;
	HeroBufferId           model_vertex_buffer_id;
	HeroBufferId           model_index_buffer_id;
	HeroShaderId           model_shader_id;
	HeroShaderGlobalsId    model_shader_globals_id;
	HeroPipelineId         model_pipeline_id;
	HeroMaterialId         model_material_id;
	HeroVertexLayoutId     billboard_vertex_layout_id;
	HeroBufferId           billboard_vertex_buffer_id;
	HeroBufferId           billboard_index_buffer_id;
	HeroShaderId           billboard_shader_id;
	HeroShaderGlobalsId    billboard_shader_globals_id;
	HeroPipelineId         billboard_pipeline_id;
	HeroMaterialId         billboard_material_id;
	HeroBufferId           uniform_buffer_id;
	HeroDescriptorPoolId   descriptor_pool_id;
	HeroRenderPassLayoutId render_pass_layout_id;
	HeroRenderPassId       render_pass_id;
	HeroFrameBufferId      frame_buffer_id;
	HeroImageId            voxel_raytrace_color_image_id;
	HeroImageId            voxel_raytrace_depth_image_id;
	HeroImageId            attachment_image_id;
	HeroImageId            depth_image_id;
};

HeroResult game_gfx_play_init();
HeroResult game_gfx_play_deinit();
HeroResult game_gfx_play_render(HeroCommandRecorder* command_recorder);

// ===========================================
//
//
// Graphics
//
//
// ===========================================

typedef struct GameGfx GameGfx;
struct GameGfx {
	HeroLogicalDevice* ldev;
	HeroStack(GameSceneType) scene_queue;

	GameGfxIslandGenDebug island_gen_debug;
	GameGfxPlay play;

	HeroSwapchainId swapchain_id;
	HeroFrameBufferId* swapchain_frame_buffer_ids;
	U32 swapchain_frame_buffers_count;
	U32 swapchain_image_idx;
	U32 render_width;
	U32 render_height;

	HeroRenderPassLayoutId render_pass_layout_id;
	HeroRenderPassId render_pass_id;

	HeroRenderGraphId render_graph_id;
	HeroImageInfoId image_info_swapchain_sized_rgba8_id;
	HeroImageInfoId image_info_swapchain_sized_d32_id;
	HeroImageInfoId image_info_voxel_height_map_id;

	HeroShaderId ui_shader_id;

	HeroDescriptorPoolId descriptor_pool_id;
	HeroImageId noise_image_id;
	HeroImageId font_ascii_image_id;
	HeroSamplerId clamp_nearest_sampler_id;
	HeroSamplerId clamp_linear_sampler_id;
};

void game_gfx_init(void);
void game_gfx_render(void);

// ===========================================
//
//
// Game
//
//
// ===========================================

typedef struct Game Game;
struct Game {
	HeroWindowSys window_sys;
	HeroWindowId window_id;
	HeroUIWindowId ui_window_id;
	HeroUIImageAtlasId ui_ascii_image_atlas_id;
	GameScene* scene;
	F32 dt;

	GameGfx gfx;
};

extern Game game;

void game_init(void);

typedef struct GameUIStyleSheet GameUIStyleSheet;
struct GameUIStyleSheet {
	HeroUIWidgetStyle health_bar[HERO_UI_WIDGET_STATE_COUNT];
	HeroUIWidgetStyle health_bar_inner[HERO_UI_WIDGET_STATE_COUNT];
	HeroUIWidgetStyle health_bar_text[HERO_UI_WIDGET_STATE_COUNT];
	HeroUIWidgetStyle dialog_text[HERO_UI_WIDGET_STATE_COUNT];
	HeroUIWidgetStyle move_text[HERO_UI_WIDGET_STATE_COUNT];
	HeroUIWidgetStyle controls_hint_text[HERO_UI_WIDGET_STATE_COUNT];
};

static GameUIStyleSheet game_ui_ss = {
	.health_bar = {
		[HERO_UI_WIDGET_STATE_DEFAULT] = {
			.margin = HERO_THICKNESS_INIT_EVEN(0),
			.padding = HERO_THICKNESS_INIT_EVEN(4.f),
			.background_color = hero_color_init(0x00, 0x80, 0x80, 0xff),
			.border_width = hero_ui_border_width_default,
			.border_color = hero_color_init(0xff, 0x66, 0xcc, 0xff),
			.radius = hero_ui_radius_default,
			.text_color = 0,
			.text_font_id = 0,
			.text_line_height = 0,
			.text_wrap = false,
		},
	},
	.health_bar_inner = {
		[HERO_UI_WIDGET_STATE_DEFAULT] = {
			.margin = HERO_THICKNESS_INIT_EVEN(0),
			.padding = HERO_THICKNESS_INIT_EVEN(0),
			.background_color = hero_color_init(0xff, 0x66, 0xcc, 0xff),
			.border_width = 0.f,
			.border_color = 0,
			.radius = hero_ui_radius_default,
			.text_color = 0,
			.text_font_id = 0,
			.text_line_height = 0,
			.text_wrap = false,
		},
	},
	.health_bar_text = {
		[HERO_UI_WIDGET_STATE_DEFAULT] = {
			.margin = HERO_THICKNESS_INIT(0.f, 4.f, 0.f, 0.f),
			.padding = HERO_THICKNESS_INIT_EVEN(0),
			.background_color = 0,
			.border_width = 0.f,
			.border_color = 0,
			.radius = 0.f,
			.text_color = -1,
			.text_font_id = 0,
			.text_line_height = 14,
			.text_wrap = false,
		},
	},
	.dialog_text = {
		[HERO_UI_WIDGET_STATE_DEFAULT] = {
			.margin = HERO_THICKNESS_INIT(0.f, 4.f, 0.f, 0.f),
			.padding = HERO_THICKNESS_INIT_EVEN(4.f),
			.background_color = hero_color_init(0x00, 0xa0, 0xa0, 0xff),
			.border_width = 4.f,
			.border_color = hero_color_init(0xff, 0x88, 0xdd, 0xff),
			.radius = 0.f,
			.text_color = -1,
			.text_font_id = 0,
			.text_line_height = 28,
			.text_wrap = false,
		},
	},
	.move_text = {
		[HERO_UI_WIDGET_STATE_DEFAULT] = {
			.margin = HERO_THICKNESS_INIT(0.f, 4.f, 0.f, 4.f),
			.padding = HERO_THICKNESS_INIT_EVEN(4.f),
			.background_color = 0,
			.border_width = 0.f,
			.border_color = 0,
			.radius = 0.f,
			.text_color = -1,
			.text_font_id = 0,
			.text_line_height = 28,
			.text_wrap = false,
		},
		[HERO_UI_WIDGET_STATE_FOCUSED] = {
			.margin = HERO_THICKNESS_INIT(0.f, 4.f, 0.f, 4.f),
			.padding = HERO_THICKNESS_INIT_EVEN(0),
			.background_color = 0,
			.border_width = 4.f,
			.border_color = hero_color_init(0xff, 0x88, 0xdd, 0xff),
			.radius = 0.f,
			.text_color = -1,
			.text_font_id = 0,
			.text_line_height = 28,
			.text_wrap = false,
		},
	},
	.controls_hint_text = {
		[HERO_UI_WIDGET_STATE_DEFAULT] = {
			.margin = HERO_THICKNESS_INIT_EVEN(4.f),
			.padding = HERO_THICKNESS_INIT_EVEN(4.f),
			.background_color = 0,
			.border_width = 0.f,
			.border_color = 0,
			.radius = 0.f,
			.text_color = hero_color_init(0x20, 0xcc, 0x30, 0xff),
			.text_font_id = 0,
			.text_line_height = 28,
			.text_wrap = false,
		},
		[HERO_UI_WIDGET_STATE_DISABLED] = {
			.margin = HERO_THICKNESS_INIT_EVEN(4.f),
			.padding = HERO_THICKNESS_INIT_EVEN(4.f),
			.background_color = 0,
			.border_width = 0.f,
			.border_color = 0,
			.radius = 0.f,
			.text_color = hero_color_init(0xaa, 0x10, 0x10, 0xff),
			.text_font_id = 0,
			.text_line_height = 28,
			.text_wrap = false,
		},
	},
};

#endif // _RLE_H_


