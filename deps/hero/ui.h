#ifndef _HERO_UI_H_
#define _HERO_UI_H_

#include "core.h"
#include "gfx.h"
#include "ui_glsl_shared.h"

// ===========================================
//
//
// General
//
//
// ===========================================

typedef struct HeroUIWindow HeroUIWindow;
typedef struct HeroUIWidget HeroUIWidget;

enum {
	HERO_ERROR_UI_WIDGET_IS_NOT_FOCUSABLE = HERO_ERROR_UI_START,
};

typedef struct HeroUIThickness HeroUIThickness;
struct HeroUIThickness {
	F32 left;
	F32 top;
	F32 right;
	F32 bottom;
};

#define HERO_UI_WIDGET_LIST_ITEM_MEASURE_COUNT 8

#define HERO_UI_LEN_AUTO INFINITY
#define HERO_UI_LEN_FILL (-INFINITY)
#define HERO_UI_LEN_RATIO(ratio) (-(ratio))
#define HERO_UI_LEN_RATIO_GET(ratio) (-(ratio))
#define HERO_UI_LEN_IS_RATIO(cut_length) (isfinite(cut_length) && cut_length < 0.f)
#define HERO_UI_LEN_IS_FIXED(cut_length) (isfinite(cut_length) && cut_length >= 0.f)

#define HERO_THICKNESS_INIT(l, t, r, b) ((HeroUIThickness) { .left = l, .top = t, .right = r, .bottom = b })
#define HERO_THICKNESS_INIT_EVEN(v) ((HeroUIThickness) { .left = v, .top = v, .right = v, .bottom = v })

#define hero_ui_margin_default HERO_THICKNESS_INIT_EVEN(4)
#define hero_ui_padding_default HERO_THICKNESS_INIT_EVEN(4)
#define hero_ui_border_width_default 4
#define hero_ui_radius_default 4

typedef U8 HeroUICut;
enum {
	HERO_UI_CUT_CENTER_HORIZONTAL,
	HERO_UI_CUT_CENTER_VERTICAL,
	HERO_UI_CUT_LEFT,
	HERO_UI_CUT_TOP,
	HERO_UI_CUT_RIGHT,
	HERO_UI_CUT_BOTTOM,
};

static inline bool hero_ui_cut_is_vertical(HeroUICut cut) {
	return cut == HERO_UI_CUT_TOP || cut == HERO_UI_CUT_BOTTOM || cut == HERO_UI_CUT_CENTER_VERTICAL;
}

static inline bool hero_ui_cut_is_horizontal(HeroUICut cut) {
	return cut == HERO_UI_CUT_LEFT || cut == HERO_UI_CUT_RIGHT || cut == HERO_UI_CUT_CENTER_HORIZONTAL;
}

static inline bool hero_ui_cut_is_center(HeroUICut cut) {
	return cut == HERO_UI_CUT_CENTER_VERTICAL || cut == HERO_UI_CUT_CENTER_HORIZONTAL;
}

typedef U32 HeroUIFontId;

typedef struct HeroUIWidgetStyle HeroUIWidgetStyle;
struct HeroUIWidgetStyle {
	HeroUIThickness    margin;
	HeroUIThickness    padding;
	HeroColor          background_color;
	F32                border_width;
	HeroColor          border_color;
	F32                radius;
	HeroColor          text_color;
	HeroUIFontId       text_font_id;
	F32                text_line_height;
	bool               text_wrap;
};

static HeroUIWidgetStyle HERO_UI_WIDGET_STYLE_NULL = {0};

typedef U8 HeroUIImageScaleMode;
enum {
	HERO_UI_IMAGE_SCALE_MODE_STRETCH,      // image is scaled to fit but will not maintain the aspect ratio
	HERO_UI_IMAGE_SCALE_MODE_UNIFORM,      // image is scaled to fit and will maintain the aspect ratio
	HERO_UI_IMAGE_SCALE_MODE_UNIFORM_CROP, // image is scaled to fit on a single side and crop the other, this will maintain the aspect ratio
	HERO_UI_IMAGE_SCALE_MODE_NONE,         // image is not scaled, the original size is used.
};

