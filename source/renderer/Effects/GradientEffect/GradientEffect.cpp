#include "OpenGLESEffect.hpp"
#include "GradientEffect.hpp"
#include "RenderTarget.hpp"
#include "Matrix.hpp"
#include "Time.hpp"


namespace Z
{

//----------------------------------------------------------------------------
// We expose the following named properties for animation and scripting.
//----------------------------------------------------------------------------
static const NamedProperty s_propertyTable[] =
{
    DECLARE_PROPERTY( GradientEffect, PROPERTY_COLOR,  StartColor   ),
    DECLARE_PROPERTY( GradientEffect, PROPERTY_COLOR,  EndColor     ),
    DECLARE_PROPERTY( GradientEffect, PROPERTY_VEC2,   StartPoint   ),
    DECLARE_PROPERTY( GradientEffect, PROPERTY_VEC2,   EndPoint     ),
    NULL
};
DECLARE_PROPERTY_SET( GradientEffect, s_propertyTable );





//----------------------------------------------------------------------------
// Class data
//----------------------------------------------------------------------------
UINT32                  GradientEffect::s_NumInstances                  = NULL;

bool                    GradientEffect::s_GradientShadersLoaded         = false;
GLuint                  GradientEffect::s_GradientShaderProgram         = 0;
HShader                 GradientEffect::s_hGradientShader;

const char*             GradientEffect::s_GradientVertexShaderName      = "/app/shaders/GradientEffect_vs.glsl";
const char*             GradientEffect::s_GradientFragmentShaderName    = "/app/shaders/GradientEffect_fs.glsl";

GLint                   GradientEffect::s_uProjectionMatrix             = 0;
GLint                   GradientEffect::s_uModelViewMatrix              = 0;
GLint                   GradientEffect::s_uTexture                      = 0;
GLint                   GradientEffect::s_uGlobalColor                  = 0;
GLint                   GradientEffect::s_uStartColor                   = 0;
GLint                   GradientEffect::s_uStartPoint                   = 0;
GLint                   GradientEffect::s_uEndColor                     = 0;
GLint                   GradientEffect::s_uEndPoint                     = 0;
GLint                   GradientEffect::s_uWidth                        = 0;
GLint                   GradientEffect::s_uHeight                       = 0;
//GLint                   GradientEffect::s_uTexCoordOffset               = 0;

const Color             GradientEffect::DEFAULT_START_COLOR             = Color::White();
const Color             GradientEffect::DEFAULT_END_COLOR               = Color::Black();



//----------------------------------------------------------------------------
// Object methods
//----------------------------------------------------------------------------


GradientEffect::GradientEffect() :
    m_pGradientVertices(NULL),
    m_numGradientVertices(0),
    m_startColor(DEFAULT_START_COLOR),
    m_endColor(DEFAULT_END_COLOR),
    m_startPoint(0.5f, 0.0f),
    m_endPoint(0.5f, 1.0f)
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "GradientEffect( %4d )", m_ID);

    ATOMIC_INCREMENT( GradientEffect::s_NumInstances );

    m_name = "GradientEffect";
    memset(&m_frame, 0, sizeof(m_frame));
}


GradientEffect::~GradientEffect()
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "\t~GradientEffect( %4d )", m_ID);

    // Delete the shaders when the last instance is freed
    if (0 == ATOMIC_DECREMENT( GradientEffect::s_NumInstances ))
    {
        RETAILMSG(ZONE_INFO, "~GradientEffect: last instance deleted, freeing Color shaders");
        Deinit();
    }
}



GradientEffect*
GradientEffect::Clone() const
{
    return new GradientEffect(*this);
}


GradientEffect::GradientEffect( const GradientEffect& rhs ) : 
    OpenGLESEffect(),
    m_pGradientVertices(NULL),
    m_numGradientVertices(0),
    m_startColor(DEFAULT_START_COLOR),
    m_endColor(DEFAULT_END_COLOR),
    m_startPoint(0.5f, 0.0f),
    m_endPoint(0.5f, 1.0f)
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "GradientEffect( %4d ) (copy ctor)", m_ID);

    ATOMIC_INCREMENT( GradientEffect::s_NumInstances );

    m_name = "GradientEffect";
    memset(&m_frame, 0, sizeof(m_frame));

    *this = rhs;
}


