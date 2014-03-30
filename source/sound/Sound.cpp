#include "Sound.hpp"
#include "SoundManager.hpp"
#include "Time.hpp"
#include "Audio.hpp"


namespace Z
{



// ============================================================================
//
//  Sound Implementation
//
// ============================================================================


Sound::SoundBufferMap   Sound::s_soundBufferRefCount;



#pragma mark Sound Implementation

Sound::Sound() :
//    m_pCallbackOnFinished(NULL),
    m_autoRepeat(false),
    m_deleteOnFinish(false),
    m_durationMS(0),
    m_startTimeMS(0),
    m_position(0,0,0),
    m_velocity(0,0,0),
    m_isStarted(false),
    m_isPaused(false),
    m_isMusic(false),
    m_sourceID(0),
    m_bufferID(0),
    m_format(0),
	m_size(0),
	m_sampleRate(0),
    m_volume(1.0f)
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "Sound( %4d )", m_ID);
}



Sound::~Sound()
{
    DEBUGMSG(ZONE_OBJECT | ZONE_VERBOSE, "\t~Sound( %4d, \"%s\" )", m_ID, m_name.c_str());
    
    //Stop();  // NO: might stop a sourceID that has been recycled!
    
    if (m_bufferID && 0 == --s_soundBufferRefCount[m_bufferID])
    {
        DEBUGMSG(ZONE_SOUND, "OpenAL bufferID %d refcount == 0; releaseing.", m_bufferID);
        IGNOREAL(alDeleteBuffers(1, &m_bufferID));
    }

//    SAFE_DELETE(m_pCallbackOnFinished);
}



Sound*
Sound::Clone() const
{
    Sound* pSoundClone = new Sound(*this);
   
/*     
    // DEEP COPY: m_pCallbackOnFinished
    if (m_pCallbackOnFinished)
    {
        pSoundClone->m_pCallbackOnFinished = m_pCallbackOnFinished->Clone();
    }
*/

    // Give it a new name with a random suffix
    char instanceName[MAX_NAME];
    sprintf(instanceName, "%s_%X", m_name.c_str(), (unsigned int)Platform::Random());
    pSoundClone->m_name = string(instanceName);
  
    if (m_bufferID)
    {
        s_soundBufferRefCount[m_bufferID]++;
        DEBUGMSG(ZONE_SOUND, "OpenAL bufferID %d refcount = %d", m_bufferID, s_soundBufferRefCount[m_bufferID]);
    }
      
    // Reset state
    pSoundClone->m_isStarted    = false;
    pSoundClone->m_isPaused     = false;
    pSoundClone->m_position     = vec3(0,0,0);
    pSoundClone->m_sourceID     = 0;
    
    return pSoundClone;
}



RESULT
Sound::Init( IN const string& name, IN const string& filename, bool isMusic )
{
    RESULT  rval    = S_OK;
	BYTE*   pBuffer = NULL;
    
    if (filename == "")
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Sound::Init( \"%s\", \"%s\" ): specify filename", name.c_str(), filename.c_str());
        rval = E_INVALID_ARG;
        goto Exit;
    }

    if (name == "")
    {
        char randomName[MAX_NAME];
        sprintf(randomName, "Sound_%X", (unsigned int)Platform::Random());
        m_name = randomName;
    }
    else 
    {
        m_name = name;
    }


    m_isMusic = isMusic;

    //
    // TODO: we need a separate list of buffers.
    // One buffer can be shared amongst multiple Sounds (sources), with no need
    // for re-loading.
    //
    // Better yet, define a SoundEmitter class mapped onto Sound.
    //
    
    CHR(Audio::GetOpenALDataFromFile(filename, &m_size, (ALvoid**)&pBuffer, &m_format, &m_sampleRate));


    // TODO: use alBufferDataStatic() which avoids copying, per AAPL recommendation.
    
    VERIFYAL(alGenBuffers(1, &m_bufferID));
    VERIFYAL(alBufferData(m_bufferID, m_format, pBuffer, m_size, m_sampleRate));

    s_soundBufferRefCount[m_bufferID]++;

    DEBUGMSG(ZONE_SOUND, "Sound[%4d]: \"%-32s\" size: %d sampleRate: %d format: %s", 
        m_ID, m_name.c_str(), m_size, m_sampleRate, m_format == AL_FORMAT_STEREO16 ? "AL_FORMAT_STEREO16" : "AL_FORMAT_MONO16");
    
Exit:
    if (FAILED(rval))
    {
        IGNOREAL(alDeleteBuffers(1, &m_bufferID));
    }

    SAFE_ARRAY_DELETE(pBuffer);

    return rval;
}



RESULT
Sound::Init( const string& name, const Settings* pSettings, const string& settingsPath )
{
    RESULT      rval        = S_OK;
    string      filename;
    bool        isMusic;
    
    RETAILMSG(ZONE_OBJECT, "Sound[%4d]::Init( %s )", m_ID, name.c_str());
    
    m_name = name;
    
    if (!pSettings)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Sound::Init(): NULL settings");
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    filename = pSettings->GetString( settingsPath + ".Filename"       );
    isMusic  = pSettings->GetBool  ( settingsPath + ".IsMusic", false );
    
    CHR(Init( m_name, filename, isMusic ));
    
Exit:
    return rval;
}



