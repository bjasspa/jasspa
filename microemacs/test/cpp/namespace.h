// pvk/View.h

#ifndef INCLUDED_PVKVIEW_H
#define INCLUDED_PVKVIEW_H

#ifndef INCLUDED_PVKOBSERVABLE_H
#include "Observable.h"
#endif
#ifndef INCLUDED_DP_SMARTPTRSET_H
#include <dp/SmartPtrSet.h>
#endif

namespace dg 
{
    class DBox;
    class DPoint3D;
    class Pipeline;
	class DVec3;
}
namespace dp
{
    class String;
}
namespace pvk 
{
    class CameraPtr;
    class CanvasPtr;
    class InteractionPtr;
    class PickResultPtr;
    class RendererPtr;
    class ScenePtr;
    class ToolPtr;
    class ViewPtr;
    class ViewUserProxyPtr;
    class ViewUserPtr;

////////////////////////////////////////////////////////////
//! Defines the view of a Scene onto a Canvas through a Camera.

    class PVK_EXPORT View : public Observable 
    {
        PVK_OBSERVABLE_CLASS(View)

        PVK_OBSERVABLE_BEGIN_INIT_FLAGS
        PVK_OBSERVABLE_INIT_FLAG(INPUT_FOCUS)
        PVK_OBSERVABLE_INIT_FLAG(PIPELINE)
        PVK_OBSERVABLE_INIT_FLAG(CAMERA)
        PVK_OBSERVABLE_INIT_FLAG(CANVAS)
        PVK_OBSERVABLE_INIT_FLAG(SPIN_CENTER)
        PVK_OBSERVABLE_INIT_FLAG(ACTIVE_TOOL)
        PVK_OBSERVABLE_END_INIT_FLAGS

        PVK_OBSERVABLE_BEGIN_DECLARE_FLAGS
        PVK_OBSERVABLE_DECLARE_FLAG(INPUT_FOCUS)
        PVK_OBSERVABLE_DECLARE_FLAG(PIPELINE)
        PVK_OBSERVABLE_DECLARE_FLAG(CAMERA)
        PVK_OBSERVABLE_DECLARE_FLAG(CANVAS)
        PVK_OBSERVABLE_DECLARE_FLAG(SPIN_CENTER)
        PVK_OBSERVABLE_DECLARE_FLAG(ACTIVE_TOOL)
        PVK_OBSERVABLE_END_DECLARE_FLAGS

        PVK_OBSERVABLE_BEGIN_ACCESS_FLAGS
        PVK_OBSERVABLE_ACCESS_FLAG(INPUT_FOCUS)
        PVK_OBSERVABLE_ACCESS_FLAG(PIPELINE)
        PVK_OBSERVABLE_ACCESS_FLAG(CAMERA)
        PVK_OBSERVABLE_ACCESS_FLAG(CANVAS)
        PVK_OBSERVABLE_ACCESS_FLAG(SPIN_CENTER)
        PVK_OBSERVABLE_ACCESS_FLAG(ACTIVE_TOOL)
        PVK_OBSERVABLE_END_ACCESS_FLAGS

        ////////////////////////////////////////

        static ViewPtr Construct (ScenePtr);
        virtual void DestroyHook() = 0;
        virtual void PostDestroyNotifyHook() = 0;

        // connections
        virtual ScenePtr  GetScene  () const = 0;
        virtual CameraPtr GetCamera () const = 0;
        virtual CanvasPtr GetCanvas () const = 0;
        virtual void SetCamera (CameraPtr) = 0;
        virtual void SetCanvas (CanvasPtr) = 0;

        // info from canvas
        virtual bool Is2D() const = 0;
        virtual bool Is3D() const = 0;
        virtual float GetAspectRatio () const = 0;
        virtual const dg::Pipeline &GetPipeline() const = 0;

        // interaction
        virtual bool GetInputFocus () const = 0;
        virtual dg::DPoint3D GetSpinCenter () const = 0;
        virtual ToolPtr GetActiveTool () const = 0;
        virtual void SetSpinCenter (dg::DPoint3D) = 0;
        virtual void SetActiveTool (ToolPtr) = 0;

        // functions (tools?)
        virtual void AddFunction (const dp::String& name, dp::RefCountedPtr) = 0;
        virtual void RemoveFunction (const dp::String& name, dp::RefCountedPtr) = 0;
        virtual dp::RefCountedPtr GetFunction (const dp::String& name) = 0;

        //---------- one-shot camera motions
        virtual void SetOrthographic (bool ortho) = 0;
        virtual void Zoom (bool selectionOnly) = 0; //!< now does a fly
        virtual void ZoomToWindow (int x0, int y0, int x1, int y1) = 0;
		virtual void FlyToBox (const dg::DBox &box, dg::DVec3 *pRotEulerXYZ, 
			double time, bool useLogTime) = 0;

        //---------- interaction interface functions
        virtual void SetInteraction (InteractionPtr) = 0;
        
        //! Sets the current mode handler which processes any picks.
        //! This direct setting of the mode will someday be replaced
        //! by the gui layer setting the handler itself.
        virtual void SetMode (int mode) = 0;

        //! Tell the interaction plugin to toggle between rotation
        //! and translation of selected shapes during mouse-drags.
        virtual void ToggleComponentRotate() = 0;

        //---------- renderer interface functions
        virtual void SetRenderer (RendererPtr) = 0;
        virtual bool GetWorldBounds (dg::DBox& dst, bool selectionOnly) = 0;
        virtual bool DoPick (int x, int y, pvk::PickResultPtr& dst) = 0;
        virtual void GetPickingSegment (int x, int y, 
                                        dg::DPoint3D &p0, 
                                        dg::DPoint3D &p1) = 0;

        //---------- users
        virtual const ViewUserProxyPtr AddUser (const dp::String &userType) = 0;
        virtual const ViewUserProxyPtr AddUser (const ViewUserPtr &) = 0;
        virtual void StartUsers() = 0;

        ////////////////////////////////////////
        virtual void pvk_SetInputFocus (bool x) = 0;
        virtual void pvk_UpdateCanvas () = 0;
        virtual void pvk_UpdateCamera () = 0;
    };

    DEFINE_OBSERVABLE_PTR(View);

    class ViewSet : public dp::SmartPtrSet<ViewPtr> {};

};
#endif
