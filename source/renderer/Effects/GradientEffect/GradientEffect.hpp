#pragma once


#include "OpenGLESEffect.hpp"


namespace Z
{



class GradientEffect : public OpenGLESEffect
{
//----------------------------------------------------------------------------
// Object methods
//----------------------------------------------------------------------------
public:
    virtual GradientEffect* Clone() const;
    virtual ~GradientEffect();

    virtual RESULT  Draw                ( IN Vertex *pVertices, UINT32 numVertices, PRIMITIVE_TYPE type );
    virtual RESULT  DrawTriangleStrip   ( IN Vertex *pVertices, UINT32 numVertices                      );
    virtual RESULT  DrawTriangleList    ( IN Vertex *pVertices, UINT32 numVertices                      );

    // Custom Effect Properties
    virtual void    SetStartColor       ( const Color& color );
    virtual void    SetStartPoint       ( const vec2& origin );
    virtual void    SetEndColor         ( const Color& color );
    virtual void    SetEndPoint         ( const vec2& origin );

    virtual Color   GetStartColor       ( );
    virtual vec2    GetStartPoint       ( );
    virtual Color   GetEndColor         ( );
    virtual vec2    GetEndPoint         ( );
    
    virtual HShader GetShader           ( );
    
    virtual IProperty* GetProperty      ( const string& name ) const;

protected:
    GradientEffect();
    GradientEffect( const GradientEffect& rhs );
    GradientEffect& operator=( const GradientEffect& rhs );

    virtual RESULT InitScratchSurfaces  ( Rectangle& sourceRect, Rectangle& textureRect );


//----------------------------------------------------------------------------
// Object data
//----------------------------------------------------------------------------
protected:
    Vertex*         m_pGradientVertices;
    UINT32          m_numGradientVertices;
    Rectangle       m_gradientRect;

    Color           m_startColor;
    vec2            m_startPoint;

    Color           m_endColor;
    vec2            m_endPoint;
    

//----------------------------------------------------------------------------
// Class methods
//----------------------------------------------------------------------------
public:
    static GradientEffect*      CreateInstance  ( );

protected:
    static RESULT               Init            ( );
    static RESULT               Deinit          ( );


//----------------
// Class data
//----------------
protected:
    static UINT32                   s_NumInstances;

    // Shaders
    static bool                     s_GradientShadersLoaded;
    static GLuint                   s_GradientShaderProgram;
    static HShader                  s_hGradientShader;

    static const char*              s_GradientVertexShaderName;
    static const char*              s_GradientFragmentShaderName;

    // Shader parameters
    static GLint                    s_uProjectionMatrix;
    static GLint                    s_uModelViewMatrix;
    static GLint                    s_uTexture;
    static GLint                    s_uGlobalColor;
    static GLint                    s_uStartColor;
    static GLint                    s_uStartPoint;
    static GLint                    s_uEndColor;
    static GLint                    s_uEndPoint;
    static GLint                    s_uWidth;
    static GLint                    s_uHeight;

    static const Color              DEFAULT_START_COLOR;
    static const Color              DEFAULT_END_COLOR;


    //
    // Exported Properties for animation/scripting
    //
    static       PropertySet        s_properties;
    
};



} // END namespace Z
