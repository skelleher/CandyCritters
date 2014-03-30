#pragma once

#include "Types.hpp"
#include "Handle.hpp"
#include "Object.hpp"
#include "IDrawable.hpp"
#include "Time.hpp"

#include <vector>
#include <string>
using std::string;


//#define USE_POINT_SPRITES

#ifdef USE_POINT_SPRITES
    #define VERTS_PER_PARTICLE 1
#else
    #define VERTS_PER_PARTICLE 6
#endif


namespace Z
{


//
// The fields of this structure are defined to be compatible with Particle Designer
// from 71squared.com.
//
struct Particle
{
    vec3            vStartPosition;
	vec3            vPosition;
	vec3            vVelocity;
	Color           color;
	Color           deltaColor;
    float           fRadialAcceleration;
    float           fTangentialAcceleration;
	float           fRadius;
	float           fRadiusDelta;
	float           fAngle;
	float           fDegreesPerSecond;
	float           fParticleSize;
	float           fParticleSizeDelta;
	float           fTimeToLiveSec;
};


typedef enum
{
    PARTICLE_EMITTER_TYPE_UNKNOWN = -1,
    PARTICLE_EMITTER_TYPE_GRAVITY,
    PARTICLE_EMITTER_TYPE_RADIAL,
} ParticleEmitterType;




//=============================================================================
//
// A ParticleEmitter instance.
//
//=============================================================================
class ParticleEmitter : virtual public Object, IDrawable
{
public:
    ParticleEmitter();
    virtual ~ParticleEmitter();
    ParticleEmitter* Clone() const;

    RESULT              InitFromFile    ( IN const string& filename         );

    RESULT              Start           ( );
    RESULT              Stop            ( );
    RESULT              Pause           ( )         { m_isPaused  = true;  return S_OK; }
    
    bool                IsStarted       ( ) const   { return m_isStarted;  }
    bool                IsStopped       ( ) const   { return !m_isStarted; }
    bool                IsPaused        ( ) const   { return m_isPaused;   }
            
    RESULT              Update          ( UINT64 elapsedMS );

    RESULT              Draw            ( const mat4&   matParentWorld );

    RESULT              SetDeleteOnFinish( bool willDeleteOnFinish        )  { m_deleteOnFinish = willDeleteOnFinish; return S_OK; }
    bool                GetDeleteOnFinish( )                                 { return m_deleteOnFinish; }


	// IDrawable
    inline RESULT       SetVisible      ( bool          isVisible           )   { m_isVisible       = isVisible;                return S_OK; }
    inline RESULT       SetPosition     ( const vec3&   vPos                )   { m_vSourcePosition = vPos;                     return S_OK; }
    inline RESULT       SetRotation     ( const vec3&   vRotationDegrees    )   { /* unsupported */                             return S_OK; }
    inline RESULT       SetScale        ( float         scale               )   { m_fScale          = scale;                    return S_OK; }
    inline RESULT       SetOpacity      ( float         opacity             )   { m_fOpacity        = CLAMP(opacity, 0.0, 1.0); return S_OK; }
    RESULT              SetEffect       ( HEffect       hEffect             );
    RESULT              SetColor        ( const Color&  color               )   { /* unsupported */                             return S_OK; }
    inline RESULT       SetShadow       ( bool          hasShadow           )   { return S_OK; }

    inline bool         GetVisible      ( )                                     { return m_isVisible;       }
    inline vec3         GetPosition     ( )                                     { return m_vSourcePosition; }
    inline vec3         GetRotation     ( )                                     { return vec3(0,0,0);       }  /* unsupported */
    inline float        GetScale        ( )                                     { return m_fScale;          }
    inline float        GetOpacity      ( )                                     { return m_fOpacity;        }
    inline HEffect      GetEffect       ( )                                     { return m_hEffect;         }
    inline AABB         GetBounds       ( )                                     { return m_bounds;          }
    inline Color        GetColor        ( )                                     { return Color::Clear();    } /* unsupported */
    inline bool         GetShadow       ( )                                     { return false;             }

    inline UINT64       GetDurationMS   ( )                                     { return m_durationMS;      }
    
    virtual IProperty*  GetProperty     ( const string& name ) const;

protected:
    ParticleEmitter( const ParticleEmitter& rhs );
    ParticleEmitter& operator=( const ParticleEmitter& rhs );

    RESULT              InitParticles   ( );
    RESULT              InitParticle    ( IN Particle* pParticle );
    RESULT              SpawnParticle   ( );
    

protected:
    UINT64          m_startTimeMS;
    UINT64          m_previousFrameMS;
    UINT64          m_durationMS;
    float           m_fDurationSec;
    float           m_fEmitCounter;

    HEffect         m_hEffect;
    bool            m_deleteOnFinish;
    bool            m_isVisible;
    float           m_fScale;
    float           m_fOpacity;
    AABB            m_bounds;
    bool            m_isShadowEnabled;
    bool            m_isStarted;
    bool            m_isPaused;


    // The ParticleEmitter attributes are mainly influenced by Particle Designer from 71squared.com.
    // Particle Designer doesn't understand 3D, but we use 3D coordinates to be future-proof.
    string              m_textureFilename;
    HTexture            m_hTexture;
    vec3                m_vSourcePosition;
    vec3                m_vSourcePositionVariance;
    float               m_fSpeed;
    float               m_fSpeedVariance;
    float               m_fParticleLifeSpan;
    float               m_fParticleLifeSpanVariance;
    float               m_fAngle;
    float               m_fAngleVariance;
    vec3                m_vGravity;
    float               m_fRadialAcceleration;
    float               m_fTangentialAcceleration;
    float               m_fRadialAccelVariance;
    float               m_fTangentialAccellVariance;
    Color               m_startColor;
    Color               m_startColorVariance;
    Color               m_finishColor;
    Color               m_finishColorVariance;
    UINT32              m_maxParticles;
    float               m_fStartParticleSize;
    float               m_fStartParticleSizeVariance;
    float               m_fFinishParticleSize;
    float               m_fFinishParticleSizeVariance;
    ParticleEmitterType m_emitterType;
    float               m_fMaxRadius;
    float               m_fMaxRadiusVariance;
    float               m_fMinRadius;
    float               m_fRotatePerSecond;
    float               m_fRotatePerSecondVariance;
    UINT32              m_blendFuncSource;              // This is a GL_xxx blend function
    UINT32              m_blendFuncDestination;         // This is a GL_xxx blend function
    float               m_fRotationStart;
    float               m_fRotationStartVariance;
    float               m_fRotationEnd;
    float               m_fRotationEndVariance;
    float               m_fEmitPerSecond;

    typedef std::vector<Particle>       ParticleList;
    typedef ParticleList::iterator      ParticleListIterator;
    
    UINT32              m_numActiveParticles;
    ParticleList        m_activeParticles;
    
    Vertex*             m_pVertices;
    
    
//----------------
// Class data
//----------------

    //
    // Exported Properties for animation/scripting
    //
    static       PropertySet        s_properties;
    
};

typedef Handle<ParticleEmitter> HParticleEmitter;

typedef std::list<HParticleEmitter>     ParticleEmitterList;
typedef ParticleEmitterList::iterator   ParticleEmitterListIterator;


} // END namespace Z
