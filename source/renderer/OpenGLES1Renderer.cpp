/*
 *  OpenGLES1Renderer.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 9/4/10.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>

#include "Engine.hpp"
#include "OpenGLES1Renderer.hpp"
#include "Log.hpp"
#include "Macros.hpp"
#include "Platform.hpp"

// TEST:
#include "SpriteManager.hpp"
#include "AnimationManager.hpp"
#include "GameObjectManager.hpp"
#include "StoryboardManager.hpp"


namespace Z
{


    
//
// Static Data
//
OpenGLES1Renderer*  OpenGLES1Renderer::s_pInstance = NULL;

#ifdef SHIPBUILD
    #define PRINT_PERF_INTERVAL_SECONDS 30.0
#else
    #define PRINT_PERF_INTERVAL_SECONDS 5.0
#endif


//
// Class Methods
//
OpenGLES1Renderer* 
OpenGLES1Renderer::Create()
{
    if (!s_pInstance)
    {
        DEBUGMSG(ZONE_INFO, "OpenGLES1Renderer::Create()");
        s_pInstance = new OpenGLES1Renderer();
        DEBUGCHK(s_pInstance);
    }
    
    //s_pInstance->AddRef();
    
    return s_pInstance;
}



//
// Instance Methods
//
OpenGLES1Renderer::OpenGLES1Renderer() :
    m_pRenderContext(NULL),
    m_pRenderTarget(NULL),
    m_pDefaultRenderTarget(NULL),
    m_orientation(OrientationUnknown),
    m_width(0),
    m_height(0),
    m_lastRenderTickCount(0),
    m_framesRendered(0),
    m_framesPerSecond(0),
    m_currentDrawCalls(0),
    m_totalDrawCalls(0),
    m_drawsPerSecond(0),
    m_currentAngleDegrees(0),
    m_depthTestEnabled(false),
    m_currentTextureID(0xFFFFFFFF),
    m_currentEffectIsPost(false),
    m_alphaBlendEnabled(false),
// For Premultiplied Alpha (iPhone default):
    m_srcBlendFunction(GL_ONE),
    m_dstBlendFunction(GL_ONE_MINUS_SRC_ALPHA),
    m_globalColor(Color::White())
{
    RETAILMSG(ZONE_INFO, "OpenGLES1Renderer: m_ID: %d", m_ID);
    
    m_name = "OpenGLES1Renderer";
}



OpenGLES1Renderer::~OpenGLES1Renderer()
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "\t~OpenGLES1Renderer( %4d )", m_ID);
    Deinit();
}




RESULT
OpenGLES1Renderer::Init( UINT32 width, UINT32 height )
{
    RESULT rval = S_OK;
    
    if (!width || !height)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: OpenGLES2Renderer::Init( %d, %d ): invalid argument.", width, height);
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    DEBUGMSG(ZONE_INFO, "OpenGLES1Renderer::Init( %d x %d )", width, height);

    m_width             = width;
    m_height            = height;
    

    if (!m_pDefaultRenderTarget)
    {
        m_pDefaultRenderTarget = RenderTarget::Create( width, height, false, 0 );
        CPR(m_pDefaultRenderTarget);
    }
  


    PrintDriverInfo();
    

    //
    // Set the default renderer settings.
    //
    EnableTexturing ( true );
    EnableDepthTest ( true );
    EnableAlphaBlend( true );
    
    CHR(Resize( width, height ));
    
    CHR(Renderer.SetModelViewMatrix( GameCamera.GetViewMatrix() ));

    m_perfTimer.Start();
    
Exit:    
    return rval;
}



RESULT
OpenGLES1Renderer::SetRenderContext( IN RenderContext* pContext )
{
    RESULT rval = S_OK;
    
    DEBUGMSG(ZONE_INFO, "OpenGLES1Renderer::SetRenderContext( 0x%x )", pContext);

    if (m_pRenderContext != pContext)
    {
        SAFE_RELEASE(m_pRenderContext);

        m_pRenderContext = pContext;
        m_pRenderContext->AddRef();
        
        CHR(m_pRenderContext->Bind());
        
        // TODO: need to re-bind the RenderTarget?
    }

Exit:
    return rval;
}



RESULT
OpenGLES1Renderer::SetRenderTarget( IN RenderTarget* pTarget )
{
    RESULT rval = S_OK;
    
    DEBUGMSG(ZONE_RENDER, "OpenGLES1Renderer::SetRenderTarget( 0x%x )", pTarget);

    if (NULL == pTarget)
    {
        pTarget = m_pDefaultRenderTarget;
    }

    if (m_pRenderTarget != pTarget)
    {
        SAFE_RELEASE(m_pRenderTarget);

        m_pRenderTarget = pTarget;
        m_pRenderTarget->AddRef();
        
        CHR(m_pRenderTarget->Enable());

        // Reset the modelview and projection matrices
        CHR(Resize( m_width, m_height ));
        CHR(SetModelViewMatrix( GameCamera.GetViewMatrix() ));
    }

Exit:
    return rval;
}



RESULT
OpenGLES1Renderer::Deinit()
{
    RESULT rval = S_OK;
    
    DEBUGMSG(ZONE_INFO, "OpenGLES1Renderer::Deinit()");
/*
    MaterialMan.Release(m_hCurrentMaterial);
    MaterialMan.Release(m_hDefaultMaterial);
*/
    EffectMan.Release(m_hCurrentEffect);
    EffectMan.Release(m_hDefaultEffect);

    if (m_pRenderTarget)
    {
        IGNOREHR(m_pRenderTarget->Disable());
        SAFE_RELEASE(m_pRenderTarget);
    }
        
    if (m_pDefaultRenderTarget && m_pDefaultRenderTarget != m_pRenderTarget)
    {
        IGNOREHR(m_pDefaultRenderTarget->Disable());
        SAFE_RELEASE(m_pDefaultRenderTarget);
    }

    SAFE_RELEASE(m_pRenderContext);    

