/* Copyright Steve Rabin, 2007. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright Steve Rabin, 2007"
 */

#include "msg.hpp"
#include "Object.hpp"


namespace Z
{



MSG_Object::MSG_Object( void )
: m_name( MSG_NULL ),
  m_sender( INVALID_OBJECT_ID ),
  m_receiver( INVALID_OBJECT_ID ),
  m_scopeRule( SCOPE_TO_STATE_MACHINE ),
  m_scope( 0 ),
  m_queue( 0 ),
  m_deliveryTime( 0.0f ),
  m_delivered( false ),
  m_timer( 0 ),
  m_cc( false )
{

}


MSG_Object::MSG_Object( float deliveryTime, MSG_Name name,
                        OBJECT_ID sender, OBJECT_ID receiver,
                        Scope_Rule rule, unsigned int scope,
                        unsigned int queue, MSG_Data data, 
						bool timer, bool cc )
{
	SetDeliveryTime( deliveryTime );
	SetName( name );
	SetSender( sender );
	SetReceiver( receiver );
	SetScopeRule( rule );
	SetScope( scope );
	SetQueue( queue );
	SetDelivered( false );
	SetTimer( timer );
	SetCC( cc );
	m_data = data;
}



} // END namespace Z
