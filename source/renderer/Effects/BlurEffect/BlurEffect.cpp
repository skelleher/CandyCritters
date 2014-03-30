#include "OpenGLESEffect.hpp"
#include "BlurEffect.hpp"
#include "RenderTarget.hpp"
#include "Matrix.hpp"


namespace Z
{


//----------------------------------------------------------------------------
// We expose the following named properties for animation and scripting.
//----------------------------------------------------------------------------
static const NamedProperty s_propertyTable[] =
{
    DECLARE_PROPERTY( BlurEffect, PROPERTY_FLOAT, Radius  ),
    NULL
};
DECLARE_PROPERTY_SET( BlurEffect, s_propertyTable );


//#define SOFT_EDGES
//#define BLEND_ORIGINAL_WITH_BLURRED


//----------------------------------------------------------------------------
// Class data
//----------------------------------------------------------------------------
UINT32                  BlurEffect::s_NumInstances                  = NULL;

bool                    BlurEffect::s_BlurShadersLoaded             = false;
GLuint                  BlurEffect::s_BlurShaderProgram             = 0;
HShader                 BlurEffect::s_hBlurShader;

const char*             BlurEffect::s_BlurVertexShaderName          = "/app/shaders/BlurEffect_vs.glsl";
const char*             BlurEffect::s_BlurFragmentShaderName        = "/app/shaders/BlurEffect_fs.glsl";

GLint                   BlurEffect::s_uProjectionMatrix             = 0;
GLint                   BlurEffect::s_uModelViewMatrix              = 0;
GLint                   BlurEffect::s_uTexture                      = 0;
GLint                   BlurEffect::s_uGlobalColor                  = 0;
GLint                   BlurEffect::s_uFilterOffsets                = 0;
GLint                   BlurEffect::s_uFilterWeights                = 0;
GLint                   BlurEffect::s_uHorizontalPass               = 0;

                        // 7x7 Gaussian Blur
const int               BlurEffect::MAX_FILTER_WIDTH_HEIGHT         = 7;
float                   BlurEffect::s_FilterWeights[]               = 
                        {
                            1.0f    / 2048.0f,
                            66.0f   / 2048.0f,
                            495.0f  / 2048.0f,
                            924.0f  / 2048.0f,
                            495.0f  / 2048.0f,
                            66.0f   / 2048.0f,
                            1.0f    / 2048.0f,
                        };

const float            BlurEffect::DEFAULT_BLUR_RADIUS        = 20.0;
const float            BlurEffect::MAX_BLUR_RADIUS            = 20.0;



//----------------------------------------------------------------------------
// Object methods
//----------------------------------------------------------------------------


BlurEffect::BlurEffect() :
    m_fPreviousDownsamplingFactor(-99.0),
    m_fDownsamplingFactor(1.0),
    m_fPreviousBlurRadius(-99.0),
    m_fBlurRadius(DEFAULT_BLUR_RADIUS),
    m_FrameCount(0),
    m_pPreviousVertexArray(NULL),
    m_pOffScreenFrameBuffer(NULL),
    m_cUnpaddedBlurVertices(4),
    m_cPaddedBlurVertices(4),
    m_cBlurredVertices(4),
    m_fBlurEdgePadding(0.0),
    m_fPaddedWidth(0.0),
    m_fPaddedHeight(0.0),
    m_fWidth(0.0),
    m_fHeight(0.0),
    m_TextureA(0),
    m_TextureB(1)
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "BlurEffect( %4d )", m_ID);

    ATOMIC_INCREMENT( BlurEffect::s_NumInstances );


    // This effect needs render-to-texture.  The result is then presented on-screen in ::EndFrame()
    m_needsRenderTarget = true;
    m_isPostEffect      = true;
    

    m_pUnpaddedBlurVertices = new Vertex[4];    // for rendering to off-screen (blur) texture
    m_pPaddedBlurVertices   = new Vertex[4];    // for rendering to off-screen (blur) texture
    m_pBlurredVertices      = new Vertex[4];    // for rendering shadow to the screen
    
    m_name = "BlurEffect";
}