Exit:    
    return rval;
}



RESULT 
OpenGLES1Renderer::Resize( UINT32 width, UINT32 height )
{
    RETAILMSG(ZONE_RENDER, "OpenGLES1Renderer::Resize( %d x %d )", width, height);

    m_width     = width;
    m_height    = height;

    
    VERIFYGL(glViewport(0, 0, width, height));

    
    VERIFYGL(glMatrixMode(GL_PROJECTION));
    VERIFYGL(glLoadMatrixf( GameCamera.GetProjectionMatrix().Pointer() ));
    
//    VERIFYGL(glMatrixMode(GL_MODELVIEW));
//    VERIFYGL(glLoadIdentity()); 
    
Exit:    
    return S_OK;
}



RESULT
OpenGLES1Renderer::Rotate( Orientation orientation )
{
    DEBUGMSG(ZONE_RENDER, "OpenGLES1Renderer::Rotate( %d )", orientation);
    m_orientation = orientation;
    
    return S_OK;
}



RESULT 
OpenGLES1Renderer::Clear( Color color )
{
    return Clear( color.floats.r, color.floats.g, color.floats.b, color.floats.a );
}




RESULT
OpenGLES1Renderer::Clear( float r, float g, float b, float a)
{
    RESULT rval = S_OK;

    DEBUGMSG(ZONE_RENDER | ZONE_VERBOSE, "OpenGLES1Renderer::Clear( %1.1f, %1.1f, %1.1f, %1.1f )", r, g, b, a);

    VERIFYGL(glClearColor(r, g, b, a));
    if (m_depthTestEnabled)
    {
        VERIFYGL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }
    else 
    {
        VERIFYGL(glClear(GL_COLOR_BUFFER_BIT));
    }
    
Exit:
    return rval;
}



