/*
 *  StoryboardManager.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 10/10/10.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include "Engine.hpp"
#include "StoryboardManager.hpp"
#include "FileManager.hpp"
#include "Log.hpp"
#include "Handle.hpp"
#include "Settings.hpp"
#include "Platform.hpp"
#include "Types.hpp"
#include "Util.hpp"
#include "GameObjectManager.hpp"
#include "Storyboard.hpp"


namespace Z
{


  
StoryboardManager& 
StoryboardManager::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new StoryboardManager();
    }
    
    return static_cast<StoryboardManager&>(*s_pInstance);
}


StoryboardManager::StoryboardManager()
{
    RETAILMSG(ZONE_VERBOSE, "StoryboardManager()");
    
    s_pResourceManagerName = "StoryboardManager";
}


StoryboardManager::~StoryboardManager()
{
    RETAILMSG(ZONE_VERBOSE, "\t~StoryboardManager()");
}


// TODO: any API that takes this many params is EVIL.
// Pass in a struct, at least, so caller can set properties by name.
RESULT
StoryboardManager::CreateStoryboard( 
                                     IN    const   string&         name, 
                                     IN            HAnimation*     pHAnimations, 
                                     IN            UINT8           numAnimations, 
                                     IN            bool            autoRepeat, 
                                     IN            bool            autoReverse, 
                                     IN            bool            releaseOnFinish,
                                     IN            bool            deleteOnFinish, 
                                     IN            bool            isRelative,
                                     INOUT         HStoryboard*    pHandle 
                                   )
{
    RESULT      rval = S_OK;
    HStoryboard hStoryboard;
    
    Storyboard* pStoryboard = new Storyboard();
    if (!pStoryboard)
    {
        rval = E_OUTOFMEMORY;
        goto Exit;
    }

    rval = pStoryboard->Init( name, pHAnimations, numAnimations, autoRepeat, autoReverse, releaseOnFinish, deleteOnFinish, isRelative );
    if (FAILED(rval))
    {
        SAFE_DELETE(pStoryboard);
        goto Exit;
    }
    
    CHR(Add(pStoryboard->GetName(), pStoryboard, &hStoryboard));
    
    if (pHandle)
    {
        *pHandle = hStoryboard;
    }
    
Exit:
    return rval;
}



RESULT
StoryboardManager::Init( IN const string& settingsFilename )
{
    RETAILMSG(ZONE_INFO, "StoryboardManager::Init( %s )", settingsFilename.c_str());

    PerfTimer timer;
    timer.Start();

    RESULT rval = S_OK;
    char   path[MAX_PATH];
    
    //
    // Create a Settings object and load the file.
    //
    Settings mySettings;
    if ( FAILED(mySettings.Read( settingsFilename )) )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: StoryboardManager::Init( %s ): failed to load settings file", settingsFilename.c_str() );
        return E_UNEXPECTED;
    }
    

    //
    // Create each Storyboard.
    //
    UINT32 numStoryboards = mySettings.GetInt("/Storyboards.NumStoryboards");

    for (int i = 0; i < numStoryboards; ++i)
    {
        sprintf(path, "/Storyboards/Storyboard%d", i);
        //DEBUGMSG(ZONE_INFO, "Loading [%s]", path);

        Storyboard *pStoryboard = NULL;
        CreateStoryboard( &mySettings, path, &pStoryboard );
        if (!pStoryboard)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: StoryboardManager::Init( %s ): failed to create Storyboard", path);
            // Continue loading other Storyboards rather than aborting.
            continue;
        }
        
//        DEBUGMSG(ZONE_STORYBOARD, "Created Storyboard [%s]", pStoryboard->GetName().c_str());
        CHR(Add(pStoryboard->GetName(), pStoryboard));
    }
    
Exit:
    timer.Stop();
    RETAILMSG(ZONE_INFO, "Parse storyboards: %f seconds\n", timer.ElapsedSeconds());
    

    return rval;
}



RESULT
StoryboardManager::Shutdown( )
{
    RETAILMSG(ZONE_INFO, "StoryboardManager::Shutdown( )");

    RESULT rval = S_OK;

    CHR(AnimationMan.Shutdown());

Exit:
    return rval;
}



RESULT
StoryboardManager::CreateStoryboard( IN Settings* pSettings, IN const string& settingsPath, INOUT Storyboard** ppStoryboard )
{
    RESULT rval = S_OK;
    string name;
    
    if (!pSettings || "" == settingsPath || !ppStoryboard)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: StoryboardManager::CreateStoryboard): invalid argument");
        return E_INVALID_ARG;
    }
    
    Storyboard *pStoryboard = new Storyboard();
    CPR(pStoryboard);

    name = pSettings->GetString( settingsPath + ".Name" );
    CHR(pStoryboard->Init( name, pSettings, settingsPath ));

    // Caller must AddRef()
    *ppStoryboard = pStoryboard;
    
Exit:    
    return rval;
}


/*
RESULT
StoryboardManager::Create(
                          IN    const string&   name           = "", 
                          INOUT HStoryboard*    pHandle        = NULL, 
                          IN    UINT32          numAnimations  = 0,
                          IN    HAnimation*     pHAnimations   = NULL,
                          IN    bool            autoRepeat     = false,
                          IN    bool            autoReverse    = false,
                          IN    bool            deleteOnFinish = false,
                          IN    bool            relative       = false
                        )
{
    RESULT          rval        = S_OK;
    HStoryboard     hStoryboard;

    //
    // Allocate Storyboard
    //
    Storyboard* pStoryboard = new Storyboard;
    CPREx(pStoryboard, E_OUTOFMEMORY);
    
    if ("" == name) 
    {
        char randomName[MAX_NAME];
        sprintf(randomName, "Storyboard_%X", (unsigned int)Platform::Random());

        CHR(pStoryboard->Init ( randomName ));
    } 
    else
    {
        CHR(pStoryboard->Init ( name ));
    }

    StoryboardMan.Add( pStoryboard, &hStoryboard );


    //
    // Set Storyboard properties
    //
    pStoryboard->SetAutoRepeat( autoRepeat );
    pStoryboard->SetAutoReverse( autoReverse );
    pStoryboard->SetDeleteOnFinish( deleteOnFinish );
    pStoryboard->SetRelativeToCurrentState( relative );


    //
    // Add Animations
    //
    if (numAnimations && !pHAnimations)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: StoryboardManager::Create(): numAnimations = %d, but pHAnimations == NULL",
            numAnimations);
            
        rval = E_INVALID_ARG;
    }
    else 
    {
        for (int i = 0; i < numAnimations; ++i)
        {
            pStoryboard->AddAnimation( pHAnimations[i] );
        }
    }


    if (pHandle)
    {
        *pHandle = hStoryboard;
    }


Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: StoryboardManager::Create( \"%s\" ) failed: 0x%x",
            name.c_str(), 
            rval);
            
        // TODO: free the object and handle.
        DEBUGCHK(0);
    }
    else 
    {
        DEBUGMSG(ZONE_GAMEOBJECT, "StoryboardManager::Create( %4d \"%s\" )",
            pStoryboard->GetID(),
            name.c_str())
    }

    
    return rval;
}
*/



