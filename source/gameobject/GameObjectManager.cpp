/*
 *  GameObjectManager.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 10/10/10.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include "Engine.hpp"
#include "GameObjectManager.hpp"
#include "FileManager.hpp"
#include "Log.hpp"
#include "Handle.hpp"
#include "Settings.hpp"
#include "Platform.hpp"
#include "Types.hpp"
#include "Util.hpp"
#include "msgroute.hpp"
#include "BehaviorManager.hpp"
#include "StoryboardManager.hpp"
#include "LayerManager.hpp"


// Don't delete GameObjects when their ref count goes to zero.
// Instead mark for deletion; they'll be deleted at the end of the frame.
//#define DEFER_GAME_OBJECT_DELETION


namespace Z
{


#pragma mark -
#pragma mark Lifetime
GameObjectManager& 
GameObjectManager::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new GameObjectManager();
    }
    
    return static_cast<GameObjectManager&>(*s_pInstance);
}


GameObjectManager::GameObjectManager()
{
    RETAILMSG(ZONE_VERBOSE, "GameObjectManager()");
    
    s_pResourceManagerName = "GameObjectManager";
}


GameObjectManager::~GameObjectManager()
{
    RETAILMSG(ZONE_VERBOSE, "\t~GameObjectManager()");
}



RESULT
GameObjectManager::Init( IN const string& settingsFilename )
{
    RETAILMSG(ZONE_INFO, "GameObjectManager::Init( %s )", settingsFilename.c_str());

    RESULT rval = S_OK;
    char   path[MAX_PATH];
    
    //
    // Create a Settings object and load the file.
    //
    Settings mySettings;
    if ( FAILED(mySettings.Read( settingsFilename )) )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::Init( %s ): failed to load settings file", settingsFilename.c_str() );
        return E_UNEXPECTED;
    }
    

    //
    // Create each GameObject.
    //
    UINT32 numGameObjects = mySettings.GetInt("/GameObjects.NumGameObjects");

    for (int i = 0; i < numGameObjects; ++i)
    {
        sprintf(path, "/GameObjects/GameObject%d", i);
        //DEBUGMSG(ZONE_INFO, "Loading [%s]", path);

        GameObject *pGameObject = NULL;
        CreateGameObject( &mySettings, path, &pGameObject );
        if (!pGameObject)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::Init( %s ): failed to create GameObject", path);
            // Continue loading other GameObjects rather than aborting.
            continue;
        }
        
        DEBUGMSG(ZONE_GAMEOBJECT, "Created GameObject [%s]", pGameObject->GetName().c_str());
        CHR(Add(pGameObject->GetName(), pGameObject));
    }
    
Exit:
    return rval;
}



RESULT
GameObjectManager::Shutdown()
{
    RESULT rval = S_OK;
    
    // TODO: clean up m_GOIDToGameObjectMap
    // TODO: call base class Shutdown()
    
    DEBUGCHK(0);
    
Exit:
    return rval;
}



#pragma mark -
#pragma mark Resources

RESULT
GameObjectManager::Add( IN GameObject* pGameObject, INOUT HGameObject* pHandle )
{
    RESULT rval = S_OK;
    
    CPREx(pGameObject, E_NULL_POINTER);
    
    // Add the GO to the usual handle/name/object lists
    CHR(ResourceManager<GameObject>::Add( pGameObject->GetName(), pGameObject, pHandle ));
    
    // Also add it to our ObjectID -> GameObject map
    m_GOIDToGameObjectMap.insert( std::pair< OBJECT_ID, GameObject* >(pGameObject->GetID(), pGameObject) );
    
Exit:
    return rval;
}



RESULT
GameObjectManager::Add( IN const string& name, IN GameObject* pGameObject, INOUT HGameObject* pHandle )
{
    RESULT rval = S_OK;
    
    CPREx(pGameObject, E_NULL_POINTER);
    
    // Add the GO to the usual handle/name/object lists
    CHR(ResourceManager<GameObject>::Add( name, pGameObject, pHandle ));
    
    // Also add it to our ObjectID -> GameObject map
    m_GOIDToGameObjectMap.insert( std::pair< OBJECT_ID, GameObject* >(pGameObject->GetID(), pGameObject) );
    
Exit:
    return rval;
}



RESULT
GameObjectManager::Remove( IN HGameObject hGameObject )
{
    RESULT rval = S_OK;

    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (!pGameObject)
    {
        RETAILMSG(ZONE_ERROR, "GameObjectManager::Remove(): handle 0x%x not found", (UINT32)hGameObject);
        rval = E_NOT_FOUND;
        goto Exit;
    }
    
    CHR(Remove( pGameObject ));

Exit:
    return rval;
}



RESULT
GameObjectManager::Remove( IN GameObject* pGameObject, IN HGameObject handle )
{
    RESULT rval = S_OK;
    GOIDToGameObjectMapIterator ppGameObject;
    
    CPR(pGameObject);

    // Remove from m_GOIDToGameObjectMap
    ppGameObject = m_GOIDToGameObjectMap.find( pGameObject->GetID() );
    if (ppGameObject != m_GOIDToGameObjectMap.end())
    {
        m_GOIDToGameObjectMap.erase( ppGameObject );
    }
    else 
    {
        RETAILMSG(ZONE_ERROR, "GameObjectManager::Remove(): object %d \"%s\" not found", pGameObject->GetID(), pGameObject->GetName().c_str());
        rval = E_UNEXPECTED;
        goto Exit;
    }


    DEBUGMSG(ZONE_RESOURCE, "%s::Remove( handle: 0x%x \"%s\" )", s_pResourceManagerName, (UINT32)handle, pGameObject->GetName().c_str());
    DEBUGMSG(ZONE_RESOURCE, "%s::m_GOIDToGameObjectMap: %d",
            s_pResourceManagerName,
            m_GOIDToGameObjectMap.size());

    CHR(ResourceManager<GameObject>::Remove( pGameObject, handle ));
    
Exit:
    return rval;
}



RESULT
GameObjectManager::Release( IN HGameObject hGameObject )
{
    RESULT rval = S_OK;
    GOIDToGameObjectMapIterator ppGameObject;

    if (hGameObject.IsNull())
    {
        return S_OK;
    }


    // TODO: release the handle first; will catch any double-release and log an error.
//    CHR(ResourceManager<GameObject>::Release( hGameObject ));


    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (!pGameObject)
    {
        RETAILMSG(ZONE_ERROR, "GameObjectManager::Release(): handle 0x%x not found", (UINT32)hGameObject);
        rval = E_NOT_FOUND;
        goto Exit;
    }

    if (1 == pGameObject->GetRefCount())
    {
#ifdef DEFER_GAME_OBJECT_DELETION
        pGameObject->MarkForDeletion();
#else            
        CHR(Remove( pGameObject, hGameObject ));
#endif        
    }
    else 
    {
        CHR(ResourceManager<GameObject>::Release( hGameObject ));
    }

Exit:
    return rval;
}


#pragma mark -
#pragma mark Create

RESULT
GameObjectManager::Create(
  IN    const   string&         spriteName, 
  IN    const   string&         storyboardName, 
  IN            HLayer          hLayer,
  IN    const   vec3&           position, 
  INOUT         HGameObject*    pHGameObject,
  IN    const   float           opacity,
  IN    const   Color&          color,
  IN            bool            hasShadow
  )
{
    RESULT rval = S_OK;
    
    HGameObject hGameObject;
    HStoryboard hStoryboard;

    CHR(GOMan.Create( 
                     "",                         // IN:  name for this GameObject
                     &hGameObject,               // OUT: handle
                     spriteName,                 // IN:  sprite name
                     "",                         // IN:  mesh name
                     "",                         // IN:  effect name
                     "",                         // IN:  behavior name
                     (GO_TYPE)(GO_TYPE_SPRITE),  // IN:  GO_TYPE
                     position,                   // IN:  world position
                     opacity,                    // IN:  opacity
                     color                       // IN:  color
                ));


    if (!hLayer.IsNull())
    {
        CHR(LayerMan.AddToLayer( hLayer, hGameObject ));
    }

    // Start a storyboard.
    if ("" != storyboardName)
    {
        CHR(StoryboardMan.GetCopy( storyboardName, &hStoryboard ));
        CHR(StoryboardMan.BindTo ( hStoryboard,     hGameObject ));
        CHR(StoryboardMan.Start  ( hStoryboard ));

        // Release the GO (Storyboard owns it now).
        // If the Storyboard is set to delete on finish, it will take the GO with it.
        // BUG: should this be INSIDE the storyboard test above?
        CHR(GOMan.Release( hGameObject ));
    }
    
    if (hasShadow)
    {
        CHR(SetShadow( hGameObject, true ));
    }

    // Release the GO (Storyboard owns it now).
    // If the Storyboard is set to delete on finish, it will take the GO with it.
    // BUG: should this be INSIDE the storyboard test above?
//    CHR(GOMan.Release( hGameObject ));
    
    
    if (pHGameObject)
    {
        *pHGameObject = hGameObject;
    }
    
Exit:
    return rval;
}



RESULT
GameObjectManager::Create( 
    IN      const   string&         name, 
    INOUT           HGameObject*    pHandle, 
    IN      const   string&         spriteName, 
    IN      const   string&         meshName, 
    IN      const   string&         effectName,
    IN      const   string&         behaviorName,
    IN              GO_TYPE         type,
    IN      const   vec3&           position,
    IN              float           opacity,
    IN      const   Color&          color,
    IN              bool            hasShadow
    )
{
    RESULT rval = S_OK;

    HGameObject             hGameObject;
    HSprite                 hSprite;
    HMesh                   hMesh;
    HEffect                 hEffect;
    StateMachineManager*    pStateMachineManager;


    //
    // Allocate Game Object
    //
    GameObject* pGameObject = new GameObject( type );
    CPREx(pGameObject, E_OUTOFMEMORY);
    
    if ("" == name) 
    {
        char randomName[MAX_NAME];
        sprintf(randomName, "GameObject_%X", (unsigned int)Platform::Random());

        CHR(pGameObject->Init ( randomName ));
    } 
    else
    {
        CHR(pGameObject->Init ( name ));
    }

    GOMan.Add( pGameObject, &hGameObject ); // TODO: release hGameObject if not returning it to caller below?


    //
    // Set Game Object properties
    //
    SpriteMan.GetCopy  ( spriteName,   &hSprite );
    MeshMan.GetCopy    ( meshName,     &hMesh   );
    EffectMan.Get      ( effectName,   &hEffect );        // Copying the Effect for each GO breaks sprite batching.  TODO: they should SHARE the shader, and SB keys off THAT.  But then Renderer::SetEffect() isn't useful
    
    
    pGameObject->SetSprite  ( hSprite   );
    pGameObject->SetMesh    ( hMesh     );
    pGameObject->SetEffect  ( hEffect   );
    pGameObject->SetPosition( position  );
    pGameObject->SetOpacity ( opacity   );
    pGameObject->SetScale   ( 1.0f      );
    pGameObject->SetColor   ( color     );

    SpriteMan.Release  ( hSprite );
    MeshMan.Release    ( hMesh   );
    EffectMan.Release  ( hEffect );

    if (hasShadow)
    {
        CHR(SetShadow( hGameObject, true ));
    }


    //
    // Set Behavior
    //
    if ("" != behaviorName)
    {
        pGameObject->CreateStateMachineManager();
        pStateMachineManager = pGameObject->GetStateMachineManager();
        CPREx(pStateMachineManager, E_OUTOFMEMORY);

        // Always place an IdleState at the bottom of each StateMachine queue,
        // so that if the last StateMachine is popped we won't crash.
        HBehavior hBehavior;
        BehaviorMan.GetCopy( "IdleState", &hBehavior );
        BehaviorMan.PushBehaviorOnToGameObject( hBehavior, hGameObject );
        
        // TODO: push an IdleState onto each queue.
        
        BehaviorMan.GetCopy( behaviorName, &hBehavior );
        BehaviorMan.PushBehaviorOnToGameObject( hBehavior, hGameObject );
    }


    if (pHandle)
    {
        *pHandle = hGameObject;
    }


Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::Create(name: \"%s\", sprite: \"%s\", mesh: \"%s\", effect: \"%s\", behavior: \"%s\") failed: 0x%x",
            name.c_str(), 
            spriteName.c_str(), 
            meshName.c_str(), 
            effectName.c_str(), 
            behaviorName.c_str(), 
            rval);
            
        // TODO: free the object and handle.
        DEBUGCHK(0);
    }
    else 
    {
        DEBUGMSG(ZONE_GAMEOBJECT, "GameObjectManager::Create( %4d, name: \"%-16s\" sprite: \"%-16s\" mesh: \"%-16s\" effect: \"%-16s\" behavior: \"%-16s\")",
            pGameObject->GetID(),
            name.c_str(), 
            spriteName.c_str(), 
            meshName.c_str(), 
            effectName.c_str(), 
            behaviorName.c_str())
    }

    
    return rval;
}



RESULT
GameObjectManager::CreateGameObject( IN Settings* pSettings, IN const string& settingsPath, INOUT GameObject** ppGameObject )
{
    RESULT rval = S_OK;
    string name;
    
    if (!pSettings || "" == settingsPath || !ppGameObject)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::CreateGameObject(): invalid argument");
        return E_INVALID_ARG;
    }
    
    GameObject *pGameObject = new GameObject();
    CPR(pGameObject);

    name = pSettings->GetString( settingsPath + ".Name" );
    pGameObject->Init( name, pSettings, settingsPath );

    // Caller must AddRef()
    *ppGameObject = pGameObject;
    
Exit:    
    return rval;
}



#pragma mark -
#pragma mark Operations

RESULT
GameObjectManager::Update( UINT64 elapsedMS, GO_TYPE objectTypesToUpdate )
{
    RESULT rval = S_OK;
    
    DEBUGMSG(ZONE_GAMEOBJECT | ZONE_VERBOSE, "Updating %d Game Objects", m_resourceList.size());
    
    //
    // Update all Game Objects
    //
    ResourceList::iterator ppGameObject;
    for (ppGameObject = m_resourceList.begin(); ppGameObject != m_resourceList.end(); ++ppGameObject)
    {
        GameObject* pGameObject = *ppGameObject;
        
        // Deleted GameObjects leave holes in the list, which are reused later.
        // Skip over them.
        if (!pGameObject)
        {
            continue;
        }    

        if (pGameObject->GetType() & objectTypesToUpdate)
        {
            pGameObject->Update( elapsedMS );
        }
    }        
    

    //
    // Deliver any messages that were just queued
    //
    MsgRouter.DeliverSlices();
	MsgRouter.DeliverDelayedMessages();
    
    
    // TODO: merge this into the Update loop above,
    // so we only traverse GameObjects ONCE per tick.
    // Does mean that GOs marked for deletion (as the result of a msg) 
    // won't be deleted until the next tick; is that safe?
    
    //
	// Destroy Game Objects that have requested it
    //
    ppGameObject = m_resourceList.begin();
    while ( ppGameObject != m_resourceList.end() )
	{
        GameObject* pGameObject = *ppGameObject;
        
        // Deleted GameObjects leave holes in the list, which are reused later.
        // Skip over them.
        if (!pGameObject)
        {
            ++ppGameObject;
            continue;
        }
        
		if( pGameObject->IsMarkedForDeletion() )
		{
            //DEBUGCHK( SUCCEEDED(ResourceManager<GameObject>::Remove(pGameObject)) );
            DEBUGCHK( SUCCEEDED(Remove(pGameObject)) );
            
            // TODO: POTENTIALLY UNSAFE IN FUTURE! 
            // Index into m_resourceList instead of iterate;
            // tolerates deletion just fine. For now, we never delete from m_resourceList.
		}

        ++ppGameObject;
	}
    
Exit:
    return rval;
}



RESULT
GameObjectManager::Draw( GO_TYPE objectTypesToDraw )
{
    RESULT rval         = S_OK;
    mat4   rootMatWorld = mat4::Identity();

    DEBUGMSG(ZONE_GAMEOBJECT | ZONE_VERBOSE, "Rendering %d Game Objects", m_resourceList.size());

    ResourceList::iterator ppGameObject;
    for (ppGameObject = m_resourceList.begin(); ppGameObject != m_resourceList.end(); ++ppGameObject)
    {
        GameObject* pGameObject = *ppGameObject;
        
        // Deleted GameObjects leave holes in the list, which are reused later.
        // Skip over them.
        if (!pGameObject)
        {
            continue;
        }
        
        
        if (pGameObject->GetType() & objectTypesToDraw)
        {
            pGameObject->Draw( rootMatWorld );
        }
    }        
    
Exit:
    return rval;
}



RESULT
GameObjectManager::Draw( HGameObject hGameObject, IN const mat4& matParentWorld )
{
    RESULT rval         = S_OK;
    
    DEBUGMSG(ZONE_GAMEOBJECT | ZONE_VERBOSE, "Rendering Game Object 0x%x", (UINT32)hGameObject);

    
    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (pGameObject)
    {
        pGameObject->Draw( matParentWorld );
    }
    else 
    {
        rval = E_FAIL;
    }

    
Exit:
    return rval;
}



RESULT
GameObjectManager::AddChild( IN HGameObject hGameObject, IN HSprite hChild )
{
    RESULT rval         = S_OK;
    
    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (pGameObject)
    {
        pGameObject->AddChild( hChild );
    }
    else 
    {
        rval = E_FAIL;
    }

    
Exit:
    return rval;
}



RESULT
GameObjectManager::AddChild( IN HGameObject hGameObject, IN HGameObject hChild )
{
    RESULT rval         = S_OK;
    
    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (pGameObject)
    {
        pGameObject->AddChild( hChild );
    }
    else 
    {
        rval = E_FAIL;
    }

    
Exit:
    return rval;
}



RESULT
GameObjectManager::AddChild( IN HGameObject hGameObject, IN HParticleEmitter hChild )
{
    RESULT rval         = S_OK;
    
    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (pGameObject)
    {
        pGameObject->AddChild( hChild );
    }
    else 
    {
        rval = E_FAIL;
    }

    
Exit:
    return rval;
}



RESULT
GameObjectManager::GetChildren( IN HGameObject hGameObject, INOUT HGameObjectList* pList  )
{
    RESULT                  rval            = S_OK;
    GameObject*             pGameObject;
    
    if (!pList)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::GetChildren( 0x%x, NULL )", (UINT32)hGameObject );
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    pGameObject = GetObjectPointer( hGameObject );
    if (!pGameObject)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::GetChildren( 0x%x, 0x%x ): bad handle", (UINT32)hGameObject, pList);
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    CHR(pGameObject->GetChildren( pList ));

Exit:
    return rval;
}



RESULT
GameObjectManager::GetChildren( IN HGameObject hGameObject, INOUT HSpriteList* pList  )
{
    RESULT                  rval            = S_OK;
    GameObject*             pGameObject;
    
    if (!pList)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::GetChildren( 0x%x, NULL )", (UINT32)hGameObject );
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    pGameObject = GetObjectPointer( hGameObject );
    if (!pGameObject)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::GetChildren( 0x%x, 0x%x ): bad handle", (UINT32)hGameObject, pList);
        rval = E_BAD_HANDLE;
        goto Exit;
    }


    CHR(pGameObject->GetChildren( pList ));

Exit:
    return rval;
}



RESULT
GameObjectManager::GetChildren( IN HGameObject hGameObject, INOUT HParticleEmitterList* pList  )
{
    RESULT                          rval            = S_OK;
    GameObject*                     pGameObject;
    
    if (!pList)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::GetChildren( 0x%x, NULL )", (UINT32)hGameObject );
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    pGameObject = GetObjectPointer( hGameObject );
    if (!pGameObject)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::GetChildren( 0x%x, 0x%x ): bad handle", (UINT32)hGameObject, pList);
        rval = E_BAD_HANDLE;
        goto Exit;
    }


    CHR(pGameObject->GetChildren( pList ));

Exit:
    return rval;
}



GO_TYPE
GameObjectManager::GetType( IN HGameObject hGameObject )
{
    GO_TYPE rval = GO_TYPE_UNKNOWN;

    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (!pGameObject)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::GetType( 0x%x ): bad handle", (UINT32)hGameObject);
        goto Exit;
    }
    
    rval = pGameObject->GetType();
    
Exit:
    return rval;
}



RESULT
GameObjectManager::GetGameObjectPointer( IN HGameObject handle, INOUT GameObject** ppGameObject )
{
    RESULT      rval        = S_OK;
    GameObject* pGameObject = NULL;
    
    if (!ppGameObject)
    {
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    
    pGameObject = GetObjectPointer( handle );
    if (!pGameObject)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::GetGameObjectPointer( 0x%x ): bad handle", (UINT32)handle);
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    
    // We have NOT increased the GO's refcount!
    // Caller already holds a ref via the handle.
    //
    // Any access via the pointer is NOT thread-safe!
    *ppGameObject = pGameObject;
    
Exit:
    return rval;
}



RESULT
GameObjectManager::GetGameObjectPointer( IN OBJECT_ID objectID, INOUT GameObject** ppGameObject )
{
    RESULT      rval        = S_OK;
    GameObject* pGameObject = NULL;
    
    if (!ppGameObject)
    {
        rval = E_NULL_POINTER;
        goto Exit;
    }
    

    //
    // Lookup the GameObject by ID
    // 
    {
        GOIDToGameObjectMapIterator ppGameObject;
        ppGameObject = m_GOIDToGameObjectMap.find( objectID );
        
        if (ppGameObject == m_GOIDToGameObjectMap.end())
        {
            RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::GetGameObjectPointer(): objectID %d not found", objectID);
            rval = E_INVALID_ARG;
            goto Exit;
        }

        pGameObject = ppGameObject->second;
        if (!pGameObject)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::GetGameObjectPointer(): bad handle");
            rval = E_BAD_HANDLE;
            goto Exit;
        }
    }
    
    // We have NOT increased the GO's refcount!
    // The returned pointer could become invalid/deleted at any time!
    // Any access via the pointer is NOT thread-safe!
    *ppGameObject = pGameObject;
    
Exit:
    return rval;
}



RESULT
GameObjectManager::GetList( IN GO_TYPE type, INOUT GameObjectList* pList )
{
    RESULT               rval = S_OK;
    ResourceListIterator ppGameObject;
    
    if (!pList)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::GetList( %d, NULL )", type );
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    for( ppGameObject = m_resourceList.begin(); ppGameObject != m_resourceList.end(); ++ppGameObject )
    {
        GameObject* pGameObject = *ppGameObject;
        
        if (pGameObject && pGameObject->GetType() & type)
        {
            pList->push_back( pGameObject );
        }
    }
    
Exit:
    return rval;
}



#pragma mark -
#pragma mark IDrawable
// IDrawable
RESULT
GameObjectManager::SetVisible( IN HGameObject hGameObject, bool isVisible )
{
    RESULT rval = S_OK;
    
    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (pGameObject)
    {
        DEBUGMSG(ZONE_GAMEOBJECT | ZONE_VERBOSE, "GameObjectManager::SetVisible( %s, %d )", 
                 pGameObject->GetName().c_str(), isVisible);
        
        pGameObject->SetVisible( isVisible );
    }
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::SetVisible( 0x%x ): object not found", (UINT32)hGameObject);
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}




RESULT
GameObjectManager::SetPosition( IN HGameObject hGameObject, const vec3& vPos )
{
    RESULT rval = S_OK;
    
    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (pGameObject)
    {
        DEBUGMSG(ZONE_GAMEOBJECT | ZONE_VERBOSE, "GameObjectManager::SetPosition( %s, (%d, %d, %d) )", 
                 pGameObject->GetName().c_str(), vPos.x, vPos.y, vPos.z);
        
        pGameObject->SetPosition( vPos );
    }
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::SetPosition( 0x%x ): object not found", (UINT32)hGameObject);
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}



RESULT
GameObjectManager::SetRotation( IN HGameObject hGameObject, const vec3& vRotationDegrees )
{
    RESULT rval = S_OK;
    
    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (pGameObject)
    {
        DEBUGMSG(ZONE_GAMEOBJECT | ZONE_VERBOSE, "GameObjectManager::SetRotation( %s, (%d, %d, %d) )", 
                 pGameObject->GetName().c_str(), vRotationDegrees.x, vRotationDegrees.y, vRotationDegrees.z);
        
        pGameObject->SetRotation( vRotationDegrees );
    }
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::SetRotation( 0x%x ): object not found", (UINT32)hGameObject);
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}



RESULT
GameObjectManager::SetScale( IN HGameObject hGameObject, float scale )
{
    RESULT rval = S_OK;
    
    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (pGameObject)
    {
        DEBUGMSG(ZONE_GAMEOBJECT | ZONE_VERBOSE, "GameObjectManager::SetScale( %s, %4.4f )", 
                 pGameObject->GetName().c_str(), scale);
        
        pGameObject->SetScale( scale );
    }
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::SetScale( 0x%x ): object not found", (UINT32)hGameObject);
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}



RESULT
GameObjectManager::SetOpacity( IN HGameObject hGameObject, float opacity )
{
    RESULT rval = S_OK;
    
    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (pGameObject)
    {
        DEBUGMSG(ZONE_GAMEOBJECT | ZONE_VERBOSE, "GameObjectManager::SetOpacity( %s, %4.4f )", 
                 pGameObject->GetName().c_str(), opacity);
        
        pGameObject->SetOpacity( opacity );
    }
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::SetOpacity( 0x%x ): object not found", (UINT32)hGameObject);
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}



RESULT
GameObjectManager::SetEffect( IN HGameObject hGameObject, HEffect hEffect )
{
    RESULT rval = S_OK;
    
    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (pGameObject)
    {
#ifdef DEBUG    
        string name;
        EffectMan.GetName( hEffect, &name );
        DEBUGMSG(ZONE_GAMEOBJECT | ZONE_VERBOSE, "GameObjectManager::SetEffect( %s, %s )", 
                 pGameObject->GetName().c_str(), name.c_str());
#endif        
        pGameObject->SetEffect( hEffect );
    }
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::SetEffect( 0x%x ): object not found", (UINT32)hGameObject);
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}




RESULT
GameObjectManager::SetColor( IN HGameObject hGameObject, const Color& color )
{
    RESULT rval = S_OK;
    
    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (pGameObject)
    {
        pGameObject->SetColor( color );
    }
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::SetColor( 0x%x ): object not found", (UINT32)hGameObject);
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}



RESULT
GameObjectManager::SetShadow( IN HGameObject hGameObject, bool hasShadow )
{
    RESULT rval = S_OK;
    
    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (pGameObject)
    {
        DEBUGMSG(ZONE_GAMEOBJECT | ZONE_VERBOSE, "GameObjectManager::SetShadow( %s, %d )", 
                 pGameObject->GetName().c_str(), hasShadow);
        
        pGameObject->SetShadow( hasShadow );
    }
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::SetShadow( 0x%x ): object not found", (UINT32)hGameObject);
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}





bool
GameObjectManager::GetVisible( IN HGameObject hGameObject )
{
    bool rval = false;

    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (pGameObject)
    {
        DEBUGMSG(ZONE_GAMEOBJECT | ZONE_VERBOSE, "GameObjectManager::GetVisible( %s )", 
                 pGameObject->GetName().c_str());
        
        rval = pGameObject->GetVisible();
    }

Exit:
    return rval;
}



vec3
GameObjectManager::GetPosition( IN HGameObject hGameObject )
{
    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (pGameObject)
    {
        DEBUGMSG(ZONE_GAMEOBJECT | ZONE_VERBOSE, "GameObjectManager::GetPosition( %s )", 
                 pGameObject->GetName().c_str());
        
        return pGameObject->GetPosition();
    }
    else 
    {
        return vec3(0,0,0);
    }
}



vec3
GameObjectManager::GetRotation( IN HGameObject hGameObject )
{
    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (pGameObject)
    {
        DEBUGMSG(ZONE_GAMEOBJECT | ZONE_VERBOSE, "GameObjectManager::GetRotation( %s )", 
                 pGameObject->GetName().c_str());
        
        return pGameObject->GetRotation();
    }
    else 
    {
        return vec3(0,0,0);
    }
}



float
GameObjectManager::GetScale( IN HGameObject hGameObject )
{
    float rval = 0.0;

    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (pGameObject)
    {
        DEBUGMSG(ZONE_GAMEOBJECT | ZONE_VERBOSE, "GameObjectManager::GetScale( %s )", 
                 pGameObject->GetName().c_str());
        
        rval = pGameObject->GetScale();
    }

Exit:
    return rval;
}



float
GameObjectManager::GetOpacity( IN HGameObject hGameObject )
{
    float rval = 0.0;

    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (pGameObject)
    {
        DEBUGMSG(ZONE_GAMEOBJECT | ZONE_VERBOSE, "GameObjectManager::GetOpacity( %s )", 
                 pGameObject->GetName().c_str());
        
        rval = pGameObject->GetOpacity();
    }

Exit:
    return rval;
}

    

HEffect
GameObjectManager::GetEffect( IN HGameObject hGameObject )
{
    HEffect rval;

    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (pGameObject)
    {
        DEBUGMSG(ZONE_GAMEOBJECT | ZONE_VERBOSE, "GameObjectManager::GetEffect( %s )", 
                 pGameObject->GetName().c_str());
        
        rval = pGameObject->GetEffect();
    }

Exit:
    return rval;
}

    

AABB
GameObjectManager::GetBounds( IN HGameObject hGameObject )
{

    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (pGameObject)
    {
        DEBUGMSG(ZONE_GAMEOBJECT | ZONE_VERBOSE, "GameObjectManager::GetBounds( %s )", 
                 pGameObject->GetName().c_str());
        
        return pGameObject->GetBounds();
    }
    else 
    {
        return AABB();  // 0,0,0
    }
}



bool
GameObjectManager::GetShadow( IN HGameObject hGameObject )
{
    bool rval = false;

    GameObject* pGameObject = GetObjectPointer( hGameObject );
    if (pGameObject)
    {
        DEBUGMSG(ZONE_GAMEOBJECT | ZONE_VERBOSE, "GameObjectManager::GetShadow( %s )", 
                 pGameObject->GetName().c_str());
        
        rval = pGameObject->GetShadow();
    }

Exit:
    return rval;
}




#pragma mark -
#pragma mark Message Passing


/*---------------------------------------------------------------------------*
 Name:         SendMessageFromSystem
 
 Description:  Send a message from the system to a game object. For example,
 a keystroke or event can be sent in the form of a message.
 
 Arguments:    
 id   : the ID of the object
 name : the name of the message
 
 Returns:      None.
 *---------------------------------------------------------------------------*/
