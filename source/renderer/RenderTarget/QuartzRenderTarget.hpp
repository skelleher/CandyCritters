/*
 *  QuartzRenderTarget.hpp
 *
 *  Created by Sean Kelleher on 3/9/11.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#pragma once

#include "Types.hpp"
#include "Object.hpp"
#include "TextureManager.hpp"

#include <QuartzCore/QuartzCore.h>

// QuartzRenderTarget captures Quartz drawing and spits out OpenGL textures.
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>


namespace Z
{



class QuartzRenderTarget : virtual public Object
{
public:
    // Factory method
    static      QuartzRenderTarget* Create ( UINT32 width, UINT32 height, bool offscreen = true, UINT32 numTextures = 1 );
    virtual     ~QuartzRenderTarget( );
    
    bool        IsValid         ( );
    bool        IsOffscreen     ( );

    RESULT      Enable          ( );
    RESULT      Disable         ( );
    RESULT      Clear           ( );
    RESULT      SetRenderTexture( UINT32 index );
    GLuint      GetTextureID    ( ) const;
    GLuint      GetTextureID    ( UINT32 index ) const;
    HTexture    GetTexture      ( ) const;
    HTexture    GetTexture      ( UINT32 index ) const;
    UINT32      GetWidth        ( ) const;
    UINT32      GetHeight       ( ) const;
    Rectangle   GetTextureRect  ( ) const;
    UINT32      GetNumTextures  ( ) const;
    RESULT      GetTextureInfo  ( INOUT TextureInfo* pTextureInfo ) const;

protected:
    QuartzRenderTarget();
    RESULT      Init            ( UINT32 width, UINT32 height, bool offscreen = true, UINT32 numTextures = 1 );
    void        Deinit          ( );


protected:
    bool            m_enabledForRendering;
    bool            m_offscreen;

    CGContextRef    m_quartzContextRef;
    BYTE*           m_pBufferRGBA;

    UINT32          m_numTextures;
    GLuint          m_uiTextureIDs[8];
    GLuint          m_uiCurrentTextureID;
    HTexture        m_hTextures[8];
    HTexture        m_hCurrentTexture;
    UINT32          m_width;
    UINT32          m_height;

    
    // On OpenGL ES 1.w, the render target needs to have power-of-two dimensions.
    // So allocate that and scale the UV coordinates during render.
    float       m_fScaleTextureWidth;
    float       m_fScaleTextureHeight;
    GLint       m_textureWidth;
    GLint       m_textureHeight;
    GLint       m_textureStride;
};


} // END namespace Z
