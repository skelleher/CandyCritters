/*
 *  ParticleManager.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 7/26/11.
 *  Copyright 2011 Sean Kelleher. All rights reserved.
 *
 */

#include "Engine.hpp"
#include "ParticleManager.hpp"
#include "FileManager.hpp"
#include "Log.hpp"
#include "Handle.hpp"
#include "Settings.hpp"
#include "Platform.hpp"
#include "Types.hpp"
#include "Util.hpp"



namespace Z
{



ParticleManager& 
ParticleManager::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new ParticleManager();
    }
    
    return static_cast<ParticleManager&>(*s_pInstance);
}


ParticleManager::ParticleManager()
{
    RETAILMSG(ZONE_VERBOSE, "ParticleManager()");
    
    s_pResourceManagerName = "ParticleManager";
    
    float updateRateHZ  = GlobalSettings.GetFloat( "/Settings.ParticleUpdateRateHZ", DEFAULT_PARTICLE_UPDATE_RATE_HZ );
    float clampedRateHZ = CLAMP(updateRateHZ, 15, 60);
    if (clampedRateHZ != updateRateHZ)
    {
        RETAILMSG(ZONE_WARN, "WARNING: ParticleManager: clamped /Settings.ParticleUpdateRateHZ to %2.0f (valid range is 15-60 Hz)", clampedRateHZ);
    }
    
    m_updateIntervalMS = 1000.0 / clampedRateHZ;
}


ParticleManager::~ParticleManager()
{
    RETAILMSG(ZONE_VERBOSE, "\t~ParticleManager()");
    DEBUGCHK(0);
}



RESULT
ParticleManager::Init( IN const string& settingsFilename )
{
    RETAILMSG(ZONE_PARTICLES, "ParticleManager::Init( %s )", settingsFilename.c_str());

    RESULT rval = S_OK;
    char   path[MAX_PATH];
    
    //
    // Create a Settings object and load the file.
    //
    Settings mySettings;
    if ( FAILED(mySettings.Read( settingsFilename )) )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ParticleManager::Init( %s ): failed to load settings file", settingsFilename.c_str() );
        return E_UNEXPECTED;
    }
    

    //
    // Create each ParticleEmitter.
    //
    UINT32 numParticleEmitters = mySettings.GetInt("/ParticleEmitters.NumParticleEmitters");

    for (int i = 0; i < numParticleEmitters; ++i)
    {
        sprintf(path, "/ParticleEmitters/ParticleEmitter%d", i);

        ParticleEmitter *pParticleEmitter = new ParticleEmitter();
        CPR(pParticleEmitter);

        string name             = mySettings.GetString( string(path) + ".Name" );
        string filename         = mySettings.GetString( string(path) + ".Filename" );
        bool   deleteOnFinish   = mySettings.GetBool  ( string(path) + ".DeleteOnFinish", false );

        pParticleEmitter->SetDeleteOnFinish( deleteOnFinish );
        if ( FAILED(pParticleEmitter->InitFromFile( filename )))
        {
            RETAILMSG(ZONE_ERROR, "ERROR: ParticleManager::Init( %s ): failed to init ParticleEmitter from file %s", filename.c_str());
            // Continue loading other ParticleEmitters rather than aborting.
            continue;
        }
            
        
        RETAILMSG(ZONE_INFO, "ParticleEmitter[%4d]: \"%-32s\"", pParticleEmitter->GetID(), pParticleEmitter->GetName().c_str());

        CHR(Add(name, pParticleEmitter));
    }
    
Exit:
    return rval;
}



RESULT
ParticleManager::Start( IN HParticleEmitter handle )
{
    RESULT rval = S_OK;
    
    CHR(Start(GetObjectPointer( handle )));
    
Exit:
    return rval;
}

    

RESULT
ParticleManager::Stop( IN HParticleEmitter handle )
{
    RESULT rval = S_OK;
    
    CHR(Stop(GetObjectPointer( handle )));

Exit:
    return rval;
}



RESULT
ParticleManager::Pause( IN HParticleEmitter handle )
{
    RESULT rval = S_OK;
    
    CHR(Pause(GetObjectPointer( handle )));

Exit:
    return rval;
}



