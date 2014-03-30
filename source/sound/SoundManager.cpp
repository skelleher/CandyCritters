/*
 *  SoundManager.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 3/5/11.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include "Engine.hpp"
#include "SoundManager.hpp"
#include "FileManager.hpp"
#include "Log.hpp"
#include "Handle.hpp"
#include "Settings.hpp"
#include "Platform.hpp"
#include "Types.hpp"
#include "Util.hpp"
#include "GameObjectManager.hpp"
#include "StoryboardManager.hpp"


namespace Z
{

   
    
SoundManager& 
SoundManager::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new SoundManager();
    }
    
    return static_cast<SoundManager&>(*s_pInstance);
}


SoundManager::SoundManager() :
    m_alcontext(0),
    m_aldevice(0),
    m_fxVolume(DEFAULT_VOLUME),
    m_musicVolume(DEFAULT_VOLUME)
{
    RETAILMSG(ZONE_VERBOSE, "SoundManager()");
    
    s_pResourceManagerName = "SoundManager";
}


SoundManager::~SoundManager()
{
    RETAILMSG(ZONE_VERBOSE, "\t~SoundManager()");
}



RESULT
SoundManager::Init( IN const string& settingsFilename )
{
    RETAILMSG(ZONE_INFO, "SoundManager::Init( %s )", settingsFilename.c_str());

    RESULT rval         = S_OK;
    UINT32 numSounds    = 0;
    char   path[MAX_PATH];
    
    //
    // Create a Settings object and load the file.
    //
    Settings mySettings;
    if ( FAILED(mySettings.Read( settingsFilename )) )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: SoundManager::Init( %s ): failed to load settings file", settingsFilename.c_str() );
        return E_UNEXPECTED;
    }
    

    CHR(InitOpenAL());
    
    m_fxVolume      = mySettings.GetFloat("/Sounds.FXVolume",    DEFAULT_VOLUME);
    m_musicVolume   = mySettings.GetFloat("/Sounds.MusicVolume", DEFAULT_VOLUME);


    //
    // Create each Sound.
    //
    numSounds = mySettings.GetInt("/Sounds.NumSounds");

    for (int i = 0; i < numSounds; ++i)
    {
        sprintf(path, "/Sounds/Sound%d", i);

        Sound *pSound = NULL;
        CreateSound( &mySettings, path, &pSound );
        if (!pSound)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: SoundManager::Init( %s ): failed to create Sound", path);
            // Continue loading other Sounds rather than aborting.
            continue;
        }
        
        if (pSound->IsMusic())
        {
            IGNOREHR(pSound->SetVolume( m_musicVolume ));
        }
        else
        {
            IGNOREHR(pSound->SetVolume( m_fxVolume ));
        }
        
        CHR(Add(pSound->GetName(), pSound));
    }
    
Exit:
    return rval;
}



RESULT
SoundManager::CreateSound( IN const Settings* pSettings, IN const string& settingsPath, INOUT Sound** ppSound )
{
    RESULT rval = S_OK;
    string name;
    
    if (!pSettings || "" == settingsPath || !ppSound)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: SoundManager::CreateSound(): invalid argument");
        return E_INVALID_ARG;
    }
    
    Sound *pSound = new Sound();
    CPR(pSound);

    name = pSettings->GetString( settingsPath + ".Name" );
    
    if ("" == name)
    {
        // Auto-generate the name if needed.
        char instanceName[MAX_PATH];
        sprintf(instanceName, "%s_%X", settingsPath.c_str(), (int)Platform::Random());
            
        name = string(instanceName);
    }
    
    if (FAILED(pSound->Init( name, pSettings, settingsPath )))
    {
        delete pSound;
        pSound = NULL;
    }

    // Caller must AddRef()
    *ppSound = pSound;
    
Exit:    
    return rval;
}



/*
RESULT
SoundManager::CallbackOnFinished( IN HSound handle, ICallback& callback )
{
    RESULT      rval = S_OK;
    Sound*  pSound;
    
    if ( callback.IsNull() )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: SoundManager::CallbackOnFinished( %d, 0x%x ): callback IsNull",
            (UINT32)handle, (UINT32)&callback);
            
        rval = E_INVALID_ARG;
        goto Exit;
    }

    pSound = GetObjectPointer( handle );
    if (!pSound)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    CHR(pSound->CallbackOnFinished( callback ));
    
Exit:
    return rval;
}
*/



RESULT
SoundManager::SetFXVolume( float volume )
{
    RESULT rval = S_OK;
    
    RETAILMSG(ZONE_INFO, "SoundManager::SetFXVolume( %2.2f )", volume);
    
    m_fxVolume = volume;
    
    SoundListIterator ppSound;
    for (ppSound = m_playingSoundsList.begin(); ppSound != m_playingSoundsList.end(); ++ppSound)
    {
        Sound* pSound = *ppSound;
        if (pSound && !pSound->IsMusic())
        {
            IGNOREHR(pSound->SetVolume( volume ));
        }
    }
    
    
Exit:
    return rval;
}



