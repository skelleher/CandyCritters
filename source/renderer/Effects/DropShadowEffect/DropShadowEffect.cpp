#include "OpenGLESEffect.hpp"
#include "DropShadowEffect.hpp"
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
    DECLARE_PROPERTY( DropShadowEffect, PROPERTY_COLOR,  Color       ),
    DECLARE_PROPERTY( DropShadowEffect, PROPERTY_FLOAT,  Depth       ),
    DECLARE_PROPERTY( DropShadowEffect, PROPERTY_FLOAT,  Direction   ),
    NULL
};
DECLARE_PROPERTY_SET( DropShadowEffect, s_propertyTable );





//----------------------------------------------------------------------------
// Class data
//----------------------------------------------------------------------------
UINT32                  DropShadowEffect::s_NumInstances                = NULL;

bool                    DropShadowEffect::s_ColorShadersLoaded         = false;
GLuint                  DropShadowEffect::s_ColorShaderProgram         = 0;
HShader                 DropShadowEffect::s_hColorShader;

const char*             DropShadowEffect::s_ColorVertexShaderName      = "/app/shaders/DropShadowEffect_vs.glsl";
const char*             DropShadowEffect::s_ColorFragmentShaderName    = "/app/shaders/DropShadowEffect_fs.glsl";

GLint                   DropShadowEffect::s_uProjectionMatrix           = 0;
GLint                   DropShadowEffect::s_uModelViewMatrix            = 0;
GLint                   DropShadowEffect::s_uTexture                    = 0;
GLint                   DropShadowEffect::s_uGlobalColor               = 0;
GLint                   DropShadowEffect::s_uColor                      = 0;
GLint                   DropShadowEffect::s_uDepth                      = 0;
GLint                   DropShadowEffect::s_uDirection                  = 0;

const Color             DropShadowEffect::DEFAULT_COLOR                 = Color(0.15, 0.15, 0.15, 0.6);
const float             DropShadowEffect::DEFAULT_DEPTH                 = 16.0f;
const float             DropShadowEffect::DEFAULT_DIRECTION_DEGREES     = 315.0f;



//----------------------------------------------------------------------------
// Object methods
//----------------------------------------------------------------------------


DropShadowEffect::DropShadowEffect() :
    m_color(DEFAULT_COLOR),
    m_fDirectionDegrees(DEFAULT_DIRECTION_DEGREES),
    m_fDepth(DEFAULT_DEPTH)
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "DropShadowEffect( %4d )", m_ID);

    ATOMIC_INCREMENT( DropShadowEffect::s_NumInstances );

    m_name = "DropShadowEffect";
    memset(&m_frame, 0, sizeof(m_frame));
}


DropShadowEffect::~DropShadowEffect()
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "\t~DropShadowEffect( %4d )", m_ID);

    // Delete the shaders when the last instance is freed
    if (0 == ATOMIC_DECREMENT( DropShadowEffect::s_NumInstances ))
    {
        RETAILMSG(ZONE_INFO, "~DropShadowEffect: last instance deleted, freeing Color shaders");
        Deinit();
    }
}



DropShadowEffect*
DropShadowEffect::Clone() const
{
    return new DropShadowEffect(*this);
}


DropShadowEffect::DropShadowEffect( const DropShadowEffect& rhs ) : 
    OpenGLESEffect(),
    m_color(DEFAULT_COLOR),
    m_fDirectionDegrees(DEFAULT_DIRECTION_DEGREES),
    m_fDepth(DEFAULT_DEPTH)
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "DropShadowEffect( %4d ) (copy ctor)", m_ID);

    ATOMIC_INCREMENT( DropShadowEffect::s_NumInstances );

    m_name = "DropShadowEffect";
    memset(&m_frame, 0, sizeof(m_frame));

    *this = rhs;
}


DropShadowEffect& 
DropShadowEffect::operator=( const DropShadowEffect& rhs )
{
    // Avoid self-assignment.
    if (this == &rhs)
        return *this;
    

    // Copy base class (is there a cleaner syntax?)
    OpenGLESEffect::operator=(rhs);


    // SHALLOW COPY:
    m_color                         = rhs.m_color;
    m_fDepth                        = rhs.m_fDepth;
    m_fDirectionDegrees             = rhs.m_fDirectionDegrees;
    

    // Give it a new name with a random suffix
    char instanceName[MAX_NAME];
    sprintf(instanceName, "%s_%X", rhs.m_name.c_str(), (unsigned int)Platform::Random());
    m_name = string(instanceName);
    
    return *this;
}



#pragma mark -
#pragma mark Drawing

RESULT 
DropShadowEffect::DrawTriangleStrip( Vertex *pVertices, UINT32 numVertices )
{
    return DropShadowEffect::Draw( pVertices, numVertices, PRIMITIVE_TYPE_TRIANGLE_STRIP );
}


RESULT 
DropShadowEffect::DrawTriangleList( Vertex *pVertices, UINT32 numVertices )
{
    return DropShadowEffect::Draw( pVertices, numVertices, PRIMITIVE_TYPE_TRIANGLE_LIST );
}