RESULT
GameObjectManager::SendMessageFromSystem( OBJECT_ID id, MSG_Name name)
{
    RESULT rval = S_OK;
    
	GameObject* pGameObject;
    CHR(GetGameObjectPointer( id, &pGameObject ));
    
	if( pGameObject )
	{
		MSG_Data data;
		MSG_Object msg( 0.0f, name, SYSTEM_OBJECT_ID, id, SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, data, false, false );
		if(pGameObject->GetStateMachineManager())
		{
            pGameObject->GetStateMachineManager()->SendMsg( msg );
		}
	}
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::SendMessageFromSystem( %d, %d ): OBJECT_ID not found; has Object been deleted?", id, name);
        rval = E_NOT_FOUND;
    }

    
Exit:
    return rval;
}



/*---------------------------------------------------------------------------*
 Name:         SendMessageFromSystem
 
 Description:  Send a message from the system to a game object. For example,
 a keystroke or event can be sent in the form of a message.
 
 Arguments:    
 HGameObject    : the Handle to the object
 name           : the name of the message
 
 Returns:      None.
 *---------------------------------------------------------------------------*/
RESULT
GameObjectManager::SendMessageFromSystem( HGameObject hGameObject, MSG_Name name)
{
    RESULT rval = S_OK;
    
	GameObject* pGameObject;
    CHR(GetGameObjectPointer( hGameObject, &pGameObject ));
    
	if( pGameObject )
	{
		MSG_Data data;
		MSG_Object msg( 0.0f, name, SYSTEM_OBJECT_ID, pGameObject->GetID(), SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, data, false, false );
		if(pGameObject->GetStateMachineManager())
		{
			pGameObject->GetStateMachineManager()->SendMsg( msg );
		}
	}
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::SendMessageFromSystem( 0x%x, %d ): handle not found; has GameObject been deleted?", (UINT32)hGameObject, name);
        rval = E_NOT_FOUND;
    }
    