BlurEffect::~BlurEffect()
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "\t~BlurEffect( %4d )", m_ID);

    SAFE_RELEASE(m_pOffScreenFrameBuffer);

    SAFE_ARRAY_DELETE(m_pUnpaddedBlurVertices);
    SAFE_ARRAY_DELETE(m_pPaddedBlurVertices);
    SAFE_ARRAY_DELETE(m_pBlurredVertices);

    // Delete the shaders when the last instance is freed
    if (0 == ATOMIC_DECREMENT( BlurEffect::s_NumInstances ))
    {
        RETAILMSG(ZONE_INFO, "~BlurEffect: last instance deleted, freeing blur shaders");
        Deinit();
    }
}



BlurEffect*
BlurEffect::Clone() const
{
    return new BlurEffect(*this);
}


BlurEffect::BlurEffect( const BlurEffect& rhs ) : 
    OpenGLESEffect(),
    m_pOffScreenFrameBuffer(NULL)
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "BlurEffect( %4d ) (copy ctor)", m_ID);

    ATOMIC_INCREMENT( BlurEffect::s_NumInstances );

    m_pUnpaddedBlurVertices = new Vertex[4];    // for rendering to off-screen (blur) texture
    m_pPaddedBlurVertices   = new Vertex[4];    // for rendering to off-screen (blur) texture
    m_pBlurredVertices      = new Vertex[4];    // for rendering shadow to the screen
    
    m_name = "BlurEffect";

    *this = rhs;
}


BlurEffect& 
BlurEffect::operator=( const BlurEffect& rhs )
{
    // Avoid self-assignment.
    if (this == &rhs)
        return *this;
    

    // Copy base class (is there a cleaner syntax?)
    OpenGLESEffect::operator=(rhs);


    // SHALLOW COPY:
    m_fBlurRadius                   = rhs.m_fBlurRadius;
    m_fPreviousBlurRadius           = rhs.m_fPreviousBlurRadius;
    m_fDownsamplingFactor           = rhs.m_fDownsamplingFactor;
    m_fPreviousDownsamplingFactor   = rhs.m_fPreviousDownsamplingFactor;
    m_fBlurEdgePadding              = rhs.m_fBlurEdgePadding;
    m_fPaddedWidth                  = rhs.m_fPaddedWidth;
    m_fPaddedHeight                 = rhs.m_fPaddedHeight;
    m_fWidth                        = rhs.m_fWidth;
    m_fHeight                       = rhs.m_fHeight;
    m_FrameCount                    = rhs.m_FrameCount;
    m_pPreviousVertexArray          = rhs.m_pPreviousVertexArray;
    m_cPaddedBlurVertices           = rhs.m_cPaddedBlurVertices;
    m_cUnpaddedBlurVertices         = rhs.m_cUnpaddedBlurVertices;
    m_cBlurredVertices              = rhs.m_cBlurredVertices;
    m_TextureA                      = rhs.m_TextureA;
    m_TextureB                      = rhs.m_TextureB;
    m_modelViewMatrixOffscreen      = rhs.m_modelViewMatrixOffscreen;
    m_projectionMatrixOffscreen     = rhs.m_projectionMatrixOffscreen;


    // DEEP COPY:
    memcpy(m_HorizontalFilterOffsets, rhs.m_HorizontalFilterOffsets, sizeof(m_HorizontalFilterOffsets));
    memcpy(m_VerticalFilterOffsets,   rhs.m_VerticalFilterOffsets,   sizeof(m_VerticalFilterOffsets));

    // DEEP COPY: RenderTarget
    if (rhs.m_pOffScreenFrameBuffer)
    {
        SAFE_DELETE(m_pOffScreenFrameBuffer);
        m_pOffScreenFrameBuffer = RenderTarget::Create( rhs.m_pOffScreenFrameBuffer->GetWidth(), rhs.m_pOffScreenFrameBuffer->GetHeight(), 1, rhs.m_pOffScreenFrameBuffer->GetNumTextures() );
    }

    // DEEP COPY: vertices
    if (rhs.m_pPaddedBlurVertices)
    {
        SAFE_ARRAY_DELETE(m_pPaddedBlurVertices);
        m_pPaddedBlurVertices = new Vertex[ rhs.m_cPaddedBlurVertices ];
        memcpy(m_pPaddedBlurVertices, rhs.m_pPaddedBlurVertices, sizeof(Vertex)*rhs.m_cPaddedBlurVertices);
    }
    if (rhs.m_pUnpaddedBlurVertices)
    {
        SAFE_ARRAY_DELETE(m_pUnpaddedBlurVertices);
        m_pUnpaddedBlurVertices = new Vertex[ rhs.m_cUnpaddedBlurVertices ];
        memcpy(m_pUnpaddedBlurVertices, rhs.m_pUnpaddedBlurVertices, sizeof(Vertex)*rhs.m_cUnpaddedBlurVertices);
    }
    if (rhs.m_pBlurredVertices)
    {
        SAFE_ARRAY_DELETE(m_pBlurredVertices);
        m_pBlurredVertices = new Vertex[ rhs.m_cBlurredVertices ];
        memcpy(m_pBlurredVertices, rhs.m_pBlurredVertices, sizeof(Vertex)*rhs.m_cBlurredVertices);
    }

    
    // Give it a new name with a random suffix
    char instanceName[MAX_NAME];
    sprintf(instanceName, "%s_%X", rhs.m_name.c_str(), (unsigned int)Platform::Random());
    m_name = string(instanceName);
    
    return *this;
}



