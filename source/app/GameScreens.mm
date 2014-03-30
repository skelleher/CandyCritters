#include "Game.hpp"
#include "GameScreens.hpp"
#include "Log.hpp"
#include "LayerManager.hpp"
#include "SpriteManager.hpp"
#include "Engine.hpp"
#include "IdleState.hpp"
#include "ColumnState.hpp"
#include "GameObjectManager.hpp"
#include "GameState.hpp"
#include "SoundManager.hpp"
#include "test.hpp"
#include "AnimationManager.hpp"
#include "StoryboardManager.hpp"
#include "RippleEffect.hpp"
#include "BlurEffect.hpp"
#include "MorphEffect.hpp"
#include "Property.hpp"

#import "ConfirmViewController.h"
#import "ColumnBrickMap.hpp"
#import "DialogViewController.h"
#import "Tutorial.hpp"

// HACK HACK: hard pointers to the view controllers.
#import <UIKit/UIKit.h>
extern UIViewController*    homeScreenViewController;
extern UIViewController*    hudViewController;
extern UIViewController*    pauseScreenViewController;
extern UIViewController*    gameOverScreenViewController;
extern ConfirmViewController* confirmViewController;
extern UIViewController*    aboutViewController;
extern UIViewController*    tutorialViewController;
extern UIViewController*    levelViewController;


namespace Z 
{


// HACK HACK:
extern UINT32    g_totalScore;
extern UINT64    g_startGameTime;


//
// This StateMachine controls progress through the various game screens/modes.
//

// TODO: move game objects into GameState?
static HGameObject  hTutorialColumn;
static HGameObject  hColumn;
static HGameObject  hSkyController;

static HScene       hHomeScene;
static HScene       hOptionsScene;
static HScene       hPauseScene;
static HScene       hGameOverScene;
static HScene       hConfirmQuitScene;
static HScene       hHUDScene;
static HScene       hAboutScene;
static HScene       hTutorialScene;
static HScene       hLevelSelectScene;

static DialogViewController* dialog;

static HEffect      hHomeSceneEffect;
static HEffect      hOptionsSceneEffect;
static HEffect      hPauseSceneEffect;
static HEffect      hGameOverSceneEffect;
static HEffect      hConfirmQuitSceneEffect;
static HEffect      hAboutSceneEffect;
static HEffect      hTutorialSceneEffect;
static HEffect      hLevelSelectSceneEffect;
static HEffect      hMessageBoxEffect;

static HSound       hShowSound;
static HSound       hHideSound;
static HSound       hLevelUpSound;

static bool         animateGamePiecesOntoScreen = true;

static HStoryboard  hHideHomeSceneStoryboard;
static HStoryboard  hShowHomeSceneStoryboard; 
static HStoryboard  hHideOptionsSceneStoryboard;
static HStoryboard  hShowOptionsSceneStoryboard; 
static HStoryboard  hHidePauseSceneStoryboard;
static HStoryboard  hShowPauseSceneStoryboard; 
static HStoryboard  hHideGameOverSceneStoryboard;
static HStoryboard  hShowGameOverSceneStoryboard; 
static HStoryboard  hHideConfirmQuitSceneStoryboard;
static HStoryboard  hShowConfirmQuitSceneStoryboard; 
static HStoryboard  hHideHUDSceneStoryboard;
static HStoryboard  hShowHUDSceneStoryboard;
static HStoryboard  hShowAboutSceneStoryboard;
static HStoryboard  hHideAboutSceneStoryboard;
static HStoryboard  hShowTutorialSceneStoryboard;
static HStoryboard  hHideTutorialSceneStoryboard;
static HStoryboard  hShowLevelSelectSceneStoryboard;
static HStoryboard  hHideLevelSelectSceneStoryboard;

static HStoryboard  hShowMessageBoxStoryboard;
static HStoryboard  hHideMessageBoxStoryboard;

static HStoryboard  hColorToMonochromeStoryboard;
static HStoryboard  hMonochromeToColorStoryboard;

static HStoryboard  hFadeOutBackgroundStoryboard;
static HStoryboard  hFadeInBackgroundStoryboard;

static HStoryboard  hDarkenScreenStoryboard;
static HStoryboard  hLightenScreenStoryboard;

static HStoryboard  hShowGameGridStoryboard;
static HStoryboard  hHideGameGridStoryboard;
static HStoryboard  hHideColumnStoryboard;
static HStoryboard  hShowColumnStoryboard;
static HStoryboard  hBlurOutStoryboard;
static HStoryboard  hBlurInStoryboard;

static HLayer       hRootLayer;
static HLayer       hSkyLayer;
static HLayer       hSunBeamsLayer;
static HLayer       hPlayScreenBackground;
static HLayer       hPlayScreenGrid;
static HLayer       hPlayScreenColumn;
static HLayer       hPlayScreenForeground;
static HLayer       hPlayScreenAlerts;
static HLayer       hMenuLayer;

static Time         tutorialTime;


//
// Procedurally-generated backgrounds.
//
#define MIN_HILLS 4
#define MAX_HILLS 7
static HSprite      hGroundSprite;
static HSprite      hHillSprites[MAX_HILLS];

static HSprite      hVignetteSprite;


// Max number of critters to generate on the home screen.
static const int NUM_HOME_SCREEN_CRITTERS = 14;
static HGameObjectList g_homescreenCritters;



static HStoryboard  hPixelStoryboard;
static HStoryboard  hMorphStoryboard;
static HEffect      hRippleEffect;
static HEffect      hBlurEffect;
static HEffect      hPixelEffect;
static HEffect      hMorphEffect;
//static HEffect      hDoomEffect;
static HEffect      hMonochromeEffect;
static HEffect      hGlassEffect;


static HStoryboard  hCameraStoryboard;


// Pre-selected piece types for the tutorial mode.
static BrickType tutorialColumnBrickTypes[] =
{
    { "purple",        "CharacterState",   GO_TYPE_BLUE_BRICK,     1.0 },
    { "orange",     "CharacterState",   GO_TYPE_GREEN_BRICK,    1.0 },
    { "orange",     "CharacterState",   GO_TYPE_GREEN_BRICK,    1.0 },
};

static BrickType tutorialGroundBrickTypes[] =
{
    { "purple",       "CharacterState",   GO_TYPE_BLUE_BRICK,     1.0 },
    { "orange",     "CharacterState",   GO_TYPE_GREEN_BRICK,    1.0 },
    { "orange",     "CharacterState",   GO_TYPE_GREEN_BRICK,    1.0 },
    { "purple",       "CharacterState",   GO_TYPE_BLUE_BRICK,     1.0 },
    { "orange",     "CharacterState",   GO_TYPE_GREEN_BRICK,    1.0 },
    { "red",        "CharacterState",   GO_TYPE_RED_BRICK,      1.0 },
    { "purple",       "CharacterState",   GO_TYPE_BLUE_BRICK,     1.0 },

    { "red",        "CharacterState",   GO_TYPE_RED_BRICK,      1.0 },
    { "purple",       "CharacterState",   GO_TYPE_BLUE_BRICK,     1.0 },
    { "purple",       "CharacterState",   GO_TYPE_BLUE_BRICK,     1.0 },
    { "orange",     "CharacterState",   GO_TYPE_GREEN_BRICK,    1.0 },
    { "red",        "CharacterState",   GO_TYPE_RED_BRICK,      1.0 },
    { "orange",     "CharacterState",   GO_TYPE_GREEN_BRICK,    1.0 },
    { "purple",       "CharacterState",   GO_TYPE_BLUE_BRICK,     1.0 },
};




//Add new states here
enum StateName {
    STATE_Initialize,
    STATE_Home,
    STATE_Tutorial,
    STATE_LevelSelect,
    STATE_NewGame,
    STATE_Play,
    STATE_LevelUp,
    STATE_PauseMenu,
    STATE_OptionsMenu,
    STATE_ConfirmQuit,
    STATE_GameOver,
    STATE_Credits,
    STATE_Test,
};

//Add new substates here
enum SubstateName {
	//empty
    SUBSTATE_TestExecute,
    
