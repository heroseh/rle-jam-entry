#include "ui.h"

// ===========================================
//
//
// Image Altas
//
//
// ===========================================

HeroResult hero_ui_image_atlas_init(HeroLogicalDevice* ldev, HeroUIImageAtlasSetup* setup, HeroUIImageAtlasId* id_out) {
	HeroUIImageAtlas* atlas;
	HeroResult result = hero_object_pool(HeroUIImageAtlas, alloc)(&hero_ui_sys.image_atlas_pool, &atlas, id_out);
	if (result < 0) {
		return result;
	}

	HeroImage* image;
	result = hero_image_get(ldev, setup->image_id, &image);
	if (result < 0) {
		return result;
	}

	atlas->image_id = setup->image_id;
	atlas->sampler_id = setup->sampler_id;
	atlas->width = image->width;
	atlas->height = image->height;
	atlas->width_inv = 1.f / image->width;
	atlas->height_inv = 1.f / image->height;
	atlas->images_count = setup->images_count;
	atlas->is_uniform = setup->is_uniform;
	if (setup->is_uniform) {
		atlas->data.uniform.cell_width = setup->data.uniform.cell_width;
		atlas->data.uniform.cell_height = setup->data.uniform.cell_height;
		atlas->data.uniform.cells_count_x = setup->data.uniform.cells_count_x;
	} else {
		atlas->data.image_rects = setup->data.image_rects;
	}

	return HERO_SUCCESS;
}

HeroResult hero_ui_image_atlas_deinit(HeroUIImageAtlasId id) {
	return hero_object_pool(HeroUIImageAtlas, dealloc)(&hero_ui_sys.image_atlas_pool, id);
}

HeroResult hero_ui_image_atlas_get(HeroUIImageAtlasId id, HeroUIImageAtlas** out) {
	return hero_object_pool(HeroUIImageAtlas, get)(&hero_ui_sys.image_atlas_pool, id, out);
}

HeroResult hero_ui_image_atlas_image_size(HeroUIImageAtlasId id, U32 image_idx, Vec2* size_out) {
	HeroUIImageAtlas* atlas;
	HeroResult result = hero_ui_image_atlas_get(id, &atlas);
	if (result < 0) {
		return result;
	}

	if (image_idx >= atlas->images_count) {
		return HERO_ERROR(DOES_NOT_EXIST);
	}

	if (atlas->is_uniform) {
		size_out->x = atlas->data.uniform.cell_width;
		size_out->y = atlas->data.uniform.cell_height;
	} else {
		HeroAabb* aabb = &atlas->data.image_rects[image_idx];
		size_out->x = hero_aabb_width(aabb);
		size_out->y = hero_aabb_height(aabb);
	}

	return HERO_SUCCESS;
}

HeroResult hero_ui_image_atlas_image_uv_aabb(HeroUIImageAtlasId id, U32 image_idx, HeroAabb* uv_aabb_out) {
	HeroUIImageAtlas* atlas;
	HeroResult result = hero_ui_image_atlas_get(id, &atlas);
	if (result < 0) {
		return result;
	}

	if (image_idx >= atlas->images_count) {
		return HERO_ERROR(DOES_NOT_EXIST);
	}

	if (atlas->is_uniform) {
		F32 x = (F32)(image_idx % atlas->data.uniform.cells_count_x);
		F32 y = (F32)(image_idx / atlas->data.uniform.cells_count_x);
		uv_aabb_out->x = x * atlas->data.uniform.cell_width;
		uv_aabb_out->y = y * atlas->data.uniform.cell_height;
		uv_aabb_out->ex = uv_aabb_out->x + atlas->data.uniform.cell_width;
		uv_aabb_out->ey = uv_aabb_out->y + atlas->data.uniform.cell_height;
	} else {
		*uv_aabb_out = atlas->data.image_rects[image_idx];
	}

	uv_aabb_out->x *= atlas->width_inv;
	uv_aabb_out->y *= atlas->height_inv;
	uv_aabb_out->ex *= atlas->width_inv;
	uv_aabb_out->ey *= atlas->height_inv;

	return HERO_SUCCESS;
}


// ===========================================
//
//
// Widget
//
//
// ===========================================

HeroResult hero_ui_widget_get(HeroUIWindow* window, HeroUIWidgetId id, HeroUIWidget** ptr_out) {
	return hero_object_pool(HeroUIWidget, get)(&window->widget_pool, id, ptr_out);
}

HeroUIWidgetId hero_ui_widget_prev_id(HeroUIWindow* window) {
	return window->build.sibling_prev_id;
}

HeroResult _hero_ui_widget_focus(HeroUIWindow* window, HeroUIWidgetId id) {
	HeroUIWidget* widget;
	HeroResult result = hero_ui_widget_get(window, id, &widget);
	if (result < 0) {
		return result;
	}

	if (!(widget->flags & HERO_UI_WIDGET_FLAGS_IS_FOCUSABLE)) {
		return HERO_ERROR(UI_WIDGET_IS_NOT_FOCUSABLE);
	}
	if (widget->state < HERO_UI_WIDGET_STATE_FOCUSED) {
		widget->state = HERO_UI_WIDGET_STATE_FOCUSED;
	}

	return HERO_SUCCESS;
}

void _hero_ui_widget_unfocus(HeroUIWindow* window, HeroUIWidgetId id) {
	HeroUIWidget* widget;
	HeroResult result = hero_ui_widget_get(window, id, &widget);
	if (result >= 0) {
		if (widget->state == HERO_UI_WIDGET_STATE_FOCUSED) {
			widget->state = HERO_UI_WIDGET_STATE_DEFAULT;
		}
	}
}

HeroUIWidgetId hero_ui_widget_mouse_focused_get(HeroUIWindow* window) {
	return window->mouse_focused_id;
}

HeroResult hero_ui_widget_mouse_focused_set(HeroUIWindow* window, HeroUIWidgetId id) {
	if (window->mouse_focused_id.raw == id.raw) {
		return HERO_SUCCESS;
	}

	if (window->mouse_focused_id.raw && window->mouse_focused_id.raw != window->keyboard_focused_id.raw) {
		_hero_ui_widget_unfocus(window, window->mouse_focused_id);
	}

	if (id.raw) {
		HeroResult result = _hero_ui_widget_focus(window, id);
		if (result < 0) {
			return result;
		}
	}

	window->mouse_focused_id = id;
	return HERO_SUCCESS;
}

HeroUIWidgetId hero_ui_widget_keyboard_focused_get(HeroUIWindow* window) {
	return window->keyboard_focused_id;
}

HeroResult hero_ui_widget_keyboard_focused_set(HeroUIWindow* window, HeroUIWidgetId id) {
	if (window->keyboard_focused_id.raw == id.raw) {
		return HERO_SUCCESS;
	}

	if (window->keyboard_focused_id.raw && window->keyboard_focused_id.raw != window->mouse_focused_id.raw) {
		_hero_ui_widget_unfocus(window, window->keyboard_focused_id);
	}

	if (id.raw) {
		HeroResult result = _hero_ui_widget_focus(window, id);
		if (result < 0) {
			return result;
		}
	}
	window->keyboard_focused_id = id;
	return HERO_SUCCESS;
}

HeroUIWidgetStyle* hero_ui_widget_style(HeroUIWidget* widget) {
	return widget->styles ? &widget->styles[widget->state] : &HERO_UI_WIDGET_STYLE_NULL;
}

HeroUIWidgetFlags hero_ui_widget_flags(HeroUIWindow* window, HeroUIWidgetId id) {
	HeroUIWidget* widget;
	HeroResult result = hero_ui_widget_get(window, id, &widget);
	if (result < 0) {
		return 0;
	}
	return widget->flags;
}

HeroResult _hero_ui_widget_alloc(HeroUIWindow* window, HeroUIWidget** ptr_out, HeroUIWidgetId* id_out) {
	return hero_object_pool(HeroUIWidget, alloc)(&window->widget_pool, ptr_out, id_out);
}

HeroResult _hero_ui_widget_dealloc(HeroUIWindow* window, HeroUIWidget* widget, HeroUIWidgetId widget_id) {
	HeroUIWidget* child = NULL;
	HeroUIWidgetId sibling_next_id = HERO_UI_WIDGET_ID_NULL;
	for (HeroUIWidgetId child_id = widget->child_first_id; child_id.raw; child_id = sibling_next_id) {
		HeroResult result = hero_ui_widget_get(window, child_id, &child);
		HERO_RESULT_ASSERT(result);
		sibling_next_id = child->sibling_next_id;

		result = _hero_ui_widget_dealloc(window, child, child_id);
		if (result < 0) {
			return result;
		}
	}

	return hero_object_pool(HeroUIWidget, dealloc)(&window->widget_pool, widget_id);
}

void _hero_ui_widget_frame_reset(HeroUIWidget* widget) {
	HeroUIWidgetFlags flags_to_remove =
		HERO_UI_WIDGET_FLAGS_CHILD_HAS_CUT_VERTICAL   |
		HERO_UI_WIDGET_FLAGS_CHILD_HAS_CUT_HORIZONTAL |
		HERO_UI_WIDGET_FLAGS_CHILD_HAS_CUT_CENTER     ;
	widget->flags &= ~flags_to_remove;
	widget->fill_proportion_fixed_children_length = 0.f;
	widget->fill_proportion_fill_children_count = 0;
	widget->auto_children_length = 0.f;
}

void _hero_ui_widget_tree_insert(HeroUIWindow* window, HeroUIWidget* widget, HeroUIWidgetId widget_id) {
	HeroUIWindowBuild* build = &window->build;

	widget->parent_id = build->parent_id;
	if (build->sibling_prev) {
		//
		// in the current state of the build we have previous sibling.
		// so insert the widget after that previous sibling.
		//
		widget->sibling_prev_id = build->sibling_prev_id;
		widget->sibling_next_id = build->sibling_prev->sibling_next_id;

		build->sibling_prev->sibling_next_id = widget_id;
		if (widget->sibling_next_id.raw) {
			//
			// we have a next sibling so get that to point back to us.
			HeroUIWidget* sibling_next;
			HeroResult result = hero_ui_widget_get(window, widget->sibling_next_id, &sibling_next);
			HERO_RESULT_ASSERT(result);

			sibling_next->sibling_prev_id = widget_id;
		} else {
			//
			// no next sibling so we are the last child of the parent.
			build->parent->child_last_id = widget_id;
		}
	} else {
		//
		// in the current state of the build we have _no_ previous sibling
		// so insert the widget as the first child of the parent.
		// bearing in mind that the parent can still have children from the
		// last frame.
		//

		if (build->parent->child_first_id.raw) {
			//
			// we already have a first child, so get that to point back to us.
			HeroUIWidget* first_child;
			HeroResult result = hero_ui_widget_get(window, build->parent->child_first_id, &first_child);
			HERO_RESULT_ASSERT(result);

			first_child->sibling_prev_id = widget_id;
			widget->sibling_next_id = build->parent->child_first_id;
		} else {
			//
			// the parent has no children so we must be the last child too.
			build->parent->child_last_id = widget_id;
		}

		build->parent->child_first_id = widget_id;
	}
}

