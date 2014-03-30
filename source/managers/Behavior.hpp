#pragma once

#include "Types.hpp"
#include "Handle.hpp"
#include "Object.hpp"
#include "StateMachine.hpp"
#include <string>
using std::string;

//#include "StateMachineFactories.hpp"


namespace Z
{



//=============================================================================
//
// A Behavior instance.
//
// Behaviors are basically scripts.
// For now, they are implemented as a StateMachines written in C++.  
// They can control game flow, GameObjects, GUI Controls, and more. 
//
//=============================================================================
class Behavior : virtual public Object
{
public:
    Behavior();
    virtual ~Behavior();
    
    Behavior*       Clone( ) const;

    RESULT          Init                ( IN const string& name             );
    RESULT          BindToGameObject    ( IN HGameObject hGameObject, IN StateMachineQueue queue = STATE_MACHINE_QUEUE_0 );
    RESULT          UnbindFromGameObject( IN HGameObject hGameObject, IN StateMachineQueue queue = STATE_MACHINE_QUEUE_0 );

protected:
    string          m_stateMachineName;
    StateMachine*   m_pStateMachine;
    bool            m_boundToGameObject;
};

typedef Handle<Behavior> HBehavior;




} // END namespace Z
