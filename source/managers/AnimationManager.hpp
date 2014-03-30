#pragma once

#include "ResourceManager.hpp"
#include "Handle.hpp"
#include "Object.hpp"
#include "Settings.hpp"
#include "GameObject.hpp"
#include "Animation.hpp"

#include <string>
using std::string;


namespace Z
{



//=============================================================================
//
// AnimationManager
//
//=============================================================================

class AnimationManager : public ResourceManager<Animation>
{
public:
    static  AnimationManager& Instance();
    
    virtual RESULT  Init                        ( IN const string& settingsFilename );
    
    RESULT          CreateAnimation             ( 
                                                  IN    const   string&             name                = "",
                                                  IN    const   string&             propertyName        = "",
                                                  IN            PropertyType        propertyType        = PROPERTY_UNKNOWN, 
                                                  IN            InterpolatorType    interpolatorType    = INTERPOLATOR_TYPE_UNKNOWN, 
                                                  IN            KeyFrameType        keyFrameType        = KEYFRAME_TYPE_UNKNOWN, 
                                                  IN            KeyFrame*           pKeyFrames          = NULL, 
                                                  IN            UINT8               numKeyFrames        = 0, 
                                                  IN            bool                isRelative          = false, 
                                                  INOUT         HAnimation*         pHandle             = NULL
                                                );
    
    // TODO: only public for Animation; make friend class?                                                                                            
    RESULT          ReleaseOnNextFrame          ( IN HAnimation handle     );
    RESULT          ReleaseOnNextFrame          ( IN Animation* pAnimation );   // TODO: make all pointer methods PRIVATE and FRIEND the resource class.
    
    RESULT          BindTo                      ( IN HAnimation handle, IProperty& property );
    RESULT          BindTo                      ( IN HAnimation handle, IProperty* property );

    RESULT          SetAutoRepeat               ( IN HAnimation handle, bool willAutoRepeat     );
    RESULT          SetAutoReverse              ( IN HAnimation handle, bool willAutoReverse    );
    RESULT          SetDeleteOnFinish           ( IN HAnimation handle, bool willDeleteOnFinish );

    RESULT          CallbackOnFinished          ( IN HAnimation handle, ICallback& callback );

    RESULT          Start                       ( IN HAnimation handle );
    RESULT          Stop                        ( IN HAnimation handle );
    RESULT          Pause                       ( IN HAnimation handle );

    RESULT          Update                      ( UINT64 elapsedMS                          );
    RESULT          Update                      ( IN HAnimation handle, UINT64 elapsedMS    );
    
    // TODO: replace with struct AnimationInfo?
    UINT64          GetDurationMS               ( IN HAnimation handle );
    UINT8           GetNumKeyframes             ( IN HAnimation handle );
    PropertyType    GetPropertyType             ( IN HAnimation handle );
    const string&   GetPropertyName             ( IN HAnimation handle );
    
protected:
    AnimationManager();
    AnimationManager( const AnimationManager& rhs );
    AnimationManager& operator=( const AnimationManager& rhs );
    virtual ~AnimationManager();
 
// public so that Storyboard can call it.
public:
    // TODO: rename all these to CreateFromFile( );
    RESULT  CreateAnimation( IN const Settings* pSettings, IN const string& settingsPath, INOUT Animation** ppAnimation );
    
protected:
    // Map of handles to currently-running animations
    typedef vector<Animation*>      AnimationList;
    typedef AnimationList::iterator AnimationListIterator;
    
    AnimationList   m_runningAnimationsList;
    AnimationList   m_pendingReleaseAnimationsList;
};

#define AnimationMan ((AnimationManager&)AnimationManager::Instance())



} // END namespace Z


