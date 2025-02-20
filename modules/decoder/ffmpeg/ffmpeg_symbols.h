#pragma once

#include "stream/module/api.h"

extern VIDEO_RENDER_CALLBACKS render_callbacks_ffmpeg;

extern DECODER_RENDERER_CALLBACKS decoder_callbacks_ffmpeg;

bool decoder_init_ffmpeg(int argc, char *argv[], const HOST_CONTEXT *hctx);

bool decoder_check_ffmpeg(PDECODER_INFO info);