RESULT
SoundManager::SetMusicVolume( float volume )
{
    RESULT rval = S_OK;
    
    RETAILMSG(ZONE_INFO, "SoundManager::SetMusicVolume( %2.2f )", volume);
    
    m_musicVolume = volume;
    
    SoundListIterator ppSound;
    for (ppSound = m_playingSoundsList.begin(); ppSound != m_playingSoundsList.end(); ++ppSound)
    {
        Sound* pSound = *ppSound;
        if (pSound && pSound->IsMusic())
        {
            IGNOREHR(pSound->SetVolume( volume ));
        }
    }
    
    
Exit:
    return rval;
}



RESULT
SoundManager::Play( IN const string& name )
{
    RESULT rval = S_OK;
    HSound handle;

    GetCopy(name, &handle);

    if (!handle.IsNull())
    {
        SetDeleteOnFinish( handle, true );
        Play( handle );
    }
    
Exit:
    return rval;
}



RESULT
SoundManager::Play( IN HSound handle )
{
    RESULT rval = S_OK;
    
    Sound* pSound = GetObjectPointer( handle );
    if (pSound)
    {
        RETAILMSG(ZONE_SOUND | ZONE_VERBOSE, "SoundManager::Start( \"%s\" )", pSound->GetName().c_str());
        
        if (pSound->IsMusic())
        {
            pSound->SetVolume( m_musicVolume );
        }
        else
        {
            pSound->SetVolume( m_fxVolume );
        }
        pSound->Play();

        m_playingSoundsList.push_back(pSound);
    }
    else 
    {
        rval = E_FAIL;
    }

    
Exit:
    return rval;
}

    

