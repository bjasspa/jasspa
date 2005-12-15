// pvk/View.cpp

#include "View.h"

#include "Globals.h"
#include "Camera.h"
#include "Canvas.h"
#include "Interaction.h"
#include "Kernel.h"
#include "Renderer.h"
#include "Scene.h"
#include "SceneRuntimeManager.h"
#include "Tool.h"
#include "ViewUser.h"
#include "ViewUserProxy.h"

#include <dg/Globals.h>
#include <dg/DBox.h>
#include <dg/DPoint3D.h>
#include <dg/FMat33.h>
#include <dg/Pipeline.h>
#include <dg/Location.h>
#include <dp/SmartPtrMap.h>
#include <dp/HighRes.h>


////////////////////////////////////////////////////////////
namespace pvk
{

    PVK_OBSERVABLE_CLASS_STATIC_MEMBERS(View);

    class ViewImpl : public View
    {
    public:
        
        //! used by flyto to do updates
        class SRMObserver : public dp::Observer
        {
            ViewImpl * const m_owner;
        public:
            SRMObserver(ViewImpl * owner)
                      : m_owner(owner)
                      , dp::Observer(Kernel::GetDispatcher()->GetDefaultObserverQueue())
            {
                SetInterestFlags(SceneRuntimeManager::CONTROL_PHASE());
                m_owner->m_scene->GetSceneRuntimeManager()->AddLocalObserver(this);
            }
            
            ~SRMObserver() {} 
            virtual void  UpdateHook(dp::ObserverFlags)
            {
                m_owner->UpdateFlyTo();
            }
        };
        friend class SRMObserver;
        
        //! holds information required while doing a flyto
        struct FlytoData 
        {
			FlytoData() : m_pSrmObs(NULL) {}
            ~FlytoData() { if(m_pSrmObs) delete(m_pSrmObs); }
            
            SRMObserver *m_pSrmObs;
            
            //! where we started
            dg::DPoint3D m_startPos;
            dg::DPoint3D m_startSpinCenter;
            double m_startViewWd;
            double m_startViewHt;
            dg::DVec3 m_startRot;
            dp::HighRes m_startTime;
            
        namespace pvk
            //! where we want to end up
            dg::DPoint3D m_destinPos;
            dg::DPoint3D m_destinCenter;
            double m_destinViewWd;
            double m_destinViewHt;
            dg::DVec3 m_destinRot;
            double m_destinTime;
        };
        
        ViewImpl (ScenePtr);
        virtual ~ViewImpl ();
        virtual void DestroyHook();
        virtual void PostDestroyNotifyHook();

        virtual ScenePtr  GetScene  () const { return m_scene; }
        virtual CameraPtr GetCamera () const { return m_camera; }
        virtual CanvasPtr GetCanvas () const { return m_canvas; }

        virtual bool Is2D() const { return m_scene->Is2D(); }
        virtual bool Is3D() const { return m_scene->Is3D(); }
        virtual float GetAspectRatio () const;
        virtual const dg::Pipeline &GetPipeline() const { return m_pipeline; }

        virtual bool GetInputFocus () const { return m_inputFocus; }
        virtual dg::DPoint3D GetSpinCenter () const { return m_spinCenter; }
        virtual ToolPtr GetActiveTool () const { return m_activeTool; }

        virtual void SetCamera (CameraPtr);
        virtual void SetCanvas (CanvasPtr);
        virtual void SetSpinCenter (dg::DPoint3D);
        virtual void SetActiveTool (ToolPtr);

        virtual void AddFunction (const dp::String& name, dp::RefCountedPtr);
        virtual void RemoveFunction (const dp::String& name, dp::RefCountedPtr);
        virtual dp::RefCountedPtr GetFunction (const dp::String& name);

        //---------- zoom
        virtual void SetOrthographic (bool ortho);
        virtual void Zoom (bool selectionOnly);
        virtual void ZoomToWindow (int x0, int y0, int x1, int y1);
        /*private*/// void ZoomToBox (const dg::DBox &box);
		virtual void FlyToBox (const dg::DBox &box, dg::DVec3 *pRotEulerXYZ, double time, bool useLogTime);
        /*private*/ void UpdateFlyTo(void);
        
