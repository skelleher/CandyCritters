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

#include "msg.hpp"
#include "Time.hpp"
#include "GameObject.hpp"
#include "StateMachine.hpp"
#include <list>


namespace Z
{



//Forward declaration
//enum StateMachineQueue;

enum SlicePolicy
{
	SLICE_POLICY_NONE,
	SLICE_POLICY_CONSTRAIN_BY_TIME,				//All slices for this frame must execute within X seconds
	SLICE_POLICY_CONSTRAIN_BY_PROPORTION,		//All slices for this frame must not exceed X% of total slice requests
	SLICE_POLICY_CONSTRAIN_BY_COUNT				//All slices for this frame must not exceed X slice requests
};

class SliceRequest
{
public:

	SliceRequest( float delay, OBJECT_ID id )		{ m_delay = delay; m_deliveryTime = delay + GameTime.GetTimeDouble(); m_id = id; }
	~SliceRequest( void ) {}

	float GetDeliveryTime( void )		{ return( m_deliveryTime ); }
	void ResetDeliveryTime( void )		{ m_deliveryTime = m_delay + GameTime.GetTimeDouble(); }
	OBJECT_ID GetID( void )				{ return( m_id ); }

protected:

	float m_delay;				//Delay between slices
	float m_deliveryTime;		//Delivery time (requested, since it might be slightly postponed)
	OBJECT_ID m_id;				//Object that requested slice
};

typedef std::list<MSG_Object*> MessageContainer;
typedef std::list<SliceRequest*> SliceContainer;



class MsgRoute
{
public:
    // Singleton
    static MsgRoute& Instance()
    {
        if (!s_pMsgRoute)
        {
            s_pMsgRoute = new MsgRoute();
        }
        
        return *s_pMsgRoute;
    };
    

	void DeliverDelayedMessages( void );
	void DeliverSlices( void );

	void SendMsg( float delay, MSG_Name name,
	              OBJECT_ID receiver, OBJECT_ID sender, 
	              Scope_Rule rule, unsigned int scope,
	              StateMachineQueue queue, MSG_Data data, 
				  bool timer, bool cc );
	
	void SendMsgBroadcast( MSG_Object & msg, GO_TYPE type = GO_TYPE_ANY );

	//Slice management
	inline void SetSlicePolicy( SlicePolicy policy, float constraint )		{ m_slicePolicy = policy; m_sliceConstraint = constraint; }
	inline void RegisterOnSliceEvent(float delay, OBJECT_ID id)				{ RegisterOnSliceEventInternal( *new SliceRequest( delay, id ) ); }
	void UnregisterOnSliceEvent( OBJECT_ID id );
	
	//Removing delayed messages
	void RemoveMsg( MSG_Name name, OBJECT_ID receiver, OBJECT_ID sender, bool timer );
	void PurgeScopedMsg( OBJECT_ID receiver );

	//For testing (unit tests)
	bool VerifyDelayedMessageOrder( void );
	void DumpSliceQueue( void );

protected:
    MsgRoute( void );
	virtual ~MsgRoute( void );

    static MsgRoute* s_pMsgRoute;
    
private:

	MessageContainer m_delayedMessages;
	SliceContainer m_sliceMessages;

	SlicePolicy m_slicePolicy;
	float m_sliceConstraint;


	void RouteMsg( MSG_Object & msg );
	
	void RouteSlice( SliceRequest & request );
	void RegisterOnSliceEventInternal(SliceRequest & request);

};


#define MsgRouter ((MsgRoute&)MsgRoute::Instance())


} // END namespace Z

