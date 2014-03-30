/*
 *  RenderTarget.cpp
 *
 *  Created by Sean Kelleher on 3/9/11.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */


#include "RenderTarget.hpp"
#include "Macros.hpp"
#include "Log.hpp"
#include "Util.hpp"


namespace Z
{



RenderTarget::RenderTarget() :
    m_enabledForRendering(false),
    m_offscreen(false),
    m_uiFramebuffer(0),
    m_uiPreviousFramebuffer(0),
    m_uiColorRenderbuffer(0),
    m_uiDepthRenderbuffer(0),
    m_numTextures(0),
    m_uiCurrentTextureID(0),
    m_width(0),
    m_height(0),
    m_textureWidth(0),
    m_textureHeight(0),
    m_fScaleTextureWidth(1.0),
    m_fScaleTextureHeight(1.0),
    m_maxRenderbufferSize(0)
{
    RETAILMSG(ZONE_OBJECT, "RenderTarget::RenderTarget()");

    memset( m_uiTextureIDs, 0, sizeof(m_uiTextureIDs) );
}



RenderTarget::~RenderTarget()
{
    RETAILMSG(ZONE_OBJECT, "~RenderTarget[%d]()", m_uiFramebuffer);
    
    Deinit();
}



RenderTarget*
RenderTarget::Create( UINT32 width, UINT32 height, bool offscreen, UINT32 numTextures, bool pow2 )
{
    RESULT rval = S_OK;

    RenderTarget* pRenderTarget = new RenderTarget;
    DEBUGCHK(pRenderTarget);
    
    if (FAILED(rval = pRenderTarget->Init( width, height, offscreen, numTextures, pow2 )))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: RenderTarget::Create( %d, %d, %d, %d, %d ) failed, rval = 0x%x", rval, width, height, offscreen, numTextures, pow2);
        
        delete pRenderTarget;
        pRenderTarget = NULL;
    }
    
    return pRenderTarget;
}