RESULT
SoundManager::Stop( IN HSound handle )
{
    RESULT rval = S_OK;
    
    Sound* pSound = GetObjectPointer( handle );
    if (pSound && pSound->IsStarted())
    {
        RETAILMSG(ZONE_SOUND, "SoundManager::Stop( \"%s\" )", pSound->GetName().c_str());
        pSound->Stop();

        SoundListIterator ppSound;
        ppSound = find( m_playingSoundsList.begin(), m_playingSoundsList.end(), pSound );
        
        if (ppSound != m_playingSoundsList.end())
        {
            m_playingSoundsList.erase( ppSound );
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
SoundManager::Pause( IN HSound handle )
{
    RESULT rval = S_OK;
    
    Sound* pSound = GetObjectPointer( handle );
    if (pSound)
    {
        RETAILMSG(ZONE_SOUND | ZONE_VERBOSE, "SoundManager::Pause( \"%s\" )", pSound->GetName().c_str());
        pSound->Pause();
    }
    else 
    {
        rval = E_FAIL;
    }

    
Exit:
    return rval;
}



RESULT
SoundManager::Pause()
{
    RESULT rval = S_OK;
    
    RETAILMSG(ZONE_INFO, "SoundManager::Pause()");
    
    SoundListIterator ppSound;
    for (ppSound = m_playingSoundsList.begin(); ppSound != m_playingSoundsList.end(); ++ppSound)
    {
        Sound* pSound = *ppSound;
        if (pSound)
        {
            IGNOREHR(pSound->Pause());
        }
    }
    
    
Exit:
    return rval;
}




RESULT
SoundManager::Resume()
{
    RESULT rval = S_OK;
    
    RETAILMSG(ZONE_INFO, "SoundManager::Resume()");
    
    SoundListIterator ppSound;
    for (ppSound = m_playingSoundsList.begin(); ppSound != m_playingSoundsList.end(); ++ppSound)
    {
        Sound* pSound = *ppSound;
        if (pSound && pSound->IsPaused())
        {
            IGNOREHR(pSound->Play());
        }
    }
    
    
Exit:
    return rval;
}





RESULT
SoundManager::Update( UINT64 elapsedMS )
{
    RESULT rval = S_OK;

/*
    // Release Sounds that were marked as done on previous frame.
    SoundListIterator ppSound;
    for (ppSound = m_pendingReleaseSoundsList.begin(); ppSound != m_pendingReleaseSoundsList.end(); ++ppSound)
    {
        Sound* pSound = *ppSound;
        
        DEBUGMSG(ZONE_SOUND | ZONE_VERBOSE, "Releasing Sound [%s]", pSound->GetName().c_str());
        
        CHR(Release( pSound ));
    }
    m_pendingReleaseSoundsList.clear();
*/    

    // Check active Sounds for any that are finished, and release them.
    SoundListIterator ppSound;
    for (ppSound = m_playingSoundsList.begin(); ppSound != m_playingSoundsList.end(); /* ++ppSound */)
    {
        Sound* pSound = *ppSound;
        
        if (pSound && pSound->IsFinished())
        {
            DEBUGMSG(ZONE_SOUND, "SoundManager::Update(): sound \"%s\" finished.", pSound->GetName().c_str());
            ppSound = m_playingSoundsList.erase( ppSound );
            
            if (pSound->GetDeleteOnFinish())
            {
                DEBUGMSG(ZONE_SOUND, "SoundManager::Update(): sound \"%s\" releasing.", pSound->GetName().c_str());
                ResourceManager<Sound>::Release( pSound );
            }
        }
        else
        {        
            ++ppSound;
        }
    }

Exit:
    return rval;
}



/*
RESULT
SoundManager::Update( IN HSound handle, UINT64 elapsedMS )
{
    RESULT rval = S_OK;
    
    Sound* pSound = GetObjectPointer( handle );
    if (pSound && pSound->IsStarted())
    {
        RETAILMSG(ZONE_SOUND | ZONE_VERBOSE, "SoundManager::Update( \"%s\" )", pSound->GetName().c_str());
        
        CHR(pSound->Update( elapsedMS ));
    }
    else 
    {
        rval = E_FAIL;
    }
    
Exit:
    return rval;
}
*/


RESULT
SoundManager::Release( IN HSound handle )
{
    RESULT rval = S_OK;
    
    Sound* pSound = GetObjectPointer( handle );
    if (pSound)
    {
        pSound->Stop();
        
        SoundListIterator ppSound;
        ppSound = find( m_playingSoundsList.begin(), m_playingSoundsList.end(), pSound );
        if (ppSound != m_playingSoundsList.end())
        {
            m_playingSoundsList.erase( ppSound );
        }
        
    }

    CHR(ResourceManager<Sound>::Release( handle ));

Exit:
    return rval;
}



#pragma mark -
#pragma mark Properties

float
SoundManager::GetVolume( IN HSound handle )
{
    float rval = 0;
    
    Sound* pSound = GetObjectPointer( handle );
    if (pSound)
    {
        rval = pSound->GetVolume();
    }

    
Exit:
    return rval;
}



UINT64
SoundManager::GetDurationMS( IN HSound handle )
{
    UINT64 rval = 0;
    
    Sound* pSound = GetObjectPointer( handle );
    if (pSound)
    {
        rval = pSound->GetDurationMS();
    }

    
Exit:
    return rval;
}



bool
SoundManager::GetAutoRepeat( IN HSound handle )
{ 
    bool rval = false;
    
    Sound* pSound = GetObjectPointer( handle );
    if (pSound)
    {
        rval = pSound->GetAutoRepeat();
    }

    
Exit:
    return rval;
}



bool
SoundManager::GetDeleteOnFinish( IN HSound handle )
{ 
    bool rval = false;
    
    Sound* pSound = GetObjectPointer( handle );
    if (pSound)
    {
        rval = pSound->GetDeleteOnFinish();
    }

    
Exit:
    return rval;
}



UINT32
SoundManager::GetFadeDuration(IN HSound handle )
{ 
    UINT64 rval = 0;
    
    Sound* pSound = GetObjectPointer( handle );
    if (pSound)
    {
        rval = pSound->GetFadeDuration();
    }

    
Exit:
    return rval;
}



WORLD_POSITION
SoundManager::GetPosition( IN HSound handle )
{ 
    WORLD_POSITION rval = WORLD_POSITION(0,0,0);
    
    Sound* pSound = GetObjectPointer( handle );
    if (pSound)
    {
        rval = pSound->GetPosition();
    }

    
Exit:
    return rval;
}



vec3
SoundManager::GetVelocity( IN HSound handle )
{
    vec3 rval = vec3(0,0,0);
    
    Sound* pSound = GetObjectPointer( handle );
    if (pSound)
    {
        rval = pSound->GetVelocity();
    }

    
Exit:
    return rval;
}


RESULT
SoundManager::SetVolume( IN HSound handle, float volume )
{
    RESULT rval = S_OK;
    
    Sound* pSound = GetObjectPointer( handle );
    if (!pSound)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    CHR(pSound->SetVolume( volume ));
    
Exit:
    return rval;
}




RESULT
SoundManager::SetAutoRepeat( IN HSound handle, bool willAutoRepeat )
{
    RESULT rval = S_OK;
    
    Sound* pSound = GetObjectPointer( handle );
    if (!pSound)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    CHR(pSound->SetAutoRepeat( willAutoRepeat ));
    
Exit:
    return rval;
}




RESULT
SoundManager::SetDeleteOnFinish( IN HSound handle, bool willDeleteOnFinish )
{
    RESULT rval = S_OK;
    
    Sound* pSound = GetObjectPointer( handle );
    if (!pSound)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    CHR(pSound->SetDeleteOnFinish( willDeleteOnFinish ));
    
Exit:
    return rval;
}




RESULT
SoundManager::SetFadeDuration( IN HSound handle, UINT32 durationMS )
{
    RESULT rval = S_OK;
    
    Sound* pSound = GetObjectPointer( handle );
    if (!pSound)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    CHR(pSound->SetFadeDuration( durationMS ));
    
Exit:
    return rval;
}




RESULT
SoundManager::SetPosition( IN HSound handle, IN WORLD_POSITION& position )
{
    RESULT rval = S_OK;
    
    Sound* pSound = GetObjectPointer( handle );
    if (!pSound)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    CHR(pSound->SetPosition( position ));
    
Exit:
    return rval;
}




RESULT
SoundManager::SetVelocity( IN HSound handle, IN vec3& velocity )
{
    RESULT rval = S_OK;
    
    Sound* pSound = GetObjectPointer( handle );
    if (!pSound)
    {
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    CHR(pSound->SetVelocity( velocity ));
    
Exit:
    return rval;
}




#pragma mark -
#pragma mark OpenAL

RESULT
SoundManager::InitOpenAL()
{
    RESULT rval = S_OK;

    RETAILMSG(ZONE_SOUND, "SoundManager::InitOpenAL()");

    if (m_aldevice != 0)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: SoundManager::InitOpenAL(): has already been called.  Call ShutdownOpenAL() before calling again.");
        rval = E_FAIL;
        goto Exit;
    }

	VERIFYAL(m_aldevice = alcOpenDevice(NULL));
	if (!m_aldevice) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: SoundManager::InitOpenAL(): alcOpenDevice(NULL) failed.");
        rval = E_FAIL;
        goto Exit;
    }
    
    VERIFYAL(m_alcontext = alcCreateContext(m_aldevice, NULL));
    VERIFYAL(alcMakeContextCurrent(m_alcontext));
    VERIFYAL(alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED));
    

    // Pre-create sound sources which can be bound to buffers (sounds)
    ALuint sourceID;
    for (int i = 0; i < MAX_SIMULTANEOUS_SOUNDS; ++i) 
    {
        VERIFYAL(alGenSources(1, &sourceID));
        
        // TODO: read these from sounds.xml
        VERIFYAL(alSourcef(sourceID, AL_REFERENCE_DISTANCE, 25.0f));
        VERIFYAL(alSourcef(sourceID, AL_MAX_DISTANCE,       150.0f));
        VERIFYAL(alSourcef(sourceID, AL_ROLLOFF_FACTOR,     6.0f));
        
        m_sourceList.push_back( sourceID );
    }


Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: SoundManager::InitOpenAL(): failed, rval = 0x%x", rval);
    }
    
    return rval;
}



