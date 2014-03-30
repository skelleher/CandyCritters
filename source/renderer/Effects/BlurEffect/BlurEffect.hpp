#pragma once


#include "OpenGLESEffect.hpp"


namespace Z
{



class BlurEffect : public OpenGLESEffect
{
//----------------------------------------------------------------------------
// Object methods
//----------------------------------------------------------------------------
public:
    virtual BlurEffect* Clone() const;
    virtual ~BlurEffect();

    virtual RESULT Draw                 ( IN Vertex *pVertices, UINT32 numVertices, PRIMITIVE_TYPE type );
    virtual RESULT DrawTriangleStrip    ( IN Vertex *pVertices, UINT32 numVertices                      );
    virtual RESULT DrawTriangleList     ( IN Vertex *pVertices, UINT32 numVertices                      );

    // Custom Effect Properties
    virtual void   SetRadius( float radius );
    virtual float  GetRadius();

    virtual HShader GetShader           ( );
    
    virtual IProperty* GetProperty      ( const string& name ) const;

protected:
    BlurEffect();
    BlurEffect( const BlurEffect& rhs );
    BlurEffect& operator=( const BlurEffect& rhs );

    virtual RESULT InitScratchSurfaces  ( Rectangle& sourceRect, float blurRadius, float uStart = 0.0f, float uEnd = 1.0f, float vStart = 0.0f, float vEnd = 1.0f );
    virtual RESULT InitBlurFilter       ( float blurRadius );


//----------------------------------------------------------------------------
// Object data
//----------------------------------------------------------------------------
protected:
    Rectangle               m_frame;
    Vertex*                 m_pFrameVertices;
    UINT32                  m_numFrameVertices;
    
    float                   m_fBlurRadius;
    float                   m_fPreviousBlurRadius;

    float                   m_fDownsamplingFactor;
    float                   m_fPreviousDownsamplingFactor;
    float                   m_fBlurEdgePadding;
    float                   m_fPaddedWidth;
    float                   m_fPaddedHeight;
    float                   m_fWidth;
    float                   m_fHeight;

    Vertex*                 m_pPreviousVertexArray;

    UINT32                  m_FrameCount;
    RenderTarget*           m_pOffScreenFrameBuffer;
    Vertex*                 m_pPaddedBlurVertices;
    UINT32                  m_cPaddedBlurVertices;
    Vertex*                 m_pUnpaddedBlurVertices;
    UINT32                  m_cUnpaddedBlurVertices;
    Vertex*                 m_pBlurredVertices;
    UINT32                  m_cBlurredVertices;
    UINT32                  m_TextureA;
    UINT32                  m_TextureB;

    mat4                    m_modelViewMatrixOffscreen;
    mat4                    m_projectionMatrixOffscreen;


    float                   m_HorizontalFilterOffsets[7];
    float                   m_VerticalFilterOffsets[7];


//----------------------------------------------------------------------------
// Class methods
//----------------------------------------------------------------------------
public:
    static BlurEffect*  CreateInstance  ( );

protected:
    static RESULT Init   ( );
    static RESULT Deinit ( );

    static void   CreateGaussianFilterKernel7x1( IN float filterOffsets[], float offset );
    static void   CreateGaussianFilterKernel3x1( IN float filterOffsets[], float offset );
    static float  ChooseDownsamplingFactor( float blurRadius );


//----------------
// Class data
//----------------
protected:
    static UINT32               s_NumInstances;

    // Shaders
    static bool                 s_BlurShadersLoaded;
    static GLuint               s_BlurShaderProgram;
    static HShader              s_hBlurShader;

    static const char*          s_BlurVertexShaderName;
    static const char*          s_BlurFragmentShaderName;

    // Shader parameters
    static GLint                s_uProjectionMatrix;
    static GLint                s_uModelViewMatrix;
    static GLint                s_uTexture;
    static GLint                s_uGlobalColor;
    static GLint                s_uFilterOffsets;
    static GLint                s_uFilterWeights;
    static GLint                s_uHorizontalPass;

    static const int            MAX_FILTER_WIDTH_HEIGHT;
    static float                s_FilterWeights[];

    static const float          DEFAULT_BLUR_RADIUS;
    static const float          MAX_BLUR_RADIUS;



    //
    // Exported Properties for animation/scripting
    //
    static       PropertySet        s_properties;
    
};



} // END namespace Z
