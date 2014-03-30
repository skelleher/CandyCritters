#pragma once

#include "Animation.hpp"
#include "AnimationManager.hpp"
#include "GameObject.hpp"
#include "Callback.hpp"
#include "Layer.hpp"

#include <string>
using std::string;


namespace Z
{


//
// Bind an Animation to a Property.
// E.G. bind an opacity animation to the GameObject's opacity.
// A Storyboard groups together all the Animations for a given object.
class AnimationBinding
{
public:
    AnimationBinding() :
        m_propertyType(PROPERTY_UNKNOWN),
        m_propertyName(""),
        m_hasFinished(false),
        m_isBound(false)
    { 
    }
    
    virtual ~AnimationBinding()
    { 
        DEBUGMSG(ZONE_OBJECT | ZONE_VERBOSE, "\t~AnimationBinding( 0x%x )", this);
        AnimationMan.Release( m_hAnimation ); 
    }
    
    
public:
    HAnimation          m_hAnimation;
    string              m_animationName;
    PropertyType        m_propertyType;
    string              m_propertyName;
    bool                m_hasFinished;
    bool                m_isBound;
};



//=============================================================================
//
// Multiple Animations are grouped together in a Storyboard,
// which animates multiple properties of a single object.
//
//=============================================================================

class Storyboard : virtual public Object
{
public:
    Storyboard();
    virtual ~Storyboard();
    Storyboard* Clone () const;

    
    RESULT          Init                        ( 
                                                  IN const string& name, 
                                                  IN HAnimation* pHAnimations, 
                                                  UINT8 numAnimations, 
                                                  bool autoRepeat, 
                                                  bool autoReverse, 
                                                  bool releaseTargetOnFinish,
                                                  bool deleteOnFinish, 
                                                  bool isRelative
                                                );
    
                                                                                                
    RESULT          Init                        ( 
                                                  IN const string& name, 
                                                  IN const Settings* pSettings, 
                                                  IN const string& settingsPath 
                                                );
    
    
    // PROBLEM: we want to bind to any object (e.g. menus, sounds, etc.) but we have no generic Handle.
    RESULT          BindTo                      ( HGameObject       hGameObject );    // TODO: bind to multiple GOs?
    RESULT          BindTo                      ( HEffect           hEffect     );
    RESULT          BindTo                      ( HLayer            hLayer      );
    RESULT          BindTo                      ( HSprite           hSprite     );
    RESULT          BindTo                      ( HParticleEmitter  hEmitter    );
    RESULT          BindTo                      ( Object*           pObject     );
    RESULT          BindTo                      ( IProperty*        pProperty   );  // HACK HACK: for binding to a single AnimationVariable.
    RESULT          UnBind                      ( );

    RESULT          CallbackOnFinished          ( const ICallback& callback );
    
    RESULT          Start                       ( );
    RESULT          Stop                        ( );
    RESULT          Pause                       ( );
    
    bool            IsStarted                   ( ) const   { return m_isStarted;  }
    bool            IsStopped                   ( ) const   { return !m_isStarted; }
    bool            IsPaused                    ( ) const   { return m_isPaused;   }
    bool            IsFinished                  ( ) const   { return m_numFinishedAnimations == m_numAnimations; }
            
    RESULT          Update                      ( UINT64 elapsedMS );

    RESULT          SetAutoRepeat               ( bool willAutoRepeat            )  { m_autoRepeat              = willAutoRepeat;            return S_OK; }
    RESULT          SetAutoReverse              ( bool willAutoReverse           )  { m_autoReverse             = willAutoReverse;           return S_OK; }
    RESULT          SetReleaseOnFinish          ( bool willReleaseTargetOnFinish )  { m_releaseTargetOnFinish   = willReleaseTargetOnFinish; return S_OK; }
    RESULT          SetDeleteOnFinish           ( bool willDeleteOnFinish        )  { m_deleteOnFinish          = willDeleteOnFinish;        return S_OK; }
    RESULT          SetRelativeToCurrentState   ( bool isRelativeToCurrentState  )  { m_relativeToCurrentState  = isRelativeToCurrentState;  return S_OK; }

    UINT64          GetDurationMS               ( )                                 { return m_durationMS;              }
    UINT8           GetNumAnimations            ( )                                 { return m_numAnimations;           }
    bool            GetAutoRepeat               ( )                                 { return m_autoRepeat;              }
    bool            GetAutoReverse              ( )                                 { return m_autoReverse;             }
    bool            GetDeleteOnFinish           ( )                                 { return m_deleteOnFinish;          }
    bool            GetRelativeToCurrentState   ( )                                 { return m_relativeToCurrentState;  }

protected:
    RESULT          CreateAnimationBinding      ( IN const string& animationName, INOUT AnimationBinding* pAnimationBinding );
    
protected:
    // Storyboards shall be bound to one of the following:
    HGameObject         m_hGameObject;
    HEffect             m_hEffect;
    HLayer              m_hLayer;
    HSprite             m_hSprite;
    HParticleEmitter    m_hParticleEmitter;
    Object*             m_pObject;

    // AnimationBindings bind Animations to object Properties.
    // E.G. position, scale, opacity, sprite index, etc.
    UINT8               m_numAnimations;
    UINT8               m_numFinishedAnimations;
    AnimationBinding*   m_pAnimationBindings;

    ICallback*          m_pCallbackOnFinished;

    InterpolatorType    m_interpolatorType;
    UINT64              m_startTimeMS;
    UINT64              m_durationMS;
    bool                m_isBoundToTarget;
    bool                m_isStarted;
    bool                m_isPaused;
    bool                m_autoRepeat;
    bool                m_autoReverse;
    bool                m_releaseTargetOnFinish;
    bool                m_deleteOnFinish;
    bool                m_relativeToCurrentState;   // Animation keyframes are relative to the starting value of the animation target.
};

typedef Handle<Storyboard> HStoryboard;



} // END namespace Z


