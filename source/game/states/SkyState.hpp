#pragma once
#include "StateMachine.hpp"
#include "StateMachineFactory.hpp"


namespace Z
{



//
// This StateMachine controls the game sky: day/night transitions, stars, clouds, etc.
//

class SkyState : public StateMachine
{
public:
	SkyState( HGameObject &hGameObject );
	~SkyState( void );

private:
	virtual bool States( State_Machine_Event event, MSG_Object * msg, int state, int substate );
    
    static RESULT GenerateSunriseSky();
    static RESULT GenerateMorningSky();
    static RESULT GenerateAfternoonSky();
    static RESULT GenerateEveningSky();
    static RESULT GenerateSunsetSky();
    static RESULT GenerateNightSky();
    
    static void   HideEmitterCallback( void* pContext );
    static void   SkyDoneFadingOutCallback( void* pContext );
};


typedef StateMachineFactory<SkyState> SkyStateFactory;


} // END namespace Z


