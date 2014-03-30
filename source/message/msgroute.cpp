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


// TEST TEST: allow queing up duplicate messages to a StateMachine IFF 
// their delivery times are not identical.
// Otherwise subsequent messages are dropped.
#define ALLOW_DUPLICATE_MESSAGES


namespace Z
{



// Singleton instance
MsgRoute* MsgRoute::s_pMsgRoute = NULL;



/*---------------------------------------------------------------------------*
  Name:         MsgRoute

  Description:  Constructor
 *---------------------------------------------------------------------------*/
MsgRoute::MsgRoute( void )
: m_slicePolicy(SLICE_POLICY_NONE),
  m_sliceConstraint(0.1f)
{

}

/*---------------------------------------------------------------------------*
  Name:         ~MsgRoute

  Description:  Destructor
 *---------------------------------------------------------------------------*/
MsgRoute::~MsgRoute( void )
{
	for( MessageContainer::iterator i=m_delayedMessages.begin(); i!=m_delayedMessages.end(); ++i )
	{
		MSG_Object * msg = *i;
		delete( msg );
	}

	m_delayedMessages.clear();

	for( SliceContainer::iterator i=m_sliceMessages.begin(); i!=m_sliceMessages.end(); ++i )
	{
		SliceRequest * request = *i;
		delete( request );
	}

	m_sliceMessages.clear();

}


/*---------------------------------------------------------------------------*
  Name:         SendMsg

  Description:  Sends a message through the message router. This function
                determines if the message should be delivered immediately
				or should be held until the delivery time.

  Arguments:    delay    : the number of seconds to delay the message
                name     : the message name
				receiver : the ID of the receiver
				sender   : the ID of the sender
				rule     : the scoping rule for the message
				scope    : the scope of the message (a state index)
				queue    : the queue to send the message to
				data     : a piece of data
				timer    : if this message is a timer (sent periodically)
				cc       : if this message is a CC (a copy)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MsgRoute::SendMsg( float delay, MSG_Name name,
                        OBJECT_ID receiver, OBJECT_ID sender,
                        Scope_Rule rule, unsigned int scope,
                        StateMachineQueue queue, MSG_Data data, 
						bool timer, bool cc )
{

	if( delay <= 0.0f )
	{	//Deliver immediately
		MSG_Object msg( GameTime.GetTimeDouble(), name, sender, receiver, rule, scope, queue, data, timer, cc );
		RouteMsg( msg );
	}
	else
	{	
		float deliveryTime = delay + GameTime.GetTimeDouble();

		//Check for duplicates - time complexity O(n)
		bool set = false;
		float lastDeliveryTime = 0.0f;
		MessageContainer::iterator insertPosition = m_delayedMessages.end();
		MessageContainer::iterator i;
		for( i=m_delayedMessages.begin(); i!=m_delayedMessages.end(); ++i )
		{
			if( (*i)->IsDelivered() == false &&
				(*i)->GetName() == name &&
				(*i)->GetReceiver() == receiver &&
				(*i)->GetSender() == sender &&
				(*i)->GetScopeRule() == rule &&
				(*i)->GetScope() == scope &&
				(*i)->GetQueue() == queue &&
				(*i)->IsTimer() == timer &&
#ifdef ALLOW_DUPLICATE_MESSAGES
                (*i)->GetDeliveryTime() == deliveryTime &&
#endif
				( ((*i)->IsIntData() && data.IsInt() && (*i)->GetIntData() == data.GetInt()) ||
				  ((*i)->IsFloatData() && data.IsFloat() && (*i)->GetFloatData() == data.GetFloat()) ||
				  (!(*i)->IsDataValid() && !data.IsValid()) ) )
			{	//Already in list - don't add
//				ASSERTMSG(0, "MsgRoute::SendMsg - Message already in list");
				return;
			}

			//Sanity check that list is in order
			if( (*i)->GetDeliveryTime() < lastDeliveryTime )
			{
				ASSERTMSG( 0, "MsgRoute::SendMsg - Message list not in order" );
			}
			lastDeliveryTime = (*i)->GetDeliveryTime();

			if( !set && (*i)->GetDeliveryTime() > deliveryTime )
			{	//Record place to insert new delayed message (we need the entry one beyond)
				set = true;
				insertPosition = i;
			}
		}
		
		//Store in delivery list
		MSG_Object * msg = new MSG_Object( deliveryTime, name, sender, receiver, rule, scope, queue, data, timer, false );
		if( m_delayedMessages.empty() || deliveryTime <= m_delayedMessages.front()->GetDeliveryTime() )
		{	//Put at the front if the list is empty or the delivery time is sooner than the first entry
			m_delayedMessages.push_front( msg );
		}
		else
		{
			m_delayedMessages.insert( insertPosition, msg );
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         VerifyDelayedMessageOrder

  Description:  Verifies that the delayed messages are being ordered properly.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
bool MsgRoute::VerifyDelayedMessageOrder( void )
{	//Test for order - time complexity O(n)
	float lastDeliveryTime = 0;

	MessageContainer::iterator i;
	for( i=m_delayedMessages.begin(); i!=m_delayedMessages.end(); ++i )
	{
		float time = (*i)->GetDeliveryTime();
		if( time < lastDeliveryTime )
		{
			ASSERTMSG( 0, "MsgRoute::VerifyDelayedMessageOrder - Message list not in order" );
			return false;
		}
		lastDeliveryTime = (*i)->GetDeliveryTime();
	}

	return true;
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgBroadcast

  Description:  Sends a message to every object of a certain type.

  Arguments:    msg    : the message to broadcast
                type   : the type of object (optional)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MsgRoute::SendMsgBroadcast( MSG_Object & msg, GO_TYPE type )
{
	GameObjectList list;
	GOMan.GetList( type, &list );

	GameObjectListIterator i;
	for( i=list.begin(); i!=list.end(); ++i )
	{
		if( msg.GetSender() != (*i)->GetID() )
		{
			if((*i)->GetStateMachineManager())
			{
				(*i)->GetStateMachineManager()->SendMsg( msg );
			}
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         RegisterOnSliceEventInternal

  Description:  Register the periodic load balanced OnSlice event.

  Arguments:    request  : the slice request (already set up internally)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MsgRoute::RegisterOnSliceEventInternal(SliceRequest & request)
{
	SliceContainer::iterator insertPosition = m_sliceMessages.end();

	float lastDeliveryTime = 0.0f;

	for( SliceContainer::iterator i=m_sliceMessages.begin(); i!=m_sliceMessages.end(); ++i )
	{		
		//Sanity check that list is in order
		if( (*i)->GetDeliveryTime() < lastDeliveryTime )
		{
			ASSERTMSG( 0, "MsgRoute::RegisterOnSliceEventInternal - List not in order" );
		}
		lastDeliveryTime = (*i)->GetDeliveryTime();

		if( (*i)->GetDeliveryTime() > request.GetDeliveryTime() )
		{
			insertPosition = i;
			break;
		}
	}

	if( m_sliceMessages.empty() || request.GetDeliveryTime() <= m_sliceMessages.front()->GetDeliveryTime() )
	{
		m_sliceMessages.push_front( &request );
	}
	else
	{
		m_sliceMessages.insert( insertPosition, &request );
	}
}

void MsgRoute::DumpSliceQueue( void )
{
	DEBUGMSG(ZONE_MESSAGE, "DumpSliceQueue\n" );
	for( SliceContainer::iterator i=m_sliceMessages.begin(); i!=m_sliceMessages.end(); ++i )
	{		
		DEBUGMSG(ZONE_INFO, "Slice(id:%d, delivery:%f)\n", (*i)->GetID(), (*i)->GetDeliveryTime() );
	}
	DEBUGMSG(ZONE_MESSAGE, "\n" );
}

/*---------------------------------------------------------------------------*
  Name:         UnregisterOnSliceEvent

  Description:  Unregister the periodic load balanced OnSlice event.

  Arguments:    id  : the object id of the state machine

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MsgRoute::UnregisterOnSliceEvent( OBJECT_ID id )
{
	for( SliceContainer::iterator i = m_sliceMessages.begin(); i != m_sliceMessages.end(); ++i )
	{		
		if( id == (*i)->GetID() )
		{
			SliceRequest * request = *i;
			delete( request );
			m_sliceMessages.erase( i );
			return;
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         DeliverSlices

  Description:  Sends slices if the time is right.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MsgRoute::DeliverSlices( void )
{
	int sent = 0;
	int total = (int)m_sliceMessages.size();
    double timeStart = GameTime.GetTimeDouble();
    
	SliceContainer::iterator i = m_sliceMessages.begin();
	while( i != m_sliceMessages.end() )
	{
		ASSERTMSG( m_slicePolicy != SLICE_POLICY_NONE, "MsgRoute::DeliverSlices - Slice policy not set" );

		if( (*i)->GetDeliveryTime() <= GameTime.GetTimeDouble() )
		{	//Deliver and reschedule
			sent++;
			SliceRequest * request = *i;
			RouteSlice( *request );
			i = m_sliceMessages.erase( i );
			request->ResetDeliveryTime();
			RegisterOnSliceEventInternal( *request );
		}
		else
		{	//All messages beyond this one are not ready to fire, since the list is sorted
			return;
		}

		//Decide whether to stop sending for this frame
		if( m_slicePolicy == SLICE_POLICY_CONSTRAIN_BY_TIME )
		{
			double curtime = GameTime.GetTimeDouble();
			if( curtime - timeStart > m_sliceConstraint )
			{
				return;
			}
		}
		else if( m_slicePolicy == SLICE_POLICY_CONSTRAIN_BY_PROPORTION )
		{
			float proportionSent = (float)sent / (float) total;
			if( proportionSent >= m_sliceConstraint )
			{
				return;
			}
		}
		else if( m_slicePolicy == SLICE_POLICY_CONSTRAIN_BY_COUNT )
		{
			if( sent >= m_sliceConstraint )
			{
				return;
			}
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         RouteSlice

  Description:  Routes the slice to the receiver.

  Arguments:    request : the slice request to route

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MsgRoute::RouteSlice( SliceRequest & request )
{
    GameObject* pGameObject = NULL;
    GOMan.GetGameObjectPointer( request.GetID(), &pGameObject );
    
	if( pGameObject != 0 && pGameObject->GetStateMachineManager() )
	{
		pGameObject->GetStateMachineManager()->Process( EVENT_SliceUpdate, 0, STATE_MACHINE_QUEUE_ALL );
	}
}

/*---------------------------------------------------------------------------*
  Name:         DeliverDelayedMessages

  Description:  Sends delayed messages if the time is right.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MsgRoute::DeliverDelayedMessages( void )
{
	MessageContainer::iterator i = m_delayedMessages.begin();
	while( i != m_delayedMessages.end() )
	{
		if( (*i)->GetDeliveryTime() <= GameTime.GetTimeDouble() )
		{	//Deliver and delete msg
			MSG_Object * msg = *i;
			RouteMsg( *msg );
			delete( msg );
			i = m_delayedMessages.erase( i );
		}
		else
		{	//All messages beyond this one are not ready to fire, since the list is sorted
			return;
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         RouteMsg

  Description:  Routes the message to the receiver, only if the scoping rules
                allow it.

  Arguments:    msg : the message to route

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MsgRoute::RouteMsg( MSG_Object & msg )
{
	GameObject * pObject = NULL;
    GOMan.GetGameObjectPointer( msg.GetReceiver(), &pObject );

	if( pObject != 0 && pObject->GetStateMachineManager() )
	{
		Scope_Rule rule = msg.GetScopeRule();
		if( rule == SCOPE_TO_STATE_MACHINE ||
			( rule == SCOPE_TO_SUBSTATE && msg.GetScope() == pObject->GetStateMachineManager()->GetStateMachine((StateMachineQueue)msg.GetQueue())->GetScopeSubstate() ) ||
			( rule == SCOPE_TO_STATE    && msg.GetScope() == pObject->GetStateMachineManager()->GetStateMachine((StateMachineQueue)msg.GetQueue())->GetScopeState() ) )
		{	//Scope matches
			msg.SetDelivered( true );	//Important to set as delivered since timer messages 
										//will resend themselves immediately (and would get
										//thrown away if we didn't set this, since it would look
										//like a redundant message)
			
			if( msg.IsTimer() )
			{	//Timer message that occurs periodically
				float delay = msg.GetFloatData();	//Timer value stored in data field
				msg.SetIntData( 0 );				//Zero out data field
				//Queue up next periodic msg
				pObject->GetStateMachineManager()->GetStateMachine((StateMachineQueue)msg.GetQueue())->SetTimerExternal( delay, msg.GetName(), rule );
			}
			
			if( msg.IsCC() ) {
				pObject->GetStateMachineManager()->Process( EVENT_CCMessage, &msg, (StateMachineQueue)msg.GetQueue() );
			}
			else {
				pObject->GetStateMachineManager()->Process( EVENT_Message, &msg, (StateMachineQueue)msg.GetQueue() );
			}
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         RemoveMsg

  Description:  Removes messages from the delayed message list that meet
                the criteria. This is useful to avoid duplicate delayed
				messages.

  Arguments:    name     : the name of the message
                receiver : the receiver ID of the message
				sender   : the sender ID of the message
				timer    : whether the message is a timer

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MsgRoute::RemoveMsg( MSG_Name name, OBJECT_ID receiver, OBJECT_ID sender, bool timer )
{
	MessageContainer::iterator i = m_delayedMessages.begin();
	while( i != m_delayedMessages.end() )
	{
		MSG_Object * msg = *i;
		if( msg->GetName() == name &&
			msg->GetReceiver() == receiver &&
			msg->GetSender() == sender &&
			msg->IsTimer() == timer &&
			!msg->IsDelivered() )
		{
			delete( msg );
			i = m_delayedMessages.erase( i );
		}
		else
		{
			++i;
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         PurgeScopedMsg

  Description:  Removes messages from the delayed message list for a given
                receiver if the message is scoped to a particular state. This
				is useful if the receiver changes state machines, since the 
				messages are no longer valid.

  Arguments:    receiver : the receiver ID of the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MsgRoute::PurgeScopedMsg( OBJECT_ID receiver )
{
	MessageContainer::iterator i = m_delayedMessages.begin();
	while( i != m_delayedMessages.end() )
	{
		MSG_Object * msg = *i;
		if( msg->GetReceiver() == receiver &&
			msg->GetScopeRule() != SCOPE_TO_STATE_MACHINE &&
			!msg->IsDelivered() )
		{
			delete( msg );
			i = m_delayedMessages.erase( i );
		}
		else
		{
			++i;
		}
	}
}



} // END namespace Z