RESULT
OpenGLES1Renderer::EnableAlphaTest( bool enabled )
{
    RESULT rval = S_OK;
    
    m_alphaBlendEnabled = enabled;
    
    if (enabled)
    {
        //
        // Don't draw transparent pixels.
        // This is much faster than Alpha Blending.
        //
        VERIFYGL(glEnable(GL_ALPHA_TEST));
        VERIFYGL(glAlphaFunc( GL_GREATER, 0.0 ));
    }
    else 
    {
        VERIFYGL(glDisable(GL_ALPHA_TEST));
    }

Exit:
    return rval;
}



RESULT
OpenGLES1Renderer::SetBlendFunctions( UINT32 srcFunction, UINT32 dstFunction )
{
    RESULT rval = S_OK;
    
    m_srcBlendFunction = srcFunction;
    m_dstBlendFunction = dstFunction;
    
    if (m_alphaBlendEnabled)
    {
        VERIFYGL(glBlendFunc(m_srcBlendFunction, m_dstBlendFunction));
    }

Exit:
    return rval;
}



RESULT
OpenGLES1Renderer::SetGlobalColor( Color color )
{
    RESULT rval = S_OK;
    
    m_globalColor = color;
    VERIFYGL(glColor4f(m_globalColor.floats.r, m_globalColor.floats.g, m_globalColor.floats.b, m_globalColor.floats.a));
    
Exit:
    return rval;
}



RESULT
OpenGLES1Renderer::EnableAlphaBlend( bool enabled )
{
    RESULT rval = S_OK;
    
    m_alphaBlendEnabled = enabled;
    
    if (enabled)
    {
        VERIFYGL(glEnable(GL_BLEND));

        if (Platform::IsSimulator())
        {
            // Alpha blending broken on the simulator?
//            VERIFYGL(glBlendFunc(m_srcBlendFunction, m_dstBlendFunction));
            VERIFYGL(glBlendFunc(GL_SRC_ALPHA, m_dstBlendFunction));
        }
        else 
        {
            // For Premultiplied Alpha:
//            VERIFYGL(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
            VERIFYGL(glBlendFunc(m_srcBlendFunction, m_dstBlendFunction));
        }
    }
    else
    {
        VERIFYGL(glDisable(GL_BLEND));
    }
    
Exit:
    return rval;
}


RESULT
OpenGLES1Renderer::EnableDepthTest( bool enabled )
{
    RESULT rval = S_OK;
    
    if (enabled)
    {
        VERIFYGL(glEnable(GL_DEPTH_TEST));
    }
    else
    {
        VERIFYGL(glDisable(GL_DEPTH_TEST));
    }
    
    m_depthTestEnabled = enabled;
    
Exit:
    return rval;
}



RESULT
OpenGLES1Renderer::EnableLighting( bool enabled )
{
    RESULT rval = S_OK;
    
    if (enabled)
    {
        VERIFYGL(glEnable(GL_LIGHTING));
    }
    else
    {
        VERIFYGL(glDisable(GL_LIGHTING));
    }
    
Exit:
    return rval;
}



RESULT
OpenGLES1Renderer::EnableTexturing( bool enabled )
{
    RESULT rval = S_OK;
    
    if (enabled)
    {
        VERIFYGL(glEnable(GL_TEXTURE_2D));
    }
    else
    {
        VERIFYGL(glDisable(GL_TEXTURE_2D));
    }
    
Exit:
    return rval;
}



RESULT
OpenGLES1Renderer::BeginFrame()
{
    RESULT rval = S_OK;
    
    //
    // HACK HACK:
    //
    static bool firstFrame = true;
    if (firstFrame)
    {
        //
        // Load default Effect
        //
        IGNOREHR(EffectMan.Get( "DefaultEffect", &m_hDefaultEffect ));
        IGNOREHR(SetEffect( m_hDefaultEffect ));
        
        //
        // Bind to the render target, which may be an off-screen texture.
        //
        CHR(SetRenderTarget( m_pDefaultRenderTarget ));

        firstFrame = false;
    }

    VERIFYGL(glMatrixMode(GL_PROJECTION));
    VERIFYGL(glLoadMatrixf( GameCamera.GetProjectionMatrix().Pointer() ));

Exit:
    return rval;
}



