#pragma once

#include "Types.hpp"
#include "Handle.hpp"
#include "Object.hpp"
#include "IScene.hpp"
#include "QuartzRenderTarget.hpp"

#import <UIKit/UIKit.h>

namespace Z
{


//=============================================================================
//
// A Scene instance.
//
// Scenes encapsulate a stage of the game with one or more UIViews and/or Layers.
// E.g. home screen, play screen, pause screen.
//
// Scenes may be shown and hidden with Effects and Storyboard transitions.
//
//=============================================================================
class Scene : virtual public Object, virtual public IScene
{
public:
    Scene();
    virtual ~Scene();
    
    RESULT              Init            ( IN const string& name, UIViewController* viewController, UIWindow* window );
    virtual RESULT      Show            ( IN HEffect hEffect = HEffect::NullHandle(), IN HStoryboard hStoryboard = HStoryboard::NullHandle() ); // TODO: storyboards for both Sprite and Effect?
    virtual RESULT      Hide            ( IN HEffect hEffect = HEffect::NullHandle(), IN HStoryboard hStoryboard = HStoryboard::NullHandle() );
    virtual bool        IsAnimating     ( )  { return m_isAnimating; }
    virtual HEffect     GetEffect       ( )  { return m_hEffect;     }

    // TEST:
    virtual RESULT      Draw            ( const mat4&   matParentWorld );
    

protected:
    RESULT              RenderToTexture();
    
    static void         OnDoneShowing(void* context);
    static void         OnDoneHiding (void* context);

    
protected:
    UIViewController*   m_viewController;
    UIWindow*           m_parentWindow;
    HEffect             m_hEffect;
    HStoryboard         m_hStoryboard;

    bool                m_isAnimating;
    bool                m_isBeingShown;
    bool                m_isBeingHidden;

    HSprite             m_hSpriteForShowing;      // The Scene is rendered into a Sprite for Show/Hide animations.
    HSprite             m_hSpriteForHiding;       // The Scene is rendered into a Sprite for Show/Hide animations.


    static QuartzRenderTarget*  s_pQuartzRenderTarget;
};
//typedef Handle<Scene> HScene;



} // END namespace Z