#pragma mark -
#pragma mark Drawing

RESULT 
BlurEffect::DrawTriangleStrip( Vertex *pVertices, UINT32 numVertices )
{
    return BlurEffect::Draw( pVertices, numVertices, PRIMITIVE_TYPE_TRIANGLE_STRIP );
}


RESULT 
BlurEffect::DrawTriangleList( Vertex *pVertices, UINT32 numVertices )
{
    return BlurEffect::Draw( pVertices, numVertices, PRIMITIVE_TYPE_TRIANGLE_LIST );
}



RESULT 
BlurEffect::Draw( Vertex *pVertices, UINT32 numVertices, PRIMITIVE_TYPE primitiveType  )
{
    DEBUGMSG(ZONE_SHADER, "BlurEffect[%d]::DrawTriangleStrip()", m_ID);

    RESULT     rval          = S_OK;
    Rectangle  sourceRect;
    Rectangle  textureRect;
    Util::GetBoundingRect( pVertices, numVertices, &sourceRect  );
    Util::GetTextureMapping( pVertices, numVertices, &textureRect );
    
    float uStart = textureRect.x;
    float uEnd   = textureRect.x + textureRect.width;
    float vStart = textureRect.y;
    float vEnd   = textureRect.y + textureRect.height;
    

    // Skip the (expensive) blur effect if Radius = 0.0.
    if (Util::CompareFloats( m_fBlurRadius, 0.0 ))
    {
        return OpenGLESEffect::Draw( pVertices, numVertices, primitiveType );
    }

    //
    // TODO: cache and re-draw the blurred result if the vertices are identical
    // to previous draw call.  e.g. texture is same and bounding box is same?
    //

    // Recreate offscreen scratch surfaces whenever the blur radius has changed,
    // or when the vertex array being drawn has changed,
    // or when the Effect's frame has changed.
    if ( m_frameChanged ||
         (m_pPreviousVertexArray != pVertices) ||
         (fabs(m_fPreviousBlurRadius - m_fBlurRadius) > 1.0f) )
    {
        CHR(InitScratchSurfaces( sourceRect, m_fBlurRadius, uStart, uEnd, vStart, vEnd ));
        CHR(InitBlurFilter( m_fBlurRadius ));
        m_fPreviousBlurRadius = m_fBlurRadius;
        
        m_frameChanged = false;
    }
    m_pPreviousVertexArray = pVertices;


    // Save the original window viewport.
    GLint oldViewport[4];
    VERIFYGL(glGetIntegerv(GL_VIEWPORT, oldViewport));

    // Begin rendering to the off-screen texture.
    CHR(m_pOffScreenFrameBuffer->Enable());
    VERIFYGL(glViewport(0, 0, m_pOffScreenFrameBuffer->GetWidth(), m_pOffScreenFrameBuffer->GetHeight()));


#ifdef SOFT_EDGES    
    //
    // PASS 1: render original image into texture A (adding a border).
    //
    {
        CHR(m_pOffScreenFrameBuffer->SetRenderTexture( m_TextureA ));

        // Assign values to shader parameters.
        VERIFYGL(glUseProgram(s_defaultShaderProgram));
        VERIFYGL(glUniformMatrix4fv(s_uDefaultProjectionMatrix, 1, GL_FALSE, (GLfloat*)m_projectionMatrixOffscreen.Pointer()));
        VERIFYGL(glUniformMatrix4fv(s_uDefaultModelViewMatrix,  1, GL_FALSE, (GLfloat*)m_modelViewMatrixOffscreen.Pointer()));
        VERIFYGL(glUniform1i(s_uDefaultTexture, 0));
        VERIFYGL(glUniform4f(s_uDefaultGlobalColor, m_globalColor.floats.r, m_globalColor.floats.g, m_globalColor.floats.b, m_globalColor.floats.a));

        // Set original texture as the source.
        VERIFYGL(glActiveTexture(GL_TEXTURE0));
        VERIFYGL(glBindTexture(GL_TEXTURE_2D, m_sourceTextureID));

        // Draw to texture A.
        // This will leave a border around the source image, allowing
        // for blurred edges in the next passes.
        VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_POSITION,       3, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &m_pUnpaddedBlurVertices->x));
        VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_DIFFUSE,        4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(Vertex), &m_pUnpaddedBlurVertices->color));
        VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_TEXTURE_COORD0, 2, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &m_pUnpaddedBlurVertices->texCoord[0]));
        VERIFYGL(glDrawArrays(GL_TRIANGLE_STRIP, 0, m_cUnpaddedBlurVertices));
    }
