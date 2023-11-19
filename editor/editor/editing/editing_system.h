#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <math/math.h>
#include <rttr/variant.h>

#include <imgui/imgui.h>

namespace ace
{
struct editing_system
{
    struct selection
    {
        rttr::variant object;
    };

    struct marked
    {
        rttr::variant object;
    };

    struct snap
    {
        ///
        math::vec3 translation_snap = {1.0f, 1.0f, 1.0f};
        ///
        float rotation_degree_snap = 15.0f;
        ///
        float scale_snap = 0.1f;
    };

    //-----------------------------------------------------------------------------
    //  Name : select ()
    /// <summary>
    /// Selects an object. Can be anything.
    /// </summary>
    //-----------------------------------------------------------------------------
    void select(rttr::variant object);
    void mark(rttr::variant object);

    //-----------------------------------------------------------------------------
    //  Name : unselect ()
    /// <summary>
    /// Clears the selection data.
    /// </summary>
    //-----------------------------------------------------------------------------
    void unselect();
    void unmark();

    //-----------------------------------------------------------------------------
    //  Name : try_unselect ()
    /// <summary>
    /// Clears the selection data if it maches the type.
    /// </summary>
    //-----------------------------------------------------------------------------
    template<typename T>
    void try_unselect()
    {
        if(selection_data.object.is_type<T>())
        {
            unselect();
        }
    }

    template<typename T>
    void try_unmark()
    {
        if(marked_data.object.is_type<T>())
        {
            unmark();
        }
    }

    template<typename T>
    auto is_selected(const T& entry) -> bool
    {
        const auto& selected = selection_data.object;

        if(!selected.is_type<T>())
        {
            return false;
        }

        return selected.get_value<T>() == entry;
    }

    template<typename T>
    auto is_marked(const T& entry) -> bool
    {
        const auto& marked = marked_data.object;

        if(!marked.is_type<T>())
        {
            return false;
        }

        return marked.get_value<T>() == entry;
    }

    void close_project();

     /// enable editor grid
    bool show_grid = true;
    /// enable wireframe selection
    bool wireframe_selection = true;
    /// current manipulation gizmo operation.
    //	imguizmo::operation operation = imguizmo::translate;
    /// current manipulation gizmo space.
    //	imguizmo::mode mode = imguizmo::local;
    /// selection data containing selected object
    selection selection_data;
    marked marked_data;

    /// snap data containging various snap options
    snap snap_data;
};
} // namespace ace
