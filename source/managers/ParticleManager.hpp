#pragma once

#include "ResourceManager.hpp"
#include "Handle.hpp"
#include "Object.hpp"
#include "Settings.hpp"
#include "GameObject.hpp"
#include "ParticleEmitter.hpp"

#include <string>
using std::string;


namespace Z
{


#define MAXIMUM_UPDATE_RATE_HZ          60.0f	
#define DEFAULT_PARTICLE_UPDATE_RATE_HZ 30



//=============================================================================
//
// ParticleManager
//
//=============================================================================

class ParticleManager : public ResourceManager<ParticleEmitter>
{

friend class ParticleEmitter;


public:
    static  ParticleManager& Instance();
    
    virtual RESULT  Init            ( IN const string& settingsFilename );
    
    RESULT          Start           ( IN HParticleEmitter hParticleEmitter );
    RESULT          Stop            ( IN HParticleEmitter hParticleEmitter );
    RESULT          Pause           ( IN HParticleEmitter hParticleEmitter );
    
    bool            IsStarted       ( IN HParticleEmitter hParticleEmitter );
    bool            IsStopped       ( IN HParticleEmitter hParticleEmitter );
    bool            IsPaused        ( IN HParticleEmitter hParticleEmitter );
    
    UINT64          GetDurationMS   ( IN HParticleEmitter hParticleEmitter );
    UINT64          GetDurationMS   ( IN const string&    name             );

    RESULT          Update          ( UINT64 elapsedMS );
    
    RESULT          Draw            ( );
    RESULT          Draw            ( IN HParticleEmitter hParticleEmitter, IN const mat4& matParentWorld = mat4::Identity() );
    

    // Only public for ParticleEmitter; make friend class?
    RESULT          ReleaseOnNextFrame          ( IN HParticleEmitter handle           );
    RESULT          ReleaseOnNextFrame          ( IN ParticleEmitter* pParticleEmitter );   // TODO: make all pointer methods PRIVATE and FRIEND the resource class.


	// IDrawable
    // TODO: are handles worth the annoyance of having to expose every object method like this?!
    RESULT          SetVisible      ( IN HParticleEmitter HParticleEmitter, bool        isVisible        );
    RESULT          SetPosition     ( IN HParticleEmitter HParticleEmitter, const vec3& vPos             );
    RESULT          SetRotation     ( IN HParticleEmitter HParticleEmitter, const vec3& vRotationDegrees );
    RESULT          SetScale        ( IN HParticleEmitter HParticleEmitter, float       scale            );
    RESULT          SetOpacity      ( IN HParticleEmitter HParticleEmitter, float       opacity          );
    RESULT          SetEffect       ( IN HParticleEmitter HParticleEmitter, HEffect     hEffect          );
    RESULT          SetShadow       ( IN HParticleEmitter HParticleEmitter, bool        shadowEnabled    );

    bool            GetVisible      ( IN HParticleEmitter HParticleEmitter                               );
    vec3            GetPosition     ( IN HParticleEmitter HParticleEmitter                               );
    vec3            GetRotation     ( IN HParticleEmitter HParticleEmitter                               );
    float           GetScale        ( IN HParticleEmitter HParticleEmitter                               );
    float           GetOpacity      ( IN HParticleEmitter HParticleEmitter                               );
    HEffect         GetEffect       ( IN HParticleEmitter HParticleEmitter                               );
    AABB            GetBounds       ( IN HParticleEmitter HParticleEmitter                               );
    bool            GetShadow       ( IN HParticleEmitter HParticleEmitter                               );



protected:
    ParticleManager();
    ParticleManager( const ParticleManager& rhs );
    ParticleManager& operator=( const ParticleManager& rhs );
    virtual ~ParticleManager();
 
    RESULT          CreateParticleEmitter ( IN const string& name, INOUT HParticleEmitter* pHParticleEmitter );
    RESULT          Start                 ( IN ParticleEmitter* pParticleEmitter );
    RESULT          Stop                  ( IN ParticleEmitter* pParticleEmitter );
    RESULT          Pause                 ( IN ParticleEmitter* pParticleEmitter );
    RESULT          Draw                  ( IN ParticleEmitter* pParticleEmitter, IN const mat4& matParentWorld = mat4::Identity() );


protected:
    UINT64              m_updateIntervalMS;

    typedef std::list<ParticleEmitter*>     ParticleEmitterList;
    typedef ParticleEmitterList::iterator   ParticleEmitterListIterator;
    ParticleEmitterList  m_runningParticleEmittersList;
    ParticleEmitterList  m_pendingReleaseParticleEmittersList;
};


#define ParticleMan ((ParticleManager&)ParticleManager::Instance())



} // END namespace Z


