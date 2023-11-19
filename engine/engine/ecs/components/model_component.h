#pragma once
#include "basic_component.h"
#include <engine/rendering/model.h>

namespace ace
{
class material;
//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : model_component (Class)
/// <summary>
/// Class that contains core data for meshes.
/// </summary>
//-----------------------------------------------------------------------------
class model_component : component_crtp<model_component>
{
public:
    //-------------------------------------------------------------------------
    // Public Virtual Methods (Override)

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    //-----------------------------------------------------------------------------
    //  Name : set_casts_shadow ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_casts_shadow(bool cast_shadow);

    //-----------------------------------------------------------------------------
    //  Name : set_casts_reflection ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_casts_reflection(bool casts_reflection);

    //-----------------------------------------------------------------------------
    //  Name : set_static ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_static(bool is_static);

    //-----------------------------------------------------------------------------
    //  Name : casts_shadow ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto casts_shadow() const -> bool;

    //-----------------------------------------------------------------------------
    //  Name : casts_reflection ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto casts_reflection() const -> bool;

    //-----------------------------------------------------------------------------
    //  Name : is_static ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto is_static() const -> bool;

    //-----------------------------------------------------------------------------
    //  Name : get_model ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto get_model() const -> const model&;

    //-----------------------------------------------------------------------------
    //  Name : set_model ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_model(const model& model);

    void set_bone_entities(const std::vector<entt::handle>& bone_entities);
    auto get_bone_entities() const -> const std::vector<entt::handle>&;
    void set_bone_transforms(const std::vector<math::transform>& bone_transforms);
    auto get_bone_transforms() const -> const std::vector<math::transform>&;

private:
    //-------------------------------------------------------------------------
    // Private Member Variables.
    //-------------------------------------------------------------------------
    ///
    bool static_ = true;
    ///
    bool casts_shadow_ = true;
    ///
    bool casts_reflection_ = true;
    ///
    model model_;
    ///
    std::vector<entt::handle> bone_entities_;
    std::vector<math::transform> bone_transforms_;
};
} // namespace ace
