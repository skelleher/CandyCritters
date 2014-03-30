#pragma once

#include "Types.hpp"
#include "Handle.hpp"
#include "IDrawable.hpp"
#include "MeshManager.hpp"
#include "SpriteManager.hpp"
#include "EffectManager.hpp"
#include "ParticleManager.hpp"
#include "Property.hpp"

#include <list>
using std::list;


namespace Z
{


    
// Forward declarations
class Storyboard;
typedef Handle<Storyboard> HStoryboard;
class StateMachine;
class StateMachineManager;

class GameObject;
typedef Handle<GameObject>              HGameObject;
typedef list<HGameObject>               HGameObjectList;
typedef HGameObjectList::iterator       HGameObjectListIterator;
typedef Property<GameObject>            GameObjectProperty;

typedef list<HParticleEmitter>          HParticleEmitterList;
typedef HParticleEmitterList::iterator  HParticleEmitterListIterator;


//
// GameObject types.
// Can be combined - a GO could be both a Sprite and an Actor
//
typedef enum
{
    GO_TYPE_UNKNOWN     = BIT0,
    GO_TYPE_SPRITE      = BIT1,
    GO_TYPE_TRIGGER     = BIT2,
    GO_TYPE_UI          = BIT3,
    GO_TYPE_ACTOR       = BIT4,
    GO_TYPE_PROJECTILE  = BIT5,
    GO_TYPE_EMITTER     = BIT6,
    GO_TYPE_CONTROLLER  = BIT7,
    
    GO_TYPE_USER        = BIT8,
    // The game can specify custom types starting with this bit.
    // E.G.  const GO_TYPE GO_TYPE_BRICK  = GO_TYPE_USER;
    //       const GO_TYPE GO_TYPE_WEAPON = (GO_TYPE)(GO_TYPE_USER << 1);
    
    GO_TYPE_ANY         = 0xFFFFFFFF,
} GO_TYPE;

   

class GameObject : virtual public Object, public IDrawable
{
public:
	GameObject( GO_TYPE type = GO_TYPE_UNKNOWN );
	virtual ~GameObject( );
    
	inline GO_TYPE      GetType     ( )     { return m_type; }

    // TODO: override Release()
    // so we can mark ourselves for deletion rather than delete immediately.
    // Required so that GOs don't delete themselves while GOManager is iterating over m_resourceList.

    RESULT              Init        ( IN const string& name, IN const Settings* pSettings = NULL, IN const string& settingsPath = "" );
	RESULT              Update      ( UINT64 elapsedMS );
    
	// IDrawable
    RESULT              SetVisible  ( bool          isVisible        );
    RESULT              SetPosition ( const vec3&   vPos             );
    RESULT              SetRotation ( const vec3&   vRotationDegrees );
    RESULT              SetScale    ( float         scale            );
    RESULT              SetOpacity  ( float         opacity          );
    RESULT              SetEffect   ( HEffect       hEffect          );
    RESULT              SetColor    ( const Color&  color            );
    RESULT              SetShadow   ( bool          hasShadow        );

    inline bool         GetVisible  ( )                                     { return m_isVisible;       };
    inline vec3         GetPosition ( )                                     { return m_vWorldPosition;  };
    inline vec3         GetRotation ( )                                     { return m_vRotation;       };
    inline float        GetScale    ( )                                     { return m_fScale;          };
    inline float        GetOpacity  ( )                                     { return m_fOpacity;        };
    inline HEffect      GetEffect   ( )                                     { return m_hEffect;         };
    AABB                GetBounds   ( );
    inline Color        GetColor    ( )                                     { return m_color;           };
    inline bool         GetShadow   ( )                                     { return m_hasShadow;       };

    RESULT              Draw        ( const mat4&   matParentWorld );

    virtual IProperty*  GetProperty ( const string& name ) const;


    // TODO: Push/PopBehavior( HBehavior, queue number );
    
    
    inline void         MarkForDeletion()       { m_isMarkedForDeletion = true; }
    inline bool         IsMarkedForDeletion()   { return m_isMarkedForDeletion; }
    
    //------------------------------------------------------------------------
    // Game Object Components
    //------------------------------------------------------------------------
    

    RESULT              SetMesh                 ( HMesh             hMesh            );
    RESULT              SetSprite               ( HSprite           hSprite          );
    RESULT              SetSpriteFrame          ( UINT8             spriteFrame      );
    
    HMesh               GetMesh                 ( )    { return m_hMesh;            }
    HSprite             GetSprite               ( )    { return m_hSprite;          }
    UINT32              GetSpriteFrame          ( )    { return m_spriteFrame;      }

    RESULT              AddChild                ( IN HGameObject        HGameObject      );
    RESULT              AddChild                ( IN HSprite            hSprite          );
    RESULT              AddChild                ( IN HParticleEmitter   HParticleEmitter );
    
    RESULT              GetChildren             ( INOUT HGameObjectList*        pList );
    RESULT              GetChildren             ( INOUT HSpriteList*            pList );
    RESULT              GetChildren             ( INOUT HParticleEmitterList*   pList );
    
            
    RESULT              CreateStateMachineManager( );
    RESULT              SetStateMachineManager   ( StateMachineManager* pStateMachineManager  );
    StateMachineManager* GetStateMachineManager  ( )    { return m_pStateMachineManager;  }
    
protected:
    void                UpdateBoundingBox();


    
private:
	GO_TYPE                 m_type;
    bool                    m_isMarkedForDeletion;
    
    bool                    m_isVisible;
    mat4                    m_matWorld;
    vec3                    m_vWorldPosition;
    vec3                    m_vRotation;
    float                   m_fScale;
    float                   m_fOpacity;
    AABB                    m_bounds;
    Color                   m_color;
    bool                    m_hasShadow;
    
    // Components
    HEffect                 m_hEffect;
    HMesh                   m_hMesh;
    HSprite                 m_hSprite;
    UINT32                  m_spriteFrame;
    HParticleEmitter        m_hParticleEmitter;

    HSpriteList             m_hSpriteChildren;
    HGameObjectList         m_hGameObjectChildren;
    HParticleEmitterList    m_hParticleEmitterChildren;

    StateMachineManager*    m_pStateMachineManager; // TODO: create a ResourceManager for HStateMachineManagers.
    
    
//----------------
// Class data
//----------------

    //
    // Exported Properties for animation/scripting
    //
    static       PropertySet        s_properties;
    
};




} // END namespace Z