Exit:
    return rval;
}



/*---------------------------------------------------------------------------*
 Name:         SendMessageFromSystem
 
 Description:  Send a message from the system to a game object. For example,
 a keystroke or event can be sent in the form of a message.
 
 Arguments:    
 object : pointer to the game object
 name : the name of the message
 
 Returns:      None.
 *---------------------------------------------------------------------------*/
RESULT
GameObjectManager::SendMessageFromSystem( GameObject* pGameObject, MSG_Name name)
{
    RESULT rval = S_OK;
    
	if( pGameObject )
	{
		MSG_Data data;
		MSG_Object msg( 0.0f, name, SYSTEM_OBJECT_ID, pGameObject->GetID(), SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, data, false, false );
		if(pGameObject->GetStateMachineManager())
		{
			pGameObject->GetStateMachineManager()->SendMsg( msg );
		}
	}
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::SendMessageFromSystem( 0x%x, %d ): pGameObject is NULL", pGameObject, name);
        rval = E_NULL_POINTER;
    }
    
Exit:
    return rval;
}



/*---------------------------------------------------------------------------*
 Name:         SendMessageFromSystem
 
 Description:  Send a message from the system to all game objects. For example,
 a keystroke or event can be sent in the form of a message.
 
 Arguments:    
 name : the name of the message
 data : data to be delivered with the message
 
 Returns:      None.
 *---------------------------------------------------------------------------*/
