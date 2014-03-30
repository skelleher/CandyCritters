#include "Game.hpp"
#include "Engine.hpp"
#include "Log.hpp"
#include "Platform.hpp"
#include "GameObject.hpp"
#include "GameObjectManager.hpp"
#include "StateMachine.hpp"
#include "Settings.hpp"
#include "GameState.hpp"
#include "IdleState.hpp"
#include "GameScreens.hpp"


#include "test.hpp"


namespace Z 
{


extern bool g_showTutorial;


//
// Static data
//
static HGameObject s_hGameState;
static GameObject* s_pGameState = NULL;


RESULT
Game::Start()
{
    RESULT               rval                 = S_OK;
    StateMachine*        pState               = NULL; 
    StateMachineManager* pStateMachineManager = NULL;
    

    if (s_pGameState != NULL)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Game::Start(): game has already been started.");
        return E_INVALID_OPERATION;
    }

    
    Rectangle screenRect;
    Platform::GetScreenRect(&screenRect);
    
    RETAILMSG(ZONE_INFO, "\n");
    RETAILMSG(ZONE_INFO, "Build version: %s\n",                 Platform::GetBuildInfo());
    RETAILMSG(ZONE_INFO, "Build date: %s %s\n",                 __DATE__, __TIME__);
    RETAILMSG(ZONE_INFO, "Device.model:         %s",            Platform::GetDeviceType());
    RETAILMSG(ZONE_INFO, "Device.systemName:    %s",            Platform::GetOSName());
    RETAILMSG(ZONE_INFO, "Device.systemVersion: %s",            Platform::GetOSVersion());
    RETAILMSG(ZONE_INFO, "Screen:               %2.0f x %2.0f", screenRect.width, screenRect.height);
    RETAILMSG(ZONE_INFO, "Screen ScaleFactor:   %f\n",          Platform::GetScreenScaleFactor());
    DEBUGMSG(ZONE_INFO,  "DEBUG build\n");

    if (Platform::IsDebuggerAttached())
    {
        RETAILMSG(ZONE_INFO, "*** Device is attached to debugger ***\n\n");
    }


    //
    // Load user preferences
    //
    CHR(LoadUserPreferences());


    CHR(Engine::Init());

    //
    // Load resources
    //
    Engine::LoadResources();
    

    //
    // Tell any StateMachines they can begin using ZEngine.
    //
    GOMan.SendMessageFromSystem( MSG_EngineReady );

  
    //
    // Create GameObject + StateMachine to manage the game states
    //
    s_pGameState = new GameObject();
    s_pGameState->Init( "GameController" );
    CHR(GOMan.Add( "GameController", s_pGameState, &s_hGameState ));    // TODO: separate GameScreens and GameState objects!!!  GameScreens only tracks scene transitions, nothing else.
    
    
    //
    // Create GameScreens StateMachine
    //
    CHR(s_pGameState->CreateStateMachineManager());
    pStateMachineManager = s_pGameState->GetStateMachineManager();
    
    pState = new IdleState(s_hGameState);
    pStateMachineManager->PushStateMachine( *pState, STATE_MACHINE_QUEUE_0, true );

    pState = new GameScreens(s_hGameState);
    pStateMachineManager->PushStateMachine( *pState, STATE_MACHINE_QUEUE_0, false );
    

    // TODO: free all these objects when the StateMachine is destroyed
    
Exit:
    return rval;
}



RESULT
Game::Stop()
{
    RESULT rval = S_OK;
    
    CHR(GOMan.Release( s_hGameState ));
//    SAFE_RELEASE(s_pGameState);


#ifdef USE_OPENFEINT
    //OpenFeint.StopService();
#endif

Exit:
    return rval;
}



RESULT
Game::Pause()
{
    RESULT rval = S_OK;
    
Exit:
    return rval;
}



RESULT
Game::Resume()
{
    RESULT rval = S_OK;
    
Exit:
    return rval;
}



RESULT
Game::LoadUserPreferences()
{
    RESULT rval = S_OK;
    
    // Open user preferences file.
    // If not found, create one with default values.
    Settings* pSettings = &Settings::User();
    CPR(pSettings);

    if (FAILED(pSettings->Read("/user/userprefs.xml")))
    {
        // Load default user prefs from read-only storage.
        if (FAILED(pSettings->Read("/app/settings/defaultuserprefs.xml")))
        {
            RETAILMSG(ZONE_ERROR, "ERROR: failed to load default userprefs.xml.");
        }

        // Create writable copy of user prefs.
        if (FAILED(pSettings->Write("/user/userprefs.xml")))
        {
            RETAILMSG(ZONE_ERROR, "ERROR: failed to create userprefs.xml!  Will not save user preferences.");
            goto Exit;
        }
    }
    
    RETAILMSG(ZONE_INFO, "Loading settings from userprefs.xml");

    // TODO: want to lay user prefs on top of default settings to create one file?
    // e.g. for audio/music volume.
    
    IGNOREHR(SoundMan.SetFXVolume    ( pSettings->GetFloat("/UserPrefs.FXVolume",       SoundMan.GetFXVolume())     ));
    IGNOREHR(SoundMan.SetMusicVolume ( pSettings->GetFloat("/UserPrefs.MusicVolume",    SoundMan.GetMusicVolume())  ));
    IGNOREHR(g_highScore =             pSettings->GetInt  ("/UserPrefs.HighScore",      0                           ));
    IGNOREHR(g_showTutorial =          pSettings->GetBool ("/UserPrefs.bShowTutorial",  true                        ));
    IGNOREHR(g_difficulty =            pSettings->GetInt  ("/UserPrefs.Difficulty",     5                           ));
    IGNOREHR(g_highestLevelEver =      pSettings->GetInt  ("/UserPrefs.HighestLevel",   1                           ));
    
    // Override some defaults based on platform
    if (Platform::IsIPhone3G() ||
        Platform::IsIPhone3GS())
    {
        pSettings->SetInt("ParticleUpdateRateHZ", 15);
    }
    
Exit:
    return rval;
}


RESULT
Game::SaveUserPreferences()
{
    RESULT rval = S_OK;

    // Open user preferences file.
    // If not found, create one with default values.
    Settings* pSettings = &Settings::User();
    CPR(pSettings);
    
    RETAILMSG(ZONE_INFO, "Saving settings to userprefs.xml");

    CHR(pSettings->SetFloat("/UserPrefs.FXVolume",       SoundMan.GetFXVolume()      ));
    CHR(pSettings->SetFloat("/UserPrefs.MusicVolume",    SoundMan.GetMusicVolume()   ));
    CHR(pSettings->SetInt  ("/UserPrefs.HighScore",      g_highScore                 ));
    CHR(pSettings->SetBool ("/UserPrefs.bShowTutorial",  g_showTutorial              ));
    CHR(pSettings->SetInt  ("/UserPrefs.Difficulty",     g_difficulty                ));

    if (FAILED(pSettings->Write()))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: failed to save userprefs.xml!");
    }
    
    
Exit:
    return rval;
}



} // END namespace Z



