#include <stdio.h>

#include "ui/config.h"

#include "nuklear/config.h"
#include "nuklear.h"
#include "nuklear/ext_widgets.h"
#include "nuklear/ext_functions.h"
#include "nuklear/ext_styling.h"

#if defined(NK_SDL_GLES2)
#include "nuklear/platform_sdl_gles2.h"
#elif defined(NK_SDL_GL2)
#include "nuklear/platform_sdl_gl2.h"
#elif defined(NK_LGNC_GLES2)
#include "nuklear/platform_lgnc_gles2.h"
#else
#error "No valid render backend specified"
#endif

#ifdef OS_WEBOS
#include "platform/sdl/webos_keys.h"
#endif

#include "main.h"
#include "app.h"
#include "debughelper.h"
#include "backend/backend_root.h"
#include "stream/session.h"
#include "ui/gui_root.h"
#include "util/bus.h"

bool running = true;

int main(int argc, char *argv[])
{
#ifdef OS_WEBOS
    REDIR_STDOUT("moonlight");
#endif
    bus_init();

    int ret = app_init(argc, argv);
    if (ret != 0)
    {
        return ret;
    }

    backend_init();

    /* GUI */
    struct nk_context *ctx;
    APP_WINDOW_CONTEXT win = app_window_create();
    streaming_display_size(WINDOW_WIDTH, WINDOW_HEIGHT);
    gui_display_size(WINDOW_WIDTH, WINDOW_HEIGHT);

    ctx = nk_platform_init(win);
    {
        struct nk_font_atlas *atlas;
        nk_platform_font_stash_begin(&atlas);
        struct nk_font *noto = nk_font_atlas_add_from_file_s(atlas, "assets/NotoSans-Regular.ttf", 20, NULL);
        nk_platform_font_stash_end();
        nk_style_set_font(ctx, &noto->handle);
    }
    nk_ext_apply_styles(ctx);

    gui_root_init(ctx);

    while (running)
    {
        app_main_loop((void *)ctx);
    }

    nk_platform_shutdown();
    backend_destroy();
    app_destroy();
    bus_destroy();
    return 0;
}

void request_exit()
{
    running = false;
}