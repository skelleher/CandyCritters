/*
 *  QuartzRenderTarget.cpp
 *
 *  Created by Sean Kelleher on 3/9/11.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */


#include "QuartzRenderTarget.hpp"
#include "Macros.hpp"
#include "Log.hpp"
#include "Util.hpp"

#include <CoreGraphics/CGImage.h>
#include <UIKit/UIKit.h>


// TEST
#define IOS4_RECOMMENDED_MODE


namespace Z
{



QuartzRenderTarget::QuartzRenderTarget() :
    m_enabledForRendering(false),
    m_offscreen(false),
    m_quartzContextRef(NULL),
    m_numTextures(0),
    m_uiCurrentTextureID(0),
    m_width(0),
    m_height(0),
    m_textureWidth(0),
    m_textureHeight(0),
    m_textureStride(0),
    m_fScaleTextureWidth(1.0),
    m_fScaleTextureHeight(1.0)
{
    RETAILMSG(ZONE_OBJECT, "QuartzRenderTarget::QuartzRenderTarget()");

    memset( m_uiTextureIDs, 0, sizeof(m_uiTextureIDs) );
}



QuartzRenderTarget::~QuartzRenderTarget()
{
    RETAILMSG(ZONE_OBJECT, "~QuartzRenderTarget()");
    
    SAFE_DELETE(m_pBufferRGBA);
    
    Deinit();
}



QuartzRenderTarget*
QuartzRenderTarget::Create( UINT32 width, UINT32 height, bool offscreen, UINT32 numTextures )
{
    RESULT rval = S_OK;

    QuartzRenderTarget* pQuartzRenderTarget = new QuartzRenderTarget;
    DEBUGCHK(pQuartzRenderTarget);
    
    if (FAILED(rval = pQuartzRenderTarget->Init( width, height, offscreen, numTextures )))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: QuartzRenderTarget::Create( %d, %d, %d, %d ) failed, rval = 0x%x", rval, width, height, offscreen, numTextures);
        
        delete pQuartzRenderTarget;
        pQuartzRenderTarget = NULL;
    }
    
    return pQuartzRenderTarget;
}



RESULT
QuartzRenderTarget::Init( UINT32 width, UINT32 height, bool offscreen, UINT32 numTextures )
{
    RESULT          rval            = S_OK;
    CGColorSpaceRef colorSpaceRef   = NULL;
    CGBitmapInfo    bitmapInfo      = 0;

    
    if (!width || !height || (offscreen && !numTextures))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: QuartzRenderTarget::Init(): width %d, height %d, %s, numTextures %d\n",
            width, height, 
            offscreen ? "offscreen" : "onscreen", 
            numTextures);

        return E_INVALID_ARG;
    }
    

    m_width         = width;
    m_height        = height;
    m_textureWidth  = width;
    m_textureHeight = height;
    m_offscreen     = offscreen;
    m_numTextures   = numTextures;
    

    //
    // Round up dimensions to nearest power-of-two, for optimal texture rendering by OpenGL.
    //
