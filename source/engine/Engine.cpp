
#include <stddef.h>
#include <string>
using std::string;

#include "Engine.hpp"
#include "Log.hpp"
#include "Platform.hpp"
#include "Settings.hpp"
#include "FileManager.hpp"
#include "TextureManager.hpp"
#include "SpriteManager.hpp"
#include "MeshManager.hpp"
#include "StoryboardManager.hpp"
#include "GameObjectManager.hpp"
#include "IRenderer.hpp"
#include "DebugRenderer.hpp"
#include "Camera.hpp"
#include "Metrics.hpp"
#include "StateMachine.hpp"
#include "LayerManager.hpp"
#include "BehaviorManager.hpp"
#include "FontManager.hpp"
#include "SoundManager.hpp"
#include "EffectManager.hpp"
#include "ParticleManager.hpp"
#include "Test.hpp"
#include "Util.hpp"
#include "FontManager.hpp"  // just for temp rendering of score and level
#include "BlurEffect.hpp"
#include "RippleEffect.hpp"
#include "Level.hpp"
#include "GameState.hpp"
#include "BoxedVariable.hpp"

#include "OpenGLES1Renderer.hpp"
#include "OpenGLES2Renderer.hpp"


namespace Z
{

// HACK HACK: display score and level
extern UINT32    g_totalScore;
extern Level*    g_pLevel;
 
// Animate the size of the score text when it changes
static float scoreScale = 1.0f;
BoxedVariable g_scoreScale( PROPERTY_FLOAT, &scoreScale );




#undef Accelerometer

//
// Static Data
//
bool                    Engine::s_isInitialized         = false;
bool                    Engine::s_resourcesLoaded       = false;
bool                    Engine::s_isPaused              = false;
GameObjectManager*      Engine::s_pGameObjectManager    = NULL;
SpriteManager*          Engine::s_pSpriteManager        = NULL;
TextureManager*         Engine::s_pTextureManager       = NULL;
SoundManager*           Engine::s_pSoundManager         = NULL;
EffectManager*          Engine::s_pEffectManager        = NULL;
IRenderer*              Engine::s_pRenderer             = NULL;
RenderContext*          Engine::s_pRenderContext        = NULL;
Camera*                 Engine::s_pCamera               = NULL;
TouchInput*             Engine::s_pTouchInput           = NULL;
Accelerometer*          Engine::s_pAccelerometer        = NULL;
SceneManager*           Engine::s_pSceneManager         = NULL;
ParticleManager*        Engine::s_pParticleManager      = NULL;
StoryboardManager*      Engine::s_pStoryboardManager    = NULL;


RESULT
Engine::Init()
{
    RESULT      rval                = S_OK;
    string      persistantFolder    = "";
    ZONE_MASK   zoneMask            = 0;
    string      path                = "/Settings/Log.";


    if (s_isInitialized)
        return S_OK;

    //
    // Open the log
    //
    if (SUCCEEDED(Platform::GetPathToPersistantStorage( &persistantFolder )))
    {
        std::string logfilename = persistantFolder + "/critters.log";
        //Log::SetFilename( logfilename );
        Log::OpenWithFilename( logfilename );
    }

    //
    // Log the UDID.
    //
    RETAILMSG(ZONE_INFO, "UDID: %s", Platform::GetDeviceUDID());

    
    //
    // Start the file manager.
    //
    FileMan.Init();
   
    
    //
    // Load the default settings.
    //
    GlobalSettings.Read( "/app/settings/settings.xml" );
    
    
    //
    // Set log zones.
    //
    for (int i = 0; i < ARRAY_SIZE(Log::ZoneMapping); ++i)
    {
        if (Log::ZoneMapping[i].name == NULL)
            break;
        
        string zoneName = path + string(Log::ZoneMapping[i].name);
        
        if (GlobalSettings.GetInt( zoneName ))
        {
            RETAILMSG(ZONE_INFO, "Enabled %s", Log::ZoneMapping[i].name);
            zoneMask |= Log::ZoneMapping[i].zone;
        }
    }
    
    if (zoneMask)
    {
        Log::SetZoneMask( zoneMask );
    }



    //
    // Scale the rendered output, if needed.
    //
    // All rendering/movement/animation is done internally using the 
    // high-res coordinate system of the game, and then down-sampled at render time.  
    // It uses more graphics memory, but makes the code much simpler.
    //
    
    if (Platform::GetScreenScaleFactor() != 2.0)
    {
        RETAILMSG(ZONE_INFO, "Not a Retina display; force Settings.fWorldScaleFactor = 0.5");
        GlobalSettings.SetFloat("/Settings.fWorldScaleFactor",  0.5f);
    }
    
    if (Platform::IsIPhone3G())
    {
        GlobalSettings.SetBool ("/Settings.bUseOpenGLES1",      true);
        RETAILMSG(ZONE_INFO, "iPhone3G; force Settings.bUseOpenGLES1 = 1");
    }
    

    s_isInitialized = true;
    
Exit:   
    return rval;
}



RESULT
Engine::SetRenderContext( RenderContext* pContext )
{
    RESULT rval = S_OK;

    if (!pContext)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Engine::SetRenderContext(): pContext must not be NULL");
        rval = E_NULL_POINTER;
        
        DEBUGCHK(0);
        
        goto Exit;
    }
    s_pRenderContext = pContext;
    s_pRenderContext->AddRef();
    