RESULT
ParticleManager::Start( IN ParticleEmitter* pParticleEmitter )
{
    RESULT rval = S_OK;
    
    CPR(pParticleEmitter);

/*
    if (!pParticleEmitter->IsStarted())
    {
        pParticleEmitter->Start();
        m_runningParticleEmittersList.push_front( pParticleEmitter );
    }
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ParticleEmitterManager::Start( %s ): already started", pParticleEmitter->GetName().c_str());
        rval = E_FAIL;
    }
*/
    
    if (!pParticleEmitter->IsStarted())
    {
        m_runningParticleEmittersList.push_front( pParticleEmitter );
    }

    pParticleEmitter->Start();

Exit:
    return rval;
}

    

RESULT
ParticleManager::Stop( IN ParticleEmitter* pParticleEmitter )
{
    RESULT rval = S_OK;
    
    CPR(pParticleEmitter);

    if (pParticleEmitter->IsStarted() || pParticleEmitter->IsPaused())
    {
        pParticleEmitter->Stop();

        ParticleEmitterListIterator ppParticleEmitter;
        ppParticleEmitter = find( m_runningParticleEmittersList.begin(), m_runningParticleEmittersList.end(), pParticleEmitter );
        if (ppParticleEmitter != m_runningParticleEmittersList.end())
        {
            m_runningParticleEmittersList.erase( ppParticleEmitter );
        }
    }
    else 
    {
//        RETAILMSG(ZONE_WARN, "WARNING: ParticleEmitterManager::Stop( %s ): already stopped", pParticleEmitter->GetName().c_str());
        rval = E_INVALID_OPERATION;
    }


Exit:
    return rval;
}



RESULT
ParticleManager::Pause( IN ParticleEmitter* pParticleEmitter )
{
    RESULT rval = S_OK;
    
    CPR(pParticleEmitter);

    if (pParticleEmitter->IsStarted())
    {
//        pParticleEmitter->Pause();

        ParticleEmitterListIterator ppParticleEmitter;
        ppParticleEmitter = find( m_runningParticleEmittersList.begin(), m_runningParticleEmittersList.end(), pParticleEmitter );
        if (ppParticleEmitter != m_runningParticleEmittersList.end())
        {
            m_runningParticleEmittersList.erase( ppParticleEmitter );
        }
    }
    else 
    {
//        RETAILMSG(ZONE_WARN, "WARNING: ParticleEmitterManager::Pause( %s ): already paused", pParticleEmitter->GetName().c_str());
        rval = E_INVALID_OPERATION;
    }

    pParticleEmitter->Pause();

Exit:
    return rval;
}



bool
ParticleManager::IsStarted( IN HParticleEmitter handle )
{
    ParticleEmitter*     pParticleEmitter;
    
    pParticleEmitter = GetObjectPointer( handle );
    if (pParticleEmitter)
    {
        return pParticleEmitter->IsStarted();
    }
    
    return false;
}



bool
ParticleManager::IsStopped( IN HParticleEmitter handle )
{
    ParticleEmitter*     pParticleEmitter;
    
    pParticleEmitter = GetObjectPointer( handle );
    if (pParticleEmitter)
    {
        return pParticleEmitter->IsStopped();
    }
    
    return false;
}



bool
ParticleManager::IsPaused( IN HParticleEmitter handle )
{
    ParticleEmitter*     pParticleEmitter;
    
    pParticleEmitter = GetObjectPointer( handle );
    if (pParticleEmitter)
    {
        return pParticleEmitter->IsPaused();
    }
    
    return false;
}



UINT64
ParticleManager::GetDurationMS( IN const string& name )
{
    RESULT      rval        = S_OK;
    UINT64      duration    = 0;
    HParticleEmitter hParticleEmitter;

    CHR(Get( name, &hParticleEmitter ));
    duration = GetDurationMS( hParticleEmitter );
    Release( hParticleEmitter );
    
Exit:    
    return duration;
}



UINT64
ParticleManager::GetDurationMS( IN HParticleEmitter hParticleEmitter )
{
    ParticleEmitter*     pParticleEmitter;
    UINT64          duration = 0;
    
    pParticleEmitter = GetObjectPointer( hParticleEmitter );
    if (pParticleEmitter)
    {
        duration = pParticleEmitter->GetDurationMS();
    }

Exit:
    return duration;
}



RESULT
ParticleManager::ReleaseOnNextFrame( IN HParticleEmitter handle )
{
    RESULT           rval = S_OK;
    ParticleEmitter* pParticleEmitter;

    pParticleEmitter = GetObjectPointer( handle );
    if (!pParticleEmitter)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    
    CHR(ReleaseOnNextFrame( pParticleEmitter ));

Exit:
    return rval;
}