#endif


    //
    // PASS 2: render texture A to texture B with horizontal blur.
    //
    {
        CHR(m_pOffScreenFrameBuffer->SetRenderTexture( m_TextureB ));

        // Assign values to shader parameters.
        VERIFYGL(glUseProgram(s_BlurShaderProgram));
        VERIFYGL(glUniformMatrix4fv(s_uProjectionMatrix, 1, GL_FALSE, (GLfloat*)m_projectionMatrixOffscreen.Pointer()));
        VERIFYGL(glUniformMatrix4fv(s_uModelViewMatrix,  1, GL_FALSE, (GLfloat*)m_modelViewMatrixOffscreen.Pointer()));
        VERIFYGL(glUniform1i(s_uTexture, 0));
        VERIFYGL(glUniform4f(s_uGlobalColor, m_globalColor.floats.r, m_globalColor.floats.g, m_globalColor.floats.b, m_globalColor.floats.a));
        VERIFYGL(glUniform1fv(s_uFilterWeights, MAX_FILTER_WIDTH_HEIGHT, (GLfloat*) &s_FilterWeights));

#ifdef SOFT_EDGES    
        // Set Texture A as the source.
        VERIFYGL(glActiveTexture(GL_TEXTURE0));
        VERIFYGL(glBindTexture(GL_TEXTURE_2D, m_pOffScreenFrameBuffer->GetTextureID(m_TextureA)));
#else
        // Set input texture as the source.
        VERIFYGL(glActiveTexture(GL_TEXTURE0));
        VERIFYGL(glBindTexture(GL_TEXTURE_2D, m_sourceTextureID));
#endif

        VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_POSITION,       3, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &m_pPaddedBlurVertices->x));
        VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_DIFFUSE,        4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(Vertex), &m_pPaddedBlurVertices->color));
        VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_TEXTURE_COORD0, 2, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &m_pPaddedBlurVertices->texCoord[0]));

        // Draw to the texture B with horizontal blur.
        VERIFYGL(glUniform1f(s_uHorizontalPass, 1));
        VERIFYGL(glUniform1fv(s_uFilterOffsets, MAX_FILTER_WIDTH_HEIGHT, (GLfloat*) &m_HorizontalFilterOffsets));
        VERIFYGL(glDrawArrays(GL_TRIANGLE_STRIP, 0, m_cPaddedBlurVertices));
    }


    //
    // PASS 3: render from texture B to texture A with vertical blur.
    //
    {
        CHR(m_pOffScreenFrameBuffer->SetRenderTexture( m_TextureA ));

        // Assign values to shader parameters.
        VERIFYGL(glUseProgram(s_BlurShaderProgram));
        VERIFYGL(glUniformMatrix4fv(s_uProjectionMatrix, 1, GL_FALSE, (GLfloat*)m_projectionMatrixOffscreen.Pointer()));
        VERIFYGL(glUniformMatrix4fv(s_uModelViewMatrix,  1, GL_FALSE, (GLfloat*)m_modelViewMatrixOffscreen.Pointer()));
        VERIFYGL(glUniform1i(s_uTexture, 0));
        VERIFYGL(glUniform4f(s_uGlobalColor, m_globalColor.floats.r, m_globalColor.floats.g, m_globalColor.floats.b, m_globalColor.floats.a));
        VERIFYGL(glUniform1fv(s_uFilterWeights, MAX_FILTER_WIDTH_HEIGHT, (GLfloat*) &s_FilterWeights));

        // Set Texture B as the source.
        VERIFYGL(glActiveTexture(GL_TEXTURE0));
        VERIFYGL(glBindTexture(GL_TEXTURE_2D, m_pOffScreenFrameBuffer->GetTextureID(m_TextureB)));

        VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_POSITION,       3, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &m_pPaddedBlurVertices->x));
        VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_DIFFUSE,        4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(Vertex), &m_pPaddedBlurVertices->color));
        VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_TEXTURE_COORD0, 2, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &m_pPaddedBlurVertices->texCoord[0]));

        // Draw to the texture A with vertical blur.
        VERIFYGL(glUniform1f(s_uHorizontalPass, 0));
        VERIFYGL(glUniform1fv(s_uFilterOffsets, MAX_FILTER_WIDTH_HEIGHT, (GLfloat*) &m_VerticalFilterOffsets));
        VERIFYGL(glDrawArrays(GL_TRIANGLE_STRIP, 0, m_cPaddedBlurVertices));
    }


    // Resume rendering to the on-screen window.
    CHR(m_pOffScreenFrameBuffer->Disable());
    VERIFYGL(glViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]));


