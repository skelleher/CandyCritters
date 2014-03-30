#pragma once

#include "ResourceManager.hpp"
#include "Handle.hpp"
#include "Object.hpp"
#include "Settings.hpp"
#include "IScene.hpp"
#include "LayerManager.hpp"
#include "SpriteManager.hpp"
#include "QuartzRenderTarget.hpp"

#include <string>
using std::string;


namespace Z
{



//=============================================================================
//
// SceneManager
//
//=============================================================================

class SceneManager : public ResourceManager<IScene>
{
public:
    static  SceneManager& Instance();
    
    virtual RESULT  Init            ( );
    
    // HACK: we pass UIViewController* as a void* to make the C++ compiler happy.
    RESULT          CreateScene     ( IN const string& name, void* viewController, INOUT HScene* phScene      );

    RESULT          Show            ( IN HScene hScene, IN HStoryboard hStoryboard = HStoryboard::NullHandle(), IN HEffect hEffect = HEffect::NullHandle() );
    RESULT          Hide            ( IN HScene hScene, IN HStoryboard hStoryboard = HStoryboard::NullHandle(), IN HEffect hEffect = HEffect::NullHandle() );

    // TEST:
    virtual RESULT  Draw            ( );

protected:
    SceneManager();
    SceneManager( const SceneManager& rhs );
    SceneManager& operator=( const SceneManager& rhs );
    virtual ~SceneManager();
 
protected:
    RESULT          CreateScene( IN const Settings* pSettings, IN const string& settingsPath, INOUT IScene** ppScene );
    
    
protected:
    static bool s_initialized;
};

#define SceneMan ((Z::SceneManager&)Z::SceneManager::Instance())



} // END namespace Z