RESULT
OpenGLES1Renderer::EndFrame()
{
    RESULT rval = S_OK;
    
    DEBUGCHK(m_pRenderContext);

#ifndef SHIPBUILD
    if (Log::IsZoneEnabled(ZONE_PERF))
    {
        // TODO: multiply the screen rect by the camera's scale factor 
        // to get proper on-screen coordinates independant of device resolution.

        char buf[32];
        sprintf(buf, "FPS: %d", (int)m_framesPerSecond );
        FontMan.Draw( vec2( 40, 0 ), &buf[0] );
    }
#endif

    CHR(m_pRenderContext->Present());

    m_framesRendered++;
    
    if (Log::IsZoneEnabled(ZONE_PERF))
    {
        PrintPerfStats();
    }
    
Exit:
    return rval;
}




#pragma mark -
#pragma mark Effect
RESULT
OpenGLES1Renderer::PushEffect( IN HEffect hEffect  )
{
    RESULT rval = S_OK;

    // Setting a NULL effect should be equivalent to DefaultEffect.
    if (hEffect.IsNull())
        hEffect = m_hDefaultEffect;


    if ( !hEffect.IsNull() )
    {
        CHR(EffectMan.AddRef( hEffect ));
    }
    
    // Push onto the stack
    m_effectStack.push( hEffect );

    CHR(SetEffect( hEffect, true ));
    
Exit:
    return rval;
}



RESULT
OpenGLES1Renderer::PopEffect( INOUT HEffect* phEffect )
{
    RESULT    rval = S_OK;
    HEffect   hOldEffect;
    
    CBR( m_effectStack.size() > 0 );

    hOldEffect = m_effectStack.top();
    if ( !hOldEffect.IsNull() )
    {
        // Finish and present any prior Effect
        CHR(Effects.EndFrame( hOldEffect, true ));
        CHR(EffectMan.Release( hOldEffect ));
    }
    

    // Pop from the stack
    m_effectStack.pop();

    if (phEffect)
    {
        *phEffect = hOldEffect;
    }

    if ( m_effectStack.size() > 0 )
    {
        CHR(SetEffect( m_effectStack.top(), false ));
    }
    else 
    {
        CHR(SetEffect( m_hDefaultEffect, false ));
    }


Exit:
    return rval;
}



RESULT
OpenGLES1Renderer::SetEffect( IN HEffect hEffect, bool beginFrame )
{
    RESULT rval = S_OK;
    
    // Begin drawing with the new Effect
    if (m_hCurrentEffect != hEffect)
    {
        // Don't take a reference to the Effect.
        m_hCurrentEffect = hEffect;
        
        m_currentEffectIsPost = EffectMan.IsPostEffect( hEffect );
        
        // Track number of draw calls made with the current Effect.
        m_totalDrawCalls   += m_currentDrawCalls;
        m_currentDrawCalls = 0;
        
#ifdef DEBUG
        string name;
        CHR(EffectMan.GetName( hEffect, &name ));
        DEBUGMSG(ZONE_SHADER, "OpenGLES1Renderer::SetEffect( %s ) IsPost: %d", name.c_str(), m_currentEffectIsPost);
#endif        
        
        // TODO: caller should pass the frame; we shouldn't assume full-screen.
        Rectangle frame;
        Platform::GetScreenRectCamera( &frame );
        
        if (beginFrame)
        {
            CHR(Effects.BeginFrame( hEffect, frame ));
        }
        CHR(Effects.SetTexture( hEffect, 0, m_hCurrentTexture ));
        
        
        {
            mat4  world       = GameCamera.GetViewMatrix();
            mat4  projection  = GameCamera.GetProjectionMatrix();
            
            CHR(Effects.SetModelViewMatrix ( hEffect, world      ));
            CHR(Effects.SetProjectionMatrix( hEffect, projection ));
        }
    }
    
    
Exit:
    if (FAILED(rval))
        RETAILMSG(ZONE_ERROR, "ERROR: OpenGLES1Renderer::SetEffect() rval = %d", rval);
    
    return rval;
}