RESULT
Sound::Play( )
{ 
    RESULT rval = S_OK;

    if (!m_isPaused)
    {
        m_sourceID = SoundMan.NextAvailableSourceID();
        
        VERIFYAL(alSourcei (m_sourceID, AL_BUFFER,   m_bufferID));
        VERIFYAL(alSourcef (m_sourceID, AL_GAIN,     m_volume));
        VERIFYAL(alSource3f(m_sourceID, AL_POSITION, m_position.x, m_position.y, m_position.z));
        
        
        if(m_autoRepeat) 
        {
            VERIFYAL(alSourcei(m_sourceID, AL_LOOPING, AL_TRUE));
        } 
        else 
        {
            VERIFYAL(alSourcei(m_sourceID, AL_LOOPING, AL_FALSE));
        }
    }

    m_isStarted     = true;  
    m_isPaused      = false; 
    m_startTimeMS   = GameTime.GetTime();
    
    DEBUGMSG(ZONE_SOUND, "Sound::Play( \"%s\" sourceID: %d )", m_name.c_str(), m_sourceID);
    IGNOREAL(alSourcePlay(m_sourceID));

Exit:
    return rval;
}



RESULT
Sound::Stop( )
{ 
    RESULT rval = S_OK;

    DEBUGMSG(ZONE_SOUND, "Sound::Stop( \"%s\" sourceID: %d )", m_name.c_str(), m_sourceID);

    m_isStarted = false;  
    m_isPaused  = false; 

    if (m_sourceID)
    {
        VERIFYAL(alSourceStop(m_sourceID));
        m_sourceID = 0;
    }

Exit:
    return rval;
}



RESULT
Sound::Pause( )
{ 
    RESULT rval = S_OK;

    DEBUGMSG(ZONE_SOUND, "Sound::Pause( \"%s\" sourceID: %d )", m_name.c_str(), m_sourceID);

    m_isPaused = true; 

    if (m_sourceID)
    {
        VERIFYAL(alSourcePause(m_sourceID));
    }

Exit:
    return rval;
}


/*
RESULT
Sound::CallbackOnFinished( ICallback& callback )
{
    RESULT rval = S_OK;
    
    if (callback.IsNull())
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Sound::CallbackOnFinished( \"%s\", 0x%x ): Callback is NULL",
                  m_name.c_str(), (UINT32)&callback);
        
        SAFE_DELETE(m_pCallbackOnFinished);
        
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    SAFE_DELETE(m_pCallbackOnFinished);
    m_pCallbackOnFinished = callback.Clone();
    
Exit:
    return rval;
}
*/



#pragma mark -
#pragma mark Properties

bool
Sound::IsFinished() const
{
    ALint state = AL_STOPPED;
    
    // Only return true if the Sound had started playing previously.
    // Sounds that were never started will not satisfy IsFinished().
    if (m_startTimeMS == 0)
    {
        DEBUGMSG(ZONE_SOUND, "Sound::IsFinished( \"%s\" ): startTime = 0", m_name.c_str());
        return false;
    }
    
    if (m_sourceID)
    {
        VERIFYAL(alGetSourcei(m_sourceID, AL_SOURCE_STATE, &state));
    }
    
Exit:
    ////DEBUGMSG(ZONE_SOUND, "Sound::IsFinished( \"%s\" ): sourceID: %d state: 0x%x", m_name.c_str(), m_sourceID, state);
        
    return state == AL_STOPPED ? true : false;
}



RESULT
Sound::SetVolume( float volume )
{
    RESULT rval = S_OK;

    m_volume = volume;

    if (m_sourceID)
        VERIFYAL(alSourcef(m_sourceID, AL_GAIN, m_volume));
    
Exit:
    return rval;
}




RESULT
Sound::SetAutoRepeat( bool autoRepeat )
{
    RESULT rval = S_OK;
    
    m_autoRepeat = autoRepeat;

    if (m_sourceID)
        VERIFYAL(alSourcei(m_sourceID, AL_LOOPING, m_autoRepeat ? AL_TRUE : AL_FALSE));
    
Exit:
    return rval;
}



RESULT
Sound::SetPosition( const WORLD_POSITION& position )
{
    RESULT rval = S_OK;
    
    m_position = position;

    if (m_sourceID)
        VERIFYAL(alSource3f(m_sourceID, AL_POSITION, m_position.x, m_position.y, m_position.z));
    
Exit:
    return rval;
}




RESULT
Sound::SetVelocity( const vec3& velocity )
{
    RESULT rval = S_OK;
    
    m_velocity = velocity;

    if (m_sourceID)
        VERIFYAL(alSource3f(m_sourceID, AL_POSITION, m_velocity.x, m_velocity.y, m_velocity.z));
    
Exit:
    return rval;
}




} // END namespace Z