/*
RESULT
GameObjectManager::SendMessageFromSystem( MSG_Name name, float data )
{
    RESULT rval = S_OK;
  
    ResourceListIterator ppGameObject;
    for ( ppGameObject = m_resourceList.begin(); ppGameObject != m_resourceList.end(); ++ppGameObject )
	{
        GameObject* pGameObject = *ppGameObject;
        
		MSG_Data dataObject;
		dataObject.SetFloat( data );
		MSG_Object msg( 0.0f, name, SYSTEM_OBJECT_ID, pGameObject->GetID(), SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, dataObject, false, false );
		if(pGameObject->GetStateMachineManager())
		{
			pGameObject->GetStateMachineManager()->SendMsg( msg );
		}
	}
    
Exit:
    return rval;
}
*/


/*---------------------------------------------------------------------------*
 Name:         SendMessageFromSystem
 
 Description:  Send a message from the system to all game objects. For example,
 a keystroke or event can be sent in the form of a message.
 
 Arguments:    
 name : the name of the message
 
 Returns:      None.
 *---------------------------------------------------------------------------*/
RESULT
GameObjectManager::SendMessageFromSystem( MSG_Name name)
{
    RESULT rval = S_OK;
    
    // Use an index instead of iterator because SendMsg() may cause m_resourceList.push_back(),
    // which could re-allocate the vector and trash the iterator. Index is always safe.
    //
    // TODO: do we want to CACHE m_resourceList.size(), and thus avoid sending the message
    // to any GOs created WHILE iterator the list?
    for ( int index = 0; index < m_resourceList.size(); ++index )
	{
        GameObject* pGameObject = m_resourceList[ index ];
        
        // Deleted GameObjects leave holes in the list, which are reused later.
        // Skip over them.
        if (!pGameObject)
        {
            continue;
        }    

        
		MSG_Data data;
		MSG_Object msg( 0.0f, name, SYSTEM_OBJECT_ID, pGameObject->GetID(), SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, data, false, false );
		if(pGameObject->GetStateMachineManager())
		{
			pGameObject->GetStateMachineManager()->SendMsg( msg );
		}
	}

    
Exit:
    return rval;
}