RESULT
OpenGLES1Renderer::GetEffect( INOUT HEffect* phEffect )
{
    RESULT rval = S_OK;

    CPREx(phEffect, E_NULL_POINTER);
    *phEffect = m_hCurrentEffect;

Exit:
    return rval;
}



#pragma mark -
#pragma mark Texture
RESULT
OpenGLES1Renderer::SetTexture( IN UINT8 textureUnit, IN HTexture hTexture )
{
    RESULT rval = S_OK;
    
    TextureInfo textureInfo;
    string      textureName;
    CHR(TextureMan.GetInfo( hTexture, &textureInfo ));
    CHR(TextureMan.GetName( hTexture, &textureName ));

    DEBUGMSG(ZONE_TEXTURE | ZONE_VERBOSE, "OpenGLES1Renderer::SetTexture( %d, %s )", textureUnit, textureName.c_str());
    
    switch (textureUnit)
    {
        case 0:
            VERIFYGL(glActiveTexture(GL_TEXTURE0));
            break;
        case 1:
            VERIFYGL(glActiveTexture(GL_TEXTURE1));
            break;
        default:
            RETAILMSG(ZONE_WARN, "WARNING: OpenGLES1Renderer::SetTexture( %d, %d ): we only support two texture units for now.",
                      textureUnit, (UINT32)hTexture);
            CHR(E_INVALID_ARG);
    }
    
    
    // Don't rebind the texture if it was already set.
    // The driver /shouldn't/ stall in this case, but we don't know
    // for certain.
    if (textureInfo.textureID != m_currentTextureID)
    {
        m_hCurrentTexture  = hTexture;
        m_currentTextureID = textureInfo.textureID;
        VERIFYGL(glBindTexture(GL_TEXTURE_2D, textureInfo.textureID));
    }
    
Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "OpenGLES1Renderer::SetTexture( %d, %d ): bad handle", textureUnit, (UINT32)hTexture);
    }

    return rval;
}



RESULT
OpenGLES1Renderer::SetTexture( IN UINT8 textureUnit, IN UINT32 textureID )
{
    RESULT rval = S_OK;
    
    DEBUGMSG(ZONE_TEXTURE | ZONE_VERBOSE, "OpenGLES1Renderer::SetTexture( %d, %d )", textureUnit, textureID);
    
    switch (textureUnit)
    {
        case 0:
            VERIFYGL(glActiveTexture(GL_TEXTURE0));
            break;
        case 1:
            VERIFYGL(glActiveTexture(GL_TEXTURE1));
            break;
        default:
            RETAILMSG(ZONE_WARN, "WARNING: OpenGLES1Renderer::SetTexture( %d, %d ): we only support two texture units for now.",
                      textureUnit, textureID);
            CHR(E_INVALID_ARG);
    }
    
    
    m_currentTextureID = textureID;
    VERIFYGL(glBindTexture(GL_TEXTURE_2D, textureID));
    
Exit:
    return rval;
}



RESULT
OpenGLES1Renderer::GetTexture( INOUT HTexture* phTexture )
{
    RESULT rval = S_OK;

    CPREx(phTexture, E_NULL_POINTER);
    
    *phTexture = m_hCurrentTexture;

Exit:
    return rval;
}



#pragma mark -
#pragma mark Matrix
RESULT
OpenGLES1Renderer::SetModelViewMatrix( IN const mat4& matrix )
{
    RESULT rval = S_OK;
    
    m_currentModelViewMatrix = matrix;
    
    VERIFYGL(glMatrixMode(GL_MODELVIEW));
    VERIFYGL(glLoadMatrixf(m_currentModelViewMatrix.Pointer()));
    
Exit:
    return rval;
}



