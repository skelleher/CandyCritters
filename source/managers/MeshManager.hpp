#pragma once

#include "ResourceManager.hpp"
#include "Handle.hpp"
#include "IRenderer.hpp"
#include "Object.hpp"
#include "Settings.hpp"
#include "TextureManager.hpp"
#include "IDrawable.hpp"

#include <string>
using std::string;


namespace Z
{


    
// Forward declarations
class Mesh;
typedef Handle<Mesh>            HMesh;
typedef list<HMesh>             HMeshList;
typedef HMeshList::iterator     HMeshListIterator;


class MeshManager : public ResourceManager<Mesh>
{
public:
    static  MeshManager&    Instance    ( );
    
    virtual RESULT          Init        ( IN const string& settingsFilename    );
    RESULT                  DrawMesh    ( IN HMesh hMesh, mat4& matWorld );

	// IDrawable
    // TODO: are handles worth the annoyance of having to expose every object method like this?!
    RESULT              SetVisible       ( IN HMesh hMesh, bool        isVisible        );
    RESULT              SetPosition      ( IN HMesh hMesh, const vec3& vPos             );
    RESULT              SetRotation      ( IN HMesh hMesh, const vec3& vRotationDegrees );
    RESULT              SetScale         ( IN HMesh hMesh, float       scale            );
    RESULT              SetOpacity       ( IN HMesh hMesh, float       opacity          );
    RESULT              SetEffect        ( IN HMesh hMesh, HEffect     hEffect          );
    RESULT              SetColor         ( IN HMesh hMesh, const Color& color           );
    RESULT              SetShadow        ( IN HMesh hMesh, bool        shadowEnabled    );

    
    bool                GetVisible       ( IN HMesh hMesh                               );
    vec3                GetPosition      ( IN HMesh hMesh                               );
    vec3                GetRotation      ( IN HMesh hMesh                               );
    float               GetScale         ( IN HMesh hMesh                               );
    float               GetOpacity       ( IN HMesh hMesh                               );
    HEffect             GetEffect        ( IN HMesh hMesh                               );
    AABB                GetBounds        ( IN HMesh hMesh                               );
    bool                GetShadow        ( IN HMesh hMesh                               );

    
protected:
    MeshManager();
    MeshManager( const MeshManager& rhs );
    MeshManager& operator=( const MeshManager& rhs );
    virtual ~MeshManager();
 
protected:
    RESULT  CreateMesh( IN Settings* pSettings, IN const string& settingsPath, INOUT Mesh** ppMesh );
};

#define MeshMan ((MeshManager&)MeshManager::Instance())



#pragma mark -
#pragma mark Mesh

//=============================================================================
//
// A Mesh instance.
//
//=============================================================================
class Mesh : virtual public Object, IDrawable
{
public:
    Mesh();
    virtual ~Mesh();
    
    RESULT              Init        ( IN const string& name, IN const Settings* pSettings, IN const string& settingsPath );
    
	// IDrawable
    inline RESULT       SetVisible      ( bool          isVisible        )      { m_isVisible       = isVisible; return S_OK; }
    inline RESULT       SetPosition     ( const vec3&   vPos             )      { m_vWorldPosition  = vPos;             UpdateBoundingBox(); return S_OK; }
    inline RESULT       SetRotation     ( const vec3&   vRotationDegrees )      { m_vRotation       = vRotationDegrees; UpdateBoundingBox(); return S_OK; }
    inline RESULT       SetScale        ( float         scale            )      { m_fScale          = scale;            UpdateBoundingBox(); return S_OK; }
    RESULT              SetOpacity      ( float         opacity          );
    RESULT              SetEffect       ( HEffect       hEffect          );
    inline RESULT       SetColor        ( const Color&  color            )      { m_color           = color;     return S_OK; }
    inline RESULT       SetShadow       ( bool          enabled          )      { m_hasShadow       = enabled;   return S_OK; }


    inline bool         GetVisible      ( )                                     { return m_isVisible;       };
    inline vec3         GetPosition     ( )                                     { return m_vWorldPosition;  };
    inline vec3         GetRotation     ( )                                     { return m_vRotation;       };
    inline float        GetScale        ( )                                     { return m_fScale;          };
    inline float        GetOpacity      ( )                                     { return m_fOpacity;        };
    inline AABB         GetBounds       ( )                                     { return m_bounds;          };
    inline HEffect      GetEffect       ( )                                     { return m_hEffect;         };
    inline Color        GetColor        ( )                                     { return m_color;           };
    inline bool         GetShadow       ( )                                     { return m_hasShadow;       };


    RESULT              Draw            ( const mat4&   matParentWorld );
    
protected:
    void                UpdateBoundingBox();


protected:
    HEffect             m_hEffect;
    HTexture            m_hTexture;
    Vertex*             m_pVertices;
    
    mat4                m_matWorld;
    vec3                m_vWorldPosition;
    vec3                m_vRotation;
    float               m_fScale;
    float               m_fOpacity;
    bool                m_isVisible;
    AABB                m_bounds;
    Color               m_color;
    bool                m_hasShadow;
    
    // TODO: VBO, multi-texturing
};

typedef Handle<Mesh> HMesh;



} // END namespace Z


