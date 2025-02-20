#pragma once

#include <lvgl.h>

#include "client.h"
#include "stream/session.h"

typedef struct {
    lv_obj_controller_t base;
    lv_obj_t *scene;
    lv_group_t *group;
    lv_obj_t *progress;
    lv_obj_t *video;
    lv_obj_t *kbd_btn, *vmouse_btn;
    lv_obj_t *suspend_btn, *quit_btn;
    lv_obj_t *stats;
    struct {
        lv_obj_t *resolution;
        lv_obj_t *decoder;
        lv_obj_t *audio;
        lv_obj_t *rtt;
        lv_obj_t *net_fps;
        lv_obj_t *drop_rate;
        lv_obj_t *decode_time;
    } stats_items;
    lv_obj_t *notice, *notice_label;
    lv_style_t overlay_button_style;
    lv_style_t overlay_button_label_style;
    lv_point_t button_points[5];
} streaming_controller_t;

typedef struct {
    const SERVER_DATA *server;
    const APP_LIST *app;
} streaming_scene_arg_t;

extern const lv_obj_controller_class_t streaming_controller_class;

lv_obj_t *streaming_scene_create(lv_obj_controller_t *self, lv_obj_t *parent);

void streaming_styles_init(streaming_controller_t *controller);

void streaming_overlay_resized(streaming_controller_t *controller);

bool streaming_overlay_shown();

bool streaming_refresh_stats();

void streaming_notice_show(const char *message);