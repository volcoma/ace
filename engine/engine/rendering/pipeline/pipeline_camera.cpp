#include "pipeline_camera.h"
#include "deferred/pipeline.h"
namespace ace
{
pipeline_camera::pipeline_camera()
{
    pipeline_ = std::make_shared<rendering::deferred>();
}
pipeline_camera::~pipeline_camera()
{

}
} // namespace ace