RESULT
OpenGLES1Renderer::GetModelViewMatrix( INOUT mat4* pMatrix )
{
    RESULT rval = S_OK;

    CPREx(pMatrix, E_NULL_POINTER);

    *pMatrix = m_currentModelViewMatrix;
    
Exit:
    return rval;
}



#pragma mark -
#pragma mark Drawing
RESULT
OpenGLES1Renderer::DrawTriangleStrip( IN Vertex *pVertices, UINT32 numVertices )
{
    RESULT rval = S_OK;

    CPREx(pVertices, E_NULL_POINTER);
    
    if (0 == numVertices)
    {
        RETAILMSG(ZONE_WARN, "WARNING: OpenGLES1Renderer::DrawTriangleStrip(): numVertices == 0");
        //return E_INVALID_ARG;
        return S_OK;
    }    
    
    //
    // Enable vertex fields
    //
    VERIFYGL(glEnableClientState(GL_VERTEX_ARRAY));
    VERIFYGL(glEnableClientState(GL_COLOR_ARRAY));
    VERIFYGL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
    
    VERIFYGL(glVertexPointer  (3, GL_FLOAT,         sizeof(Vertex), &pVertices->position[0]));
    VERIFYGL(glColorPointer   (4, GL_UNSIGNED_BYTE, sizeof(Vertex), &pVertices->color));
    VERIFYGL(glTexCoordPointer(2, GL_FLOAT,         sizeof(Vertex), &pVertices->texCoord[0]));

    //
    // Draw triangles
    //
    VERIFYGL(glDrawArrays(GL_TRIANGLE_STRIP, 0, numVertices));
    m_currentDrawCalls++;
    
    
Exit:
    return rval;
}



RESULT
OpenGLES1Renderer::DrawTriangleList( IN Vertex *pVertices, UINT32 numVertices )
{
    RESULT rval = S_OK;
    
    CPREx(pVertices, E_NULL_POINTER);
    
    if (0 == numVertices)
    {
        RETAILMSG(ZONE_WARN, "WARNING: OpenGLES1Renderer::DrawTriangleStrip(): numVertices == 0");
        return S_OK;
    }    
    
    //
    // Enable vertex fields
    //
    VERIFYGL(glEnableClientState(GL_VERTEX_ARRAY));
    VERIFYGL(glEnableClientState(GL_COLOR_ARRAY));
    VERIFYGL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
    
    VERIFYGL(glVertexPointer  (3, GL_FLOAT,         sizeof(Vertex), &pVertices->position[0]));
    VERIFYGL(glColorPointer   (4, GL_UNSIGNED_BYTE, sizeof(Vertex), &pVertices->color));
    VERIFYGL(glTexCoordPointer(2, GL_FLOAT,         sizeof(Vertex), &pVertices->texCoord[0]));
    
    VERIFYGL(glDrawArrays(GL_TRIANGLES, 0, numVertices));
    m_currentDrawCalls++;
    
    
Exit:
    return rval;
}



RESULT 
OpenGLES1Renderer::DrawPointSprites( IN Vertex *pVertices, UINT32 numVertices, float fScale )
{
    RESULT rval = S_OK;
    
    CPREx(pVertices, E_NULL_POINTER);
    
    if (0 == numVertices)
    {
        RETAILMSG(ZONE_WARN, "WARNING: OpenGLES1Renderer::DrawPointSprites(): numVertices == 0");
        //return E_INVALID_ARG;
        return S_OK;
    }    

    
    //
    // Enable vertex fields
    //
    VERIFYGL(glEnableClientState(GL_VERTEX_ARRAY));
    VERIFYGL(glEnableClientState(GL_COLOR_ARRAY));
    VERIFYGL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
    
    VERIFYGL(glVertexPointer  (3, GL_FLOAT,         sizeof(Vertex), &pVertices->position[0]));
    VERIFYGL(glColorPointer   (4, GL_UNSIGNED_BYTE, sizeof(Vertex), &pVertices->color));
    VERIFYGL(glTexCoordPointer(2, GL_FLOAT,         sizeof(Vertex), &pVertices->texCoord[0]));
    
    //
    // Draw point sprites
    //
    VERIFYGL(glDrawArrays(GL_POINTS, 0, numVertices));
    m_currentDrawCalls++;
    
    
Exit:
    return rval;
}



