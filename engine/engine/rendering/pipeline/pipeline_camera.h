#pragma once

#include <engine/rendering/camera.h>
#include <engine/rendering/pipeline/pipeline.h>
namespace ace
{

class pipeline_camera
{
public:


    pipeline_camera();
    ~pipeline_camera();

    /**
     * @brief Gets the camera object (const version).
     * @return A constant reference to the camera object.
     */
    auto get_camera() const -> const camera&
    {
        return camera_;
    }
    /**
     * @brief Gets the camera object.
     * @return A reference to the camera object.
     */
    auto get_camera() -> camera&
    {
        return camera_;
    }

    auto get_pipeline() const -> const rendering::pipeline::sptr&
    {
        return pipeline_;
    }

    auto get_pipeline() -> rendering::pipeline::sptr&
    {
        return pipeline_;
    }

private:
    rendering::pipeline::sptr pipeline_;
    camera camera_;
};

} // namespace ace