//    if ( (!Util::IsPowerOfTwo( m_width ) || !Util::IsPowerOfTwo( m_height )) )
//    {
//        m_textureWidth  = Util::RoundUpToPowerOfTwo( width  );
//        m_textureHeight = Util::RoundUpToPowerOfTwo( height );
//        
//        m_fScaleTextureWidth  = (float)width /(float)m_textureWidth;
//        m_fScaleTextureHeight = (float)height/(float)m_textureHeight;
//    }

    //
    // Create the Quartz CGBitmapContext to capture rendering.
    //
    colorSpaceRef       = CGColorSpaceCreateDeviceRGB();
    bitmapInfo          = kCGBitmapByteOrderDefault | kCGImageAlphaPremultipliedLast; // iOS does not support kCGImageAlphaLast


    //
    // Ensure stride is 16-byte-aligned for performance. 
    //
    m_textureStride     = m_textureWidth * 4;
    UINT32 misalignment = m_textureStride & 0xF;
    if (misalignment)
    {
        RETAILMSG(ZONE_INFO, "Realigning m_textureStride 0x%x to 0x%x", m_textureStride, m_textureStride + (0x10 - misalignment));
        m_textureStride += (0x10 - misalignment);
    }

    m_pBufferRGBA       = (BYTE*)new BYTE[ ((m_textureHeight * m_textureStride) + 15) ];

    misalignment = (UINT32)m_pBufferRGBA & 0xF;
    if (misalignment)
    {
        RETAILMSG(ZONE_INFO, "Realigning m_pBufferRGBA 0x%x to 0x%x", m_pBufferRGBA, m_pBufferRGBA + (0x10 - misalignment));
        m_pBufferRGBA += (0x10 - misalignment);
    }


    m_quartzContextRef  = CGBitmapContextCreate (
        m_pBufferRGBA,
        m_width,
        m_height,
        8,                      // size_t bitsPerComponent,
        m_textureStride,        // size_t bytesPerRow,              TODO: ensure rows are 16-byte aligned for performance!  See Quartz book p353
        colorSpaceRef,          // CGColorSpaceRef colorspace,
        bitmapInfo              // CGBitmapInfo bitmapInfo
    );
    CGColorSpaceRelease(colorSpaceRef);


    //
    // Create one or more textures.
    // A single QuartzRenderTarget may be used to render to multiple textures of the same size
    // by changing the currently-bound texture before drawing.
    //

    VERIFYGL(glGenTextures(numTextures, &m_uiTextureIDs[0]));
    m_uiCurrentTextureID = m_uiTextureIDs[0];



    for (UINT32 i = 0; i < numTextures; ++i)
    {
        VERIFYGL(glActiveTexture(GL_TEXTURE0));
        VERIFYGL(glBindTexture  (GL_TEXTURE_2D, m_uiTextureIDs[i]));
//        VERIFYGL(glTexImage2D   (GL_TEXTURE_2D, 0, GL_RGBA, m_textureStride, m_textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
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
        snprintf(name, sizeof(name), "QuartzRT_texture_%lX", Platform::Random());
        CHR(TextureMan.Create( name, textureInfo, &hTexture ));
        m_hTextures[i] = hTexture;
    }

Exit:
    RETAILMSG(ZONE_INFO, "QuartzRenderTarget[%d]::Init( %d x %d, texture: %d x %d, %s, %d textures)",
              m_ID,
              m_width, m_height, 
              m_textureWidth, m_textureHeight,
              m_offscreen ? "offscreen" : "onscreen", 
              m_numTextures);

    return rval;
}



void
QuartzRenderTarget::Deinit()
{
    RETAILMSG(ZONE_RENDER, "QuartzRenderTarget[%d]::Deinit( %d x %d %s numTextures: %d )", 
        m_ID, 
        m_width, m_height, 
        m_offscreen  ? "offscreen" : "onscreen", 
        m_numTextures);

    VERIFYGL(glDeleteTextures(m_numTextures, &m_uiTextureIDs[0] ));
    
    CGContextRelease(m_quartzContextRef);
    
Exit:
    return;
}



bool
QuartzRenderTarget::IsValid()
{
    return true;
}



bool
QuartzRenderTarget::IsOffscreen()
{
    return m_offscreen;
}



RESULT 
QuartzRenderTarget::Enable()
{
    RETAILMSG(ZONE_RENDER, "QuartzRenderTarget::Enable()");

#ifdef IOS4_RECOMMENDED_MODE
    UIGraphicsBeginImageContextWithOptions( [[UIScreen mainScreen] bounds].size, false, 0.0 ); // TODO: use Texture bounds not screen bounds? Texture may be square/POW2/etc.
#else
    UIGraphicsPushContext( m_quartzContextRef );
    DEBUGCHK(UIGraphicsGetCurrentContext() == m_quartzContextRef);
#endif

    Clear();

    return S_OK;
}



RESULT 
QuartzRenderTarget::Disable()
{
    RETAILMSG(ZONE_RENDER, "QuartzRenderTarget::Disable()");

    RESULT rval = S_OK;

#ifdef IOS4_RECOMMENDED_MODE
    UIImage * image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    CGContextDrawImage(m_quartzContextRef, CGRectMake(0.0, 0.0, (CGFloat)m_width, (CGFloat)m_height), image.CGImage);
#else
    CGContextFlush(m_quartzContextRef);
    UIGraphicsPopContext();
#endif

    // Copy the bitmap bytes into our currrent texture.
    VERIFYGL(glActiveTexture(GL_TEXTURE0));
    VERIFYGL(glBindTexture(GL_TEXTURE_2D, m_uiCurrentTextureID));
    VERIFYGL(glTexImage2D( GL_TEXTURE_2D, 
                           0, 
                           GL_RGBA,
                           m_textureWidth, 
                           m_textureHeight, 
                           0,
                           GL_RGBA,
                           GL_UNSIGNED_BYTE,
                           m_pBufferRGBA
                          ));

Exit:
    return rval;
}