/*---------------------------------------------------------------------------*
 Name:         SendMessageFromSystem
 
 Description:  Send a message from the system to a game object. For example,
 a keystroke or event can be sent in the form of a message.
 
 Arguments:    
 HGameObject : Handle to the game object
 name        : the name of the message
 pData       : pointer to message data
 
 Returns:      None.
 *---------------------------------------------------------------------------*/
RESULT
GameObjectManager::SendMessageFromSystem( HGameObject hGameObject, MSG_Name name, void* pData)
{
    RESULT rval = S_OK;

	GameObject* pGameObject;
    CHR(GetGameObjectPointer( hGameObject, &pGameObject ));
    
	if( pGameObject )
	{
		MSG_Data data;
        data.SetPointer( pData );
		MSG_Object msg( 0.0f, name, SYSTEM_OBJECT_ID, pGameObject->GetID(), SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, data, false, false );
		if(pGameObject->GetStateMachineManager())
		{
			pGameObject->GetStateMachineManager()->SendMsg( msg );
		}
	}
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::SendMessageFromSystem( 0x%x, %d ): handle not found; has GameObject been deleted?", (UINT32)hGameObject, name);
        rval = E_NOT_FOUND;
    }
    
Exit:
    return rval;
}



/*---------------------------------------------------------------------------*
 Name:         SendMessageFromSystem
 
 Description:  Send a message from the system to a game object. For example,
 a keystroke or event can be sent in the form of a message.
 
 Arguments:    
 objectID : ID of the game object
 name     : the name of the message
 pData    : pointer to message data
 
 Returns:      None.
 *---------------------------------------------------------------------------*/
