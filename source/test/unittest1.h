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

#include "StateMachine.hpp"

namespace Z
{


class UnitTest1 : public StateMachine
{
public:

	UnitTest1( GameObject & object )
		: StateMachine( object ) {}
	UnitTest1( HGameObject & hGameObject )
		: StateMachine( hGameObject ) {}
	~UnitTest1( void ) {}


private:

	virtual bool States( State_Machine_Event event, MSG_Object * msg, int state, int substate );

	//Put state variables here
	unsigned int m_count;

};



} // END namespace Z


