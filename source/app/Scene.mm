/*
 *  Scene.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 4/18/10.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#import <UIKit/UIKit.h>


#include "Scene.hpp"
#include "Util.hpp"
#include "SpriteManager.hpp"
#include "GameObjectManager.hpp"
#include "StoryboardManager.hpp"
#include "Engine.hpp"
#include "Callback.hpp"


// TEST:
#include "SpriteManager.hpp"
#include "PerfTimer.hpp"
#include <QuartzCore/QuartzCore.h>


namespace Z 
{


// ============================================================================
//
//  Scene Implementation
//
// ============================================================================


#pragma mark Scene Implementation

const uint32_t TEXTURE_FOR_SHOW_ANIMATION = 0;
const uint32_t TEXTURE_FOR_HIDE_ANIMATION = 1;

QuartzRenderTarget* Scene::s_pQuartzRenderTarget = NULL;


Scene::Scene() :
    m_isAnimating(false),
    m_isBeingShown(false),
    m_isBeingHidden(false),
    m_parentWindow(nil),
    m_viewController(nil)
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "Scene( %4d )", m_ID);
}


Scene::~Scene()
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "\t~Scene( %4d )", m_ID);

    m_hEffect.Release();
    m_hStoryboard.Release();
    m_hSpriteForShowing.Release();
    m_hSpriteForHiding.Release();

    SAFE_RELEASE(s_pQuartzRenderTarget);

    [m_viewController release];
    [m_parentWindow   release];
}



RESULT
Scene::Init( const string& name, UIViewController* viewController, UIWindow* window )
{
    RESULT   rval        = S_OK;
    HTexture hTextureForShowing;
    HTexture hTextureForHiding;

    m_name              = name;
    m_viewController    = viewController;
    m_parentWindow      = window;
    [m_viewController   retain];
    [m_parentWindow     retain];

    // TODO: asking for the view will force it to instantiate; 
    // send "viewDidUnload" to VC afterwards?
    DEBUGCHK(m_viewController.view);

    //
    // Create a QuartzRenderTarget, sized in pixels (not points).
    // We render the UIView into this and use it as a texture.
    //
    Rectangle viewRect;
    Platform::GetScreenRectCamera( &viewRect );

    //
    // Create a full-screen Sprite (imposter) for animating the UIView on- and off-screen.
    // When animation is complete the Sprite is hidden and replaced by the actual UIView.
    //
    // Create a render target with two textures (assumes we never have more than one scene showing and one hiding,
    // and that they're the same dimensions).
    if (!s_pQuartzRenderTarget)
    {
        s_pQuartzRenderTarget = QuartzRenderTarget::Create( (UINT32)viewRect.width, (UINT32)viewRect.height, true, 2 );

        if (!s_pQuartzRenderTarget)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: Scene::Init(): failed to create QuartzRenderTarget.");
            return E_UNEXPECTED;
        }
    }
    s_pQuartzRenderTarget->AddRef();


    // Render to texture.  We won't draw this capture, but it seems to pre-warm some Quartz state
    // so that future renders are slightly faster.
    CHR(RenderToTexture());

    // Wrap each Texture in a Sprite.
    // For now, assume all Scenes are full-screen.
    Rectangle screenRect;
    CHR(Platform::GetScreenRectCamera( &screenRect ));

    hTextureForShowing = s_pQuartzRenderTarget->GetTexture( TEXTURE_FOR_SHOW_ANIMATION );
    CHR(SpriteMan.CreateFromTexture( name + "_showSprite", hTextureForShowing, screenRect, &m_hSpriteForShowing ));
    CHR(SpriteMan.SetOpacity( m_hSpriteForShowing, m_viewController.view.layer.opacity ));

    hTextureForHiding  = s_pQuartzRenderTarget->GetTexture( TEXTURE_FOR_HIDE_ANIMATION );
    CHR(SpriteMan.CreateFromTexture( name + "_hideSprite", hTextureForHiding, screenRect, &m_hSpriteForHiding ));
    CHR(SpriteMan.SetOpacity( m_hSpriteForHiding, m_viewController.view.layer.opacity ));

    m_hSpriteForShowing.AddRef();
    m_hSpriteForHiding.AddRef();

    RETAILMSG(ZONE_OBJECT, "Scene[%4d]: \"%-32s\"", m_ID, m_name.c_str());
    
Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Scene::Init( \"%s\" ): failed", name.c_str());
    }
    return rval;
}



// 
// IScene
//

RESULT      
Scene::Show( IN HEffect hEffect, IN HStoryboard hStoryboard )
{
    RESULT rval = S_OK;

    RETAILMSG(ZONE_INFO, "Scene::Show( \"%s\" )", m_name.c_str());

    m_isAnimating   = true;
    m_isBeingShown  = true;
    m_isBeingHidden = false;

    if (!s_pQuartzRenderTarget)
    {
        rval = E_UNEXPECTED;
        goto Exit;
    }

    if (m_hEffect != hEffect)
    {
        EffectMan.AddRef( hEffect );
        EffectMan.Release( m_hEffect );
        m_hEffect = hEffect;
    }

    if (m_hStoryboard != hStoryboard)
    {
        StoryboardMan.Stop( m_hStoryboard );
        StoryboardMan.AddRef( hStoryboard );
        StoryboardMan.Release( m_hStoryboard );
        m_hStoryboard = hStoryboard;
    }


    [m_viewController.view setHidden:YES];
    [m_parentWindow addSubview:[m_viewController view]];
    [m_viewController viewWillAppear:YES];

    // TEST: center view on iPhone 5 (wide screen)
    if (Platform::IsWidescreen()) {
        Rectangle screen;
        Platform::GetScreenRectPoints(&screen);
        CGRect frame = m_viewController.view.frame;
        frame.origin.y = (screen.height - frame.size.height)/2;
        m_viewController.view.frame = frame;
    }

    // Should we show the scene with a transition storyboard?
    if (!hStoryboard.IsNull())
    {
        // Should the storyboard control the Scene's Sprite or the Scene's Effect?
        if (!hEffect.IsNull())
        {
            CHR(StoryboardMan.BindTo( hStoryboard, hEffect ));
        }
        else
        {
            CHR(StoryboardMan.BindTo( hStoryboard, m_hSpriteForShowing ));
        }

        Callback callback( Scene::OnDoneShowing, this );
        CHR(StoryboardMan.CallbackOnFinished( hStoryboard, callback ));
    }

    CHR(RenderToTexture());
    CHR(StoryboardMan.Start( hStoryboard ));
    
Exit:
    if (FAILED(rval))
    {   
        RETAILMSG(ZONE_WARN, "WARNING Scene::Show( \"%s\" ): RTT or storyboard failure, showing immediately", m_name.c_str());
    
        // If we failed render-to-texture or storyboard binding,
        // just make the scene immediately visible.
        m_viewController.view.hidden = NO;
    }

    return rval;
}


RESULT
Scene::Hide( IN HEffect hEffect, IN HStoryboard hStoryboard )
{
    RESULT rval = S_OK;
    
    RETAILMSG(ZONE_INFO, "Scene::Hide( \"%s\" )", m_name.c_str());

    m_isAnimating   = true;
    m_isBeingHidden = true;
    m_isBeingShown  = false;
    
    if (!s_pQuartzRenderTarget)
    {
        rval = E_UNEXPECTED;
        goto Exit;
    }
    
    if (m_hEffect != hEffect)
    {
        EffectMan.AddRef( hEffect );
        EffectMan.Release( m_hEffect );
        m_hEffect = hEffect;
    }
    
    if (m_hStoryboard != hStoryboard)
    {
        StoryboardMan.Stop( m_hStoryboard );
        StoryboardMan.AddRef( hStoryboard );
        StoryboardMan.Release( m_hStoryboard );
        m_hStoryboard = hStoryboard;
    }


    // Should we show the scene with a transition storyboard?
    if (!hStoryboard.IsNull())
    {
        // Should the storyboard control the Scene or the Scene's Effect?
        if (!hEffect.IsNull())
        {
            CHR(StoryboardMan.BindTo( hStoryboard, hEffect ));
        }
        else
        {
            CHR(StoryboardMan.BindTo( hStoryboard, m_hSpriteForHiding ));
        }
        
        Callback callback( Scene::OnDoneHiding, this );
        CHR(StoryboardMan.CallbackOnFinished( hStoryboard, callback ));
    }
    
    CHR(RenderToTexture());
    CHR(StoryboardMan.Start( hStoryboard ));

Exit:
    if (FAILED(rval))
    {   
        RETAILMSG(ZONE_WARN, "WARNING Scene::Hide( \"%s\" ): RTT or storyboard failure, hiding immediately", m_name.c_str());

        // If we failed render-to-texture or storyboard binding,
        // just hide the scene immediately.
        m_viewController.view.hidden = YES;
    }
    
    return rval;
}



RESULT
Scene::Draw( const mat4& matParentWorld )
{
    RESULT rval = S_OK;
    
    
    // Hide the view as soon as we start the animation.
    // This is the easiest place to do it, since it guarantees no flicker.
    if (m_viewController.view.hidden == NO)
    {
        m_viewController.view.hidden = YES;
    }
    
    CHR(Renderer.PushEffect( m_hEffect ));


    if (m_isBeingShown)
    {
        CHR(SpriteMan.DrawSprite( m_hSpriteForShowing, GameCamera.GetViewMatrix() ));
    }
    else
    {
        CHR(SpriteMan.DrawSprite( m_hSpriteForHiding, GameCamera.GetViewMatrix() ));
    }
    
Exit:
    IGNOREHR(Renderer.PopEffect());
    return rval;
}



RESULT
Scene::RenderToTexture()
{
    RESULT rval = S_OK;

    //
    // Render the view into a texture.
    //
    PerfTimer timer;
    timer.Start();
    
    if (m_isBeingShown)
    {
        s_pQuartzRenderTarget->SetRenderTexture( TEXTURE_FOR_SHOW_ANIMATION );
    }
    else
    {
        s_pQuartzRenderTarget->SetRenderTexture( TEXTURE_FOR_HIDE_ANIMATION );
    }

    s_pQuartzRenderTarget->Enable();
    
    
    // TEST: center view on iPhone 5 (wide screen)
    if (Platform::IsWidescreen()) {
        Rectangle screen;
        Platform::GetScreenRectPoints(&screen);
        CGRect frame = m_viewController.view.frame;

        float yOffset = (screen.height - frame.size.height)/2;
        CGContextTranslateCTM(UIGraphicsGetCurrentContext(), 0, yOffset);
    }
    
    BOOL state = m_viewController.view.hidden;
    m_viewController.view.hidden = NO;
    [m_viewController.view.layer renderInContext:UIGraphicsGetCurrentContext()];
    m_viewController.view.hidden = state;
    
    s_pQuartzRenderTarget->Disable();
    timer.Stop();

Exit:
    return rval;
}



void
Scene::OnDoneShowing(void* context)
{
    Scene*  pScene  = (Scene*)context;
    
    pScene->m_isAnimating = false;
    StoryboardMan.Stop( pScene->m_hStoryboard );

    UIView* view    = pScene->m_viewController.view;
    view.hidden     = NO;

    RETAILMSG(ZONE_INFO, "Scene::Show( \"%s\" ): DONE", pScene->GetName().c_str());
}


void
Scene::OnDoneHiding(void* context)
{
    Scene*  pScene  = (Scene*)context;

    pScene->m_isAnimating = false;
    StoryboardMan.Stop( pScene->m_hStoryboard );

    UIView* view    = pScene->m_viewController.view;
    view.hidden     = YES;

    [view removeFromSuperview];

    RETAILMSG(ZONE_INFO, "Scene::Hide( \"%s\" ): DONE", pScene->GetName().c_str());
}




} // END namespace Z


