/*
 *  OpenGLES2Renderer.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 9/26/10.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "Engine.hpp"
#include "OpenGLES2Renderer.hpp"
#include "Log.hpp"
#include "Macros.hpp"
#include "Platform.hpp"
#include "ShaderManager.hpp"
#include "RenderContext.hpp"
#include "FontManager.hpp"


namespace Z
{

  
    
//
// Static Data
//
OpenGLES2Renderer*  OpenGLES2Renderer::s_pInstance              = NULL;

#ifdef SHIPBUILD
    #define PRINT_PERF_INTERVAL_SECONDS 30.0
#else
    #define PRINT_PERF_INTERVAL_SECONDS 5.0
#endif

//
// Class Methods
//
OpenGLES2Renderer* 
OpenGLES2Renderer::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new OpenGLES2Renderer();
        DEBUGCHK(s_pInstance);
    }
    
    ////s_pInstance->AddRef();
    
    return s_pInstance;
}



//
// Instance Methods
//
OpenGLES2Renderer::OpenGLES2Renderer() :
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
    m_defaultShaderProgram(0xFFFFFFFF),
    m_currentShaderProgram(0xFFFFFFFF),
    m_pointSpriteShaderProgram(0xFFFFFFFF),
    m_uPointSpriteSize(0xFFFFFFFF),
    m_uPointSpriteProjectionMatrix(0xFFFFFFFF),
    m_uPointSpriteModelViewMatrix(0xFFFFFFFF),
    m_uPointSpriteTexture(0xFFFFFFFF),
    m_uPointSpriteGlobalColor(0xFFFFFFFF),
    m_currentTextureID(0xFFFFFFFF),
    m_uTextureEnabled(0xFFFFFFFF),
    m_uLightingEnabled(0xFFFFFFFF),
    m_currentEffectIsPost(false),
    m_alphaBlendEnabled(false),
// For Premultiplied Alpha (iPhone default):
    m_srcBlendFunction(GL_ONE),
    m_dstBlendFunction(GL_ONE_MINUS_SRC_ALPHA),
    m_globalColor(Color::White())
{
    RETAILMSG(ZONE_INFO, "Created OpenGLES2Renderer");
    
    m_name = "OpenGLES2Renderer";
}



OpenGLES2Renderer::~OpenGLES2Renderer()
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "\t~OpenGLES2Renderer( %4d )", m_ID);
    Deinit();
}



RESULT
OpenGLES2Renderer::Init( UINT32 width, UINT32 height )
{
    RESULT  rval = S_OK;
    HShader hShader;
    
    if (!width || !height)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: OpenGLES2Renderer::Init( %d, %d ): invalid argument.", width, height);
        rval = E_INVALID_ARG;
        goto Exit;
    }


    DEBUGMSG(ZONE_INFO, "OpenGLES2Renderer::Init()");

    m_width             = width;
    m_height            = height;
    

    // HACK HACK: cannot call SetRenderTarget() here because the OpenGLView hasn't finished being created.
    // Without framebuffer storage (EAGLLayer), binding the render target fails.
    // We defer binding the render target until ::BeginFrame().
    if (!m_pDefaultRenderTarget)
    {
        m_pDefaultRenderTarget = RenderTarget::Create( width, height, false, 0 );
        CPR(m_pDefaultRenderTarget);
    }
  

    PrintDriverInfo();

    
    //
    // Enable vertex position, diffuse, and texture coordinates
    //
    VERIFYGL(glEnableVertexAttribArray(ATTRIBUTE_VERTEX_POSITION));
    VERIFYGL(glEnableVertexAttribArray(ATTRIBUTE_VERTEX_DIFFUSE));
    VERIFYGL(glEnableVertexAttribArray(ATTRIBUTE_VERTEX_TEXTURE_COORD0));

    CHR(Resize( width, height ));
    
    CHR(Renderer.SetModelViewMatrix( GameCamera.GetViewMatrix() ));

    m_perfTimer.Start();
    
Exit:    
    return rval;
}



RESULT
OpenGLES2Renderer::SetRenderContext( IN RenderContext* pContext )
{
    RESULT rval = S_OK;
    
    DEBUGMSG(ZONE_INFO, "OpenGLES2Renderer::SetRenderContext( 0x%x )", pContext);

    if (m_pRenderContext != pContext)
    {
        SAFE_RELEASE(m_pRenderContext);

        m_pRenderContext = pContext;
        m_pRenderContext->AddRef();
        
        CHR(m_pRenderContext->Bind());
        
        // TODO: need to re-bind the RenderTarget?
    }

Exit:
    if (FAILED(rval))
        RETAILMSG(ZONE_ERROR, "ERROR: OpenGLES2Renderer::SetRenderContext() failed");
        
    return rval;
}



RESULT
OpenGLES2Renderer::SetRenderTarget( IN RenderTarget* pTarget )
{
    RESULT rval = S_OK;
    
    DEBUGMSG(ZONE_RENDER, "OpenGLES2Renderer::SetRenderTarget( 0x%x )", pTarget);

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
OpenGLES2Renderer::Deinit()
{
    RESULT rval = S_OK;
    
    DEBUGMSG(ZONE_INFO, "OpenGLES2Renderer::Deinit()");

    m_hCurrentEffect.Release();
    m_hDefaultEffect.Release();

    
    // 
    // Unbind and delete the Frame Buffer Object
    // 
    if (m_pRenderTarget)
    {
        IGNOREHR(m_pRenderTarget->Disable());
        m_pRenderTarget->Release();  // Do not use SAFE_RELEASE(); will NULL the ptr and cause next check to fail.
    }
        
    if (m_pDefaultRenderTarget && m_pDefaultRenderTarget != m_pRenderTarget)
    {
        IGNOREHR(m_pDefaultRenderTarget->Disable());
        m_pDefaultRenderTarget->Release();
    }

    SAFE_RELEASE(m_pRenderContext);
        
Exit:    
    return rval;
}



RESULT 
OpenGLES2Renderer::Resize( UINT32 width, UINT32 height )
{
    RESULT rval = S_OK;

    RETAILMSG(ZONE_RENDER, "OpenGLES2Renderer::Resize( %d x %d )", width, height);

    m_width     = width;
    m_height    = height;
    
    VERIFYGL(glViewport(0, 0, width, height));

    if (!m_hCurrentEffect.IsNull())
    {
        CHR(EffectMan.SetProjectionMatrix( m_hCurrentEffect, GameCamera.GetProjectionMatrix() ));
    }
    
    // TODO: always set projection on m_hDefaultEffect, s_defaultShaderProgram?
    
Exit:    
    return rval;
}



RESULT
OpenGLES2Renderer::Rotate( Orientation orientation )
{
    DEBUGMSG(ZONE_RENDER, "OpenGLES2Renderer::Rotate( %d )", orientation);
    m_orientation = orientation;
    
    return S_OK;
}



RESULT 
OpenGLES2Renderer::Clear( Color color )
{
    return Clear( color.floats.r, color.floats.g, color.floats.b, color.floats.a );
}




RESULT
OpenGLES2Renderer::Clear( float r, float g, float b, float a)
{
    RESULT rval = S_OK;

    DEBUGMSG(ZONE_INFO | ZONE_RENDER | ZONE_VERBOSE, "OpenGLES2Renderer::Clear( %1.1f, %1.1f, %1.1f, %1.1f )", r, g, b, a);

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



#pragma mark -
#pragma mark State
RESULT
OpenGLES2Renderer::EnableAlphaTest( bool enabled )
{
    // GL_ALPHA_TEST isn't available on OGLES 2; ignore.
    // TODO: tell shaders to skip alpha test
    
    return S_OK;
}



RESULT
OpenGLES2Renderer::SetBlendFunctions( UINT32 srcFunction, UINT32 dstFunction )
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
OpenGLES2Renderer::SetGlobalColor( Color color )
{
    RESULT rval = S_OK;
    
    m_globalColor = color;

    CHR(EffectMan.SetGlobalColor( m_hCurrentEffect, m_globalColor ));

Exit:
    return rval;
}



RESULT
OpenGLES2Renderer::EnableAlphaBlend( bool enabled )
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
            //VERIFYGL(glBlendFunc(GL_SRC_ALPHA, m_dstBlendFunction));
            VERIFYGL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        }
        else 
        {
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
OpenGLES2Renderer::EnableDepthTest( bool enabled )
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
OpenGLES2Renderer::EnableLighting( bool enabled )
{
    RESULT rval = S_OK;


    return rval;

//    VERIFYGL(glUseProgram(m_currentShaderProgram));

    // Tell shaders to skip lighting
    if (enabled)
    {
        VERIFYGL(glUniform1i( m_uLightingEnabled, 1 ));
    }
    else 
    {
        VERIFYGL(glUniform1i( m_uLightingEnabled, 0 ));
    }
 

Exit:
    return rval;
}



RESULT
OpenGLES2Renderer::EnableTexturing( bool enabled )
{
    RESULT rval = S_OK;


//    {
//        GLuint shader;
//        glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&shader);
//        DEBUGCHK(shader == m_currentShaderProgram);
//    }

    GLuint shader;
    VERIFYGL(glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&shader));
    if (shader != m_currentShaderProgram)
    {
        VERIFYGL(glUseProgram(m_currentShaderProgram));
    }

    // Tell shaders to skip texture fetch
    if (enabled)
    {
        VERIFYGL(glUniform1i( m_uTextureEnabled, 1 ));
    }
    else 
    {
        VERIFYGL(glUniform1i( m_uTextureEnabled, 0 ));
    }
 

Exit:
    return rval;
}



RESULT
OpenGLES2Renderer::BeginFrame()
{
    RESULT rval = S_OK;
    

    //
    // HACK HACK:
    //
    static bool firstFrame = true;
    if (firstFrame)
    {
    
        // TODO: assert render context, render target.

        //
        // Bind to the render target, which may be an off-screen texture.
        //
        CHR(SetRenderTarget( m_pDefaultRenderTarget ));
    
        //
        // Load default Effect.
        //
        CHR(EffectMan.Get( "DefaultEffect", &m_hDefaultEffect ));
        CHR(SetEffect( m_hDefaultEffect ));
        
        m_defaultShaderProgram = EffectMan.GetShaderProgramID( m_hDefaultEffect );   // TODO: get rid of this, trust that DefaultEffect is always set?
        VERIFYGL(m_uGlobalColor                 = glGetUniformLocation(m_defaultShaderProgram,     "uGlobalColor"     ));


        //
        // Load point sprite shader.
        //
        m_pointSpriteShaderProgram              = ShaderMan.GetShaderProgramID( "PointSprite" );
        VERIFYGL(m_uPointSpriteSize             = glGetUniformLocation(m_pointSpriteShaderProgram, "uPointSpriteSize" ));
        VERIFYGL(m_uPointSpriteProjectionMatrix = glGetUniformLocation(m_pointSpriteShaderProgram, "uMatProjection"   ));
        VERIFYGL(m_uPointSpriteModelViewMatrix  = glGetUniformLocation(m_pointSpriteShaderProgram, "uMatModelView"    ));
        VERIFYGL(m_uPointSpriteTexture          = glGetUniformLocation(m_pointSpriteShaderProgram, "uTexture"         ));
        VERIFYGL(m_uPointSpriteGlobalColor      = glGetUniformLocation(m_pointSpriteShaderProgram, "uGlobalColor"     ));

        firstFrame = false;
    }
        
Exit:
    return rval;
}



RESULT
OpenGLES2Renderer::EndFrame()
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
OpenGLES2Renderer::PushEffect( IN HEffect hEffect  )
{
    RESULT rval = S_OK;

    // Setting a NULL effect should be equivalent to DefaultEffect.
    if (hEffect.IsNull())
    {
        hEffect = m_hDefaultEffect;
    }

    // Push onto the stack
    m_effectStack.push( hEffect );

    if ( !hEffect.IsNull() )
    {
        hEffect.AddRef();
    }
    
    if (hEffect != m_hCurrentEffect)
    {
        CHR(SetEffect( hEffect, true ));
    }
    else
    {
        CHR(SetEffect( hEffect, false ));
    }
    
Exit:
    return rval;
}



RESULT
OpenGLES2Renderer::PopEffect( INOUT HEffect* phEffect )
{
    RESULT    rval = S_OK;
    HEffect   hPreviousEffect;
    
    CBR( m_effectStack.size() > 0 );

    // Pop from the stack
    hPreviousEffect = m_effectStack.top();
    m_effectStack.pop();

    if (phEffect)
    {
        *phEffect = hPreviousEffect;
    }
    

    // Restore the previous Effect, or default if the stack is empty.
    if ( m_effectStack.size() > 0 )
    {
        CHR(SetEffect( m_effectStack.top(), false ));
    }
    else 
    {
        CHR(SetEffect( m_hDefaultEffect, false ));
    }


    // Finish and present the prior Effect, only if it is not the same as the new/current effect.
    // For example if BlendEffect is nested, only call EndFrame() when popped for the last time.
    // This yields much better performance.
    if ( !hPreviousEffect.IsNull() && hPreviousEffect != m_hCurrentEffect )
    {
//        CHR(Effects.EndFrame( hPreviousEffect, m_currentDrawCalls > 0 ? true : false ));
        CHR(Effects.EndFrame( hPreviousEffect, true ));
    }

    if ( !hPreviousEffect.IsNull() )
    {
        hPreviousEffect.Release();
    }


Exit:
    return rval;
}



RESULT
OpenGLES2Renderer::SetEffect( IN HEffect hEffect, bool beginFrame )
{
    RESULT rval = S_OK;

    // Begin drawing with the new Effect
    if (m_hCurrentEffect != hEffect)
    {
        if (hEffect.IsNull())
        {
            m_hCurrentEffect.Release();
            m_hCurrentEffect = hEffect;
            goto Exit;
        }

    
        hEffect.AddRef();
        m_hCurrentEffect.Release();
        m_hCurrentEffect = hEffect;

        m_currentEffectIsPost = EffectMan.IsPostEffect( hEffect );

        // Track number of draw calls made with the current Effect.
        m_totalDrawCalls   += m_currentDrawCalls;
        m_currentDrawCalls = 0;

#ifdef DEBUG
        DEBUGMSG(ZONE_SHADER, "OpenGLES2Renderer::SetEffect( %s ) IsPost: %d", hEffect.GetName().c_str(), m_currentEffectIsPost);
#endif        

        // TODO: caller should pass the frame; we shouldn't assume full-screen.
        Rectangle frame;
        Platform::GetScreenRectCamera( &frame );

        if (beginFrame)
        {
            CHR(Effects.BeginFrame( hEffect, frame ));
        }
        
        {
        //
        // Get location of each shader uniform parameter
        // TODO: Remove from Renderer all together and hide behind IEffect!! 
        //
        m_currentShaderProgram  = ShaderMan.GetShaderProgramID( EffectMan.GetShader( hEffect ) );
        CBREx(0xFFFFFFFF != m_currentShaderProgram, E_UNEXPECTED);
        VERIFYGL(glUseProgram(m_currentShaderProgram));

        m_uLightingEnabled  = glGetUniformLocation(m_currentShaderProgram, "uLightingEnabled"   );
        m_uTextureEnabled   = glGetUniformLocation(m_currentShaderProgram, "uTextureEnabled"    );
        
        // TODO: specular, etc.
        }

        CHR(Effects.Enable( hEffect ));

        CHR(Effects.SetModelViewMatrix ( hEffect, GameCamera.GetViewMatrix()        ));
        CHR(Effects.SetProjectionMatrix( hEffect, GameCamera.GetProjectionMatrix()  ));
        CHR(Effects.SetTexture         ( hEffect, 0, m_hCurrentTexture              ));
        CHR(Effects.SetGlobalColor     ( hEffect, m_globalColor                     ));
    }


Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: OpenGLES2Renderer::SetEffect() rval = %d", rval);
        DEBUGCHK(0);
    }
        
    return rval;
}



RESULT
OpenGLES2Renderer::GetEffect( INOUT HEffect* phEffect )
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
OpenGLES2Renderer::SetTexture( IN UINT8 textureUnit, IN HTexture hTexture )
{
    RESULT rval = S_OK;
    
#ifdef DEBUG
    string      textureName;
    CHR(TextureMan.GetName       ( hTexture, &textureName ));

    DEBUGMSG(ZONE_TEXTURE | ZONE_VERBOSE, "OpenGLES2Renderer::SetTexture( %d, %s )", textureUnit, textureName.c_str());
#endif
    
    switch (textureUnit)
    {
        case 0:
            VERIFYGL(glActiveTexture(GL_TEXTURE0));
            break;
        case 1:
            VERIFYGL(glActiveTexture(GL_TEXTURE1));
            break;
        default:
            RETAILMSG(ZONE_WARN, "WARNING: OpenGLES2Renderer::SetTexture( %d, %d ): we only support two texture units for now.",
                      textureUnit, (UINT32)hTexture);
                      
            DEBUGCHK(0);
            
            CHR(E_INVALID_ARG);
    }
    
    
    TextureInfo textureInfo;
    CHR(TextureMan.GetInfo( hTexture, &textureInfo ));
    m_hCurrentTexture  = hTexture;
    m_currentTextureID = textureInfo.textureID;

    VERIFYGL(glBindTexture(GL_TEXTURE_2D, textureInfo.textureID));

    if (!m_hCurrentEffect.IsNull())
    {
        CHR(EffectMan.SetTexture( m_hCurrentEffect, textureUnit, m_hCurrentTexture ));
    }
    
Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "OpenGLES2Renderer::SetTexture( %d, %d ): bad handle", textureUnit, (UINT32)hTexture);
    }

    return rval;
}



RESULT
OpenGLES2Renderer::SetTexture( IN UINT8 textureUnit, IN UINT32 textureID )
{
    RESULT rval = S_OK;
    
    DEBUGMSG(ZONE_TEXTURE | ZONE_VERBOSE, "OpenGLES2Renderer::SetTexture( %d, %d )", textureUnit, textureID);
    
    switch (textureUnit)
    {
        case 0:
            VERIFYGL(glActiveTexture(GL_TEXTURE0));
            break;
        case 1:
            VERIFYGL(glActiveTexture(GL_TEXTURE1));
            break;
        default:
            RETAILMSG(ZONE_WARN, "WARNING: OpenGLES2Renderer::SetTexture( %d, %d ): we only support two texture units for now.",
                      textureUnit, textureID);
            CHR(E_INVALID_ARG);
    }
    
    m_currentTextureID = textureID;
    VERIFYGL(glBindTexture(GL_TEXTURE_2D, textureID));
    
    if (!m_hCurrentEffect.IsNull())
    {
        CHR(EffectMan.SetTexture( m_hCurrentEffect, 0, m_currentTextureID ));
    }
    
Exit:
    return rval;
}



RESULT
OpenGLES2Renderer::GetTexture( INOUT HTexture* phTexture )
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
OpenGLES2Renderer::SetModelViewMatrix( IN const mat4& matrix )
{
    RESULT rval = S_OK;

    m_currentModelViewMatrix = matrix;
    
    if (!m_hCurrentEffect.IsNull())
    {
        CHR(EffectMan.SetModelViewMatrix( m_hCurrentEffect, m_currentModelViewMatrix ));
    }
    
Exit:
    return rval;
}



RESULT
OpenGLES2Renderer::GetModelViewMatrix( INOUT mat4* pMatrix )
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
OpenGLES2Renderer::DrawTriangleStrip( IN Vertex *pVertices, UINT32 numVertices )
{
    RESULT rval = S_OK;
    
    CPREx(pVertices, E_NULL_POINTER);
    
    if (0 == numVertices)
    {
        RETAILMSG(ZONE_WARN, "WARNING: OpenGLES2Renderer::DrawTriangleStrip(): numVertices == 0");
        //return E_INVALID_ARG;
        return S_OK;
    }

    // Draw via the Effect object, if set and not a post-process effect.
    if (!m_hCurrentEffect.IsNull() && !m_currentEffectIsPost)
    {
        CHR(EffectMan.DrawTriangleStrip( m_hCurrentEffect, pVertices, numVertices ));
    }
    else
    {
        VERIFYGL(glUseProgram(m_defaultShaderProgram));
        
        VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_POSITION,       3, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &pVertices->position));
        VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_DIFFUSE,        4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(Vertex), &pVertices->color));
        VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_TEXTURE_COORD0, 2, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &pVertices->texCoord));
        
        VERIFYGL(glUniform4f(m_uGlobalColor, m_globalColor.floats.r, m_globalColor.floats.g, m_globalColor.floats.b, m_globalColor.floats.a));

        VERIFYGL(glDrawArrays( GL_TRIANGLE_STRIP, 0, numVertices ));
    }

Exit:
    m_currentDrawCalls++;

    return rval;
}



RESULT
OpenGLES2Renderer::DrawTriangleList( IN Vertex *pVertices, UINT32 numVertices )
{
    RESULT rval = S_OK;
    
    CPREx(pVertices, E_NULL_POINTER);
    
    if (0 == numVertices)
    {
        RETAILMSG(ZONE_WARN, "WARNING: OpenGLES2Renderer::DrawTriangleList(): numVertices == 0");
        return S_OK;
    }    
    
    // Draw via the Effect object if set and not a post-process effect.
    if (!m_hCurrentEffect.IsNull() && !m_currentEffectIsPost)
    {
        CHR(EffectMan.DrawTriangleList( m_hCurrentEffect, pVertices, numVertices ));
    }
    else
    {
        VERIFYGL(glUseProgram(m_defaultShaderProgram));

        VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_POSITION,       3, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &pVertices->position));
        VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_DIFFUSE,        4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(Vertex), &pVertices->color));
        VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_TEXTURE_COORD0, 2, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &pVertices->texCoord));

        VERIFYGL(glUniform4f(m_uGlobalColor, m_globalColor.floats.r, m_globalColor.floats.g, m_globalColor.floats.b, m_globalColor.floats.a));

        VERIFYGL(glDrawArrays(GL_TRIANGLES, 0, numVertices));
    }
    
Exit:
    m_currentDrawCalls++;
    
    return rval;
}



RESULT 
OpenGLES2Renderer::DrawPointSprites( IN Vertex *pVertices, UINT32 numVertices, float fScale )
{
    RESULT rval = S_OK;
    
    CPREx(pVertices, E_NULL_POINTER);
    
    if (0 == numVertices)
    {
        RETAILMSG(ZONE_WARN, "WARNING: OpenGLES2Renderer::DrawPointSprites(): numVertices == 0");
        return S_OK;
    }    
    
    // We ignore the current Effect and use a dedicated point sprite shader.
    // This is for performance; testing if point sprites are enabled in the fragment shader is way too expensive on iPhone.
    VERIFYGL(glUseProgram(m_pointSpriteShaderProgram));
    
    VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_POSITION,       3, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &pVertices->position));
    VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_DIFFUSE,        4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(Vertex), &pVertices->color));
    VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_TEXTURE_COORD0, 2, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &pVertices->texCoord));

    VERIFYGL(glUniformMatrix4fv(m_uPointSpriteProjectionMatrix,     1, GL_FALSE, (GLfloat*)GameCamera.GetProjectionMatrix().Pointer()));
    VERIFYGL(glUniformMatrix4fv(m_uPointSpriteModelViewMatrix,      1, GL_FALSE, (GLfloat*)GameCamera.GetViewMatrix().Pointer()));
    VERIFYGL(glUniform1i(m_uPointSpriteTexture, 0));
    VERIFYGL(glUniform4f(m_uPointSpriteGlobalColor, m_globalColor.floats.r, m_globalColor.floats.g, m_globalColor.floats.b, m_globalColor.floats.a));

    // TODO: this needs to be a vertex attibute pointer, so each sprite can have unique size.
    VERIFYGL(glUniform1f( m_uPointSpriteSize, fScale ));

    VERIFYGL(glDrawArrays(GL_POINTS, 0, numVertices));

    VERIFYGL(glUseProgram(m_currentShaderProgram));
    
Exit:
    m_currentDrawCalls++;
    
    return rval;
}



RESULT
OpenGLES2Renderer::DrawLines( IN Vertex *pVertices, UINT32 numVertices, float fWidth )
{
    RESULT rval = S_OK;
    
    CPREx(pVertices,        E_NULL_POINTER);
    CBREx(fWidth > 0,       E_INVALID_ARG);
    CBREx(numVertices > 0,  E_INVALID_ARG);
    
    EnableTexturing( false );
    
    
    //
    // Set up shader parameters
    //
    VERIFYGL(glUseProgram(m_defaultShaderProgram));
    
    VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_POSITION,       3, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &pVertices->position));
    VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_DIFFUSE,        4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(Vertex), &pVertices->color));
    VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_TEXTURE_COORD0, 2, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &pVertices->texCoord));
    
    VERIFYGL(glUniform4f(m_uGlobalColor, m_globalColor.floats.r, m_globalColor.floats.g, m_globalColor.floats.b, m_globalColor.floats.a));


    //
    // Draw lines
    //
    VERIFYGL(glLineWidth(fWidth));
    VERIFYGL(glDrawArrays(GL_LINES, 0, numVertices));
    m_currentDrawCalls++;
    
Exit:
    EnableTexturing( true );

    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: OpenGLES2Renderer::DrawLines(): rval = 0x%x", rval);
    }
    
    return rval;
}



UINT32
OpenGLES2Renderer::GetWidth()
{
    return m_width;
}



UINT32
OpenGLES2Renderer::GetHeight()
{
    return m_height;
}



RESULT
OpenGLES2Renderer::ShowOverdraw( bool show )
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
OpenGLES2Renderer::PrintPerfStats()
{
    double elapsedSeconds = m_perfTimer.ElapsedSeconds();
    
    if (elapsedSeconds >= PRINT_PERF_INTERVAL_SECONDS)
    {
        UINT32 freeRAM = Platform::GetAvailableMemory();
        UINT32 usedRAM = Platform::GetProcessUsedMemory();

        m_framesPerSecond = m_framesRendered / elapsedSeconds;
        m_drawsPerSecond  = m_totalDrawCalls / elapsedSeconds;
        
        RETAILMSG(ZONE_INFO, "FPS: %2.2f DPF: %2.2f Free: %d MB Used: %d MB", m_framesPerSecond, (float)m_totalDrawCalls/(float)m_framesRendered, freeRAM / 1048576, usedRAM / 1048576);
        
        // Restart the timer
        m_perfTimer.Start();
        m_framesRendered = 0;
        m_totalDrawCalls = 0;
    }
}



void
OpenGLES2Renderer::PrintDriverInfo()
{
    RETAILMSG(ZONE_INFO, "GL_VENDOR             = %s",   glGetString(GL_VENDOR));
    RETAILMSG(ZONE_INFO, "GL_RENDERER           = %s",   glGetString(GL_RENDERER));
    RETAILMSG(ZONE_INFO, "GL_VERSION            = %s\n", glGetString(GL_VERSION));
    RETAILMSG(ZONE_INFO, "GLSL_VERSION          = %s",   glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    GLint       iParam;
    VERIFYGL(glGetIntegerv(GL_SUBPIXEL_BITS,  &iParam));
    RETAILMSG(ZONE_INFO, "GL_SUBPIXEL_BITS      = %d", iParam);
    VERIFYGL(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,  &iParam));
    RETAILMSG(ZONE_INFO, "GL_MAX_TUs            = %d\n", iParam);
    VERIFYGL(glGetIntegerv( GL_MAX_TEXTURE_SIZE, &iParam ));
    RETAILMSG(ZONE_INFO, "GL_MAX_TEXTURE_SIZE   = %d\n", iParam);
    

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



} // END namespace Z

