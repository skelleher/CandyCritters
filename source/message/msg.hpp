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

#include "Types.hpp"
#include "Macros.hpp"


//namespace Z
//{


//Macro trick to make message names enums
//rom the file msgnames.hpp
#define REGISTER_MESSAGE_NAME(x) x,
typedef enum
{
	#include "msgnames.hpp"
	MSG_NUM
} MSG_Name;
#undef REGISTER_MESSAGE_NAME



namespace Z
{



//Delayed messages can be scoped with the following enum.
//
//Scoping explanation:
//A message scoped to a state or substate will not be delivered
//if the state or substate changes (only applies to delayed
//messages sent to yourself). If a delayed message scoped to the
//state is sent from state "A" and the state changes to "B" then 
//back to "A", the message will not be delivered since there was 
//a state change. By default, delayed messages are scoped to the 
//substate (which is equivalient to being scoped to the state if 
//there are no substates).
//
enum Scope_Rule { 
	SCOPE_TO_SUBSTATE,
	SCOPE_TO_STATE,
	SCOPE_TO_STATE_MACHINE
};

#define NEXT_FRAME 0.0001f

union MSG_Data_Union
{
	int     intValue;
	float   floatValue;
    void*   pVoid;
};

enum MSG_Data_Value
{
	MSG_DATA_INVALID,
	MSG_DATA_INT,
	MSG_DATA_FLOAT,
    MSG_DATA_POINTER
};

class MSG_Data
{
public:
	MSG_Data( void )						{ m_valueType = MSG_DATA_INVALID; }
	MSG_Data( int data )					{ m_data.intValue = data;       m_valueType = MSG_DATA_INT; }
	MSG_Data( float data )					{ m_data.floatValue = data;     m_valueType = MSG_DATA_FLOAT; }
    MSG_Data( void* data )                  { m_data.pVoid = data;          m_valueType = MSG_DATA_POINTER; }
	inline bool  IsValid( void )			{ return( m_valueType != MSG_DATA_INVALID ); }
	inline bool  IsInt( void )				{ return( m_valueType == MSG_DATA_INT ); }
	inline bool  IsFloat( void )			{ return( m_valueType == MSG_DATA_FLOAT ); }
    inline bool  IsPointer( void )		    { return( m_valueType == MSG_DATA_POINTER ); }
	inline void  SetInt( int data )			{ m_data.intValue = data; m_valueType = MSG_DATA_INT; }
	inline int   GetInt( void )				{ ASSERTMSG( m_valueType == MSG_DATA_INT, "Message data not of correct type" ); return( m_data.intValue ); }
	inline void  SetFloat( float data )		{ m_data.floatValue = data; m_valueType = MSG_DATA_FLOAT; }
	inline float GetFloat( void )			{ ASSERTMSG( m_valueType == MSG_DATA_FLOAT, "Message data not of correct type" ); return( m_data.floatValue ); }
	inline void  SetPointer( void* data )	{ m_data.pVoid = data; m_valueType = MSG_DATA_POINTER; }
	inline void* GetPointer( void )			{ ASSERTMSG( m_valueType == MSG_DATA_POINTER, "Message data not of correct type" ); return( m_data.pVoid ); }

private:
	MSG_Data_Value m_valueType;
	MSG_Data_Union m_data;
};


class MSG_Object
{
public:

	MSG_Object( void );
	MSG_Object( float deliveryTime, MSG_Name name, 
	            OBJECT_ID sender, OBJECT_ID receiver, 
	            Scope_Rule rule, unsigned int scope, 
				unsigned int queue, MSG_Data data, 
				bool timer, bool cc );
	            
	~MSG_Object( void ) {}

	inline MSG_Name GetName( void )					{ return( m_name ); }
	inline void SetName( MSG_Name name )			{ m_name = name; }

	inline OBJECT_ID GetSender( void )				{ return( m_sender ); }
	inline void SetSender( OBJECT_ID sender )		{ m_sender = sender; }

	inline OBJECT_ID GetReceiver( void )			{ return( m_receiver ); }
	inline void SetReceiver( OBJECT_ID receiver )	{ m_receiver = receiver; }

	inline Scope_Rule GetScopeRule( void )			{ return( (Scope_Rule)m_scopeRule ); }
	inline void SetScopeRule( Scope_Rule rule )		{ m_scopeRule = rule; }

	inline unsigned int GetScope( void )			{ return( m_scope ); }
	inline void SetScope( unsigned int scope )		{ m_scope = scope; }

	inline unsigned int GetQueue( void )			{ return( m_queue ); }
	inline void SetQueue( unsigned int queue )		{ ASSERTMSG( queue < 8, "MSG_Object::SetQueue - queue out of bounds for 3 bit encoding. Change encoding if needed." ); m_queue = queue; }

	inline float GetDeliveryTime( void )			{ return( m_deliveryTime ); }
	inline void SetDeliveryTime( float time )		{ m_deliveryTime = time; }

	inline bool IsDelivered( void )					{ return( m_delivered ); }
	inline void SetDelivered( bool value )			{ m_delivered = value; }

	inline bool IsDataValid( void )					{ return( m_data.IsValid() ); }
	inline bool IsIntData( void )					{ return( m_data.IsInt() ); }
	inline bool IsFloatData( void )					{ return( m_data.IsFloat() ); }
    inline bool IsPointerData( void )               { return( m_data.IsPointer() ); }

	inline int GetIntData( void )					{ return( m_data.GetInt() ); }
	inline void SetIntData( int data )				{ m_data.SetInt( data ); }

	inline float GetFloatData( void )				{ return( m_data.GetFloat() ); }
	inline void SetFloatData( float data )			{ m_data.SetFloat( data ); }

	inline void* GetPointerData( void )				{ return( m_data.GetPointer() ); }
	inline void  SetPointerData( void* pData )		{ m_data.SetPointer( pData ); }

    inline MSG_Data GetMsgData( void )				{ return( m_data ); }

	inline bool IsTimer( void )						{ return( m_timer ); }
	inline void SetTimer( bool value )				{ m_timer = value; }
	
	inline bool IsCC( void )						{ return( m_cc ); }
	inline void SetCC( bool value )					{ m_cc = value; }
	

private:

	MSG_Name m_name;				//Message name
	OBJECT_ID m_sender;				//Object that sent the message
	OBJECT_ID m_receiver;			//Object that will get the message
	MSG_Data m_data;				//Data that is passed with the message
	float m_deliveryTime;			//Time at which to send the message
	unsigned int m_scope;			//State or substate instance in which the receiver is allowed to get the message

	unsigned int m_queue: 3;		//Queue index to deliver message to (only valid when sender = receiver)
	unsigned int m_scopeRule: 2;	//Rule for how to interpret scope
	unsigned int m_delivered: 1;	//Whether the message has been delivered
	unsigned int m_timer: 1;		//Message is sent periodically
	unsigned int m_cc: 1;			//Message is a carbon copy that was received by someone else
};



} // END namespace Z