        //---------- interaction
        virtual void SetInteraction (InteractionPtr ip) { m_interaction=ip; }
        virtual void SetMode (int mode);
        virtual void ToggleComponentRotate();

        //---------- renderer
        virtual void SetRenderer (RendererPtr rp) { m_renderer=rp; }
        virtual bool GetWorldBounds (dg::DBox& dst, bool selectionOnly);
        virtual bool DoPick (int x, int y, pvk::PickResultPtr& dst);
        virtual void GetPickingSegment (int x, int y, dg::DPoint3D &p0, dg::DPoint3D &p1);

        //---------- users
        virtual const ViewUserProxyPtr AddUser (const dp::String &userType);
        virtual const ViewUserProxyPtr AddUser (const ViewUserPtr &user);
        const ViewUserProxyPtr AddUserProxy (const ViewUserProxyPtr &proxy);

        virtual void StartUsers ();
        void RemoveAllUsers ();

        virtual void pvk_SetInputFocus (bool x);
        virtual void pvk_UpdateCanvas();
        virtual void pvk_UpdateCamera();

    private:
        ScenePtr  m_scene;
        CameraPtr m_camera;
        CanvasPtr m_canvas;
        bool m_inputFocus;

        dg::Pipeline m_pipeline;
        dg::DPoint3D m_spinCenter;
        ToolPtr m_activeTool;

        ViewUserProxySet m_userProxies;
        bool m_usersStarted;

        dp::SmartPtrMap<dp::RefCountedPtr> m_functions;
        RendererPtr m_renderer;
        InteractionPtr m_interaction;
        FlytoData m_flytoData;
        
        static const float ViewImpl::WINDOW_MARGIN;
    };

	//! in flyto 1 means flown to object precisely fills the screen, 
	//! > 1 gives a bit of white space around it
    const float ViewImpl::WINDOW_MARGIN = 1.01f;
}

////////////////////////////////////////////////////////////

pvk::ViewPtr
pvk::View::Construct(ScenePtr s)
{
	return new ViewImpl(s);
}

pvk::ViewImpl::ViewImpl (ScenePtr s)
    : m_scene(s),
      m_usersStarted(false),
      m_camera(),
      m_canvas(),
      m_inputFocus(false),
      m_spinCenter(0,0,0)
{
    m_scene->pvk_AddView(this);
    m_pipeline.Set2D(m_scene->Is2D());

    Kernel::GetSingleton()->pvk_AddView(this);
	Register();
}

pvk::ViewImpl::~ViewImpl ()
{
}

////////////////////////////////////////////////////////////

//! This object is being destroyed, so explicitly inform
//! the view-users, since we don't want to wait for the
//! the notifications.
void
pvk::ViewImpl::DestroyHook ()
{
    RemoveAllUsers();
}

//! detach this destroyed object from the rest of pvk
void
pvk::ViewImpl::PostDestroyNotifyHook()
{
    if (m_scene)
    {
        m_scene->pvk_RemoveView(this);
        m_scene = NULL;
    }
    if (m_camera)
    {
        m_camera->pvk_RemoveView(this);
        m_camera = NULL;
    }
    if (m_canvas)
    {
        m_canvas->pvk_SetView(NULL);
        m_canvas = NULL;
    }
    Kernel::GetSingleton()->pvk_RemoveView(this);
}

////////////////////////////////////////////////////////////

float
pvk::ViewImpl::GetAspectRatio () const
{
    return (m_canvas ? m_canvas->GetAspectRatio() : 1.0f);
}

void
pvk::ViewImpl::SetOrthographic (bool ortho)
{
    if (Is2D()) ortho = true;

    // no camera?
    if (m_camera == NULL) return;

    // no change?
    bool prev = m_camera->IsOrthographic();
    if (ortho == prev) return;

    // no canvas?
    if (m_canvas == NULL)
    {
        m_camera->SetOrthographic(ortho);
        return;
    }

    // find bbox of what we are now looking at
    int wd = m_canvas->GetWidth();
    int ht = m_canvas->GetHeight();
    dg::DBox box;
    int rect[] = {0,0,wd-1,ht-1};
    bool haveBox = (m_renderer &&
                    m_renderer->GetSweptBounds(box, rect));

    m_camera->SetOrthographic(ortho);
    if (haveBox)
    {
        FlyToBox(box, NULL, 0.5, false);
    }
    else
    {
        Zoom(false);
    }
}

