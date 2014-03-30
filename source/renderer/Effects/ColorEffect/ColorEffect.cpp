#include "OpenGLESEffect.hpp"
#include "ColorEffect.hpp"
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
    DECLARE_PROPERTY( ColorEffect, PROPERTY_COLOR,  Color       ),
    DECLARE_PROPERTY( ColorEffect, PROPERTY_FLOAT,  Blend       ),
    DECLARE_PROPERTY( ColorEffect, PROPERTY_VEC2,   Origin      ),
    DECLARE_PROPERTY( ColorEffect, PROPERTY_FLOAT,  Radius      ),
    NULL
};
DECLARE_PROPERTY_SET( ColorEffect, s_propertyTable );





//----------------------------------------------------------------------------
// Class data
//----------------------------------------------------------------------------
UINT32                  ColorEffect::s_NumInstances                = NULL;

bool                    ColorEffect::s_ColorShadersLoaded           = false;
GLuint                  ColorEffect::s_ColorShaderProgram           = 0;
HShader                 ColorEffect::s_hColorShader;

const char*             ColorEffect::s_ColorVertexShaderName        = "/app/shaders/ColorEffect_vs.glsl";
const char*             ColorEffect::s_ColorFragmentShaderName      = "/app/shaders/ColorEffect_fs.glsl";

GLint                   ColorEffect::s_uProjectionMatrix            = 0;
GLint                   ColorEffect::s_uModelViewMatrix             = 0;
GLint                   ColorEffect::s_uTexture                     = 0;
GLint                   ColorEffect::s_uGlobalColor                 = 0;
GLint                   ColorEffect::s_uColor                       = 0;
GLint                   ColorEffect::s_uBlend                       = 0;
GLint                   ColorEffect::s_uOrigin                      = 0;
GLint                   ColorEffect::s_uRadius                      = 0;

const Color             ColorEffect::DEFAULT_COLOR                  = Color(0.3,  0.59, 0.11, 1.0);
const float             ColorEffect::DEFAULT_ORIGIN_X               = 0.5;
const float             ColorEffect::DEFAULT_ORIGIN_Y               = 0.5;
const float             ColorEffect::DEFAULT_RADIUS                 = 0.0;



//----------------------------------------------------------------------------
// Object methods
//----------------------------------------------------------------------------


ColorEffect::ColorEffect() :
    m_origin(0,0),
    m_fRadius(DEFAULT_RADIUS),
    m_color(DEFAULT_COLOR),
    m_fBlend(1.0)
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "ColorEffect( %4d )", m_ID);

    ATOMIC_INCREMENT( ColorEffect::s_NumInstances );

    m_name = "ColorEffect";
    memset(&m_frame, 0, sizeof(m_frame));
}


ColorEffect::~ColorEffect()
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "\t~ColorEffect( %4d )", m_ID);

    // Delete the shaders when the last instance is freed
    if (0 == ATOMIC_DECREMENT( ColorEffect::s_NumInstances ))
    {
        RETAILMSG(ZONE_INFO, "~ColorEffect: last instance deleted, freeing Color shaders");
        Deinit();
    }
}



ColorEffect*
ColorEffect::Clone() const
{
    return new ColorEffect(*this);
}


ColorEffect::ColorEffect( const ColorEffect& rhs ) : 
    OpenGLESEffect(),
    m_color(DEFAULT_COLOR),
    m_fBlend(1.0),
    m_fRadius(DEFAULT_RADIUS),
    m_origin(0,0)
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "ColorEffect( %4d ) (copy ctor)", m_ID);

    ATOMIC_INCREMENT( ColorEffect::s_NumInstances );

    m_name = "ColorEffect";
    memset(&m_frame, 0, sizeof(m_frame));

    *this = rhs;
}


ColorEffect& 
ColorEffect::operator=( const ColorEffect& rhs )
{
    // Avoid self-assignment.
    if (this == &rhs)
        return *this;
    

    // Copy base class (is there a cleaner syntax?)
    OpenGLESEffect::operator=(rhs);


    // SHALLOW COPY:
    m_color                         = rhs.m_color;
    m_fBlend                        = rhs.m_fBlend;
    m_fRadius                       = rhs.m_fRadius;
    m_origin                        = rhs.m_origin;
    

    // Give it a new name with a random suffix
    char instanceName[MAX_NAME];
    sprintf(instanceName, "%s_%X", rhs.m_name.c_str(), (unsigned int)Platform::Random());
    m_name = string(instanceName);
    
    return *this;
}



#pragma mark -
#pragma mark Drawing

RESULT 
ColorEffect::DrawTriangleStrip( Vertex *pVertices, UINT32 numVertices )
{
    return ColorEffect::Draw( pVertices, numVertices, PRIMITIVE_TYPE_TRIANGLE_STRIP );
}


RESULT 
ColorEffect::DrawTriangleList( Vertex *pVertices, UINT32 numVertices )
{
    return ColorEffect::Draw( pVertices, numVertices, PRIMITIVE_TYPE_TRIANGLE_LIST );
}