RESULT
ParticleManager::ReleaseOnNextFrame( IN ParticleEmitter* pParticleEmitter )
{
    RESULT rval = S_OK;
    
    CPR(pParticleEmitter)
    
    m_pendingReleaseParticleEmittersList.push_back( pParticleEmitter );
    
    // TEST:
    DEBUGMSG(ZONE_PARTICLES, "ParticleManager::ReleaseOnNextFrame( \"%s\" )", pParticleEmitter->GetName().c_str());
    
      
Exit:
    return rval;
}






#pragma mark -
#pragma mark IDrawable

RESULT
ParticleManager::SetVisible( IN HParticleEmitter hParticleEmitter, bool isVisible )
{
    RESULT rval = S_OK;

    ParticleEmitter* pParticleEmitter = GetObjectPointer(hParticleEmitter);
    if (!pParticleEmitter)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ParticleManager::SetVisible( 0x%x, %d ): invalid ParticleEmitter", (UINT32)hParticleEmitter, isVisible);
        rval = E_INVALID_ARG;
        goto Exit;
    }

    CHR(pParticleEmitter->SetVisible( isVisible ));
    
Exit:
    return rval;
}



RESULT
ParticleManager::SetPosition( IN HParticleEmitter hParticleEmitter, const vec3& position )
{
    RESULT rval = S_OK;

    ParticleEmitter* pParticleEmitter = GetObjectPointer(hParticleEmitter);
    if (!pParticleEmitter)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ParticleManager::SetPosition( 0x%x ): invalid ParticleEmitter", (UINT32)hParticleEmitter);
        rval = E_INVALID_ARG;
        goto Exit;
    }

    CHR(pParticleEmitter->SetPosition( position ));
    
Exit:
    return rval;
}



RESULT
ParticleManager::SetRotation( IN HParticleEmitter hParticleEmitter, const vec3& rotation )
{
    RESULT rval = S_OK;

    ParticleEmitter* pParticleEmitter = GetObjectPointer(hParticleEmitter);
    if (!pParticleEmitter)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ParticleManager::SetRotation( 0x%x ): invalid ParticleEmitter", (UINT32)hParticleEmitter);
        rval = E_INVALID_ARG;
        goto Exit;
    }

    CHR(pParticleEmitter->SetRotation( rotation ));
    
Exit:
    return rval;
}



RESULT
ParticleManager::SetScale( IN HParticleEmitter hParticleEmitter, float scale )
{
    RESULT rval = S_OK;

    ParticleEmitter* pParticleEmitter = GetObjectPointer(hParticleEmitter);
    if (!pParticleEmitter)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ParticleManager::SetScale( 0x%x, %2.2f ): invalid ParticleEmitter", (UINT32)hParticleEmitter, scale);
        rval = E_INVALID_ARG;
        goto Exit;
    }

    CHR(pParticleEmitter->SetScale( scale ));
    
Exit:
    return rval;
}



RESULT
ParticleManager::SetOpacity( IN HParticleEmitter hParticleEmitter, float opacity )
{
    RESULT rval = S_OK;

    ParticleEmitter* pParticleEmitter = GetObjectPointer(hParticleEmitter);
    if (!pParticleEmitter)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ParticleManager::SetOpacity( 0x%x, %2.2f ): invalid ParticleEmitter", (UINT32)hParticleEmitter, opacity);
        rval = E_INVALID_ARG;
        goto Exit;
    }

    CHR(pParticleEmitter->SetOpacity( opacity ));
    
Exit:
    return rval;
}



RESULT
ParticleManager::SetEffect( IN HParticleEmitter hParticleEmitter, HEffect hEffect )
{
    RESULT rval = S_OK;
    
    ParticleEmitter* pParticleEmitter = GetObjectPointer( hParticleEmitter );
    if (pParticleEmitter)
    {
#ifdef DEBUG    
        string name;
        EffectMan.GetName( hEffect, &name );
        DEBUGMSG(ZONE_PARTICLES, "ParticleManager::SetEffect( %s, %s )", 
                 pParticleEmitter->GetName().c_str(), name.c_str());
#endif        
        pParticleEmitter->SetEffect( hEffect );
    }
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ParticleManager::SetEffect( 0x%x, 0x%x ): object not found", (UINT32)hParticleEmitter, (UINT32)hEffect);
        rval = E_INVALID_ARG;
    }

Exit:
    return rval;
}