void
pvk::ViewImpl::Zoom (bool selection)
{
    if (m_camera != NULL)
    {
        dg::DBox box;
        if (GetWorldBounds(box,selection))
        {
            FlyToBox(box, NULL, 1, false);
        }
    }
}

void
pvk::ViewImpl::ZoomToWindow (int x0, int y0, int x1, int y1)
{
    if (m_camera != NULL)
    {
        if (x0>x1) { int t=x1; x1=x0; x0=t; }
        if (y0>y1) { int t=y1; y1=y0; y0=t; }

        dg::Location loc = m_camera->GetLocation();
        if (m_camera->IsOrthographic())
        {
            // get window size
            double wd = m_canvas->GetWidth();
            double ht = m_canvas->GetHeight();
            if (wd <= 1) return;
            if (ht <= 1) return;

            // find how big the zoom-window is
            double sx = (x1-x0) / wd;
            double sy = (y1-y0) / ht;
            if (sx < 0.01) sx = 0.01;
            if (sy < 0.01) sy = 0.01;
            double s = dg::Max(sx,sy);

            // translate center of window to center of drag-box
            double cx0 = wd / 2.0;
            double cy0 = ht / 2.0;
            double cx1 = (x0+x1) / 2.0;
            double cy1 = (y0+y1) / 2.0;

            dg::DPoint3D p0,p1;
            m_pipeline.PixelToWorld(cx0,cy0,0, p0);
            m_pipeline.PixelToWorld(cx1,cy1,0, p1);
            dg::DVec3 trans = p1 - p0;

            // reduce the camera width and move the eyepos
            double width = m_camera->GetWidth();
            m_camera->SetWidth(s * width);
            m_camera->WorldMove(trans[0],trans[1],trans[2]);

            //!TODO  m_spinCenter = ???
        }
        else //perspective view
        {
            dg::DBox box;
            int rect[] = {x0,y0,x1,y1};
            if (m_renderer &&
                m_renderer->GetSweptBounds(box, rect))
            {
				FlyToBox(box, NULL, 0, false);
            }
        }
    }
}

#if 0
void
pvk::ViewImpl::ZoomToBox (const dg::DBox &box)
{
    PVK_CHECK(! box.IsEmpty());

    dg::DPoint3D ctr = box.GetCenter();
    dg::DPoint3D mn = box.GetMinPoint();
    dg::DPoint3D mx = box.GetMaxPoint();
    double diag = box.GetDiagonal();

    // move camera to world-center
    m_camera->SetPosition(ctr);
    m_spinCenter = ctr;

    if (Is2D())
    {
        // get 2D extents of data
        float dx = (float) (mx[0] - mn[0]);
        float dy = (float) (mx[1] - mn[1]);

        // flip extents if camera is rotated 90 or 270
        float m00 = m_camera->GetLocation().GetMat().Get(0,0);
        bool rotated = (dg::Abs(m00) < 0.50f);
        if (rotated)
        {
            float tmp=dx; dx=dy; dy=tmp;
        }

        // get window size
        int wd = m_canvas->GetWidth();
        int ht = m_canvas->GetHeight();
        PVK_CHECK(wd > 0);
        PVK_CHECK(ht > 0);

        // find data-units-per-pixel in each direction
        float sx = dx / (float) wd;
        float sy = dy / (float) ht;

        // width of view-volume is greatest-scale times width-in-pixels
        float scale = dg::Max(sx,sy);
        float width = wd * scale;
        m_camera->SetWidth(width);
    }
    else if (m_camera->IsOrthographic())
    {
        m_camera->SetWidth(diag);
        // map hex corners to image plane to fit more tightly?
        //dvHex img; img.CalcTransform (*hex, m_vox.Camera().ViewXF());
        //dvImageRect r; r.EncloseHex (img);
        //float w = WINDOW_MARGIN * r.Wd();
        //float h = WINDOW_MARGIN * r.Ht();
        //SetWindow (w,h);
    }
    else
    {
        double angle = dg::DegToRad(m_camera->GetFOV() * 0.5);
        double dist = (diag * 0.6) / dg::Tan(angle);
        // TODO - avoid clipping object with near plane
        // ?? if (dist < zMin) dist = 2*zMin;
        m_camera->LocalMove(0,0,dist);
    }
}
#endif

