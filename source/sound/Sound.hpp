#pragma once

#include "Types.hpp"
#include "Handle.hpp"
#include "Object.hpp"
#include "Callback.hpp"
#include "FileManager.hpp"
#include "Settings.hpp"

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#include <list>
using std::list;



namespace Z
{

//=============================================================================
//
// Sound types.
//
//=============================================================================


class Sound : virtual public Object
{
public:
    Sound();
    virtual ~Sound();
    Sound* Clone() const;
    
    
    RESULT          Init                        ( IN const string& name, IN const string& filename, bool isMusic );
    RESULT          Init                        ( IN const string& name, IN const Settings* pSettings, IN const string& settingsPath );
    
    RESULT          CallbackOnFinished          ( ICallback& callback );
    
    RESULT          Play                        ( );
    RESULT          Stop                        ( );
    RESULT          Pause                       ( );
    
    bool            IsStarted                   ( ) const                           { return m_isStarted;  }
    bool            IsStopped                   ( ) const                           { return (!m_isStarted) || IsFinished(); }
    bool            IsPaused                    ( ) const                           { return m_isPaused;   }
    bool            IsMusic                     ( ) const                           { return m_isMusic;    }
    bool            IsFinished                  ( ) const;
    
//    RESULT          Update                      ( UINT64 elapsedMS );

    RESULT          SetVolume                   ( float volume                      );
    RESULT          SetAutoRepeat               ( bool willAutoRepeat               );
    RESULT          SetPosition                 ( IN const WORLD_POSITION& position );
    RESULT          SetVelocity                 ( IN const vec3& velocity           );
    RESULT          SetDeleteOnFinish           ( bool willDeleteOnFinish           )   { m_deleteOnFinish  = willDeleteOnFinish;   return S_OK; }
    RESULT          SetFadeDuration             ( UINT32 durationMS                 )   { m_fadeDurationMS  = durationMS;           return S_OK; }

    float           GetVolume                   ( )                                     { return m_volume;          }
    UINT64          GetDurationMS               ( )                                     { return m_durationMS;      }
    bool            GetAutoRepeat               ( )                                     { return m_autoRepeat;      }
    bool            GetDeleteOnFinish           ( )                                     { return m_deleteOnFinish;  }
    UINT32          GetFadeDuration             ( )                                     { return m_fadeDurationMS;  }
    WORLD_POSITION  GetPosition                 ( )                                     { return m_position;        }
    vec3            GetVelocity                 ( )                                     { return m_velocity;        }


protected:
    bool            m_isStarted;
    bool            m_isPaused;
    bool            m_autoRepeat;
    bool            m_deleteOnFinish;
    bool            m_isMusic;
    bool            m_fade;
    UINT32          m_fadeDurationMS;

    UINT64          m_durationMS;
    UINT64          m_startTimeMS;
    float           m_volume;

    WORLD_POSITION  m_position;
    vec3            m_velocity;

//    ICallback*      m_pCallbackOnFinished;

    ALuint          m_sourceID;
    ALuint          m_bufferID;
	ALenum          m_format;
	ALsizei         m_size;
	ALsizei         m_sampleRate;


    typedef std::map<ALuint, UINT32>    SoundBufferMap;
    typedef SoundBufferMap::iterator    SoundBufferMapIterator;

    static SoundBufferMap               s_soundBufferRefCount;

//    typedef std::list<SoundBuffer>      SoundBufferList;
//    typedef SoundBufferList::iterator   SoundBufferListIterator;

//    UINT32          m_bufferSize;
//    //UINT8           m_numBuffers;
//    SoundBufferList m_bufferList;
    
};


typedef Handle<Sound> HSound;



} // END namespace Z

