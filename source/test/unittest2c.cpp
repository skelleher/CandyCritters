/* Copyright Steve Rabin, 2007. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright Steve Rabin, 2007"
 */

#include "unittest2c.h"
//#include "body.h"


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

//unittest2 covers:
//1. PushStateMachine
//2. PopStateMachine
//3. QueueStateMachine
//4. RequeueStateMachine

bool UnitTest2c::States( State_Machine_Event event, MSG_Object * msg, int state, int substate )
{
BeginStateMachine

	//Global message responses
	
	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Initialize )

		OnEnter
			ChangeStateDelayed( 1.0f, STATE_Success );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Success )

		OnEnter
			//m_owner->GetBody().SetHealth( 100 );
			PopStateMachine();
			RETAILMSG(ZONE_INFO, "UnitTest2c Success\n" );


EndStateMachine
}



} // END namespace Z