RESULT 
ColorEffect::Draw( Vertex *pVertices, UINT32 numVertices, PRIMITIVE_TYPE primitiveType  )
{
    DEBUGMSG(ZONE_SHADER, "ColorEffect[%d]::DrawTriangleStrip()", m_ID);

    RESULT     rval          = S_OK;


    // Assign values to shader parameters.
    // TODO: move to BeginPass()?
    VERIFYGL(glUseProgram(s_ColorShaderProgram));
    VERIFYGL(glUniformMatrix4fv(s_uProjectionMatrix, 1, GL_FALSE, (GLfloat*)m_projectionMatrix.Pointer()));
    VERIFYGL(glUniformMatrix4fv(s_uModelViewMatrix,  1, GL_FALSE, (GLfloat*)m_modelViewMatrix.Pointer()));
    VERIFYGL(glUniform1i(s_uTexture,        0));
    VERIFYGL(glUniform4f(s_uGlobalColor,   m_globalColor.floats.r, m_globalColor.floats.g, m_globalColor.floats.b, m_globalColor.floats.a));
    VERIFYGL(glUniform4f(s_uColor,          m_color.floats.r, m_color.floats.g, m_color.floats.b, m_color.floats.a));
    VERIFYGL(glUniform1f(s_uBlend,          m_fBlend));
    VERIFYGL(glUniform1f(s_uRadius,         m_fRadius));
    VERIFYGL(glUniform2f(s_uOrigin,         m_origin.x, m_origin.y));
    
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
ColorEffect::SetColor( const Color& color )
{
    m_color = color;
}


Color
ColorEffect::GetColor()
{
    return m_color;
}



void
ColorEffect::SetBlend( float blend )
{
    m_fBlend = blend;
}


float
ColorEffect::GetBlend()
{
    return m_fBlend;
}



void
ColorEffect::SetRadius( float radius )
{
    m_fRadius = MAX(radius, 0.0f);
}


float
ColorEffect::GetRadius()
{
    return m_fRadius;
}



void
ColorEffect::SetOrigin( const vec2& origin )
{
    m_origin = origin;
}


vec2
ColorEffect::GetOrigin()
{
    return m_origin;
}




//----------------------------------------------------------------------------
// Class methods
//----------------------------------------------------------------------------

ColorEffect* 
ColorEffect::CreateInstance()
{
    DEBUGMSG(ZONE_SHADER, "ColorEffect::CreateInstance()");

    RESULT        rval      = S_OK;
    ColorEffect* pEffect   = NULL;

    if (Platform::IsOpenGLES2())
    {
        if (!s_ColorShadersLoaded)
        {
            CHR(Init());
        }

        CPREx(pEffect = new ColorEffect(), E_OUTOFMEMORY);
    }
    else 
    {
        RETAILMSG(ZONE_WARN, "Device doesn't support ColorEffect.");
    }

Exit:
    return pEffect;
}



RESULT 
ColorEffect::Init()
{
    RETAILMSG(ZONE_SHADER, "ColorEffect::Init()");

    RESULT rval = S_OK;

    if (s_ColorShadersLoaded)
    {
        return S_OK;
    }

    CHR(OpenGLESEffect::Init());

    CHR(ShaderMan.CreateShader( "ColorEffect", s_ColorVertexShaderName, s_ColorFragmentShaderName, &s_hColorShader ));
    
    // Save uniform handles.
    s_ColorShaderProgram = ShaderMan.GetShaderProgramID( s_hColorShader );
    VERIFYGL(glUseProgram(s_ColorShaderProgram));

    VERIFYGL(s_uModelViewMatrix     = glGetUniformLocation(s_ColorShaderProgram, "uMatModelView"));
    VERIFYGL(s_uProjectionMatrix    = glGetUniformLocation(s_ColorShaderProgram, "uMatProjection"));
    VERIFYGL(s_uTexture             = glGetUniformLocation(s_ColorShaderProgram, "uTexture"));
    VERIFYGL(s_uGlobalColor         = glGetUniformLocation(s_ColorShaderProgram, "uGlobalColor"));
    VERIFYGL(s_uColor               = glGetUniformLocation(s_ColorShaderProgram, "uColor"));
    VERIFYGL(s_uBlend               = glGetUniformLocation(s_ColorShaderProgram, "uBlend"));
    VERIFYGL(s_uRadius              = glGetUniformLocation(s_ColorShaderProgram, "uRadius"));
    VERIFYGL(s_uOrigin              = glGetUniformLocation(s_ColorShaderProgram, "uOrigin"));
    
    s_ColorShadersLoaded = true;

Exit:
    return rval;
}



RESULT 
ColorEffect::Deinit()
{
    RETAILMSG(ZONE_SHADER, "ColorEffect::Deinit()");

    RESULT rval = S_OK;

    CHR(ShaderMan.Release( s_hColorShader ));
    s_ColorShaderProgram  = 0;
    s_ColorShadersLoaded  = false;

Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ColorEffect::Deinit(): failed to free shaders");
    }

    return rval;
}
    
    
    
HShader
ColorEffect::GetShader()
{
    return s_hColorShader;
}




IProperty*
ColorEffect::GetProperty( const string& propertyName ) const
{
    return s_properties.Get( this, propertyName );
}


} // END namespace Z