#ifdef BLEND_ORIGINAL_WITH_BLURRED
    //
    // PASS 4: render from texture A to display.
    // Blend the blurred and unblurred images together based on BlurRadius.
    // This fakes a more fine-grained range of BlurRadius than we truly compute.
    // This also makes animation of BlurRadius appear more smooth.
    //
    {
        //
        // Override the per-vertex opacity.  
        // Use a custom opacity as a means of cheaply blending two images.
        //
        VERIFYGL(glDisableVertexAttribArray(ATTRIBUTE_VERTEX_DIFFUSE));
        GLfloat blurFactor = (GLfloat) (m_fBlurRadius / MAX_BLUR_RADIUS);
        GLfloat opacity[]  = { 1.0f, 1.0f, 1.0f, 1.0f };

        float sourceOpacity     = (float)(pVertices->color & 0xFF)/0xFF;
        float blurredWeight     = blurFactor;
        float unblurredWeight   = 1.0f - blurFactor;
        float blurredOpacity    = blurredWeight   * sourceOpacity;
        float unblurredOpacity  = unblurredWeight * sourceOpacity;

        VERIFYGL(glEnable(GL_BLEND));
        
        // TODO: standard blending on Mac/Android/other non-iOS devices.
//        VERIFYGL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        VERIFYGL(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
        VERIFYGL(glActiveTexture(GL_TEXTURE0));


        // 
        // Draw the blurred texture.
        //
        {
            // Create a triangle strip at same position as source, but padded.
            Rectangle blurRect  = sourceRect;
            blurRect.x      -= (float)m_fBlurEdgePadding;
            blurRect.y      -= (float)m_fBlurEdgePadding;
            blurRect.width  += (float)m_fBlurEdgePadding*2.0f;
            blurRect.height += (float)m_fBlurEdgePadding*2.0f;

            CHR(Util::CreateTriangleStrip( &blurRect, m_pBlurredVertices, 0.0f, 1.0f, 1.0f, 0.0f ));   // reverse Y texture coordinates, because render-to-texture puts (0,0) at bottom of screen, not top.


            opacity[0] = blurredOpacity;
            opacity[1] = blurredOpacity;
            opacity[2] = blurredOpacity;
            opacity[3] = blurredOpacity;

            VERIFYGL(glVertexAttrib4fv(ATTRIBUTE_VERTEX_DIFFUSE, opacity));
            VERIFYGL(glBindTexture(GL_TEXTURE_2D, m_pOffScreenFrameBuffer->GetTextureID(m_TextureA)));
            CHR(OpenGLESEffect::DrawTriangleStrip( m_pBlurredVertices, m_cBlurredVertices ));
        }

        //
        // Draw the original texture.
        //
        {
            opacity[0] = unblurredOpacity;
            opacity[1] = unblurredOpacity;
            opacity[2] = unblurredOpacity;
            opacity[3] = unblurredOpacity;

            VERIFYGL(glVertexAttrib4fv(ATTRIBUTE_VERTEX_DIFFUSE, opacity));
            VERIFYGL(glBindTexture(GL_TEXTURE_2D, m_sourceTextureID));
            CHR(OpenGLESEffect::Draw( pVertices, numVertices, primitiveType ));
        }

        //
        // Restore per-vertex opacity.
        //
        VERIFYGL(glEnableVertexAttribArray(ATTRIBUTE_VERTEX_DIFFUSE));
    }

