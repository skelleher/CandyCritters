#include "IdleState.hpp"
#include "Log.hpp"



namespace Z
{



//
// This is a do-nothing StateMachine for parking at the bottom of each queue.
// This way if the last StateMachine is accidentally popped, the game won't crash.
//

//Add new states here
enum StateName {
    STATE_Initialize,
    STATE_Idle
};

//Add new substates here
enum SubstateName {
	//empty
};



IdleState::IdleState( HGameObject &hGameObject ) :
    StateMachine( hGameObject )
{
}

IdleState::~IdleState( void )
{
}


bool IdleState::States( State_Machine_Event event, MSG_Object * msg, int state, int substate )
{
BeginStateMachine

	//Global message responses
	OnMsg( MSG_Reset )
		ResetStateMachine();
        //RETAILMSG(ZONE_STATEMACHINE, "IdleState: MSG_Reset");


    ///////////////////////////////////////////////////////////////
	DeclareState( STATE_Initialize )

		OnEnter
			ChangeState( STATE_Idle );
	

	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Idle )

		OnEnter
            //RETAILMSG(ZONE_STATEMACHINE, "IdleState: STATE_Idle");

        OnFrameUpdate

EndStateMachine
}



} // END namespace Z