void _hero_ui_widget_tree_remove(HeroUIWindow* window, HeroUIWidget* widget, HeroUIWidgetId widget_id) {
	//
	// if the previous sibling exists then make it point to our next sibling,
	// if not we are the first child of parent, and make our next sibling the new first child.
	if (widget->sibling_prev_id.raw) {
		HeroUIWidget* sibling_prev;
		HeroResult result = hero_ui_widget_get(window, widget->sibling_prev_id, &sibling_prev);
		HERO_RESULT_ASSERT(result);

		sibling_prev->sibling_next_id = widget->sibling_next_id;
	} else {
		HeroUIWidget* parent;
		HeroResult result = hero_ui_widget_get(window, widget->parent_id, &parent);
		HERO_RESULT_ASSERT(result);

		parent->child_first_id = widget->sibling_next_id;
	}

	//
	// if the next sibling exists then make it point to our previous sibling,
	// if not we are the last child of parent, and make our previous sibling the new last child.
	if (widget->sibling_next_id.raw) {
		HeroUIWidget* sibling_next;
		HeroResult result = hero_ui_widget_get(window, widget->sibling_next_id, &sibling_next);
		HERO_RESULT_ASSERT(result);

		sibling_next->sibling_prev_id = widget->sibling_prev_id;
	} else {
		HeroUIWidget* parent;
		HeroResult result = hero_ui_widget_get(window, widget->parent_id, &parent);
		HERO_RESULT_ASSERT(result);

		parent->child_last_id = widget->sibling_next_id;
	}

	widget->parent_id = HERO_UI_WIDGET_ID_NULL;
	widget->sibling_prev_id = HERO_UI_WIDGET_ID_NULL;
	widget->sibling_next_id = HERO_UI_WIDGET_ID_NULL;
}

void _hero_ui_widget_layout(HeroUIWindow* window, HeroUIWidget* widget) {
	HeroResult result;

	//
	// convert the layout from an outer layout to an inner layout
	HeroAabb inner_area = widget->area;
	if (widget->styles) {
		HeroUIWidgetStyle* style = hero_ui_widget_style(widget);
		hero_ui_area_outer_to_inner(&inner_area, style);
	}
	HeroAabb layout = inner_area;

	//
	// if the widget has children that want to proportionally fill
	// this widget's inner area. then work out how to evenly fill
	// the remaining space after the fixed controls length have
	// been deducted.
	F32 fill_proportion_length = 0;
	if (widget->fill_proportion_fill_children_count) {
		if (widget->flags & HERO_UI_WIDGET_FLAGS_CHILD_HAS_CUT_HORIZONTAL) {
			fill_proportion_length = layout.ex - layout.x;
		} else if (widget->flags & HERO_UI_WIDGET_FLAGS_CHILD_HAS_CUT_VERTICAL) {
			fill_proportion_length = layout.ey - layout.y;
		}
		fill_proportion_length -= widget->fill_proportion_fixed_children_length;
		fill_proportion_length /= (F32)widget->fill_proportion_fill_children_count;
		fill_proportion_length = HERO_MAX(fill_proportion_length, 0);
	}

	//
	// iterate over the children and lay them out
	//
	F32 list_item_length = 0.f;
	U32 children_count = 0;
	HeroUIWidget* child = NULL;
	HeroUIWidgetId sibling_next_id = HERO_UI_WIDGET_ID_NULL;
	for (HeroUIWidgetId child_id = widget->child_first_id; child_id.raw; child_id = sibling_next_id) {
		HeroResult result = hero_ui_widget_get(window, child_id, &child);
		HERO_RESULT_ASSERT(result);
		sibling_next_id = child->sibling_next_id;

		//
		// if the widget was not updated this frame, then remove it from the tree.
		if (child->last_updated_frame_idx != window->build.last_updated_frame_idx) {
			_hero_ui_widget_tree_remove(window, child, child_id);
			result = _hero_ui_widget_dealloc(window, child, child_id);
			HERO_RESULT_ASSERT(result);
			continue;
		}

		//
		// perform the cut operation on the layout
		//
		HeroUIWidgetStyle* style = hero_ui_widget_style(child);
		HeroUICut cut = child->cut;
		F32 cut_length = child->cut_length;
		if (cut_length == HERO_UI_LEN_FILL) {
			cut_length = fill_proportion_length;
		}

		HeroAabb child_area;
		if (child->flags & HERO_UI_WIDGET_FLAGS_CUSTOM_OFFSET) {
			Vec2 size;
			if (hero_ui_cut_is_horizontal(child->cut)) {
				size.x = cut_length;
				size.y = child->custom_perp_cut_length;
			} else {
				size.x = child->custom_perp_cut_length;
				size.y = cut_length;
			}

			Vec2 top_left = child->custom_offset;

			switch (child->custom_align & HERO_UI_ALIGN_X_RIGHT) {
				case HERO_UI_ALIGN_X_CENTER:
					top_left.x -= size.x / 2.f;
					break;
				case HERO_UI_ALIGN_X_LEFT:
					break;
				case HERO_UI_ALIGN_X_RIGHT:
					top_left.x -= size.x;
					break;
			}

			switch (child->custom_align & HERO_UI_ALIGN_Y_BOTTOM) {
				case HERO_UI_ALIGN_Y_CENTER:
					top_left.y -= size.y / 2.f;
					break;
				case HERO_UI_ALIGN_Y_TOP:
					break;
				case HERO_UI_ALIGN_Y_BOTTOM:
					top_left.y -= size.y;
					break;
			}

			child_area.x = top_left.x;
			child_area.y = top_left.y;
			child_area.ex = top_left.x + size.x;
			child_area.ey = top_left.y + size.y;
		} else if (child->flags & HERO_UI_WIDGET_FLAGS_NEXT_LAYER) {
			//
			// reset the layout to the parent's original size.
			child_area = inner_area;
		} else {
			switch (cut) {
				case HERO_UI_CUT_CENTER_HORIZONTAL: child_area = hero_aabb_cut_center_horizontal(&layout, cut_length + style->margin.left + style->margin.right); break;
				case HERO_UI_CUT_CENTER_VERTICAL: child_area = hero_aabb_cut_center_vertical(&layout, cut_length + style->margin.top + style->margin.bottom); break;
				case HERO_UI_CUT_LEFT: child_area = hero_aabb_cut_left(&layout, cut_length + style->margin.left + style->margin.right); break;
				case HERO_UI_CUT_TOP: child_area = hero_aabb_cut_top(&layout, cut_length + style->margin.top + style->margin.bottom); break;
				case HERO_UI_CUT_RIGHT: child_area = hero_aabb_cut_right(&layout, cut_length + style->margin.left + style->margin.right); break;
				case HERO_UI_CUT_BOTTOM: child_area = hero_aabb_cut_bottom(&layout, cut_length + style->margin.top + style->margin.bottom); break;
			}
		}

		child->area = child_area;

		//
		// layout the child if it is visible
		float width = child->area.ex - child->area.x;
		float height = child->area.ey - child->area.y;
		if (width > 0.f && height > 0.f) {
			list_item_length += height;
			_hero_ui_widget_layout(window, child);
			children_count += 1;
		}
	}
}

void _hero_ui_widget_render_children(HeroUIWindow* window, HeroUIWidget* widget);
void _hero_ui_widget_render(HeroUIWindow* window, HeroUIWidget* widget) {
	HeroResult result;
	HeroUIWidgetStyle* style = hero_ui_widget_style(widget);

	HeroAabb* area = &widget->area;
	float width = area->ex - area->x - style->margin.left - style->margin.right;
	float height = area->ey - area->y - style->margin.top - style->margin.bottom;
	if (width <= 0.f || height <= 0.f) {
		return;
	}

	if (widget->render_fn) {
		result = widget->render_fn(window, widget);
		HERO_RESULT_ASSERT(result);
	}

	width -= style->padding.left - style->padding.right - style->border_width - style->border_width;
	height -= style->padding.top - style->padding.bottom - style->border_width - style->border_width;
	if (width <= 0.f || height <= 0.f) {
		return;
	}

	_hero_ui_widget_render_children(window, widget);
}

void _hero_ui_widget_render_children(HeroUIWindow* window, HeroUIWidget* widget) {
	HeroUIWidget* child = NULL;
	HeroUIWidgetId sibling_next_id = HERO_UI_WIDGET_ID_NULL;
	for (HeroUIWidgetId child_id = widget->child_first_id; child_id.raw; child_id = sibling_next_id) {
		HeroResult result = hero_ui_widget_get(window, child_id, &child);
		HERO_RESULT_ASSERT(result);
		sibling_next_id = child->sibling_next_id;

		_hero_ui_widget_render(window, child);
	}
}

// ===========================================
//
//
// Draw
//
//
// ===========================================

HeroResult _hero_ui_widget_draw_push(HeroUIWindow* window, HeroUIDrawCmd** draw_cmd_out) {
	return hero_stack(HeroUIDrawCmd, push)(&window->update.render_data.draw_cmds, hero_system_alctor, 0, draw_cmd_out);
}

HeroResult hero_ui_widget_draw_aabb(HeroUIWindow* window, HeroAabb* aabb, F32 radius, HeroColor color) {
	HeroUIDrawCmd* draw_cmd;
	HeroResult result = _hero_ui_widget_draw_push(window, &draw_cmd);
	if (result < 0) {
		return result;
	}

	if (window->update.render_data.aabbs_count && window->update.render_data.aabbs_count % HERO_UI_AABBS_CAP == 0) {
		window->update.render_data.materials_count += 1;
	}
	window->update.render_data.aabbs_count += 1;

	draw_cmd->type = HERO_UI_DRAW_CMD_TYPE_AABB;
	draw_cmd->data.aabb.aabb = *aabb;
	draw_cmd->data.aabb.radius = radius;
	draw_cmd->data.aabb.color = color;
	draw_cmd->data.aabb.border_width = 0.f;

	return HERO_SUCCESS;
}

HeroResult hero_ui_widget_draw_aabb_border(HeroUIWindow* window, HeroAabb* aabb, F32 radius, HeroColor color, F32 border_width) {
	HeroUIDrawCmd* draw_cmd;
	HeroResult result = _hero_ui_widget_draw_push(window, &draw_cmd);
	if (result < 0) {
		return result;
	}

	if (window->update.render_data.aabbs_count && window->update.render_data.aabbs_count % HERO_UI_AABBS_CAP == 0) {
		window->update.render_data.materials_count += 1;
	}
	window->update.render_data.aabbs_count += 1;

	draw_cmd->type = HERO_UI_DRAW_CMD_TYPE_AABB_BORDER;
	draw_cmd->data.aabb.aabb = *aabb;
	draw_cmd->data.aabb.radius = radius;
	draw_cmd->data.aabb.color = color;
	draw_cmd->data.aabb.border_width = border_width;

	return HERO_SUCCESS;
}

HeroResult _hero_ui_widget_draw_image_push(HeroUIWindow* window, HeroUIImageId image_id, HeroUIDrawCmd** draw_cmd_out) {
	HeroResult result = _hero_ui_widget_draw_push(window, draw_cmd_out);
	if (result < 0) {
		return result;
	}

	U32 image_idx = -1;
	for_range(i, 0, window->update.render_data.unique_image_atlas_ids.count) {
		if (
			window->update.render_data.unique_image_atlas_ids.data[i].raw == image_id.atlas_id.raw
		) {
			image_idx = i;
			break;
		}
	}
	if (image_idx == -1) {
		image_idx = window->update.render_data.unique_image_atlas_ids.count;
		result = hero_stack(HeroUIImageAtlasId, push_value)(&window->update.render_data.unique_image_atlas_ids, hero_system_alctor, 0, image_id.atlas_id);
		if (result < 0) {
			return result;
		}
	}

	U32 image_group_idx = image_idx / HERO_UI_TEXTURES_CAP;
	image_idx %= HERO_UI_TEXTURES_CAP;

	if (window->render.image_group_idx != image_group_idx) {
		window->update.render_data.materials_count += 1;
	}

	(*draw_cmd_out)->data.image.image_group_idx = image_group_idx;
	(*draw_cmd_out)->data.image.image_idx = image_idx;

	return HERO_SUCCESS;
}

