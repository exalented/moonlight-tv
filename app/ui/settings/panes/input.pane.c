#include "app.h"

#include "pref_obj.h"

#include "util/i18n.h"

static lv_obj_t *create_obj(lv_obj_controller_t *self, lv_obj_t *parent);

static void pane_ctor(lv_obj_controller_t *self, void *args);

const lv_obj_controller_class_t settings_pane_input_cls = {
        .constructor_cb = pane_ctor,
        .create_obj_cb = create_obj,
        .instance_size = sizeof(lv_obj_controller_t),
};

static void pane_ctor(lv_obj_controller_t *self, void *args) {

}

static lv_obj_t *create_obj(lv_obj_controller_t *self, lv_obj_t *parent) {
    lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    pref_checkbox(parent, locstr("View-only mode"), &app_configuration->viewonly, false);
    pref_desc_label(parent, locstr("Don't send mouse, keyboard or gamepad input to host computer."), false);

    pref_checkbox(parent, locstr("Absolute mouse mode"), &app_configuration->absmouse, false);
    pref_desc_label(parent, locstr("Better for remote desktop. For some games, mouse will not work properly."), false);

    pref_checkbox(parent, locstr("Swap ABXY buttons"), &app_configuration->swap_abxy, false);
    pref_desc_label(parent, locstr("Swap A/B and X/Y gamepad buttons. Useful when you prefer Nintendo-like layouts."), false);
    return NULL;
}