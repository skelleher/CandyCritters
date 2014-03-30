#pragma once

#include "ResourceManager.hpp"
#include "Handle.hpp"
#include "Object.hpp"
#include "Settings.hpp"
#include "GameObject.hpp"
#include "Sound.hpp"

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#include <string>
using std::string;


namespace Z
{



//=============================================================================
//
// SoundManager
//
//=============================================================================

class SoundManager : public ResourceManager<Sound>
{
public:
    static  SoundManager& Instance();
    
    virtual RESULT  Init                        ( IN const string& settingsFilename );
   

    RESULT          Update                      ( UINT64 elapsedMS );
    RESULT          Pause                       ( );
    RESULT          Resume                      ( );

    RESULT          SetFXVolume                 ( float volume );
    RESULT          SetMusicVolume              ( float volume );

    inline float    GetFXVolume                 ( ) { return m_fxVolume;    }
    inline float    GetMusicVolume              ( ) { return m_musicVolume; }

    // TODO: mute/unmute support

    virtual RESULT  Release                     ( IN HSound handle );


    RESULT          Play                        ( IN HSound handle /* TODO: delay, volume, loop, position, etc.  */ );
    RESULT          Play                        ( IN const string& name /* TODO: delay, volume, loop, position, etc.  */ );
    RESULT          Stop                        ( IN HSound handle );
    RESULT          Pause                       ( IN HSound handle );

//    RESULT          CallbackOnFinished          ( IN HSound handle, ICallback& callback );

   
    RESULT          SetVolume                   ( IN HSound handle, float volume                    );
    RESULT          SetAutoRepeat               ( IN HSound handle, bool willAutoRepeat             );
    RESULT          SetDeleteOnFinish           ( IN HSound handle, bool willDeleteOnFinish         );
    RESULT          SetFadeDuration             ( IN HSound handle, UINT32 durationMS               );
    RESULT          SetPosition                 ( IN HSound handle, IN WORLD_POSITION& position     );
    RESULT          SetVelocity                 ( IN HSound handle, IN vec3& velocity               );

    float           GetVolume                   ( IN HSound handle );
    UINT64          GetDurationMS               ( IN HSound handle );
    bool            GetAutoRepeat               ( IN HSound handle );
    bool            GetDeleteOnFinish           ( IN HSound handle );
    UINT32          GetFadeDuration             ( IN HSound handle );
    WORLD_POSITION  GetPosition                 ( IN HSound handle );
    vec3            GetVelocity                 ( IN HSound handle );


protected:
    SoundManager();
    SoundManager( const SoundManager& rhs );
    SoundManager& operator=( const SoundManager& rhs );
    virtual ~SoundManager();
 
protected:
    RESULT  InitOpenAL();
    RESULT  ShutdownOpenAL();
    RESULT  CreateSound( IN const Settings* pSettings, IN const string& settingsPath, INOUT Sound** ppSound );
    ALuint  NextAvailableSourceID();

    
protected:
	ALCcontext* m_alcontext;
	ALCdevice*  m_aldevice; 

    #define DEFAULT_VOLUME 0.75f
    float       m_fxVolume;
    float       m_musicVolume;

    #define MAX_SIMULTANEOUS_SOUNDS 32
    typedef vector<ALuint>          SourceList;
    typedef SourceList::iterator    SourceListIterator;
    SourceList                      m_sourceList;
    
    // Map of handles to currently-running Sounds
    typedef vector<Sound*>      SoundList;
    typedef SoundList::iterator SoundListIterator;
    SoundList                   m_playingSoundsList;


friend class Sound;
};

#define SoundMan ((SoundManager&)SoundManager::Instance())



} // END namespace Z