GradientEffect& 
GradientEffect::operator=( const GradientEffect& rhs )
{
    // Avoid self-assignment.
    if (this == &rhs)
        return *this;
    

    // Copy base class (is there a cleaner syntax?)
    OpenGLESEffect::operator=(rhs);


    // SHALLOW COPY:
    m_numGradientVertices       = rhs.m_numGradientVertices;
    m_startColor                = rhs.m_startColor;
    m_endColor                  = rhs.m_endColor;
    m_startPoint                = rhs.m_startPoint;
    m_endPoint                  = rhs.m_endPoint;


    // DEEP COPY: vertices
    if (rhs.m_pGradientVertices)
    {
        SAFE_ARRAY_DELETE(m_pGradientVertices);
        m_pGradientVertices = new Vertex[ rhs.m_numGradientVertices ];
        memcpy(m_pGradientVertices, rhs.m_pGradientVertices, sizeof(Vertex)*rhs.m_numGradientVertices);
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
GradientEffect::DrawTriangleStrip( Vertex *pVertices, UINT32 numVertices )
{
    return GradientEffect::Draw( pVertices, numVertices, PRIMITIVE_TYPE_TRIANGLE_STRIP );
}


RESULT 
GradientEffect::DrawTriangleList( Vertex *pVertices, UINT32 numVertices )
{
    return GradientEffect::Draw( pVertices, numVertices, PRIMITIVE_TYPE_TRIANGLE_LIST );
}



RESULT 
GradientEffect::Draw( Vertex *pVertices, UINT32 numVertices, PRIMITIVE_TYPE primitiveType  )
{
    DEBUGMSG(ZONE_SHADER, "GradientEffect[%d]::DrawTriangleStrip()", m_ID);

    RESULT     rval          = S_OK;
    Rectangle  sourceRect;
    Rectangle  textureRect;

    Util::GetBoundingRect  ( pVertices, numVertices, &sourceRect  );
    Util::GetTextureMapping( pVertices, numVertices, &textureRect );


    // Reinit if our source frame changes, i.e. the object
    // we're drawing is being scaled.
    Util::GetBoundingRect( m_pGradientVertices, m_numGradientVertices, &m_gradientRect );
    if (!Util::CompareRectangles(sourceRect, m_gradientRect) || !m_pGradientVertices || m_frameChanged)
    {
        CHR(InitScratchSurfaces( sourceRect, textureRect ));
        m_frameChanged = false;
    }


    // Assign values to shader parameters.
    // TODO: move to BeginFrame()?
    VERIFYGL(glUseProgram(s_GradientShaderProgram));
    VERIFYGL(glUniformMatrix4fv(s_uProjectionMatrix, 1, GL_FALSE, (GLfloat*)m_projectionMatrix.Pointer()));
    VERIFYGL(glUniformMatrix4fv(s_uModelViewMatrix,  1, GL_FALSE, (GLfloat*)m_modelViewMatrix.Pointer()));
    VERIFYGL(glUniform1i(s_uTexture,            0));
    VERIFYGL(glUniform4f(s_uGlobalColor,        m_globalColor.floats.r, m_globalColor.floats.g, m_globalColor.floats.b, m_globalColor.floats.a));
    VERIFYGL(glUniform4f(s_uStartColor,         m_startColor.floats.r, m_startColor.floats.g, m_startColor.floats.b, m_startColor.floats.a));
    VERIFYGL(glUniform2f(s_uStartPoint,         (m_startPoint.x * textureRect.width) + textureRect.x, (m_startPoint.y * textureRect.height) + textureRect.y));
    VERIFYGL(glUniform4f(s_uEndColor,           m_endColor.floats.r, m_endColor.floats.g, m_endColor.floats.b, m_endColor.floats.a));
    VERIFYGL(glUniform2f(s_uEndPoint,           (m_endPoint.x * textureRect.width) + textureRect.x, (m_endPoint.y * textureRect.height) + textureRect.y));
    VERIFYGL(glUniform1f(s_uWidth,              textureRect.width));
    VERIFYGL(glUniform1f(s_uHeight,             textureRect.height));


    // Draw the mesh with gradient applies.
    VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_POSITION,       3, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &m_pGradientVertices->x));
    VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_DIFFUSE,        4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(Vertex), &m_pGradientVertices->color));
    VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_TEXTURE_COORD0, 2, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &m_pGradientVertices->texCoord[0]));

    switch (primitiveType)
    {
        // NOTE: for this effect we ignore the passed-in vertices and draw a triangle list (GL_TRIANGLES) not a triangle strip (GL_TRIANGLE_STRIP).
        case PRIMITIVE_TYPE_TRIANGLE_STRIP:
        case PRIMITIVE_TYPE_TRIANGLE_LIST:
            VERIFYGL(glDrawArrays(GL_TRIANGLES, 0, m_numGradientVertices));
            break;
        default:
            // TODO: might it ever be useful to apply Color to a field of point sprites?
            DEBUGCHK(0);
    }

Exit:
    return rval;
}



void
GradientEffect::SetStartColor( const Color& color )
{
    m_startColor = color;
}


Color
GradientEffect::GetStartColor()
{
    return m_startColor;
}


void
GradientEffect::SetStartPoint( const vec2& point )
{
    m_startPoint = point;
}


vec2
GradientEffect::GetStartPoint()
{
    return m_startPoint;
}