#else

    //
    // PASS 4: render from texture A to display.
    //
    {
        // Create a triangle strip at same position as source, but padded.
        Rectangle blurRect  = sourceRect;
        blurRect.x      -= (float)m_fBlurEdgePadding;
        blurRect.y      -= (float)m_fBlurEdgePadding;
        blurRect.width  += (float)m_fBlurEdgePadding*2.0f;
        blurRect.height += (float)m_fBlurEdgePadding*2.0f;


//        CHR(Util::CreateTriangleStrip( &blurRect, m_pBlurredVertices, 0.0f, 1.0f, 1.0f, 0.0f ));   // reverse Y texture coordinates, because render-to-texture puts (0,0) at bottom of screen, not top.

        CHR(Util::CreateTriangleStrip( &blurRect, m_pBlurredVertices, uStart, uEnd, vStart, vEnd ));   // reverse Y texture coordinates, because render-to-texture puts (0,0) at bottom of screen, not top.
        VERIFYGL(glBindTexture(GL_TEXTURE_2D, m_pOffScreenFrameBuffer->GetTextureID(m_TextureA)));
        CHR(OpenGLESEffect::DrawTriangleStrip( m_pBlurredVertices, m_cBlurredVertices ));
    }
#endif // BLEND_ORIGINAL_WITH_BLURRED


Exit:
    return rval;
}



void
BlurEffect::SetRadius( float radius )
{
    m_fBlurRadius = MIN(radius, MAX_BLUR_RADIUS);
}


float
BlurEffect::GetRadius()
{
    return m_fBlurRadius;
}



