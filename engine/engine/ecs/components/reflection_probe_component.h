#pragma once
#include "basic_component.h"
#include <engine/rendering/reflection_probe.h>

#include <array>
#include <base/basetypes.hpp>
#include <graphics/render_pass.h>
#include <graphics/render_view.h>

namespace ace
{
//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : reflection_probe_component (Class)
/// <summary>
/// Class that contains our core light data, used for rendering and other
/// things.
/// </summary>
//-----------------------------------------------------------------------------
class reflection_probe_component : public component_crtp<reflection_probe_component>
{
public:
    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    //  Name : get_probe ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto get_probe() const -> const reflection_probe&;

    //-----------------------------------------------------------------------------
    //  Name : set_probe ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_probe(const reflection_probe& probe);

    //-----------------------------------------------------------------------------
    //  Name : compute_projected_sphere_rect ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto compute_projected_sphere_rect(irect32_t& rect,
                                       const math::vec3& position,
                                       const math::transform& view,
                                       const math::transform& proj) const -> int;

    //-----------------------------------------------------------------------------
    //  Name : get_render_view ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto get_render_view(size_t idx) -> gfx::render_view&;

    //-----------------------------------------------------------------------------
    //  Name : get_cubemap ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto get_cubemap() -> std::shared_ptr<gfx::texture>;

    //-----------------------------------------------------------------------------
    //  Name : get_cubemap_fbo ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto get_cubemap_fbo() -> std::shared_ptr<gfx::frame_buffer>;

    void update();

private:
    //-------------------------------------------------------------------------
    // Private Member Variables.
    //-------------------------------------------------------------------------
    /// The probe object this component represents
    reflection_probe probe_;
    /// The render view for this component
    std::array<gfx::render_view, 6> render_view_;
};
} // namespace ace