    SUBSTATE_LevelUp_HighliteLevel,
    SUBSTATE_LevelUp_Fireworks,
    SUBSTATE_LevelUp_HideBackground,
};




//=============================================================================
//
// This function is called when a Critter has fallen to bottom of screen.
//
//=============================================================================
void
GameScreens::DropCritterCallback( void* pContext )
{
    static HSound hThud;
    
    if (hThud.IsNull())
    {
        Sounds.GetCopy( "Clink", &hThud );
    }
    
    bool playSound = (bool)pContext;
    if (playSound)
    {
        Sounds.Play( hThud );
    }
}


void
GameScreens::ShowHomeScreenDoneCallback( void* pContext )
{
#ifdef USE_OPENFEINT
    static bool bOpenFeintStarted = false;

    if (!bOpenFeintStarted)
    {
        OpenFeint.StartService();
        bOpenFeintStarted = true;
    }
#endif
}


void
GameScreens::LightenScreenDoneCallback( void* )
{
    Layers.RemoveFromLayer( hPlayScreenForeground, hVignetteSprite );
}



void
GameScreens::OnDialogConfirmCallback( void* pContext )
{
    HGameObject hGO = *(HGameObject*)pContext;
    [dialog hide];
    tutorialTime.Resume();

    GameObjects.SendMessageFromSystem( hGO, MSG_DialogConfirmed );
}



void
GameScreens::OnDialogCancelCallback( void* pContext )
{
    HGameObject hGO = *(HGameObject*)pContext;
    [dialog hide];
    tutorialTime.Resume();

    GameObjects.SendMessageFromSystem( hGO, MSG_DialogCancelled );
}



GameScreens::GameScreens( HGameObject &hGameObject ) :
    StateMachine( hGameObject )
{
}

GameScreens::~GameScreens( void )
{
}



void  
GameScreens::ShowMenuDoneCallback  ( void* pContext )
{
    LayerMan.SetEffect( hPlayScreenGrid, HEffect::NullHandle() );
}


void
GameScreens::HideMenuDoneCallback  ( void* pContext )
{
    LayerMan.SetEffect( hPlayScreenGrid, HEffect::NullHandle() );
}


void
GameScreens::BackgroundFadeDoneCallback ( void* pContext )
{
    GenerateBackground();

    StoryboardMan.BindTo( hFadeInBackgroundStoryboard, hPlayScreenBackground );
    StoryboardMan.Start( hFadeInBackgroundStoryboard );
}




//
// Procedurally generate a background consisting of sky, hills, and particles
// (clouds or stars).
//    
RESULT
GameScreens::GenerateBackground()
{
    RESULT rval = S_OK;

    static HSprite hGroundSprite;

    UINT8       numHills         = 0;

    // TODO: Generate N hills, ground.

    CHR(LayerMan.Clear( hPlayScreenBackground ));
    hGroundSprite.Release();
    for (int i = 0; i < MAX_HILLS; ++i)
    {
        hHillSprites[i].Release();
    }

    // Hills
    numHills = Platform::Random(MIN_HILLS, MAX_HILLS);
    for (int i = 0; i < numHills; ++i)
    {
        GenerateHill( &hHillSprites[i] );
        // TODO: sort by area so large hills are behind small ones.
//                CHR(LayerMan.AddToLayer( hPlayScreenBackground, hHillSprites[i] ));
    }
    
    // Foreground
//    CHR(SpriteMan.GetCopy( "ground", &hGroundSprite ));
//    CHR(SpriteMan.GetCopy( "strick-ground", &hGroundSprite ));
//    CHR(LayerMan.AddToLayer( hPlayScreenForeground, hGroundSprite ));
            

Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GenerateBackground() failed with error 0x%x", rval);
    }

    return rval;
}



RESULT
GameScreens::GenerateHill( IN HSprite* pHSprite )
{
    RESULT rval = S_OK;

    CPR(pHSprite);

    // TODO: generate a high-res triangle list.
    // TODO: deform it: sin wave, vScale, hScale.
    // TODO: generate a texture.
    // TODO: apply the "paper" texture.  Requires: multi-texturing in HSprite/HMesh/Vertex.


Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GenerateHill() failed with error 0x%x", rval);
    }

    return rval;
}
    
  

