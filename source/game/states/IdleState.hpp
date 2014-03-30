#pragma once
#include "StateMachine.hpp"
#include "StateMachineFactory.hpp"


namespace Z
{



//
// This is a do-nothing StateMachine for parking at the bottom of each queue.
// This way if the last StateMachine is accidentally popped, the game won't crash.
//

class IdleState : public StateMachine
{
public:
	IdleState( HGameObject &hGameObject );
	~IdleState( void );

private:
	virtual bool States( State_Machine_Event event, MSG_Object * msg, int state, int substate );
};


typedef StateMachineFactory<IdleState> IdleStateFactory;


} // END namespace Z