typedef U8 HeroUIImageFlip;
enum {
	HERO_UI_IMAGE_FLIP_NONE = 0,
	HERO_UI_IMAGE_FLIP_X =    0x1,
	HERO_UI_IMAGE_FLIP_Y =    0x2,
};

typedef U8 HeroUIFocusState;
enum {
	HERO_UI_FOCUS_STATE_NONE = 0x0,
	// signals that the ctrl is mouse or keyboard focused
	HERO_UI_FOCUS_STATE_FOCUSED = 0x1,
	// signals has been pressed this frame
	HERO_UI_FOCUS_STATE_PRESSED = 0x2,
	// signals has been double pressed this frame
	HERO_UI_FOCUS_STATE_DOUBLE_PRESSED = 0x4,
	// signals is being held this frame
	HERO_UI_FOCUS_STATE_HELD = 0x8,
	// signals has been released this frame
	HERO_UI_FOCUS_STATE_RELEASED = 0x10,
};

typedef U8 HeroUIAlign;
enum {
	HERO_UI_ALIGN_X_CENTER = 0x0,
	HERO_UI_ALIGN_X_LEFT =   0x1,
	HERO_UI_ALIGN_X_RIGHT =  0x3,

	HERO_UI_ALIGN_Y_CENTER = 0x0,
	HERO_UI_ALIGN_Y_TOP =    0x4,
	HERO_UI_ALIGN_Y_BOTTOM = 0xC,
};

// ===========================================
//
//
// Image Altas
//
//
// ===========================================

typedef struct HeroUIImageAtlas HeroUIImageAtlas;
struct HeroUIImageAtlas {
	HeroObjectHeader header;
	HeroImageId image_id;
	HeroSamplerId sampler_id;
	F32 width;
	F32 height;
	F32 width_inv;
	F32 height_inv;
	U32 images_count: 31;
	U32 is_uniform: 1;
	union {
		HeroAabb* image_rects;
		struct {
			F32 cell_width;
			F32 cell_height;
			U32 cells_count_x;
		} uniform;
	} data;
};

HERO_TYPEDEF_OBJECT_ID(HeroUIImageAtlasId);
#define HERO_OBJECT_ID_TYPE HeroUIImageAtlasId
#define HERO_OBJECT_TYPE HeroUIImageAtlas
#include "object_pool_gen.inl"

typedef struct HeroUIImageId HeroUIImageId;
struct HeroUIImageId {
	HeroUIImageAtlasId atlas_id;
	U32 image_idx;
};

#define HERO_STACK_ELMT_TYPE HeroUIImageAtlasId
#include "stack_gen.inl"

#define HERO_UI_IMAGE_ID(atlas_id, image_idx) ((HeroUIImageId){ (atlas_id), (image_idx) })

typedef struct HeroUIImageAtlasSetup HeroUIImageAtlasSetup;
struct HeroUIImageAtlasSetup {
	HeroImageId image_id;
	HeroSamplerId sampler_id;
	U32 images_count: 31;
	U32 is_uniform: 1;
	union {
		HeroAabb* image_rects;
		struct {
			F32 cell_width;
			F32 cell_height;
			U16 cells_count_x;
			U16 cells_count_y;
		} uniform;
	} data;
};

HeroResult hero_ui_image_atlas_init(HeroLogicalDevice* ldev, HeroUIImageAtlasSetup* setup, HeroUIImageAtlasId* id_out);
HeroResult hero_ui_image_atlas_deinit(HeroUIImageAtlasId id);
HeroResult hero_ui_image_atlas_get(HeroUIImageAtlasId id, HeroUIImageAtlas** out);
HeroResult hero_ui_image_atlas_image_size(HeroUIImageAtlasId id, U32 image_idx, Vec2* size_out);
HeroResult hero_ui_image_atlas_image_uv_aabb(HeroUIImageAtlasId id, U32 image_idx, HeroAabb* uv_aabb_out);