//! flies to make box visible  If rotEulerXYZ then rotate to that angle.  time is amount
//! of time taken to do fly to, except if useLogTime and not in orthographic mode then time
//! is multiplier of inverse log of distance (e.g. if time = 1, 1m ~= 1sec, 10m ~= 2sec, etc).
//! rotation is in radians.  Note: the projection method can change while doing the flyto
void
pvk::ViewImpl::FlyToBox (const dg::DBox &box, dg::DVec3 *pRotEulerXYZ, double time, bool useLogTime)
{
    PVK_CHECK(! box.IsEmpty());
    
    // set starting position
    m_flytoData.m_startPos =        m_camera->GetLocation().GetPos();
    m_flytoData.m_startSpinCenter = m_spinCenter;
    double canvasRatio = double(m_canvas->GetHeight()) / (m_canvas->GetWidth() ? m_canvas->GetWidth() : 1);
    m_flytoData.m_startViewWd =     m_camera->GetWidth();
    m_flytoData.m_startViewHt =     m_flytoData.m_startViewWd * canvasRatio;
    dg::DMat44 xf;
    m_camera->GetLocation().GetXform(xf);
    xf.GetRotationXYZ(m_flytoData.m_startRot);
    m_flytoData.m_startTime.Reset();
    
    // setup and convert rotation
    dg::DVec3 dstRot = m_flytoData.m_startRot;
    if (pRotEulerXYZ)
    {
        dstRot = *pRotEulerXYZ;
    }

    // calculate time if required
	if(useLogTime && m_camera->IsOrthographic())
	{
		// then we can't calculate distance properly
        // because we don't move in z
	}
    else if(useLogTime)
    {
        // then work out distance, and use that to generate time
        dg::DPoint3D &boxCtr = box.GetCenter();
        double dist = dg::Distance(boxCtr, m_flytoData.m_startPos);
        if ((dist < 0.001) || (time < 0.001))
        {
            // value very small so don't try to calculate log time
        }
        else
        {
            time = (dg::Log10((dist * time) + 1.0)) * 2;
        }
    }

    // calculate a hex taking into acount rotation of box
    xf = dg::DMat44::Compose(dstRot, dg::DPoint3D(0.0, 0.0, 0.0));
    dg::DBox rotatedBox = box * xf;
    
	// simon 13/9/00 HACK
	// for some reason the camera seems to go to the opposite of
	// what is required i.e. camera.SetRotation(90, 0, 0), 
	// would lead to camera.Rot() == (-90, 0, 0)
	// so make sure rotations are negated it to guarantee correct rotation
//	dstRot.Negate();
//	m_flytoData.m_startRot.Negate();
    
	// get a box that encloses it now it's been rotated the correct way
    double width  = rotatedBox.GetMaxPoint()[0] - rotatedBox.GetMinPoint()[0];
    double height = rotatedBox.GetMaxPoint()[1] - rotatedBox.GetMinPoint()[1];
    double depth  = rotatedBox.GetMaxPoint()[2] - rotatedBox.GetMinPoint()[2];

    // store destin details specific to type of flyto
    // calculate view w,h
    // store destin details
    m_flytoData.m_destinCenter  = box.GetCenter();
    m_flytoData.m_destinRot     = dstRot;
    m_flytoData.m_destinTime    = time; //!< time in seconds
    m_flytoData.m_destinViewWd = WINDOW_MARGIN * width;
    m_flytoData.m_destinViewHt = WINDOW_MARGIN * height;
    
    // work out distance back from bbox required to show whole object
    double dist = (width > height) ? width : height;
    dist = dist / m_camera->GetTanFOV();
    
    // check that none of object will be clipped
    // if(dist < minClip) dist = minClip;  TO_DO work out how to get minClip
    
    // adjust for centre of object
    dist += depth * 0.5;
    
    // calculate the point we need to end up at
    dg::DVec3 negEyeDir(m_camera->GetLocation().GetW());
    negEyeDir.Normalize();
    m_flytoData.m_destinPos = (negEyeDir * float(dist)) + m_flytoData.m_destinCenter;
    
    if (time <= 0)
    {
        // then might as well do the one and only update now
        UpdateFlyTo();
    }
    else {
        // add a observer to the scene runtime manager
        if(!m_flytoData.m_pSrmObs) m_flytoData.m_pSrmObs = new SRMObserver(this);
	}
}

