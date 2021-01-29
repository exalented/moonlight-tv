#include "backend/computer_manager.h"
#include <SDL.h>

static Uint32 _auto_discovery_cb(Uint32 interval, void *param);
static SDL_TimerID _auto_discovery_timer;

void computer_manager_auto_discovery_start()
{
    _auto_discovery_timer = SDL_AddTimer(30000, _auto_discovery_cb, NULL);
}

void computer_manager_auto_discovery_stop()
{
    SDL_RemoveTimer(_auto_discovery_timer);
}

Uint32 _auto_discovery_cb(Uint32 interval, void *param)
{
    computer_manager_run_scan();
    return interval;
}