// ===========================================
//
//
// Widget
//
//
// ===========================================

typedef U32 HeroUIWidgetSibId;

typedef U32 HeroUIWidgetFlags;
enum {
	HERO_UI_WIDGET_FLAGS_CREATED_THIS_FRAME =             0x1,
	HERO_UI_WIDGET_FLAGS_IS_FOCUSABLE =                   0x2,
	HERO_UI_WIDGET_FLAGS_IS_PRESSABLE =                   0x4,
	HERO_UI_WIDGET_FLAGS_IS_SELECTABLE =                  0x8,
	HERO_UI_WIDGET_FLAGS_IS_TOGGLEABLE =                 0x10,
	HERO_UI_WIDGET_FLAGS_CHILD_HAS_CUT_VERTICAL =        0x20,
	HERO_UI_WIDGET_FLAGS_CHILD_HAS_CUT_HORIZONTAL =      0x40,
	HERO_UI_WIDGET_FLAGS_CHILD_HAS_CUT_CENTER =          0x80,
	HERO_UI_WIDGET_FLAGS_NEXT_LAYER =                   0x100,
	HERO_UI_WIDGET_FLAGS_CUSTOM_OFFSET =                0x200,
	HERO_UI_WIDGET_FLAGS_FORCE_NEXT_STATE =             0x400,
};

typedef U8 HeroUIWidgetState;
enum {
	HERO_UI_WIDGET_STATE_DEFAULT,
	HERO_UI_WIDGET_STATE_FOCUSED,
	HERO_UI_WIDGET_STATE_ACTIVE,
	HERO_UI_WIDGET_STATE_DISABLED,
	HERO_UI_WIDGET_STATE_COUNT,
};

HERO_TYPEDEF_OBJECT_ID(HeroUIWidgetId);
#define HERO_UI_WIDGET_ID_NULL ((HeroUIWidgetId){0})
#define HERO_UI_ROOT_WIDGET_ID ((HeroUIWidgetId){ .raw = 1 << HERO_OBJECT_ID_COUNTER_SHIFT(20) })

typedef HeroResult (*HeroUIWidgetRenderFn)(HeroUIWindow* window, HeroUIWidget* widget);

struct HeroUIWidget {
	HeroObjectHeader     header;
	HeroUIWidgetFlags    flags;
	HeroUIWidgetState    state;
	HeroAabb             area;
	HeroUIWidgetSibId    sib_id; // unique sibling identifier
	HeroUIWidgetId       parent_id;
	HeroUIWidgetId       child_first_id;
	HeroUIWidgetId       child_last_id;
	HeroUIWidgetId       sibling_prev_id;
	HeroUIWidgetId       sibling_next_id;
	HeroUICut            cut;
	F32                  cut_length;
	F32                  auto_perp_cut_length;
	F32                  auto_children_length;
	F32                  fill_proportion_fixed_children_length;
	U32                  fill_proportion_fill_children_count;
	HeroString           string;
	HeroUIWidgetStyle*   styles; // index with HeroUIWidget.state
	HeroGfxFrameIdx      last_updated_frame_idx;
	void*                userdata;
	HeroUIWidgetRenderFn render_fn;
	HeroUIImageId        image_id;
	HeroColor            image_bg_color;
	HeroColor            image_fg_color;
	HeroUIImageScaleMode image_scale_mode;
	HeroUIImageFlip      image_flip;
	Vec2                 image_size;
	bool                 image_grayscale;
	HeroUIFocusState     focus_state;
	Vec2                 custom_offset;
	F32                  custom_perp_cut_length;
	HeroUIAlign          custom_align;
};

