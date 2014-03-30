//
//  TutorialState.cpp
//  Critters
//
//  Created by Sean Kelleher on 5/27/13.
//
//

#include "TutorialState.hpp"
#include "Log.hpp"
#include "SceneManager.hpp"
#include "Game.hpp"
#include "Engine.hpp"
//#include "GameScreens.hpp"
//#include "Log.hpp"
//#include "LayerManager.hpp"
//#include "SpriteManager.hpp"
//#include "Engine.hpp"
//#include "IdleState.hpp"
#include "ColumnState.hpp"
//#include "GameObjectManager.hpp"
//#include "GameState.hpp"
//#include "SoundManager.hpp"

// TEST:
//#include "test.hpp"
//#include "AnimationManager.hpp"
#include "StoryboardManager.hpp"
//#include "RippleEffect.hpp"
//#include "BlurEffect.hpp"
//#include "PixelEffect.hpp"
//#include "MorphEffect.hpp"
//#include "Property.hpp"

#include "DialogViewController.h"


//#import <UIKit/UIKit.h>
//extern UIViewController*    tutorialViewController;


namespace Z
{


static HGameObject  hTutorialColumn;
static HScene       hTutorialScene;
static HEffect      hTutorialSceneEffect;
static HStoryboard  hShowTutorialSceneStoryboard;
static HStoryboard  hHideTutorialSceneStoryboard;
static HLayer       hPlayScreenColumn;
static HLayer       hPlayScreenForeground;


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
    STATE_Tutorial,
    STATE_Idle
};

//Add new substates here
enum SubstateName {
	//empty
};



void
TutorialState::LightenScreenDoneCallback( void* )
{
    Layers.RemoveFromLayer( hPlayScreenForeground, hVignetteSprite );
}



void
TutorialState::OnDialogConfirmCallback( void* pContext )
{
    HGameObject hGO = *(HGameObject*)pContext;
    
    [dialog hide];
    
    GameObjects.SendMessageFromSystem( hGO, MSG_DialogConfirmed );
}



void
TutorialState::OnDialogCancelCallback( void* pContext )
{
    HGameObject hGO = *(HGameObject*)pContext;
    
    [dialog hide];

    GameObjects.SendMessageFromSystem( hGO, MSG_DialogCancelled );
}





TutorialState::TutorialState( HGameObject &hGameObject ) :
    StateMachine( hGameObject )
{
}

TutorialState::~TutorialState( void )
{
}


bool TutorialState::States( State_Machine_Event event, MSG_Object * msg, int state, int substate )
{
BeginStateMachine

	//Global message responses
	OnMsg( MSG_Reset )
		ResetStateMachine();
        //RETAILMSG(ZONE_STATEMACHINE, "TutorialState: MSG_Reset");


    ///////////////////////////////////////////////////////////////
	DeclareState( STATE_Initialize )

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "TutorialState: STATE_Initialize");
            
            //SceneMan.CreateScene ( "TutorialScene",     tutorialViewController,        &hTutorialScene );
            EffectMan.GetCopy    ( "MorphEffect",       &hTutorialSceneEffect );
            StoryboardMan.GetCopy( "TwistIn",           &hShowTutorialSceneStoryboard );
            StoryboardMan.GetCopy( "TwistOut",          &hHideTutorialSceneStoryboard );
            Layers.GetCopy       ( "PlayScreenColumn",  &hPlayScreenColumn );

			ChangeState( STATE_Idle );
	

	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Idle )

		OnEnter
            //RETAILMSG(ZONE_STATEMACHINE, "TutorialState: STATE_Idle");

        OnFrameUpdate



	///////////////////////////////////////////////////////////////
    #pragma mark STATE_Tutorial
	DeclareState( STATE_Tutorial )
    
