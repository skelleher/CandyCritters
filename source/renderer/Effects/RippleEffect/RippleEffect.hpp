#pragma once


#include "OpenGLESEffect.hpp"


namespace Z
{



class RippleEffect : public OpenGLESEffect
{
//----------------------------------------------------------------------------
// Object methods
//----------------------------------------------------------------------------
public:
    virtual RippleEffect* Clone() const;
    virtual ~RippleEffect();

    virtual RESULT Draw                 ( IN Vertex *pVertices, UINT32 numVertices, PRIMITIVE_TYPE type );
    virtual RESULT DrawTriangleStrip    ( IN Vertex *pVertices, UINT32 numVertices                      );
    virtual RESULT DrawTriangleList     ( IN Vertex *pVertices, UINT32 numVertices                      );

    // Custom Effect Properties
    virtual void    SetRows             ( UINT8  rows        );
    virtual void    SetColumns          ( UINT8  columns     );
    virtual void    SetPadding          ( UINT32 pixels      );
    virtual void    SetOrigin           ( const vec2& origin );
    virtual void    SetAmplitude        ( float amplitude    );
    virtual void    SetSpeed            ( float speed        );
    virtual void    SetRadius           ( float radius       );
    virtual void    SetWaveLength       ( float waveLength   );
    virtual void    SetNumWaves         ( float numWaves     );
    virtual void    SetColor            ( const Color& color );

    virtual UINT8   GetRows             ( );
    virtual UINT8   GetColumns          ( );
    virtual UINT32  GetPadding          ( );
    virtual vec2    GetOrigin           ( );
    virtual float   GetAmplitude        ( );
    virtual float   GetSpeed            ( );
    virtual float   GetRadius           ( );
    virtual float   GetWaveLength       ( );
    virtual float   GetNumWaves         ( );
    virtual Color   GetColor            ( );
    
    virtual HShader GetShader           ( );
    
    virtual IProperty* GetProperty      ( const string& name ) const;

protected:
    RippleEffect();
    RippleEffect( const RippleEffect& rhs );
    RippleEffect& operator=( const RippleEffect& rhs );

    virtual RESULT InitScratchSurfaces  ( Rectangle& sourceRect, Rectangle& textureRect );




//----------------------------------------------------------------------------
// Object data
//----------------------------------------------------------------------------
protected:
    Vertex*         m_pRippledVertices;
    UINT32          m_numRippledVertices;

    UINT8           m_rows;
    UINT8           m_columns;
    UINT32          m_padding;
    float           m_fAmplitude;
    vec2            m_origin;
    float           m_fSpeed;
    float           m_fRadius;
    float           m_fWaveLength;
    float           m_fNumWaves;
    Color           m_color;
    

//----------------------------------------------------------------------------
// Class methods
//----------------------------------------------------------------------------
public:
    static RippleEffect*    CreateInstance  ( );

protected:
    static RESULT           Init            ( );
    static RESULT           Deinit          ( );


//----------------
// Class data
//----------------
protected:
    static UINT32                   s_NumInstances;

    // Shaders
    static bool                     s_RippleShadersLoaded;
    static GLuint                   s_RippleShaderProgram;
    static HShader                  s_hRippleShader;

    static const char*              s_RippleVertexShaderName;
    static const char*              s_RippleFragmentShaderName;

    // Shader parameters
    static GLint                    s_uProjectionMatrix;
    static GLint                    s_uModelViewMatrix;
    static GLint                    s_uTexture;
    static GLint                    s_uGlobalColor;
    static GLint                    s_uOrigin;
    static GLint                    s_uAmplitude;
    static GLint                    s_uTime;
    static GLint                    s_uRadius;
    static GLint                    s_uHalfWaveLength;
    static GLint                    s_uNumWaves;
    static GLint                    s_uColor;

    static const UINT8              DEFAULT_ROWS;
    static const UINT8              DEFAULT_COLUMNS;
    static const UINT32             DEFAULT_PADDING;
    static const float              DEFAULT_AMPLITUDE;
    static const float              DEFAULT_ORIGIN_X;
    static const float              DEFAULT_ORIGIN_Y;
    static const float              DEFAULT_SPEED;
    static const float              DEFAULT_RADIUS;
    static const float              DEFAULT_WAVE_LENGTH;
    static const float              DEFAULT_NUM_WAVES;
    static const Color              DEFAULT_COLOR;
    static const float              MAX_FREQUENCY;
    static const float              MAX_AMPLITUDE;
    static const float              MAX_SPEED;


    //
    // Exported Properties for animation/scripting
    //
    static       PropertySet        s_properties;
    
};



} // END namespace Z