RESULT
StoryboardManager::Remove( IN HStoryboard hStoryboard )
{
    RESULT          rval = S_OK;
    Storyboard*     pStoryboard;
    
    pStoryboard = GetObjectPointer( hStoryboard );
    if (!pStoryboard)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    IGNOREHR(pStoryboard->Stop());
    m_runningStoryboardsList.remove( pStoryboard );

    ResourceManager<Storyboard>::Remove( hStoryboard );
    
Exit:
    return rval;
}




RESULT
StoryboardManager::BindTo( IN HStoryboard hStoryboard, HGameObject hGameObject )
{
    RESULT          rval = S_OK;
    Storyboard*     pStoryboard;
    
    pStoryboard = GetObjectPointer( hStoryboard );
    if (!pStoryboard)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    if (hGameObject.IsNull())
    {
        CHR(pStoryboard->UnBind());
    }
    else
    {
        CHR(pStoryboard->BindTo( hGameObject ));
    }
    
Exit:
    return rval;
}



RESULT
StoryboardManager::BindTo( IN HStoryboard hStoryboard, HEffect hEffect )
{
    RESULT          rval = S_OK;
    Storyboard*     pStoryboard;
    
    pStoryboard = GetObjectPointer( hStoryboard );
    if (!pStoryboard)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    if (hEffect.IsNull())
    {
        CHR(pStoryboard->UnBind());
    }
    else
    {
        CHR(pStoryboard->BindTo( hEffect ));
    }
    
    
Exit:
    return rval;
}



RESULT
StoryboardManager::BindTo( IN HStoryboard hStoryboard, IN Object* pObject )
{
    RESULT          rval = S_OK;
    Storyboard*     pStoryboard;
    
    pStoryboard = GetObjectPointer( hStoryboard );
    if (!pStoryboard)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    if (!pObject)
    {
        CHR(pStoryboard->UnBind());
    }
    else
    {
        CHR(pStoryboard->BindTo( pObject ));
    }
    
Exit:
    return rval;
}