HeroResult hero_ui_widget_get(HeroUIWindow* window, HeroUIWidgetId id, HeroUIWidget** ptr_out);
HeroUIWidgetId hero_ui_widget_prev_id(HeroUIWindow* window);
HeroUIWidgetId hero_ui_widget_mouse_focused_get(HeroUIWindow* window);
HeroResult hero_ui_widget_mouse_focused_set(HeroUIWindow* window, HeroUIWidgetId id);
HeroUIWidgetId hero_ui_widget_keyboard_focused_get(HeroUIWindow* window);
HeroResult hero_ui_widget_keyboard_focused_set(HeroUIWindow* window, HeroUIWidgetId id);
HeroUIWidgetStyle* hero_ui_widget_style(HeroUIWidget* widget);
HeroUIWidgetFlags hero_ui_widget_flags(HeroUIWindow* window, HeroUIWidgetId id);

// ===========================================
//
//
// Draw
//
//
// ===========================================

typedef struct HeroUIDrawCmd HeroUIDrawCmd;
struct HeroUIDrawCmd {
	HeroUIDrawCmdType type;
	union {
		struct {
			HeroAabb aabb;
			F32 radius;
			HeroColor color;
			F32 border_width;
		} aabb;
		struct {
			HeroAabb  aabb;
			HeroUIImageId image_id;
			HeroColor bg_color;
			HeroColor fg_color;
			U16       image_group_idx;
			U16       image_idx;
		} image;
		struct {
			Vec2 center_pos;
			F32 radius;
			HeroColor color;
			F32 border_width;
		} circle;
	} data;
};

#define HERO_STACK_ELMT_TYPE HeroUIDrawCmd
#include "stack_gen.inl"

HeroResult hero_ui_widget_draw_aabb(HeroUIWindow* window, HeroAabb* aabb, F32 radius, HeroColor color);
HeroResult hero_ui_widget_draw_aabb_border(HeroUIWindow* window, HeroAabb* aabb, F32 radius, HeroColor color, F32 border_width);
HeroResult hero_ui_widget_draw_image(HeroUIWindow* window, HeroAabb* aabb, HeroUIImageId image_id, HeroColor color);
HeroResult hero_ui_widget_draw_image_grayscale_remap(HeroUIWindow* window, HeroAabb* aabb, HeroUIImageId image_id, HeroColor bg_color, HeroColor fg_color);
HeroResult hero_ui_widget_draw_circle(HeroUIWindow* window, Vec2 center_pos, F32 radius, HeroColor color);
HeroResult hero_ui_widget_draw_circle_border(HeroUIWindow* window, Vec2 center_pos, F32 radius, HeroColor color, F32 border_width);

// ===========================================
//
//
// Window
//
//
// ===========================================

#define HERO_OBJECT_ID_TYPE HeroUIWidgetId
#define HERO_OBJECT_TYPE HeroUIWidget
#include "object_pool_gen.inl"

typedef struct HeroUIWindowBuild HeroUIWindowBuild;
struct HeroUIWindowBuild {
	HeroUIWidget*     parent;
	HeroUIWidget*     sibling_prev;
	HeroUIWidgetId    parent_id;
	HeroUIWidgetId    sibling_prev_id;
	HeroGfxFrameIdx   last_updated_frame_idx;
	Vec2              custom_offset;
	HeroUIAlign       custom_align;
	F32               custom_perp_cut_length;
	HeroUIWidgetState next_forced_state;

	HeroUIWidgetFlags next_widget_flags;
};

HERO_TYPEDEF_OBJECT_ID(HeroUIWindowId);

typedef struct HeroUIWindowRenderData HeroUIWindowRenderData;
struct HeroUIWindowRenderData {
	HeroStack(HeroUIDrawCmd)      draw_cmds;
	HeroStack(HeroUIImageAtlasId) unique_image_atlas_ids;
	U32                           aabbs_count;
	U32                           circles_count;
	U32                           materials_count;
	U32                           window_width;
	U32                           window_height;
};

typedef struct HeroUIWindowUpdate HeroUIWindowUpdate;
struct HeroUIWindowUpdate {
	HeroUIWindowRenderData        render_data;
	U32                           image_group_idx;
};

