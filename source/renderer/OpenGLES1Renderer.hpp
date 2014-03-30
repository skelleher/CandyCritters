
#include "Object.hpp"
#include "Errors.hpp"
#include "IRenderer.hpp"
#include "PerfTimer.hpp"
#include "TextureManager.hpp"
#include "EffectManager.hpp"


#include <stack>
using std::stack;



namespace Z
{



class OpenGLES1Renderer : virtual public Object, public IRenderer
{
public:
    // Factory method
    static  OpenGLES1Renderer* Create();
    virtual ~OpenGLES1Renderer();
   

    // IRenderer
    virtual RESULT Init                 ( UINT32 width, UINT32 height );
    virtual RESULT Deinit               ( );
    
    virtual RESULT SetRenderContext     ( IN RenderContext* pContext );
    virtual RESULT SetRenderTarget      ( IN RenderTarget*  pTarget  );

    virtual RESULT Resize               ( UINT32 width, UINT32 height );
    virtual RESULT Rotate               ( Orientation orientation );
    virtual UINT32 GetWidth             ( );
    virtual UINT32 GetHeight            ( );
    
    virtual RESULT Clear                ( Color color );
    virtual RESULT Clear                ( float r, float g, float b, float a );

    virtual RESULT EnableAlphaTest      ( bool enabled );
    virtual RESULT EnableAlphaBlend     ( bool enabled );
    virtual RESULT EnableDepthTest      ( bool enabled );
    virtual RESULT EnableLighting       ( bool enabled );
    virtual RESULT EnableTexturing      ( bool enabled );
    virtual RESULT ShowOverdraw         ( bool show    );
    virtual RESULT SetBlendFunctions    ( UINT32 srcFunction, UINT32 dstFunction );
    virtual RESULT SetGlobalColor       ( Color color = Color::White() );

    virtual RESULT BeginFrame           ( );
    virtual RESULT EndFrame             ( );

    virtual RESULT PushEffect           ( IN    HEffect  hEffect             );
    virtual RESULT PopEffect            ( INOUT HEffect* phEffect = NULL     );
    virtual RESULT GetEffect            ( INOUT HEffect* phEffect            );

    virtual RESULT SetTexture           ( IN    UINT8     textureUnit, IN UINT32 textureID   );    // HACK: so Effects can quickly bind temp textures created on-the-fly w/o going through TextureMan
    virtual RESULT SetTexture           ( IN    UINT8     textureUnit, IN HTexture hTexture  );
    virtual RESULT GetTexture           ( INOUT HTexture* phTexture );

    virtual RESULT SetModelViewMatrix   ( IN    const mat4& matrix  );
    virtual RESULT GetModelViewMatrix   ( INOUT       mat4* pMatrix );
    
    virtual RESULT DrawTriangleStrip    ( IN Vertex *pVertices, UINT32 numVertices );
    virtual RESULT DrawTriangleList     ( IN Vertex *pVertices, UINT32 numVertices );
    virtual RESULT DrawLines            ( IN Vertex *pVertices, UINT32 numVertices, float fWidth );
    virtual RESULT DrawPointSprites     ( IN Vertex *pVertices, UINT32 numVertices, float fScale );

protected:
    OpenGLES1Renderer();
    OpenGLES1Renderer(const OpenGLES1Renderer& rhs);
    const OpenGLES1Renderer& operator=(const OpenGLES1Renderer& rhs);

    virtual RESULT SetEffect            ( IN HEffect hEffect, bool beginFrame = true );
    
    void PrintPerfStats();
    void PrintDriverInfo();

    
protected:
    static OpenGLES1Renderer*  s_pInstance;
    
    RenderContext*  m_pRenderContext;
    RenderTarget*   m_pRenderTarget;
    RenderTarget*   m_pDefaultRenderTarget;
    
    typedef stack<HEffect> EffectStack;
    EffectStack     m_effectStack;
    HEffect         m_hDefaultEffect;
    HEffect         m_hCurrentEffect;
    bool            m_currentEffectIsPost;

    HTexture        m_hCurrentTexture;
    GLuint          m_currentTextureID;
    mat4            m_currentModelViewMatrix;

    bool            m_depthTestEnabled;

    bool            m_alphaBlendEnabled;
    UINT32          m_srcBlendFunction;
    UINT32          m_dstBlendFunction;

    Color           m_globalColor; // Used for setting global opacity or global highlite for subsequent draw calls.  E.g. make an entire Layer's contents translucent.

    Orientation     m_orientation;
    UINT32          m_width;
    UINT32          m_height;
    
    PerfTimer       m_perfTimer;
    UINT64          m_lastRenderTickCount;
    UINT32          m_framesRendered;
    float           m_framesPerSecond;
    UINT32          m_currentDrawCalls;
    UINT32          m_totalDrawCalls;
    float           m_drawsPerSecond;
   
    // TEST
    UINT16          m_currentAngleDegrees;
};



} // END namespace Z