RESULT
OpenGLES1Renderer::DrawLines( IN Vertex *pVertices, UINT32 numVertices, float fWidth )
{
    RESULT rval = S_OK;
    
    CPREx(pVertices,        E_NULL_POINTER);
    CBREx(fWidth > 0,       E_INVALID_ARG);
    CBREx(numVertices > 0,  E_INVALID_ARG);
    

    //
    // Enable vertex fields
    //
    VERIFYGL(glEnableClientState(GL_VERTEX_ARRAY));
    VERIFYGL(glEnableClientState(GL_COLOR_ARRAY));
    
    VERIFYGL(glVertexPointer  (3, GL_FLOAT,         sizeof(Vertex), &pVertices->position[0]));
    VERIFYGL(glColorPointer   (4, GL_UNSIGNED_BYTE, sizeof(Vertex), &pVertices->color));
    
    
    //
    // Draw lines
    //
    VERIFYGL(glLineWidth(fWidth));
    VERIFYGL(glDrawArrays(GL_LINES, 0, numVertices));
    m_currentDrawCalls++;
    
    
Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: OpenGLES2Renderer::DrawLines(): rval = 0x%x", rval);
    }
    
    return rval;
}



UINT32
OpenGLES1Renderer::GetWidth()
{
    return m_width;
}



UINT32
OpenGLES1Renderer::GetHeight()
{
    return m_height;
}



RESULT
OpenGLES1Renderer::ShowOverdraw( bool show )
{
    RESULT rval = S_OK;
    
    if (show)
    {
        VERIFYGL(glEnable(GL_BLEND));
        VERIFYGL(glBlendFunc(GL_ONE, GL_ONE));
    }
    else 
    {
        VERIFYGL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    }

    
Exit:
    return rval;
}



void
OpenGLES1Renderer::PrintPerfStats()
{
    double elapsedSeconds = m_perfTimer.ElapsedSeconds();
    
    if (elapsedSeconds >= PRINT_PERF_INTERVAL_SECONDS)
    {
        UINT32 freeRAM = Platform::GetAvailableMemory();
        UINT32 usedRAM = Platform::GetProcessUsedMemory();
        
        m_framesPerSecond = m_framesRendered / elapsedSeconds;
        m_drawsPerSecond  = m_totalDrawCalls / elapsedSeconds;
        
        RETAILMSG(ZONE_INFO, "FPS: %2.2f DPS: %2.2f Free: %d MB Used: %d MB", m_framesPerSecond, m_drawsPerSecond, freeRAM / 1048576, usedRAM / 1048576);
        
        // Restart the timer
        m_perfTimer.Start();
        m_framesRendered = 0;
        m_totalDrawCalls = 0;
    }
}



void
OpenGLES1Renderer::PrintDriverInfo()
{
    RETAILMSG(ZONE_INFO, "GL_VENDOR       = %s",   glGetString(GL_VENDOR));
    RETAILMSG(ZONE_INFO, "GL_RENDERER     = %s",   glGetString(GL_RENDERER));
    RETAILMSG(ZONE_INFO, "GL_VERSION      = %s\n", glGetString(GL_VERSION));
    
    GLint size = 0;
    VERIFYGL(glGetIntegerv( GL_MAX_TEXTURE_SIZE, &size ));
    RETAILMSG(ZONE_INFO, "GL_MAX_TEXTURE_SIZE = %d\n", size);

    {
        string extensionsCopy = (const char*)glGetString(GL_EXTENSIONS);
        char*  pExtensions = &extensionsCopy[0];
        char** ppExtension = &pExtensions;
        
        RETAILMSG(ZONE_INFO, "Extensions:");
        while (*ppExtension)
        {
            RETAILMSG(ZONE_INFO, "%s", strsep(ppExtension, " "));
        }
    }
    
Exit:
    return;
}


