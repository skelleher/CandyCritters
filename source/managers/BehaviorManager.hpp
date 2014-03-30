#pragma once

#include "ResourceManager.hpp"
#include "Handle.hpp"
#include "Object.hpp"
#include "Settings.hpp"
#include "GameObject.hpp"
#include "Behavior.hpp"

#include <string>
using std::string;


namespace Z
{



//=============================================================================
//
// BehaviorManager
//
//=============================================================================


class BehaviorManager : public ResourceManager<Behavior>
{
public:
    static  BehaviorManager& Instance();
    
    virtual RESULT  Init( IN const string& settingsFilename );
    
    RESULT PushBehaviorOnToGameObject( IN HBehavior   hBehavior,   IN HGameObject hGameObject, IN StateMachineQueue queue = STATE_MACHINE_QUEUE_0 );
    RESULT PopBehaviorFromGameObject ( IN HGameObject hGameObject, IN StateMachineQueue queue = STATE_MACHINE_QUEUE_0 );
    
    
protected:
    BehaviorManager();
    BehaviorManager( const BehaviorManager& rhs );
    BehaviorManager& operator=( const BehaviorManager& rhs );
    virtual ~BehaviorManager();
 
protected:
    RESULT          CreateBehavior( IN const Settings* pSettings, IN const string& settingsPath, INOUT Behavior** ppBehavior );
};

#define BehaviorMan ((BehaviorManager&)BehaviorManager::Instance())



} // END namespace Z


