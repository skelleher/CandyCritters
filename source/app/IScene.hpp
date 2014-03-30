#pragma once


#include "Types.hpp"
#include "Handle.hpp"
#include "Object.hpp"
#include "EffectManager.hpp"
#include "Storyboard.hpp"


namespace Z
{


//=============================================================================
//
// A Scene instance.
//
// Scenes encapsulate a stage of the game with one or more UIViews and/or Layers.
// E.g. home screen, play screen, pause screen.
//
// Scenes may be shown and hidden with Storyboard transitions.
//
//=============================================================================
class IScene : virtual public IObject
{
public:
    virtual RESULT      Show            ( IN HEffect hEffect = HEffect::NullHandle(), IN HStoryboard hStoryboard = HStoryboard::NullHandle() )  = 0;
    virtual RESULT      Hide            ( IN HEffect hEffect = HEffect::NullHandle(), IN HStoryboard hStoryboard = HStoryboard::NullHandle() )  = 0;
    virtual bool        IsAnimating     ( ) = 0;
    virtual HEffect     GetEffect       ( ) = 0;  // TODO: get rid of this after removing debug render

    // TEST:
    virtual RESULT      Draw            ( const mat4&   matParentWorld ) = 0;
    
    
    virtual ~IScene() {};
};
typedef Handle<IScene> HScene;



} // END namespace Z