RESULT
SoundManager::ShutdownOpenAL()
{
    RESULT rval = S_OK;

    RETAILMSG(ZONE_INFO, "AnimationManager::ShutdownOpenAL()");

    DEBUGCHK(0);

    // release all sounds
    // release the device
    // release the context
    
Exit:
    return rval;
}



ALuint
SoundManager::NextAvailableSourceID()
{
    ALuint sourceID = 0;
    ALint  state    = 0;
    ALint  looping  = 0;

    // Find a source which is not being used at the moment
    SourceListIterator ppSource;
    for (ppSource = m_sourceList.begin(); ppSource != m_sourceList.end(); ++ppSource)
    {
        sourceID = *ppSource;
        
        VERIFYAL(alGetSourcei(sourceID, AL_SOURCE_STATE, &state));
        
        if (state != AL_PLAYING) 
        {
            DEBUGMSG(ZONE_SOUND, "NextAvailableSourceId: %d state: 0x%x (unused)", sourceID, state);
            goto Exit;
        }
    }

    // If all the sources are being used we look for the first non-looping source and steal it.
    for (ppSource = m_sourceList.begin(); ppSource != m_sourceList.end(); ++ppSource)
    {
        sourceID = *ppSource;

        VERIFYAL(alGetSourcei(sourceID, AL_LOOPING, &looping));
        
        if(!looping) 
        {
            VERIFYAL(alSourceStop(sourceID));
            DEBUGMSG(ZONE_SOUND, "NextAvailableSourceId = %d (reuse non-looping source)", sourceID);
            goto Exit;
        }
    }
    

    // If there are no looping sources to be found then just re-use the first one.
    sourceID = *m_sourceList.begin();
    VERIFYAL(alSourceStop(sourceID));
    DEBUGMSG(ZONE_SOUND | ZONE_VERBOSE, "NextAvailableSourceId = %d (reuse first source)", sourceID);

Exit:
    // HACK: force release of any Sound that just had its sourceID stolen.
    // TODO: smarter mapping of sourceID -> Sound.
    Update( 0 );
    
    return sourceID;
}


} // END namespace Z