HeroResult hero_ui_widget_draw_image(HeroUIWindow* window, HeroAabb* aabb, HeroUIImageId image_id, HeroColor color) {
	HeroUIDrawCmd* draw_cmd;
	HeroResult result = _hero_ui_widget_draw_image_push(window, image_id, &draw_cmd);
	if (result < 0) {
		return result;
	}

	draw_cmd->type = HERO_UI_DRAW_CMD_TYPE_IMAGE;
	draw_cmd->data.image.aabb = *aabb;
	draw_cmd->data.image.image_id = image_id;
	draw_cmd->data.image.bg_color = color;

	return HERO_SUCCESS;
}

HeroResult hero_ui_widget_draw_image_grayscale_remap(HeroUIWindow* window, HeroAabb* aabb, HeroUIImageId image_id, HeroColor bg_color, HeroColor fg_color) {
	HeroUIDrawCmd* draw_cmd;
	HeroResult result = _hero_ui_widget_draw_image_push(window, image_id, &draw_cmd);
	if (result < 0) {
		return result;
	}

	draw_cmd->type = HERO_UI_DRAW_CMD_TYPE_IMAGE_GRAYSCALE_REMAP;
	draw_cmd->data.image.aabb = *aabb;
	draw_cmd->data.image.image_id = image_id;
	draw_cmd->data.image.bg_color = bg_color;
	draw_cmd->data.image.fg_color = fg_color;

	return HERO_SUCCESS;
}

HeroResult hero_ui_widget_draw_circle(HeroUIWindow* window, Vec2 center_pos, F32 radius, HeroColor color) {
	HeroUIDrawCmd* draw_cmd;
	HeroResult result = _hero_ui_widget_draw_push(window, &draw_cmd);
	if (result < 0) {
		return result;
	}

	if (window->update.render_data.circles_count && window->update.render_data.circles_count % HERO_UI_CIRCLES_CAP == 0) {
		window->update.render_data.materials_count += 1;
	}
	window->update.render_data.circles_count += 1;

	draw_cmd->type = HERO_UI_DRAW_CMD_TYPE_CIRCLE;
	draw_cmd->data.circle.center_pos = center_pos;
	draw_cmd->data.circle.radius = radius;
	draw_cmd->data.circle.color = color;
	draw_cmd->data.circle.border_width = 0.f;

	return HERO_SUCCESS;
}

HeroResult hero_ui_widget_draw_circle_border(HeroUIWindow* window, Vec2 center_pos, F32 radius, HeroColor color, F32 border_width) {
	HeroUIDrawCmd* draw_cmd;
	HeroResult result = _hero_ui_widget_draw_push(window, &draw_cmd);
	if (result < 0) {
		return result;
	}

	if (window->update.render_data.circles_count && window->update.render_data.circles_count % HERO_UI_CIRCLES_CAP == 0) {
		window->update.render_data.materials_count += 1;
	}
	window->update.render_data.circles_count += 1;

	draw_cmd->type = HERO_UI_DRAW_CMD_TYPE_CIRCLE_BORDER;
	draw_cmd->data.circle.center_pos = center_pos;
	draw_cmd->data.circle.radius = radius;
	draw_cmd->data.circle.color = color;
	draw_cmd->data.circle.border_width = border_width;

	return HERO_SUCCESS;
}

// ===========================================
//
//
// Graphics: Vertex
//
//
// ===========================================

typedef struct HeroUIVertex HeroUIVertex;
struct HeroUIVertex {
	Vec2 pos;
	HeroUIDrawAux aux;
	// uv coord when type == HERO_UI_DRAW_CMD_TYPE_{IMAGE, IMAGE_GRAYSCALE_REMAP}
	// x = radius, y = border_width when type == HERO_UI_DRAW_CMD_TYPE_{AABB, AABB_BORDER, CIRCLE, CIRCLE_BORDER}
	Vec2 uv;
};

static HeroVertexAttribInfo hero_ui_vertex_attribs[] = {
	{ // HeroUIVertex.pos
		.location = 0,
		.elmt_type = HERO_VERTEX_ELMT_TYPE_F32,
		.vector_type = HERO_VERTEX_VECTOR_TYPE_2,
	},
	{ // HeroUIVertex.aux
		.location = 1,
		.elmt_type = HERO_VERTEX_ELMT_TYPE_U32,
		.vector_type = HERO_VERTEX_VECTOR_TYPE_1,
	},
	{ // HeroUIVertex.uv
		.location = 2,
		.elmt_type = HERO_VERTEX_ELMT_TYPE_F32,
		.vector_type = HERO_VERTEX_VECTOR_TYPE_2,
	},
};

static HeroVertexBindingInfo hero_ui_vertex_bindings[] = {
	{
		.attribs = hero_ui_vertex_attribs,
		.attribs_count = HERO_ARRAY_COUNT(hero_ui_vertex_attribs),
		.size = sizeof(HeroUIVertex),
	}
};

static HeroVertexLayout hero_ui_vertex_layout = {
	.bindings_count = HERO_ARRAY_COUNT(hero_ui_vertex_bindings),
	.bindings = hero_ui_vertex_bindings,
};

// ===========================================
//
//
// Window
//
//
// ===========================================

HeroResult _hero_ui_uniform_buffer_init(HeroLogicalDevice* ldev, U64 elmts_count, U64 elmt_size, HeroBufferId* id_out) {
	HeroBufferSetup buffer_setup = {
		.flags = HERO_BUFFER_FLAGS_USED_FOR_GRAPHICS,
		.type = HERO_BUFFER_TYPE_UNIFORM,
		.memory_location = HERO_MEMORY_LOCATION_SHARED,
		.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
		.elmts_count = elmts_count,
		.elmt_size = elmt_size,
	};

	return hero_buffer_init(ldev, &buffer_setup, id_out);
}

HeroResult hero_ui_window_init(HeroUIWindowSetup* setup, HeroUIWindowId* id_out) {
	HeroResult result;

	HeroUIWindow* window;
	result = hero_object_pool(HeroUIWindow, alloc)(&hero_ui_sys.window_pool, &window, id_out);
	if (result < 0) {
		return result;
	}
	window->window_id = setup->window_id;

	result = hero_object_pool(HeroUIWidget, init)(&window->widget_pool, setup->widgets_cap, hero_system_alctor, 0);
	if (result < 0) {
		return result;
	}

	HeroUIWidget* root_widget;
	HeroUIWidgetId root_widget_id;
	result = _hero_ui_widget_alloc(window, &root_widget, &root_widget_id);
	if (result < 0) {
		return result;
	}

	{
		HeroBufferSetup buffer_setup = {
			.flags = HERO_BUFFER_FLAGS_USED_FOR_GRAPHICS,
			.type = HERO_BUFFER_TYPE_VERTEX,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.elmts_count = 4,
			.typed.vertex.layout_id = hero_ui_sys.vertex_layout_id,
			.typed.vertex.binding_idx = 0,
		};

		result = hero_buffer_init(setup->ldev, &buffer_setup, &window->render.vertex_buffer_id);
		HERO_RESULT_ASSERT(result);
	}

	{
		HeroBufferSetup buffer_setup = {
			.flags = HERO_BUFFER_FLAGS_USED_FOR_GRAPHICS,
			.type = HERO_BUFFER_TYPE_INDEX,
			.memory_location = HERO_MEMORY_LOCATION_SHARED,
			.queue_support_flags = HERO_QUEUE_SUPPORT_FLAGS_GRAPHICS,
			.elmts_count = 6,
			.typed.index_type = HERO_INDEX_TYPE_U32,
		};

		result = hero_buffer_init(setup->ldev, &buffer_setup, &window->render.index_buffer_id);
		HERO_RESULT_ASSERT(result);
	}

	result = _hero_ui_uniform_buffer_init(setup->ldev, 1, sizeof(HeroUIGlobalUBO), &window->render.global_uniform_buffer_id);
	HERO_RESULT_ASSERT(result);

	{
		HeroShaderGlobalsSetup shader_globals_setup = {
			.shader_id = hero_ui_sys.shader_id,
			.descriptor_pool_id = hero_ui_sys.descriptor_pool_id,
		};

		result = hero_shader_globals_init(setup->ldev, &shader_globals_setup, &window->render.shader_globals_id);
		HERO_RESULT_ASSERT(result);

		result = hero_shader_globals_set_uniform_buffer(setup->ldev, window->render.shader_globals_id, 0, 0, window->render.global_uniform_buffer_id, 0);
		HERO_RESULT_ASSERT(result);

		result = hero_shader_globals_update(setup->ldev, window->render.shader_globals_id);
		HERO_RESULT_ASSERT(result);
	}

	return HERO_SUCCESS;
}

HeroResult hero_ui_window_deinit(HeroUIWindowId id);

void _hero_ui_widget_find_mouse_focused(HeroUIWindow* window, HeroUIWidget* widget, HeroUIWidgetId widget_id) {
	HeroAabb parent_clip_rect = window->clip_rect;

	window->clip_rect = hero_aabb_keep_overlapping(&window->clip_rect, &widget->area);
	Vec2 mouse_pt = VEC2_INIT(hero_window_sys.mouse.x, hero_window_sys.mouse.y);

	if (
		!hero_ui_sys.is_mouse_over_widget &&
		widget == window->widget_pool.data && // is_root_widget
		hero_color_a(hero_ui_widget_style(widget)->background_color) != 0
	) {
		hero_ui_sys.is_mouse_over_widget = hero_aabb_intersects_pt(&widget->area, mouse_pt);
	}

	if (hero_aabb_intersects_pt(&window->clip_rect, mouse_pt)) {
		if (widget->flags & HERO_UI_WIDGET_FLAGS_IS_FOCUSABLE) {
			hero_ui_widget_mouse_focused_set(window, widget_id);
		}
	}

	HeroUIWidget* child = NULL;
	HeroUIWidgetId sibling_next_id = HERO_UI_WIDGET_ID_NULL;
	for (HeroUIWidgetId child_id = widget->child_first_id; child_id.raw; child_id = child->sibling_next_id) {
		HeroResult result = hero_ui_widget_get(window, child_id, &child);
		HERO_RESULT_ASSERT(result);

		_hero_ui_widget_find_mouse_focused(window, child, child_id);
	}

	window->clip_rect = parent_clip_rect;
}

HeroResult hero_ui_window_get(HeroUIWindowId id, HeroUIWindow** out) {
	return hero_object_pool(HeroUIWindow, get)(&hero_ui_sys.window_pool, id, out);
}

