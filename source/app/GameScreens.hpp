#pragma once
#include "StateMachine.hpp"
#include "GameState.hpp"


namespace Z
{



//
// This StateMachine controls progress through the various game screens/modes.
//

class GameScreens : public StateMachine  // TODO: ResourceManager<UIView>
{
public:
	GameScreens( HGameObject &hGameObject );
	~GameScreens( void );

    // TODO: AddView( IN const string& name, IN UIView* pView, INOUT hView )

private:
	virtual bool States( State_Machine_Event event, MSG_Object * msg, int state, int substate );


    static void  ShowMenuDoneCallback       ( void* pContext );
    static void  HideMenuDoneCallback       ( void* pContext );
    static void  BackgroundFadeDoneCallback ( void* pContext );
    static void  DropCritterCallback        ( void* pContext );
    static void  ShowHomeScreenDoneCallback ( void* pContext );
    static void  LightenScreenDoneCallback  ( void* pContext );
    static void  OnDialogConfirmCallback    ( void* pContext );
    static void  OnDialogCancelCallback     ( void* pContext );
    
    static RESULT GenerateBackground        ( );
    static RESULT GenerateHill              ( IN HSprite* pHSprite );   // TODO: HMesh (or vertex[]) for shaped hills like Tiny Wings
    static RESULT GenerateHomeScreenCritter ( MAP_POSITION startPos = MAP_POSITION(-1,-1), MAP_POSITION endPos = MAP_POSITION(-1,-1), bool playSound = false, BrickType* pBrickType = NULL );
};



} // END namespace Z