    CHR(Renderer.SetRenderContext( s_pRenderContext ));
    
Exit:
    return rval;
}



RESULT
Engine::LoadResources( )
{
    RESULT rval = S_OK;
    
    RETAILMSG(ZONE_INFO, "Engine::LoadResources()");
    
    if (s_resourcesLoaded)
        return S_OK;
        
    
    UINT32 usedRAM1 = 0;
    UINT32 usedRAM2 = 0;
    usedRAM1 = Platform::GetProcessUsedMemory();

    CHR( ShaderMan.Init             ( "/app/settings/shaders.xml"       ) );
    CHR( EffectMan.Init             ( "/app/settings/effects.xml"       ) );
    CHR( StoryboardMan.Init         ( "/app/settings/storyboards.xml"   ) );
    CHR( TextureMan.Init            ( "/app/settings/textures.xml"      ) );
    CHR( FontMan.Init               ( "/app/settings/fonts.xml"         ) );
    CHR( SpriteMan.Init             ( "/app/settings/sprites.xml"       ) );
    CHR( MeshMan.Init               ( "/app/settings/meshes.xml"        ) );
    CHR( BehaviorMan.Init           ( "/app/settings/behaviors.xml"     ) );
    CHR( SoundMan.Init              ( "/app/settings/sounds.xml"        ) );
    CHR( ParticleMan.Init           ( "/app/settings/particles.xml"     ) );
    
    usedRAM2 = Platform::GetProcessUsedMemory();
    RETAILMSG(ZONE_INFO, "RAM consumed during LoadResources(): %4.2fMB (process)", (float)(usedRAM2 - usedRAM1)/1048657.0f);
    
    
    // Prewarm the shaders so there won't be any stutter on first draw.
    CHR( ShaderMan.PrewarmShaders() );
    CHR( EffectMan.PrewarmEffects() );

    // HACK: Effect prewarming leaves the last shader program set, WITHOUT the Renderer knowing it.
    // Next call to set a shader uniform will blow up.
    // TODO: Effects and Renderer need to always work together.
    if ( Platform::IsOpenGLES2() )
    {
        HEffect hDefault;
        CHR( EffectMan.GetCopy( "DefaultEffect", &hDefault ));
        CHR( Renderer.PushEffect( hDefault ) );
    }

    //
    // Configure camera.
    //
    // Don't do this until after the renderer has been created and its shaders loaded;
    // the camera needs to know the renderer window size and needs to set the
    // shader projection matrix.
    //
    if ("Orthographic" == GlobalSettings.GetString( "/Settings.CameraMode" ))
    {
        DEBUGMSG(ZONE_INFO, "CameraMode == ORTHOGRAPHIC");
        GameCamera.SetMode( CAMERA_MODE_ORTHOGRAPHIC );
    }
    {
    vec4 projection = GlobalSettings.GetVec4( "/Settings.CameraProjection", vec4(53.0f, 0.5f, 1.0f, 5000.0f) );
    GameCamera.SetProjection( projection.x, projection.y, projection.z, projection.w );
    }


    s_resourcesLoaded = true;

Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "FATAL: Engine::LoadResources() failed");
        DEBUGCHK(0);
    }

    return rval;
}



