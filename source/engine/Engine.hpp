#pragma once

#include "Types.hpp"
#include "Errors.hpp"
#include "PerfTimer.hpp"
#include "Util.hpp"
#include "IRenderer.hpp"
#include "RenderContext.hpp"
#include "Camera.hpp"
#include "TouchInput.hpp"
#include "Accelerometer.hpp"   
#include "GameObjectManager.hpp"
#include "TextureManager.hpp"
#include "EffectManager.hpp"
#include "FontManager.hpp"
#include "SpriteManager.hpp"
#include "SoundManager.hpp"
#include "DebugRenderer.hpp"
#include "SceneManager.hpp"
#include "ParticleManager.hpp"
#include "StoryboardManager.hpp"
#include "Game.hpp"

   
namespace Z
{   
   
    
      
class Engine
{
public:
    // Singleton / static class
    static  RESULT                  Init                    ( );
    static  RESULT                  LoadResources           ( );
    static  RESULT                  UnloadResources         ( );
    static  RESULT                  Shutdown                ( );
    static  RESULT                  SetRenderContext        ( RenderContext* pContext );
    static  RESULT                  Update                  ( );
    static  RESULT                  Render                  ( );
    static  RESULT                  Pause                   ( );
    static  RESULT                  Resume                  ( );
    static  bool                    IsPaused                ( );

    static  GameObjectManager&      GetGameObjectManager    ( );
    static  EffectManager&          GetEffectManager        ( );
    static  TextureManager&         GetTextureManager       ( );
    static  SpriteManager&          GetSpriteManager        ( );
    static  SoundManager&           GetSoundManager         ( );
    static  IRenderer&              GetRenderer             ( );
    static  Camera&                 GetCamera               ( );
    static  TouchInput&             GetTouchInput           ( );
    static  Accelerometer&          GetAccelerometer        ( );
    static  SceneManager&           GetSceneManager         ( );
    static  ParticleManager&        GetParticleManager      ( );
    static  StoryboardManager&      GetStoryboardManager    ( );
    
protected:
    Engine();
    Engine( const Engine& rhs );
    Engine& operator=( const Engine& rhs );
    virtual ~Engine();

protected:
    static bool                     s_isInitialized;
    static bool                     s_resourcesLoaded;
    static bool                     s_isPaused;
    static GameObjectManager*       s_pGameObjectManager;
    static EffectManager*           s_pEffectManager;
    static TextureManager*          s_pTextureManager;
    static SpriteManager*           s_pSpriteManager;
    static SoundManager*            s_pSoundManager;
    static IRenderer*               s_pRenderer;
    static RenderContext*           s_pRenderContext;
    static Camera*                  s_pCamera;
    static TouchInput*              s_pTouchInput;
    static Accelerometer*           s_pAccelerometer;
    static SceneManager*            s_pSceneManager;
    static ParticleManager*         s_pParticleManager;
    static StoryboardManager*       s_pStoryboardManager;
};


#define GameObjects     Z::Engine::GetGameObjectManager()
#define Effects         Z::Engine::GetEffectManager()
#define Textures        Z::Engine::GetTextureManager()
#define Sprites         Z::Engine::GetSpriteManager()
#define Sounds          Z::Engine::GetSoundManager()
#define Renderer        Z::Engine::GetRenderer()
#define GameCamera      Z::Engine::GetCamera()
#define TouchScreen     Z::Engine::GetTouchInput()
#define Accelerometer   Z::Engine::GetAccelerometer()
#define Scenes          Z::Engine::GetSceneManager()
#define Particles       Z::Engine::GetParticleManager()
#define Storyboards     Z::Engine::GetStoryboardManager()

} // END namespace Z