/******************
void 
OpenGLES1Renderer::PrintConfigInfo()
{
    RESULT hr = S_OK;
    
    EGLint totalConfigs = 0;
    EGLConfig configs[32];
    
    
    VERIFYEGL(eglGetConfigs(eglGetDisplay(), configs, 32, &totalConfigs));
    
    bool bFoundConfig = false;
    for (int i = 0; i < totalConfigs; i++)
    {
        if (configs[i] == config)
        {
            bFoundConfig = true;
            break;
        }
    }
    
    if (!bFoundConfig)
        CHR(E_FAIL);
    
    
    EGLConfig configId;
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_CONFIG_ID, (EGLint*)&configId));
    
    EGLint red=0, blue=0, green=0, alpha=0;
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_RED_SIZE,   &red));
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_GREEN_SIZE, &green));
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_BLUE_SIZE,  &blue));
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_ALPHA_SIZE, &alpha));
    RETAILMSG(ZONE_INFO, "config[%d]: r%db%dg%da%d", configId, red, green, blue, alpha);
    
    EGLint value;
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_BUFFER_SIZE,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_BUFFER_SIZE = %d", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_LUMINANCE_SIZE,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_LUMINANCE_SIZE = %d", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_ALPHA_MASK_SIZE,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_ALPHA_MASK_SIZE = %d", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_BIND_TO_TEXTURE_RGB,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_BIND_TO_TEXTURE_RGB = %d", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_BIND_TO_TEXTURE_RGBA,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_BIND_TO_TEXTURE_RGBA = %d", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_COLOR_BUFFER_TYPE,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_COLOR_BUFFER_TYPE = 0x%x", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_CONFIG_CAVEAT,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_CONFIG_CAVEAT = 0x%x", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_CONFORMANT,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_CONFORMANT = %d", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_DEPTH_SIZE,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_DEPTH_SIZE = %d", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_LEVEL,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_LEVEL = %d", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_MAX_PBUFFER_WIDTH,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_MAX_PBUFFER_WIDTH = %d", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_MAX_PBUFFER_HEIGHT,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_MAX_PBUFFER_HEIGHT = %d", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_MAX_PBUFFER_PIXELS,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_MAX_PBUFFER_PIXELS = %d", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_MAX_SWAP_INTERVAL,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_MAX_SWAP_INTERVAL = %d", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_MIN_SWAP_INTERVAL,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_MIN_SWAP_INTERVAL = %d", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_NATIVE_RENDERABLE,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_NATIVE_RENDERABLE = %d", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_NATIVE_VISUAL_ID,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_NATIVE_VISUAL_ID = %d", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_NATIVE_VISUAL_TYPE,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_NATIVE_VISUAL_TYPE = %d", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_RENDERABLE_TYPE,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_RENDERABLE_TYPE = %d", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_SURFACE_TYPE,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_SURFACE_TYPE = 0x%x", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_TRANSPARENT_TYPE,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_TRANSPARENT_TYPE = 0x%x", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_SAMPLE_BUFFERS,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_SAMPLE_BUFFERS = 0x%x", configId, value);
    
    VERIFYEGL(eglGetConfigAttrib(s_eglDisplay, config, EGL_SAMPLES,  &value));
    RETAILMSG(ZONE_INFO, "config[%d]: EGL_SAMPLES = 0x%x", configId, value);
    
Exit:
    return;
}
 */



} // END namespace Z