HeroUIWindow* hero_ui_window_start(HeroUIWindowId id, U32 render_width, U32 render_height) {
	HeroUIWindow* window;
	HeroResult result = hero_object_pool(HeroUIWindow, get)(&hero_ui_sys.window_pool, id, &window);

	HeroUIWidget* root_widget;
	result = hero_ui_widget_get(window, HERO_UI_ROOT_WIDGET_ID, &root_widget);
	HERO_RESULT_ASSERT(result);

	if (window->window_id.raw == hero_window_sys.mouse.focused_window_id.raw) {
		HeroResult result = hero_ui_widget_mouse_focused_set(window, HERO_UI_WIDGET_ID_NULL);
		HERO_RESULT_ASSERT(result);

		hero_ui_sys.is_mouse_over_widget = false;
		window->clip_rect = root_widget->area;
		_hero_ui_widget_find_mouse_focused(window, root_widget, HERO_UI_ROOT_WIDGET_ID);
	}

	//
	// setup the build state for this frame
	HeroUIWindowBuild* build = &window->build;
	{
		build->parent_id = HERO_UI_ROOT_WIDGET_ID;
		build->parent = root_widget;

		//
		// setup the root widget
		_hero_ui_widget_frame_reset(build->parent);
		build->parent->area = HERO_AABB_INIT(0, 0, render_width, render_height);

		build->sibling_prev = NULL;
		build->sibling_prev_id = HERO_UI_WIDGET_ID_NULL;

		build->last_updated_frame_idx += 1;
	}

	return window;
}

void hero_ui_window_end(HeroUIWindow* window) {
	HeroUIWidget* root_widget;
	HeroResult result = hero_ui_widget_get(window, HERO_UI_ROOT_WIDGET_ID, &root_widget);
	HERO_RESULT_ASSERT(result);

	_hero_ui_widget_layout(window, root_widget);

	HeroUIWindowBuild* build = &window->build;
	build->parent = NULL;
	build->parent_id = HERO_UI_WIDGET_ID_NULL;
	build->sibling_prev = NULL;
	build->sibling_prev_id = HERO_UI_WIDGET_ID_NULL;
}

HeroResult hero_ui_window_update(HeroUIWindowId id) {
	HeroUIWindow* window;
	HeroResult result = hero_object_pool(HeroUIWindow, get)(&hero_ui_sys.window_pool, id, &window);
	if (result < 0) {
		return result;
	}
	HeroUIWindowUpdate* update = &window->update;

	//
	// generate the HeroUIDrawCmd structures by traversing the UI tree.
	// also collect some metrics so we know how much GPU data is needed after this.
	{
		update->render_data.draw_cmds.count = 0;
		update->render_data.aabbs_count = 0;
		update->render_data.circles_count = 0;
		update->render_data.materials_count = 1;
		update->render_data.unique_image_atlas_ids.count = 0;
		update->image_group_idx = 0;

		HeroUIWidget* root_widget;
		result = hero_ui_widget_get(window, HERO_UI_ROOT_WIDGET_ID, &root_widget);
		HERO_RESULT_ASSERT(result);

		_hero_ui_widget_render_children(window, root_widget);
	}

	return HERO_SUCCESS;
}

HeroResult hero_ui_window_update_render_data(HeroUIWindowId id) {
	HeroUIWindow* window;
	HeroResult result = hero_object_pool(HeroUIWindow, get)(&hero_ui_sys.window_pool, id, &window);
	if (result < 0) {
		return result;
	}
	HeroUIWindowUpdate* update = &window->update;
	HeroUIWindowRender* render = &window->render;

	{
		HeroStack(HeroUIDrawCmd) tmp = render->render_data.draw_cmds;
		render->render_data.draw_cmds = update->render_data.draw_cmds;
		update->render_data.draw_cmds = tmp;
	}

	{
		HeroStack(HeroUIImageAtlasId) tmp = render->render_data.unique_image_atlas_ids;
		render->render_data.unique_image_atlas_ids = update->render_data.unique_image_atlas_ids;
		update->render_data.unique_image_atlas_ids = tmp;
	}

	render->render_data.aabbs_count = update->render_data.aabbs_count;
	render->render_data.circles_count = update->render_data.circles_count;
	render->render_data.materials_count = update->render_data.materials_count;

	{
		HeroUIWidget* root_widget;
		result = hero_ui_widget_get(window, HERO_UI_ROOT_WIDGET_ID, &root_widget);
		HERO_RESULT_ASSERT(result);

		render->render_data.window_width = (U32)root_widget->area.ex;
		render->render_data.window_height = (U32)root_widget->area.ey;
	}

	return HERO_SUCCESS;
}

HeroResult _hero_window_ui_render_new_material(HeroUIWindow* window, HeroLogicalDevice* ldev) {
	HeroResult result;

	HeroUIWindowRender* render = &window->render;

	HeroMaterialId material_id = render->material_ids.data[render->render_data.materials_count];
	render->render_data.materials_count += 1;

	result = hero_material_set_uniform_buffer(ldev, material_id, HERO_UI_MATERIAL_BINDING_COLOR_UBO, 0, render->color_uniform_buffer_ids.data[render->color_uniform_buffer_idx], 0);
	if (result < 0) {
		return result;
	}

	result = hero_material_set_uniform_buffer(ldev, material_id, HERO_UI_MATERIAL_BINDING_AABB_UBO, 0, render->aabb_uniform_buffer_ids.data[render->aabb_uniform_buffer_idx], 0);
	if (result < 0) {
		return result;
	}

	result = hero_material_set_uniform_buffer(ldev, material_id, HERO_UI_MATERIAL_BINDING_CIRCLE_UBO, 0, render->circle_uniform_buffer_ids.data[render->circle_uniform_buffer_idx], 0);
	if (result < 0) {
		return result;
	}

	U32 image_start_idx = render->image_group_idx * HERO_UI_TEXTURES_CAP;
	U32 image_end_idx = image_start_idx + HERO_UI_TEXTURES_CAP;
	image_end_idx = HERO_MIN(image_end_idx, render->render_data.unique_image_atlas_ids.count);

	for_range(i, image_start_idx, image_end_idx) {
		HeroUIImageAtlasId atlas_id = render->render_data.unique_image_atlas_ids.data[i];

		HeroUIImageAtlas* atlas;
		result = hero_ui_image_atlas_get(atlas_id, &atlas);
		if (result < 0) {
			return result;
		}

		result = hero_material_set_image_sampler(ldev, material_id, HERO_UI_MATERIAL_BINDING_TEXTURES, i - image_start_idx, atlas->image_id, atlas->sampler_id);
		if (result < 0) {
			return result;
		}
	}

	result = hero_material_update(ldev, material_id);
	HERO_RESULT_ASSERT(result);

	return HERO_SUCCESS;
}

