#pragma once

#include <audiopp/source.h>
#include <engine/audio/audio_clip.h>
#include <engine/ecs/components/basic_component.h>

#include <math/math.h>

namespace ace
{

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : audio_source_component (Class)
/// <summary>
/// Class that contains core data for audio listeners.
/// There can only be one instance of it per scene.
/// </summary>
//-----------------------------------------------------------------------------
class audio_source_component : public component_crtp<audio_source_component>
{
public:
    void update(const math::transform& t, delta_t dt);
    void on_play_begin();
    void on_play_end();

    void set_loop(bool on);
    void set_volume(float volume);
    void set_pitch(float pitch);

    void set_volume_rolloff(float rolloff);
    void set_range(const frange_t& range);
    void set_autoplay(bool on);
    auto get_autoplay() const -> bool;

    auto get_volume() const -> float;
    auto get_pitch() const -> float;
    auto get_volume_rolloff() const -> float;
    auto get_range() const -> const frange_t&;

    void set_playback_position(audio::duration_t offset);
    auto get_playback_position() const -> audio::duration_t;
    auto get_playback_duration() const -> audio::duration_t;

    void play();
    void stop();
    void pause();
    void resume();
    void set_mute(bool mute);

    auto is_muted() const -> bool;

    auto is_playing() const -> bool;
    auto is_paused() const -> bool;

    auto is_looping() const -> bool;

    void set_sound(asset_handle<audio_clip> sound);
    auto get_sound() const -> asset_handle<audio_clip>;

    auto has_bound_sound() const -> bool;

private:
    void apply_all();
    auto is_sound_valid() const -> bool;
    auto create_source() -> bool;
    //-------------------------------------------------------------------------
    // Private Member Variables.1
    //-------------------------------------------------------------------------

    bool auto_play_ = true;
    bool loop_ = true;
    bool muted_ = false;
    float volume_ = 1.0f;
    float pitch_ = 1.0f;
    float volume_rolloff_ = 1.0f;
    frange_t range_ = {1.0f, 20.0f};
    std::shared_ptr<audio::source> source_;
    asset_handle<audio_clip> sound_;
};

} // namespace ace