RESULT 
DropShadowEffect::Draw( Vertex *pVertices, UINT32 numVertices, PRIMITIVE_TYPE primitiveType  )
{
    DEBUGMSG(ZONE_SHADER, "DropShadowEffect[%d]::DrawTriangleStrip()", m_ID);

    RESULT     rval          = S_OK;


    // Assign values to shader parameters.
    // TODO: move to BeginPass()?
    VERIFYGL(glUseProgram(s_ColorShaderProgram));
    VERIFYGL(glUniformMatrix4fv(s_uProjectionMatrix, 1, GL_FALSE, (GLfloat*)m_projectionMatrix.Pointer()));
    VERIFYGL(glUniformMatrix4fv(s_uModelViewMatrix,  1, GL_FALSE, (GLfloat*)m_modelViewMatrix.Pointer()));
    VERIFYGL(glUniform1i(s_uTexture,        0));
    VERIFYGL(glUniform4f(s_uGlobalColor,    m_globalColor.floats.r, m_globalColor.floats.g, m_globalColor.floats.b, m_globalColor.floats.a));
    VERIFYGL(glUniform4f(s_uColor,          m_color.floats.r, m_color.floats.g, m_color.floats.b, m_color.floats.a));
    VERIFYGL(glUniform1f(s_uDepth,          m_fDepth));
    VERIFYGL(glUniform1f(s_uDirection,      m_fDirectionDegrees));
    
    // Draw the texture mapped onto the rippling mesh.
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
            // TODO: might it ever be useful to apply Color to a field of point sprites?
            DEBUGCHK(0);
    }

Exit:
    return rval;
}



void
DropShadowEffect::SetColor( const Color& color )
{
    m_color = color;
}


Color
DropShadowEffect::GetColor()
{
    return m_color;
}



void
DropShadowEffect::SetDepth( float depth )
{
    m_fDepth = depth;
}


float
DropShadowEffect::GetDepth()
{
    return m_fDepth;
}


void
DropShadowEffect::SetDirection( float directionDegres )
{
    m_fDirectionDegrees = directionDegres;
}


float
DropShadowEffect::GetDirection()
{
    return m_fDirectionDegrees;
}




//----------------------------------------------------------------------------
// Class methods
//----------------------------------------------------------------------------

DropShadowEffect* 
DropShadowEffect::CreateInstance()
{
    DEBUGMSG(ZONE_SHADER, "DropShadowEffect::CreateInstance()");

    RESULT        rval      = S_OK;
    DropShadowEffect* pEffect   = NULL;

    if (Platform::IsOpenGLES2())
    {
        if (!s_ColorShadersLoaded)
        {
            CHR(Init());
        }

        CPREx(pEffect = new DropShadowEffect(), E_OUTOFMEMORY);
    }
    else 
    {
        RETAILMSG(ZONE_WARN, "Device doesn't support DropShadowEffect.");
    }

Exit:
    return pEffect;
}



RESULT 
DropShadowEffect::Init()
{
    RETAILMSG(ZONE_SHADER, "DropShadowEffect::Init()");

    RESULT rval = S_OK;

    if (s_ColorShadersLoaded)
    {
        return S_OK;
    }

    CHR(OpenGLESEffect::Init());

    CHR(ShaderMan.CreateShader( "DropShadowEffect", s_ColorVertexShaderName, s_ColorFragmentShaderName, &s_hColorShader ));
    
    // Save uniform handles.
    s_ColorShaderProgram = ShaderMan.GetShaderProgramID( s_hColorShader );
    VERIFYGL(glUseProgram(s_ColorShaderProgram));

    VERIFYGL(s_uModelViewMatrix     = glGetUniformLocation(s_ColorShaderProgram, "uMatModelView"));
    VERIFYGL(s_uProjectionMatrix    = glGetUniformLocation(s_ColorShaderProgram, "uMatProjection"));
    VERIFYGL(s_uTexture             = glGetUniformLocation(s_ColorShaderProgram, "uTexture"));
    VERIFYGL(s_uColor               = glGetUniformLocation(s_ColorShaderProgram, "uColor"));
    VERIFYGL(s_uDepth               = glGetUniformLocation(s_ColorShaderProgram, "uDepth"));
    VERIFYGL(s_uDirection           = glGetUniformLocation(s_ColorShaderProgram, "uDirectionDegrees"));
    VERIFYGL(s_uGlobalColor         = glGetUniformLocation(s_ColorShaderProgram, "uGlobalColor"));
    
    s_ColorShadersLoaded = true;

Exit:
    return rval;
}



RESULT 
DropShadowEffect::Deinit()
{
    RETAILMSG(ZONE_SHADER, "DropShadowEffect::Deinit()");

    RESULT rval = S_OK;

    CHR(ShaderMan.Release( s_hColorShader ));
    s_ColorShaderProgram  = 0;
    s_ColorShadersLoaded  = false;

Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: DropShadowEffect::Deinit(): failed to free shaders");
    }

    return rval;
}
    
    
    
HShader
DropShadowEffect::GetShader()
{
    return s_hColorShader;
}




IProperty*
DropShadowEffect::GetProperty( const string& propertyName ) const
{
    return s_properties.Get( this, propertyName );
}


} // END namespace Z