HeroResult hero_ui_window_render(HeroUIWindowId id, HeroLogicalDevice* ldev, HeroCommandRecorder* command_recorder) {
	HeroUIWindow* window;
	HeroResult result = hero_object_pool(HeroUIWindow, get)(&hero_ui_sys.window_pool, id, &window);
	if (result < 0) {
		return result;
	}
	HeroUIWindowRender* render = &window->render;

	{
		HeroUIGlobalUBO* ubo;
		result = hero_buffer_write(ldev, render->global_uniform_buffer_id, 0, 1, (void**)&ubo);
		HERO_RESULT_ASSERT(result);
		mat4x4_ortho(&ubo->mvp, 0.f, render->render_data.window_width, render->render_data.window_height, 0.f, -1.f, 1.f);
	}

	//
	//
	// now we need to convert the HeroUIDrawCmd into GPU friendly data that our ui shader uses.
	//
	//

	//
	// resize buffers & allocate all the GPU resources
	U32 vertices_count = render->render_data.draw_cmds.count * 4;
	U32 indices_count = render->render_data.draw_cmds.count * 6;
	{

		result = hero_buffer_reserve(ldev, render->vertex_buffer_id, vertices_count);
		HERO_RESULT_ASSERT(result);
		result = hero_buffer_reserve(ldev, render->index_buffer_id, indices_count);
		HERO_RESULT_ASSERT(result);

		U32 color_uniform_buffers_count = (render->render_data.draw_cmds.count / HERO_UI_COLORS_CAP) + 1;
		U32 aabb_uniform_buffers_count = (render->render_data.aabbs_count / HERO_UI_AABBS_CAP) + 1;
		U32 circle_uniform_buffers_count = (render->render_data.circles_count / HERO_UI_CIRCLES_CAP) + 1;

		for_range(i, render->color_uniform_buffer_ids.count, color_uniform_buffers_count) {
			HeroBufferId* dst;
			result = hero_stack(HeroBufferId, push)(&render->color_uniform_buffer_ids, hero_system_alctor, 0, &dst);
			if (result < 0) {
				return result;
			}

			result =  _hero_ui_uniform_buffer_init(ldev, HERO_UI_COLORS_CAP, sizeof(HeroColor), dst);
			if (result < 0) {
				return result;
			}
		}

		for_range(i, render->aabb_uniform_buffer_ids.count, aabb_uniform_buffers_count) {
			HeroBufferId* dst;
			result = hero_stack(HeroBufferId, push)(&render->aabb_uniform_buffer_ids, hero_system_alctor, 0, &dst);
			if (result < 0) {
				return result;
			}

			result =  _hero_ui_uniform_buffer_init(ldev, HERO_UI_AABBS_CAP, sizeof(Vec4), dst);
			if (result < 0) {
				return result;
			}
		}

		for_range(i, render->circle_uniform_buffer_ids.count, circle_uniform_buffers_count) {
			HeroBufferId* dst;
			result = hero_stack(HeroBufferId, push)(&render->circle_uniform_buffer_ids, hero_system_alctor, 0, &dst);
			if (result < 0) {
				return result;
			}

			result =  _hero_ui_uniform_buffer_init(ldev, HERO_UI_CIRCLES_CAP, sizeof(Vec2), dst);
			if (result < 0) {
				return result;
			}
		}

		for_range(i, render->material_ids.count, render->render_data.materials_count) {
			HeroMaterialId* dst;
			result = hero_stack(HeroMaterialId, push)(&render->material_ids, hero_system_alctor, 0, &dst);
			if (result < 0) {
				return result;
			}

			HeroMaterialSetup setup = {
				.pipeline_id = hero_ui_sys.pipeline_id,
				.shader_globals_id = render->shader_globals_id,
				.descriptor_pool_id = hero_ui_sys.descriptor_pool_id,
			};

			result = hero_material_init(ldev, &setup, dst);
			if (result < 0) {
				return result;
			}
		}
	}

	//
	// get the write buffers to the vertices, indices & initial uniform buffers.
	// later we will need to advance to the next uniform buffers when will fill them up.
	HeroUIVertex* vertices;
	U32* indices;
	HeroColor* colors;
	Vec4* aabbs;
	Vec2* circle_positions;
	U32 remaining_colors_count = render->render_data.draw_cmds.count;
	U32 remaining_aabbs_count = render->render_data.aabbs_count;
	U32 remaining_circles_count = render->render_data.circles_count;
	{
		result =  hero_buffer_write(ldev, render->vertex_buffer_id, 0, vertices_count, (void**)&vertices);
		if (result < 0) {
			return result;
		}

		result =  hero_buffer_write(ldev, render->index_buffer_id, 0, indices_count, (void**)&indices);
		if (result < 0) {
			return result;
		}

		U32 count = remaining_colors_count % HERO_UI_COLORS_CAP;
		result =  hero_buffer_write(ldev, render->color_uniform_buffer_ids.data[0], 0, count, (void**)&colors);
		if (result < 0) {
			return result;
		}
		remaining_colors_count -= count;

		count = remaining_aabbs_count % HERO_UI_AABBS_CAP;
		result =  hero_buffer_write(ldev, render->aabb_uniform_buffer_ids.data[0], 0, count, (void**)&aabbs);
		if (result < 0) {
			return result;
		}
		remaining_aabbs_count -= count;

		count = remaining_circles_count % HERO_UI_CIRCLES_CAP;
		result =  hero_buffer_write(ldev, render->circle_uniform_buffer_ids.data[0], 0, count, (void**)&circle_positions);
		if (result < 0) {
			return result;
		}
		remaining_circles_count -= count;
	}

	U32 vertex_idx = 0;
	U32 index_idx = 0;
	U32 colors_count = 0;
	U32 aabbs_count = 0;
	U32 circles_count = 0;
	U32 materials_count = 1;
	render->color_uniform_buffer_idx = 0;
	render->aabb_uniform_buffer_idx = 0;
	render->circle_uniform_buffer_idx = 0;
	render->image_group_idx = 0;
	render->render_data.materials_count = 0;
	U32 prev_draw_call_vertices_idx = 0;
	U32 prev_draw_call_indices_idx = 0;
	U32 prev_draw_call_colors_count = 0;

	result = _hero_window_ui_render_new_material(window, ldev);
	if (result < 0) {
		return result;
	}

	//
	// loop over all draw commands and translate the data into GPU friendly data for our shader.
	// we will up the vertices and indices buffers until the material changes that triggers another draw call
	// to be recordered into the GPU command buffer.
	U32 cmd_idx = 0;
	for (; cmd_idx < render->render_data.draw_cmds.count; cmd_idx += 1) {
		HeroUIDrawCmd* cmd = &render->render_data.draw_cmds.data[cmd_idx];
		HeroAabb aabb;
		HeroAabb* aabb_ptr;

		//
		// all draw commnds have a color so advance the color here
		if (colors_count == HERO_UI_COLORS_CAP) {
			colors_count = 0;
			render->color_uniform_buffer_idx += 1;

			U32 count = remaining_colors_count % HERO_UI_COLORS_CAP;
			result =  hero_buffer_write(ldev, render->color_uniform_buffer_ids.data[render->color_uniform_buffer_idx], 0, count, (void**)&colors);
			if (result < 0) {
				return result;
			}
			remaining_colors_count -= count;

			result = _hero_window_ui_render_new_material(window, ldev);
			if (result < 0) {
				return result;
			}
		}
		colors_count += 1;

		HeroUIDrawAux aux = 0;
		Vec2 uv = {0};

		//
		// deal with the draw command type specific data here
		HERO_UI_DRAW_AUX_TYPE_SET(&aux, cmd->type);
		switch (cmd->type) {
			case HERO_UI_DRAW_CMD_TYPE_AABB:
			case HERO_UI_DRAW_CMD_TYPE_AABB_BORDER: {
				if (aabbs_count == HERO_UI_AABBS_CAP) {
					aabbs_count = 0;
					render->aabb_uniform_buffer_idx += 1;

					U32 count = remaining_aabbs_count % HERO_UI_AABBS_CAP;
					result =  hero_buffer_write(ldev, render->aabb_uniform_buffer_ids.data[render->aabb_uniform_buffer_idx], 0, count, (void**)&aabbs);
					if (result < 0) {
						return result;
					}
					remaining_aabbs_count -= count;

					result = _hero_window_ui_render_new_material(window, ldev);
					if (result < 0) {
						return result;
					}
				}

				HERO_UI_DRAW_AUX_SHAPE_IDX_SET(&aux, aabbs_count);
				Vec2 half_size = hero_aabb_half_size(&cmd->data.aabb.aabb);

				Vec4* dst = &aabbs[aabbs_count];
				dst->x = cmd->data.aabb.aabb.x + half_size.x;
				dst->y = cmd->data.aabb.aabb.y + half_size.y;
				dst->z = half_size.x;
				dst->w = half_size.y;

				uv.x = cmd->data.aabb.radius;
				uv.y = cmd->data.aabb.border_width;

				aabbs_count += 1;

				colors[colors_count - 1] = cmd->data.aabb.color;
				aabb_ptr = &cmd->data.aabb.aabb;
				break;
			};
			case HERO_UI_DRAW_CMD_TYPE_IMAGE:
			case HERO_UI_DRAW_CMD_TYPE_IMAGE_GRAYSCALE_REMAP: {
				if (render->image_group_idx != cmd->data.image.image_group_idx) {
					render->image_group_idx = cmd->data.image.image_group_idx;
					result = _hero_window_ui_render_new_material(window, ldev);
					if (result < 0) {
						return result;
					}
				}
				HERO_UI_DRAW_AUX_TEXTURE_IDX_SET(&aux, cmd->data.image.image_idx);
				if (cmd->type == HERO_UI_DRAW_CMD_TYPE_IMAGE_GRAYSCALE_REMAP) {
					HERO_UI_DRAW_AUX_COLOR_FG_SET(&aux, cmd->data.image.fg_color);
				}
				colors[colors_count - 1] = cmd->data.image.bg_color;
				aabb_ptr = &cmd->data.image.aabb;
				break;
			};
			case HERO_UI_DRAW_CMD_TYPE_CIRCLE:
			case HERO_UI_DRAW_CMD_TYPE_CIRCLE_BORDER: {
				if (circles_count == HERO_UI_CIRCLES_CAP) {
					circles_count = 0;
					render->circle_uniform_buffer_idx += 1;

					U32 count = remaining_circles_count % HERO_UI_CIRCLES_CAP;
					result =  hero_buffer_write(ldev, render->circle_uniform_buffer_ids.data[render->circle_uniform_buffer_idx], 0, count, (void**)&circle_positions);
					if (result < 0) {
						return result;
					}
					remaining_circles_count -= count;

					result = _hero_window_ui_render_new_material(window, ldev);
					if (result < 0) {
						return result;
					}
				}

				HERO_UI_DRAW_AUX_SHAPE_IDX_SET(&aux, circles_count);
				circle_positions[circles_count] = cmd->data.circle.center_pos;
				uv.x = cmd->data.circle.radius;
				uv.y = cmd->data.circle.border_width;

				circles_count += 1;
				colors[colors_count - 1] = cmd->data.circle.color;
				aabb.x = cmd->data.circle.center_pos.x - cmd->data.circle.radius;
				aabb.y = cmd->data.circle.center_pos.y - cmd->data.circle.radius;
				aabb.ex = cmd->data.circle.center_pos.x + cmd->data.circle.radius;
				aabb.ey = cmd->data.circle.center_pos.y + cmd->data.circle.radius;
				aabb_ptr = &aabb;
				break;
			};
		}

		//
		// write the vertices for this draw command
		//

		vertices[vertex_idx + 0] = (HeroUIVertex) {
			.pos = hero_aabb_top_left(aabb_ptr),
			.aux = aux,
			.uv = uv,
		};

		vertices[vertex_idx + 1] = (HeroUIVertex) {
			.pos = hero_aabb_top_right(aabb_ptr),
			.aux = aux,
			.uv = uv,
		};

		vertices[vertex_idx + 2] = (HeroUIVertex) {
			.pos = hero_aabb_bottom_right(aabb_ptr),
			.aux = aux,
			.uv = uv,
		};

		vertices[vertex_idx + 3] = (HeroUIVertex) {
			.pos = hero_aabb_bottom_left(aabb_ptr),
			.aux = aux,
			.uv = uv,
		};

		//
		// set the vertex uvs if we are using an image
		//

		switch (cmd->type) {
			case HERO_UI_DRAW_CMD_TYPE_IMAGE:
			case HERO_UI_DRAW_CMD_TYPE_IMAGE_GRAYSCALE_REMAP: {
				HeroAabb uv_aabb;
				result = hero_ui_image_atlas_image_uv_aabb(cmd->data.image.image_id.atlas_id, cmd->data.image.image_id.image_idx, &uv_aabb);
				if (result < 0) {
					return result;
				}

				vertices[vertex_idx + 0].uv = hero_aabb_top_left(&uv_aabb);
				vertices[vertex_idx + 1].uv = hero_aabb_top_right(&uv_aabb);
				vertices[vertex_idx + 2].uv = hero_aabb_bottom_right(&uv_aabb);
				vertices[vertex_idx + 3].uv = hero_aabb_bottom_left(&uv_aabb);
				break;
			};
		}

		//
		// write the indices for this draw command
		//

		indices[index_idx + 0] = vertex_idx + 0;
		indices[index_idx + 1] = vertex_idx + 1;
		indices[index_idx + 2] = vertex_idx + 2;
		indices[index_idx + 3] = vertex_idx + 2;
		indices[index_idx + 4] = vertex_idx + 3;
		indices[index_idx + 5] = vertex_idx + 0;

		vertex_idx += 4;
		index_idx += 6;

		if (materials_count != render->render_data.materials_count) {
RECORD_DRAW:
			result = hero_cmd_draw_start(command_recorder, render->material_ids.data[materials_count - 1]);
			if (result < 0) {
				return result;
			}

			result = hero_cmd_draw_set_vertex_buffer(command_recorder, render->vertex_buffer_id, 0, prev_draw_call_vertices_idx);
			if (result < 0) {
				return result;
			}

			U32 indices_count = index_idx - prev_draw_call_indices_idx;
			result = hero_cmd_draw_end_indexed(command_recorder, render->index_buffer_id, prev_draw_call_indices_idx, indices_count, prev_draw_call_colors_count * 4);
			if (result < 0) {
				return result;
			}

			materials_count = render->render_data.materials_count;
			prev_draw_call_vertices_idx = vertex_idx;
			prev_draw_call_indices_idx = index_idx;
			prev_draw_call_colors_count = colors_count;
		}
	}

	if (prev_draw_call_vertices_idx != vertex_idx) {
		//
		// do one final draw with the remaining vertices & indices
		goto RECORD_DRAW;
	}

	return HERO_SUCCESS;
}

// ===========================================
//
//
// UI System
//
//
// ===========================================

HeroUISys hero_ui_sys;