RESULT
QuartzRenderTarget::Clear( )
{
    CGRect rect = CGRectMake(0, 0, (CGFloat)m_width, (CGFloat)m_height);

    // TEST:
//    CGColorRef color = [[UIColor blackColor] CGColor];
//    CGContextSetFillColorWithColor(m_quartzContextRef, color);
//    CGContextFillRect(m_quartzContextRef, rect);

    CGContextClearRect(m_quartzContextRef, rect);

    return S_OK;
}



GLuint
QuartzRenderTarget::GetTextureID( ) const
{
    return m_uiCurrentTextureID;
}



GLuint
QuartzRenderTarget::GetTextureID( UINT32 index ) const
{
    if( index > m_numTextures-1 )
    {
        DEBUGCHK(0);
        return 0;
    }

    return m_uiTextureIDs[index];
}



HTexture
QuartzRenderTarget::GetTexture() const
{
    return m_hCurrentTexture;
}



HTexture
QuartzRenderTarget::GetTexture( UINT32 index ) const
{
    if( index > m_numTextures-1 )
    {
        DEBUGCHK(0);
        return HTexture::NullHandle();
    }

    return m_hTextures[index];
}



RESULT 
QuartzRenderTarget::SetRenderTexture( UINT32 index )
{
    RESULT rval = S_OK;

    DEBUGCHK(m_numTextures > 0);

    if( index > m_numTextures-1 )
    {
        DEBUGCHK(0);
        return E_INVALID_ARG;
    }

    m_uiCurrentTextureID = m_uiTextureIDs[index];

Exit:
    return rval;
}



UINT32
QuartzRenderTarget::GetWidth() const
{
    return m_width;
}


UINT32
QuartzRenderTarget::GetHeight() const
{
    return m_height;
}


Rectangle
QuartzRenderTarget::GetTextureRect() const
{
    Rectangle rect;
    
    // On OpenGL ES 1.1 devices, we may have had to scale the QuartzRenderTarget
    // width and height up to the nearest powers-of-two.
    // In which case our UV coordinates must be scaled down to match (we waste some texture space).

#ifdef IOS4_RECOMMENDED_MODE
    rect.x      = 0;
    rect.y      = 0;
    rect.width  = m_fScaleTextureWidth;
    rect.height = m_fScaleTextureHeight;
#else    
    rect.x      = 0;
    rect.y      = m_fScaleTextureHeight;
    rect.width  = m_fScaleTextureWidth;
    rect.height = -1.0*m_fScaleTextureHeight;   // Invert the Y axis when rendering with Quartz and NOT using UIGraphicsBeginImageContext().
#endif
    
    return rect;
}


RESULT
QuartzRenderTarget::GetTextureInfo( INOUT TextureInfo* pTextureInfo ) const
{
    RESULT rval = S_OK;

    if (!pTextureInfo)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: QuartzREnderTArget::GetTextureInfo(): NULL pointer");
        rval = E_NULL_POINTER;
        goto Exit;
    }

    pTextureInfo->textureID                 = m_uiCurrentTextureID;
    pTextureInfo->widthPixelsTextureAtlas   = m_textureWidth;
    pTextureInfo->heightPixelsTextureAtlas  = m_textureHeight;
    pTextureInfo->widthPixels               = m_width;
    pTextureInfo->heightPixels              = m_height;
    pTextureInfo->offsetX                   = 0;
    pTextureInfo->offsetY                   = 0;

#ifdef IOS4_RECOMMENDED_MODE
    pTextureInfo->uStart                    = 0;
    pTextureInfo->vStart                    = 0;
    pTextureInfo->uEnd                      = m_fScaleTextureWidth;
    pTextureInfo->vEnd                      = m_fScaleTextureHeight;
#else
    pTextureInfo->uStart                    = 0;
    pTextureInfo->vStart                    = m_fScaleTextureHeight;
    pTextureInfo->uEnd                      = m_fScaleTextureWidth;
    pTextureInfo->vEnd                      = 0;
#endif

Exit:
    return rval;
}


UINT32
QuartzRenderTarget::GetNumTextures() const
{
    return m_numTextures;
}



} // END namespace Z
