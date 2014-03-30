#pragma once

#include "Engine.hpp"
#include "Macros.hpp"
#include "Util.hpp"
#include "IEffect.hpp"
#include "RenderTarget.hpp"
#include "ShaderManager.hpp"
#include "EffectManager.hpp"
#include "Property.hpp"

#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

// TEST
#include "DebugRenderer.hpp"


namespace Z
{

typedef enum 
{
    PRIMITIVE_TYPE_TRIANGLE_STRIP = 1,
    PRIMITIVE_TYPE_TRIANGLE_LIST
} PRIMITIVE_TYPE;


//
// This base class contains the default shaders; 
// subclasses should override to load custom shaders and implement custom drawing.
//

class OpenGLESEffect : virtual public Object, virtual public IEffect  
{
public:
    virtual OpenGLESEffect* Clone       ( ) const;
    virtual ~OpenGLESEffect();

    virtual bool   IsPostEffect         ( ) const;

    virtual RESULT Enable               ( ) const;
    virtual RESULT BeginFrame           ( IN const Rectangle& frame );
    virtual RESULT EndFrame             ( bool present );

    virtual RESULT SetModelViewMatrix   ( IN const mat4& modelView  );
    virtual RESULT SetProjectionMatrix  ( IN const mat4& projection );
    virtual RESULT SetTexture           ( UINT8 textureUnit, HTexture hTexture  );
    virtual RESULT SetTexture           ( UINT8 textureUnit, UINT32   textureID );
    virtual RESULT SetGlobalColor       ( IN const Color& color );

    virtual RESULT Draw                 ( IN Vertex *pVertices, UINT32 numVertices, PRIMITIVE_TYPE type );
    virtual RESULT DrawTriangleStrip    ( IN Vertex *pVertices, UINT32 numVertices                      );
    virtual RESULT DrawTriangleList     ( IN Vertex *pVertices, UINT32 numVertices                      );

    virtual HShader GetShader           ( );

    virtual IProperty* GetProperty      ( const string& name ) const;

protected:
    OpenGLESEffect();
    OpenGLESEffect( const OpenGLESEffect& rhs );
    OpenGLESEffect& operator=( const OpenGLESEffect& rhs );



//----------------------------------------------------------------------------
// Class methods
//----------------------------------------------------------------------------
public:
    static  OpenGLESEffect* CreateInstance( );

protected:
    static  RESULT          Init   ( );
    static  RESULT          Deinit ( );


//----------------------------------------------------------------------------
// Object data
//----------------------------------------------------------------------------
protected:
    // Effect may optionally be applied post-render,
    // in which case we create an offscreen RenderTarget
    // to accumulate all the draw calls, then present it
    // in ::EndFrame().
    bool                m_isPostEffect;
    bool                m_needsRenderTarget;
    RenderTarget*       m_pRenderTarget;
    Rectangle           m_frame;
    bool                m_frameChanged;
    Vertex*             m_pFrameVertices;
    UINT32              m_numFrameVertices;
    

    mat4                m_modelViewMatrix;
    mat4                m_projectionMatrix;
    HTexture            m_hSourceTexture;
    GLuint              m_sourceTextureID;
    Color               m_globalColor;


//----------------------------------------------------------------------------
// Class data
//----------------------------------------------------------------------------
protected:
    static UINT32               s_numInstances;

    static bool                 s_haveCheckedPlatformSupport;
    static bool                 s_isPlatformSupported;

    static bool                 s_defaultShadersLoaded;
    static HShader              s_hDefaultShader;
    static GLuint               s_defaultShaderProgram;

    static GLint                s_uDefaultModelViewMatrix;
    static GLint                s_uDefaultProjectionMatrix;
    static GLint                s_uDefaultTexture;
    static GLint                s_uDefaultGlobalColor;

    static const char*          s_defaultVertexShaderName;
    static const char*          s_defaultFragmentShaderName;
};



} // END namespace Z
