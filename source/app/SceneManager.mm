/*
 *  SceneManager.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 4/18/10.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include "Engine.hpp"
#include "SceneManager.hpp"
#include "FileManager.hpp"
#include "Log.hpp"
#include "Handle.hpp"
#include "Settings.hpp"
#include "Platform.hpp"
#include "Types.hpp"
#include "Util.hpp"
#include "GameObjectManager.hpp"
#include "Camera.hpp"
#include "Scene.hpp"

#import  <UIKit/UIKit.h>


namespace Z
{


bool     SceneManager::s_initialized = false;
//
// This is here only to hide it from C++ code that needs to #include "SceneManager.hpp".
//
static UIWindow* s_mainWindow = nil;



SceneManager& 
SceneManager::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new SceneManager();
        s_pInstance->Init();
    }
    
    return static_cast<SceneManager&>(*s_pInstance);
}


SceneManager::SceneManager()
{
    RETAILMSG(ZONE_VERBOSE, "SceneManager()");
    
    s_pResourceManagerName = "SceneManager";
}


SceneManager::~SceneManager()
{
    RETAILMSG(ZONE_VERBOSE, "\t~SceneManager()");

    [s_mainWindow release];

    DEBUGCHK(0);
}



RESULT
SceneManager::Init()
{
    RETAILMSG(ZONE_INFO, "SceneManager::Init()");
    RESULT rval = S_OK;

    if (s_initialized)
    {
        return S_OK;
    }
    
    s_mainWindow = [[[UIApplication sharedApplication] windows] objectAtIndex:0];
    [s_mainWindow retain];
    
    s_initialized = true;
    
Exit:
    return rval;
}



RESULT
SceneManager::CreateScene( IN const string& name, void* viewController, INOUT HScene* phScene )  // HACK: we pass UIViewController* / UIWindow* as void* to make the C++ compiler happy.

{
    RESULT rval   = S_OK;
    Scene* pScene = NULL;
 
 
    if (!phScene || name == "")
    {
        RETAILMSG(ZONE_ERROR, "ERROR: SceneManager::CreateScene( \"%s\", 0x%x ): invalid arg",
            name.c_str(), phScene);
            
        rval = E_INVALID_ARG;
        goto Exit;
    }


    pScene = new Scene();
    pScene->Init( name, (UIViewController*)viewController, (UIWindow*)s_mainWindow );
    
    CHR(Add( pScene->GetName(), dynamic_cast<IScene*>(pScene), phScene ));

Exit:
    if (FAILED(rval))
    {
        SAFE_DELETE(pScene);
    }
    
    return rval;
}



RESULT
SceneManager::Show( IN HScene hScene, IN HStoryboard hStoryboard, IN HEffect hEffect )
{
    RESULT rval = S_OK;

    IScene* pScene = GetObjectPointer( hScene );
    if (!pScene)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: SceneManager::Show( 0x%x ): hScene is not a valid handle", (UINT32)hScene);
        return E_BAD_HANDLE;
    }
    
    
    CHR(pScene->Show( hEffect, hStoryboard ));

Exit:    
    return rval;
}



RESULT
SceneManager::Hide( IN HScene hScene, IN HStoryboard hStoryboard, IN HEffect hEffect )
{
    RESULT rval = S_OK;
    
    IScene* pScene = GetObjectPointer( hScene );
    if (!pScene)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: SceneManager::Hide( 0x%x ): hScene is not a valid handle", (UINT32)hScene);
        return E_BAD_HANDLE;
    }
    
    
    CHR(pScene->Hide( hEffect, hStoryboard ));
    
Exit:    
    return rval;
}





//
// TODO: skip all this.  Scenes should render to hSprites in a dedicated layer.
//

RESULT
SceneManager::Draw()
{
    RESULT rval = S_OK;
    ResourceListIterator ppScene;
    
    //    CHR(Renderer.EnableDepthTest ( true ));
    CHR(Renderer.EnableDepthTest ( false ));
    CHR(Renderer.EnableLighting  ( false ));
    CHR(Renderer.EnableAlphaBlend( true  ));
    CHR(Renderer.EnableAlphaTest ( false ));   // Alpha Test on iPhone (3G) is not any faster than Alpha Blend.
    CHR(Renderer.EnableTexturing ( true  ));
    
    
    for (ppScene = m_resourceList.begin(); ppScene != m_resourceList.end(); ++ppScene)
    {
        IScene* pScene = *ppScene;
        DEBUGCHK(pScene);
        
        if ( !pScene->IsAnimating() )
            continue;
        
        
        if (Log::IsZoneEnabled(ZONE_LAYER))
        {
            string effectName;
            if (!pScene->GetEffect().IsNull())
            {
                EffectMan.GetName( pScene->GetEffect(), &effectName );
            }
            
            char str[256];
            sprintf(str, "%s Effect: %s", 
                pScene->GetName().c_str(), 
                effectName.c_str());
                
            DebugRender.Text(str, Color::Blue(), 1.0f, 1.0f);
        }
        

        DEBUGMSG(ZONE_LAYER, "SceneManager::Draw(): \"%s\"", 
                 pScene->GetName().c_str());
        
        pScene->Draw( GameCamera.GetViewMatrix() );
        
        DEBUGMSG(ZONE_LAYER, "SceneManager::Draw(): \"%s\" DONE", 
                 pScene->GetName().c_str());
    }
    
Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::Draw(): rval = 0x%x", rval);
        DEBUGCHK(0);
    }
    
    return rval;
}



} // END namespace Z


