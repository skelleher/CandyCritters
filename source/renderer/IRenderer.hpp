#pragma once

#include "Object.hpp"
#include "Color.hpp"
#include "Matrix.hpp"
#include "Vertex.hpp"
#include "RenderContext.hpp"
#include "RenderTarget.hpp"
//#include "IEffect.hpp"
#include "EffectManager.hpp"

//class IEffect;
//class HEffect;

namespace Z 
{



class IRenderer : virtual public IObject
{
public:
    // This must match UIDeviceOrientation on iOS
    typedef enum
    {
        OrientationUnknown,
        OrientationPortrait,
        OrientationPortraitUpsideDown,
        OrientationLandscapeLeft,
        OrientationLandscapeRight,
        OrientationFaceUp,
        OrientationFaceDown,
    } Orientation;
    
    
public:
    virtual RESULT Init                 ( UINT32 width, UINT32 height )                                     = 0;
    virtual RESULT Deinit               ( )                                                                 = 0;
    
    virtual RESULT SetRenderContext     ( IN RenderContext* pContext )                                      = 0;
    virtual RESULT SetRenderTarget      ( IN RenderTarget*  pTarget  )                                      = 0;
    
    virtual RESULT Resize               ( UINT32 width, UINT32 height )                                     = 0;
    virtual RESULT Rotate               ( Orientation orientation )                                         = 0;
    virtual UINT32 GetWidth             ( )                                                                 = 0;
    virtual UINT32 GetHeight            ( )                                                                 = 0;

    virtual RESULT Clear                ( Color  color )                                                    = 0;
    virtual RESULT Clear                ( float r, float g, float b, float a )                              = 0;
  
    virtual RESULT EnableAlphaTest      ( bool enabled )                                                    = 0;
    virtual RESULT EnableAlphaBlend     ( bool enabled )                                                    = 0;
    virtual RESULT EnableDepthTest      ( bool enabled )                                                    = 0;
    virtual RESULT EnableLighting       ( bool enabled )                                                    = 0;
    virtual RESULT EnableTexturing      ( bool enabled )                                                    = 0;
    virtual RESULT ShowOverdraw         ( bool show    )                                                    = 0;
    virtual RESULT SetBlendFunctions    ( UINT32 srcFunction, UINT32 dstFunction )                          = 0;
    virtual RESULT SetGlobalColor       ( Color color  )                                                    = 0;


    virtual RESULT BeginFrame           ( )                                                                 = 0;
    virtual RESULT EndFrame             ( )                                                                 = 0;

    virtual RESULT PushEffect           ( IN    HEffect  hEffect             )                              = 0;
    virtual RESULT PopEffect            ( INOUT HEffect* phEffect = NULL     )                              = 0;
    virtual RESULT GetEffect            ( INOUT HEffect* phEffect            )                              = 0;

    virtual RESULT SetTexture           ( IN    UINT8     textureUnit, IN HTexture hTexture )               = 0;
    virtual RESULT SetTexture           ( IN    UINT8     textureUnit, IN UINT32 textureID  )               = 0;    // HACK: so Effects can quickly bind temp textures created on-the-fly w/o going through TextureMan
    virtual RESULT GetTexture           ( INOUT HTexture* phTexture )                                       = 0;

    virtual RESULT SetModelViewMatrix   ( IN    const mat4& matrix  )                                       = 0;
    virtual RESULT GetModelViewMatrix   ( INOUT       mat4* pMatrix )                                       = 0;
    
    virtual RESULT DrawTriangleStrip    ( IN Vertex *pVertices, UINT32 numVertices                      )   = 0;
    virtual RESULT DrawTriangleList     ( IN Vertex *pVertices, UINT32 numVertices                      )   = 0;
    virtual RESULT DrawLines            ( IN Vertex *pVertices, UINT32 numVertices, float fWidth = 1.0f )   = 0;
    virtual RESULT DrawPointSprites     ( IN Vertex *pVertices, UINT32 numVertices, float fScale = 8.0f )   = 0;    // TODO: deprecate in favor of PointSpriteVertex w/o tex coords.
    
//    virtual RESULT ScreenPointToRenderer( IN const vec2 inPoint, OUT vec2& outPoint ) = 0;

    virtual ~IRenderer()    {};
};



} // END namespace Z