//        OnMsg( MSG_TutorialShowTitle )
//            SceneMan.Show( hTutorialScene, hShowTutorialSceneStoryboard, hTutorialSceneEffect );

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
                    startPos.x = col;
                    startPos.y = GAME_GRID_NUM_ROWS-1;
                    endPos.x   = col;
                    endPos.y   = row;

                    GenerateHomeScreenCritter( startPos, endPos, true, &tutorialGroundBrickTypes[col + row*GAME_GRID_NUM_COLUMNS] );
                }
            }
            g_gameMap.Update();
            
        
        OnMsg( MSG_TutorialDropColumn )
            GOMan.SendMessageFromSystem( hTutorialColumn, MSG_ColumnDropOneUnit );

        OnMsg( MSG_TutorialShowText )
            const char* pString = (const char*)msg->GetPointerData();
            if (pString)
            {
                //dialog.message = @"Drag Critters left, right, and down.";
                dialog.message = [NSString stringWithCString:pString encoding:NSUTF8StringEncoding];
                dialog.cancelButtonLabel = nil;
                dialog.confirmButtonLabel = @"OK";

                Callback confirm( &GameScreens::OnDialogConfirmCallback, &m_hOwner );
                Callback cancel ( &GameScreens::OnDialogCancelCallback,  &m_hOwner );
                dialog.onConfirm = confirm;
                dialog.onCancel  = cancel;
                
                [dialog show];
                //ChangeSubstate( SUBSTATE_WaitingForDialogToClose );
            
                // TODO: push to pause state, pop when user closes dialog.
                
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
            ChangeStateDelayed( 3.5f, STATE_Tutorial );


        OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "GameScreens: STATE_Tutorial");
        
            // HACK: tell some of the game logic not to run, like recording high scores.
            g_isTutorialMode = true;

            // Show play screen
            // HACK HACK: don't render score/level.
            g_showScore = false;


            // Remove any bricks hanging out from a previous game.
            HGameObjectList list = g_gameMap.GetListOfAllGameObjects();
            HGameObjectListIterator phGO;
            for (phGO = list.begin(); phGO != list.end(); ++phGO)
            {
                LayerMan.RemoveFromLayer( hPlayScreenGrid,   *phGO );
                LayerMan.RemoveFromLayer( hPlayScreenColumn, *phGO );
                GOMan.Remove( *phGO );
            }
            g_gameMap.Clear();

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

//            SoundMan.Play( hShowSound );
        
            SendMsgToState( MSG_TutorialShowTitle );
            SendMsgDelayedToState(1.0f, MSG_TutorialPopulateWithCritters);
            
            SendMsgDelayedToState(2.0f, MSG_TutorialShowText, (void*)&"Let's learn how to play!");

            SendMsgDelayedToState(3.0f, MSG_TutorialCreateColumn);
            SendMsgDelayedToState(3.5f, MSG_TutorialDropColumn);
            SendMsgDelayedToState(4.0f, MSG_TutorialDropColumn);
            SendMsgDelayedToState(4.5f, MSG_TutorialDropColumn);

            SendMsgDelayedToState(5.0f, MSG_TutorialShowFinger);

            //SendMsgDelayedToState(2.0f, MSG_TutorialShowText, (void*)&"Swipe Critters left, right, and down.");


            // Show finger swiping side to side
            SendMsgDelayedToState(6.00f, MSG_TutorialShowFingerLeft);
            SendMsgDelayedToState(6.25f, MSG_TutorialShowFingerLeft);
            SendMsgDelayedToState(6.50f, MSG_TutorialShowFingerLeft);
            SendMsgDelayedToState(6.75f, MSG_TutorialShowFingerRight);
            SendMsgDelayedToState(7.00f, MSG_TutorialShowFingerRight);
            SendMsgDelayedToState(7.25f, MSG_TutorialShowFingerRight);
            SendMsgDelayedToState(7.50f, MSG_TutorialShowFingerRight);
            SendMsgDelayedToState(7.75f, MSG_TutorialShowFingerRight);
            SendMsgDelayedToState(8.00f, MSG_TutorialShowFingerLeft);
            SendMsgDelayedToState(8.25f, MSG_TutorialShowFingerLeft);


            // Show finger tapping to rotate
            SendMsgDelayedToState(9.5f,  MSG_TutorialShowFingerRotate);
            SendMsgDelayedToState(10.0f, MSG_TutorialShowFingerRotate);
            SendMsgDelayedToState(10.5f, MSG_TutorialShowFingerRotate);

            // Show finger swiping to drop
            SendMsgDelayedToState(12.0f, MSG_TutorialShowFingerSwipeDown);

            SendMsgDelayedToState(12.75f, MSG_TutorialHideFinger );
            
            // Show OK button
            // Go back to start


        OnFrameUpdate
            DebugRender.Text( "Tutorial Mode" );
            

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
        
        
//        DeclareSubstate( SUBSTATE_WaitingForDialogToClose )
//
//            OnMsg( MSG_DialogConfirmed )
//                PopState();
//    
//            OnMsg( MSG_DialogCancelled )
//                PopState();
//
//            OnEnter
//                RETAILMSG(ZONE_STATEMACHINE, "++GameScreens: SUBSTATE_WaitingForDialogToClose");
//                Z::Engine::Pause();
//    
//            OnExit
//                RETAILMSG(ZONE_STATEMACHINE, "--GameScreens: SUBSTATE_WaitingForDialogToClose");
//                [dialog hide];
//                Z::Engine::Resume();

    
EndStateMachine
}



} // END namespace Z





