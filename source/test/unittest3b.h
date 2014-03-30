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



class UnitTest3b : public StateMachine
{
public:

	UnitTest3b( GameObject & object )
		: StateMachine( object ) {}
	UnitTest3b( HGameObject & hGameObject )
		: StateMachine( hGameObject ) {}
	~UnitTest3b( void ) {}


private:

	virtual bool States( State_Machine_Event event, MSG_Object * msg, int state, int substate );

	//Put state variables here

};



} // END namespace Z


