/* Copyright Steve Rabin, 2007. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright Steve Rabin, 2007"
 */

#include "unittest5.h"
#include "GameObjectManager.hpp"


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

//unittest5 covers:
//OnSlice
//

bool UnitTest5::States( State_Machine_Event event, MSG_Object * msg, int state, int substate )
{
BeginStateMachine

	//Global message responses
	OnMsg( MSG_UnitTestMessage )
		ChangeState( STATE_Broken );
	
	OnMsg( MSG_UnitTestBroken )
		ChangeState( STATE_Broken );

	OnSliceUpdate
		m_sliceCount++;

	OnFrameUpdate
		m_updateCount++;
	

	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Initialize )

		OnEnter
			MsgRouter.SetSlicePolicy( SLICE_POLICY_CONSTRAIN_BY_PROPORTION, 0.5f );	//Note: this is a global setting, so it's best to put this outside of state machines
			ChangeStateDelayed( 3.0f, STATE_Chain1 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain1 )

		OnEnter
			//Test if OnSlice gets called 10 times, every 0.5 seconds
			EnableOnSliceUpdate( 0.5f );
			m_sliceCount = 0;
			ChangeStateDelayed( 5.2f, STATE_Chain2 );
			


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain2 )

		OnEnter
			DisableOnSliceUpdate();
			if(m_sliceCount != 10) {
				ChangeState( STATE_Broken );
			}
			else {
				ChangeState( STATE_Chain3 );
			}


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain3 )

		OnEnter
			//Test if OnSlice gets called half as many times as OnUpdate
			EnableOnSliceUpdate( 0.0f );
			m_sliceCount = 0;
			m_updateCount = 0;
			ChangeStateDelayed( 1.0f, STATE_Chain4 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain4 )

		OnEnter
			DisableOnSliceUpdate();
			if( m_sliceCount > ((m_updateCount / 2)+1) ) {
				ChangeState( STATE_Broken );
			}
			else {
				ChangeState( STATE_Chain5 );
			}

		

	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain5 )

		OnEnter
			//Test if OnSlice gets called half as many times as OnUpdate
			MsgRouter.SetSlicePolicy( SLICE_POLICY_CONSTRAIN_BY_COUNT, 1 );	//Note: this is a global setting, so it's best to put this outside of state machines
			EnableOnSliceUpdate( 0.0f );
			m_sliceCount = 0;
			m_updateCount = 0;
			ChangeStateDelayed( 1.0f, STATE_Chain6 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain6 )

		OnEnter
			DisableOnSliceUpdate();
			if( m_sliceCount > ((m_updateCount / 2)+1) ) {
				ChangeState( STATE_Broken );
			}
			else {
				ChangeState( STATE_Chain7 );
			}


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain7 )

		OnEnter
            HGameObject hGO;
            GameObject* pGO = NULL;
            GOMan.Get( "UnitTest5a", &hGO );
            GOMan.GetGameObjectPointer( hGO, &pGO );
			//if( g_database.GetIDByName("UnitTest5a") == m_owner->GetID() ) {
            if (pGO && pGO->GetID() == m_owner->GetID()) {
				SendMsgDelayedToState( 1.0f, MSG_UnitTestPing );
                
                HGameObject hGO;
                GameObject* pGO = NULL;
                GOMan.Get( "UnitTest5b", &hGO );
                GOMan.GetGameObjectPointer( hGO, &pGO );
				SendMsgDelayed( 1.0f, MSG_UnitTestPing, pGO->GetID() );
			}
			else {
                HGameObject hGO;
                GameObject* pGO = NULL;
                GOMan.Get( "UnitTest5a", &hGO );
                GOMan.GetGameObjectPointer( hGO, &pGO );
                
				SendMsgDelayed( 1.0f, MSG_UnitTestPing, pGO->GetID() );
				SendMsgDelayedToState( 1.0f, MSG_UnitTestPing );
			}

		OnMsg( MSG_UnitTestPing )
			//Test if OnSlice gets called half as many times as OnUpdate
			MsgRouter.SetSlicePolicy( SLICE_POLICY_CONSTRAIN_BY_TIME, 0.0f );	//Note: this is a global setting, so it's best to put this outside of state machines
			ChangeState( STATE_Chain8 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain8 )

		OnEnter
			EnableOnSliceUpdate( 0.0f );
			m_sliceCount = 0;
			m_updateCount = 0;
			ChangeStateDelayed( 1.0f, STATE_Chain9 );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Chain9 )

		OnEnter
			DisableOnSliceUpdate();
			if( m_sliceCount > ((m_updateCount / 2)+1) ) {
				ChangeState( STATE_Broken );
			}
			else {
				ChangeState( STATE_Success );
			}


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Success )

		OnEnter
			RETAILMSG(ZONE_INFO, "UnitTest5 Success\n" );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Broken )

		OnEnter
			RETAILMSG(ZONE_INFO, "UnitTest5 Broken\n" );


EndStateMachine
}



} // END namespace Z