HeroResult hero_ui_sys_init(HeroUISysSetup* setup) {
	HeroResult result;

	hero_ui_sys.shader_id = setup->shader_id;
	hero_ui_sys.descriptor_pool_id = setup->descriptor_pool_id;
	hero_ui_sys.text_size_fn = setup->text_size_fn;
	hero_ui_sys.text_render_fn = setup->text_render_fn;

	result = hero_object_pool(HeroUIWindow, init)(&hero_ui_sys.window_pool, setup->windows_cap, hero_system_alctor, 0);
	if (result < 0) {
		return result;
	}

	result = hero_object_pool(HeroUIImageAtlas, init)(&hero_ui_sys.image_atlas_pool, setup->image_atlases_cap, hero_system_alctor, 0);
	if (result < 0) {
		return result;
	}

	result = hero_vertex_layout_register(&hero_ui_vertex_layout, true, &hero_ui_sys.vertex_layout_id);
	HERO_RESULT_ASSERT(result);

	HeroPipelineId pipeline_id;
	{
		HeroRenderState render_state = {0};
		render_state.topology = HERO_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		render_state.viewports_count = 1;

		render_state.rasterization.polygon_mode = HERO_POLYGON_MODE_FILL;
		render_state.rasterization.front_face = HERO_FRONT_FACE_CLOCKWISE;
		render_state.rasterization.line_width = 1.f;

		render_state.multisample.rasterization_samples_count = HERO_SAMPLE_COUNT_1;

		static HeroRenderStateBlendAttachment blend_color_attachment = {0};
		blend_color_attachment.blend_enable = true;
		blend_color_attachment.color_write_mask = 0xf;
		blend_color_attachment.src_color_blend_factor = HERO_BLEND_FACTOR_SRC_ALPHA;
		blend_color_attachment.dst_color_blend_factor = HERO_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blend_color_attachment.color_blend_op = HERO_BLEND_OP_ADD;
		blend_color_attachment.src_alpha_blend_factor = HERO_BLEND_FACTOR_ONE;
		blend_color_attachment.dst_alpha_blend_factor = HERO_BLEND_FACTOR_ONE;
		blend_color_attachment.alpha_blend_op = HERO_BLEND_OP_ADD;

		render_state.blend.attachments = &blend_color_attachment;
		render_state.blend.attachments_count = 1;

		render_state.depth_stencil.flags = HERO_DEPTH_STENCIL_FLAGS_ENABLE_DEPTH_TEST | HERO_DEPTH_STENCIL_FLAGS_ENABLE_DEPTH_WRITE;
		render_state.depth_stencil.depth_compare_op = HERO_COMPARE_OP_LESS_OR_EQUAL;
		render_state.depth_stencil.back.fail_op = HERO_STENCIL_OP_KEEP;
		render_state.depth_stencil.back.pass_op = HERO_STENCIL_OP_KEEP;
		render_state.depth_stencil.back.compare_op = HERO_COMPARE_OP_ALWAYS;
		render_state.depth_stencil.front.fail_op = HERO_STENCIL_OP_KEEP;
		render_state.depth_stencil.front.pass_op = HERO_STENCIL_OP_KEEP;
		render_state.depth_stencil.front.compare_op = HERO_COMPARE_OP_ALWAYS;

		HeroPipelineGraphicsSetup pipeline_setup = {
			.render_state = &render_state,
			.shader_id = setup->shader_id,
			.render_pass_layout_id = setup->render_pass_layout_id,
			.vertex_layout_id = hero_ui_sys.vertex_layout_id,
			.cache_id.raw = 0,
		};

		result = hero_pipeline_graphics_init(setup->ldev, &pipeline_setup, &hero_ui_sys.pipeline_id);
		HERO_RESULT_ASSERT(result);
	}

	return HERO_SUCCESS;
}

// ===========================================
//
//
// Utility Functions
//
//
// ===========================================

void hero_ui_area_outer_to_inner(HeroAabb* area_mut, HeroUIWidgetStyle* style) {
	area_mut->x += style->margin.left + style->padding.left + style->border_width;
	area_mut->y += style->margin.top + style->padding.top + style->border_width;
	area_mut->ex -= style->margin.right + style->padding.right + style->border_width;
	area_mut->ey -= style->margin.bottom + style->padding.bottom + style->border_width;
}

HeroUIFocusState hero_ui_focus_state(HeroUIWindow* window, HeroUIWidgetId widget_id) {
	HeroUIWidget* widget;
	HeroResult result = hero_ui_widget_get(window, widget_id, &widget);
	HERO_RESULT_ASSERT(result);
	return widget->focus_state;
}

HeroUIWidgetState hero_ui_widget_state(HeroUIWindow* window, HeroUIWidgetId widget_id) {
	HeroUIWidget* widget;
	HeroResult result = hero_ui_widget_get(window, widget_id, &widget);
	HERO_RESULT_ASSERT(result);
	return widget->state;
}

// ===========================================
//
//
// Widgets API
//
//
// ===========================================

HeroUIWidget* _hero_ui_widget_find_by_sib_id(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUIWidgetId start_id, HeroUIWidgetId end_id, HeroUIWidgetId* id_out) {
	HeroUIWidgetId sibling_id = start_id;
	while (sibling_id.raw != end_id.raw) {
		HeroUIWidget* sibling;
		HeroResult result = hero_ui_widget_get(window, sibling_id, &sibling);
		HERO_RESULT_ASSERT(result);

		if (sibling->sib_id == sib_id) {
			*id_out = sibling_id;
			return sibling;
		}

		sibling_id = sibling->sibling_next_id;
	}
	return NULL;
}

HeroUIFocusState _hero_ui_widget_update_focus_state(HeroUIWindow* window, HeroUIWidgetId widget_id) {
	//
	// keyboard state
	//

	if (hero_keyboard_key_code_is_pressed(HERO_KEY_CODE_SPACE)) {
		if (window->keyboard_focused_id.raw == widget_id.raw) {
			return HERO_UI_FOCUS_STATE_PRESSED | HERO_UI_FOCUS_STATE_HELD | HERO_UI_FOCUS_STATE_FOCUSED;
		}
	}

	if (hero_keyboard_key_code_is_pressed(HERO_KEY_CODE_SPACE)) {
		if (window->keyboard_focused_id.raw == widget_id.raw) {
			return HERO_UI_FOCUS_STATE_HELD | HERO_UI_FOCUS_STATE_FOCUSED;
		}
	}

	if (hero_keyboard_key_code_has_been_pressed(HERO_KEY_CODE_SPACE)) {
		if (window->keyboard_focused_id.raw == widget_id.raw) {
			return HERO_UI_FOCUS_STATE_RELEASED | HERO_UI_FOCUS_STATE_FOCUSED;
		}
	}

	//
	// mouse state
	//

	bool is_mouse_focused = window->mouse_focused_id.raw == widget_id.raw;
	if (hero_window_sys.mouse.buttons_has_been_pressed & HERO_MOUSE_BUTTONS_LEFT) {
		if (is_mouse_focused) {
			hero_ui_widget_keyboard_focused_set(window, widget_id);
			if (hero_window_sys.mouse.buttons_has_been_released & HERO_MOUSE_BUTTONS_LEFT) {
				return HERO_UI_FOCUS_STATE_PRESSED | HERO_UI_FOCUS_STATE_HELD | HERO_UI_FOCUS_STATE_RELEASED | HERO_UI_FOCUS_STATE_FOCUSED;
			}
			return HERO_UI_FOCUS_STATE_PRESSED | HERO_UI_FOCUS_STATE_HELD | HERO_UI_FOCUS_STATE_FOCUSED;
		} else {
			if (window->keyboard_focused_id.raw == widget_id.raw) {
				hero_ui_widget_keyboard_focused_set(window, HERO_UI_WIDGET_ID_NULL);
			}
		}
	}

	if (hero_window_sys.mouse.buttons_is_pressed & HERO_MOUSE_BUTTONS_LEFT) {
		if (window->keyboard_focused_id.raw == widget_id.raw) {
			return HERO_UI_FOCUS_STATE_HELD | HERO_UI_FOCUS_STATE_FOCUSED;
		}
	}

	if (hero_window_sys.mouse.buttons_has_been_released & HERO_MOUSE_BUTTONS_LEFT) {
		if (window->mouse_focused_id.raw == widget_id.raw) {
			return HERO_UI_FOCUS_STATE_RELEASED | HERO_UI_FOCUS_STATE_FOCUSED;
		}
	}

	if (is_mouse_focused) {
		return HERO_UI_FOCUS_STATE_FOCUSED;
	} else {
		return HERO_UI_FOCUS_STATE_NONE;
	}
}

void hero_ui_widget_next_layer_start(HeroUIWindow* window, HeroUIWidgetSibId sib_id) {
	window->build.next_widget_flags |= HERO_UI_WIDGET_FLAGS_NEXT_LAYER;
	hero_ui_box_start(window, sib_id, HERO_UI_CUT_LEFT, HERO_UI_LEN_FILL, NULL);
}

void hero_ui_widget_next_layer_end(HeroUIWindow* window) {
	hero_ui_box_end(window);
}

void hero_ui_widget_custom_offset(HeroUIWindow* window, Vec2 offset, F32 perp_cut_length, HeroUIAlign align) {
	window->build.next_widget_flags |= HERO_UI_WIDGET_FLAGS_CUSTOM_OFFSET;
	window->build.custom_offset = offset;
	window->build.custom_align = align;
	window->build.custom_perp_cut_length = perp_cut_length;
}

void hero_ui_widget_next_forced_state(HeroUIWindow* window, HeroUIWidgetState state) {
	window->build.next_widget_flags |= HERO_UI_WIDGET_FLAGS_FORCE_NEXT_STATE;
	window->build.next_forced_state = state;
}