RESULT
RenderTarget::Init( UINT32 width, UINT32 height, bool offscreen, UINT32 numTextures, bool pow2 )
{
    RESULT rval = S_OK;
    
    if (!width || !height || (offscreen && !numTextures))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: RenderTarget::Init(): width %d, height %d, %s, numTextures %d",
            width, height, 
            offscreen ? "offscreen" : "onscreen", 
            numTextures);

        rval = E_INVALID_ARG;
        goto Exit;
    }
    

    VERIFYGL(glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&m_uiPreviousFramebuffer));

    if (m_uiDepthRenderbuffer || m_uiColorRenderbuffer || m_uiFramebuffer)
    {
        // We are being re-initialized / resized
        CHR(SetRenderTexture( 0 ));

        if (!m_offscreen)
        {
            VERIFYGL( glDeleteRenderbuffers(1,          &m_uiColorRenderbuffer  ));
        }
        VERIFYGL( glDeleteRenderbuffers(1,              &m_uiDepthRenderbuffer  ));
        VERIFYGL( glDeleteFramebuffers (1,              &m_uiFramebuffer        ));
        VERIFYGL( glDeleteTextures     (m_numTextures,  &m_uiTextureIDs[0]      ));

        m_uiColorRenderbuffer   = 0;
        m_uiDepthRenderbuffer   = 0;
        m_uiFramebuffer         = 0;
        m_numTextures           = 0;
        memset(m_uiTextureIDs, 0, sizeof(m_uiTextureIDs));

        RETAILMSG(ZONE_INFO, "RenderTarget[%d]::Init(); discard old buffers and textures because of resize", m_uiFramebuffer);
    }

    m_width         = width;
    m_height        = height;
    m_textureWidth  = width;
    m_textureHeight = height;
    m_offscreen     = offscreen;
    m_numTextures   = numTextures;
    
    
    if (m_numTextures)
    {
        // Round up the texture size to nearest power-of-two, if requested.
        // This can offer better performance at the cost of graphics memory.
        if ( pow2 && (!Util::IsPowerOfTwo( width ) || !Util::IsPowerOfTwo( height )) )
        {
            m_textureWidth  = Util::RoundUpToPowerOfTwo( width  );
            m_textureHeight = Util::RoundUpToPowerOfTwo( height );
            
            m_fScaleTextureWidth  = (float)width /(float)m_textureWidth;
            m_fScaleTextureHeight = (float)height/(float)m_textureHeight;
        }


        // Create one or more textures.
        // A single FBO may be used to render to multiple textures of the same size
        // by changing the currently-bound texture before drawing.
        VERIFYGL(glGenTextures(numTextures, &m_uiTextureIDs[0]));
        m_uiCurrentTextureID = m_uiTextureIDs[0];

        for (UINT32 i = 0; i < numTextures; ++i)
        {
            VERIFYGL(glActiveTexture(GL_TEXTURE0));
            VERIFYGL(glBindTexture  (GL_TEXTURE_2D, m_uiTextureIDs[i]));

            // TODO: 16-bit option?  Doesn't seem any faster on iPhone, and we lose Alpha (5551 not working!)
            VERIFYGL(glTexImage2D   (GL_TEXTURE_2D, 0, GL_RGBA, m_textureWidth, m_textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
            VERIFYGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            VERIFYGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            VERIFYGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            VERIFYGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));

            // Create a corresponding HTexture to wrap the OpenGL texture.
            // this is what we normally want to expose to the users of this class.
            HTexture hTexture;
            TextureInfo textureInfo;
            memset(&textureInfo, 0, sizeof(TextureInfo));

            textureInfo.textureID                 = m_uiTextureIDs[i];
            textureInfo.widthPixelsTextureAtlas   = m_textureWidth;
            textureInfo.heightPixelsTextureAtlas  = m_textureHeight;
            textureInfo.widthPixels               = m_width;
            textureInfo.heightPixels              = m_height;
            textureInfo.offsetX                   = 0;
            textureInfo.offsetY                   = 0;

#ifdef IOS4_RECOMMENDED_MODE
            textureInfo.uStart                    = 0;
            textureInfo.vStart                    = 0;
            textureInfo.uEnd                      = m_fScaleTextureWidth;
            textureInfo.vEnd                      = m_fScaleTextureHeight;
#else
            textureInfo.uStart                    = 0;
            textureInfo.vStart                    = m_fScaleTextureHeight;
            textureInfo.uEnd                      = m_fScaleTextureWidth;
            textureInfo.vEnd                      = 0;
#endif

            char name[MAX_NAME];
            snprintf(name, sizeof(name), "RT_texture_%lX", Platform::Random());
            CHR(TextureMan.Create( name, textureInfo, &hTexture ));
            m_hTextures[i] = hTexture;
        }
    }

    // Generate an off-screen framebuffer and make it active
    VERIFYGL(glGenFramebuffers  (1, &m_uiFramebuffer));
    VERIFYGL(glBindFramebuffer(GL_FRAMEBUFFER, m_uiFramebuffer));

    if (m_offscreen)
    {
        VERIFYGL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uiCurrentTextureID, 0));
    }
    else
    {
        VERIFYGL(glGenRenderbuffers(1, &m_uiColorRenderbuffer));
        VERIFYGL(glBindRenderbuffer(GL_RENDERBUFFER, m_uiColorRenderbuffer));
        // Defer finalizing (binding a renderbuffer to the FBO) until the ::Enable() call.
        // We need to do this because of the stupid way Apple replaces glRenderbufferStorage() with the Obj-C call [EAGLContext renderbufferStorage:fromLayer]
    }

        
Exit:
    RETAILMSG(ZONE_INFO, "RenderTarget[%d]::Init( %d x %d, texture: %d x %d, %s, %d textures)",
        m_uiFramebuffer,
        width, height, 
        m_textureWidth, m_textureHeight,
        offscreen ? "offscreen" : "onscreen", 
        numTextures);

    // Restore the previous framebuffer.
    // NOT '0' - RenderTargets may be nested.
    VERIFYGL(glBindFramebuffer(GL_FRAMEBUFFER, m_uiPreviousFramebuffer));

    return rval;
}



void
RenderTarget::Deinit()
{
    RETAILMSG(ZONE_RENDER, "RenderTarget[%d]::Deinit( %dx%d %s numTextures: %d )", 
        m_uiFramebuffer, 
        m_width, m_height, 
        m_offscreen  ? "offscreen" : "onscreen", 
        m_numTextures);

    if (!m_offscreen)
    {
        VERIFYGL(glDeleteRenderbuffers(1,         &m_uiColorRenderbuffer ));
    }
    
    VERIFYGL(glDeleteRenderbuffers(1,             &m_uiDepthRenderbuffer ));
    VERIFYGL(glDeleteFramebuffers (1,             &m_uiFramebuffer       ));
    VERIFYGL(glDeleteTextures     (m_numTextures, &m_uiTextureIDs[0]     ));
    
Exit:
    return;
}



bool
RenderTarget::IsValid()
{
    return m_uiFramebuffer && GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_FRAMEBUFFER);
}



bool
RenderTarget::IsOffscreen()
{
    return m_offscreen;
}