RESULT
GameObjectManager::SendMessageFromSystem( OBJECT_ID id, MSG_Name name, void* pData)
{
    RESULT rval = S_OK;

	GameObject* pGameObject;
    CHR(GetGameObjectPointer( id, &pGameObject ));
    
	if( pGameObject )
	{
		MSG_Data data;
        data.SetPointer( pData );
		MSG_Object msg( 0.0f, name, SYSTEM_OBJECT_ID, pGameObject->GetID(), SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, data, false, false );
		if(pGameObject->GetStateMachineManager())
		{
			pGameObject->GetStateMachineManager()->SendMsg( msg );
		}
	}
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::SendMessageFromSystem( %d, %d ): OBJECT_ID not found; has Object been deleted?", id, name);
        rval = E_NOT_FOUND;
    }
    
Exit:
    return rval;
}



/*---------------------------------------------------------------------------*
 Name:         SendMessageFromSystem
 
 Description:  Send a message from the system to a game object. For example,
 a keystroke or event can be sent in the form of a message.
 
 Arguments:    
 object : pointer to the game object
 name   : the name of the message
 fData  : float message data
 
 Returns:      None.
 *---------------------------------------------------------------------------*/
/*
RESULT
GameObjectManager::SendMessageFromSystem( GameObject* pGameObject, MSG_Name name, float fData )
{
    RESULT rval = S_OK;
    
	if( pGameObject )
	{
		MSG_Data data;
        data.SetFloat( fData );
		MSG_Object msg( 0.0f, name, SYSTEM_OBJECT_ID, pGameObject->GetID(), SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, data, false, false );
		if(pGameObject->GetStateMachineManager())
		{
			pGameObject->GetStateMachineManager()->SendMsg( msg );
		}
	}

Exit:
    return rval;
}
*/


