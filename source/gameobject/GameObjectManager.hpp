#pragma once

#include "ResourceManager.hpp"
#include "Handle.hpp"
#include "Object.hpp"
#include "Settings.hpp"
#include "GameObject.hpp"
#include "Layer.hpp"
#include "msg.hpp"

#include <list>
#include <set>
using std::list;
using std::set;

namespace Z
{



typedef list<GameObject*>        GameObjectList;
typedef GameObjectList::iterator GameObjectListIterator;
typedef set<GameObject*>         GameObjectSet;
typedef GameObjectSet::iterator  GameObjectSetIterator;

typedef list<HGameObject>         HGameObjectList;
typedef HGameObjectList::iterator HGameObjectListIterator;
typedef set<HGameObject>          HGameObjectSet;
typedef HGameObjectSet::iterator  HGameObjectSetIterator;


class GameObjectManager : public ResourceManager<GameObject>
{
public:
    static  GameObjectManager& Instance();
    
    virtual RESULT      Init            ( IN const string& settingsFilename );
    virtual RESULT      Add             ( IN GameObject* pGameObject, INOUT HGameObject* pHandle = NULL );
    virtual RESULT      Add             ( IN const string& name, IN GameObject* pGameObject, INOUT HGameObject* pHandle = NULL );
    virtual RESULT      Remove          ( IN HGameObject handle );
    virtual RESULT      Remove          ( IN GameObject* pGameObject, IN HGameObject handle = HGameObject::NullHandle() );
    virtual RESULT      Release         ( IN HGameObject handle );
    virtual RESULT      Shutdown        ( );


    RESULT              Create          ( 
                                          IN    const   string&         spriteName, 
                                          IN    const   string&         storyboardName  = "", 
                                          IN            HLayer          hLayer          = HLayer::NullHandle(),
                                          IN    const   vec3&           position        = vec3(0,0,0), 
                                          INOUT         HGameObject*    pHGameObject    = NULL,
                                          IN    const   float           opacity         = 1.0f,
                                          IN    const   Color&          color           = Color::White(),
                                          IN            bool            hasShadow       = false
                                        );


    RESULT              Create          (
                                          IN      const   string&         name, 
                                          INOUT           HGameObject*    pHandle       = NULL, 
                                          IN      const   string&         spriteName    = "", 
                                          IN      const   string&         meshName      = "", 
                                          IN      const   string&         effectName    = "",
                                          IN      const   string&         behaviorName  = "",
                                          IN              GO_TYPE         type          = GO_TYPE_UNKNOWN,
                                          IN      const   vec3&           position      = vec3(0,0,0),
                                          IN              float           opacity       = 1.0,
                                          IN      const   Color&          color         = Color::White(),
                                          IN              bool            hasShadow     = false
                                        );


    RESULT              Update          ( UINT64 elapsedMS, GO_TYPE objectTypesToUpdate = GO_TYPE_ANY );
    
    RESULT              Draw            ( GO_TYPE objectTypesToDraw = GO_TYPE_ANY );
    RESULT              Draw            ( IN HGameObject hGameObject, IN const mat4& matParentWorld = mat4::Identity() );

   
    RESULT              AddChild        ( IN HGameObject hGameObject, IN HSprite          hChild        );
    RESULT              AddChild        ( IN HGameObject hGameObject, IN HGameObject      hChild        );
    RESULT              AddChild        ( IN HGameObject hGameObject, IN HParticleEmitter hChild        );
    
    RESULT              GetChildren     ( IN HGameObject hGameObject, INOUT HGameObjectList*        pList );
    RESULT              GetChildren     ( IN HGameObject hGameObject, INOUT HSpriteList*            pList );
    RESULT              GetChildren     ( IN HGameObject hGameObject, INOUT HParticleEmitterList*   pList );


    GO_TYPE             GetType         ( IN HGameObject hGameObject );


  	// IDrawable
    // TODO: are handles worth the annoyance of having to expose every object method like this?!
    RESULT              SetVisible      ( IN HGameObject hGameObject, bool        isVisible        );
    RESULT              SetPosition     ( IN HGameObject hGameObject, const vec3& vPos             );
    RESULT              SetRotation     ( IN HGameObject hGameObject, const vec3& vRotationDegrees );
    RESULT              SetScale        ( IN HGameObject hGameObject, float       scale            );
    RESULT              SetOpacity      ( IN HGameObject hGameObject, float       opacity          );
    RESULT              SetEffect       ( IN HGameObject hGameObject, HEffect     hEffect          );
    RESULT              SetColor        ( IN HGameObject hGameObject, const Color& color           );
    RESULT              SetShadow       ( IN HGameObject hGameObject, bool        hasShadow        );
    
    bool                GetVisible      ( IN HGameObject hGameObject                               );
    vec3                GetPosition     ( IN HGameObject hGameObject                               );
    vec3                GetRotation     ( IN HGameObject hGameObject                               );
    float               GetScale        ( IN HGameObject hGameObject                               );
    float               GetOpacity      ( IN HGameObject hGameObject                               );
    HEffect             GetEffect       ( IN HGameObject hGameObject                               );
    AABB                GetBounds       ( IN HGameObject hGameObject                               );
    bool                GetShadow       ( IN HGameObject hGameObject                               );


    // GameObject is the one case where we  allow 
    // callers to fetch the object itself, 
    // since we often want to manipulate GOs directly.
    // The state machine managers do this heavily.
    //
    // TODO: rewrite the state machine and messaging code from scratch.
    //
    // DANGER, DANGER WILL ROBINSON: concurrent access!
    //
    RESULT              GetGameObjectPointer( IN HGameObject handle,   INOUT GameObject** ppGameObject );
    RESULT              GetGameObjectPointer( IN OBJECT_ID   objectID, INOUT GameObject** ppGameObject );
    
    // TODO: queries.  Find GOs within radius of vPos, find GOs of type x, GOs within viewing frustum, etc.
    RESULT              GetList         ( IN GO_TYPE type, INOUT GameObjectList* pList );
    
    //
    // Messaging
    // TODO: extend this to support delivery over sockets, to unify messaging locally and client/server.
    //
    
    // Send message w/o payload
    RESULT              SendMessageFromSystem( HGameObject hGameObject, MSG_Name name );
    RESULT              SendMessageFromSystem( OBJECT_ID id, MSG_Name name );
    RESULT              SendMessageFromSystem( IN GameObject* pGameObject, MSG_Name name );
    RESULT              SendMessageFromSystem( MSG_Name name );

    // Send message with void* payload
    RESULT              SendMessageFromSystem( HGameObject hGameObject, MSG_Name name, IN void* pData );
    RESULT              SendMessageFromSystem( OBJECT_ID id, MSG_Name name, IN void* pData );
    RESULT              SendMessageFromSystem( IN GameObject* pGameObject, MSG_Name name, IN void* pData );
    RESULT              SendMessageFromSystem( MSG_Name name, IN void* pData );

protected:
    GameObjectManager();
    GameObjectManager( const GameObjectManager& rhs );
    GameObjectManager& operator=( const GameObjectManager& rhs );
    virtual ~GameObjectManager();
 
    RESULT  CreateGameObject( IN Settings* pSettings, IN const string& settingsPath, INOUT GameObject** ppGameObject );
    
protected:
    typedef map< OBJECT_ID, GameObject* >   GOIDToGameObjectMap;
    typedef GOIDToGameObjectMap::iterator   GOIDToGameObjectMapIterator;
    
    GOIDToGameObjectMap m_GOIDToGameObjectMap;
};

#define GOMan ((GameObjectManager&)GameObjectManager::Instance())


} // END namespace Z

