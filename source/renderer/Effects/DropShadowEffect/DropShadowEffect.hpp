#pragma once


#include "OpenGLESEffect.hpp"


namespace Z
{



class DropShadowEffect : public OpenGLESEffect
{
//----------------------------------------------------------------------------
// Object methods
//----------------------------------------------------------------------------
public:
    virtual DropShadowEffect* Clone() const;
    virtual ~DropShadowEffect();

    virtual RESULT  Draw                ( IN Vertex *pVertices, UINT32 numVertices, PRIMITIVE_TYPE type );
    virtual RESULT  DrawTriangleStrip   ( IN Vertex *pVertices, UINT32 numVertices                      );
    virtual RESULT  DrawTriangleList    ( IN Vertex *pVertices, UINT32 numVertices                      );

    // Custom Effect Properties
    virtual void    SetColor            ( const Color& color );
    virtual void    SetDirection        ( float directionDegrees );
    virtual void    SetDepth            ( float depth );

    virtual Color   GetColor            ( );
    virtual float   GetDirection        ( );
    virtual float   GetDepth            ( );
    
    virtual HShader GetShader           ( );
    
    virtual IProperty* GetProperty      ( const string& name ) const;

protected:
    DropShadowEffect();
    DropShadowEffect( const DropShadowEffect& rhs );
    DropShadowEffect& operator=( const DropShadowEffect& rhs );



//----------------------------------------------------------------------------
// Object data
//----------------------------------------------------------------------------
protected:
    Color           m_color;
    float           m_fDirectionDegrees;
    float           m_fDepth;
    

//----------------------------------------------------------------------------
// Class methods
//----------------------------------------------------------------------------
public:
    static DropShadowEffect*        CreateInstance  ( );

protected:
    static RESULT                   Init            ( );
    static RESULT                   Deinit          ( );


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
    static GLint                    s_uDepth;
    static GLint                    s_uDirection;
    

    static const Color              DEFAULT_COLOR;
    static const float              DEFAULT_DEPTH;
    static const float              DEFAULT_DIRECTION_DEGREES;


    //
    // Exported Properties for animation/scripting
    //
    static       PropertySet        s_properties;
    
};



} // END namespace Z