RESULT
GameScreens::GenerateHomeScreenCritter( MAP_POSITION startMapPos, MAP_POSITION endMapPos, bool playSound, BrickType* pBrickType )
{
    RESULT          rval = S_OK;
    WORLD_POSITION  startPos;
    WORLD_POSITION  endPos;
    HGameObject     hCritter;
    
    // For each critter
    //  Select Sprite at random.
    //  Select position at random.
    //  Create drop animation.
    //  Bind and start drop animation.

    if (startMapPos != MAP_POSITION(-1,-1))
    {
        g_pGameMap->MapToWorldPosition(startMapPos, &startPos, false);
    }
    else
    {
        if (Platform::IsWidescreen()) {
            startPos.y  = GAME_GRID_TOP;
        } else {
            startPos.y  = GAME_GRID_TOP_3_5;
        }
        
        startPos.x  = Platform::Random(GAME_GRID_LEFT, GAME_GRID_RIGHT-COLUMN_BRICK_WIDTH);
        startPos.z  = 0;
    }

    if (endMapPos != MAP_POSITION(-1,-1))
    {
        g_pGameMap->MapToWorldPosition(endMapPos, &endPos, false);
    }
    else
    {
        endPos      = startPos;
        
        if (Platform::IsWidescreen()) {
            endPos.y    = GAME_GRID_BOTTOM;
        } else {
            endPos.y    = GAME_GRID_BOTTOM_3_5;
        }
    }


    
    if (g_homescreenCritters.size() >= NUM_HOME_SCREEN_CRITTERS)
    {
        // Reuse a previous critter
        hCritter = g_homescreenCritters.front(); g_homescreenCritters.pop_front();
        
        // Remove from the draw list so we don't double-add it down below.
        Layers.RemoveFromLayer( hPlayScreenBackground, hCritter );
    }
    else
    {
        // Create a critter.
        BrickType* pType = pBrickType;
        if (!pType)
        {
            pType = &g_brickTypes[ Platform::Random() % g_numBrickTypes ];
        }

        CHR(GOMan.Create( "",                           // GO name
                          &hCritter,                    // HGameObject
                          pType->spriteName,            // Sprite name
                          "",                           // Mesh name
                          "",                           // Effect name
                          pType->behaviorName,          // Behavior name
                          pType->goType,                // GO_TYPE
                          startPos,                     // position
                          1.0f,                         // opacity
                          Color::White(),               // color
                          true                          // hasShadow
                          ));
    }
    
    
    //
    // Create drop animation.
    //
    {
    MAP_POSITION startMapPos, endMapPos;
    g_pGameMap->WorldToMapPosition( startPos, &startMapPos );
    g_pGameMap->WorldToMapPosition( endPos,   &endMapPos   );
    
    UINT64 dropDurationMS  = (startMapPos.y - endMapPos.y) * 35;

    KeyFrame keyFrames[2];
    keyFrames[0].SetTimeMS(0);
    keyFrames[0].SetVec3Value( startPos );
    keyFrames[1].SetTimeMS( dropDurationMS );
    keyFrames[1].SetVec3Value( endPos   );

    HAnimation  hDropAnimation;
    HStoryboard hDropStoryboard;
    CHR(AnimationMan.CreateAnimation( "", "Position", PROPERTY_VEC3, INTERPOLATOR_TYPE_QUADRATIC_IN, KEYFRAME_TYPE_VEC3, &keyFrames[0], 2, false, &hDropAnimation ));

    CHR(StoryboardMan.CreateStoryboard( "",                     // Storyboard name
                                        &hDropAnimation,        // HAnimations[]
                                        1,                      // numAnimations
                                        false,                  // autoRepeat
                                        false,                  // autoReverse
                                        true,                   // releaseTargetOnFinish
                                        true,                   // deleteOnFinish
                                        false,                  // isRelative
                                        &hDropStoryboard        // HStoryboard
                                      ));
    
    if (playSound)
    {
        Callback callback( GameScreens::DropCritterCallback, (void*)true );
        CHR(StoryboardMan.CallbackOnFinished( hDropStoryboard, callback ));
    }
    else
    {
        Callback callback( GameScreens::DropCritterCallback );
        CHR(StoryboardMan.CallbackOnFinished( hDropStoryboard, callback ));
    }
    
    CHR(StoryboardMan.BindTo( hDropStoryboard, hCritter ));
    CHR(StoryboardMan.Start( hDropStoryboard ));
    CHR(AnimationMan.Release( hDropAnimation ));
    }

    //
    // Drop that critter.
    //
    CHR(Layers.AddToLayer( hPlayScreenBackground, hCritter ));
    g_pGameMap->AddGameObject(hCritter);
      
Exit:
    g_homescreenCritters.push_back( hCritter );

    return rval;
}



