#pragma once

#include "ResourceManager.hpp"
#include "Handle.hpp"
#include "Object.hpp"
#include "Settings.hpp"
#include "GameObject.hpp"
#include "Layer.hpp"
#include "Storyboard.hpp"

#include <string>
using std::string;

class HAnimation;


namespace Z
{



//=============================================================================
//
// StoryboardManager
//
//=============================================================================

class StoryboardManager : public ResourceManager<Storyboard>
{
public:
    static  StoryboardManager& Instance();
    
    virtual RESULT  Init                ( IN const string& settingsFilename );
    virtual RESULT  Shutdown            ( );
    
    virtual RESULT  Remove              ( IN HStoryboard handle );
 
    
    RESULT          CreateStoryboard    ( 
                                          IN    const   string&         name, 
                                          IN            HAnimation*     pHAnimations            = NULL, 
                                          IN            UINT8           numAnimations           = 0, 
                                          IN            bool            autoRepeat              = false, 
                                          IN            bool            autoReverse             = false,
                                          IN            bool            releaseTargetOnFinish   = true,
                                          IN            bool            deleteOnFinish          = true, 
                                          IN            bool            isRelative              = false,
                                          INOUT         HStoryboard*    pHandle                 = NULL
                                        );

    // TODO: Need convenience method for getting/binding/starting a Storyboard in one line of code.
    // StoryboardMan.RunWithTarget( "Name", hTarget, &hStoryboard );

    RESULT          BindTo              ( IN HStoryboard hStoryboard, IN HGameObject        hGameObject );
    RESULT          BindTo              ( IN HStoryboard hStoryboard, IN HEffect            hEffect     );
    RESULT          BindTo              ( IN HStoryboard hStoryboard, IN HLayer             hLayer      );
    RESULT          BindTo              ( IN HStoryboard hStoryboard, IN HSprite            hSprite     );
    RESULT          BindTo              ( IN HStoryboard hStoryboard, IN HParticleEmitter   hEmitter    );
    RESULT          BindTo              ( IN HStoryboard hStoryboard, IN Object*            pObject     );
    RESULT          BindTo              ( IN HStoryboard hStoryboard, IN IProperty*         pProperty   );  // HACK HACK: for binding to a single AnimationVariable.

    RESULT          CallbackOnFinished  ( IN HStoryboard handle, const ICallback& callback );

    RESULT          SetAutoRepeat               ( IN HStoryboard handle, bool willAutoRepeat            );
    RESULT          SetAutoReverse              ( IN HStoryboard handle, bool willAutoReverse           );
    RESULT          SetReleaseOnFinish          ( IN HStoryboard handle, bool willReleaseTargetOnFinish );
    RESULT          SetDeleteOnFinish           ( IN HStoryboard handle, bool willDeleteOnFinish        );
    RESULT          SetRelativeToCurrentState   ( IN HStoryboard handle, bool isRelativeToCurrentState  );

    RESULT          Start               ( IN HStoryboard handle );
    RESULT          Stop                ( IN HStoryboard handle );
    RESULT          Pause               ( IN HStoryboard handle );

    RESULT          Start               ( IN Storyboard* pStoryboard );
    RESULT          Stop                ( IN Storyboard* pStoryboard );
    RESULT          Pause               ( IN Storyboard* pStoryboard );

    bool            IsStarted           ( IN HStoryboard handle );
    bool            IsStopped           ( IN HStoryboard handle );
    bool            IsPaused            ( IN HStoryboard handle );
    
    RESULT          Update              ( IN HStoryboard handle, UINT64 elapsedMS     );
    RESULT          Update              ( UINT64 elapsedMS );
    
    // TODO: replace with struct StoryboardInfo?
    UINT64          GetDurationMS       ( IN HStoryboard handle );
    UINT64          GetDurationMS       ( IN const string& name );
    UINT8           GetNumAnimations    ( IN HStoryboard handle );
    UINT8           GetNumAnimations    ( IN const string& nmae );


    // Only public for Storyboard; make friend class?
    // TODO: move into ResourceManager<TYPE> since this is not uncommon.
    // E.G. ParticleEmitters need the same thing.
    RESULT          ReleaseOnNextFrame          ( IN HStoryboard handle      );
    RESULT          ReleaseOnNextFrame          ( IN Storyboard* pStoryboard );   // TODO: make all pointer methods PRIVATE and FRIEND the resource class.
    
protected:
    StoryboardManager();
    StoryboardManager( const StoryboardManager& rhs );
    StoryboardManager& operator=( const StoryboardManager& rhs );
    virtual ~StoryboardManager();
 
protected:
    RESULT  CreateStoryboard( IN Settings* pSettings, IN const string& settingsPath, INOUT Storyboard** ppStoryboard );
    
    
protected:
    // Map of handles to currently-running Storyboards
    typedef list<Storyboard*>           StoryboardList;
    typedef StoryboardList::iterator    StoryboardListIterator;
    
    StoryboardList  m_runningStoryboardsList;
    StoryboardList  m_pendingReleaseStoryboardsList;
};

#define StoryboardMan ((StoryboardManager&)StoryboardManager::Instance())
//#define Storyboards   ((StoryboardManager&)StoryboardManager::Instance())



} // END namespace Z