/*---------------------------------------------------------------------------*
 Name:         SendMessageFromSystem
 
 Description:  Send a message from the system to a game object. For example,
 a keystroke or event can be sent in the form of a message.
 
 Arguments:    
 objectID : ID of the game object
 name     : the name of the message
 fData    : float message data
 
 Returns:      None.
 *---------------------------------------------------------------------------*/
/*
RESULT
GameObjectManager::SendMessageFromSystem( OBJECT_ID id, MSG_Name name, float fData )
{
    RESULT rval = S_OK;
    
	GameObject* pGameObject;
    CHR(GetGameObjectPointer( id, &pGameObject ));
    
	if( pGameObject )
	{
		MSG_Data data;
        data.SetFloat( fData );
		MSG_Object msg( 0.0f, name, SYSTEM_OBJECT_ID, pGameObject->GetID(), SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, data, false, false );
		if(pGameObject->GetStateMachineManager())
		{
			pGameObject->GetStateMachineManager()->SendMsg( msg );
		}
	}

Exit:
    return rval;
}
*/


/*---------------------------------------------------------------------------*
 Name:         SendMessageFromSystem
 
 Description:  Send a message from the system to a game object. For example,
 a keystroke or event can be sent in the form of a message.
 
 Arguments:    
 object : pointer to the game object
 name   : the name of the message
 iData  : int message data
 
 Returns:      None.
 *---------------------------------------------------------------------------*/
