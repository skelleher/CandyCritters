#pragma once

#include "ResourceManager.hpp"
#include "Handle.hpp"
#include "IRenderer.hpp"
#include "Object.hpp"
#include "Settings.hpp"
#include "TextureManager.hpp"
#include "EffectManager.hpp"
#include "IDrawable.hpp"


#include <string>
#include <vector>   // for SpriteBatch
#include <map>

using std::string;
using std::vector;
using std::map;
    

namespace Z
{



// Forward declarations
class Sprite;
typedef Handle<Sprite>          HSprite;
typedef list<HSprite>           HSpriteList;
typedef HSpriteList::iterator   HSpriteListIterator;



// Don't bother with 4 verts per Sprite (GL_TRIANGLE_STRIP).
// We batch them up and submit one big array, and 
// that's easier without strips.
#define VERTS_PER_SPRITE 6


class SpriteManager : public ResourceManager<Sprite>
{
public:
    static  SpriteManager& Instance();

    virtual RESULT      Init             ( IN const string& settingsFilename );
    virtual RESULT      Shutdown         ( );


    RESULT              CreateFromFile   ( IN    const  string&     filename, 
                                           INOUT        HSprite*    pHandle,
                                           IN    const  Color&      color = Color::White() );

    RESULT              CreateFromTexture( IN    const  string&     name, 
                                           IN    const  HTexture&   hTexture,
                                           IN    const  Rectangle&  spriteRect,
                                           INOUT        HSprite*    pHandle = NULL,
                                           IN    const  Color&      color   = Color::White() );

    RESULT              CreateFromTexture( IN    const  string&     name, 
                                           IN    const  string&     textureName,
                                           IN    const  Rectangle&  spriteRect,
                                           INOUT        HSprite*    pHandle = NULL,
                                           IN    const  Color&      color   = Color::White() );

    RESULT              BeginBatch       ( );

    RESULT              DrawSprite       ( 
                                            IN       HSprite hSprite, 
                                            IN const mat4&   matParentWorld = mat4::Identity()
                                         );
                                         
    RESULT              DrawSprite       ( 
                                            IN       HSprite hSprite, 
                                            IN const WORLD_POSITION& pos, 
                                                     float   opacity        = 1.0f, 
                                                     float   scale          = 1.0f, 
                                                     float   rotateX        = 0.0f, 
                                                     float   rotateY        = 0.0f, 
                                                     float   rotateZ        = 0.0f, 
                                            IN const mat4&   matParentWorld = mat4::Identity()
                                         );
                                          
    RESULT              DrawSprite       ( 
                                            IN       HSprite  hSprite, 
                                                     float    x, 
                                                     float    y, 
                                                     float    z, 
                                                     float    opacity = 1.0f, 
                                                     float    scale   = 1.0f, 
                                                     float    rotateX = 0.0f, 
                                                     float    rotateY = 0.0f, 
                                                     float    rotateZ = 0.0f, 
                                            IN const mat4&    matParentWorld = mat4::Identity()  
                                          );
    
    RESULT              EndBatch         ( );
    
    RESULT              SetSpriteFrame   ( IN HSprite hSprite, UINT8 frame );
    
	// IDrawable
    // TODO: are handles worth the annoyance of having to expose every object method like this?!
    RESULT              SetVisible       ( IN HSprite hSprite, bool        isVisible        );
    RESULT              SetPosition      ( IN HSprite hSprite, const vec3& vPos             );
    RESULT              SetRotation      ( IN HSprite hSprite, const vec3& vRotationDegrees );
    RESULT              SetScale         ( IN HSprite hSprite, float       scale            );
    RESULT              SetOpacity       ( IN HSprite hSprite, float       opacity          );
    RESULT              SetEffect        ( IN HSprite hSprite, HEffect     hEffect          );
    RESULT              SetColor         ( IN HSprite hSprite, const Color& color           );
    RESULT              SetShadow        ( IN HSprite hSprite, bool        hasShadow        );


    bool                GetVisible       ( IN HSprite hSprite                               );
    vec3                GetPosition      ( IN HSprite hSprite                               );
    vec3                GetRotation      ( IN HSprite hSprite                               );
    float               GetScale         ( IN HSprite hSprite                               );
    float               GetOpacity       ( IN HSprite hSprite                               );
    HEffect             GetEffect        ( IN HSprite hSprite                               );
    AABB                GetBounds        ( IN HSprite hSprite                               );
    Color               GetColor         ( IN HSprite hSprite                               );
    bool                GetShadow        ( IN HSprite hSprite                               );


    
protected:
    SpriteManager();
    SpriteManager( const SpriteManager& rhs );
    SpriteManager& operator=( const SpriteManager& rhs );
    virtual ~SpriteManager();
 
    RESULT  CreateSprite( IN Settings* pSettings, IN const string& settingsPath, INOUT Sprite** ppSprite );
    
protected:
    // We create one SpriteBatch for each Material / texture atlas combo
    // between BeginBatch() and EndBatch().
    // Then we batch up all sprites for each Material / texture atlas into a vertex array
    // and submit them at once.

    struct BatchedSprite
    {
        Sprite*     pSprite;
        float       opacity;
        float       scale;
        vec3        rotation;
        vec3        position;
        mat4        matWorldParent; // TODO: apply to scale/rotation/position directly, save 16 floats!
    };
    
    typedef     vector<BatchedSprite*>      BatchedSprites;
    typedef     BatchedSprites::iterator    BatchedSpritesIterator;