RESULT 
RenderTarget::Enable()
{
    RETAILMSG(ZONE_RENDER, "RenderTarget[%d]::Enable()", m_uiFramebuffer);

    RESULT rval = S_OK;
    GLint  status;

    DEBUGCHK(m_uiFramebuffer);

    m_enabledForRendering = true;

    // Save off the previous FBO ID
    VERIFYGL(glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&m_uiPreviousFramebuffer));

    // Set the FBO as the render target
    VERIFYGL(glBindFramebuffer(GL_FRAMEBUFFER, m_uiFramebuffer));
    
    // TODO: too expensive to do this on each frame?  Do it once in Init()?
    if (m_offscreen)
    {
        VERIFYGL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uiCurrentTextureID, 0));
    }
    else 
    {
        VERIFYGL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_uiColorRenderbuffer));
    }


    // Save the original window viewport.
    VERIFYGL(glGetIntegerv(GL_VIEWPORT, m_oldViewport));

    // Set the viewport
    VERIFYGL(glViewport(0, 0, m_width, m_height));


    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: RenderTarget::Enable() failed.\n");
        DEBUGCHK(0);
        rval = E_FAIL;

        // TODO: restore viewport?
        VERIFYGL(glBindFramebuffer(GL_FRAMEBUFFER, m_uiPreviousFramebuffer));
    }

Exit:
    return rval;
}



RESULT 
RenderTarget::Disable()
{
    RETAILMSG(ZONE_RENDER, "RenderTarget[%d]::Disable(); new RT = %d", m_uiFramebuffer, m_uiPreviousFramebuffer);

    RESULT rval = S_OK;
    GLint  status;

    DEBUGCHK(m_uiFramebuffer);

    m_enabledForRendering = false;

    // Restore previous framebuffer
    VERIFYGL(glBindFramebuffer(GL_FRAMEBUFFER, m_uiPreviousFramebuffer));

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: RenderTarget::Enable() failed.\n");
        DEBUGCHK(0);
        rval = E_FAIL;
    }

    // Restore previous viewport
    VERIFYGL(glViewport(m_oldViewport[0] , m_oldViewport[1], m_oldViewport[2], m_oldViewport[3]));

Exit:
    return rval;
}


GLuint
RenderTarget::GetTextureID( ) const
{
    return m_uiCurrentTextureID;
}



GLuint
RenderTarget::GetTextureID( UINT32 index ) const
{
    if( index > m_numTextures-1 )
    {
        DEBUGCHK(0);
        return 0;
    }

    return m_uiTextureIDs[index];
}



HTexture
RenderTarget::GetTexture() const
{
    return m_hCurrentTexture;
}



HTexture
RenderTarget::GetTexture( UINT32 index ) const
{
    if( index > m_numTextures-1 )
    {
        DEBUGCHK(0);
        return HTexture::NullHandle();
    }

    return m_hTextures[index];
}



RESULT 
RenderTarget::SetRenderTexture( UINT32 index )
{
    RESULT rval = S_OK;
    GLint  status;

    DEBUGCHK(m_numTextures > 0);

    if( index > m_numTextures-1 )
    {
        DEBUGCHK(0);
        return E_INVALID_ARG;
    }

    m_uiCurrentTextureID = m_uiTextureIDs[index];

    // Attach the new texture
    VERIFYGL(glBindFramebuffer(GL_FRAMEBUFFER, m_uiFramebuffer));
    VERIFYGL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_uiCurrentTextureID, 0));

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: RenderTarget::SetRenderTexture( %d ) failed.\n", index);
        DEBUGCHK(0);
        rval = E_FAIL;
    }

    // Clear to transparent
    VERIFYGL(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
    VERIFYGL(glClear(GL_COLOR_BUFFER_BIT));

Exit:
    return rval;
}



UINT32
RenderTarget::GetWidth() const
{
    return m_width;
}


UINT32
RenderTarget::GetHeight() const
{
    return m_height;
}


Rectangle
RenderTarget::GetTextureRect() const
{
    Rectangle rect;
    
    // On OpenGL ES 1.1 devices, we may have had to scale the RenderTarget
    // width and height up to the nearest powers-of-two.
    // In which case our UV coordinates must be scaled down to match (we waste some texture space).
    
    rect.x      = 0;
    rect.y      = 0;
    rect.width  = m_fScaleTextureWidth;
    rect.height = m_fScaleTextureHeight;
    
    return rect;
}


UINT32
RenderTarget::GetNumTextures() const
{
    return m_numTextures;
}



} // END namespace Z
