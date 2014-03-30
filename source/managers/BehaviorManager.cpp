/*
 *  BehaviorManager.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 1/26/11.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include "Engine.hpp"
#include "BehaviorManager.hpp"
#include "FileManager.hpp"
#include "Log.hpp"
#include "Handle.hpp"
#include "Settings.hpp"
#include "Platform.hpp"
#include "Types.hpp"
#include "Util.hpp"
#include "GameObjectManager.hpp"



namespace Z
{



BehaviorManager& 
BehaviorManager::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new BehaviorManager();
    }
    
    return static_cast<BehaviorManager&>(*s_pInstance);
}


BehaviorManager::BehaviorManager()
{
    RETAILMSG(ZONE_VERBOSE, "BehaviorManager()");
    
    s_pResourceManagerName = "BehaviorManager";
}


BehaviorManager::~BehaviorManager()
{
    RETAILMSG(ZONE_VERBOSE, "\t~BehaviorManager()");
    
    DEBUGCHK(0);
}



RESULT
BehaviorManager::Init( IN const string& settingsFilename )
{
    RETAILMSG(ZONE_INFO, "BehaviorManager::Init( %s )", settingsFilename.c_str());

    RESULT rval = S_OK;
    char   path[MAX_PATH];
    
    //
    // Create a Settings object and load the file.
    //
    Settings mySettings;
    if ( FAILED(mySettings.Read( settingsFilename )) )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: BehaviorManager::Init( %s ): failed to load settings file", settingsFilename.c_str() );
        return E_UNEXPECTED;
    }
    

    //
    // Create each Behavior.
    //
    UINT32 numBehaviors = mySettings.GetInt("/Behaviors.NumBehaviors");

    for (int i = 0; i < numBehaviors; ++i)
    {
        sprintf(path, "/Behaviors/Behavior%d", i);
        //DEBUGMSG(ZONE_INFO, "Loading [%s]", path);

        Behavior *pBehavior = NULL;
        CreateBehavior( &mySettings, path, &pBehavior );
        if (!pBehavior)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: BehaviorManager::Init( %s ): failed to create Behavior", path);
            // Continue loading other Behaviors rather than aborting.
            continue;
        }
        
        //DEBUGMSG(ZONE_Behavior, "Created Behavior [%s]", pBehavior->GetName().c_str());
        CHR(Add(pBehavior->GetName(), pBehavior));
    }
    
Exit:
    return rval;
}



RESULT
BehaviorManager::CreateBehavior( IN const Settings* pSettings, IN const string& settingsPath, INOUT Behavior** ppBehavior )
{
    RESULT rval = S_OK;
    string name;
    
    if (!pSettings || "" == settingsPath || !ppBehavior)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: BehaviorManager::CreateBehavior(): invalid argument");
        return E_INVALID_ARG;
    }
    
    Behavior *pBehavior = new Behavior();
    CPR(pBehavior);

    name = pSettings->GetString( settingsPath + ".Name" );
    pBehavior->Init( name );

    // Caller must AddRef()
    *ppBehavior = pBehavior;
    
Exit:    
    return rval;
}



RESULT 
BehaviorManager::PushBehaviorOnToGameObject( IN HBehavior hBehavior, IN HGameObject hGameObject, IN StateMachineQueue queue )
{
    RESULT rval           = S_OK;
    string gameObjectName = "";
    
    Behavior* pBehavior = GetObjectPointer( hBehavior );
    if (pBehavior)
    {
        GOMan.GetName( hGameObject, &gameObjectName );
        
        RETAILMSG(ZONE_STATEMACHINE | ZONE_VERBOSE, "BehaviorManager::PushBehaviorOnToGameObject( \"%s\", \"%s\", queue: %d )", 
            pBehavior->GetName().c_str(),
            gameObjectName.c_str(),
            queue);

        CHR(pBehavior->BindToGameObject( hGameObject, queue ));
    }
    else 
    {
        rval = E_FAIL;
    }

Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: BehaviorManager::PushBehaviorOnToGameObject( \"%s\", \"%s\" ): Behavior or GO not found", 
            pBehavior ? pBehavior->GetName().c_str() : "NULL",
            gameObjectName.c_str());
    }
    
    return rval;
}



RESULT
BehaviorManager::PopBehaviorFromGameObject( IN HGameObject hGameObject, IN StateMachineQueue queue )
{
    RESULT               rval                 = S_OK;
    GameObject*          pGameObject          = NULL;
    StateMachineManager* pStateMachineManager = NULL;

    CHR(GOMan.GetGameObjectPointer( hGameObject, &pGameObject ));
    pStateMachineManager = pGameObject->GetStateMachineManager();
    CPREx(pStateMachineManager, E_UNEXPECTED);
    
    pStateMachineManager->PopStateMachine( queue );

    RETAILMSG(ZONE_STATEMACHINE | ZONE_VERBOSE, "BehaviorManager::PopBehaviorFromGameObject( \"%s\", queue: %d )", 
        pGameObject->GetName().c_str(),
        queue);

    
Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: BehaviorManager::PopBehaviorFromGameObject( \"%s\" ) GO not found, or has no Behaviors", 
            pGameObject ? pGameObject->GetName().c_str() : "NULL");
    }   

    return rval;
}
    


} // END namespace Z


