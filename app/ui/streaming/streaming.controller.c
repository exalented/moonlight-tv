#include <util/user_event.h>
#include <ui/root.h>
#include <stream/video/delegate.h>
#include <stream/platform.h>
#include "streaming.controller.h"

lv_obj_t *progress_dialog_create(const char *message);

static void exit_streaming(lv_event_t *event);

static void suspend_streaming(lv_event_t *event);

static void hide_overlay(lv_event_t *event);

static bool show_overlay(streaming_controller_t *controller);

static void on_view_created(lv_obj_controller_t *self, lv_obj_t *view);

static bool on_event(lv_obj_controller_t *, int, void *, void *);

static void streaming_controller_ctor(lv_obj_controller_t *self, void *args);

static void controller_dtor(lv_obj_controller_t *self);

static void session_error(streaming_controller_t *controller);

static void session_error_dialog_cb(lv_event_t *event);

const lv_obj_controller_class_t streaming_controller_class = {
        .constructor_cb = streaming_controller_ctor,
        .destructor_cb = controller_dtor,
        .create_obj_cb = streaming_scene_create,
        .obj_created_cb = on_view_created,
        .event_cb = on_event,
        .instance_size = sizeof(streaming_controller_t),
};

static bool overlay_showing;
static streaming_controller_t *current_controller = NULL;

bool streaming_overlay_shown() {
    return overlay_showing;
}

bool streaming_refresh_stats() {
    streaming_controller_t *controller = current_controller;
    if (!controller) return false;
    if (!streaming_overlay_shown()) {
        return false;
    }
    lv_label_set_text_fmt(controller->stats_items.decoder, "%s (%s)", decoder_definitions[decoder_current].name,
                          vdec_stream_info.format);
    if (audio_current == AUDIO_DECODER) {
        lv_label_set_text_static(controller->stats_items.audio, "Use decoder");
    } else {
        lv_label_set_text_static(controller->stats_items.audio, audio_definitions[audio_current].name);
    }
    const struct VIDEO_STATS *dst = &vdec_summary_stats;
    lv_label_set_text_fmt(controller->stats_items.rtt, "%d ms (var. %d ms)", dst->rtt, dst->rttVariance);
    lv_label_set_text_fmt(controller->stats_items.net_fps, "%.2f FPS", dst->receivedFps);

    if (dst->decodedFrames) {
        lv_label_set_text_fmt(controller->stats_items.drop_rate, "%.2f%%",
                              (float) dst->networkDroppedFrames / dst->totalFrames * 100);
        lv_label_set_text_fmt(controller->stats_items.decode_time, "%.2f ms",
                              (float) dst->totalDecodeTime / dst->decodedFrames);
    } else {
        lv_label_set_text(controller->stats_items.drop_rate, "-");
        lv_label_set_text_fmt(controller->stats_items.decode_time, "-");
    }
    return true;
}

static void streaming_controller_ctor(lv_obj_controller_t *self, void *args) {
    streaming_controller_t *controller = (streaming_controller_t *) self;
    LV_ASSERT(current_controller == NULL);
    current_controller = controller;
    const streaming_scene_arg_t *req = (streaming_scene_arg_t *) args;
    streaming_begin(req->server, req->app);

    overlay_showing = false;
}

static void controller_dtor(lv_obj_controller_t *self) {
    current_controller = NULL;
}

static bool on_event(lv_obj_controller_t *self, int which, void *data1, void *data2) {
    streaming_controller_t *controller = (streaming_controller_t *) self;
    switch (which) {
        case USER_STREAM_CONNECTING: {
            controller->progress = progress_dialog_create("Starting session.");
            lv_obj_add_flag(controller->base.obj, LV_OBJ_FLAG_HIDDEN);
            return true;
        }
        case USER_STREAM_OPEN: {
            if (controller->progress) {
                lv_msgbox_close(controller->progress);
                controller->progress = NULL;
            }
            lv_obj_add_flag(controller->base.obj, LV_OBJ_FLAG_HIDDEN);
            break;
        }
        case USER_STREAM_CLOSE: {
            controller->progress = progress_dialog_create("Disconnecting.");
            lv_obj_add_flag(controller->base.obj, LV_OBJ_FLAG_HIDDEN);
            break;
        }
        case USER_STREAM_FINISHED: {
            if (controller->progress) {
                lv_msgbox_close(controller->progress);
                controller->progress = NULL;
            }
            lv_obj_add_flag(controller->base.obj, LV_OBJ_FLAG_HIDDEN);
            if (streaming_errno != 0) {
                session_error(controller);
                break;
            }
            lv_obj_controller_pop((lv_obj_controller_t *) controller);
            return true;
        }
        case USER_OPEN_OVERLAY: {
            show_overlay(controller);
            return true;
        }
        default: {
            break;
        }
    }
    return false;
}

static void on_view_created(lv_obj_controller_t *self, lv_obj_t *view) {
    streaming_controller_t *controller = (streaming_controller_t *) self;
    lv_obj_add_event_cb(controller->quit_btn, exit_streaming, LV_EVENT_CLICKED, self);
    lv_obj_add_event_cb(controller->suspend_btn, suspend_streaming, LV_EVENT_CLICKED, self);
    lv_obj_add_event_cb(controller->base.obj, hide_overlay, LV_EVENT_CLICKED, self);
}

static void exit_streaming(lv_event_t *event) {
    streaming_interrupt(true);
}

static void suspend_streaming(lv_event_t *event) {
    streaming_interrupt(false);
}

bool show_overlay(streaming_controller_t *controller) {
    if (overlay_showing)
        return false;
    overlay_showing = true;
    lv_obj_clear_flag(controller->base.obj, LV_OBJ_FLAG_HIDDEN);

    lv_area_t coords = controller->video->coords;
    streaming_enter_overlay(coords.x1, coords.y1, lv_area_get_width(&coords), lv_area_get_height(&coords));
    streaming_refresh_stats();
    return true;
}

static void hide_overlay(lv_event_t *event) {
    streaming_controller_t *controller = (streaming_controller_t *) lv_event_get_user_data(event);
    lv_obj_add_flag(controller->base.obj, LV_OBJ_FLAG_HIDDEN);
    if (!overlay_showing)
        return;
    overlay_showing = false;
    streaming_enter_fullscreen();
}

static void session_error(streaming_controller_t *controller) {
    static const char *btn_texts[] = {"OK", ""};
    lv_obj_t *dialog = lv_msgbox_create(NULL, "Failed to start session", streaming_errmsg, btn_texts,
                                        false);
    lv_obj_add_event_cb(dialog, session_error_dialog_cb, LV_EVENT_VALUE_CHANGED, controller);
    lv_obj_center(dialog);
}

static void session_error_dialog_cb(lv_event_t *event) {
    streaming_controller_t *controller = lv_event_get_user_data(event);
    lv_obj_t *dialog = lv_event_get_current_target(event);
    lv_msgbox_close_async(dialog);
    lv_obj_controller_pop((lv_obj_controller_t *) controller);
}