typedef struct HeroUIWindowRender HeroUIWindowRender;
struct HeroUIWindowRender {
	HeroUIWindowRenderData        render_data;
	U32                           image_group_idx;
	HeroBufferId                  vertex_buffer_id;
	HeroBufferId                  index_buffer_id;
	HeroBufferId                  global_uniform_buffer_id;
	HeroShaderGlobalsId           shader_globals_id;
	HeroStack(HeroBufferId)       color_uniform_buffer_ids;
	HeroStack(HeroBufferId)       aabb_uniform_buffer_ids;
	HeroStack(HeroBufferId)       circle_uniform_buffer_ids;
	HeroStack(HeroMaterialId)     material_ids;
	U32                           color_uniform_buffer_idx;
	U32                           aabb_uniform_buffer_idx;
	U32                           circle_uniform_buffer_idx;
};

struct HeroUIWindow {
	HeroObjectHeader             header;
	HeroObjectPool(HeroUIWidget) widget_pool;
	HeroUIWindowBuild            build;
	HeroUIWidgetId               mouse_focused_id;
	HeroUIWidgetId               keyboard_focused_id;
	HeroUIWindowUpdate           update;
	HeroUIWindowRender           render;
	HeroAabb                     clip_rect;
	HeroWindowId                 window_id;
};

typedef struct HeroUIWindowSetup HeroUIWindowSetup;
struct HeroUIWindowSetup {
	U32                widgets_cap;
	HeroLogicalDevice* ldev;
	HeroWindowId       window_id;
};

HeroResult hero_ui_window_init(HeroUIWindowSetup* setup, HeroUIWindowId* id_out);
HeroResult hero_ui_window_deinit(HeroUIWindowId id);
HeroResult hero_ui_window_get(HeroUIWindowId id, HeroUIWindow** out);

HeroUIWindow* hero_ui_window_start(HeroUIWindowId id, U32 render_width, U32 render_height);
void hero_ui_window_end(HeroUIWindow* window);
HeroResult hero_ui_window_update(HeroUIWindowId id);
HeroResult hero_ui_window_update_render_data(HeroUIWindowId id);
HeroResult hero_ui_window_render(HeroUIWindowId id, HeroLogicalDevice* ldev, HeroCommandRecorder* command_recorder);

// ===========================================
//
//
// UI System
//
//
// ===========================================

#define HERO_OBJECT_ID_TYPE HeroUIWindowId
#define HERO_OBJECT_TYPE HeroUIWindow
#include "object_pool_gen.inl"

typedef Vec2 (*HeroUITextSizeFn)(HeroUIWindow* window, HeroString string, F32 wrap_at_width, HeroUIWidgetStyle* style);
typedef HeroResult (*HeroUITextRenderFn)(HeroUIWindow* window, Vec2 top_left, HeroString string, F32 wrap_at_width, HeroUIWidgetStyle* style);

typedef struct HeroUISys HeroUISys;
struct HeroUISys {
	HeroObjectPool(HeroUIWindow) window_pool;
	HeroObjectPool(HeroUIImageAtlas) image_atlas_pool;
	HeroShaderId shader_id;
	HeroDescriptorPoolId descriptor_pool_id;
	HeroVertexLayoutId vertex_layout_id;
	HeroPipelineId pipeline_id;
	HeroUITextSizeFn text_size_fn;
	HeroUITextRenderFn text_render_fn;
	bool is_mouse_over_widget;
};

extern HeroUISys hero_ui_sys;

typedef struct HeroUISysSetup HeroUISysSetup;
struct HeroUISysSetup {
	HeroLogicalDevice* ldev;
	U32 windows_cap;
	U32 image_atlases_cap;
	HeroShaderId shader_id;
	HeroDescriptorPoolId descriptor_pool_id;
	HeroRenderPassLayoutId render_pass_layout_id;
	HeroUITextSizeFn text_size_fn;
	HeroUITextRenderFn text_render_fn;
};

