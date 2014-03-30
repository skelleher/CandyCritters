/* Copyright Steve Rabin, 2007. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright Steve Rabin, 2007"
 */

#pragma once
#pragma warning(disable: 4996)
#pragma warning(disable: 4995)

#include <vector>
#include "msg.hpp"
#include "Object.hpp"
#include "GameObject.hpp"
#include "Log.hpp"

using std::vector;


namespace Z
{



#define MAX_STATE_NAME_SIZE (64)
//#define DEBUG_STATE_MACHINE_MACROS		//Comment out to get the release macros (no debug logging info)
#define ONE_FRAME (0.0001f)

//State Machine Language Macros (put the 9 keywords in the file USERTYPE.DAT in the same directory as MSDEV.EXE to get keyword highlighting)
#ifdef DEBUG_STATE_MACHINE_MACROS
	#define BeginStateMachine				StateName laststatedeclared; char eventbuffer[5]; if( state < 0 ) { const char statename[MAX_STATE_NAME_SIZE] = "STATE_Global"; const char substatename[MAX_STATE_NAME_SIZE] = ""; if( EVENT_Message == event && msg && MSG_CHANGE_STATE_DELAYED == msg->GetName() ) { ChangeState( static_cast<unsigned int>( msg->GetIntData() ) ); return( true ); } if( EVENT_Message == event && msg && MSG_CHANGE_SUBSTATE_DELAYED == msg->GetName() ) { ChangeSubstate( static_cast<unsigned int>( msg->GetIntData() ) );
	#define EndStateMachine					return( true ); } Log::LogStateMachineEvent( m_owner->GetID(), m_owner->GetName(), msg, statename, substatename, itoa(event, eventbuffer, 10), false ); return( false ); } ASSERTMSG( 0, "Invalid State" ); return( false );
	#define DeclareState(name)				return( true ); } Log::LogStateMachineEvent( m_owner->GetID(), m_owner->GetName(), msg, statename, substatename, itoa(event, eventbuffer, 10), false ); return( false ); } laststatedeclared = name; int name ## __DUPLICATE_DeclareState_CHECK = 0; if( name == state && substate < 0 ) { const char statename[MAX_STATE_NAME_SIZE] = #name; const char substatename[MAX_STATE_NAME_SIZE] = ""; if( EVENT_Enter == event ) { SetCurrentStateName( #name ); } if(0) { 
	#define DeclareSubstate(name)			return( true ); } return( false ); } else if( laststatedeclared == state && name == substate ) { const char statename[MAX_STATE_NAME_SIZE] = ""; const char substatename[MAX_STATE_NAME_SIZE] = #name; if( EVENT_Enter == event ) { SetCurrentSubstateName( #name ); } SubstateName verifysubstatename = name; if(0) { 
	#define OnMsg(msgname)					return( true ); } else if( EVENT_Message == event && msg && msgname == msg->GetName() ) { VerifyMessageEnum( msgname ); Log::LogStateMachineEvent( m_owner->GetID(), m_owner->GetName(), msg, statename, substatename, #msgname, true );
	#define OnCCMsg(msgname)				return( true ); } else if( EVENT_CCMessage == event && msg && msgname == msg->GetName() ) { Log::LogStateMachineEvent( m_owner->GetID(), m_owner->GetName(), msg, statename, substatename, #msgname, true );
	#define ONEVENT_INTERNAL_HELPER(a)		return( true ); } else if( a == event ) { Log::LogStateMachineEvent( m_owner->GetID(), m_owner->GetName(), msg, statename, substatename, #a, true );
	#define OnFrameUpdate					ONEVENT_INTERNAL_HELPER( EVENT_FrameUpdate )
	#define OnSliceUpdate					ONEVENT_INTERNAL_HELPER( EVENT_SliceUpdate )
	#define OnEnter							ONEVENT_INTERNAL_HELPER( EVENT_Enter )
	#define OnExit							ONEVENT_INTERNAL_HELPER( EVENT_Exit )
#else
	#define BeginStateMachine				StateName laststatedeclared; if( state < 0 ) { if( EVENT_Message == event && msg && MSG_CHANGE_STATE_DELAYED == msg->GetName() ) { ChangeState( static_cast<unsigned int>( msg->GetIntData() ) ); return( true ); } if( EVENT_Message == event && msg && MSG_CHANGE_SUBSTATE_DELAYED == msg->GetName() ) { ChangeSubstate( static_cast<unsigned int>( msg->GetIntData() ) );
	#define EndStateMachine					return( true ); } return( false ); } ASSERTMSG( 0, "Invalid State" ); return( false );
	#define DeclareState(name)				return( true ); } return( false ); } laststatedeclared = name; if( name == state && substate < 0 ) { if(0) { 
	#define DeclareSubstate(name)			return( true ); } return( false ); } else if( laststatedeclared == state && name == substate ) { if(0) { 
	#define OnMsg(msgname)					return( true ); } else if( EVENT_Message == event && msg && msgname == msg->GetName() ) {
	#define OnCCMsg(msgname)				return( true ); } else if( EVENT_CCMessage == event && msg && msgname == msg->GetName() ) {
	#define ONEVENT_INTERNAL_HELPER(a)		return( true ); } else if( a == event ) {
	#define OnFrameUpdate					ONEVENT_INTERNAL_HELPER( EVENT_FrameUpdate )
	#define OnSliceUpdate					ONEVENT_INTERNAL_HELPER( EVENT_SliceUpdate )
	#define OnEnter							ONEVENT_INTERNAL_HELPER( EVENT_Enter )
	#define OnExit							ONEVENT_INTERNAL_HELPER( EVENT_Exit )
#endif

//Helpers to set breakpoints for particular game objects
#define WATCHPOINT_ID(id)					if( m_owner->GetID() == id ) { __debugbreak(); }
#define WATCHPOINT_NAME(name)				if( strcmp(name, m_owner->GetName() ) == 0 ) { __debugbreak(); }

enum State_Machine_Event {
	EVENT_INVALID,
	EVENT_FrameUpdate,
	EVENT_SliceUpdate,
	EVENT_Message,
	EVENT_CCMessage,
	EVENT_Enter,
	EVENT_Exit
};

enum StateMachineQueue {
	STATE_MACHINE_QUEUE_0,
	STATE_MACHINE_QUEUE_1,
	STATE_MACHINE_QUEUE_2,
	STATE_MACHINE_QUEUE_3,
	STATE_MACHINE_NUM_QUEUES,
	STATE_MACHINE_QUEUE_ALL,
	STATE_MACHINE_QUEUE_NULL
};

enum StateMachineChange {
	NO_STATE_MACHINE_CHANGE,
	STATE_MACHINE_RESET,
	STATE_MACHINE_REPLACE,
	STATE_MACHINE_QUEUE,
	STATE_MACHINE_REQUEUE,
	STATE_MACHINE_PUSH,
	STATE_MACHINE_POP
};



} // END namespace Z


