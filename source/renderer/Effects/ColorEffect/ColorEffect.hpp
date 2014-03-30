#pragma once


#include "OpenGLESEffect.hpp"


namespace Z
{



class ColorEffect : public OpenGLESEffect
{
//----------------------------------------------------------------------------
// Object methods
//----------------------------------------------------------------------------
public:
    virtual ColorEffect* Clone() const;
    virtual ~ColorEffect();

    virtual RESULT  Draw                ( IN Vertex *pVertices, UINT32 numVertices, PRIMITIVE_TYPE type );
    virtual RESULT  DrawTriangleStrip   ( IN Vertex *pVertices, UINT32 numVertices                      );
    virtual RESULT  DrawTriangleList    ( IN Vertex *pVertices, UINT32 numVertices                      );

    // Custom Effect Properties
    virtual void    SetColor            ( const Color& color );
    virtual void    SetBlend            ( float blend        );
    virtual void    SetOrigin           ( const vec2& origin );
    virtual void    SetRadius           ( float radius       );

    virtual Color   GetColor            ( );
    virtual float   GetBlend            ( );
    virtual vec2    GetOrigin           ( );
    virtual float   GetRadius           ( );
    
    virtual HShader GetShader           ( );
    
    virtual IProperty* GetProperty      ( const string& name ) const;

protected:
    ColorEffect();
    ColorEffect( const ColorEffect& rhs );
    ColorEffect& operator=( const ColorEffect& rhs );



//----------------------------------------------------------------------------
// Object data
//----------------------------------------------------------------------------
protected:
    Color           m_color;
    float           m_fBlend;
    vec2            m_origin;
    float           m_fRadius;
    

//----------------------------------------------------------------------------
// Class methods
//----------------------------------------------------------------------------
public:
    static ColorEffect*     CreateInstance  ( );

protected:
    static RESULT           Init            ( );
    static RESULT           Deinit          ( );


//----------------
// Class data
//----------------
protected:
    static UINT32                   s_NumInstances;

    // Shaders
    static bool                     s_ColorShadersLoaded;
    static GLuint                   s_ColorShaderProgram;
    static HShader                  s_hColorShader;

    static const char*              s_ColorVertexShaderName;
    static const char*              s_ColorFragmentShaderName;

    // Shader parameters
    static GLint                    s_uProjectionMatrix;
    static GLint                    s_uModelViewMatrix;
    static GLint                    s_uTexture;
    static GLint                    s_uGlobalColor;
    static GLint                    s_uColor;
    static GLint                    s_uBlend;
    static GLint                    s_uOrigin;
    static GLint                    s_uRadius;

    static const Color              DEFAULT_COLOR;
    static const float              DEFAULT_ORIGIN_X;
    static const float              DEFAULT_ORIGIN_Y;
    static const float              DEFAULT_RADIUS;


    //
    // Exported Properties for animation/scripting
    //
    static       PropertySet        s_properties;
    
};



} // END namespace Z