HeroResult hero_ui_sys_init(HeroUISysSetup* setup);

// ===========================================
//
//
// Utility Functions
//
//
// ===========================================

void hero_ui_area_outer_to_inner(HeroAabb* area_mut, HeroUIWidgetStyle* style);
HeroUIFocusState hero_ui_focus_state(HeroUIWindow* window, HeroUIWidgetId widget_id);
HeroUIWidgetState hero_ui_widget_state(HeroUIWindow* window, HeroUIWidgetId widget_id);

// ===========================================
//
//
// Widgets API
//
//
// ===========================================

void hero_ui_widget_next_layer_start(HeroUIWindow* window, HeroUIWidgetSibId sib_id);
void hero_ui_widget_next_layer_end(HeroUIWindow* window);
void hero_ui_widget_custom_offset(HeroUIWindow* window, Vec2 offset, F32 perp_cut_length, HeroUIAlign align);
void hero_ui_widget_next_forced_state(HeroUIWindow* window, HeroUIWidgetState state);

HeroUIWidgetId hero_ui_box_start(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, F32 cut_length, HeroUIWidgetStyle* styles);
void hero_ui_box_end(HeroUIWindow* window);
HeroUIWidgetId hero_ui_box(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, F32 cut_length, HeroUIWidgetStyle* styles);
HeroResult hero_ui_box_render(HeroUIWindow* window, HeroUIWidget* widget);

HeroUIWidgetId hero_ui_image(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, HeroUIImageId image_id, HeroColor image_tint, F32 scale, HeroUIImageScaleMode scale_mode, HeroUIImageFlip flip, HeroUIWidgetStyle* styles);
HeroUIWidgetId hero_ui_image_grayscale(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, HeroUIImageId image_id, HeroColor bg_color, HeroColor fg_color, F32 scale, HeroUIImageScaleMode scale_mode, HeroUIImageFlip flip, HeroUIWidgetStyle* styles);
HeroResult hero_ui_image_render(HeroUIWindow* window, HeroUIWidget* widget);

HeroUIWidgetId hero_ui_text(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, F32 cut_length, HeroString string, HeroUIWidgetStyle* styles);
HeroResult hero_ui_text_render(HeroUIWindow* window, HeroUIWidget* widget);

HeroUIWidgetId hero_ui_button_start(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, F32 cut_length, HeroUIWidgetStyle* styles);
void hero_ui_button_end(HeroUIWindow* window);

HeroUIWidgetId hero_ui_text_button(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, F32 cut_length, HeroString string, HeroUIWidgetStyle* styles);
HeroUIWidgetId hero_ui_text_toggle_button(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, F32 cut_length, HeroString string, HeroUIWidgetStyle* styles);
HeroUIWidgetId hero_ui_image_button(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, F32 cut_length, HeroUIImageId image_id, HeroColor image_tint, F32 scale, HeroUIImageScaleMode scale_mode, HeroUIImageFlip flip, HeroUIWidgetStyle* styles);
HeroUIWidgetId hero_ui_image_grayscale_button(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, F32 cut_length, HeroUIImageId image_id, HeroColor bg_color, HeroColor fg_color, F32 scale, HeroUIImageScaleMode scale_mode, HeroUIImageFlip flip, HeroUIWidgetStyle* styles);

// ===========================================
//
//
// Built-in Style Sheet
//
//
// ===========================================

typedef struct HeroUIStyleSheet HeroUIStyleSheet;
struct HeroUIStyleSheet {
	HeroUIWidgetStyle root[HERO_UI_WIDGET_STATE_COUNT];
	HeroUIWidgetStyle box[HERO_UI_WIDGET_STATE_COUNT];
	HeroUIWidgetStyle text[HERO_UI_WIDGET_STATE_COUNT];
	HeroUIWidgetStyle button[HERO_UI_WIDGET_STATE_COUNT];
};

