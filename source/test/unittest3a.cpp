/* Copyright Steve Rabin, 2007. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright Steve Rabin, 2007"
 */

#include "unittest3a.h"


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
	STATE_Success
};

//Add new substates here
enum SubstateName {
	SUBSTATE_Inside1,
	SUBSTATE_Inside2
};

//unittest3 covers:
//1. SendMsg
//2. SendMsgBroadcast
//3. SendMsgBroadcastToList
//4. BroadcastAddToList
//5. SetCCReceiver
//6. OnMsgCC

bool UnitTest3a::States( State_Machine_Event event, MSG_Object * msg, int state, int substate )
{
BeginStateMachine

	//Global message responses
	
	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Initialize )

		OnEnter
			ChangeStateDelayed( 1.0f, STATE_Chain1 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain1 )

		OnEnter
			SendMsgBroadcast( MSG_UnitTestPing );

		OnMsg( MSG_UnitTestAck )
			BroadcastAddToList( msg->GetSender() );
			SetCCReceiver( msg->GetSender() );
			SendMsgBroadcastToList( MSG_UnitTestAck );

		OnMsg( MSG_UnitTestDone )
			ClearCCReceiver();
			SendMsgDelayedToState( 1.0f, MSG_UnitTestMessage );
			
		OnMsg( MSG_UnitTestMessage )
			ChangeState( STATE_Success );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Success )

		OnEnter
			RETAILMSG(ZONE_INFO, "UnitTest3a Success\n" );


EndStateMachine
}



} // END namespace Z


