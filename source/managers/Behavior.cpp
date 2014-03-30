/*
 *  Behavior.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 1/26/11.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include "Behavior.hpp"
#include "StateMachineFactory.hpp"
#include "GameObjectManager.hpp"


namespace Z 
{



// ============================================================================
//
//  Behavior Implementation
//
// ============================================================================


#pragma mark Behavior Implementation

Behavior::Behavior() :
    m_stateMachineName(""),
	m_pStateMachine(NULL),
    m_boundToGameObject(false)
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "Behavior( %4d )", m_ID);
}


Behavior::~Behavior()
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "\t~Behavior( %4d )", m_ID);
    
    // TODO: ensure the StateMachine is removed from all StateMachineManagers?
    
    SAFE_DELETE(m_pStateMachine);
    
    DEBUGCHK(0);
}



Behavior*
Behavior::Clone() const
{
    Behavior* pBehaviorClone = new Behavior(*this);
    
    // Give it a new name with a random suffix
    char instanceName[MAX_PATH];
    sprintf(instanceName, "%s_%X", m_name.c_str(), (unsigned int)Platform::Random());
    pBehaviorClone->m_name = string(instanceName);
    
    // Clone should only ever be called for "template" Behaviors
    // that BehaviorManager creates at startup.  E.G.  BehaviorManager::GetCopy( "someBehavior" );
    // It should NEVER be called for a runtime instance of a Behavior which is actively bound to a GameObject.
    DEBUGCHK(!m_boundToGameObject);
    
    return pBehaviorClone;
}




RESULT
Behavior::Init( const string& name )
{
    RESULT rval        = S_OK;
    
    RETAILMSG(ZONE_OBJECT, "Behavior[%4d]::Init( %s )", m_ID, name.c_str());
    
    // Set the members
    m_name             = name;
    m_stateMachineName = name;
    
    RETAILMSG(ZONE_STATEMACHINE, "Behavior[%4d]: \"%-32s\"", m_ID, m_name.c_str());
    
    
Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Behavior::Init( \"%s\" ): failed", name.c_str());
    }
    return rval;
}



RESULT
Behavior::BindToGameObject( IN HGameObject hGameObject, IN StateMachineQueue queue )
{
    RESULT               rval                 = S_OK;
    StateMachineManager* pStateMachineManager = NULL;
    GameObject*          pGameObject          = NULL;
    

    DEBUGCHK(!m_boundToGameObject);

    SAFE_DELETE(m_pStateMachine);
    m_pStateMachine = StateMachines::Create( m_stateMachineName, hGameObject );
    if (!m_pStateMachine)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Behavior::BindToGameObject(): failed to create StateMachine \"%s\"",
            m_stateMachineName.c_str());
        goto Exit;
    }

    CHR(GOMan.GetGameObjectPointer( hGameObject, &pGameObject ));
    pStateMachineManager = pGameObject->GetStateMachineManager();
    CPREx(pStateMachineManager, E_UNEXPECTED);

    pStateMachineManager->PushStateMachine( *m_pStateMachine, queue, true );

    m_boundToGameObject = true;

Exit:
    return rval;
}



RESULT
Behavior::UnbindFromGameObject( IN HGameObject hGameObject, IN StateMachineQueue queue )
{
    RESULT               rval                 = S_OK;
    StateMachineManager* pStateMachineManager = NULL;
    GameObject*          pGameObject          = NULL;

    DEBUGCHK(m_boundToGameObject);

    CHR(GOMan.GetGameObjectPointer( hGameObject, &pGameObject ));
    pStateMachineManager = pGameObject->GetStateMachineManager();
    CPREx(pStateMachineManager, E_UNEXPECTED);

    pStateMachineManager->PopStateMachine( queue );

    m_boundToGameObject = false;

Exit:
    return rval;
}



} // END namespace Z