void 
pvk::ViewImpl::UpdateFlyTo(void)
{
    /* 
     * moves a body fraction of distance calculated by using time gone
     * from m_flytoData.m_start??? to m_flytoData.m_destin????
     */
    // calculate fraction we are to move to
    double curTime = double(m_flytoData.m_startTime.MSec()) / 1000;
    double frac;
    if (curTime >= m_flytoData.m_destinTime)
    {
        // then this is our last update
        frac = 1;
        if(m_flytoData.m_pSrmObs) delete(m_flytoData.m_pSrmObs);
        m_flytoData.m_pSrmObs = NULL;
        m_spinCenter = m_flytoData.m_destinCenter;
    }
    else
    {
        frac = curTime / m_flytoData.m_destinTime;
    }
    
    ///rotate to face correct direction
    dg::DVec3 newRot;
    newRot = dg::DVec3::Lirp(frac, m_flytoData.m_startRot, m_flytoData.m_destinRot);
    m_camera->SetRotation (newRot); // note: this should not be rotated around spin centre
    
    // calculate centre
    m_spinCenter = dg::DPoint3D::Lirp(frac, m_flytoData.m_startSpinCenter, m_flytoData.m_destinCenter);
    
    if(m_camera->IsOrthographic())
    {
        // calculate viewport such that camera encloses (newWidth, newHieght)
        double canvasRatio = double(m_canvas->GetHeight()) / (m_canvas->GetWidth() ? m_canvas->GetWidth() : 1);
        double width = m_flytoData.m_startViewWd + ((m_flytoData.m_destinViewWd - 
                                                        m_flytoData.m_startViewWd) * frac);
        double height = m_flytoData.m_startViewHt + ((m_flytoData.m_destinViewHt - 
                                                         m_flytoData.m_startViewHt) * frac);
        
        double camHeight = width * canvasRatio;
        if(camHeight >= height) m_camera->SetWidth(width);
        else                    m_camera->SetWidth(height / (canvasRatio ? canvasRatio : 1));
    }

	// calculate position
    dg::DPoint3D pos = dg::DPoint3D::Lirp(frac, m_flytoData.m_startPos, m_flytoData.m_destinPos);
    m_camera->SetPosition(pos);
}

void
pvk::ViewImpl::SetMode (int mode)
{
    if (m_interaction)
    {
        m_interaction->SetMode(mode);
    }
}

void
pvk::ViewImpl::ToggleComponentRotate()
{
    if (m_interaction)
    {
        m_interaction->ToggleComponentRotate();
    }
}

bool
pvk::ViewImpl::GetWorldBounds (dg::DBox& dst, bool selectionOnly)
{
    return (m_renderer &&
            m_renderer->GetWorldBounds(dst,selectionOnly));
}

bool
pvk::ViewImpl::DoPick (int x, int y, pvk::PickResultPtr& dst)
{
    return (m_renderer &&
            m_renderer->DoPick(x,y,dst));
}

void
pvk::ViewImpl::GetPickingSegment (int x, int y,
                                  dg::DPoint3D &p0,
                                  dg::DPoint3D &p1)
{
    m_pipeline.PixelToWorld(x,y,0, p0);
    m_pipeline.PixelToWorld(x,y,1, p1);
}

////////////////////////////////////////////////////////////

void
pvk::ViewImpl::SetCamera (CameraPtr c)
{
    if (m_camera != c)
    {
        Update(CAMERA());
        if (m_camera)
        {
            m_camera->pvk_RemoveView(this);
        }
        m_camera = c;
        if (m_camera)
        {
            m_camera->pvk_AddView(this);
            if (Is2D())
            {
                m_camera->SetOrthographic(true);
            }
            pvk_UpdateCamera();
        }
    }
}