static HeroUIStyleSheet hero_ui_ss = {
	.root = {
		[HERO_UI_WIDGET_STATE_DEFAULT] = {
			.margin = HERO_THICKNESS_INIT_EVEN(0),
			.padding = hero_ui_padding_default,
			.background_color = hero_color_init(0x00, 0x80, 0x80, 0xff),
			.border_width = hero_ui_border_width_default,
			.border_color = hero_color_init(0xff, 0x66, 0xcc, 0xff),
			.radius = hero_ui_radius_default,
			.text_color = 0,
			.text_font_id = 0,
			.text_line_height = 0,
			.text_wrap = false,
		},
		[HERO_UI_WIDGET_STATE_FOCUSED] = {
			.margin = HERO_THICKNESS_INIT_EVEN(0),
			.padding = hero_ui_padding_default,
			.background_color = hero_color_init(0x00, 0x80, 0x80, 0xff),
			.border_width = hero_ui_border_width_default,
			.border_color = hero_color_init(0xff, 0x66, 0xcc, 0xff),
			.radius = hero_ui_radius_default,
			.text_color = 0,
			.text_font_id = 0,
			.text_line_height = 0,
			.text_wrap = false,
		},
		[HERO_UI_WIDGET_STATE_ACTIVE] = {
			.margin = HERO_THICKNESS_INIT_EVEN(0),
			.padding = hero_ui_padding_default,
			.background_color = hero_color_init(0x00, 0x80, 0x80, 0xff),
			.border_width = hero_ui_border_width_default,
			.border_color = hero_color_init(0xff, 0x66, 0xcc, 0xff),
			.radius = hero_ui_radius_default,
			.text_color = 0,
			.text_font_id = 0,
			.text_line_height = 0,
			.text_wrap = false,
		},
		[HERO_UI_WIDGET_STATE_DISABLED] = {
			.margin = HERO_THICKNESS_INIT_EVEN(0),
			.padding = hero_ui_padding_default,
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
	.box = {
		[HERO_UI_WIDGET_STATE_DEFAULT] = {
			.margin = hero_ui_margin_default,
			.padding = hero_ui_padding_default,
			.background_color = hero_color_init(0x00, 0x80, 0x80, 0xff),
			.border_width = hero_ui_border_width_default,
			.border_color = hero_color_init(0xff, 0x66, 0xcc, 0xff),
			.radius = hero_ui_radius_default,
			.text_color = 0,
			.text_font_id = 0,
			.text_line_height = 0,
			.text_wrap = false,
		},
		[HERO_UI_WIDGET_STATE_FOCUSED] = {
			.margin = hero_ui_margin_default,
			.padding = hero_ui_padding_default,
			.background_color = hero_color_init(0x00, 0x80, 0x80, 0xff),
			.border_width = hero_ui_border_width_default,
			.border_color = hero_color_init(0xff, 0x66, 0xcc, 0xff),
			.radius = hero_ui_radius_default,
			.text_color = 0,
			.text_font_id = 0,
			.text_line_height = 0,
			.text_wrap = false,
		},
		[HERO_UI_WIDGET_STATE_ACTIVE] = {
			.margin = hero_ui_margin_default,
			.padding = hero_ui_padding_default,
			.background_color = hero_color_init(0x00, 0x80, 0x80, 0xff),
			.border_width = hero_ui_border_width_default,
			.border_color = hero_color_init(0xff, 0x66, 0xcc, 0xff),
			.radius = hero_ui_radius_default,
			.text_color = 0,
			.text_font_id = 0,
			.text_line_height = 0,
			.text_wrap = false,
		},
		[HERO_UI_WIDGET_STATE_DISABLED] = {
			.margin = hero_ui_margin_default,
			.padding = hero_ui_padding_default,
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
	.text = {
		[HERO_UI_WIDGET_STATE_DEFAULT] = {
			.margin = hero_ui_margin_default,
			.padding = hero_ui_padding_default,
			.background_color = 0,
			.border_width = 0.f,
			.border_color = 0,
			.radius = 0.f,
			.text_color = hero_color_init(0xff, 0xff, 0xff, 0xff),
			.text_font_id = 0,
			.text_line_height = 28,
			.text_wrap = false,
		},
		[HERO_UI_WIDGET_STATE_FOCUSED] = {
			.margin = hero_ui_margin_default,
			.padding = hero_ui_padding_default,
			.background_color = 0,
			.border_width = 0.f,
			.border_color = 0,
			.radius = 0.f,
			.text_color = hero_color_init(0xff, 0xff, 0xff, 0xff),
			.text_font_id = 0,
			.text_line_height = 28,
		},
		[HERO_UI_WIDGET_STATE_ACTIVE] = {
			.margin = hero_ui_margin_default,
			.padding = hero_ui_padding_default,
			.background_color = 0,
			.border_width = 0.f,
			.border_color = 0,
			.radius = 0.f,
			.text_color = hero_color_init(0xff, 0xff, 0xff, 0xff),
			.text_font_id = 0,
			.text_line_height = 28,
		},
		[HERO_UI_WIDGET_STATE_DISABLED] = {
			.margin = hero_ui_margin_default,
			.padding = hero_ui_padding_default,
			.background_color = 0,
			.border_width = 0.f,
			.border_color = 0,
			.radius = 0.f,
			.text_color = hero_color_init(0xff, 0xff, 0xff, 0xff),
			.text_font_id = 0,
			.text_line_height = 28,
		},
	},
	.button = {
		[HERO_UI_WIDGET_STATE_DEFAULT] = {
			.margin = hero_ui_margin_default,
			.padding = hero_ui_padding_default,
			.background_color = hero_color_init(0x00, 0x80, 0x80, 0xff),
			.border_width = hero_ui_border_width_default,
			.border_color = hero_color_init(0xff, 0x66, 0xcc, 0xff),
			.radius = hero_ui_radius_default,
			.text_color = hero_color_init(0xff, 0xff, 0xff, 0xff),
			.text_font_id = 0,
			.text_line_height = 28,
			.text_wrap = false,
		},
		[HERO_UI_WIDGET_STATE_FOCUSED] = {
			.margin = hero_ui_margin_default,
			.padding = hero_ui_padding_default,
			.background_color = hero_color_init(0x00, 0xa0, 0xa0, 0xff),
			.border_width = hero_ui_border_width_default,
			.border_color = hero_color_init(0xff, 0x88, 0xdd, 0xff),
			.radius = hero_ui_radius_default,
			.text_color = hero_color_init(0xff, 0xff, 0xff, 0xff),
			.text_font_id = 0,
			.text_line_height = 28,
			.text_wrap = false,
		},
		[HERO_UI_WIDGET_STATE_ACTIVE] = {
			.margin = hero_ui_margin_default,
			.padding = hero_ui_padding_default,
			.background_color = hero_color_init(0xff, 0x66, 0xcc, 0xff),
			.border_width = hero_ui_border_width_default,
			.border_color = hero_color_init(0x00, 0x80, 0x80, 0xff),
			.radius = hero_ui_radius_default,
			.text_color = hero_color_init(0xff, 0xff, 0xff, 0xff),
			.text_font_id = 0,
			.text_line_height = 28,
			.text_wrap = false,
		},
		[HERO_UI_WIDGET_STATE_DISABLED] = {
			.margin = hero_ui_margin_default,
			.padding = hero_ui_padding_default,
			.background_color = hero_color_init(0x00, 0x80, 0x80, 0xff),
			.border_width = hero_ui_border_width_default,
			.border_color = hero_color_init(0xff, 0x66, 0xcc, 0xff),
			.radius = hero_ui_radius_default,
			.text_color = hero_color_init(0xff, 0xff, 0xff, 0xff),
			.text_font_id = 0,
			.text_line_height = 28,
			.text_wrap = false,
		},
	},
};

#endif // _HERO_UI_H_

