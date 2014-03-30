/* Copyright Steve Rabin, 2007. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright Steve Rabin, 2007"
 */

#include "unittest1.h"
#include "msgroute.hpp"

#include <stdio.h>


namespace Z
{


//Add new states here
enum StateName {
	STATE_Initialize,	//Note: the first enum is the starting state
	STATE_Chain1,
	STATE_Chain2,
	STATE_Chain3,
	STATE_Chain4,
	STATE_Chain5,
	STATE_Chain6,
	STATE_Chain7,
	STATE_Chain8,
	STATE_Chain9,
	STATE_Chain10,
	STATE_Chain11,
	STATE_Chain12,
	STATE_Chain13,
	STATE_Chain14,
	STATE_Success,
	STATE_Failure
};

//Add new substates here
enum SubstateName {
	SUBSTATE_Inside1,
	SUBSTATE_Inside2
};

//unittest1 covers:
//1. BeginStateMachine, EndStateMachine
//2. DeclareState, DeclareSubstate
//3. OnMsg
//4. OnEnter, OnExit
//5. OnUpdate
//6. ChangeState, ChangeStateDelayed
//7. ChangeSubstate, ChangeSubstateDelayed
//8. SetTimer
//9. SendMsgDelayedToMe
//10. SendMsgToMe
//11. PopState


bool UnitTest1::States( State_Machine_Event event, MSG_Object * msg, int state, int substate )
{
BeginStateMachine

	//Global message responses
	OnMsg( MSG_UnitTestMessage )
		ChangeState( STATE_Chain4 );
	

	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Initialize )

		OnEnter
			ChangeStateDelayed( RandDelay( 1.0f, 3.0f ), STATE_Chain1 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain1 )

		OnEnter
			ChangeStateDelayed( 1.0f, STATE_Chain2 );
			ChangeStateDelayed( 1.5f, STATE_Chain1 );	//Should be ignored when state changes because of scoping


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain2 )

		OnEnter
			SendMsgDelayedToState( 1.0f, MSG_UnitTestMessage );

		OnMsg( MSG_UnitTestMessage )
			ChangeState( STATE_Chain3 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain3 )

		OnEnter
		SendMsgDelayedToState( 1.0f, MSG_UnitTestMessage );	//Will get caught by the global message response


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain4 )

		OnEnter
			m_count = 0;
			SetTimerState( 1.18f, MSG_UnitTestTimer );

		OnMsg( MSG_UnitTestTimer )
			m_count++;
			if( m_count == 5 )
			{
				ChangeState( STATE_Chain5 );
			}


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain5 )

		OnEnter
			ChangeSubstate( SUBSTATE_Inside1 );

		OnMsg( MSG_UnitTestTimer )
			ASSERTMSG( 0, "Shouldn't get here. The timer from the previous state should get killed due to scoping." );

		DeclareSubstate( SUBSTATE_Inside1 )
			OnEnter
				ChangeSubstateDelayed( 1.0f, SUBSTATE_Inside2 );

		DeclareSubstate( SUBSTATE_Inside2 )
			OnEnter
				ChangeStateDelayed( 1.0f, STATE_Chain6 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain6 )
		
		OnEnter
			SendMsgDelayedToStateMachine( 2.5f, MSG_UnitTestMessage, GetScopeState() );	//Message will be sent regardless of current state
			ChangeStateDelayed( 1.0f, STATE_Chain7 );

		OnMsg( MSG_UnitTestMessage )
			ChangeState( STATE_Chain8 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain7 )
		
		OnEnter
			SendMsgDelayedToState( 1.0f, MSG_UnitTestMessage2 );

		OnMsg( MSG_UnitTestMessage2 )
			PopState();


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain8 )
		
		OnEnter
			ChangeState( STATE_Chain9 );
		
		OnExit
			SendMsgDelayedToState( 1.0f, MSG_UnitTestMessage, SCOPE_TO_STATE_MACHINE );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain9 )
		
		OnMsg( MSG_UnitTestMessage )
			ChangeState( STATE_Chain10 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain10 )
		
		OnFrameUpdate
			ChangeState( STATE_Chain11 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain11 )
		
		OnEnter
			SendMsgToState( MSG_UnitTestMessage );

		OnMsg( MSG_UnitTestMessage );
			ChangeState( STATE_Chain12 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain12 )
		
		OnEnter
			SendMsgDelayedToState( 1.0f, MSG_UnitTestMessage, 3 );
			SendMsgDelayedToState( 1.0f, MSG_UnitTestMessage, 2 );
			SendMsgDelayedToState( 1.0f, MSG_UnitTestMessage, 1 );
		
		OnMsg( MSG_UnitTestMessage );
		if( msg->GetIntData() == 2)
		{
			ChangeState( STATE_Chain13 );
		}


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain13 )
		
		OnEnter
			SendMsgToState( MSG_UnitTestMessage );
			ChangeState( STATE_Chain14 );

		OnMsg( MSG_UnitTestMessage );
			ChangeState( STATE_Failure );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain14 )
		
		OnEnter
			SendMsgDelayedToState( 1.0f, MSG_UnitTestMessage );
			SendMsgDelayedToState( 1.3f, MSG_UnitTestMessage2 );
			SendMsgDelayedToState( 1.2f, MSG_UnitTestMessage3 );
			SendMsgDelayedToState( 1.0f, MSG_UnitTestMessage4 );
			SendMsgDelayedToState( 0.01f, MSG_UnitTestMessage5 );
			SendMsgDelayedToState( 2.0f, MSG_UnitTestMessage6 );
			SendMsgDelayedToState( 0.001f, MSG_UnitTestMessage7 );
			SendMsgDelayedToState( 1.99f, MSG_UnitTestMessage8 );
			SendMsgDelayedToState( 2.01f, MSG_UnitTestMessage9 );
			if( !MsgRouter.VerifyDelayedMessageOrder() )
			{
				ChangeState( STATE_Failure );
			}

		OnFrameUpdate
			if( !MsgRouter.VerifyDelayedMessageOrder() )
			{
				ChangeState( STATE_Failure );
			}

		OnMsg( MSG_UnitTestMessage )	//Consume message
		OnMsg( MSG_UnitTestMessage2 )	//Consume message
		OnMsg( MSG_UnitTestMessage3 )	//Consume message
		OnMsg( MSG_UnitTestMessage4 )	//Consume message
		OnMsg( MSG_UnitTestMessage5 )	//Consume message
		OnMsg( MSG_UnitTestMessage6 )	//Consume message
		OnMsg( MSG_UnitTestMessage7 )	//Consume message
		OnMsg( MSG_UnitTestMessage8 )	//Consume message

		OnMsg( MSG_UnitTestMessage9 )
			if( MsgRouter.VerifyDelayedMessageOrder() )
			{
				ChangeState( STATE_Success );
			}
			else
			{
				ChangeState( STATE_Failure );
			}


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Success )
		
		OnEnter
			RETAILMSG(ZONE_INFO, "UnitTest1 Success\n" );
			
		
	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Failure )
		
		OnEnter
			RETAILMSG(ZONE_INFO, "UnitTest1 Failure\n" );


EndStateMachine
}



} // END namespace Z