RESULT
Engine::UnloadResources()
{
    RESULT rval = S_OK;
    
    CHR( ParticleMan.Shutdown()     );
    CHR( SoundMan.Shutdown()        );
    CHR( SceneMan.Shutdown()        );
    CHR( LayerMan.Shutdown()        );
    CHR( BehaviorMan.Shutdown()     );
    CHR( MeshMan.Shutdown()         );
    CHR( SpriteMan.Shutdown()       );
    CHR( FontMan.Shutdown()         );
    CHR( TextureMan.Shutdown()      );
    CHR( StoryboardMan.Shutdown()   );
    CHR( EffectMan.Shutdown()       );
    CHR( ShaderMan.Shutdown()       );
    
Exit:
    return rval;
}



RESULT
Engine::Pause( )
{
    RESULT rval = S_OK;

    RETAILMSG(ZONE_INFO, "-- Engine paused --");
    s_isPaused = true;
    
Exit:
    return rval;
}



RESULT
Engine::Resume( )
{
    RESULT rval = S_OK;

    RETAILMSG(ZONE_INFO, "-- Engine resumed from pause --");
    s_isPaused = false;

Exit:
    return rval;
}



bool
Engine::IsPaused( )
{
    return s_isPaused;
}



RESULT
Engine::Update( )
{
    RESULT rval = S_OK;

    PerfTimer timer;
    timer.Start();

    // Don't tick GameObjects when paused.
    // Do update animations, particles, and sound (so that menus work).
    if (!s_isPaused)
    {
        CHR(GOMan.Update       ( GameTime.GetTime() ))
    }
    
    CHR(StoryboardMan.Update   ( GameTime.GetTime() ));
    CHR(ParticleMan.Update     ( GameTime.GetTime() ));
    CHR(SoundMan.Update        ( GameTime.GetTime() ));

    timer.Stop();
    
    if (Log::IsZoneEnabled(ZONE_PERF | ZONE_VERBOSE))
    {
        char str[256];
        sprintf(str, "U:%1.0f ms", timer.ElapsedMilliseconds());
        DebugRender.Text(vec3(180,0,0), str, Color::Green(), 1.0f, 1.0f);
    }
    
Exit:
    return rval;
}



RESULT
Engine::Render( )
{
    RESULT rval = S_OK;
    
    PerfTimer timer;
    timer.Start();

    CHR(Renderer.BeginFrame());
    CHR(Renderer.Clear( 0.0, 0.0, 0.0, 1.0 ));
    CHR(LayerMan.Draw());
    CHR(SceneMan.Draw());   // TODO: merge Scenes and Layers.
    

    //
    // Total HACK
    // Render the level and score here, until we get Controls up and running.
    //
    Rectangle screen;
    Platform::GetScreenRect(&screen);

    if (g_showScore)
    {
    HFont hFont;
    FontMan.Get( "BanzaiBros", &hFont );
    float fontHeight = FontMan.GetHeight( hFont );

    char str[128];

    sprintf(str, "L %d", (int)g_pLevel->level);
    float width = FontMan.GetWidth( hFont, str );
    vec2 pos = vec2( screen.width - width - 4, screen.height - fontHeight - 16 );
    FontMan.Draw( pos, str, hFont, g_pLevel->backgroundColor );
    
    // Center score on screen
    sprintf(str, "%d", (int)g_totalScore);
    width = FontMan.GetWidth( hFont, str ) * g_scoreScale.GetFloat();
    pos.x = ((screen.width - width)/2);
    FontMan.Draw( pos, str, hFont, Color::LightBlue(), g_scoreScale.GetFloat() );
    }


#ifndef SHIPBUILD
    // Number of GameObjects
    {
    HFont hFont;
    FontMan.Get( "BanzaiBros", &hFont );
    vec2 pos;
    char str[128];
    sprintf(str, "%d", (int)GOMan.Count());
    int width = FontMan.GetWidth( hFont, str );
    pos.x = ((screen.width - width)/2);
    pos.y = 0;
    FontMan.Draw( pos, str );
    }
#endif

#ifndef SHIPBUILD
    {
    vec2 pos;
    pos = vec2(screen.width - 96, 0);
    FontMan.Draw( pos, Platform::GetBuildInfo() );
    }
#endif


#ifdef DEBUG
    CHR(DebugRender.Draw());
#endif
    CHR(DebugRender.Reset());


    CHR(Renderer.EndFrame());


    timer.Stop();

#ifndef SHIPBUILD
    if (Log::IsZoneEnabled(ZONE_PERF | ZONE_VERBOSE))
    {
        // Display the render time; will show up on next frame but that's OK.
        
        char str[256];
        sprintf(str, "R:%2.0f ms", timer.ElapsedMilliseconds());
        Color color = Color::Green();
        if (timer.ElapsedMilliseconds() > 18)
        {
            color = Color::Red();
        }
        DebugRender.Text(vec3(354,0,0), str, color, 1.0f, 1.0f);
    }
#endif


    
Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Engine::Render(): rval = 0x%x", rval);
        DEBUGCHK(0);
    }

    return rval;
}



