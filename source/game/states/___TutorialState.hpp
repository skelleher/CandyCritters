//
//  TutorialState.hpp
//  Critters
//
//  Created by Sean Kelleher on 5/27/13.
//
//

#pragma once
#include "StateMachine.hpp"
#include "StateMachineFactory.hpp"


namespace Z
{



class TutorialState : public StateMachine
{
public:
	TutorialState( HGameObject &hGameObject );
	~TutorialState( void );

private:
	virtual bool States( State_Machine_Event event, MSG_Object * msg, int state, int substate );

    static RESULT GenerateHomeScreenCritter ( MAP_POSITION startPos = MAP_POSITION(-1,-1), MAP_POSITION endPos = MAP_POSITION(-1,-1), bool playSound = false, BrickType* pBrickType = NULL );
};


typedef StateMachineFactory<TutorialState> TutorialStateFactory;


} // END namespace Z