void
pvk::ViewImpl::SetCanvas (CanvasPtr c)
{
    if (m_canvas != c)
    {
        Update(CANVAS());
        if (m_canvas)
        {
            m_canvas->pvk_SetView(NULL);
        }
        m_canvas = c;
        if (m_canvas)
        {
            m_canvas->pvk_SetView(this);
            pvk_UpdateCanvas();
        }
    }
}

void
pvk::ViewImpl::SetSpinCenter (dg::DPoint3D p)
{
    if (m_spinCenter != p)
    {
        Update(SPIN_CENTER());
        m_spinCenter = p;
    }
}

void
pvk::ViewImpl::SetActiveTool (ToolPtr tool)
{
    if (m_activeTool != tool)
    {
        if (m_activeTool)
        {
            m_activeTool->SetActive(false);
        }

        Update(ACTIVE_TOOL());
        m_activeTool = tool;

        if (m_activeTool)
        {
            m_activeTool->SetActive(true);
        }
    }
}

void
pvk::ViewImpl::pvk_SetInputFocus (bool x)
{
    if (m_inputFocus != x)
    {
        Update(INPUT_FOCUS());
        m_inputFocus = x;
    }
}

////////////////////////////////////////////////////////////

void
pvk::ViewImpl::AddFunction (const dp::String& name, dp::RefCountedPtr func)
{
    m_functions[name] = func;
}

void
pvk::ViewImpl:: RemoveFunction (const dp::String& name, dp::RefCountedPtr func)
{
    m_functions.erase(name);
}

dp::RefCountedPtr
pvk::ViewImpl::GetFunction (const dp::String& name)
{
    dp::RefCountedPtr result (NULL);
    m_functions.find(name, result);
    return result;
}

////////////////////////////////////////////////////////////
// USERS
////////////////////////////////////////////////////////////

const pvk::ViewUserProxyPtr
pvk::ViewImpl::AddUser (const dp::String &userType)
{
    return AddUserProxy(ViewUserProxy::Construct(this, userType));
}

const pvk::ViewUserProxyPtr
pvk::ViewImpl::AddUser (const ViewUserPtr &viewUser)
{
    return AddUserProxy(ViewUserProxy::Construct(this, viewUser));
}

const pvk::ViewUserProxyPtr
pvk::ViewImpl::AddUserProxy (const ViewUserProxyPtr &proxy)
{
    if (proxy)
    {
        m_userProxies.insert(proxy);
        if (m_usersStarted)
        {
            proxy->Start();
        }
    }
    return proxy;
}

void
pvk::ViewImpl::StartUsers ()
{
    if (!m_usersStarted)
    {
        m_usersStarted = true;

        ViewUserProxySet::iterator i = m_userProxies.begin();
        ViewUserProxySet::iterator end = m_userProxies.end();
        for (; i!=end; ++i)
        {
            (*i)->Start();
        }
    }
}

void
pvk::ViewImpl::RemoveAllUsers ()
{
    ViewUserProxySet::iterator i = m_userProxies.begin();
    ViewUserProxySet::iterator end = m_userProxies.end();
    for (; i!=end; ++i)
    {
        (*i)->Stop();
    }
    m_userProxies.clear();
    m_usersStarted = false;
}

void
pvk::ViewImpl::pvk_UpdateCanvas ()
{
    m_pipeline.SetCanvasSize(m_canvas->GetWidth(),
                             m_canvas->GetHeight());
    Update(PIPELINE());
}

void
pvk::ViewImpl::pvk_UpdateCamera ()
{
    //TODO -- remove these hard-coded values.
    bool ortho = m_camera->IsOrthographic();
    if (ortho)
    {
        m_pipeline.SetDepthRange(-10.0,10.0);
    }
    else
    {
        m_pipeline.SetDepthRange(0.05, 10.0);
    }

    m_pipeline.SetCameraLocation(m_camera->GetLocation());
    m_pipeline.SetOrthographic(m_camera->IsOrthographic());
    m_pipeline.SetOrthoWidth(m_camera->GetWidth());
    m_pipeline.SetPerspFOV(m_camera->GetFOV());

    Update(PIPELINE());
}

////////////////////////////////////////////////////////////