RESULT
StoryboardManager::BindTo( IN HStoryboard hStoryboard, IN HSprite hSprite )
{
    RESULT          rval = S_OK;
    Storyboard*     pStoryboard;
    
    pStoryboard = GetObjectPointer( hStoryboard );
    if (!pStoryboard)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    
    if (hSprite.IsNull())
    {
        CHR(pStoryboard->UnBind());
    }
    else
    {
        CHR(pStoryboard->BindTo( hSprite ));
    }
    
    
Exit:
    return rval;
}



RESULT
StoryboardManager::BindTo( IN HStoryboard hStoryboard, IN IProperty* pProperty )
{
    RESULT          rval = S_OK;
    Storyboard*     pStoryboard;
    
    pStoryboard = GetObjectPointer( hStoryboard );
    if (!pStoryboard)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    if (!pProperty)
    {
        CHR(pStoryboard->UnBind());
    }
    else
    {
        CHR(pStoryboard->BindTo( pProperty ));
    }
    
   
Exit:
    return rval;
}



RESULT
StoryboardManager::BindTo( IN HStoryboard hStoryboard, IN HLayer hLayer )
{
    RESULT          rval = S_OK;
    Storyboard*     pStoryboard;
    
    pStoryboard = GetObjectPointer( hStoryboard );
    if (!pStoryboard)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    
    if (hLayer.IsNull())
    {
        CHR(pStoryboard->UnBind());
    }
    else
    {
        CHR(pStoryboard->BindTo( hLayer ));
    }
    
    
Exit:
    return rval;
}



RESULT
StoryboardManager::BindTo( IN HStoryboard hStoryboard, IN HParticleEmitter hEmitter )
{
    RESULT          rval = S_OK;
    Storyboard*     pStoryboard;
    
    pStoryboard = GetObjectPointer( hStoryboard );
    if (!pStoryboard)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    
    if (hEmitter.IsNull())
    {
        CHR(pStoryboard->UnBind());
    }
    else
    {
        CHR(pStoryboard->BindTo( hEmitter ));
    }
    
    
Exit:
    return rval;
}





RESULT
StoryboardManager::CallbackOnFinished( IN HStoryboard handle, const ICallback& callback )
{
    RESULT      rval = S_OK;
    Storyboard* pStoryboard;
    
    if ( callback.IsNull() )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: StoryboardManager::CallbackOnFinished( %d, 0x%x ): callback IsNull",
            (UINT32)handle, (UINT32)&callback);
            
        rval = E_INVALID_ARG;
        goto Exit;
    }

    pStoryboard = GetObjectPointer( handle );
    if (!pStoryboard)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    CHR(pStoryboard->CallbackOnFinished( callback ));
    
Exit:
    return rval;
}



RESULT
StoryboardManager::SetAutoRepeat( IN HStoryboard handle, bool willAutoRepeat )
{
    RESULT rval = S_OK;
    
    Storyboard* pStoryboard = GetObjectPointer( handle );
    if (!pStoryboard)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    CHR(pStoryboard->SetAutoRepeat( willAutoRepeat ));
    
Exit:
    return rval;
}



RESULT
StoryboardManager::SetAutoReverse( IN HStoryboard handle, bool willAutoReverse )
{
    RESULT rval = S_OK;
    
    Storyboard* pStoryboard = GetObjectPointer( handle );
    if (!pStoryboard)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    CHR(pStoryboard->SetAutoReverse( willAutoReverse ));
    
Exit:
    return rval;
}



RESULT
StoryboardManager::SetReleaseOnFinish( IN HStoryboard handle, bool willReleaseTargetOnFinish )
{
    RESULT rval = S_OK;
    
    Storyboard* pStoryboard = GetObjectPointer( handle );
    if (!pStoryboard)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    CHR(pStoryboard->SetReleaseOnFinish( willReleaseTargetOnFinish ));
    
Exit:
    return rval;
}



RESULT
StoryboardManager::SetDeleteOnFinish( IN HStoryboard handle, bool willDeleteOnFinish )
{
    RESULT rval = S_OK;
    
    Storyboard* pStoryboard = GetObjectPointer( handle );
    if (!pStoryboard)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    CHR(pStoryboard->SetDeleteOnFinish( willDeleteOnFinish ));
    
Exit:
    return rval;
}



RESULT
StoryboardManager::SetRelativeToCurrentState( IN HStoryboard handle, bool isRelativeToCurrentState  )
{
    RESULT rval = S_OK;
    
    Storyboard* pStoryboard = GetObjectPointer( handle );
    if (!pStoryboard)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    CHR(pStoryboard->SetRelativeToCurrentState( isRelativeToCurrentState ));
    
Exit:
    return rval;
}