// Need to include this down here
// because GCC doesn't allow forward declaration of enums like StateMachineQueue.
#include "msgroute.hpp"


namespace Z
{



//Forward declarations
class StateMachineManager;
class GameObject;


#pragma mark StateMachine

class StateMachine : virtual public Object
{
public:

    StateMachine( GameObject & object );    // This is only for the unit tests to build; too lazy to rewrite them all to use handles.
    
	StateMachine( HGameObject & hGameObject );
	virtual ~StateMachine( void ) {}

	void SetStateMachineQueue( StateMachineQueue queue )	{ ASSERTMSG( queue < STATE_MACHINE_NUM_QUEUES, "StateMachine::SetQueue - invalid queue" ); m_queue = queue; }

	//Should only be called by GameObject
	void Update( void );
	void Reset( void );

	//Only to be used by msgroute!
	void SetTimerExternal( float delay, MSG_Name name, Scope_Rule rule );

	//Access state and scope
	inline int GetState( void )							{ return( (int)m_currentState ); }
	inline int GetSubstate( void )						{ return( m_currentSubstate ); }
	inline unsigned int GetScopeState( void )			{ return( m_scopeState ); }
	inline unsigned int GetScopeSubstate( void )		{ return( m_scopeSubstate ); }
	
	//Main state machine code stored in here
	void Process( State_Machine_Event event, MSG_Object * msg );

	//Debug info
	inline char * GetCurrentStateNameString( void )		{ return( m_currentStateNameString ); }
	inline char * GetCurrentSubstateNameString( void )	{ return( m_currentSubstateNameString ); }


protected:

    HGameObject  m_hOwner;
	GameObject * m_owner;				//GameObject that owns this state machine
	StateMachineManager * m_mgr;		//StateMachineManager that owns this state machine
	StateMachineQueue m_queue;			//The queue this state machine is on