HeroUIWidgetId hero_ui_box_start(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, F32 cut_length, HeroUIWidgetStyle* styles) {
	HeroUIWindowBuild* build = &window->build;

	HeroUIWidgetId widget_id = HERO_UI_WIDGET_ID_NULL;
	HeroUIWidget* widget = NULL;


	HeroUIWidgetId predicted_widget_id = HERO_UI_WIDGET_ID_NULL;
	if (build->sibling_prev) {
		predicted_widget_id = build->sibling_prev->sibling_next_id;
		widget = _hero_ui_widget_find_by_sib_id(window, sib_id, predicted_widget_id, HERO_UI_WIDGET_ID_NULL, &widget_id);
	}

	if (widget == NULL) {
		widget = _hero_ui_widget_find_by_sib_id(window, sib_id, build->parent->child_first_id, predicted_widget_id, &widget_id);
	}

	if (widget) {
		HERO_ASSERT(widget->last_updated_frame_idx != build->last_updated_frame_idx, "error widget sibling id '%u' has already been used this frame", sib_id);
		widget->flags &= ~HERO_UI_WIDGET_FLAGS_CREATED_THIS_FRAME;

		if (build->sibling_prev_id.raw != widget->sibling_prev_id.raw) {
			_hero_ui_widget_tree_remove(window, widget, widget_id);
		}
	} else {
		//
		// widget was not in the tree, so allocated a new widget.
		//
		HeroResult result = _hero_ui_widget_alloc(window, &widget, &widget_id);
		HERO_RESULT_ASSERT(result);

		widget->sib_id = sib_id;
		widget->flags |= HERO_UI_WIDGET_FLAGS_CREATED_THIS_FRAME;
		widget->render_fn = hero_ui_box_render;
	}

	//
	// if widget was allocated this frame or the existing widget
	// has a different previous sibling, then re/insert it into
	// the correct location based on the current state of the build.
	if (widget->parent_id.raw == HERO_UI_WIDGET_ID_NULL.raw) {
		_hero_ui_widget_tree_insert(window, widget, widget_id);
	}

	//
	// make fill or ratios length widgets inherit auto sizing if the parent is auto.
	// if fill length then let the parent widget know of this widget.
	HeroUIWidget* parent = build->parent;
	if (cut_length == HERO_UI_LEN_FILL || HERO_UI_LEN_IS_RATIO(cut_length)) {
		if (parent->cut_length == HERO_UI_LEN_AUTO && !(hero_ui_cut_is_horizontal(parent->cut) != hero_ui_cut_is_horizontal(cut))) {
			cut_length = HERO_UI_LEN_AUTO;
		} else if (cut_length == HERO_UI_LEN_FILL) {
			parent->fill_proportion_fill_children_count += 1;
		}
	}

	HERO_ASSERT(!(parent->flags & HERO_UI_WIDGET_FLAGS_CHILD_HAS_CUT_CENTER), "there may only be a single child widget with a cut length HERO_UI_CUT_CENTER_* 0x%x");
	if (hero_ui_cut_is_center(cut)) {
		parent->flags |= HERO_UI_WIDGET_FLAGS_CHILD_HAS_CUT_CENTER;
	}

	//
	// keep widget data updated and reset the data that is recalculated every frame.
	_hero_ui_widget_frame_reset(widget);
	widget->styles = styles;
	widget->last_updated_frame_idx = build->last_updated_frame_idx;
	widget->cut = cut;
	widget->cut_length = cut_length;
	widget->flags |= window->build.next_widget_flags;
	window->build.next_widget_flags = 0;
	widget->auto_perp_cut_length = 0.f;

	if (widget->flags & HERO_UI_WIDGET_FLAGS_CUSTOM_OFFSET) {
		widget->custom_offset = build->custom_offset;
		widget->custom_align = build->custom_align;
		widget->custom_perp_cut_length = build->custom_perp_cut_length;
	}

	//
	// make widget focusable or not and can change on the fly.
	if (widget->flags & HERO_UI_WIDGET_FLAGS_IS_FOCUSABLE) {
		if (widget->flags & HERO_UI_WIDGET_FLAGS_IS_FOCUSABLE) {
			widget->focus_state = _hero_ui_widget_update_focus_state(window, widget_id);
		}
	} else {
		widget->flags &= ~HERO_UI_WIDGET_FLAGS_IS_FOCUSABLE;
		if (window->mouse_focused_id.raw == widget_id.raw) {
			HeroResult result = hero_ui_widget_mouse_focused_set(window, HERO_UI_WIDGET_ID_NULL);
			HERO_RESULT_ASSERT(result);
		}
		if (window->keyboard_focused_id.raw == widget_id.raw) {
			HeroResult result = hero_ui_widget_keyboard_focused_set(window, HERO_UI_WIDGET_ID_NULL);
			HERO_RESULT_ASSERT(result);
		}
	}

	//
	// work out the widget's state.
	if (window->mouse_focused_id.raw == widget_id.raw || window->keyboard_focused_id.raw == widget_id.raw) {
		//
		// handle pressable and toggleable controls active state.
		// pressable means it is active when held down.
		// selectable means it's active state at the moment of being pressed.
		// toggleable means it's active state is toggled with every press.
		if (widget->flags & HERO_UI_WIDGET_FLAGS_IS_PRESSABLE) {
			if (widget->focus_state & HERO_UI_FOCUS_STATE_HELD) {
				widget->state = HERO_UI_WIDGET_STATE_ACTIVE;
			} else {
				widget->state = HERO_UI_WIDGET_STATE_FOCUSED;
			}
		} else if (widget->flags & HERO_UI_WIDGET_FLAGS_IS_SELECTABLE) {
			if (widget->focus_state & HERO_UI_FOCUS_STATE_PRESSED) {
				widget->state = HERO_UI_WIDGET_STATE_ACTIVE;
			}
		} else if (widget->flags & HERO_UI_WIDGET_FLAGS_IS_TOGGLEABLE) {
			if (widget->focus_state & HERO_UI_FOCUS_STATE_PRESSED) {
				if (widget->state == HERO_UI_WIDGET_STATE_ACTIVE) {
					widget->state = HERO_UI_WIDGET_STATE_FOCUSED;
				} else {
					widget->state = HERO_UI_WIDGET_STATE_ACTIVE;
				}
			}
		}
	} else if (widget->state == HERO_UI_WIDGET_STATE_FOCUSED) {
		widget->state = HERO_UI_WIDGET_STATE_DEFAULT;
	}

	if (widget->flags & HERO_UI_WIDGET_FLAGS_FORCE_NEXT_STATE) {
		widget->state = build->next_forced_state;
	}

	//
	// if the parent has an automatic length or the widget is fill or ratio length.
	//     ensure that we are soley a cutting along an single axis to make calculating these things possible.
	if (parent->cut_length == HERO_UI_LEN_AUTO || cut_length == HERO_UI_LEN_FILL || HERO_UI_LEN_IS_RATIO(cut_length)) {
		HeroUIWidgetFlags cut_both = HERO_UI_WIDGET_FLAGS_CHILD_HAS_CUT_VERTICAL | HERO_UI_WIDGET_FLAGS_CHILD_HAS_CUT_HORIZONTAL;
		HERO_ASSERT( // this assert only fail for fill and ratio lengths when previous sibling widgets have already cut both axises
			(parent->flags & cut_both) != cut_both,
			"all siblings of a fill or ratio length widget must use the same cut axis but both x and y axis have been used"
		);
		HERO_ASSERT(
			!(parent->flags & HERO_UI_WIDGET_FLAGS_CHILD_HAS_CUT_VERTICAL) || hero_ui_cut_is_vertical(cut),
			parent->cut_length == HERO_UI_LEN_AUTO
				? "all children of an auto length widget must use the same cut axis, expected them all to be vertical"
				: "all siblings of a fill or ratio length widget must use the same cut axis, expected them all to be vertical"
		);
		HERO_ASSERT(
			!(parent->flags & HERO_UI_WIDGET_FLAGS_CHILD_HAS_CUT_HORIZONTAL) || hero_ui_cut_is_horizontal(cut),
			parent->cut_length == HERO_UI_LEN_AUTO
				? "all children of an auto length widget must use the same cut axis, expected them all to be horizontal"
				: "all siblings of a fill or ratio length widget must use the same cut axis, expected them all to be horizontal"
		);
	}

	//
	// keep track of the axises that we are cutting down.
	if (hero_ui_cut_is_horizontal(cut)) {
		parent->flags |= HERO_UI_WIDGET_FLAGS_CHILD_HAS_CUT_HORIZONTAL;
	} else if (hero_ui_cut_is_vertical(cut)) {
		parent->flags |= HERO_UI_WIDGET_FLAGS_CHILD_HAS_CUT_VERTICAL;
	}

	//
	// setup the build state so this widget is now the parent
	// and future widgets will become the children of this one.
	build->parent = widget;
	build->parent_id = widget_id;
	build->sibling_prev = NULL;
	build->sibling_prev_id = HERO_UI_WIDGET_ID_NULL;

	return widget_id;
}

void hero_ui_box_end(HeroUIWindow* window) {
	HeroUIWindowBuild* build = &window->build;
	HeroUIWidget* widget = build->parent;
	HeroUIWidgetStyle* style = hero_ui_widget_style(widget);

	//
	// if the widget has an automatic length, use the sum or maximum length that we calcuated from our children.
	if (widget->cut_length == HERO_UI_LEN_AUTO) {
		widget->cut_length = widget->auto_children_length + (style->border_width * 2.f);
		if (hero_ui_cut_is_horizontal(widget->cut)) {
			widget->cut_length += style->padding.left + style->padding.right;
		} else if (hero_ui_cut_is_vertical(widget->cut)) {
			widget->cut_length += style->padding.top + style->padding.bottom;
		}
	}

	//
	// because we are ending a widget, go back up the tree to the parent
	// and make the current widget the previous sibling in the build state.
	{
		build->sibling_prev_id = build->parent_id;
		build->sibling_prev = build->parent;

		build->parent_id = build->parent->parent_id;
		HeroResult result = hero_ui_widget_get(window, build->parent->parent_id, &build->parent);
		HERO_RESULT_ASSERT(result);
	}

	//
	// add the rest of the inner padding for the auto perpendicular cut length.
	if (hero_ui_cut_is_horizontal(widget->cut)) {
		widget->auto_perp_cut_length += style->padding.top + style->padding.bottom;
	} else if (hero_ui_cut_is_vertical(widget->cut)) {
		widget->auto_perp_cut_length += style->padding.left + style->padding.right;
	}
	widget->auto_perp_cut_length += style->border_width * 2.f;

	//
	// help calculate the auto perpendicular cut length for the parent.
	HeroUIWidget* parent = build->parent;
	bool is_same_axis = hero_ui_cut_is_vertical(parent->cut) == hero_ui_cut_is_vertical(widget->cut);
	if (is_same_axis) {
		parent->auto_perp_cut_length = HERO_MAX(parent->auto_perp_cut_length, widget->auto_perp_cut_length);
	} else {
		float length = widget->cut_length;
		if (hero_ui_cut_is_horizontal(widget->cut)) {
			length += style->margin.left + style->margin.right;
		} else if (hero_ui_cut_is_vertical(widget->cut)) {
			length += style->margin.top + style->margin.bottom;
		}
		parent->auto_perp_cut_length += length;
	}

	//
	// if the parent is automatic, add the widget's cut length to the parent's sum or maximum length value.
	if (parent->cut_length == HERO_UI_LEN_AUTO) {
		F32 child_outer_length = widget->cut_length;
		if (is_same_axis) {
			child_outer_length = widget->cut_length;
		} else {
			child_outer_length = widget->auto_perp_cut_length;
		}

		if (hero_ui_cut_is_horizontal(parent->cut)) {
			child_outer_length += style->margin.left + style->margin.right;
		} else if (hero_ui_cut_is_vertical(parent->cut)) {
			child_outer_length += style->margin.top + style->margin.bottom;
		}

		//
		// if the parent is cutting along the same axis as it's children,
		//     then we want to keep a sum of the outer lengths of it's children.
		// else
		//     then we want to keep a maximum outer length of all of it's children.
		if (is_same_axis) {
			parent->auto_children_length += child_outer_length;
		} else {
			parent->auto_children_length = HERO_MAX(parent->auto_children_length, child_outer_length);
		}
	}

	//
	// if the widget has a ratio length, work out the length here.
	// we do this by multipling our ratio with the inner length of
	// the parent and subtracting the margin of the widget.
	if (HERO_UI_LEN_IS_RATIO(widget->cut_length)) {
		HeroUIWidgetStyle* parent_style = hero_ui_widget_style(parent);

		F32 parent_inner_length = 0.f;
		if (hero_ui_cut_is_horizontal(widget->cut)) {
			parent_inner_length = parent->cut_length - (parent_style->padding.left + parent_style->padding.right + (parent_style->border_width * 2.f));
		} else if (hero_ui_cut_is_vertical(widget->cut)) {
			parent_inner_length = parent->cut_length - (parent_style->padding.top + parent_style->padding.bottom + (parent_style->border_width * 2.f));
		}
		parent_inner_length = HERO_MAX(0.f, parent_inner_length);
		widget->cut_length = parent_inner_length * HERO_UI_LEN_RATIO_GET(widget->cut_length);

		if (hero_ui_cut_is_horizontal(widget->cut)) {
			widget->cut_length -= style->margin.left + style->margin.right;
		} else if (hero_ui_cut_is_vertical(widget->cut)) {
			widget->cut_length -= style->margin.top + style->margin.bottom;
		}
	}

	//
	// for parents with fill proportion length.
	// keep track of the sum of all the fixed length child widgets.
	// keep in mind that a widget with a ratio length would have
	// become a fixed length by now.
	if (HERO_UI_LEN_IS_FIXED(widget->cut_length)) {
		parent->fill_proportion_fixed_children_length += widget->cut_length;
	}

	//
	// for parents with fill proportion length.
	// we also need to have the margin's of all child widgets kept track of too.
	if (hero_ui_cut_is_horizontal(widget->cut)) {
		parent->fill_proportion_fixed_children_length += style->margin.left + style->margin.right;
	} else if (hero_ui_cut_is_vertical(widget->cut)) {
		parent->fill_proportion_fixed_children_length += style->margin.top + style->margin.bottom;
	}
}

HeroUIWidgetId hero_ui_box(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, F32 cut_length, HeroUIWidgetStyle* styles) {
	HeroUIWidgetId widget_id = hero_ui_box_start(window, sib_id, cut, cut_length, styles);
	hero_ui_box_end(window);
	return widget_id;
}