/*
RESULT
GameObjectManager::SendMessageFromSystem( GameObject* pGameObject, MSG_Name name, int iData )
{
    RESULT rval = S_OK;
    
	if( pGameObject )
	{
		MSG_Data data;
        data.SetInt( iData );
		MSG_Object msg( 0.0f, name, SYSTEM_OBJECT_ID, pGameObject->GetID(), SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, data, false, false );
		if(pGameObject->GetStateMachineManager())
		{
			pGameObject->GetStateMachineManager()->SendMsg( msg );
		}
	}

Exit:
    return rval;
}
*/




/*---------------------------------------------------------------------------*
 Name:         SendMessageFromSystem
 
 Description:  Send a message from the system to a game object. For example,
 a keystroke or event can be sent in the form of a message.
 
 Arguments:    
 objectID : ID of the game object
 name     : the name of the message
 iData    : int message data
 
 Returns:      None.
 *---------------------------------------------------------------------------*/
/*
RESULT
GameObjectManager::SendMessageFromSystem( objectID id, MSG_Name name, int iData )
{
    RESULT rval = S_OK;
    
	GameObject* pGameObject;
    CHR(GetGameObjectPointer( id, &pGameObject ));
        
	if( pGameObject )
	{
		MSG_Data data;
        data.SetInt( iData );
		MSG_Object msg( 0.0f, name, SYSTEM_OBJECT_ID, pGameObject->GetID(), SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, data, false, false );
		if(pGameObject->GetStateMachineManager())
		{
			pGameObject->GetStateMachineManager()->SendMsg( msg );
		}
	}
    
Exit:
    return rval;
}
*/


/*---------------------------------------------------------------------------*
 Name:         SendMessageFromSystem
 
 Description:  Send a message from the system to a game object. For example,
 a keystroke or event can be sent in the form of a message.
 
 Arguments:    
 object : pointer to the game object
 name   : the name of the message
 pData  : pointer to message data
 
 Returns:      None.
 *---------------------------------------------------------------------------*/
RESULT
GameObjectManager::SendMessageFromSystem( GameObject* pGameObject, MSG_Name name, void* pData)
{
    RESULT rval = S_OK;
    
	if( pGameObject )
	{
		MSG_Data data;
        data.SetPointer( pData );
		MSG_Object msg( 0.0f, name, SYSTEM_OBJECT_ID, pGameObject->GetID(), SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, data, false, false );
		if(pGameObject->GetStateMachineManager())
		{
			pGameObject->GetStateMachineManager()->SendMsg( msg );
		}
	}
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::SendMessageFromSystem( 0x%x, %d ): pGameObject is NULL", pGameObject, name);
        rval = E_NOT_FOUND;
    }
    
Exit:
    return rval;
}



/*---------------------------------------------------------------------------*
 Name:         SendMessageFromSystem
 
 Description:  Send a message from the system to a game object. For example,
 a keystroke or event can be sent in the form of a message.
 
 Arguments:    
 name   : the name of the message
 pData  : pointer to message data
 
 Returns:      None.
 *---------------------------------------------------------------------------*/
RESULT
GameObjectManager::SendMessageFromSystem( MSG_Name name, void* pData)
{
    RESULT rval = S_OK;
    
    // Use an index instead of iterator because SendMsg() may cause m_resourceList.push_back(),
    // which could re-allocate the vector and trash the iterator. Index is always safe.
    //
    // TODO: do we want to CACHE m_resourceList.size(), and thus avoid sending the message
    // to any GOs created WHILE iterator the list?
    for ( int index = 0; index < m_resourceList.size(); ++index )
	{
        GameObject* pGameObject = m_resourceList[ index ];
        
        // Deleted GameObjects leave holes in the list, which are reused later.
        // Skip over them.
        if (!pGameObject)
        {
            continue;
        }    

		MSG_Data data;
        data.SetPointer( pData );
		MSG_Object msg( 0.0f, name, SYSTEM_OBJECT_ID, pGameObject->GetID(), SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, data, false, false );
		if(pGameObject->GetStateMachineManager())
		{
			pGameObject->GetStateMachineManager()->SendMsg( msg );
		}
	}
    
Exit:
    return rval;
}


/*
//---------------------------------------------------------------------------
// Name:         SendMessageFromSystem
// 
// Description:  Send a message from the system to a game object. For example,
// a keystroke or event can be sent in the form of a message.
// 
// Arguments:    
// HGameObject : Handle to the game object
// name        : the name of the message
// pData       : pointer to message data
// 
// Returns:      None.
//---------------------------------------------------------------------------
RESULT
GameObjectManager::SendMessageFromSystem( HGameObject hGameObject, MSG_Name name, const HGameObject& handle )
{
    RESULT rval = S_OK;

	GameObject* pGameObject;
    CHR(GetGameObjectPointer( hGameObject, &pGameObject ));
    
	if( pGameObject )
	{
		MSG_Data data;
        data.SetHandle( handle );
		MSG_Object msg( 0.0f, name, SYSTEM_OBJECT_ID, pGameObject->GetID(), SCOPE_TO_STATE_MACHINE, 0, STATE_MACHINE_QUEUE_ALL, data, false, false );
		if(pGameObject->GetStateMachineManager())
		{
			pGameObject->GetStateMachineManager()->SendMsg( msg );
		}
	}
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObjectManager::SendMessageFromSystem( 0x%x, %d ): handle not found; has GameObject been deleted?", (UINT32)hGameObject, name);
        rval = E_NOT_FOUND;
    }
    
Exit:
    return rval;
}
*/




} // END namespace Z