RESULT
StoryboardManager::Start( IN HStoryboard handle )
{
    RESULT rval = S_OK;
    
    CHR(Start(GetObjectPointer( handle )));
    
Exit:
    return rval;
}

    

RESULT
StoryboardManager::Stop( IN HStoryboard handle )
{
    RESULT rval = S_OK;
    
    CHR(Stop(GetObjectPointer( handle )));

Exit:
    return rval;
}



RESULT
StoryboardManager::Pause( IN HStoryboard handle )
{
    RESULT rval = S_OK;
    
    CHR(Pause(GetObjectPointer( handle )));

Exit:
    return rval;
}



RESULT
StoryboardManager::Start( IN Storyboard* pStoryboard )
{
    RESULT rval = S_OK;
    
    CPR(pStoryboard);

/*
    if (!pStoryboard->IsStarted())
    {
        pStoryboard->Start();
        m_runningStoryboardsList.push_front( pStoryboard );
    }
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: StoryboardManager::Start( %s ): already started", pStoryboard->GetName().c_str());
        rval = E_FAIL;
    }
*/
    
    if (!pStoryboard->IsStarted())
    {
        m_runningStoryboardsList.push_front( pStoryboard );
        pStoryboard->Start();
    }

///    pStoryboard->Start();

Exit:
    return rval;
}

    

RESULT
StoryboardManager::Stop( IN Storyboard* pStoryboard )
{
    RESULT rval = S_OK;
    
    CPR(pStoryboard);

    if (pStoryboard->IsStarted() || pStoryboard->IsPaused())
    {
        pStoryboard->Stop();

        StoryboardListIterator ppStoryboard;
        ppStoryboard = find( m_runningStoryboardsList.begin(), m_runningStoryboardsList.end(), pStoryboard );
        if (ppStoryboard != m_runningStoryboardsList.end())
        {
            m_runningStoryboardsList.erase( ppStoryboard );
        }
    }
    else 
    {
//        RETAILMSG(ZONE_WARN, "WARNING: StoryboardManager::Stop( %s ): already stopped", pStoryboard->GetName().c_str());
        rval = E_INVALID_OPERATION;
    }


Exit:
    return rval;
}



RESULT
StoryboardManager::Pause( IN Storyboard* pStoryboard )
{
    RESULT rval = S_OK;
    
    CPR(pStoryboard);

    if (pStoryboard->IsStarted())
    {
//        pStoryboard->Pause();

        StoryboardListIterator ppStoryboard;
        ppStoryboard = find( m_runningStoryboardsList.begin(), m_runningStoryboardsList.end(), pStoryboard );
        if (ppStoryboard != m_runningStoryboardsList.end())
        {
            m_runningStoryboardsList.erase( ppStoryboard );
        }
    }
    else 
    {
//        RETAILMSG(ZONE_WARN, "WARNING: StoryboardManager::Pause( %s ): already paused", pStoryboard->GetName().c_str());
        rval = E_INVALID_OPERATION;
    }

    pStoryboard->Pause();

Exit:
    return rval;
}



bool
StoryboardManager::IsStarted( IN HStoryboard handle )
{
    Storyboard*     pStoryboard;
    
    pStoryboard = GetObjectPointer( handle );
    if (pStoryboard)
    {
        return pStoryboard->IsStarted();
    }
    
    return false;
}



bool
StoryboardManager::IsStopped( IN HStoryboard handle )
{
    Storyboard*     pStoryboard;
    
    pStoryboard = GetObjectPointer( handle );
    if (pStoryboard)
    {
        return pStoryboard->IsStopped();
    }
    
    return false;
}



bool
StoryboardManager::IsPaused( IN HStoryboard handle )
{
    Storyboard*     pStoryboard;
    
    pStoryboard = GetObjectPointer( handle );
    if (pStoryboard)
    {
        return pStoryboard->IsPaused();
    }
    
    return false;
}



UINT64
StoryboardManager::GetDurationMS( IN const string& name )
{
    RESULT      rval        = S_OK;
    UINT64      duration    = 0;
    HStoryboard hStoryboard;

    CHR(Get( name, &hStoryboard ));
    duration = GetDurationMS( hStoryboard );
    Release( hStoryboard );
    
Exit:    
    return duration;
}



UINT64
StoryboardManager::GetDurationMS( IN HStoryboard hStoryboard )
{
    Storyboard*     pStoryboard;
    UINT64          duration = 0;
    
    pStoryboard = GetObjectPointer( hStoryboard );
    if (pStoryboard)
    {
        duration = pStoryboard->GetDurationMS();
    }

Exit:
    return duration;
}



