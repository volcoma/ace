#include "events.h"

namespace ace
{
void events::toggle_play_mode(rtti::context& ctx)
{
    set_play_mode(ctx, !is_playing);
}

void events::set_play_mode(rtti::context& ctx, bool play)
{
    if(is_playing == play)
    {
        return;
    }

    is_playing = play;

    if (!is_playing)
    {
        if(is_paused)
        {
            set_paused(ctx, false);
        }
    }


    is_playing ? on_play_begin(ctx) : on_play_end(ctx);
}

void events::toggle_pause(rtti::context& ctx)
{
    set_paused(ctx, !is_paused);
}

void events::set_paused(rtti::context& ctx, bool paused)
{
    if(paused && !is_playing)
    {
        return;
    }

    if(is_paused == paused)
    {
        return;
    }

    is_paused = paused;
    is_paused ? on_pause(ctx) : on_resume(ctx);
}

void events::skip_next_frame(rtti::context& ctx)
{
    if(!is_playing)
    {
        return;
    }

    if(!is_paused)
    {
        return;
    }

    on_skip_next_frame(ctx);
}

}