bool
ParticleManager::GetVisible( IN HParticleEmitter hParticleEmitter )
{
    bool rval = false;

    ParticleEmitter* pParticleEmitter = GetObjectPointer(hParticleEmitter);
    if (!pParticleEmitter)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ParticleManager::GetVisible( 0x%x ): invalid ParticleEmitter", (UINT32)hParticleEmitter);
        return rval;
    }
    
    return pParticleEmitter->GetVisible();
}



vec3
ParticleManager::GetPosition( IN HParticleEmitter hParticleEmitter )
{
    vec3 rval(0,0,0);

    ParticleEmitter* pParticleEmitter = GetObjectPointer(hParticleEmitter);
    if (!pParticleEmitter)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ParticleManager::GetPosition( 0x%x ): invalid ParticleEmitter", (UINT32)hParticleEmitter);
        return rval;
    }

    return pParticleEmitter->GetPosition();
}



vec3
ParticleManager::GetRotation( IN HParticleEmitter hParticleEmitter )
{
    vec3 rval(0,0,0);

    ParticleEmitter* pParticleEmitter = GetObjectPointer(hParticleEmitter);
    if (!pParticleEmitter)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ParticleManager::GetRotation( 0x%x ): invalid ParticleEmitter", (UINT32)hParticleEmitter);
        return rval;
    }

    return pParticleEmitter->GetRotation();
}



float
ParticleManager::GetScale( IN HParticleEmitter hParticleEmitter )
{
    float rval = 0.0f;

    ParticleEmitter* pParticleEmitter = GetObjectPointer(hParticleEmitter);
    if (!pParticleEmitter)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ParticleManager::GetScale( 0x%x ): invalid ParticleEmitter", (UINT32)hParticleEmitter);
        return rval;
    }

    return pParticleEmitter->GetScale();
}



float
ParticleManager::GetOpacity( IN HParticleEmitter hParticleEmitter )
{
    float rval = 0.0f;

    ParticleEmitter* pParticleEmitter = GetObjectPointer(hParticleEmitter);
    if (!pParticleEmitter)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ParticleManager::GetOpacity( 0x%x ): invalid ParticleEmitter", (UINT32)hParticleEmitter);
        return rval;
    }

    return pParticleEmitter->GetOpacity();
}



HEffect
ParticleManager::GetEffect( IN HParticleEmitter hParticleEmitter )
{
    HEffect rval;

    ParticleEmitter* pParticleEmitter = GetObjectPointer(hParticleEmitter);
    if (!pParticleEmitter)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ParticleManager::GetEffect( 0x%x ): invalid ParticleEmitter", (UINT32)hParticleEmitter);
        return rval;
    }

    return pParticleEmitter->GetEffect();
}



AABB
ParticleManager::GetBounds( IN HParticleEmitter hParticleEmitter )
{
    AABB rval;

    ParticleEmitter* pParticleEmitter = GetObjectPointer(hParticleEmitter);
    if (!pParticleEmitter)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ParticleManager::GetBounds( 0x%x ): invalid ParticleEmitter", (UINT32)hParticleEmitter);
        return rval;
    }

    return pParticleEmitter->GetBounds();
}




RESULT
ParticleManager::SetShadow( IN HParticleEmitter hParticleEmitter, bool shadowEnabled )
{
    RESULT rval = S_OK;

    ParticleEmitter* pParticleEmitter = GetObjectPointer(hParticleEmitter);
    if (!pParticleEmitter)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ParticleManager::SetShadowEnabled( 0x%x, %d ): invalid ParticleEmitter", (UINT32)hParticleEmitter, shadowEnabled);
        rval = E_INVALID_ARG;
        goto Exit;
    }

    CHR(pParticleEmitter->SetShadow( shadowEnabled ));
    
Exit:
    return rval;
}



bool
ParticleManager::GetShadow( IN HParticleEmitter hParticleEmitter )
{
    bool rval = false;

    ParticleEmitter* pParticleEmitter = GetObjectPointer(hParticleEmitter);
    if (!pParticleEmitter)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ParticleManager::IsShadowEnabled( 0x%x ): invalid ParticleEmitter", (UINT32)hParticleEmitter);
        return rval;
    }
    
    return pParticleEmitter->GetShadow();
}