void
GradientEffect::SetEndColor( const Color& color )
{
    m_endColor = color;
}


Color
GradientEffect::GetEndColor()
{
    return m_endColor;
}


void
GradientEffect::SetEndPoint( const vec2& point )
{
    m_endPoint = point;
}


vec2
GradientEffect::GetEndPoint()
{
    return m_endPoint;
}




RESULT 
GradientEffect::InitScratchSurfaces( Rectangle& sourceRect, Rectangle& textureRect )
{
    RESULT rval = S_OK;
    
    // We render a mesh in place of the (likely) quad sent down from the caller.
    // This allows for more fine-grained gradients, while still keeping the math in
    // the vertex shader.
    //
    // Create the new mesh with the same position and dimensions, 
    // and a reasonable number of number of triangles (so the gradient looks smooth.
    
    float uStart    = textureRect.x;
    float uEnd      = textureRect.x + textureRect.width;
    float vStart    = textureRect.y;
    float vEnd      = textureRect.y + textureRect.height;
    
    SAFE_ARRAY_DELETE(m_pGradientVertices);
    
    CHR(Util::CreateTriangleList( &sourceRect, 5, 5, &m_pGradientVertices, &m_numGradientVertices, uStart, uEnd, vStart, vEnd  ));

    // TODO: create a VBO.
    
Exit:
    return rval;
}




//----------------------------------------------------------------------------
// Class methods
//----------------------------------------------------------------------------

GradientEffect* 
GradientEffect::CreateInstance()
{
    DEBUGMSG(ZONE_SHADER, "GradientEffect::CreateInstance()");

    RESULT        rval      = S_OK;
    GradientEffect* pEffect   = NULL;

    if (Platform::IsOpenGLES2())
    {
        if (!s_GradientShadersLoaded)
        {
            CHR(Init());
        }

        CPREx(pEffect = new GradientEffect(), E_OUTOFMEMORY);
    }
    else 
    {
        RETAILMSG(ZONE_WARN, "Device doesn't support GradientEffect.");
    }

Exit:
    return pEffect;
}



RESULT 
GradientEffect::Init()
{
    RETAILMSG(ZONE_SHADER, "GradientEffect::Init()");

    RESULT rval = S_OK;

    if (s_GradientShadersLoaded)
    {
        return S_OK;
    }

    CHR(OpenGLESEffect::Init());

    CHR(ShaderMan.CreateShader( "GradientEffect", s_GradientVertexShaderName, s_GradientFragmentShaderName, &s_hGradientShader ));
    
    // Save uniform handles.
    s_GradientShaderProgram = ShaderMan.GetShaderProgramID( s_hGradientShader );
    VERIFYGL(glUseProgram(s_GradientShaderProgram));

    VERIFYGL(s_uModelViewMatrix     = glGetUniformLocation(s_GradientShaderProgram, "uMatModelView"));
    VERIFYGL(s_uProjectionMatrix    = glGetUniformLocation(s_GradientShaderProgram, "uMatProjection"));
    VERIFYGL(s_uTexture             = glGetUniformLocation(s_GradientShaderProgram, "uTexture"));
    VERIFYGL(s_uStartColor          = glGetUniformLocation(s_GradientShaderProgram, "uStartColor"));
    VERIFYGL(s_uStartPoint          = glGetUniformLocation(s_GradientShaderProgram, "uStartPoint"));
    VERIFYGL(s_uEndColor            = glGetUniformLocation(s_GradientShaderProgram, "uEndColor"));
    VERIFYGL(s_uEndPoint            = glGetUniformLocation(s_GradientShaderProgram, "uEndPoint"));
    VERIFYGL(s_uGlobalColor         = glGetUniformLocation(s_GradientShaderProgram, "uGlobalColor"));
    VERIFYGL(s_uWidth               = glGetUniformLocation(s_GradientShaderProgram, "uWidth"));
    VERIFYGL(s_uHeight              = glGetUniformLocation(s_GradientShaderProgram, "uHeight"));
//    VERIFYGL(s_uTexCoordOffset      = glGetUniformLocation(s_GradientShaderProgram, "uTexCoordOffset"));
    

    s_GradientShadersLoaded = true;

Exit:
    return rval;
}



RESULT 
GradientEffect::Deinit()
{
    RETAILMSG(ZONE_SHADER, "GradientEffect::Deinit()");

    RESULT rval = S_OK;

    CHR(ShaderMan.Release( s_hGradientShader ));
    s_GradientShaderProgram  = 0;
    s_GradientShadersLoaded  = false;

Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GradientEffect::Deinit(): failed to free shaders");
    }

    return rval;
}
    
    
    
HShader
GradientEffect::GetShader()
{
    return s_hGradientShader;
}




IProperty*
GradientEffect::GetProperty( const string& propertyName ) const
{
    return s_properties.Get( this, propertyName );
}


} // END namespace Z

