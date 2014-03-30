#pragma once
#include "StateMachine.hpp"
#include "StateMachineFactory.hpp"
#include "TouchInput.hpp"


namespace Z
{



//
// This StateMachine controls the game characters: their facial expressions.
//

class CharacterState : public StateMachine
{
public:
	CharacterState( HGameObject &hGameObject );
	~CharacterState( void );

private:
	virtual bool States( State_Machine_Event event, MSG_Object * msg, int state, int substate );

    RESULT HandleTouch ( MSG_Name msgName, IN TouchEvent* pTouchEvent );

    static void DropCritterCallback( void* pContext );

private:
    GameObject*         m_pGameObject;
    HSprite             m_hSprite;

    TouchEvent          m_startTouchEvent;
    TouchEvent          m_lastTouchEvent;
    bool                m_tapBegan;
    
    
    static HGameObject  s_hColumnController;
};


typedef StateMachineFactory<CharacterState> CharacterStateFactory;


} // END namespace Z