UINT8
StoryboardManager::GetNumAnimations( IN const string& name )
{
    RESULT      rval            = S_OK;
    UINT8       numAnimations   = 0;
    HStoryboard hStoryboard;
    
    CHR(Get( name, &hStoryboard ));
    numAnimations = GetNumAnimations( hStoryboard );
    Release( hStoryboard );
    
Exit:    
    return numAnimations;
}


    
UINT8
StoryboardManager::GetNumAnimations( IN HStoryboard hStoryboard )
{
    Storyboard*     pStoryboard;
    UINT8           numAnimations = 0;
    
    pStoryboard = GetObjectPointer( hStoryboard );
    if (pStoryboard)
    {
        numAnimations = pStoryboard->GetNumAnimations();
    }


Exit:
    return numAnimations;
}




RESULT
StoryboardManager::Update( IN HStoryboard handle, UINT64 elapsedMS )
{
    RESULT rval = S_OK;
    
    Storyboard* pStoryboard = GetObjectPointer( handle );
    if (pStoryboard && pStoryboard->IsStarted())
    {
        if (FAILED(pStoryboard->Update( elapsedMS )))
        {
            pStoryboard->Stop();

            if ( pStoryboard->GetDeleteOnFinish() )
            {
                DEBUGMSG(ZONE_STORYBOARD, "StoryboardManager::Update(): \"%s\" DELETE on finish.", pStoryboard->GetName().c_str());
                Release( pStoryboard );
            }

        }
    }
    else 
    {
        rval = E_FAIL;
    }
    
    
Exit:
    return rval;
}



RESULT
StoryboardManager::Update( UINT64 elapsedMS )
{
    RESULT rval = S_OK;


    // Release Storyboards that were marked as done on previous frame.
    StoryboardListIterator ppStoryboard;
    for (ppStoryboard = m_pendingReleaseStoryboardsList.begin(); ppStoryboard != m_pendingReleaseStoryboardsList.end(); ++ppStoryboard)
    {
        Storyboard* pStoryboard = *ppStoryboard;
        if (!pStoryboard)
            continue;
        
        DEBUGMSG(ZONE_STORYBOARD, "Releasing Storyboard [%s]", pStoryboard->GetName().c_str());
        
        CHR(Release( pStoryboard ));
    }
    m_pendingReleaseStoryboardsList.clear();



    // Update every Storyboard, which in turn will update its Animations and the targets they are bound to
    for (ppStoryboard = m_runningStoryboardsList.begin(); ppStoryboard != m_runningStoryboardsList.end(); /*++ppStoryboard*/)
    {
        Storyboard* pStoryboard = *ppStoryboard;
        DEBUGCHK(pStoryboard);


        if (Log::IsZoneEnabled(ZONE_STORYBOARD))
        {
            char str[256];
            sprintf(str, "%s", pStoryboard->GetName().c_str());
            DebugRender.Text(str, Color::Red(), 1.0f, 1.0f);
        }
        


        
        if (FAILED(pStoryboard->Update( elapsedMS )))
        {
            pStoryboard->Stop();

            //
            // NOTE: if you erase a list member while iterating the list,
            // be sure to properly increment the iterator as seen below.
            //
            ppStoryboard = m_runningStoryboardsList.erase( ppStoryboard );
        }
        else 
        {
            ++ppStoryboard;
        }
    }

Exit:
    return rval;
}



RESULT
StoryboardManager::ReleaseOnNextFrame( IN HStoryboard handle )
{
    RESULT      rval = S_OK;
    Storyboard* pStoryboard;

    pStoryboard = GetObjectPointer( handle );
    if (!pStoryboard)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    
    CHR(ReleaseOnNextFrame( pStoryboard ));

Exit:
    return rval;
}



RESULT
StoryboardManager::ReleaseOnNextFrame( IN Storyboard* pStoryboard )
{
    RESULT rval = S_OK;
    
    CPR(pStoryboard)
    
    m_pendingReleaseStoryboardsList.push_back( pStoryboard );
    
Exit:
    return rval;
}





/*
RESULT
StoryboardManager::GetInfo( IN HStoryboard handle, INOUT StoryboardInfo* pInfo )
{
    RESULT      rval        = S_OK;
    Storyboard* pStorybaord = NULL;
    
    CPR(pInfo);
    
    pStoryboard = GetObjectPointer( handle );
    if (pStorybaord)
    {
        memcpy(pInfo, pStorybaord->GetInfo(), sizeof(StoryboardInfo));
        CPREx(pInfo, E_UNEXPECTED);
    }
    
Exit:
    return rval;
}
*/



} // END namespace Z