HeroResult hero_ui_box_render(HeroUIWindow* window, HeroUIWidget* widget) {
	HeroResult result;
	HeroAabb area = widget->area;
	HeroUIWidgetStyle* style = hero_ui_widget_style(widget);

	bool has_background = hero_color_a(style->background_color);
	bool has_border = hero_color_a(style->border_color) && style->border_width > 0.f;
	if (has_background || has_border) {
		area.x += style->margin.left;
		area.y += style->margin.top;
		area.ex -= style->margin.right;
		area.ey -= style->margin.bottom;

		if (has_border) {
			result = hero_ui_widget_draw_aabb_border(window, &area, style->radius, style->border_color, style->border_width);
			if (result < 0) {
				return result;
			}

			area.x += style->border_width;
			area.y += style->border_width;
			area.ex -= style->border_width;
			area.ey -= style->border_width;
		}

		result = hero_ui_widget_draw_aabb(window, &area, style->radius, style->background_color);
		if (result < 0) {
			return result;
		}

		if (has_border) {
			area.x -= style->border_width;
			area.y -= style->border_width;
			area.ex += style->border_width;
			area.ey += style->border_width;
		}
	}

	return HERO_SUCCESS;
}

HeroUIWidgetId _hero_ui_image(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, bool is_grayscale, HeroUIImageId image_id, HeroColor image_bg_color, HeroColor image_fg_color, F32 scale, HeroUIImageScaleMode scale_mode, HeroUIImageFlip flip, HeroUIWidgetStyle* styles) {
	HeroUIWidgetId widget_id = hero_ui_box_start(window, sib_id, cut, HERO_UI_LEN_AUTO, styles);

	HeroUIWidget* widget;
	HeroResult result = hero_ui_widget_get(window, widget_id, &widget);
	HERO_RESULT_ASSERT(result);

	//
	// set the size of the image widget
	Vec2 size;
	{
		HeroResult result = hero_ui_image_atlas_image_size(image_id.atlas_id, image_id.image_idx, &size);
		HERO_RESULT_ASSERT(result);
		size = vec2_mul_scalar(size, scale);

		HeroUIWidgetStyle* style = hero_ui_widget_style(widget);
		if (hero_ui_cut_is_horizontal(widget->cut)) {
			widget->cut_length = size.x + style->padding.left + style->padding.right;
			widget->auto_perp_cut_length = size.y;
		} else if (hero_ui_cut_is_vertical(widget->cut)) {
			widget->cut_length = size.y + style->padding.top + style->padding.bottom;
			widget->auto_perp_cut_length = size.x;
		}
		widget->cut_length += style->border_width * 2.f;
	}

	widget->render_fn = hero_ui_image_render;
	widget->image_id = image_id;
	widget->image_bg_color = image_bg_color;
	widget->image_fg_color = image_fg_color;
	widget->image_grayscale = is_grayscale;
	widget->image_scale_mode = scale_mode;
	widget->image_flip = flip;
	widget->image_size = size;

	hero_ui_box_end(window);
	return widget_id;
}

HeroUIWidgetId hero_ui_image(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, HeroUIImageId image_id, HeroColor image_tint, F32 scale, HeroUIImageScaleMode scale_mode, HeroUIImageFlip flip, HeroUIWidgetStyle* styles) {
	return _hero_ui_image(window, sib_id, cut, false, image_id, image_tint, 0, scale, scale_mode, flip, styles);
}

HeroUIWidgetId hero_ui_image_grayscale(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, HeroUIImageId image_id, HeroColor bg_color, HeroColor fg_color, F32 scale, HeroUIImageScaleMode scale_mode, HeroUIImageFlip flip, HeroUIWidgetStyle* styles) {
	return _hero_ui_image(window, sib_id, cut, true, image_id, bg_color, fg_color, scale, scale_mode, flip, styles);
}

HeroResult hero_ui_image_render(HeroUIWindow* window, HeroUIWidget* widget) {
	HeroResult result = hero_ui_box_render(window, widget);
	if (result < 0) {
		return result;
	}

	HeroAabb area = widget->area;
	HeroUIWidgetStyle* style = hero_ui_widget_style(widget);
	hero_ui_area_outer_to_inner(&area, style);

	F32 image_width = widget->image_size.x;
	F32 image_height = widget->image_size.y;
	switch (widget->image_scale_mode) {
		case HERO_UI_IMAGE_SCALE_MODE_STRETCH: {
			// do nothing, the size of the area will stretch the image by default.
			break;
		};
		case HERO_UI_IMAGE_SCALE_MODE_UNIFORM:
		case HERO_UI_IMAGE_SCALE_MODE_UNIFORM_CROP: {
			F32 width = hero_aabb_width(&area);
			F32 height = hero_aabb_height(&area);

			bool cond = image_width >= image_height;
			if (widget->image_scale_mode == HERO_UI_IMAGE_SCALE_MODE_UNIFORM_CROP) {
				cond = !cond;
			}

			if (cond) {
				//
				// width is larger than the height. (unless uniform_crop then this is the opposite way round)
				// so the width will take up the whole width of the area passed in.
				// so scale the height of the image to maintain the aspect ratio.
				F32 image_aspect_ratio = image_height / image_width;
				F32 scale_factor_width = width / image_width;
				F32 scale_factor_height = scale_factor_width * image_aspect_ratio;
				area.ey = area.y + image_height * scale_factor_height;
			} else {
				//
				// height is larger than the width. (unless uniform_crop then this is the opposite way round)
				// so the height will take up the whole height of the area passed in.
				// so scale the width of the image to maintain the aspect ratio.
				F32 image_aspect_ratio = image_width / image_height;
				F32 scale_factor_height = height / image_height;
				F32 scale_factor_width = scale_factor_height * image_aspect_ratio;
				area.ex = area.x + image_width * scale_factor_width;
			}
			break;
		};
		case HERO_UI_IMAGE_SCALE_MODE_NONE:
			area.ex = area.x + image_width;
			area.ey = area.y + image_height;
			break;
	}

	if (widget->image_flip & HERO_UI_IMAGE_FLIP_X) {
		F32 tmp = area.x;
		area.x = area.ex;
		area.ex = tmp;
	}

	if (widget->image_flip & HERO_UI_IMAGE_FLIP_Y) {
		F32 tmp = area.y;
		area.y = area.ey;
		area.ey = tmp;
	}

	if (widget->image_grayscale) {
		result = hero_ui_widget_draw_image_grayscale_remap(window, &area, widget->image_id, widget->image_bg_color, widget->image_fg_color);
		if (result < 0) {
			return result;
		}
	} else {
		result = hero_ui_widget_draw_image(window, &area, widget->image_id, widget->image_bg_color);
		if (result < 0) {
			return result;
		}
	}

	return HERO_SUCCESS;
}

HeroUIWidgetId hero_ui_text(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, F32 cut_length, HeroString string, HeroUIWidgetStyle* styles) {
	HeroUIWidgetId widget_id = hero_ui_box_start(window, sib_id, cut, cut_length, styles);

	HeroUIWidget* widget = window->build.parent;
	HeroUIWidgetStyle* style = hero_ui_widget_style(widget);

	F32 wrap_at_width = 0;
	if (style->text_wrap) {
		wrap_at_width = (widget->area.ex - widget->area.x) - style->margin.left - style->margin.right - style->padding.left - style->padding.right - (style->border_width * 2.f);
	}

	//
	// if the widget has automatic length, measure the text and apply the padding and border lengths.
	if (widget->cut_length == HERO_UI_LEN_AUTO) {
		Vec2 size = hero_ui_sys.text_size_fn(window, string, wrap_at_width, style);
		if (hero_ui_cut_is_horizontal(widget->cut)) {
			HERO_ASSERT(!style->text_wrap, "cannot wrap text with an automatic cut length with a horizontal cut");
			widget->cut_length = size.x + style->padding.left + style->padding.right;
			widget->auto_perp_cut_length = size.y;
		} else if (hero_ui_cut_is_vertical(widget->cut)) {
			widget->cut_length = size.y + style->padding.top + style->padding.bottom;
			widget->auto_perp_cut_length = size.x;
		}
		widget->cut_length += style->border_width * 2.f;
	} else if (style->text_wrap) {
		Vec2 size = hero_ui_sys.text_size_fn(window, string, wrap_at_width, style);
		if (hero_ui_cut_is_horizontal(widget->cut)) {
			widget->auto_perp_cut_length = size.y;
		} else if (hero_ui_cut_is_vertical(widget->cut)) {
			widget->auto_perp_cut_length = size.x;
		}
	}

	widget->render_fn = hero_ui_text_render;
	widget->string = string;

	hero_ui_box_end(window);
	return widget_id;
}

HeroResult hero_ui_text_render(HeroUIWindow* window, HeroUIWidget* widget) {
	HeroResult result = hero_ui_box_render(window, widget);
	if (result < 0) {
		return result;
	}

	HeroAabb area = widget->area;
	HeroUIWidgetStyle* style = hero_ui_widget_style(widget);
	hero_ui_area_outer_to_inner(&area, style);

	Vec2 top_left = VEC2_INIT(area.x, area.y);
	F32 wrap_at_width = hero_aabb_width(&area);
	return hero_ui_sys.text_render_fn(window, top_left, widget->string, wrap_at_width, style);
}

HeroUIWidgetId hero_ui_button_start(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, F32 cut_length, HeroUIWidgetStyle* styles) {
	window->build.next_widget_flags |= HERO_UI_WIDGET_FLAGS_IS_FOCUSABLE | HERO_UI_WIDGET_FLAGS_IS_PRESSABLE;
	return hero_ui_box_start(window, sib_id, cut, cut_length, styles);
}

void hero_ui_button_end(HeroUIWindow* window) {
	hero_ui_box_end(window);
}

HeroUIWidgetId hero_ui_text_button(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, F32 cut_length, HeroString string, HeroUIWidgetStyle* styles) {
	window->build.next_widget_flags |= HERO_UI_WIDGET_FLAGS_IS_FOCUSABLE | HERO_UI_WIDGET_FLAGS_IS_PRESSABLE;
	return hero_ui_text(window, sib_id, cut, cut_length, string, styles);
}

HeroUIWidgetId hero_ui_text_toggle_button(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, F32 cut_length, HeroString string, HeroUIWidgetStyle* styles) {
	window->build.next_widget_flags |= HERO_UI_WIDGET_FLAGS_IS_FOCUSABLE | HERO_UI_WIDGET_FLAGS_IS_TOGGLEABLE;
	return hero_ui_text(window, sib_id, cut, cut_length, string, styles);
}

HeroUIWidgetId hero_ui_image_button(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, F32 cut_length, HeroUIImageId image_id, HeroColor image_tint, F32 scale, HeroUIImageScaleMode scale_mode, HeroUIImageFlip flip, HeroUIWidgetStyle* styles) {
	window->build.next_widget_flags |= HERO_UI_WIDGET_FLAGS_IS_FOCUSABLE | HERO_UI_WIDGET_FLAGS_IS_PRESSABLE;
	return hero_ui_image(window, sib_id, cut, image_id, image_tint, scale, scale_mode, flip, styles);
}

HeroUIWidgetId hero_ui_image_grayscale_button(HeroUIWindow* window, HeroUIWidgetSibId sib_id, HeroUICut cut, F32 cut_length, HeroUIImageId image_id, HeroColor bg_color, HeroColor fg_color, F32 scale, HeroUIImageScaleMode scale_mode, HeroUIImageFlip flip, HeroUIWidgetStyle* styles) {
	window->build.next_widget_flags |= HERO_UI_WIDGET_FLAGS_IS_FOCUSABLE | HERO_UI_WIDGET_FLAGS_IS_PRESSABLE;
	return hero_ui_image_grayscale(window, sib_id, cut, image_id, bg_color, fg_color, scale, scale_mode, flip, styles);
}