#pragma mark -
#pragma mark Update
RESULT
ParticleManager::Update( UINT64 elapsedMS )
{
    RESULT rval = S_OK;


    //
    // TODO: if our update frequency is less than 60Hz, we should
    // spread out the emitter updates, rather than updating ALL of them
    // every fourth frame (for example).
    // 
    
    static UINT64 lastUpdateTimeMS = 0;
    if ((elapsedMS - lastUpdateTimeMS) < m_updateIntervalMS)
    {
        return S_OK;
    }
    lastUpdateTimeMS = elapsedMS;


    // Release ParticleEmitters that were marked as done on previous frame.
    ParticleEmitterListIterator ppParticleEmitter;
    for (ppParticleEmitter = m_pendingReleaseParticleEmittersList.begin(); ppParticleEmitter != m_pendingReleaseParticleEmittersList.end(); ++ppParticleEmitter)
    {
        ParticleEmitter* pParticleEmitter = *ppParticleEmitter;
        if (!pParticleEmitter)
            continue;
        
        DEBUGMSG(ZONE_PARTICLES, "Releasing ParticleEmitter [%s], refCount = %d", pParticleEmitter->GetName().c_str(), pParticleEmitter->GetRefCount());
        
        //pParticleEmitter->Stop();
        
        CHR(Release( pParticleEmitter ));
    }
    m_pendingReleaseParticleEmittersList.clear();



    // Update every running ParticleEmitter.
    for (ppParticleEmitter = m_runningParticleEmittersList.begin(); ppParticleEmitter != m_runningParticleEmittersList.end(); /*++ppParticleEmitter*/)
    {
        ParticleEmitter* pParticleEmitter = *ppParticleEmitter;
        if (!pParticleEmitter)
        {
            ppParticleEmitter = m_runningParticleEmittersList.erase( ppParticleEmitter );
            continue;
        } 

//        if (Log::IsZoneEnabled(ZONE_PARTICLES | ZONE_VERBOSE))
//        {
//            char str[1024];
//            sprintf(str, "PE: %s", pParticleEmitter->GetName().c_str());
//            DebugRender.Text(str, Color::White(), 1.0f, 1.0f);
//        }
        
        
        if (FAILED(pParticleEmitter->Update( elapsedMS )))
        {
            pParticleEmitter->Stop();

            //
            // NOTE: if you erase a list member while iterating the list,
            // be sure to properly increment the iterator as seen below.
            //
            ppParticleEmitter = m_runningParticleEmittersList.erase( ppParticleEmitter );
        }
        else 
        {
            ++ppParticleEmitter;
        }
    }

Exit:
    return rval;
}





#pragma mark -
#pragma mark Rendering

RESULT
ParticleManager::Draw( IN HParticleEmitter hParticleEmitter, IN const mat4& matParentWorld )
{
    RESULT rval = S_OK;
    
    ParticleEmitter* pParticleEmitter = GetObjectPointer( hParticleEmitter );
    if (pParticleEmitter)
    {
        CHR(Draw( pParticleEmitter, matParentWorld ));
    }
    else 
    {
        //RETAILMSG(ZONE_ERROR, "ERROR: ParticleManager::Draw( 0x%x ): object not found", (UINT32)hParticleEmitter);
        rval = E_INVALID_ARG;
    }

Exit:
    return rval;
}



RESULT
ParticleManager::Draw( IN ParticleEmitter* pParticleEmitter, IN const mat4& matParentWorld )
{
    RESULT rval = S_OK;

    CPR(pParticleEmitter);

    if ( !pParticleEmitter->GetVisible() || !pParticleEmitter->IsStarted() )
    {
        return S_OK;
    }
    
    pParticleEmitter->Draw( matParentWorld );

Exit:
    return rval;
}



RESULT
ParticleManager::Draw()
{
    RESULT rval = S_OK;
    ResourceListIterator ppParticleEmitter;

    //
    // Render all active ParticleEmitters.
    //
//    CHR(Renderer.EnableDepthTest ( true ));
    CHR(Renderer.EnableDepthTest ( false ));
    CHR(Renderer.EnableLighting  ( false ));
    CHR(Renderer.EnableAlphaBlend( true  ));
    CHR(Renderer.EnableAlphaTest ( false ));   // Alpha Test on iPhone (3G) is not any faster than Alpha Blend.
    CHR(Renderer.EnableTexturing ( true  ));
    
    
    for (ppParticleEmitter = m_resourceList.begin(); ppParticleEmitter != m_resourceList.end(); ++ppParticleEmitter)
    {
        ParticleEmitter* pParticleEmitter = *ppParticleEmitter;
        if (pParticleEmitter)
        {
            CHR(Draw( pParticleEmitter, GameCamera.GetViewMatrix() ));
        }
    }
    
Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ParticleManager::Draw(): rval = 0x%x", rval);
        DEBUGCHK(0);
    }

    return rval;
}




} // END namespace Z


