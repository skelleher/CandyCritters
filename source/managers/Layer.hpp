#pragma once

#include "Types.hpp"
#include "Handle.hpp"
#include "Object.hpp"
#include "Settings.hpp"
#include "IDrawable.hpp"
#include "GameObject.hpp"
#include "ParticleEmitter.hpp"

#include <list>
using std::list;


namespace Z
{


// Forward declaration.
class Layer;
typedef Handle<Layer> HLayer;


//=============================================================================
//
// A Layer instance.
//
// Layers are used to group IDrawables on the screen.
// One Layer might contain the HUD, another the playing pieces, and another
// the game background.  
//
//=============================================================================
class Layer : virtual public Object, IDrawable
{
public:
    Layer();
    virtual ~Layer();
    
    RESULT              Init                 ( IN const string& name             );

    RESULT              AddGameObject        ( IN HGameObject        hGameObject        );
    RESULT              AddSprite            ( IN HSprite            hSprite            );
    RESULT              AddLayer             ( IN HLayer             hLayer             );
    RESULT              AddParticleEmitter   ( IN HParticleEmitter   hParticleEmitter   );
    RESULT              RemoveGameObject     ( IN HGameObject        hGameObject        );
    RESULT              RemoveSprite         ( IN HSprite            hSprite            );
    RESULT              RemoveLayer          ( IN HLayer             hLayer             );
    RESULT              RemoveParticleEmitter( IN HParticleEmitter   hParticleEmitter   );
    RESULT              Clear           ( );

    RESULT              Show            ( IN HEffect hEffect = HEffect::NullHandle(), IN HStoryboard hStoryboard = HStoryboard::NullHandle() ); // TODO: storyboards for both Sprite and Effect?
    RESULT              Hide            ( IN HEffect hEffect = HEffect::NullHandle(), IN HStoryboard hStoryboard = HStoryboard::NullHandle() );


    inline RESULT       SetParentLayer  ( IN HLayer hParentLayer )              { m_hParentLayer = hParentLayer; return S_OK; }
    inline HLayer       GetParentLayer  ( )                                     { return m_hParentLayer;           }
    inline UINT32       GetNumSprites   ( )                                     { return m_spriteList.size();      }
    inline UINT32       GetNumGameObjects( )                                    { return m_gameObjectList.size();  }
    inline UINT32       GetNumLayers    ( )                                     { return m_layerList.size();       }
    inline UINT32       GetNumEmitters  ( )                                     { return m_particleEmitterList.size();       }
    inline bool         IsRootLayer     ( )                                     { return m_hParentLayer.IsNull();  }
    inline bool         IsChildLayer    ( )                                     { return !m_hParentLayer.IsNull(); }

	// IDrawable
    inline RESULT       SetVisible      ( bool          isVisible           )   { m_isVisible       = isVisible;                return S_OK; }
    inline RESULT       SetPosition     ( const vec3&   vPos                )   { m_vWorldPosition  = vPos;                     return S_OK; }
    inline RESULT       SetRotation     ( const vec3&   vRotationDegrees    )   { m_vRotation       = vRotationDegrees;         return S_OK; }
    inline RESULT       SetScale        ( float         scale               )   { m_fScale          = scale;                    return S_OK; }
    inline RESULT       SetOpacity      ( float         opacity             )   { m_fOpacity        = CLAMP(opacity, 0.0, 1.0); return S_OK; }
    RESULT              SetEffect       ( HEffect       hEffect             );
    RESULT              SetColor        ( const Color&  color               )   { m_color           = color;                    return S_OK; }
    RESULT              SetShadow       ( bool          hasShadow           );

    inline bool         GetVisible      ( )                                     { return m_isVisible;       }
    inline vec3         GetPosition     ( )                                     { return m_vWorldPosition;  }
    inline vec3         GetRotation     ( )                                     { return m_vRotation;       }
    inline float        GetScale        ( )                                     { return m_fScale;          }
    inline float        GetOpacity      ( )                                     { return m_fOpacity;        }
    inline HEffect      GetEffect       ( )                                     { return m_hEffect;         }
    inline AABB         GetBounds       ( )                                     { return m_bounds;          }
    inline Color        GetColor        ( )                                     { return m_color;           }
    inline bool         GetShadow       ( )                                     { return m_isShadowEnabled; }

    RESULT              Draw            ( const mat4&   matParentWorld );
    
    virtual IProperty*  GetProperty     ( const string& name ) const;


protected:
    RESULT              DrawContents    ( const mat4&   matParentWorld, bool shadowPass = false );
    RESULT              DrawShadowPass  ( const mat4&   matParentWorld );

    static void         OnDoneShowing(void* context);
    static void         OnDoneHiding (void* context);
    
    

protected:
    HLayer          m_hParentLayer;
    HEffect         m_hEffect;
    HStoryboard     m_hStoryboard;
    bool            m_isVisible;
    float           m_fScale;
    float           m_fOpacity;
    vec3            m_vWorldPosition;
    vec3            m_vRotation;
    AABB            m_bounds;
    Color           m_color;
    bool            m_isShadowEnabled;
    
    typedef list<HSprite>               SpriteList;
    typedef SpriteList::iterator        SpriteListIterator;
    typedef list<HGameObject>           GameObjectList;
    typedef GameObjectList::iterator    GameObjectListIterator;
    typedef list<HLayer>                LayerList;
    typedef LayerList::iterator         LayerListIterator;
    
    
    // TODO: should be one list, for intermingling of IDrawables regardless of type/Z-depth.
    SpriteList          m_spriteList;
    GameObjectList      m_gameObjectList;
    LayerList           m_layerList;
    ParticleEmitterList m_particleEmitterList;
    
    
//----------------
// Class data
//----------------

    //
    // Exported Properties for animation/scripting
    //
    static       PropertySet        s_properties;
    
};

typedef Handle<Layer> HLayer;



} // END namespace Z
