/* Copyright Steve Rabin, 2007. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright Steve Rabin, 2007"
 */

#include "msgroute.hpp"
#include "StateMachine.hpp"
#include "GameObjectManager.hpp"



namespace Z 
{



#define MAX_STATE_STACK_SIZE 10


StateMachine::StateMachine( GameObject & object )
: m_queue( STATE_MACHINE_QUEUE_NULL ),
  m_owner( &object )
{
    // NOTE: does not take a reference to the GameObject.

	ASSERTMSG( m_owner->GetStateMachineManager(), "StateMachine::StateMachine - StateMachineManager not set yet in GameObject" );

	m_mgr = m_owner->GetStateMachineManager();
	Initialize();
}


StateMachine::StateMachine( HGameObject &hGameObject )
: m_queue( STATE_MACHINE_QUEUE_NULL ),
  m_owner( NULL ),
  m_hOwner(hGameObject)
{
    // NOTE: does not take a reference to the GameObject.

    RESULT result = GOMan.GetGameObjectPointer( hGameObject, &m_owner );
    ASSERTMSG( result == S_OK, "StateMachine::StateMachine - invalid HGameObject" );

	ASSERTMSG( m_owner->GetStateMachineManager(), "StateMachine::StateMachine - StateMachineManager not set yet in GameObject" );

	m_mgr = m_owner->GetStateMachineManager();
	Initialize();
}

/*---------------------------------------------------------------------------*
  Name:         Initialize

  Description:  Initializes the state machine.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::Initialize( void )
{
	m_scopeState = 0;
	m_scopeSubstate = 0;
	m_currentState = 0;
	m_currentSubstate = -1;
	m_stateChange = NO_STATE_CHANGE;
	m_nextState = 0;
	m_nextSubstate = 0;
	m_stateChangeAllowed = true;
	m_updateEventEnabled = true;
	m_timeOnEnter = 0.0f;
	m_ccMessagesToGameObject = 0;

	m_currentStateNameString[0] = 0;
	m_currentSubstateNameString[0] = 0;
  
	m_broadcastList.clear();
	m_stack.clear();
}

/*---------------------------------------------------------------------------*
  Name:         Reset

  Description:  Resets the state machine to the initial settings and default
                state. The state machine is also given the very first
				EVENT_Enter event.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::Reset( void )
{
	Initialize();
	Process( EVENT_Enter, 0 );
}

/*---------------------------------------------------------------------------*
  Name:         Update

  Description:  An update event is sent to the state machine.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::Update( void )
{
	if( m_updateEventEnabled )
	{
		Process( EVENT_FrameUpdate, 0 );
	}
}

/*---------------------------------------------------------------------------*
  Name:         Process

  Description:  Processes an event in the state machine. It is first sent
                to the current substate of the current state. If not handled,
				it is sent to the current state. If not handled, it is then 
				sent to the global state. Finally, any state changes are
				propagated.

  Arguments:    event : the event to process
                msg   : an optional msg to process with the event

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::Process( State_Machine_Event event, MSG_Object * msg )
{
	if( !m_owner->IsMarkedForDeletion() )
	{
		if( GetCCReceiver() > 0 && event == EVENT_Message && msg )
		{	//CC this message
			SendCCMsg( msg->GetName(), GetCCReceiver(), msg->GetMsgData() );
		}

		//Process this event inside the state machine
		bool handled = false;
		if( m_currentSubstate >= 0 )
		{	//Send to current substate
			handled = States( event, msg, m_currentState, m_currentSubstate );
		}
		if( !handled )
		{	//Send to current state
			handled = States( event, msg, m_currentState, -1 );
		}
		if( !handled )
		{	//Send to global state
			handled = States( event, msg, -1, -1 );
		}
		
		PerformStateChanges();
	}
}

/*---------------------------------------------------------------------------*
  Name:         PerformStateChanges

  Description:  Checks for a requested state change and executes it. This
                repeats until all requested state changes have completed. To
				avoid an infinite loop of state changes, it is stopped after
				a fixed number of times.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::PerformStateChanges( void )
{
	//Check for a state change
	int safetyCount = 20;
	while( m_stateChange != NO_STATE_CHANGE && (--safetyCount >= 0) )
	{
		ASSERTMSG( safetyCount > 0, "StateMachine::PerformStateChanges - States are flip-flopping in an infinite loop." );

		m_stateChangeAllowed = false;

		//Let the last state clean-up
		if( m_currentSubstate >= 0 )
		{	//Moving from a substate
			States( EVENT_Exit, 0, static_cast<int>( m_currentState ), m_currentSubstate );
		}
		if( m_nextSubstate < 0 )
		{	//Leaving current state
			States( EVENT_Exit, 0, static_cast<int>( m_currentState ), -1 );
		}
		

		//Perform state change
		switch( m_stateChange )
		{
			case STATE_CHANGE:
				if( m_nextSubstate < 0 )
				{	//This is a state change and not a substate change
					//Store the old state on the state stack
					m_stack.push_back( m_currentState );
					//Restrict stack size
					if( m_stack.size() > MAX_STATE_STACK_SIZE ) {
						m_stack.pop_front();
					}
				}
				//Set the new state
				m_currentState = m_nextState;
				m_currentSubstate = m_nextSubstate;
#ifdef DEBUG_STATE_MACHINE_MACROS
//				g_debuglog.LogStateMachineStateChange( m_owner->GetID(), m_owner->GetName(), m_currentState, m_currentSubstate );
#endif
				break;
				
			case STATE_POP:
				if( !m_stack.empty() ) {
					//Get last state off stack and pop it
					m_currentState = m_stack.back();
					m_currentSubstate = -1;
					m_stack.pop_back();
				}
				else {
					ASSERTMSG( 0, "StateMachine::PerformStateChanges - Hit bottom of state stack. Can't pop state." );
				}
#ifdef DEBUG_STATE_MACHINE_MACROS
//				g_debuglog.LogStateMachineStateChange( m_owner->GetID(), m_owner->GetName(), m_currentState, m_currentSubstate );
#endif
				break;
			
			default:
				ASSERTMSG( 0, "StateMachine::PerformStateChanges - Invalid state change." );
		}
				
		//Increment the scope (every state change gets a unique scope)
		m_scopeSubstate++;
		if( m_nextSubstate < 0 ) {
			m_scopeState++;
		}

		//Remember the time we entered this state
		m_timeOnEnter = GameTime.GetTimeDouble();

		//Let the new state initialize
		m_stateChange = NO_STATE_CHANGE;
		m_stateChangeAllowed = true;
		States( EVENT_Enter, 0, static_cast<int>( m_currentState ), m_currentSubstate );
	}

}

/*---------------------------------------------------------------------------*
  Name:         ChangeState

  Description:  Requests a state change. The state change will occur once the
                state machine is done executing all code in the current state
				or substate.

  Arguments:    newState : the new destination state

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::ChangeState( unsigned int newState )
{
	ASSERTMSG( m_stateChangeAllowed, "StateMachine::ChangeState - State change not allowed in OnExit." );
	ASSERTMSG( m_stateChange == NO_STATE_CHANGE, "StateMachine::ChangeState - State change already requested." );
	if( m_stateChangeAllowed ) {
		m_stateChange = STATE_CHANGE;
		m_nextState = newState;
		m_nextSubstate = -1;		//Next state begins in "no particular" substate
	}
}

/*---------------------------------------------------------------------------*
  Name:         ChangeSubstate

  Description:  Requests a substate change. The substate change will occur 
                once the state machine is done executing all code in the 
				current state or substate.

  Arguments:    newSubstate : the new destination substate

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::ChangeSubstate( unsigned int newSubstate )
{
	ASSERTMSG( m_stateChangeAllowed, "StateMachine::ChangeState - State change not allowed in OnExit." );
	ASSERTMSG( m_stateChange == NO_STATE_CHANGE, "StateMachine::ChangeState - State change already requested." );
	if( m_stateChangeAllowed ) {
		m_stateChange = STATE_CHANGE;
		m_nextState = m_currentState;
		m_nextSubstate = static_cast<int>( newSubstate );
	}

}

/*---------------------------------------------------------------------------*
  Name:         ChangeStateDelayed

  Description:  Requests a state change at a specified time in the future.
  
  Arguments:    delay    : the number of seconds in which to execute the state change
                newState : the new destination state

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::ChangeStateDelayed( float delay, unsigned int newState )
{
	ASSERTMSG( m_stateChangeAllowed, "StateMachine::ChangeStateDelayed - State change not allowed in OnExit." );
	if( m_stateChangeAllowed ) {
		SendMsgDelayedToSubstate( delay, MSG_CHANGE_STATE_DELAYED, newState );
	}
}

/*---------------------------------------------------------------------------*
  Name:         ChangeSubstateDelayed

  Description:  Requests a substate change at a specified time in the future.
  
  Arguments:    delay       : the number of seconds in which to execute the state change
                newSubstate : the new destination substate

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::ChangeSubstateDelayed( float delay, unsigned int newSubstate )
{
	ASSERTMSG( m_stateChangeAllowed, "StateMachine::ChangeSubstateDelayed - State change not allowed in OnExit." );
	if( m_stateChangeAllowed ) {
		SendMsgDelayedToSubstate( delay, MSG_CHANGE_SUBSTATE_DELAYED, newSubstate );
	}
}


void StateMachine::EnableOnSliceUpdate( float delay )
{ 
    MsgRouter.RegisterOnSliceEvent( delay, m_owner->GetID() ); 
}


void StateMachine::DisableOnSliceUpdate( void )
{ 
    MsgRouter.UnregisterOnSliceEvent( m_owner->GetID() ); 
}


/*---------------------------------------------------------------------------*
  Name:         PopState

  Description:  Requests to pop the current state. The state change will occur 
                once the state machine is done executing all code in the current 
				state or substate.
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::PopState( void )
{
	ASSERTMSG( m_stateChangeAllowed, "StateMachine::PopState - State change not allowed in OnExit." );
	ASSERTMSG( m_stateChange == NO_STATE_CHANGE, "StateMachine::PopState - State change already requested." );
	if( m_stateChangeAllowed ) {
		m_stateChange = STATE_POP;
	}
}

/*---------------------------------------------------------------------------*
  Name:         SendMsg

  Description:  Sends a message to another game object.
  
  Arguments:    name     : the name of the message
                receiver : the receiver of the message
				data     : associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsg( MSG_Name name, OBJECT_ID receiver, int data )
{
	ASSERTMSG( receiver != m_owner->GetID(), "StateMachine::SendMsg - Do not use to send messages to your own state machine. Use the other variations, such as SendMsgToState or SendMsgToStateMachine." );

	MSG_Data msgdata( data );
	MsgRouter.SendMsg( 0.0f, name, receiver, m_owner->GetID(), SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, msgdata, false, false );

}

/*---------------------------------------------------------------------------*
  Name:         SendMsgToSubstate

  Description:  Sends a message to the state machine next frame, as long as 
				the substate doesn't change (substate, state, and global 
				message responses can receive the message).
				Note: This send message automatically gets delayed for one frame 
				to avoid state change paradoxes.
  
  Arguments:    name : the name of the message
				data : (optional) associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgToSubstate( MSG_Name name, int data )
{
	SendMsgDelayedToMeHelper( ONE_FRAME, name, SCOPE_TO_SUBSTATE, m_queue, data, false );
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgToState

  Description:  Sends a message to the state machine next frame, as long as 
				the state doesn't change (substates can change and substates, 
				state, and global message responses can receive the message).
				Note: This send message automatically gets delayed for one frame 
				to avoid state change paradoxes.
  
  Arguments:    name : the name of the message
				data : (optional) associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgToState( MSG_Name name, int data )
{
	SendMsgDelayedToMeHelper( ONE_FRAME, name, SCOPE_TO_STATE, m_queue, data, false );
}



/*---------------------------------------------------------------------------*
  Name:         SendMsgToState

  Description:  Sends a message to the state machine next frame, as long as 
				the state doesn't change (substates can change and substates, 
				state, and global message responses can receive the message).
				Note: This send message automatically gets delayed for one frame 
				to avoid state change paradoxes.
  
  Arguments:    name : the name of the message
				data : (optional) associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgToState( MSG_Name name, void* pData )
{
    MSG_Data data;
    data.SetPointer( pData );

	SendMsgDelayedToMeHelper( ONE_FRAME, name, SCOPE_TO_STATE, m_queue, data, false );
}



/*---------------------------------------------------------------------------*
  Name:         SendMsgToStateMachine

  Description:  Sends a message to the state machine next frame, regardless 
				of state changes (substates, states, and global message 
				responses can receive the message).
				Note: This send message automatically gets delayed for one frame 
				to avoid state change paradoxes.
  
  Arguments:    name : the name of the message
				data : (optional) associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgToStateMachine( MSG_Name name, int data )
{
	SendMsgDelayedToMeHelper( ONE_FRAME, name, SCOPE_TO_STATE_MACHINE, m_queue, data, false );
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgToStateMachine

  Description:  Sends a message to the state machine next frame, regardless 
				of state changes (substates, states, and global message 
				responses can receive the message).
				Note: This send message automatically gets delayed for one frame 
				to avoid state change paradoxes.
  
  Arguments:    name : the name of the message
				data : (optional) associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgToStateMachine( MSG_Name name, void* pData )
{
    MSG_Data data;
    data.SetPointer( pData );

	SendMsgDelayedToMeHelper( ONE_FRAME, name, SCOPE_TO_STATE_MACHINE, m_queue, data, false );
}


/*---------------------------------------------------------------------------*
  Name:         SendMsgToStateMachineNow

  Description:  Sends a message to the state machine this frame
 				Note: Beware of state change paradoxes.
 
  Arguments:    name : the name of the message
				data : (optional) associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgToStateMachineNow( MSG_Name name, int data )
{
	SendMsgDelayedToMeHelper( 0.0f, name, SCOPE_TO_STATE_MACHINE, m_queue, data, false );
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgToSingleQueue

  Description:  Sends a message to the state machine in a 
				particular queue next frame.
  
  Arguments:    name  : the name of the message
				queue : the queue to send the message to
				data  : (optional) associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgToSingleQueue( MSG_Name name, StateMachineQueue queue, int data )
{
	ASSERTMSG( queue != m_queue, "StateMachine::SendMsgToSingleQueue - Use SendMsgToStateMachine instead" );
	ASSERTMSG( queue != STATE_MACHINE_QUEUE_ALL, "StateMachine::SendMsgToSingleQueue - Use SendMsgToAllQueues instead" );
	ASSERTMSG( queue < STATE_MACHINE_NUM_QUEUES, "StateMachine::SendMsgToSingleQueue - Argument queue out of bounds" );

	SendMsgDelayedToMeHelper( ONE_FRAME, name, SCOPE_TO_STATE_MACHINE, queue, data, false );
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgToSingleQueueNow

  Description:  Immediately sends a message to the state machine in a 
				particular queue.
  
  Arguments:    name  : the name of the message
				queue : the queue to send the message to
				data  : (optional) associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgToSingleQueueNow( MSG_Name name, StateMachineQueue queue, int data )
{
	ASSERTMSG( queue != m_queue, "StateMachine::SendMsgToSingleQueue - Use SendMsgToStateMachine instead" );
	ASSERTMSG( queue != STATE_MACHINE_QUEUE_ALL, "StateMachine::SendMsgToSingleQueue - Use SendMsgToAllQueues instead" );
	ASSERTMSG( queue < STATE_MACHINE_NUM_QUEUES, "StateMachine::SendMsgToSingleQueue - Argument queue out of bounds" );

	SendMsgDelayedToMeHelper( 0.0f, name, SCOPE_TO_STATE_MACHINE, queue, data, false );
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgToAllQueues

  Description:  Sends a message to all queues owned by the game 
				object (including this one) next frame.
  
  Arguments:    name  : the name of the message
				data  : (optional) associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgToAllQueues( MSG_Name name, int data )
{
	for( int queue=0; queue<STATE_MACHINE_NUM_QUEUES; ++queue )
	{
		SendMsgDelayedToMeHelper( ONE_FRAME, name, SCOPE_TO_STATE_MACHINE, (StateMachineQueue)queue, data, false );
	}
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgToAllQueuesNow

  Description:  Immediately sends a message to all queues owned by the game 
				object (including this one).
  
  Arguments:    name  : the name of the message
				data  : (optional) associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgToAllQueuesNow( MSG_Name name, int data )
{
	for( int queue=0; queue<STATE_MACHINE_NUM_QUEUES; ++queue )
	{
		SendMsgDelayedToMeHelper( 0.0f, name, SCOPE_TO_STATE_MACHINE, (StateMachineQueue)queue, data, false );
	}
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgToAllOtherQueues

  Description:  Sends a message to all queues owned by the game 
				object (except this one) next frame.
  
  Arguments:    name  : the name of the message
				data  : (optional) associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgToAllOtherQueues( MSG_Name name, int data )
{
	for( int queue=0; queue<STATE_MACHINE_NUM_QUEUES; ++queue )
	{
		if( queue != m_queue ) {
			SendMsgDelayedToMeHelper( ONE_FRAME, name, SCOPE_TO_STATE_MACHINE, (StateMachineQueue)queue, data, false );
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgToAllOtherQueuesNow

  Description:  Immediately sends a message to all queues owned by the game 
				object (except this one).
  
  Arguments:    name  : the name of the message
				data  : (optional) associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgToAllOtherQueuesNow( MSG_Name name, int data )
{
	for( int queue=0; queue<STATE_MACHINE_NUM_QUEUES; ++queue )
	{
		if( queue != m_queue ) {
			SendMsgDelayedToMeHelper( 0.0f, name, SCOPE_TO_STATE_MACHINE, (StateMachineQueue)queue, data, false );
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgDelayed

  Description:  Sends a message at a specified time in the future.
  
  Arguments:    delay    : the number of seconds in the future to deliver the message
                name     : the name of the message
                receiver : the receiver of the message
				data     : associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgDelayed( float delay, MSG_Name name, OBJECT_ID receiver, int data )
{
	ASSERTMSG( delay > 0.0f, "StateMachine::SendMsgDelayed - Argument delay must be > 0.0f, or else consider using SendMsg" );
	ASSERTMSG( receiver != m_owner->GetID(), "StateMachine::SendMsgDelayed - Do not use to send messages to your own state machine. Use the other variations, such as SendMsgDelayedToState or SendMsgDelayedToStateMachine." );

	MSG_Data msgdata( data );
	MsgRouter.SendMsg( delay, name, receiver, m_owner->GetID(), SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, msgdata, false, false );
	
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgDelayedToSubstate

  Description:  Sends a delayed message to the state machine as long as the 
				substate doesn't change (substate, state, and global message 
				responses can receive the message).
  
  Arguments:    delay : the number of seconds in the future to deliver the message
                name  : the name of the message
				data  : associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgDelayedToSubstate( float delay, MSG_Name name, int data )
{
	ASSERTMSG( delay > 0.0f, "delay must be > 0.0f, or else consider using SendMsgToSubstate" );

	SendMsgDelayedToMeHelper( delay, name, SCOPE_TO_SUBSTATE, m_queue, data, false );
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgDelayedToState

  Description:  Sends a delayed message to the state machine as long as the 
				state doesn't change (substates can change and substates, 
				state, and global message responses can receive the message).
  
  Arguments:    delay : the number of seconds in the future to deliver the message
                name  : the name of the message
				data  : associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgDelayedToState( float delay, MSG_Name name, int data )
{
	ASSERTMSG( delay > 0.0f, "delay must be > 0.0f, or else consider using SendMsgToState" );

	SendMsgDelayedToMeHelper( delay, name, SCOPE_TO_STATE, m_queue, data, false );
}


/*---------------------------------------------------------------------------*
  Name:         SendMsgDelayedToState

  Description:  Sends a delayed message to the state machine as long as the 
				state doesn't change (substates can change and substates, 
				state, and global message responses can receive the message).
  
  Arguments:    delay : the number of seconds in the future to deliver the message
                name  : the name of the message
				data  : associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgDelayedToState( float delay, MSG_Name name, void* pData )
{
	ASSERTMSG( delay > 0.0f, "delay must be > 0.0f, or else consider using SendMsgToState" );

    MSG_Data data;
    data.SetPointer( pData );

	SendMsgDelayedToMeHelper( delay, name, SCOPE_TO_STATE, m_queue, data, false );
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgDelayedToStateMachine

  Description:  Sends a delayed message to the state machine regardless of 
				substate or state changes (substates, states, and global 
				message responses can receive the message).
  
  Arguments:    delay : the number of seconds in the future to deliver the message
                name  : the name of the message
				data  : associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgDelayedToStateMachine( float delay, MSG_Name name, int data )
{
	ASSERTMSG( delay > 0.0f, "delay must be > 0.0f, or else consider using SendMsgToStateMachine" );

	SendMsgDelayedToMeHelper( delay, name, SCOPE_TO_STATE_MACHINE, m_queue, data, false );
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgBroadcast

  Description:  Preferred way to broadcast a message to many objects at once.
  
  Arguments:    name : the name of the message
                type : the type of object to broadcast the message to
                zone : the spatial zone to broadcast to
				data : associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgBroadcast( MSG_Name name, GO_TYPE type, int data )
{
	MSG_Object msg( 0.0f, name, m_owner->GetID(), 0, SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, data, false, false );
	MsgRouter.SendMsgBroadcast( msg, type );
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgBroadcastToList

  Description:  Broadcast a message to a pre-composed list of objects.
  
  Arguments:    name : the name of the message
				data : associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgBroadcastToList( MSG_Name name, int data )
{
	ASSERTMSG( m_broadcastList.size() > 0, "StateMachine::SendMsgBroadcastToList - No objects in broadcast list." );
	MSG_Data msgdata( data );

	BroadcastListContainer::iterator i;
	for( i=m_broadcastList.begin(); i!=m_broadcastList.end(); ++i )
	{
		OBJECT_ID id = (OBJECT_ID)(*i);
		if( id != m_owner->GetID() ) {
			MsgRouter.SendMsg( 0, name, id, m_owner->GetID(), SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, msgdata, false, false );
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         BroadcastClearList

  Description:  Clear all objects from the broadcast list.
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::BroadcastClearList( void )
{
	m_broadcastList.clear();
}

/*---------------------------------------------------------------------------*
  Name:         BroadcastAddToList

  Description:  Add an object to the broadcast list.
  
  Arguments:    id : an ID of an object to add to the list

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::BroadcastAddToList( OBJECT_ID id )
{
	m_broadcastList.push_back( id );
}

/*---------------------------------------------------------------------------*
  Name:         SetTimerSubstate

  Description:  Set a timer to deliver a message periodically to yourself.
				Only valid if substate doesn't change.
  
  Arguments:    delay : the number of seconds in the future to deliver the message
                name  : the name of the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SetTimerSubstate( float delay, MSG_Name name )
{
	//pass in delay as data, so timer can be resent when received
	if( delay < 0.001f )
	{	//Enforce minimum to avoid ugly bugs (effectively the next frame)
		delay = 0.001f;
	}

	MSG_Data data( delay );
	SendMsgDelayedToMeHelper( delay, name, SCOPE_TO_SUBSTATE, m_queue, data, true );
}

/*---------------------------------------------------------------------------*
  Name:         SetTimerState

  Description:  Set a timer to deliver a message periodically to yourself.
				Only valid if state doesn't change.
  
  Arguments:    delay : the number of seconds in the future to deliver the message
                name  : the name of the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SetTimerState( float delay, MSG_Name name )
{
	//pass in delay as data, so timer can be resent when received
	if( delay < 0.001f )
	{	//Enforce minimum to avoid ugly bugs (effectively the next frame)
		delay = 0.001f;
	}

	MSG_Data data( delay );
	SendMsgDelayedToMeHelper( delay, name, SCOPE_TO_SUBSTATE, m_queue, data, true );
}

/*---------------------------------------------------------------------------*
  Name:         SetTimerStateMachine

  Description:  Set a timer to deliver a message periodically to yourself.
				Valid regardless of substate or state changes.
  
  Arguments:    delay : the number of seconds in the future to deliver the message
                name  : the name of the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SetTimerStateMachine( float delay, MSG_Name name )
{
	//pass in delay as data, so timer can be resent when received
	if( delay < 0.001f )
	{	//Enforce minimum to avoid ugly bugs (effectively the next frame)
		delay = 0.001f;
	}

	MSG_Data data( delay );
	SendMsgDelayedToMeHelper( delay, name, SCOPE_TO_STATE_MACHINE, m_queue, data, true );
}

/*---------------------------------------------------------------------------*
  Name:         SetTimerExternal

  Description:  Used by msgroute to reset timer after it's been sent.
  
  Arguments:    delay : the number of seconds in the future to deliver the message
                name  : the name of the message
				rule  : the scoping rule for the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SetTimerExternal( float delay, MSG_Name name, Scope_Rule rule )
{
	MSG_Data data( delay );
	SendMsgDelayedToMeHelper( delay, name, rule, m_queue, data, true );
}

/*---------------------------------------------------------------------------*
  Name:         StopTimer

  Description:  Stop a timer based on the message name.
  
  Arguments:    name  : the name of the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::StopTimer( MSG_Name name )
{
	MsgRouter.RemoveMsg( name, m_owner->GetID(), m_owner->GetID(), true );
}

/*---------------------------------------------------------------------------*
  Name:         SendCCMsg

  Description:  Send a CCd message to the receiver. This is like a regular
                message, but the CC flag is on.
  
  Arguments:    name     : the name of the message
                receiver : the receiver of the message
				data     : associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendCCMsg( MSG_Name name, OBJECT_ID receiver, MSG_Data data )
{
	MsgRouter.SendMsg( 0, name, receiver, m_owner->GetID(), SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, data, false, true );
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgDelayedToMeHelper

  Description:  Helper function for several other functions to send a message
                to yourself.
  
  Arguments:    delay : the number of seconds in the future to deliver the message
                name  : the name of the message
                rule  : the scoping rule for the message
				queue : the queue to send it to
				data  : associated data to deliver with the message
				timer : whether this is a periodic timer message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgDelayedToMeHelper( float delay, MSG_Name name, Scope_Rule rule, StateMachineQueue queue, MSG_Data data, bool timer )
{
	unsigned int scope = 0;
	
	if( rule == SCOPE_TO_SUBSTATE ) {	
		scope = m_scopeSubstate;
	}
	else if( rule == SCOPE_TO_STATE ) {
		scope = m_scopeState;
	}

	MsgRouter.SendMsg( delay, name, m_owner->GetID(), m_owner->GetID(), rule, scope, queue, data, timer, false );
}

/*---------------------------------------------------------------------------*
  Name:         RandDelay

  Description:  Returns a random delay within the range [min,max].
  
  Arguments:    min : the lower bound
                max : the higher bound

  Returns:      random delay in range
 *---------------------------------------------------------------------------*/
float StateMachine::RandDelay( float min, float max )
{
	ASSERTMSG( min >= 0.0, "RandDelay - min must be greater than or equal to zero" );
	ASSERTMSG( min <= max, "RandDelay - min must be less than or equal to max" );

	float value = ((float)rand()) / ((float)RAND_MAX);
	value *= max - min;		//Multiply by the range
	value += min;			//Move value up to min

	return( value );
}

/*---------------------------------------------------------------------------*
  Name:         GetNumStateMachinesInQueue

  Description:  Gets the number of state machines in the queue.
  
  Arguments:    None.

  Returns:      Number of state machines in the game object's queue
 *---------------------------------------------------------------------------*/
int StateMachine::GetNumStateMachinesInQueue( void )
{
	return( m_mgr->GetNumStateMachinesInQueue( m_queue ) );

}

/*---------------------------------------------------------------------------*
  Name:         ResetStateMachine

  Description:  Requests that the state machine is reset. The reset will occur
                at the next update cycle.
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::ResetStateMachine( void )
{
	m_mgr->RequestStateMachineChange( 0, STATE_MACHINE_RESET, m_queue );
}

/*---------------------------------------------------------------------------*
  Name:         ReplaceStateMachine

  Description:  Requests that the state machine be replaced. The replacement
                will occur at the next update cycle.
  
  Arguments:    mch : the new state machine

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::ReplaceStateMachine( StateMachine & mch )
{
	m_mgr->RequestStateMachineChange( &mch, STATE_MACHINE_REPLACE, m_queue );
}

/*---------------------------------------------------------------------------*
  Name:         QueueStateMachine

  Description:  Requests that a new state machine be queued. This will occur 
                at the next update cycle.
  
  Arguments:    mch : the new state machine

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::QueueStateMachine( StateMachine & mch )
{
	m_mgr->RequestStateMachineChange( &mch, STATE_MACHINE_QUEUE, m_queue );
}

/*---------------------------------------------------------------------------*
  Name:         RequeueStateMachine

  Description:  Requests that the current state machine be requeued. This will
                occur at the next update cycle.
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::RequeueStateMachine( void )
{
	m_mgr->RequestStateMachineChange( 0, STATE_MACHINE_REQUEUE, m_queue );
}

/*---------------------------------------------------------------------------*
  Name:         PushStateMachine

  Description:  Requests that a new state machine be pushed to the top of the
                list and become the current state machine. This will
                occur at the next update cycle.
  
  Arguments:    mch : the new state machine

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::PushStateMachine( StateMachine & mch )
{
	m_mgr->RequestStateMachineChange( &mch, STATE_MACHINE_PUSH, m_queue );
}

/*---------------------------------------------------------------------------*
  Name:         PopStateMachine

  Description:  Requests that the current state machine be popped from the list.
                This will occur at the next update cycle.
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::PopStateMachine( void )
{
	m_mgr->RequestStateMachineChange( 0, STATE_MACHINE_POP, m_queue );
}





StateMachineManager::StateMachineManager( GameObject & object )
: m_owner( &object )
{
	for( int i=0; i<STATE_MACHINE_NUM_QUEUES; ++i )
	{
		m_stateMachineChange[i] = NO_STATE_MACHINE_CHANGE;
		m_newStateMachine[i] = 0;
	}
}

StateMachineManager::~StateMachineManager( void )
{
	DestroyStateMachineList( STATE_MACHINE_QUEUE_ALL );
}

/*---------------------------------------------------------------------------*
  Name:         Update

  Description:  Updates the currently active state machine in each queue.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachineManager::Update( void )
{
	for( int queue=0; queue<STATE_MACHINE_NUM_QUEUES; ++queue )
	{
		if( !m_stateMachineList[queue].empty() )
		{
			ProcessStateMachineChangeRequests((StateMachineQueue)queue);
			m_stateMachineList[queue].back()->Update();
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         SendMsg

  Description:  Sends a message to the currently active state machine in each queue

  Arguments:    MSG_Object

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachineManager::SendMsg( MSG_Object msg )
{
	for( int queue=0; queue<STATE_MACHINE_NUM_QUEUES; ++queue )
	{
		if( !m_stateMachineList[queue].empty() ) {
			m_stateMachineList[queue].back()->Process( EVENT_Message, &msg );
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         Process

  Description:  Processes an event in the state machine.

  Arguments:    event : the event to process
                msg   : an optional msg to process with the event

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachineManager::Process( State_Machine_Event event, MSG_Object * msg, StateMachineQueue queue )
{
	if( queue < STATE_MACHINE_NUM_QUEUES )
	{
		if( !m_stateMachineList[queue].empty() ) {
			m_stateMachineList[queue].back()->Process( event, msg );
		}
	}
	else if( queue == STATE_MACHINE_QUEUE_ALL )
	{
		for( int i=0; i<STATE_MACHINE_NUM_QUEUES; ++i )
		{
			if( !m_stateMachineList[i].empty() ) {
				m_stateMachineList[i].back()->Process( event, msg );
			}
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         ProcessStateMachineChangeRequests

  Description:  Checks if a state machine should be changed. It will loop
                until no more state machine change requests have been made.

  Arguments:    queue : the queue to operate on

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachineManager::ProcessStateMachineChangeRequests( StateMachineQueue queue )
{
	int safetyCount = 20;
	StateMachineChange change = m_stateMachineChange[queue];
	StateMachine * tempStateMachine = m_newStateMachine[queue];
	
	while( change != NO_STATE_MACHINE_CHANGE && (--safetyCount >= 0) )
	{
		ASSERTMSG( safetyCount > 0, "StateMachineManager::ProcessStateMachineChangeRequests - State machiens are flip-flopping in an infinite loop." );

		m_stateMachineChange[queue] = NO_STATE_MACHINE_CHANGE;			//Reset
		m_newStateMachine[queue] = 0;									//Reset

		switch( change )
		{
			case STATE_MACHINE_RESET:
				MsgRouter.PurgeScopedMsg( m_owner->GetID() ); //Remove all delayed messages addressed to me that are scoped
				ResetStateMachine(queue);
				break;
				
			case STATE_MACHINE_REPLACE:
				MsgRouter.PurgeScopedMsg( m_owner->GetID() ); //Remove all delayed messages addressed to me that are scoped
				ReplaceStateMachine( *tempStateMachine, queue );
				break;
				
			case STATE_MACHINE_QUEUE:
				QueueStateMachine( *tempStateMachine, queue );
				break;
				
			case STATE_MACHINE_REQUEUE:
				MsgRouter.PurgeScopedMsg( m_owner->GetID() ); //Remove all delayed messages addressed to me that are scoped
				RequeueStateMachine( queue );
				break;
				
			case STATE_MACHINE_PUSH:
				MsgRouter.PurgeScopedMsg( m_owner->GetID() ); //Remove all delayed messages addressed to me that are scoped
				PushStateMachine( *tempStateMachine, queue, true );
				break;
				
			case STATE_MACHINE_POP:
				MsgRouter.PurgeScopedMsg( m_owner->GetID() ); //Remove all delayed messages addressed to me that are scoped
				PopStateMachine( queue );
				break;
				
			default:
				ASSERTMSG( 0, "GameObject::ProcessStateMachineChangeRequests - invalid StateMachineChange request." );
		}
				
		//Check if new change
		change = m_stateMachineChange[queue];
		tempStateMachine = m_newStateMachine[queue];
	}
}

/*---------------------------------------------------------------------------*
  Name:         RequestStateMachineChange

  Description:  Requests that a state machine change take place.

  Arguments:    mch    : the new state machine
                change : the change to take place
				queue  : the queue to operate on

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachineManager::RequestStateMachineChange( StateMachine * mch, StateMachineChange change, StateMachineQueue queue )
{
	ASSERTMSG( m_stateMachineChange[queue] == NO_STATE_MACHINE_CHANGE, "StateMachineManager::RequestStateMachineChange - Change already requested." );

	m_newStateMachine[queue] = mch;
	m_stateMachineChange[queue] = change;
}

/*---------------------------------------------------------------------------*
  Name:         ResetStateMachine

  Description:  Resets the current state machine.

  Arguments:    queue : the queue to operate on

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachineManager::ResetStateMachine( StateMachineQueue queue )
{
	ASSERTMSG( queue < STATE_MACHINE_NUM_QUEUES, "StateMachineManager::ResetStateMachine - queue out of bounds" );
	ASSERTMSG( m_stateMachineList[queue].size() > 0, "StateMachineManager::ResetStateMachine - No existing state machine to reset." );

	if( m_stateMachineList[queue].size() > 0 ) {
		StateMachine * mch = m_stateMachineList[queue].back();
		mch->Reset();
	}
}

/*---------------------------------------------------------------------------*
  Name:         ReplaceStateMachine

  Description:  Replaces the current state machine with the provided one
                by popping off the current state machine and pushing the 
				new one.

  Arguments:    mch   : the new state machine
				queue : the queue to operate on

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachineManager::ReplaceStateMachine( StateMachine & mch, StateMachineQueue queue )
{
	ASSERTMSG( queue < STATE_MACHINE_NUM_QUEUES, "StateMachineManager::ReplaceStateMachine - queue out of bounds" );
	ASSERTMSG( m_stateMachineList[queue].size() > 0, "StateMachineManager::ReplaceStateMachine - No existing state machine to replace." );

	mch.SetStateMachineQueue( queue );

	if( m_stateMachineList[queue].size() > 0 ) {
		StateMachine * temp = m_stateMachineList[queue].back();
		m_stateMachineList[queue].pop_back();
        if ( temp )
            delete( temp );
	}
	PushStateMachine( mch, queue, true );
}

/*---------------------------------------------------------------------------*
  Name:         QueueStateMachine

  Description:  Queues a state machine behind all others, except for the
                very last one. The last state machine is the "default" and
				should always remain the last.

  Arguments:    mch   : the new state machine
                queue : the queue to operate on

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachineManager::QueueStateMachine( StateMachine & mch, StateMachineQueue queue )
{
	ASSERTMSG( queue < STATE_MACHINE_NUM_QUEUES, "StateMachineManager::QueueStateMachine - queue out of bounds" );

	mch.SetStateMachineQueue( queue );

	//Insert state machine one up from bottom
	if( m_stateMachineList[queue].size() <= 1 ) {
		PushStateMachine( mch, queue, false );
	}
	else {
		stateMachineListContainer::iterator i;
		i = m_stateMachineList[queue].begin();
		i++;	//Move iterator past the first entry
		m_stateMachineList[queue].insert(i, &mch);
		//Purposely do not "Reset" state machine until it is active
	}
}

/*---------------------------------------------------------------------------*
  Name:         RequeueStateMachine

  Description:  Requeues the current state machine behind all others, except
                for the very last one.

  Arguments:    queue      : the queue to operate on

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachineManager::RequeueStateMachine( StateMachineQueue queue )
{
	ASSERTMSG( queue < STATE_MACHINE_NUM_QUEUES, "StateMachineManager::RequeueStateMachine - queue out of bounds" );
	ASSERTMSG( m_stateMachineList[queue].size() > 0, "StateMachineManager::RequeueStateMachine - No existing state machines to requeue." );

	if( m_stateMachineList[queue].size() > 1 ) {
		StateMachine * mch = m_stateMachineList[queue].back();
		QueueStateMachine( *mch, queue );
		m_stateMachineList[queue].pop_back();

		//Initialize new state machine
		mch = m_stateMachineList[queue].back();
		mch->Reset();
	}
	else if( m_stateMachineList[queue].size() == 1 ) {
		//Just reinitialize
		StateMachine * mch = m_stateMachineList[queue].back();
		mch->Reset();	
	}
}

/*---------------------------------------------------------------------------*
  Name:         PushStateMachine

  Description:  Pushes a state machine onto the front of the state machine list.

  Arguments:    mch        : the new state machine
				queue      : the queue to operate on
				initialize : whether to send the initial OnEnter event

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachineManager::PushStateMachine( StateMachine & mch, StateMachineQueue queue, bool initialize )
{
	ASSERTMSG( queue < STATE_MACHINE_NUM_QUEUES, "StateMachineManager::PushStateMachine - queue out of bounds" );

	mch.SetStateMachineQueue( queue );
	m_stateMachineList[queue].push_back( &mch );
	
	if( initialize )
	{
		mch.Reset();
	}
}

/*---------------------------------------------------------------------------*
  Name:         PopStateMachine

  Description:  Pops a state machine from the front of the state machine list.

  Arguments:    queue : the queue to operate on

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachineManager::PopStateMachine( StateMachineQueue queue )
{
	ASSERTMSG( queue < STATE_MACHINE_NUM_QUEUES, "StateMachineManager::PopStateMachine - queue out of bounds" );
	ASSERTMSG( m_stateMachineList[queue].size() > 1, "StateMachineManager::PopStateMachine - Can't pop last state machine." );

	if( m_stateMachineList[queue].size() > 1 ) {
		StateMachine * mch = m_stateMachineList[queue].back();
		m_stateMachineList[queue].pop_back();
        if ( mch )
            delete( mch );
		
		//Initialize new state machine
		mch = m_stateMachineList[queue].back();
		mch->Reset();
	}
}

/*---------------------------------------------------------------------------*
  Name:         DestroyStateMachineList

  Description:  Destroys all state machines in the state machine list.

  Arguments:    queue : the queue(s) to destroy

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachineManager::DestroyStateMachineList( StateMachineQueue queue )
{
	if( queue == STATE_MACHINE_QUEUE_ALL )
	{
		for( int i=0; i<STATE_MACHINE_NUM_QUEUES; ++i )
		{
			while( m_stateMachineList[i].size() > 0 ) {
				StateMachine * mch = m_stateMachineList[i].back();
				m_stateMachineList[i].pop_back();
                if ( mch )
                    delete( mch );
			}
		}
	}
	else if( queue < STATE_MACHINE_NUM_QUEUES )
	{
		while( m_stateMachineList[queue].size() > 0 ) {
			StateMachine * mch = m_stateMachineList[queue].back();
			m_stateMachineList[queue].pop_back();
            if ( mch )
                delete( mch );
		}
	}
}
    


} // END namespace Z