RESULT 
BlurEffect::InitScratchSurfaces( Rectangle& sourceRect, float blurRadius, float uStart, float uEnd, float vStart, float vEnd )
{
    RESULT rval = S_OK;

    m_fDownsamplingFactor = ChooseDownsamplingFactor(blurRadius);

    // Only resize our scratch surfaces when the image downsampling needs to change,
    // which will only happen when there's been a large change in the blurRadius.
//    if ( Util::CompareFloats( m_fDownsamplingFactor, m_fPreviousDownsamplingFactor) )
//    {
//        return S_OK;
//    }
//    else
//    {
//        m_fPreviousDownsamplingFactor = m_fDownsamplingFactor;       
//    }


    // Create a FrameBufferObject with two textures to hold the blur results.
    //
    // The larger the blur radius, the more we can downsample the source image.
    // Downsampling a) yields extra blur and b) calls the pixel shader for fewer pixels
    // (making it much faster).  The blur will hide the loss of resolution.
    // 
    // Add padding, so there's room to blur the edge out.
    //
    // We render in multiple passes: 
    //  source   -> TextureA  (apply padding)
    //  TextureA -> TextureB  (horizontal blur)
    //  TextureB -> TextureA  (vertical blur)
    //  TextureA -> display   (shadow)
    {
#ifdef SOFT_EDGES    
        m_fBlurEdgePadding = MAX_BLUR_RADIUS;
#else        
        //****************************
        // TEST TEST TEST
        //****************************
        m_fBlurEdgePadding = 0.0f;
#endif        

        m_fPaddedWidth  = sourceRect.width  + (m_fBlurEdgePadding*2.0);
        m_fPaddedHeight = sourceRect.height + (m_fBlurEdgePadding*2.0);
        m_fWidth        = sourceRect.width;
        m_fHeight       = sourceRect.height;

        // Scale the scratch textures size according to the blur radius.
        // Larger blurs works best with smaller textures (we get more blur 
        // from the down-sampling, and the shader has fewer pixels to operate on
        // so it executes faster).
///        m_pOffScreenFrameBuffer = RenderTarget::Create( (UINT32)(m_fPaddedWidth/m_fDownsamplingFactor), (UINT32)(m_fPaddedHeight/m_fDownsamplingFactor), true, 2 );
        m_pOffScreenFrameBuffer = RenderTarget::Create( (UINT32)(m_fPaddedWidth/m_fDownsamplingFactor), (UINT32)(m_fPaddedHeight/m_fDownsamplingFactor), true, 2, false );
    }

    // Create an unpadded triangle strip for drawing to m_OffScreenFrameBuffer (where we do the blurring).
    Rectangle unpaddedRect;
    unpaddedRect.x      = (float)(m_fBlurEdgePadding)              / (float)m_fDownsamplingFactor;
    unpaddedRect.y      = (float)(m_fBlurEdgePadding)              / (float)m_fDownsamplingFactor;
    unpaddedRect.width  = (float)(m_fWidth)                       / (float)m_fDownsamplingFactor;
    unpaddedRect.height = (float)(m_fHeight)                      / (float)m_fDownsamplingFactor;
    CHR(Util::CreateTriangleStrip( &unpaddedRect, m_pUnpaddedBlurVertices, uStart, uEnd, vStart, vEnd ));

    // Create a padded triangle strip for drawing to m_OffScreenFrameBuffer (where we do the blurring).
    Rectangle paddedRect;
    paddedRect.x        = 0.0f;
    paddedRect.y        = 0.0f;
    paddedRect.width    = (float)m_fPaddedWidth   / (float)m_fDownsamplingFactor;
    paddedRect.height   = (float)m_fPaddedHeight  / (float)m_fDownsamplingFactor;
    CHR(Util::CreateTriangleStrip( &paddedRect, m_pPaddedBlurVertices ));

    {
        m_modelViewMatrixOffscreen = mat4::Translate(-1.0f*(paddedRect.width/2), 
                                                     -1.0f*(paddedRect.height/2), 
                                                     -1.0f* MAX(paddedRect.width, paddedRect.height));
                                         
        // Set the near and far clip planes
        GLfloat zFar = MAX(paddedRect.width, paddedRect.height) * 2.0f;
        zFar         = MAX(zFar, 3000.0f);
        
        //float fFov, float aspectRatio, float nearZ, float farZ
        m_projectionMatrixOffscreen = mat4::Perspective(53.0f, (float)paddedRect.width/(float)paddedRect.height, 1.0f, zFar );
    }


Exit:
    return rval;
}


RESULT
BlurEffect::InitBlurFilter( float blurRadius )
{
    // Compute texture offsets based on width/height of the texture being drawn,
    // and current value of BlurRadius.  
    //
    // Texture coordinates are in the range [0.0 .. 1.0], so if surface is 512x512 pixels, 
    // a pixel is (1.0 / 512) wide in texels.
    //
    // Multiply the texel offset by 1.5 so that samples are taken from between two texels, 
    // which leverages the hardware's built-in bilinear filtering for extra blur.
    float horizontalTexelSize   = 1.0 / m_pOffScreenFrameBuffer->GetWidth();
    float verticalTexelSize     = 1.0 / m_pOffScreenFrameBuffer->GetHeight();
    float verticalOffset        = verticalTexelSize   * (blurRadius/MAX_BLUR_RADIUS) * 1.5;
    float horizontalOffset      = horizontalTexelSize * (blurRadius/MAX_BLUR_RADIUS) * 1.5;

    CreateGaussianFilterKernel7x1( m_VerticalFilterOffsets,   (float)verticalOffset   );
    CreateGaussianFilterKernel7x1( m_HorizontalFilterOffsets, (float)horizontalOffset );

    return S_OK;
}



//----------------------------------------------------------------------------
// Class methods
//----------------------------------------------------------------------------