RESULT
Engine::Shutdown( )
{
    RESULT rval = S_OK;

    RETAILMSG(ZONE_INFO, "Engine::Shutdown()");

    CHR(UnloadResources());
    
    SAFE_RELEASE(s_pRenderer);
    SAFE_RELEASE(s_pRenderContext);
    SAFE_RELEASE(s_pCamera);
    SAFE_RELEASE(s_pTouchInput);
    SAFE_RELEASE(s_pAccelerometer);

    
    RETAILMSG(ZONE_INFO, "Engine::Shutdown(): %d Objects were not freed.", Object::Count());

    Particles.Print();
    Sounds.Print();
    SceneMan.Print();
    LayerMan.Print();
    BehaviorMan.Print();
    MeshMan.Print();
    SpriteMan.Print();
    TextureMan.Print();
    StoryboardMan.Print();
    ShaderMan.Print();
    Effects.Print();
    GOMan.Print();

Exit:
    return rval;
}



GameObjectManager&
Engine::GetGameObjectManager( )
{
    if (!s_pGameObjectManager)
    {
        s_pGameObjectManager = &GameObjectManager::Instance();
    }
    
    return *s_pGameObjectManager;
}




SpriteManager&
Engine::GetSpriteManager( )
{
    if (!s_pSpriteManager)
    {
        s_pSpriteManager = &SpriteManager::Instance();
    }
    
    return *s_pSpriteManager;
}




TextureManager&
Engine::GetTextureManager( )
{
    if (!s_pTextureManager)
    {
        s_pTextureManager = &TextureManager::Instance();
    }
    
    return *s_pTextureManager;
}




SoundManager&
Engine::GetSoundManager( )
{
    if (!s_pSoundManager)
    {
        s_pSoundManager = &SoundManager::Instance();
    }
    
    return *s_pSoundManager;
}




EffectManager&
Engine::GetEffectManager( )
{
    if (!s_pEffectManager)
    {
        s_pEffectManager = (Z::EffectManager*) &EffectManager::Instance();
    }
    
    return *s_pEffectManager;
}




IRenderer&
Engine::GetRenderer( )
{
    if (!s_pRenderer) 
    {
        bool bUseOpenGLES1 = Settings::Global().GetBool("/Settings.bUseOpenGLES1");
        
        if (bUseOpenGLES1)
        {
            s_pRenderer = OpenGLES1Renderer::Create();
        }
        else 
        {
            s_pRenderer = OpenGLES2Renderer::Create();
        }

        s_pRenderer->AddRef();
    }
    
    return *s_pRenderer;
}



Camera&
Engine::GetCamera( )
{
    if (!s_pCamera)
    {
        s_pCamera = new Camera();
        s_pCamera->AddRef();
    }
    
    return *s_pCamera;
}



TouchInput&
Engine::GetTouchInput( )
{
    if (!s_pTouchInput)
    {
        s_pTouchInput = &TouchInput::GetDefaultTouchInput();
        s_pTouchInput->AddRef();
    }
    
    return *s_pTouchInput;
}



Accelerometer&
Engine::GetAccelerometer( )
{
    if (!s_pAccelerometer)
    {
        s_pAccelerometer = &Accelerometer::GetDefaultAccelerometer();
        s_pAccelerometer->AddRef();
    }
    
    return *s_pAccelerometer;
}



SceneManager&
Engine::GetSceneManager( )
{
    if (!s_pSceneManager)
    {
        s_pSceneManager = &SceneManager::Instance();
    }
    
    return *s_pSceneManager;
}


    

ParticleManager&
Engine::GetParticleManager( )
{
    if (!s_pParticleManager)
    {
        s_pParticleManager = &ParticleManager::Instance();
    }
    
    return *s_pParticleManager;
}



StoryboardManager&
Engine::GetStoryboardManager( )
{
    if (!s_pStoryboardManager)
    {
        s_pStoryboardManager = &StoryboardManager::Instance();
    }
    
    return *s_pStoryboardManager;
}




        
} // END namespace Z