    // A batch of Sprites, which share the same Material and texture atlas.
    struct SpriteBatch
    {
//        const string    textureName;
        HEffect         hEffect;
        HTexture        hTexture;
        UINT32          textureID;
        UINT32          numSprites;
        UINT32          numVertices;
        UINT32          zOrder;
        Vertex*         pVertices;
        BatchedSprites  sprites;
    };
    

    // Key for our map of SpriteBatches.
    // They are sorted by Material, texture atlas, then z-order (back to front).
    class SpriteBatchKey
    {
    public:
        SpriteBatchKey() :
            textureID(0),
            zOrder(0)
        { }
        
        SpriteBatchKey( long unsigned int x) {}
        SpriteBatchKey(const int A, const int B) :
            hEffect(), textureID(B) {}
        
        HEffect     hEffect;
        UINT32      textureID;
        UINT32      zOrder;
        
        // std::map requires that Keys be sortable using the less-than operator.
        bool operator<(const SpriteBatchKey &rhs) const
        {
            UINT32 left  = hEffect     + textureID      + zOrder;
            UINT32 right = rhs.hEffect + rhs.textureID  + zOrder;
            
            return left < right;
        }
    };
    
    typedef     map<SpriteBatchKey, SpriteBatch*>   SpriteBatchMap;
    typedef     SpriteBatchMap::iterator            SpriteBatchMapIterator;

    SpriteBatchMap  m_spriteBatchMap;
    bool            m_inSpriteBatch;
    UINT32          m_zOrderForBatch;
};

#define SpriteMan ((SpriteManager&)SpriteManager::Instance())



#pragma mark -
#pragma mark Sprite

//=============================================================================
//
// A Sprite instance.
//
//=============================================================================
const UINT32 MAX_SPRITE_FRAMES = 32;
class Sprite : virtual public Object, IDrawable
{
public:
    Sprite();
    virtual ~Sprite();

    Sprite*             Clone           ( ) const;
    

    RESULT              Init            ( IN const string& name, IN const Settings* pSettings, IN const string& settingsPath );
    


    RESULT              Init            ( IN const  string&     name, 
                                          IN const  HTexture&   hTexture, 
                                          IN const  Rectangle&  spriteRect,
                                          IN const  Color&      color = Color::White() );
	// IDrawable
    inline RESULT       SetVisible      ( bool          isVisible        )      { m_isVisible       = isVisible; return S_OK; }
    inline RESULT       SetPosition     ( const vec3&   vPos             )      { m_vWorldPosition  = vPos;             UpdateBoundingBox(); return S_OK; }
    inline RESULT       SetRotation     ( const vec3&   vRotationDegrees )      { m_vRotation       = vRotationDegrees; UpdateBoundingBox(); return S_OK; }
    inline RESULT       SetScale        ( float         scale            )      { m_fScale          = scale;            UpdateBoundingBox(); return S_OK; }
    RESULT              SetOpacity      ( float         opacity          );
    RESULT              SetEffect       ( HEffect       hEffect          );
    RESULT              SetColor        ( const Color&  color            );
    RESULT              SetShadow       ( bool          hasShadow        )      { m_hasShadow       = hasShadow; return S_OK; }

    
    inline bool         GetVisible      ( )                                     { return m_isVisible;       };
    inline vec3         GetPosition     ( )                                     { return m_vWorldPosition;  };
    inline vec3         GetRotation     ( )                                     { return m_vRotation;       };
    inline float        GetScale        ( )                                     { return m_fScale;          };
    inline float        GetOpacity      ( )                                     { return m_fOpacity;        };
    inline HEffect      GetEffect       ( )                                     { return m_hEffect;         };
    inline AABB         GetBounds       ( )                                     { return m_bounds;          };
    inline Color        GetColor        ( )                                     { return m_color;           };
    inline bool         GetShadow       ( )                                     { return m_hasShadow;       };


    RESULT              Draw            ( const mat4&   matParentWorld );
    
    RESULT              SetSpriteFrame  ( UINT8 frame );
    UINT32              GetSpriteFrame  ( )    { return m_frame; }

    RESULT              GetVertices     ( INOUT Vertex** ppVertices, OUT UINT32* pNumVertices );
    RESULT              GetTexture      ( INOUT HTexture* phTexture );
//    RESULT              GetTextureAtlas ( INOUT HTextureAtlas* phTextureAtlas );
    bool                IsBackedByTextureAtlas( )   { return m_isBackedByTextureAtlas; }

    virtual IProperty*  GetProperty     ( const string& name ) const;

protected:
    void                UpdateBoundingBox();


protected:
    //HTextureAtlas       m_hTextureAtlas;
    HEffect             m_hEffect;

    bool                m_isBackedByTextureAtlas;

    UINT32              m_width;
    UINT32              m_height;
    AABB                m_bounds;
    
    UINT8               m_frame;
    UINT8               m_numFrames;
    
    Color               m_color;
    bool                m_isVisible;
    float               m_fScale;
    float               m_fOpacity;
    vec3                m_vWorldPosition;
    vec3                m_vRotation;
    bool                m_hasShadow;
    
    HTexture            m_hTextures[MAX_SPRITE_FRAMES];
    Vertex              m_vertices[MAX_SPRITE_FRAMES][VERTS_PER_SPRITE];


    
//----------------
// Class data
//----------------

    //
    // Exported Properties for animation/scripting
    //
    static       PropertySet        s_properties;

};

typedef Handle<Sprite> HSprite;



} // END namespace Z