	//Send Messages
	void SendMsg( MSG_Name name, OBJECT_ID receiver, int data = 0 );
	void SendMsgToState( MSG_Name name, int data = 0 );
	void SendMsgToSubstate( MSG_Name name, int data = 0 );
	void SendMsgToStateMachine( MSG_Name name, int data = 0 );
    void SendMsgToStateMachine( MSG_Name name, void* pData );
	void SendMsgToStateMachineNow( MSG_Name name, int data = 0 );
	void SendMsgToSingleQueue( MSG_Name name, StateMachineQueue queue, int data = 0 );
	void SendMsgToSingleQueueNow( MSG_Name name, StateMachineQueue queue, int data = 0 );
	void SendMsgToAllQueues( MSG_Name name, int data = 0 );
	void SendMsgToAllQueuesNow( MSG_Name name, int data = 0 );
	void SendMsgToAllOtherQueues( MSG_Name name, int data = 0 );
	void SendMsgToAllOtherQueuesNow( MSG_Name name, int data = 0 );

	//Send Messages Delayed
	void SendMsgDelayed( float delay, MSG_Name name, OBJECT_ID receiver, int data = 0 );
	void SendMsgDelayedToSubstate( float delay, MSG_Name name, int data = 0 );
	void SendMsgDelayedToState( float delay, MSG_Name name, int data = 0 );
	void SendMsgDelayedToStateMachine( float delay, MSG_Name name, int data = 0 );
	void SendMsgDelayedToSingleQueue( float delay, MSG_Name name, int data = 0 );
	void SendMsgDelayedToAllQueues( float delay, MSG_Name name, int data = 0 );
	void SendMsgDelayedToAllOtherQueues( float delay, MSG_Name name, int data = 0 );

	void SendMsgDelayedToState( float delay, MSG_Name name, void* pData );
    void SendMsgToState( MSG_Name name, void* pData );


	//Send Messages Broadcast
	void SendMsgBroadcast( MSG_Name name, GO_TYPE = GO_TYPE_ANY, int data = 0 );
	void SendMsgBroadcastToList( MSG_Name name, int data = 0 );

	//Broadcast List
	void BroadcastClearList( void );
	void BroadcastAddToList( OBJECT_ID id );

	//CCing other objects
	inline void SetCCReceiver( OBJECT_ID id )		{ m_ccMessagesToGameObject = id; }
	inline void ClearCCReceiver( void )				{ m_ccMessagesToGameObject = 0; }
	inline OBJECT_ID GetCCReceiver( void )			{ return( m_ccMessagesToGameObject ); }
	
	//Message Timers
	void SetTimerSubstate( float delay, MSG_Name name );
	void SetTimerState( float delay, MSG_Name name );
	void SetTimerStateMachine( float delay, MSG_Name name );
	void StopTimer( MSG_Name name );

	//Controlling OnUpdate event
	inline void EnableOnFrameUpdate( void )				{ m_updateEventEnabled = true; }
	inline void DisableOnFrameUpdate( void )			{ m_updateEventEnabled = false; }
	
	//Controlling OnSlice event
//	inline void EnableOnSliceUpdate( float delay )		{ MsgRouter.RegisterOnSliceEvent( delay, m_owner->GetID() ); }
//	inline void DisableOnSliceUpdate( void )			{ MsgRouter.UnregisterOnSliceEvent( m_owner->GetID() ); }
    void EnableOnSliceUpdate( float delay );
    void DisableOnSliceUpdate( void );
    
	//Change State
	void PopState( void );
	void ChangeState( unsigned int newState );
	void ChangeStateDelayed( float delay, unsigned int newState );
	void ChangeSubstate( unsigned int newSubstate );
	void ChangeSubstateDelayed( float delay, unsigned int newSubstate );
	
	//Switch to another State Machine
	int GetNumStateMachinesInQueue( void );
	void ResetStateMachine( void );
	void ReplaceStateMachine( StateMachine & mch );
	void QueueStateMachine( StateMachine & mch );
	void RequeueStateMachine( void );
	void PushStateMachine( StateMachine & mch );
	void PopStateMachine( void );

	//Random generation for times
	float RandDelay( float min, float max );

	//Destroy game object
	void MarkForDeletion( void )				{ m_owner->MarkForDeletion(); }

