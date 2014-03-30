/*
 *  OpenGLESEFfect.cpp
 *
 *  Created by Sean Kelleher on 3/9/11.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include "OpenGLESEffect.hpp"


namespace Z
{



//----------------------------------------------------------------------------
// Class data
//----------------------------------------------------------------------------
UINT32      OpenGLESEffect::s_numInstances                      = 0;

bool        OpenGLESEffect::s_haveCheckedPlatformSupport        = false;
bool        OpenGLESEffect::s_isPlatformSupported               = false;

bool        OpenGLESEffect::s_defaultShadersLoaded              = false;
GLuint      OpenGLESEffect::s_defaultShaderProgram              = 0;
HShader     OpenGLESEffect::s_hDefaultShader;

const char* OpenGLESEffect::s_defaultVertexShaderName           = "/app/shaders/DefaultEffect_vs";
const char* OpenGLESEffect::s_defaultFragmentShaderName         = "/app/shaders/DefaultEffect_fs";

GLint       OpenGLESEffect::s_uDefaultModelViewMatrix           = 0;
GLint       OpenGLESEffect::s_uDefaultProjectionMatrix          = 0;
GLint       OpenGLESEffect::s_uDefaultTexture                   = 0;
GLint       OpenGLESEffect::s_uDefaultGlobalColor               = 0;



//----------------------------------------------------------------------------
// Object methods
//----------------------------------------------------------------------------

RESULT 
OpenGLESEffect::SetTexture(
    UINT8       textureUnit,
    UINT32      textureID
    )
{
    DEBUGMSG(ZONE_SHADER | ZONE_VERBOSE, "OpenGLESEffect[%d]::SetTexture( %d: %d )", m_ID, textureUnit, textureID);

    RESULT rval = S_OK;

    m_hSourceTexture  = HTexture::NullHandle();
    m_sourceTextureID = textureID;

    switch (textureUnit)
    {
        case 0:
            VERIFYGL(glActiveTexture(GL_TEXTURE0));
            break;
        case 1:
            VERIFYGL(glActiveTexture(GL_TEXTURE1));
            break;
        default:
            RETAILMSG(ZONE_WARN, "WARNING: OpenGLESEffect::SetTexture( %d, %d ): we only support two texture units for now.",
                      textureUnit, textureID);
                      
            assert(0);
            
            CHR(E_INVALID_ARG);
    }
    
    
    VERIFYGL(glBindTexture(GL_TEXTURE_2D, m_sourceTextureID));

Exit:
    return rval;
}

    


RESULT 
OpenGLESEffect::SetTexture(
    UINT8       textureUnit,
    HTexture    hTexture
    )
{
    DEBUGMSG(ZONE_SHADER | ZONE_VERBOSE, "OpenGLESEffect[%d]::SetTexture( %d: 0x%x )", m_ID, textureUnit, (UINT32)hTexture);

    RESULT rval = S_OK;


    if (m_hSourceTexture != hTexture)
    {
        m_hSourceTexture = hTexture;

        TextureInfo textureInfo;
        CHR(TextureMan.GetInfo( hTexture, & textureInfo ));
        m_sourceTextureID = textureInfo.textureID;

        switch (textureUnit)
        {
            case 0:
                VERIFYGL(glActiveTexture(GL_TEXTURE0));
                break;
            case 1:
                VERIFYGL(glActiveTexture(GL_TEXTURE1));
                break;
            default:
                RETAILMSG(ZONE_WARN, "WARNING: OpenGLESEffect::SetTexture( %d, %d ): we only support two texture units for now.",
                          textureUnit, (UINT32)hTexture);
                          
                assert(0);
                
                CHR(E_INVALID_ARG);
        }
        
        VERIFYGL(glBindTexture(GL_TEXTURE_2D, m_sourceTextureID));
    }

Exit:
    return rval;
}

    

RESULT 
OpenGLESEffect::SetGlobalColor( const Color& color )
{
    RESULT rval = S_OK;
    
    DEBUGMSG(ZONE_SHADER | ZONE_VERBOSE, "OpenGLESEffect[%d]::SetGlobalColor()", m_ID);

    m_globalColor = color;

Exit:
    return rval;
}



RESULT 
OpenGLESEffect::SetModelViewMatrix( const mat4& modelViewMatrix )
{
    RESULT rval = S_OK;
    
    DEBUGMSG(ZONE_SHADER | ZONE_VERBOSE, "OpenGLESEffect[%d]::SetModelViewMatrix()", m_ID);

    m_modelViewMatrix = modelViewMatrix;

    VERIFYGL(glUseProgram(s_defaultShaderProgram));
    VERIFYGL(glUniformMatrix4fv(s_uDefaultModelViewMatrix,  1, GL_FALSE, (GLfloat*)m_modelViewMatrix.Pointer()));

Exit:
    return rval;
}



RESULT 
OpenGLESEffect::SetProjectionMatrix( const mat4& projectionMatrix )
{
    RESULT rval = S_OK;

    DEBUGMSG(ZONE_SHADER | ZONE_VERBOSE, "OpenGLESEffect[%d]::SetProjectionMatrix()", m_ID);

    m_projectionMatrix = projectionMatrix;

    VERIFYGL(glUseProgram(s_defaultShaderProgram));
    VERIFYGL(glUniformMatrix4fv(s_uDefaultProjectionMatrix, 1, GL_FALSE, (GLfloat*)m_projectionMatrix.Pointer()));

Exit:
    return rval;
}


#pragma mark -
#pragma mark Drawing


bool
OpenGLESEffect::IsPostEffect() const
{
    return m_isPostEffect;
}



RESULT
OpenGLESEffect::BeginFrame( IN const Rectangle& frame )
{
    RESULT rval = S_OK;

    if (!Util::CompareRectangles(frame, m_frame))
    {
        m_frame        = frame;
        m_frameChanged = true;  // Subclasses may test this and recreate their scratch surfaces.
    }
    else
    {
        m_frameChanged = false;
    }

    // Don't use an off-screen render target unless the effect needs it.  VERY expensive.
    if (!m_needsRenderTarget)
        return S_OK;
    

    DEBUGMSG(ZONE_SHADER, "------------------------------------");
    DEBUGMSG(ZONE_SHADER, "OpenGLESEffect::BeginFrame( \"%s\" )", m_name.c_str());
    

    //
    // Create off-screen render target.
    // All subsequent draw calls will render to its texture.
    // Then in ::EndFrame() we'll present the result while applying the Effect.
    //
    if (m_frameChanged)
    {
        SAFE_RELEASE(m_pRenderTarget);
        SAFE_ARRAY_DELETE(m_pFrameVertices);

        m_pRenderTarget = RenderTarget::Create( frame.width, frame.height, true, 1 );
        DEBUGCHK(m_pRenderTarget);
        m_pRenderTarget->AddRef();

        // TODO: RenderTarget should return the correct TextureRect w/o us needing to invert Y.
        Rectangle textureRect = m_pRenderTarget->GetTextureRect();
        float uStart = textureRect.x;
        float uEnd   = textureRect.x + textureRect.width;
        float vStart = textureRect.y + textureRect.height;  // reverse Y texture coordinates, because render-to-texture puts (0,0) at bottom of screen.
        float vEnd   = textureRect.y;

        CHR(Util::CreateTriangleList(&frame, 1, 1, &m_pFrameVertices, &m_numFrameVertices, uStart, uEnd, vStart, vEnd));
    }

    CHR(m_pRenderTarget->Enable());
    
    // TODO: set scissor rect
    // TODO: AVOID this when we can; it really hurts perf.
    CHR(Renderer.Clear( Color::Clear() ));
    

Exit:
    return rval;
}


RESULT
OpenGLESEffect::Enable() const
{
    RESULT rval = S_OK;
    
    // Bind shader and default parameters.
    VERIFYGL(glUseProgram(s_defaultShaderProgram));

    // TODO: don't do this here.
    VERIFYGL(glUniform4f(s_uDefaultGlobalColor, m_globalColor.floats.r, m_globalColor.floats.g, m_globalColor.floats.b, m_globalColor.floats.a));

Exit:
    return rval;
}



RESULT
OpenGLESEffect::EndFrame( bool present )
{
    RESULT rval = S_OK;

    // Don't use an off-screen render target unless the effect needs it.  VERY expensive.
    if (!m_needsRenderTarget)
        return S_OK;


    CHR(m_pRenderTarget->Disable());
    
    if (present)
    {
        CHR(SetTexture( 0, m_pRenderTarget->GetTextureID() ));
        CHR(DrawTriangleList( m_pFrameVertices, m_numFrameVertices ));
    }

Exit:
    DEBUGMSG(ZONE_SHADER, "OpenGLESEffect::EndFrame( \"%s\" present: %d)", m_name.c_str(), present);
    DEBUGMSG(ZONE_SHADER, "------------------------------------");
    
    return rval;
}



RESULT 
OpenGLESEffect::DrawTriangleStrip( Vertex *pVertices, UINT32 numVertices )
{
    return OpenGLESEffect::Draw( pVertices, numVertices, PRIMITIVE_TYPE_TRIANGLE_STRIP );
}


RESULT 
OpenGLESEffect::DrawTriangleList( Vertex *pVertices, UINT32 numVertices )
{
    return OpenGLESEffect::Draw( pVertices, numVertices, PRIMITIVE_TYPE_TRIANGLE_LIST );
}




RESULT 
OpenGLESEffect::Draw(
        Vertex *pVertices,  
        UINT32 numVertices,
        PRIMITIVE_TYPE primitiveType
        )
{
    DEBUGMSG(ZONE_SHADER | ZONE_VERBOSE, "OpenGLESEffect[%d]::DrawTriangleStrip()", m_ID);

    RESULT rval = S_OK;

    if (!s_defaultShadersLoaded)
    {
        CHR(Init());
    }
    
    // TODO: Only do this in ::Enable()
    // so we can batch render calls by Effect and then avoid this per-triangle-strip-overhead.
    VERIFYGL(glUseProgram(s_defaultShaderProgram));
    VERIFYGL(glUniformMatrix4fv(s_uDefaultProjectionMatrix, 1, GL_FALSE, (GLfloat*)m_projectionMatrix.Pointer()));
    VERIFYGL(glUniformMatrix4fv(s_uDefaultModelViewMatrix,  1, GL_FALSE, (GLfloat*)m_modelViewMatrix.Pointer()));
    VERIFYGL(glUniform1i(s_uDefaultTexture, 0));
    VERIFYGL(glUniform4f(s_uDefaultGlobalColor, m_globalColor.floats.r, m_globalColor.floats.g, m_globalColor.floats.b, m_globalColor.floats.a));

    // Don't bind texture; assume that caller already did it (useful for Effects that subclass us).

    VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_POSITION,       3, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &pVertices->x));
    VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_DIFFUSE,        4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(Vertex), &pVertices->color));
    VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_TEXTURE_COORD0, 2, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &pVertices->texCoord[0]));

    switch (primitiveType)
    {
        case PRIMITIVE_TYPE_TRIANGLE_STRIP:
            VERIFYGL(glDrawArrays(GL_TRIANGLE_STRIP, 0, numVertices));
            break;
        case PRIMITIVE_TYPE_TRIANGLE_LIST:
            VERIFYGL(glDrawArrays(GL_TRIANGLES, 0, numVertices));
            break;
        default:
            DEBUGCHK(0);
    }

Exit:
    return rval;
}




//
// Private Object Methods
//
OpenGLESEffect::OpenGLESEffect() :
    m_isPostEffect(false),
    m_needsRenderTarget(false),
    m_pRenderTarget(NULL),
    m_pFrameVertices(NULL),
    m_numFrameVertices(0),
    m_frameChanged(false),
    m_globalColor(Color::White())
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "OpenGLESEffect( %4d )", m_ID);

    ATOMIC_INCREMENT( s_numInstances );
    
    m_name = "DefaultEffect";
}


OpenGLESEffect::~OpenGLESEffect()
{
    RETAILMSG(ZONE_OBJECT, "\t~OpenGLESEffect( %4d )", m_ID);

    IGNOREHR(TextureMan.Release( m_hSourceTexture ));

    SAFE_RELEASE(m_pRenderTarget);

    // Delete the shaders when the last instance is freed
    if (0 == ATOMIC_DECREMENT( OpenGLESEffect::s_numInstances ))
    {
        RETAILMSG(ZONE_INFO, "~OpenGLESEffect: last instance deleted, freeing default shaders");
        Deinit();
    }
}


OpenGLESEffect*
OpenGLESEffect::Clone() const
{
    return new OpenGLESEffect(*this);
}


OpenGLESEffect::OpenGLESEffect( const OpenGLESEffect& rhs ) : 
    Object(),
    m_isPostEffect(false),
    m_needsRenderTarget(false),
    m_pRenderTarget(NULL),
    m_pFrameVertices(NULL),
    m_frameChanged(false),
    m_globalColor(Color::White())
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "OpenGLESEffect( %4d ) (copy ctor)", m_ID);

    ATOMIC_INCREMENT( s_numInstances );
    
    *this = rhs;
}


OpenGLESEffect& 
OpenGLESEffect::operator=( const OpenGLESEffect& rhs )
{
    // Avoid self-assignment.
    if (this == &rhs)
        return *this;
    

    // Explicitly DO NOT copy the base class state.
    // We DO NOT want to copy the Object::m_RefCount, m_ID, or m_Name from the copied OpenGLESEffect.

    // SHALLOW COPY:
    m_isPostEffect          = rhs.m_isPostEffect;
    m_needsRenderTarget     = rhs.m_needsRenderTarget;
    m_modelViewMatrix       = rhs.m_modelViewMatrix;
    m_projectionMatrix      = rhs.m_projectionMatrix;
    m_sourceTextureID       = rhs.m_sourceTextureID;
    m_numFrameVertices      = rhs.m_numFrameVertices;
    m_hSourceTexture        = rhs.m_hSourceTexture;
    m_frame                 = rhs.m_frame;
    m_frameChanged          = rhs.m_frameChanged;
    m_globalColor           = rhs.m_globalColor;
    
    
    if (!m_hSourceTexture.IsNull())
        IGNOREHR(TextureMan.AddRef( m_hSourceTexture ));


    // DEEP COPY:
    if (rhs.m_pRenderTarget)
    {
        SAFE_RELEASE(m_pRenderTarget);
        m_pRenderTarget = RenderTarget::Create( rhs.m_pRenderTarget->GetWidth(), rhs.m_pRenderTarget->GetHeight(), rhs.m_pRenderTarget->IsOffscreen(), rhs.m_pRenderTarget->GetNumTextures() );
    }
               
    if (rhs.m_pFrameVertices)
    {
        SAFE_ARRAY_DELETE(m_pFrameVertices);
        m_pFrameVertices = new Vertex[ rhs.m_numFrameVertices ];
        memcpy(m_pFrameVertices, rhs.m_pFrameVertices, sizeof(Vertex)*rhs.m_numFrameVertices);
    }

    // Give it a new name with a random suffix
    char instanceName[MAX_NAME];
    sprintf(instanceName, "%s_%X", rhs.m_name.c_str(), (unsigned int)Platform::Random());
    m_name = string(instanceName);
    
    return *this;
}



//----------------------------------------------------------------------------
// Class methods
//----------------------------------------------------------------------------

OpenGLESEffect* 
OpenGLESEffect::CreateInstance()
{
    DEBUGMSG(ZONE_SHADER, "OpenGLESEffect::CreateInstance()");

    RESULT          rval        = S_OK;
    OpenGLESEffect* pEffect     = NULL;

    if (Platform::IsOpenGLES2())
    {
        if (!s_defaultShadersLoaded)
        {
            CHR(Init());
        }

        CPREx(pEffect = new OpenGLESEffect(), E_OUTOFMEMORY);
    }
    else 
    {
        RETAILMSG(ZONE_WARN, "Device doesn't support OpenGLESEffect.");
    }
    

Exit:
    return pEffect;
}



RESULT 
OpenGLESEffect::Init()
{
    RETAILMSG(ZONE_SHADER, "OpenGLESEffect::Init()");

    RESULT rval = S_OK;

    if (s_defaultShadersLoaded)
    {
        return S_OK;
    }

    CHR(ShaderMan.Get( "Default", &s_hDefaultShader ));
    
    // Save uniform handles.
    s_defaultShaderProgram = ShaderMan.GetShaderProgramID( s_hDefaultShader );
    VERIFYGL(glUseProgram(s_defaultShaderProgram));

    VERIFYGL(s_uDefaultModelViewMatrix    = glGetUniformLocation(s_defaultShaderProgram, "uMatModelView"  ));
    VERIFYGL(s_uDefaultProjectionMatrix   = glGetUniformLocation(s_defaultShaderProgram, "uMatProjection" ));
    VERIFYGL(s_uDefaultTexture            = glGetUniformLocation(s_defaultShaderProgram, "uTexture"       ));
    VERIFYGL(s_uDefaultGlobalColor        = glGetUniformLocation(s_defaultShaderProgram, "uGlobalColor"   ));
    
    s_defaultShadersLoaded = true;

Exit:
    return rval;
}


RESULT 
OpenGLESEffect::Deinit()
{
    RETAILMSG(ZONE_SHADER, "OpenGLESEffect::Deinit()");

    RESULT rval = S_OK;

    CHR(ShaderMan.Release( s_hDefaultShader ));

    s_defaultShaderProgram  = 0;
    s_defaultShadersLoaded  = false;

Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: OpenGLESEffect::Deinit(): failed to free shaders");
    }

    return rval;
}



HShader
OpenGLESEffect::GetShader()
{
    return s_hDefaultShader;
}



IProperty*
OpenGLESEffect::GetProperty( const string& propertyName ) const
{
    return Object::GetProperty( propertyName );
}


    

} // END namespace Z

