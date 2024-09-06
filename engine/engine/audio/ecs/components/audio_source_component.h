#pragma once
#include <engine/engine_export.h>

#include <audiopp/source.h>
#include <engine/audio/audio_clip.h>
#include <engine/ecs/components/basic_component.h>
#include <math/math.h>

namespace ace
{

/**
 * @class audio_source_component
 * @brief Class that contains core data for audio sources.
 */
class audio_source_component : public component_crtp<audio_source_component>
{
public:
    /**
     * @brief Updates the audio source with the given transform and delta time.
     * @param t The transform to update with.
     * @param dt The delta time.
     */
    void update(const math::transform& t, delta_t dt);

    /**
     * @brief Called when audio playback begins.
     */
    void on_play_begin();

    /**
     * @brief Called when audio playback ends.
     */
    void on_play_end();

    /**
     * @brief Sets whether the audio source should loop.
     * @param on True to loop, false otherwise.
     */
    void set_loop(bool on);

    /**
     * @brief Sets the volume of the audio source.
     * @param volume The volume level. Valid range: [0.0, 1.0].
     */
    void set_volume(float volume);

    /**
     * @brief Sets the pitch of the audio source.
     * @param pitch The pitch level. Valid range: [0.5, 2.0].
     */
    void set_pitch(float pitch);

    /**
     * @brief Sets the volume rolloff factor of the audio source.
     * @param rolloff The volume rolloff factor. Valid range: [0.0, 10.0].
     */
    void set_volume_rolloff(float rolloff);

    /**
     * @brief Sets the range of the audio source.
     * @param range The range of the audio source. min should be <= max and both >= 0.0.
     */
    void set_range(const frange_t& range);

    /**
     * @brief Sets whether the audio source should autoplay.
     * @param on True to autoplay, false otherwise.
     */
    void set_autoplay(bool on);

    /**
     * @brief Gets whether the audio source is set to autoplay.
     * @return True if autoplay is enabled, false otherwise.
     */
    auto get_autoplay() const -> bool;

    /**
     * @brief Gets the volume of the audio source.
     * @return The volume level.
     */
    auto get_volume() const -> float;

    /**
     * @brief Gets the pitch of the audio source.
     * @return The pitch level.
     */
    auto get_pitch() const -> float;

    /**
     * @brief Gets the volume rolloff factor of the audio source.
     * @return The volume rolloff factor.
     */
    auto get_volume_rolloff() const -> float;

    /**
     * @brief Gets the range of the audio source.
     * @return A constant reference to the range.
     */
    auto get_range() const -> const frange_t&;

    /**
     * @brief Sets the playback position of the audio source.
     * @param offset The playback position offset.
     */
    void set_playback_position(audio::duration_t offset);

    /**
     * @brief Gets the playback position of the audio source.
     * @return The playback position.
     */
    auto get_playback_position() const -> audio::duration_t;

    /**
     * @brief Gets the total playback duration of the audio source.
     * @return The playback duration.
     */
    auto get_playback_duration() const -> audio::duration_t;

    /**
     * @brief Starts playing the audio source.
     */
    void play();

    /**
     * @brief Stops playing the audio source.
     */
    void stop();

    /**
     * @brief Pauses the audio source.
     */
    void pause();

    /**
     * @brief Resumes playing the audio source.
     */
    void resume();

    /**
     * @brief Sets whether the audio source is muted.
     * @param mute True to mute, false otherwise.
     */
    void set_mute(bool mute);

    /**
     * @brief Checks if the audio source is muted.
     * @return True if muted, false otherwise.
     */
    auto is_muted() const -> bool;

    /**
     * @brief Checks if the audio source is currently playing.
     * @return True if playing, false otherwise.
     */
    auto is_playing() const -> bool;

    /**
     * @brief Checks if the audio source is currently paused.
     * @return True if paused, false otherwise.
     */
    auto is_paused() const -> bool;

    /**
     * @brief Checks if the audio source is set to loop.
     * @return True if looping, false otherwise.
     */
    auto is_looping() const -> bool;

    /**
     * @brief Sets the audio clip for the audio source.
     * @param sound The audio clip to set.
     */
    void set_sound(asset_handle<audio_clip> sound);

    /**
     * @brief Gets the audio clip of the audio source.
     * @return The asset handle to the audio clip.
     */
    auto get_sound() const -> asset_handle<audio_clip>;

    /**
     * @brief Checks if the audio source has a valid sound bound.
     * @return True if a valid sound is bound, false otherwise.
     */
    auto has_bound_sound() const -> bool;

private:
    /**
     * @brief Applies all settings to the audio source.
     */
    void apply_all();

    /**
     * @brief Checks if the bound sound is valid.
     * @return True if the sound is valid, false otherwise.
     */
    auto is_sound_valid() const -> bool;

    /**
     * @brief Creates the audio source.
     * @return True if the source was created successfully, false otherwise.
     */
    auto create_source() -> bool;

    bool auto_play_ = true;                 ///< Indicates if the audio source should autoplay.
    bool loop_ = true;                      ///< Indicates if the audio source should loop.
    bool muted_ = false;                    ///< Indicates if the audio source is muted.
    float volume_ = 1.0f;                   ///< The volume level of the audio source. Range: [0.0, 1.0].
    float pitch_ = 1.0f;                    ///< The pitch level of the audio source. Range: [0.5, 2.0].
    float volume_rolloff_ = 1.0f;           ///< The volume rolloff factor of the audio source. Range: [0.0, 10.0].
    frange_t range_ = {1.0f, 20.0f};        ///< The range of the audio source.
    std::shared_ptr<audio::source> source_; ///< The audio source object.
    asset_handle<audio_clip> sound_;        ///< The audio clip bound to the audio source.
};

} // namespace ace
