/* Copyright Steve Rabin, 2005. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright Steve Rabin, 2005"
 */

#pragma once

#include "StateMachine.hpp"


namespace Z
{



class UnitTest5 : public StateMachine
{
public:

	UnitTest5( GameObject & object )
		: StateMachine( object ) {}
	UnitTest5( HGameObject &hGameObject )
		: StateMachine( hGameObject ) {}
	~UnitTest5( void ) {}


private:

	virtual bool States( State_Machine_Event event, MSG_Object * msg, int state, int substate );

	//Put state variables here
	int m_sliceCount;
	int m_updateCount;

};



} // END namespace Z