BlurEffect* 
BlurEffect::CreateInstance()
{
    DEBUGMSG(ZONE_RENDER, "BlurEffect::CreateInstance()");

    RESULT         rval     = S_OK;
    BlurEffect*    pEffect  = NULL;

    if (Platform::IsOpenGLES2())
    {
        if (!s_BlurShadersLoaded)
        {
            CHR(Init());
        }

        CPREx(pEffect = new BlurEffect(), E_OUTOFMEMORY);
    }
    else 
    {
        RETAILMSG(ZONE_WARN, "Device doesn't support BlurEffect.");
    }
    
Exit:
    return pEffect;
}



RESULT 
BlurEffect::Init()
{
    RETAILMSG(ZONE_RENDER, "BlurEffect::Init()");

    RESULT rval = S_OK;

    if (s_BlurShadersLoaded)
    {
        return S_OK;
    }

    CHR(OpenGLESEffect::Init());

    CHR(ShaderMan.CreateShader( "BlurEffect", s_BlurVertexShaderName, s_BlurFragmentShaderName, &s_hBlurShader ));
    
    // Save uniform handles.
    s_BlurShaderProgram = ShaderMan.GetShaderProgramID( s_hBlurShader );
    VERIFYGL(glUseProgram(s_BlurShaderProgram));

    VERIFYGL(s_uModelViewMatrix     = glGetUniformLocation(s_BlurShaderProgram, "uMatModelView"));
    VERIFYGL(s_uProjectionMatrix    = glGetUniformLocation(s_BlurShaderProgram, "uMatProjection" ));
    VERIFYGL(s_uTexture             = glGetUniformLocation(s_BlurShaderProgram, "uTexture"));
    VERIFYGL(s_uFilterOffsets       = glGetUniformLocation(s_BlurShaderProgram, "uFilterOffsets"));
    VERIFYGL(s_uFilterWeights       = glGetUniformLocation(s_BlurShaderProgram, "uFilterWeights"));
    VERIFYGL(s_uHorizontalPass      = glGetUniformLocation(s_BlurShaderProgram, "uHorizontalPass"));
    VERIFYGL(s_uGlobalColor         = glGetUniformLocation(s_BlurShaderProgram, "uGlobalColor"));
    
    s_BlurShadersLoaded = true;


Exit:
    return rval;
}



RESULT 
BlurEffect::Deinit()
{
    RETAILMSG(ZONE_RENDER, "BlurEffect::Deinit()");

    RESULT rval = S_OK;

    CHR(ShaderMan.Release( s_hBlurShader ));
    s_BlurShaderProgram  = 0;
    s_BlurShadersLoaded  = false;

Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: BlurEffect::Deinit(): failed to free shaders");
    }

    return rval;
}



HShader
BlurEffect::GetShader()
{
    return s_hBlurShader;
}



IProperty*
BlurEffect::GetProperty( const string& propertyName ) const
{
    return s_properties.Get( this, propertyName );
}



    
void
BlurEffect::CreateGaussianFilterKernel7x1( IN float* filterOffsets, float offset )
{
    RESULT rval = S_OK;

    float newFilterOffsets[]  = 
        {
            -3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0
        };

    for (int i = 0; i < 7; ++i)
    {
        newFilterOffsets[i] *= offset;
    }

    CPR(filterOffsets);

    memcpy( filterOffsets, newFilterOffsets, sizeof(newFilterOffsets) );

Exit:
    return;
}



float
BlurEffect::ChooseDownsamplingFactor( float blurRadius )
{
    // The larger the blur radius, the smaller resolution we can use
    // for our temp textures.  The blur hides the lower resolution,
    // and the blur both looks better and runs faster (fewer pixels to process).

    blurRadius               = MIN(blurRadius, MAX_BLUR_RADIUS);
    blurRadius               = MAX(blurRadius, 0.0);

    int index = (int)blurRadius;

    static float downsamplingFactors[21] = 
    { 
        2.0, 2.0, 4.0, 4.0, 4.0, 
        4.0, 4.0, 4.0, 4.0, 4.0, 
        5.0, 6.0, 6.0, 6.0, 6.0, 
        6.0, 6.0, 6.0, 6.0, 6.0,
        8.0
    };

    return downsamplingFactors[ index ];
}


} // END namespace Z