	//Helper functions
	float GetTimeInState( void )				{ return( GameTime.GetTimeDouble() - m_timeOnEnter ); }				

	//Used to verify proper message enums
	inline void VerifyMessageEnum( MSG_Name name ) {}

	//Used for debug to capture current state/substate name string
	inline void SetCurrentStateName( char * state )				{ strcpy( m_currentStateNameString, state ); m_currentSubstateNameString[0] = 0; }
	inline void SetCurrentSubstateName( char * substate )		{ strcpy( m_currentSubstateNameString, substate ); }


private:

	typedef std::vector<OBJECT_ID> BroadcastListContainer;	//Container to hold game objects to broadcast to
	typedef std::list<unsigned int> StateListContainer;		//Container to hold past states

	enum State_Change {							//Possible state change requests
		NO_STATE_CHANGE,						//No change pending
		STATE_CHANGE,							//State change pending
		STATE_POP								//State pop pending
	};

	unsigned int m_scopeState;					//The current scope of the state
	unsigned int m_scopeSubstate;				//The current scope of the substate
	unsigned int m_currentState;				//Current state
	unsigned int m_nextState;					//Next state to switch to
	int m_currentSubstate;						//Current substate (-1 indicates not valid)
	int m_nextSubstate;							//Next substate (-1 indicates not valid)
	bool m_stateChangeAllowed;					//If a state change is allowed
	State_Change m_stateChange;					//If a state change is pending
	float m_timeOnEnter;						//Time since state was entered
	bool m_updateEventEnabled;					//Whether update events are sent to the state machine
	OBJECT_ID m_ccMessagesToGameObject;			//A GameObject to CC messages to
	BroadcastListContainer m_broadcastList;		//List of GameObjects to broadcast to
	StateListContainer m_stack;					//Stack of past states (used for PopState)


	//Debug info
	char m_currentStateNameString[MAX_STATE_NAME_SIZE];		//Current state name string
	char m_currentSubstateNameString[MAX_STATE_NAME_SIZE];	//Current substate name string

	void Initialize( void );
	virtual bool States( State_Machine_Event event, MSG_Object * msg, int state, int substate ) = 0;
	void PerformStateChanges( void );
	void SendCCMsg( MSG_Name name, OBJECT_ID receiver, MSG_Data data );
	void SendMsgDelayedToMeHelper( float delay, MSG_Name name, Scope_Rule scope, StateMachineQueue queue, MSG_Data data, bool timer );

};


#pragma mark StateMachineManager

class StateMachineManager
{
public:
	StateMachineManager( GameObject & object );
	~StateMachineManager( void );

	void Update( void );
	void SendMsg( MSG_Object msg );
	void Process( State_Machine_Event event, MSG_Object * msg, StateMachineQueue queue );
	void DestroyStateMachineList( StateMachineQueue queue );

	inline StateMachine* GetStateMachine( StateMachineQueue queue )	{ if( m_stateMachineList[queue].empty() ) { return( 0 ); } else { return( m_stateMachineList[queue].back() ); } }
	inline int GetNumStateMachinesInQueue( StateMachineQueue queue )	{ return( (int)m_stateMachineList[queue].size() ); }
	void RequestStateMachineChange( StateMachine * mch, StateMachineChange change, StateMachineQueue queue );
	void ResetStateMachine( StateMachineQueue queue );
	void ReplaceStateMachine( StateMachine & mch, StateMachineQueue queue );
	void QueueStateMachine( StateMachine & mch, StateMachineQueue queue );
	void RequeueStateMachine( StateMachineQueue queue );
	void PushStateMachine( StateMachine & mch, StateMachineQueue queue, bool initialize );
	void PopStateMachine( StateMachineQueue queue );

private:

	GameObject * m_owner;													//GameObject that owns this state machine

	typedef std::list<StateMachine*> stateMachineListContainer;				//Queue of state machines. Top one is active.
	stateMachineListContainer m_stateMachineList[STATE_MACHINE_NUM_QUEUES];	//Array of state machine queues
	StateMachineChange m_stateMachineChange[STATE_MACHINE_NUM_QUEUES];		//Directions for any pending state machine changes
	StateMachine * m_newStateMachine[STATE_MACHINE_NUM_QUEUES];				//A state machine that will be added to the queue later
	void ProcessStateMachineChangeRequests( StateMachineQueue queue );

};


} // END namespace Z




