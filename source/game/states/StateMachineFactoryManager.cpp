#include "StateMachineFactory.hpp"
#include "IdleState.hpp"
#include "ColumnState.hpp"
#include "GameState.hpp"
#include "GameScreens.hpp"
#include "SkyState.hpp"
#include "CharacterState.hpp"
#include "BombState.hpp"
//#include "TutorialState.hpp"


namespace Z
{

// TODO: Either get rid of behaviors.xml, or better yet generate this table from it, 
// so that we define StateMachines in one place.

StateMachineFactoryManager::StateMachineFactoryItem   StateMachineFactoryManager::s_stateMachineFactories[] =
{
    { "IdleState",      (SM_FACTORY_METHOD)StateMachineFactory< IdleState       >::Create    },
    { "ColumnState",    (SM_FACTORY_METHOD)StateMachineFactory< ColumnState     >::Create    },
    { "GameState",      (SM_FACTORY_METHOD)StateMachineFactory< GameState       >::Create    },
    { "GameScreens",    (SM_FACTORY_METHOD)StateMachineFactory< GameScreens     >::Create    },
    { "SkyState",       (SM_FACTORY_METHOD)StateMachineFactory< SkyState        >::Create    },
    { "CharacterState", (SM_FACTORY_METHOD)StateMachineFactory< CharacterState  >::Create    },
    { "BombState",      (SM_FACTORY_METHOD)StateMachineFactory< BombState       >::Create    },
//    { "TutorialState",  (SM_FACTORY_METHOD)StateMachineFactory< TutorialState   >::Create    },
    { "",               NULL }
};



} // END namespace Z