bool GameScreens::States( State_Machine_Event event, MSG_Object * msg, int state, int substate )
{
BeginStateMachine

	//Global message responses
	OnMsg( MSG_Reset )
		ResetStateMachine();
        RETAILMSG(ZONE_STATEMACHINE, "GameScreens: MSG_Reset");

    #pragma mark -
    #pragma mark MSG_Handlers
    
    OnMsg( MSG_GameScreenPrevious )
        // Return to previous screen.
        // TODO: ensure there's a non-idle state to pop, else don't pop!  Can hang the game!
        PopState();
    
    OnMsg( MSG_GameScreenTest )
        ChangeState( STATE_Test );

	OnMsg( MSG_GameScreenHome )
        ChangeState( STATE_Home );


    OnMsg( MSG_GameScreenLevel )
        ChangeState( STATE_LevelSelect );

    OnMsg( MSG_GameScreenTutorial )
    
        // TODO: push new TutorialState machine
    
        ChangeState( STATE_Tutorial );
    
    OnMsg( MSG_NewGame )
        if (GetState() != STATE_NewGame)
            ChangeState( STATE_NewGame );
                
	OnMsg( MSG_GameScreenPlay )
        ChangeState( STATE_Play );
        
    OnMsg( MSG_PauseGame )
        RETAILMSG(ZONE_STATEMACHINE, "GameScreens: MSG_PauseGame");
        if (GetState() == STATE_NewGame || GetState() == STATE_Play)
            ChangeState( STATE_PauseMenu );
    
    OnMsg( MSG_UnpauseGame )
        RETAILMSG(ZONE_STATEMACHINE, "GameScreens: MSG_UnpauseGame");
        if (GetState() == STATE_PauseMenu)
            PopState();
        
	OnMsg( MSG_GameScreenCredits )
        ChangeState( STATE_Credits );
        
    OnMsg( MSG_GameScreenConfirmQuit )
        ChangeState( STATE_ConfirmQuit );

    OnMsg( MSG_GameOver )
        ChangeState( STATE_GameOver );

    OnMsg( MSG_LevelUp )
        ChangeState( STATE_LevelUp );


    ///////////////////////////////////////////////////////////////
    #pragma mark -
    #pragma mark STATE_Test
	DeclareState( STATE_Test )
        OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_Test");
            ChangeSubstateDelayed( 0.1f, SUBSTATE_TestExecute ); 
            

        DeclareSubstate( SUBSTATE_TestExecute )
            OnEnter
                RETAILMSG(ZONE_STATEMACHINE, "GameScreens: SUBSTATE_TestExecute");

                //TestStoryboard();
                //TestStoryboardStress();
                //TestGameAnimations();
                //TestAABB();
                //TestFont();
                //TestSound();
                //TestEffect();
                //TestRandom();
                //TestProperties();
                //TestProbability();
                //TestSpriteFromFile();
                //TestJSON();
                //TestTexturePacker();
                //TestParticles();
                //TestGameTime();
                //TestEasing();
                //TestRipple();

                ChangeState( STATE_Initialize );
                
    
    
    ///////////////////////////////////////////////////////////////
    #pragma mark STATE_Initialize
	DeclareState( STATE_Initialize )
        OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_Initialize");
            

            if (Platform::IsWidescreen()) {
                g_pGameMap = new ColumnBrickMap(
                                     GAME_GRID_NUM_COLUMNS,     // columns
                                     GAME_GRID_NUM_ROWS+1,      // rows
                                     COLUMN_BRICK_WIDTH,        // cell width
                                     GAME_GRID_LEFT,            // X offset in world coordinates
                                     GAME_GRID_BOTTOM,          // Y offset in world coordinates
                                     0.0,                       // Z offset in world coordinates
                                     true                       // isVertical
                                   );
            } else {
                g_pGameMap = new ColumnBrickMap(
                                     GAME_GRID_NUM_COLUMNS,     // columns
                                     GAME_GRID_NUM_ROWS_3_5+1,  // rows
                                     COLUMN_BRICK_WIDTH,        // cell width
                                     GAME_GRID_LEFT,            // X offset in world coordinates
                                     GAME_GRID_BOTTOM_3_5,      // Y offset in world coordinates
                                     0.0,                       // Z offset in world coordinates
                                     true                       // isVertical
                                   );
            }

    
            // HACK HACK: prevent rendering of score/level while showing home screen.
            // TODO: create UILabel objects.
            g_showScore = false;
            

            //
            // Create Layers.
            //
            
            LayerMan.CreateLayer( "RootLayer",              &hRootLayer );
            LayerMan.CreateLayer( "SkyLayer",               &hSkyLayer );
            LayerMan.CreateLayer( "SunBeamsLayer",          &hSunBeamsLayer );
            LayerMan.CreateLayer( "PlayScreenBackground",   &hPlayScreenBackground );
            LayerMan.CreateLayer( "PlayScreenGrid",         &hPlayScreenGrid );
            LayerMan.CreateLayer( "PlayScreenColumn",       &hPlayScreenColumn );
            LayerMan.CreateLayer( "PlayScreenForeground",   &hPlayScreenForeground );
            LayerMan.CreateLayer( "PlayScreenAlerts",       &hPlayScreenAlerts );
            LayerMan.CreateLayer( "Menu",                   &hMenuLayer );
            
            // Add the game layers to root, so we can render them all and then apply
            // BlurEffect in a single pass.
            // Keep the menu and alert layers separate.
            LayerMan.AddToLayer( hRootLayer, hSkyLayer );
            LayerMan.AddToLayer( hRootLayer, hSunBeamsLayer );
            LayerMan.AddToLayer( hRootLayer, hPlayScreenBackground );
            LayerMan.AddToLayer( hRootLayer, hPlayScreenGrid );
            LayerMan.AddToLayer( hRootLayer, hPlayScreenColumn );
            LayerMan.AddToLayer( hRootLayer, hPlayScreenForeground );
    
            LayerMan.SetVisible( hMenuLayer, false );


            //
            // Effects for drawing screen Layers
            //
            Effects.GetCopy      ( "RippleEffect", &hRippleEffect );
            Effects.GetCopy      ( "BlurEffect",   &hBlurEffect   );
            Effects.GetCopy      ( "ColorEffect",  &hMonochromeEffect );
            
            
            
            //
            // Sounds
            //
            Sounds.Get( "ShowMelody", &hShowSound    );
            Sounds.Get( "HideMelody", &hHideSound    );
            Sounds.Get( "LevelUp",    &hLevelUpSound );
            
            

            //
            // Scenes
            //
            SceneMan.CreateScene ( "homeScene",         homeScreenViewController,      &hHomeScene );
            SceneMan.CreateScene ( "pauseScene",        pauseScreenViewController,     &hPauseScene );
            SceneMan.CreateScene ( "gameOverScene",     gameOverScreenViewController,  &hGameOverScene );
            SceneMan.CreateScene ( "confirmQuitScene",  confirmViewController,         &hConfirmQuitScene );
            SceneMan.CreateScene ( "HUDScene",          hudViewController,             &hHUDScene );
            SceneMan.CreateScene ( "AboutScene",        aboutViewController,           &hAboutScene );
            SceneMan.CreateScene ( "TutorialScene",     tutorialViewController,        &hTutorialScene );
            SceneMan.CreateScene ( "LevelSelectScene",  levelViewController,           &hLevelSelectScene );

            EffectMan.GetCopy    ( "MorphEffect", &hHomeSceneEffect );
            EffectMan.GetCopy    ( "MorphEffect", &hOptionsSceneEffect );
            EffectMan.GetCopy    ( "MorphEffect", &hPauseSceneEffect );
            EffectMan.GetCopy    ( "MorphEffect", &hGameOverSceneEffect );
            EffectMan.GetCopy    ( "MorphEffect", &hConfirmQuitSceneEffect );
            EffectMan.GetCopy    ( "MorphEffect", &hAboutSceneEffect );
            EffectMan.GetCopy    ( "MorphEffect", &hTutorialSceneEffect );
            EffectMan.GetCopy    ( "MorphEffect", &hLevelSelectSceneEffect );

            dialog = [DialogViewController createDialogWithMessage:@"This is some text" confirmLabel:@"Yes" cancelLabel:@"No"];

            StoryboardMan.GetCopy( "TwistIn",       &hShowHomeSceneStoryboard );
            StoryboardMan.GetCopy( "TwistOut",      &hHideHomeSceneStoryboard );
            StoryboardMan.GetCopy( "TwistIn",       &hShowAboutSceneStoryboard );
            StoryboardMan.GetCopy( "TwistOut",      &hHideAboutSceneStoryboard );
            StoryboardMan.GetCopy( "TwistIn",       &hShowOptionsSceneStoryboard );
            StoryboardMan.GetCopy( "TwistOut",      &hHideOptionsSceneStoryboard );
            StoryboardMan.GetCopy( "TwistIn",       &hShowPauseSceneStoryboard );
            StoryboardMan.GetCopy( "TwistOut",      &hHidePauseSceneStoryboard );
            StoryboardMan.GetCopy( "TwistIn",       &hShowConfirmQuitSceneStoryboard );
            StoryboardMan.GetCopy( "TwistOut",      &hHideConfirmQuitSceneStoryboard );
            StoryboardMan.GetCopy( "TwistIn",       &hShowGameOverSceneStoryboard );
            StoryboardMan.GetCopy( "TwistOut",      &hHideGameOverSceneStoryboard );

            StoryboardMan.GetCopy( "TwistIn",       &hShowMessageBoxStoryboard );
            StoryboardMan.GetCopy( "TwistOut",      &hHideMessageBoxStoryboard );

            StoryboardMan.GetCopy( "TwistIn",       &hShowLevelSelectSceneStoryboard );
            StoryboardMan.GetCopy( "TwistOut",      &hHideLevelSelectSceneStoryboard );

            StoryboardMan.GetCopy( "FadeIn",        &hShowHUDSceneStoryboard );
            StoryboardMan.GetCopy( "FadeOut",       &hHideHUDSceneStoryboard );
            StoryboardMan.GetCopy( "TwistIn",       &hShowTutorialSceneStoryboard );
            StoryboardMan.GetCopy( "TwistOut",      &hHideTutorialSceneStoryboard );

            StoryboardMan.GetCopy( "HideGameGrid",  &hHideGameGridStoryboard );
            StoryboardMan.GetCopy( "ShowGameGrid",  &hShowGameGridStoryboard );
            StoryboardMan.GetCopy( "HideGameGrid",  &hHideColumnStoryboard );
            StoryboardMan.GetCopy( "ShowGameGrid",  &hShowColumnStoryboard );

            StoryboardMan.GetCopy( "BlurOut",       &hBlurOutStoryboard );
            StoryboardMan.GetCopy( "BlurIn",        &hBlurInStoryboard );

            StoryboardMan.GetCopy( "FadeOutBackground", &hFadeOutBackgroundStoryboard );
            StoryboardMan.GetCopy( "FadeInBackground",  &hFadeInBackgroundStoryboard  );

            StoryboardMan.GetCopy( "DarkenScreen",        &hDarkenScreenStoryboard );
            StoryboardMan.GetCopy( "LightenScreen",       &hLightenScreenStoryboard );


            StoryboardMan.GetCopy( "ColorToMonochrome", &hColorToMonochromeStoryboard );
            StoryboardMan.GetCopy( "MonochromeToColor", &hMonochromeToColorStoryboard );

            //
            // Vignette for when the game is paused.
            //
            Sprites.CreateFromFile("/app/vignette.png", &hVignetteSprite);
            Storyboards.BindTo( hDarkenScreenStoryboard,  hVignetteSprite );
            Storyboards.BindTo( hLightenScreenStoryboard, hVignetteSprite );
            Sprites.SetScale(hVignetteSprite, 2.0);



            //
            // Sky controller (day/night cycle, etc).
            //
            GOMan.Create( "SkyController", &hSkyController,  "",  "", "", "SkyState", GO_TYPE_CONTROLLER );
            
            HGameObject hSelf;
            GOMan.Get( "GameController", &hSelf );
            TouchScreen.AddListener( hSelf, MSG_TouchBegin );
            TouchScreen.AddListener( hSelf, MSG_TouchUpdate );
            GOMan.Release( hSelf );

            LayerMan.SetVisible( hPlayScreenBackground, true  );
            LayerMan.SetVisible( hPlayScreenAlerts,     false );


            // Display the home screen/menu after the fly-in storyboard completes.
            float duration = StoryboardMan.GetDurationMS( hCameraStoryboard ) / 1000.0f;
            if (duration > 0)
            {
                ChangeStateDelayed( duration, STATE_Home );
            }
            else
            {
                ChangeState( STATE_Home );
            }



	///////////////////////////////////////////////////////////////
    #pragma mark STATE_LevelSelect
	DeclareState( STATE_LevelSelect )

        OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_LevelSelect");
        
            // HACK HACK: don't render score/level.
            g_showScore = false;
            SceneMan.Show( hLevelSelectScene, hShowLevelSelectSceneStoryboard, hLevelSelectSceneEffect );
    
        OnExit
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_LevelSelect: Exit");
            SceneMan.Hide( hLevelSelectScene, hHideLevelSelectSceneStoryboard, hLevelSelectSceneEffect );
    
        
    
	///////////////////////////////////////////////////////////////
    #pragma mark STATE_Tutorial
	DeclareState( STATE_Tutorial )
    
        OnMsg( MSG_TutorialShowTitle )
            SceneMan.Show( hTutorialScene, hShowTutorialSceneStoryboard, hTutorialSceneEffect );

        OnMsg( MSG_TutorialCreateColumn )
            GOMan.Create( "TutorialColumnController", &hTutorialColumn,  "",  "", "", "ColumnState", GO_TYPE_SPRITE );
            
            // Create a column with pre-selected pieces.
            GOMan.SendMessageFromSystem( hTutorialColumn, MSG_TutorialCreateColumn, &tutorialColumnBrickTypes );

            // Add the finger sprite as a child, so it moves with the column.
            HSprite hFingerSprite;
            Sprites.GetCopy( "critters_hand", &hFingerSprite );
            SpriteMan.SetPosition( hFingerSprite, vec3(0, -360, 0) ); // Offset from parent GO
            SpriteMan.SetOpacity( hFingerSprite, 0.0f );
            GOMan.AddChild( hTutorialColumn, hFingerSprite );
            LayerMan.AddToLayer( hPlayScreenColumn, hTutorialColumn );

        OnMsg( MSG_TutorialPopulateWithCritters )
            MAP_POSITION startPos, endPos;
            for (int row = 0; row < 2; ++row)
            {
                for (int col = 0; col < GAME_GRID_NUM_COLUMNS; ++col)
                {
                    int top;
                    if (Platform::IsWidescreen()) {
                        top = GAME_GRID_NUM_ROWS;
                    } else {
                        top = GAME_GRID_NUM_ROWS_3_5;
                    }
                    startPos.x = col;
                    startPos.y = top-1;
                    endPos.x   = col;
                    endPos.y   = row;

                    GenerateHomeScreenCritter( startPos, endPos, true, &tutorialGroundBrickTypes[col + row*GAME_GRID_NUM_COLUMNS] );
                }
            }
            g_pGameMap->Update();
            
        
        OnMsg( MSG_TutorialDropColumn )
            GOMan.SendMessageFromSystem( hTutorialColumn, MSG_ColumnDropOneUnit );

        OnMsg( MSG_TutorialShowText )
            const char* pString = (const char*)msg->GetPointerData();
            if (pString)
            {
                dialog.message = [NSString stringWithCString:pString encoding:NSUTF8StringEncoding];
                dialog.cancelButtonLabel = nil;
                dialog.confirmButtonLabel = @"OK";

                Callback confirm( &GameScreens::OnDialogConfirmCallback, &m_hOwner );
                Callback cancel ( &GameScreens::OnDialogCancelCallback,  &m_hOwner );
                dialog.onConfirm = confirm;
                dialog.onCancel  = cancel;
                
                tutorialTime.Pause();
                [dialog show];
            }

        OnMsg( MSG_TutorialShowFinger )
            HStoryboard hShowFinger;
            Storyboards.GetCopy( "FadeIn", &hShowFinger );
            Storyboards.BindTo( hShowFinger, hTutorialColumn );
            Storyboards.SetReleaseOnFinish( hShowFinger, true );
            Storyboards.Start( hShowFinger );

        OnMsg( MSG_TutorialHideFinger )
            HStoryboard hHideFinger;
            Storyboards.GetCopy( "FadeOut", &hHideFinger );
            Storyboards.BindTo( hHideFinger, hTutorialColumn );
            Storyboards.SetReleaseOnFinish( hHideFinger, true );
            Storyboards.Start( hHideFinger );

        OnMsg( MSG_TutorialShowFingerLeft )
            GOMan.SendMessageFromSystem( hTutorialColumn, MSG_ColumnLeft );

        OnMsg( MSG_TutorialShowFingerRight )
            GOMan.SendMessageFromSystem( hTutorialColumn, MSG_ColumnRight );
        
        OnMsg( MSG_TutorialShowFingerRotate )
            HStoryboard hTapFinger;
            Storyboards.GetCopy( "TutorialFingerTap", &hTapFinger );
            Storyboards.BindTo( hTapFinger, hTutorialColumn );
            Storyboards.Start( hTapFinger );
            GOMan.SendMessageFromSystem( hTutorialColumn, MSG_ColumnRotate );

        OnMsg( MSG_TutorialShowFingerSwipeDown )
            HStoryboard hSwipeFinger;
            Storyboards.GetCopy( "TutorialFingerSwipe", &hSwipeFinger );
            Storyboards.BindTo( hSwipeFinger, hTutorialColumn );
            Storyboards.Start( hSwipeFinger );
            SendMsgDelayed( 0.25f, MSG_ColumnDropToBottom, hTutorialColumn.GetID() );

        OnMsg( MSG_TutorialClearScreen )
            HGameObjectList list = g_pGameMap->GetListOfAllGameObjects();
            HGameObjectListIterator phGO;
            for (phGO = list.begin(); phGO != list.end(); ++phGO)
            {
                LayerMan.RemoveFromLayer( hPlayScreenGrid,   *phGO );
                LayerMan.RemoveFromLayer( hPlayScreenColumn, *phGO );
                GOMan.Remove( *phGO );
            }
            g_pGameMap->Clear();

        OnMsg( MSG_TutorialFinished )
            SendMsgBroadcast( MSG_GameScreenLevel );
            ChangeState( STATE_LevelSelect );



        OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_Tutorial");
        
            // HACK: tell some of the game logic not to run, like recording high scores.
            g_isTutorialMode = true;

            // Show play screen
            // HACK HACK: don't render score/level.
            g_showScore = false;


            // Remove any bricks hanging out from a previous game.
            HGameObjectList list = g_pGameMap->GetListOfAllGameObjects();
            HGameObjectListIterator phGO;
            for (phGO = list.begin(); phGO != list.end(); ++phGO)
            {
                LayerMan.RemoveFromLayer( hPlayScreenGrid,   *phGO );
                LayerMan.RemoveFromLayer( hPlayScreenColumn, *phGO );
                GOMan.Remove( *phGO );
            }
            g_pGameMap->Clear();

            //
            // Undarken the screen
            //
            Storyboards.Start( hLightenScreenStoryboard );
            Callback lightenDone( GameScreens::LightenScreenDoneCallback );
            StoryboardMan.CallbackOnFinished( hLightenScreenStoryboard, lightenDone );

            // HACK: Clear any BlurEffect previously set on the Sky.
            LayerMan.SetEffect( hRootLayer,      HEffect::NullHandle() );
            LayerMan.SetEffect( hSkyLayer,       HEffect::NullHandle() );
            LayerMan.SetEffect( hPlayScreenGrid, HEffect::NullHandle() );

            GenerateBackground();
            LayerMan.SetVisible( hSkyLayer,             true );
            LayerMan.SetVisible( hPlayScreenBackground, true );
            LayerMan.SetVisible( hPlayScreenGrid,       true );
            LayerMan.SetVisible( hPlayScreenColumn,     true );
            LayerMan.SetVisible( hPlayScreenForeground, true );
            LayerMan.SetVisible( hPlayScreenAlerts,     true );

            LayerMan.SetShadow( hPlayScreenBackground,  true );
            LayerMan.SetShadow( hPlayScreenColumn,      true );
            LayerMan.SetShadow( hPlayScreenGrid,        true );

            LayerMan.Show( hPlayScreenGrid, hShowGameGridStoryboard );

            g_tutorialActionsIndex = 0;
            g_tutorialStartTime    = tutorialTime.GetTimeDouble();


        OnFrameUpdate
            DebugRender.Text( "Tutorial Mode" );
    
            if (g_tutorialActionsIndex < g_numTutorialActions)
            {
                TutorialAction* pAction = &g_tutorialActions[ g_tutorialActionsIndex ];
        
                float elapsed = tutorialTime.GetTimeDouble() - g_tutorialStartTime;
                float timestamp = pAction->timestamp;
                if (elapsed >= timestamp)
                {
                    SendMsgToState( pAction->message, (void*)pAction->dialogText );
                    g_tutorialActionsIndex++;
                }
            }
            

        OnExit
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_Tutorial: Exit");
            GOMan.Release( hTutorialColumn );
            
            // Send the home screen critters to Nirvana.
            HGameObjectListIterator pHGameObject;
            for ( pHGameObject = g_homescreenCritters.begin(); pHGameObject != g_homescreenCritters.end(); /* */)
            {
                HGameObject hCritter = *pHGameObject;
                Layers.RemoveFromLayer( hPlayScreenBackground, hCritter );
                hCritter.Release();
                pHGameObject = g_homescreenCritters.erase( pHGameObject );
            }
            
            SceneMan.Hide( hTutorialScene, hHideTutorialSceneStoryboard, hTutorialSceneEffect );

            g_isTutorialMode = false;
        

    
	///////////////////////////////////////////////////////////////
    #pragma mark STATE_Home
	DeclareState( STATE_Home )

        OnMsg( MSG_SpawnCritter )
            GenerateHomeScreenCritter();
            SendMsgDelayedToState( Platform::RandomDouble(1.0f, 2.0f), MSG_SpawnCritter );


        OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_Home");

            // HACK HACK: don't render score/level.
            g_showScore = false;

            GenerateBackground();
            GenerateHomeScreenCritter();

                   
            //
            // Blur out game area
            //
            LayerMan.SetEffect( hRootLayer, hBlurEffect );
            StoryboardMan.BindTo( hBlurOutStoryboard, hBlurEffect );
            StoryboardMan.Start( hBlurOutStoryboard );

            //
            // Darken the screen
            //
            Layers.AddToLayer( hPlayScreenForeground, hVignetteSprite );
            Storyboards.Start( hDarkenScreenStoryboard );

            //
            // Hide game grid
            //
            if ( LayerMan.GetVisible( hPlayScreenGrid ) )
            {
                LayerMan.Hide( hPlayScreenGrid, hHideGameGridStoryboard );
            }

            //
            // Show home menu
            //
            LayerMan.SetVisible( hMenuLayer, true );
            Callback callback( GameScreens::ShowHomeScreenDoneCallback );
            Storyboards.CallbackOnFinished( hShowHomeSceneStoryboard, callback );
            SceneMan.Show( hHomeScene, hShowHomeSceneStoryboard, hHomeSceneEffect );

            SendMsgDelayedToState( Platform::RandomDouble(1.0f, 2.0f), MSG_SpawnCritter );

        OnExit
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_Home: Exit");

            // Send the home screen critters to Nirvana.
            HGameObjectListIterator pHGameObject;
            for ( pHGameObject = g_homescreenCritters.begin(); pHGameObject != g_homescreenCritters.end(); /* */)
            {
                HGameObject hCritter = *pHGameObject;
                Layers.RemoveFromLayer( hPlayScreenBackground, hCritter );
                hCritter.Release();
                pHGameObject = g_homescreenCritters.erase( pHGameObject );
            }

            LayerMan.SetVisible( hMenuLayer, false );
            SceneMan.Hide( hHomeScene, hHideHomeSceneStoryboard, hHomeSceneEffect );

            g_showScore = true;
            
    
    ///////////////////////////////////////////////////////////////
    #pragma mark STATE_NewGame
	DeclareState( STATE_NewGame )
        OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_NewGame");
        
            Platform::LogAnalyticsEvent( "MSG_NewGame" );

            // TODO: flourish sound, particles, "Go!" billboard, etc.
            g_showScore     = true;
            g_totalScore    = 0;
            g_newHighScore  = false;

            // HACK: Clear any BlurEffect previously set on the Sky.
            LayerMan.SetEffect( hRootLayer,      HEffect::NullHandle() );
            LayerMan.SetEffect( hSkyLayer,       HEffect::NullHandle() );
            LayerMan.SetEffect( hPlayScreenGrid, HEffect::NullHandle() );
    
            GenerateBackground();


            //
            // Create a Column.
            //
            // TODO: rename Column/ColumnState to ColumnController.
            //
            if (hColumn.IsNull())
            {
                GOMan.Create( "ColumnController",   &hColumn,  "",  "", "", "ColumnState", GO_TYPE_SPRITE );
                TouchScreen.AddListener( hColumn, MSG_TouchBegin  );
                TouchScreen.AddListener( hColumn, MSG_TouchUpdate );
                TouchScreen.AddListener( hColumn, MSG_TouchEnd    );
            }


            //
            // Undarken the screen
            //
            Callback lightenDone( GameScreens::LightenScreenDoneCallback );
            StoryboardMan.CallbackOnFinished( hLightenScreenStoryboard, lightenDone );
            Storyboards.Start( hLightenScreenStoryboard );

            // SO UGLY.  TODO: We need to make all resource-related stuff so much cleaner!!!!
            Rectangle screenRect;
            AABB spriteBounds;
            HSprite hSprite;
            Platform::GetScreenRectCamera( &screenRect );
            SpriteMan.GetCopy( "NewGame", &hSprite );
            spriteBounds = SpriteMan.GetBounds( hSprite );
            SpriteMan.Release( hSprite );
            vec3 position(screenRect.width/2 - spriteBounds.GetWidth()/2, screenRect.width/3, 0);
            GOMan.Create( "NewGame", "NewGame", hPlayScreenAlerts, position, NULL, 1.0f, Color::White(), true );
            LayerMan.SetVisible( hPlayScreenAlerts, true );

            float delay = StoryboardMan.GetDurationMS( "NewGame" );
            ChangeStateDelayed( delay/1000.0f, STATE_Play );

        OnExit
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_NewGame: Exit");

        

	///////////////////////////////////////////////////////////////
    #pragma mark STATE_Play
	DeclareState( STATE_Play )
        OnMsg( MSG_RemoveEffect )
            LayerMan.SetEffect( hRootLayer, HEffect::NullHandle() );
        
    
        OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_Play");
            
            g_showScore = true;

            SendMsgBroadcast( MSG_BeginPlay );
    
            //
            // Undarken the screen
            //
            Callback lightenDone( GameScreens::LightenScreenDoneCallback );
            StoryboardMan.CallbackOnFinished( hLightenScreenStoryboard, lightenDone );
            Storyboards.Start( hLightenScreenStoryboard );

            //
            // Flip in the game grid and active column
            //
            if (animateGamePiecesOntoScreen)
            {
                LayerMan.Show( hPlayScreenGrid,   hShowGameGridStoryboard );
                LayerMan.Show( hPlayScreenColumn, hShowColumnStoryboard );
            }

            LayerMan.SetVisible( hPlayScreenGrid,       true );
            LayerMan.SetVisible( hPlayScreenColumn,     true );
            LayerMan.SetVisible( hPlayScreenForeground, true );
            LayerMan.SetVisible( hPlayScreenAlerts,     true );
            SceneMan.Show( hHUDScene, hShowHUDSceneStoryboard );

            // TODO: a container Layer to make this simpler, but which doesn't include the sky (or do we want clouds to cast shadows?)
            LayerMan.SetShadow( hPlayScreenBackground, true );
            LayerMan.SetShadow( hPlayScreenGrid,       true );
            LayerMan.SetShadow( hPlayScreenColumn,     true );
            LayerMan.SetShadow( hPlayScreenForeground, true );
            LayerMan.SetShadow( hPlayScreenAlerts,     false);    // TODO: bake shadow into the bubble-ups, so they fade properly.
    

            // Unblur game area if resuming from Pause
            if ( hBlurEffect == LayerMan.GetEffect( hRootLayer ) )
            {
                float delay = StoryboardMan.GetDurationMS( hBlurInStoryboard )/1000.0f;
                LayerMan.SetEffect( hRootLayer, hBlurEffect );
                StoryboardMan.BindTo( hBlurInStoryboard, hBlurEffect );
                StoryboardMan.Start( hBlurInStoryboard );
                
                SendMsgDelayedToState(delay, MSG_RemoveEffect);
            }

        OnExit
            RETAILMSG(ZONE_STATEMACHINE, "GameScreen: STATE_Play: Exit");

            SceneMan.Hide( hHUDScene, hHideHUDSceneStoryboard );
    

    
	///////////////////////////////////////////////////////////////
    #pragma mark STATE_PauseMenu
	DeclareState( STATE_PauseMenu )
        OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_PauseMenu");

            g_showScore = false;

            //
            // Blur out game area
            //
            LayerMan.SetEffect( hRootLayer, hBlurEffect );
            StoryboardMan.BindTo( hBlurOutStoryboard, hBlurEffect );
            StoryboardMan.Start( hBlurOutStoryboard );

            //
            // Darken the screen
            //
            Layers.AddToLayer( hPlayScreenForeground, hVignetteSprite );
            Storyboards.Start( hDarkenScreenStoryboard );


            LayerMan.SetVisible( hPlayScreenAlerts,     false );
            LayerMan.Hide      ( hPlayScreenColumn,     hHideColumnStoryboard );
            LayerMan.Hide      ( hPlayScreenGrid,       hHideGameGridStoryboard );
    
            LayerMan.SetVisible( hMenuLayer, true );
            SceneMan.Show( hPauseScene, hShowPauseSceneStoryboard, hPauseSceneEffect );

        OnExit
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_PauseMenu: Exit");

            Game::SaveUserPreferences();

            SceneMan.Hide( hPauseScene, hHidePauseSceneStoryboard, hPauseSceneEffect );
            LayerMan.SetVisible( hMenuLayer, false );

            animateGamePiecesOntoScreen = true;

    
	///////////////////////////////////////////////////////////////
    #pragma mark STATE_OptionsMenu
    DeclareState( STATE_OptionsMenu )
        OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_OptionsMenu");
            
            g_showScore = false;

            LayerMan.SetVisible( hMenuLayer, true );
            SceneMan.Show( hOptionsScene, hShowOptionsSceneStoryboard, hOptionsSceneEffect );

            
        OnExit
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_OptionsMenu: Exit");
            
            LayerMan.SetVisible( hMenuLayer, false );
            SceneMan.Hide( hOptionsScene, hHideOptionsSceneStoryboard, hOptionsSceneEffect );
            animateGamePiecesOntoScreen = true;
    

        
	///////////////////////////////////////////////////////////////
    #pragma mark STATE_ConfirmQuit
    DeclareState( STATE_ConfirmQuit )
        OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_ConfirmQuit");
            
            g_showScore = false;

            [confirmViewController setMessage:@"Really\nQuit?"];
            LayerMan.SetVisible( hMenuLayer, true );
            SceneMan.Show( hConfirmQuitScene, hShowConfirmQuitSceneStoryboard, hConfirmQuitSceneEffect );
    
        OnExit
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_ConfirmQuit: Exit");
            
            LayerMan.SetVisible( hMenuLayer, false );
            SceneMan.Hide( hConfirmQuitScene, hHideConfirmQuitSceneStoryboard, hConfirmQuitSceneEffect );
            animateGamePiecesOntoScreen = true;
            
        
    
    
	///////////////////////////////////////////////////////////////
    #pragma mark STATE_LevelUp
    DeclareState( STATE_LevelUp )

        OnEnter
            RETAILMSG(ZONE_INFO, "------------------------------------");
            RETAILMSG(ZONE_INFO, "---      Level Up: %2d            ---", g_pLevel->level);
            RETAILMSG(ZONE_INFO, "------------------------------------");
            RETAILMSG(ZONE_INFO, "Minutes: %2.2f", (double)(GameTime.GetTime() - g_startGameTime)/60000.0 );

            ChangeSubstateDelayed(0.3f, SUBSTATE_LevelUp_HighliteLevel);

        OnExit
            // Prevent the flip-in animation when returning to STATE_Play
            animateGamePiecesOntoScreen = false;

            
        DeclareSubstate( SUBSTATE_LevelUp_HighliteLevel )
            OnEnter
                RETAILMSG(ZONE_STATEMACHINE, "GameScreens: SUBSTATE_LevelUp_HighliteLevel");

                Sounds.Play( hLevelUpSound );

                Rectangle screen;
                Platform::GetScreenRect(&screen);

                // Have some particles burst out of the level indicator.
                vec3 pos = vec3( screen.width/2, screen.height, 0 );

                HParticleEmitter hEmitter;
                Particles.GetCopy( "RainOfStars", &hEmitter );
                Particles.SetPosition( hEmitter, pos );
                LayerMan.AddToLayer( hPlayScreenForeground, hEmitter );
                Particles.Start( hEmitter );

                ChangeSubstateDelayed(0.3f, SUBSTATE_LevelUp_HideBackground);


        DeclareSubstate( SUBSTATE_LevelUp_HideBackground )
            OnEnter
                RETAILMSG(ZONE_STATEMACHINE, "GameScreens: SUBSTATE_LevelUp_HideBackground");

                // Show bubble up
                Rectangle screenRect;
                Platform::GetScreenRectCamera( &screenRect );
                WORLD_POSITION position = WORLD_POSITION( (screenRect.width-640.0)/2.0, screenRect.height/3.0, 0.0 );
                GOMan.Create( "levelup", "LevelUp", hPlayScreenAlerts, position, NULL, 1.0f, Color::White(), true );

                // Fade out the background.
                float duration = StoryboardMan.GetDurationMS( hFadeOutBackgroundStoryboard ) / 1000.0f;
                duration      += StoryboardMan.GetDurationMS( hFadeInBackgroundStoryboard  ) / 1000.0f;     

                if (duration > 0)
                {
                    ChangeStateDelayed( duration, STATE_Play );
                }
                else
                {
                    ChangeState( STATE_Play );
                }
            


    
    ///////////////////////////////////////////////////////////////
    #pragma mark STATE_GameOver
	DeclareState( STATE_GameOver )

        OnMsg( MSG_HidePlayScreen )
            RETAILMSG(ZONE_INFO, "MSG_HidePlayScreen");
            LayerMan.SetVisible( hPlayScreenColumn,     false );

            // Kill the critters.
            SendMsgBroadcast( MSG_KillCritter );



        OnMsg( MSG_ShowGameOverScene )
            RETAILMSG(ZONE_INFO, "MSG_ShowGameOverScene");

            //
            // Blur out the game.
            //
            // BUG BUG: setting Blur on RootLayer removes Monochrome
            // from the GameGrid?
            LayerMan.SetEffect( hSkyLayer, hBlurEffect );
            StoryboardMan.BindTo( hBlurOutStoryboard, hBlurEffect );
            StoryboardMan.Start( hBlurOutStoryboard );

            SceneMan.Show( hGameOverScene, hShowGameOverSceneStoryboard, hGameOverSceneEffect );

            // HACK: just to stop rendering score/level!!
            g_showScore = false;


        OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_GameOver");
            
            g_highScore = MAX(g_highScore, g_totalScore);

            // Fade game area to monochrome.
            LayerMan.SetEffect( hPlayScreenGrid, hMonochromeEffect );
            StoryboardMan.BindTo( hColorToMonochromeStoryboard, hMonochromeEffect );
            StoryboardMan.Start( hColorToMonochromeStoryboard );

            float delay = StoryboardMan.GetDurationMS( hColorToMonochromeStoryboard )/1000.0f;
            SendMsgDelayedToState( delay,        MSG_HidePlayScreen    );
            SendMsgDelayedToState( delay + 1.5f, MSG_ShowGameOverScene );
           
            // Sad critters.
            SendMsgBroadcast( MSG_SadCritter );


        OnExit
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_GameOver: Exit");

            SceneMan.Hide( hGameOverScene, hHideGameOverSceneStoryboard, hGameOverSceneEffect );
            LayerMan.SetEffect( hRootLayer,      HEffect::NullHandle() );
    
            // Remove any bricks hanging out from a previous game.
            HGameObjectList list = g_pGameMap->GetListOfAllGameObjects();
            HGameObjectListIterator phGO;
            for (phGO = list.begin(); phGO != list.end(); ++phGO)
            {
                LayerMan.RemoveFromLayer( hPlayScreenGrid,   *phGO );
                LayerMan.RemoveFromLayer( hPlayScreenColumn, *phGO );
                GOMan.Remove( *phGO );
            }
            g_pGameMap->Clear();
    

	///////////////////////////////////////////////////////////////
    #pragma mark STATE_Credits
	DeclareState( STATE_Credits )
        OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_Credits");
            g_showScore = false;
            SceneMan.Show( hAboutScene, hShowAboutSceneStoryboard, hAboutSceneEffect );
        
        OnExit
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_Credits: Exit");
            SceneMan.Hide( hAboutScene, hHideAboutSceneStoryboard, hAboutSceneEffect );

    
EndStateMachine
}



} // END namespace Z


