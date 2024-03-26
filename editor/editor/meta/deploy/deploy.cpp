#include "deploy.hpp"

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{
REFLECT(deploy_params)
{
    rttr::registration::class_<deploy_params>("deploy_params")(rttr::metadata("pretty_name", "Deploy Options"))
        .constructor<>()
        .property("deploy_location",
                  &deploy_params::deploy_location)(rttr::metadata("pretty_name", "Deploy Location"),
                                                   rttr::metadata("tooltip", "Choose the deploy location."))
        .property("deploy_dependencies", &deploy_params::deploy_dependencies)(
            rttr::metadata("pretty_name", "Deploy Dependencies"),
            rttr::metadata("tooltip", "This takes some time and if already done should't be necessary."))
        .property("run",
                  &deploy_params::deploy_and_run)(rttr::metadata("pretty_name", "Deploy & Run"),
                                                  rttr::metadata("tooltip", "Run the application after the deploy."));
}

SAVE(deploy_params)
{
}
SAVE_INSTANTIATE(deploy_params, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(deploy_params, cereal::oarchive_binary_t);

LOAD(deploy_params)
{
}
LOAD_INSTANTIATE(deploy_params, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(deploy_params, cereal::iarchive_binary_t);
} // namespace ace
