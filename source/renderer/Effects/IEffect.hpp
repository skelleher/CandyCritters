#pragma once

#include "Types.hpp"
#include "Object.hpp"
#include "Property.hpp"
#include "Vertex.hpp"
#include "TextureManager.hpp"
#include "ShaderManager.hpp"


//
// This Interface defines an Effect which may override the default rendering of an IDrawable.
//

namespace Z
{


class IEffect :  virtual public IObject  
{
public:
    virtual IEffect* Clone              ( ) const                                       = 0;

    virtual bool   IsPostEffect         ( ) const                                       = 0;

    virtual RESULT Enable               ( ) const                                       = 0;
    virtual RESULT BeginFrame           ( IN const Rectangle& rectangle )               = 0;
    virtual RESULT EndFrame             ( bool present )                                = 0;

    virtual RESULT SetModelViewMatrix   ( IN const mat4& modelView  )                   = 0;
    virtual RESULT SetProjectionMatrix  ( IN const mat4& projection )                   = 0;
    virtual RESULT SetTexture           ( UINT8 textureUnit, HTexture hTexture  )       = 0;
    virtual RESULT SetTexture           ( UINT8 textureUnit, UINT32   textureID )       = 0;
    virtual RESULT SetGlobalColor       ( IN const Color& color )                       = 0;

    virtual RESULT DrawTriangleStrip    ( IN Vertex *pVertices, UINT32 numVertices )    = 0;
    virtual RESULT DrawTriangleList     ( IN Vertex *pVertices, UINT32 numVertices )    = 0;
    
    virtual HShader GetShader           ( )                                             = 0;

    virtual ~IEffect() {};
};


typedef Handle<IEffect>      HEffect;
typedef Property<IEffect>    EffectProperty;


} // END namespace Z
