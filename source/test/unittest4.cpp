/* Copyright Steve Rabin, 2007. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright Steve Rabin, 2007"
 */

#include "unittest4.h"



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
	STATE_Success,
	STATE_Broken
};

//Add new substates here
enum SubstateName {
	SUBSTATE_Inside1,	//Note: none of these substates will be active until explicitly transitioned to
	SUBSTATE_Inside2,
	SUBSTATE_Inside3
};

//unittest4 covers:
//Within substates: OnEnter, OnExit, OnMsg, scoping
//DisableUpdateEvents

bool UnitTest4::States( State_Machine_Event event, MSG_Object * msg, int state, int substate )
{
BeginStateMachine

	//Global message responses
	OnMsg( MSG_UnitTestMessage )
		ChangeState( STATE_Broken );
	
	OnMsg( MSG_UnitTestBroken )
		ChangeState( STATE_Broken );
	

	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Initialize )

		OnEnter
			ChangeState( STATE_Chain1 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain1 )

		OnEnter
			SendMsgDelayedToSubstate( 1.0f, MSG_UnitTestMessage );
			
		OnMsg( MSG_UnitTestMessage )
			ChangeSubstate( SUBSTATE_Inside1 );

		OnExit
			SendMsgDelayedToStateMachine( 2.0f, MSG_UnitTestMessage2 ); //Should get caught in STATE_Chain3

		DeclareSubstate( SUBSTATE_Inside1 )

			OnEnter
				SendMsgDelayedToSubstate( 1.0f, MSG_UnitTestMessage );

			OnMsg( MSG_UnitTestMessage )
				SendMsgDelayedToSubstate( 1.0f, MSG_UnitTestBroken );	//Should never be delivered due to scoping
				ChangeSubstateDelayed( 0.5f, SUBSTATE_Inside2 );

		DeclareSubstate( SUBSTATE_Inside2 )

			OnEnter
				SendMsgDelayedToSubstate( 1.0f, MSG_UnitTestMessage2 );
				
			OnMsg( MSG_UnitTestMessage2 )
				ChangeSubstate( SUBSTATE_Inside3 );

			OnExit
				SendMsgDelayedToState( 1.0f, MSG_UnitTestMessage3 );

		DeclareSubstate( SUBSTATE_Inside3 )

			OnMsg( MSG_UnitTestMessage3 )
				ChangeState( STATE_Chain2 );	//Should trigger OnExit in both substate and state

			OnExit
				SendMsgDelayedToStateMachine( 1.0f, MSG_UnitTestMessage );	//Should get caught in STATE_Chain2


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain2 )

		OnMsg( MSG_UnitTestMessage )
			ChangeState( STATE_Chain3 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain3 )

		OnMsg( MSG_UnitTestMessage2 )
			ChangeState( STATE_Chain4 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain4 )

		OnEnter
			DisableOnFrameUpdate();
			SendMsgDelayedToState( 1.0f, MSG_UnitTestMessage );

		OnFrameUpdate
			ChangeState( STATE_Broken );	//Update events were previously disabled

		OnMsg( MSG_UnitTestMessage )
			ChangeState( STATE_Chain5 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain5 )

		OnEnter
			ChangeSubstate( SUBSTATE_Inside1 );

		DeclareSubstate( SUBSTATE_Inside1 )
			
			OnEnter
				ChangeSubstate( SUBSTATE_Inside2 );

			OnExit
				SendMsgDelayedToState( 1.0f, MSG_UnitTestMessage );

		DeclareSubstate( SUBSTATE_Inside2 )
			
			OnEnter
				ChangeStateDelayed( 2.0f, STATE_Broken );

			OnMsg( MSG_UnitTestMessage )
				ChangeState( STATE_Success );

		
	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Success )

		OnEnter
			RETAILMSG(ZONE_INFO, "UnitTest4 Success\n" );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Broken )

		OnEnter
			RETAILMSG(ZONE_INFO, "UnitTest4 Broken\n" );


EndStateMachine
}



} // END namespace Z


