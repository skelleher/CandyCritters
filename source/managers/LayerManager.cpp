/*
 *  LayerManager.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 11/6/10.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include "Engine.hpp"
#include "LayerManager.hpp"
#include "FileManager.hpp"
#include "Log.hpp"
#include "Handle.hpp"
#include "Settings.hpp"
#include "Platform.hpp"
#include "Types.hpp"
#include "Util.hpp"
#include "GameObjectManager.hpp"
#include "Camera.hpp"



namespace Z
{



LayerManager& 
LayerManager::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new LayerManager();
    }
    
    return static_cast<LayerManager&>(*s_pInstance);
}


LayerManager::LayerManager()
{
    RETAILMSG(ZONE_VERBOSE, "LayerManager()");
    
    s_pResourceManagerName = "LayerManager";
}


LayerManager::~LayerManager()
{
    RETAILMSG(ZONE_VERBOSE, "\t~LayerManager()");
    DEBUGCHK(0);
}



RESULT
LayerManager::Init( IN const string& settingsFilename )
{
    RETAILMSG(ZONE_INFO, "LayerManager::Init( %s )", settingsFilename.c_str());

    RESULT rval = S_OK;
    char   path[MAX_PATH];
    
    //
    // Create a Settings object and load the file.
    //
    Settings mySettings;
    if ( FAILED(mySettings.Read( settingsFilename )) )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::Init( %s ): failed to load settings file", settingsFilename.c_str() );
        return E_UNEXPECTED;
    }
    

    //
    // Create each Layer.
    //
    UINT32 numLayers = mySettings.GetInt("/Layers.NumLayers");

    for (int i = 0; i < numLayers; ++i)
    {
        sprintf(path, "/Layers/Layer%d", i);
        //DEBUGMSG(ZONE_INFO, "Loading [%s]", path);

        Layer *pLayer = NULL;
        CreateLayer( &mySettings, path, &pLayer );
        if (!pLayer)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::Init( %s ): failed to create Layer", path);
            // Continue loading other Layers rather than aborting.
            continue;
        }
        
        //DEBUGMSG(ZONE_LAYER, "Created Layer [%s]", pLayer->GetName().c_str());
        CHR(Add(pLayer->GetName(), pLayer));
    }
    
Exit:
    return rval;
}



RESULT
LayerManager::CreateLayer( IN const string& name, INOUT HLayer* phLayer )
{
    RESULT rval   = S_OK;
    Layer* pLayer = NULL;
 
 
    if (!phLayer || name == "")
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::CreateLayer( \"%s\", 0x%x ): invalid arg",
            name.c_str(), phLayer);
            
        rval = E_INVALID_ARG;
        goto Exit;
    }

    // If Layer with this name already exists, go ahead and return a handle (but also an error)
    if ( SUCCEEDED(Get( name, phLayer )) )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::CreateLayer( \"%s\", 0x%x ): Layer already exists",
            name.c_str(), phLayer);

        rval = E_ALREADY_EXISTS;
        goto Exit;
    }


    pLayer = new Layer();
    pLayer->Init( name );

    CHR(Add( pLayer->GetName(), pLayer, phLayer ));

Exit:
    if (FAILED(rval))
    {
        SAFE_DELETE(pLayer);
    }
    
    return rval;
}



RESULT
LayerManager::CreateLayer( IN const Settings* pSettings, IN const string& settingsPath, INOUT Layer** ppLayer )
{
    RESULT rval = S_OK;
    string name;
    
    if (!pSettings || "" == settingsPath || !ppLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::CreateLayer(): invalid argument");
        return E_INVALID_ARG;
    }
    
    Layer *pLayer = new Layer();
    CPR(pLayer);

    name = pSettings->GetString( settingsPath + ".Name" );
    pLayer->Init( name );

    // Caller must AddRef()
    *ppLayer = pLayer;
    
Exit:    
    return rval;
}



RESULT
LayerManager::AddToLayer( IN HLayer hLayer, IN HGameObject hGameObject )
{
    RESULT rval = S_OK;

    Layer* pLayer = GetObjectPointer( hLayer );
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::AddToLayer( 0x%x, 0x%x ): invalid destination Layer", (UINT32)hLayer, (UINT32)hGameObject);
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    CHR(pLayer->AddGameObject( hGameObject ));
    
Exit:
    return rval;
}



RESULT
LayerManager::AddToLayer( IN HLayer hLayer, IN HSprite hSprite )
{
    RESULT rval = S_OK;

    Layer* pLayer = GetObjectPointer( hLayer );
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::AddToLayer( 0x%x, 0x%x ): invalid destination Layer", (UINT32)hLayer, (UINT32)hSprite);
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    CHR(pLayer->AddSprite( hSprite ));
    
Exit:
    return rval;
}



RESULT
LayerManager::AddToLayer( IN HLayer hLayer, IN HParticleEmitter hParticleEmitter )
{
    RESULT rval = S_OK;

    Layer* pLayer = GetObjectPointer( hLayer );
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::AddToLayer( 0x%x, 0x%x ): invalid destination Layer", (UINT32)hLayer, (UINT32)hParticleEmitter);
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    CHR(pLayer->AddParticleEmitter( hParticleEmitter ));
    
Exit:
    return rval;
}



RESULT
LayerManager::AddToLayer( IN HLayer hLayer, IN HLayer hChildLayer )
{
    RESULT rval = S_OK;

    Layer* pLayer       = GetObjectPointer( hLayer );
    Layer* pChildLayer  = GetObjectPointer( hChildLayer );
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::AddToLayer( 0x%x, 0x%x ): invalid destination Layer", (UINT32)hLayer, (UINT32)hChildLayer);
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    if (!pChildLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::AddToLayer( 0x%x, 0x%x ): invalid child Layer", (UINT32)hLayer, (UINT32)hChildLayer);
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    CHR(pChildLayer->SetParentLayer( hLayer ));
    CHR(pLayer->AddLayer( hChildLayer ));
    
Exit:
    return rval;
}



RESULT
LayerManager::RemoveFromLayer( IN HLayer hLayer, IN HGameObject hGameObject )
{
    RESULT rval = S_OK;

    Layer* pLayer = GetObjectPointer( hLayer );
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::RemoveFromLayer( 0x%x, 0x%x ): invalid Layer", (UINT32)hLayer, (UINT32)hGameObject);
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    CHR(pLayer->RemoveGameObject( hGameObject ));
    
Exit:
    return rval;
}



RESULT
LayerManager::RemoveFromLayer( IN HLayer hLayer, IN HSprite hSprite )
{
    RESULT rval = S_OK;

    Layer* pLayer = GetObjectPointer( hLayer );
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::RemoveFromLayer( 0x%x, 0x%x ): invalid Layer", (UINT32)hLayer, (UINT32)hSprite);
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    CHR(pLayer->RemoveSprite( hSprite ));
    
Exit:
    return rval;
}


RESULT
LayerManager::RemoveFromLayer( IN HLayer hLayer, IN HParticleEmitter hParticleEmitter )
{
    RESULT rval = S_OK;

    Layer* pLayer = GetObjectPointer( hLayer );
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::RemoveFromLayer( 0x%x, 0x%x ): invalid Layer", (UINT32)hLayer, (UINT32)hParticleEmitter);
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    CHR(pLayer->RemoveParticleEmitter( hParticleEmitter ));
    
Exit:
    return rval;
}


RESULT
LayerManager::RemoveFromLayer( IN HLayer hLayer, IN HLayer hChildLayer )
{
    RESULT rval = S_OK;

    Layer* pLayer = GetObjectPointer( hLayer );
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::RemoveFromLayer( 0x%x, 0x%x ): invalid Layer", (UINT32)hLayer, (UINT32)hChildLayer);
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    CHR(pLayer->RemoveLayer( hChildLayer ));
    
Exit:
    return rval;
}



#pragma mark -
#pragma mark IDrawable

RESULT
LayerManager::SetVisible( IN HLayer hLayer, bool isVisible )
{
    RESULT rval = S_OK;

    Layer* pLayer = GetObjectPointer(hLayer);
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::SetVisible( 0x%x, %d ): invalid Layer", (UINT32)hLayer, isVisible);
        rval = E_INVALID_ARG;
        goto Exit;
    }

    CHR(pLayer->SetVisible( isVisible ));
    
Exit:
    return rval;
}



RESULT
LayerManager::SetPosition( IN HLayer hLayer, const vec3& position )
{
    RESULT rval = S_OK;

    Layer* pLayer = GetObjectPointer(hLayer);
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::SetPosition( 0x%x ): invalid Layer", (UINT32)hLayer);
        rval = E_INVALID_ARG;
        goto Exit;
    }

    CHR(pLayer->SetPosition( position ));
    
Exit:
    return rval;
}



RESULT
LayerManager::SetRotation( IN HLayer hLayer, const vec3& rotation )
{
    RESULT rval = S_OK;

    Layer* pLayer = GetObjectPointer(hLayer);
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::SetRotation( 0x%x ): invalid Layer", (UINT32)hLayer);
        rval = E_INVALID_ARG;
        goto Exit;
    }

    CHR(pLayer->SetRotation( rotation ));
    
Exit:
    return rval;
}



RESULT
LayerManager::SetScale( IN HLayer hLayer, float scale )
{
    RESULT rval = S_OK;

    Layer* pLayer = GetObjectPointer(hLayer);
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::SetScale( 0x%x, %2.2f ): invalid Layer", (UINT32)hLayer, scale);
        rval = E_INVALID_ARG;
        goto Exit;
    }

    CHR(pLayer->SetScale( scale ));
    
Exit:
    return rval;
}



RESULT
LayerManager::SetOpacity( IN HLayer hLayer, float opacity )
{
    RESULT rval = S_OK;

    Layer* pLayer = GetObjectPointer(hLayer);
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::SetOpacity( 0x%x, %2.2f ): invalid Layer", (UINT32)hLayer, opacity);
        rval = E_INVALID_ARG;
        goto Exit;
    }

    CHR(pLayer->SetOpacity( opacity ));
    
Exit:
    return rval;
}



RESULT
LayerManager::SetEffect( IN HLayer hLayer, HEffect hEffect )
{
    RESULT rval = S_OK;
    
    Layer* pLayer = GetObjectPointer( hLayer );
    if (pLayer)
    {
#ifdef DEBUG    
        string name;
        EffectMan.GetName( hEffect, &name );
        DEBUGMSG(ZONE_LAYER, "LayerManager::SetEffect( %s, %s )", 
                 pLayer->GetName().c_str(), name.c_str());
#endif        
        pLayer->SetEffect( hEffect );
    }
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::SetEffect( 0x%x, 0x%x ): object not found", (UINT32)hLayer, (UINT32)hEffect);
        rval = E_INVALID_ARG;
    }

Exit:
    return rval;
}



bool
LayerManager::GetVisible( IN HLayer hLayer )
{
    bool rval = false;

    Layer* pLayer = GetObjectPointer(hLayer);
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::GetVisible( 0x%x ): invalid Layer", (UINT32)hLayer);
        return rval;
    }
    
    return pLayer->GetVisible();
}



vec3
LayerManager::GetPosition( IN HLayer hLayer )
{
    vec3 rval(0,0,0);

    Layer* pLayer = GetObjectPointer(hLayer);
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::GetPosition( 0x%x ): invalid Layer", (UINT32)hLayer);
        return rval;
    }

    return pLayer->GetPosition();
}



vec3
LayerManager::GetRotation( IN HLayer hLayer )
{
    vec3 rval(0,0,0);

    Layer* pLayer = GetObjectPointer(hLayer);
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::GetRotation( 0x%x ): invalid Layer", (UINT32)hLayer);
        return rval;
    }

    return pLayer->GetRotation();
}



float
LayerManager::GetScale( IN HLayer hLayer )
{
    float rval = 0.0f;

    Layer* pLayer = GetObjectPointer(hLayer);
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::GetScale( 0x%x ): invalid Layer", (UINT32)hLayer);
        return rval;
    }

    return pLayer->GetScale();
}



float
LayerManager::GetOpacity( IN HLayer hLayer )
{
    float rval = 0.0f;

    Layer* pLayer = GetObjectPointer(hLayer);
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::GetOpacity( 0x%x ): invalid Layer", (UINT32)hLayer);
        return rval;
    }

    return pLayer->GetOpacity();
}



HEffect
LayerManager::GetEffect( IN HLayer hLayer )
{
    HEffect rval;

    Layer* pLayer = GetObjectPointer(hLayer);
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::GetEffect( 0x%x ): invalid Layer", (UINT32)hLayer);
        return rval;
    }

    return pLayer->GetEffect();
}



AABB
LayerManager::GetBounds( IN HLayer hLayer )
{
    AABB rval;

    Layer* pLayer = GetObjectPointer(hLayer);
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::GetBounds( 0x%x ): invalid Layer", (UINT32)hLayer);
        return rval;
    }

    return pLayer->GetBounds();
}




RESULT
LayerManager::SetShadow( IN HLayer hLayer, bool shadowEnabled )
{
    RESULT rval = S_OK;

    Layer* pLayer = GetObjectPointer(hLayer);
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::SetShadowEnabled( 0x%x, %d ): invalid Layer", (UINT32)hLayer, shadowEnabled);
        rval = E_INVALID_ARG;
        goto Exit;
    }

    CHR(pLayer->SetShadow( shadowEnabled ));
    
Exit:
    return rval;
}



bool
LayerManager::GetShadow( IN HLayer hLayer )
{
    bool rval = false;

    Layer* pLayer = GetObjectPointer(hLayer);
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::IsShadowEnabled( 0x%x ): invalid Layer", (UINT32)hLayer);
        return rval;
    }
    
    return pLayer->GetShadow();
}





#pragma mark -
#pragma mark Rendering

RESULT
LayerManager::Clear( IN HLayer hLayer )
{
    RESULT rval = S_OK;
    
    Layer* pLayer = GetObjectPointer( hLayer );
    if (pLayer)
    {
        CHR(pLayer->Clear());
    }
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::Clear( 0x%x ): object not found", (UINT32)hLayer);
        rval = E_INVALID_ARG;
    }

Exit:
    return rval;
}



RESULT
LayerManager::Draw( IN HLayer hLayer, IN const mat4& matParentWorld )
{
    RESULT rval = S_OK;
    
    Layer* pLayer = GetObjectPointer( hLayer );
    if (pLayer)
    {
        CHR(Draw( pLayer, matParentWorld ));
    }
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::Draw( 0x%x ): object not found", (UINT32)hLayer);
        rval = E_INVALID_ARG;
    }

Exit:
    return rval;
}



RESULT
LayerManager::Draw( IN Layer* pLayer, IN const mat4& matParentWorld )
{
    RESULT rval = S_OK;

    CPR(pLayer);

    if ( !pLayer->GetVisible() )
    {
        return S_OK;
    }
    
    
    if (Log::IsZoneEnabled(ZONE_LAYER) && (pLayer->GetNumSprites() || pLayer->GetNumGameObjects() || (pLayer->GetNumLayers())))
    {
        RETAILMSG(ZONE_LAYER, "LayerManager::Draw(): \"%s\" %d sprites %d GOs %d emitters %d layers", 
            pLayer->GetName().c_str(),
            pLayer->GetNumSprites(),
            pLayer->GetNumGameObjects(),
            pLayer->GetNumEmitters(),
            pLayer->GetNumLayers());
        
        char str[256];
        sprintf(str, "%s %lu %lu %lu Effect: %s", 
            pLayer->GetName().c_str(), 
            pLayer->GetNumSprites(), 
            pLayer->GetNumGameObjects(),  
            pLayer->GetNumEmitters(),
            pLayer->GetEffect().GetName().c_str());
            
        DebugRender.Text(str, Color::Green(), 1.0f, 1.0f);
    }
    
    
    pLayer->Draw( GameCamera.GetViewMatrix() );


//    DEBUGMSG(ZONE_LAYER, "LayerManager::Draw(): \"%s\" DONE", 
//             pLayer->GetName().c_str());

Exit:
    return rval;
}



RESULT
LayerManager::Draw()
{
    RESULT rval = S_OK;
    ResourceListIterator ppLayer;

    //
    // Render the Layer and any children.
    //
//    CHR(Renderer.EnableDepthTest ( true ));
    CHR(Renderer.EnableDepthTest ( false ));
    CHR(Renderer.EnableLighting  ( false ));
//    CHR(Renderer.EnableAlphaBlend( true  ));
    CHR(Renderer.EnableAlphaTest ( false ));   // Alpha Test on iPhone (3G) is not any faster than Alpha Blend.
    CHR(Renderer.EnableTexturing ( true  ));
    
    
    for (ppLayer = m_resourceList.begin(); ppLayer != m_resourceList.end(); ++ppLayer)
    {
        Layer* pLayer = *ppLayer;
        DEBUGCHK(pLayer);
        
        // Only draw root layers in this method.
        // They in turn draw their children.
        // We can have multiple roots (e.g. the game world and the UI).
        if (pLayer->IsRootLayer())
        {
            CHR(Draw( pLayer, GameCamera.GetViewMatrix() ));
        }
    }
    
Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::Draw(): rval = 0x%x", rval);
        DEBUGCHK(0);
    }

    return rval;
}



RESULT
LayerManager::Show( IN HLayer hLayer, IN HStoryboard hStoryboard, IN HEffect hEffect )
{
    RESULT rval = S_OK;
    
    Layer* pLayer = GetObjectPointer( hLayer );
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::Show( 0x%x ): hLayer is not a valid handle", (UINT32)hLayer);
        return E_BAD_HANDLE;
    }
    
    
    CHR(pLayer->Show( hEffect, hStoryboard ));
    
Exit:    
    return rval;
}



RESULT
LayerManager::Hide( IN HLayer hLayer, IN HStoryboard hStoryboard, IN HEffect hEffect )
{
    RESULT rval = S_OK;
    
    Layer* pLayer = GetObjectPointer( hLayer );
    if (!pLayer)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: LayerManager::Hide( 0x%x ): hLayer is not a valid handle", (UINT32)hLayer);
        return E_BAD_HANDLE;
    }
    
    
    CHR(pLayer->Hide( hEffect, hStoryboard ));
    
Exit:    
    return rval;
}







} // END namespace Z


