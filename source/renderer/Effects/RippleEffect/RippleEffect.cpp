#include "OpenGLESEffect.hpp"
#include "RippleEffect.hpp"
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
    DECLARE_PROPERTY( RippleEffect, PROPERTY_UINT32, Rows        ),
    DECLARE_PROPERTY( RippleEffect, PROPERTY_UINT32, Columns     ),
    DECLARE_PROPERTY( RippleEffect, PROPERTY_UINT32, Padding     ),
    DECLARE_PROPERTY( RippleEffect, PROPERTY_VEC2,   Origin      ),
    DECLARE_PROPERTY( RippleEffect, PROPERTY_FLOAT,  Amplitude   ),
    DECLARE_PROPERTY( RippleEffect, PROPERTY_FLOAT,  Speed       ),
    DECLARE_PROPERTY( RippleEffect, PROPERTY_FLOAT,  Radius      ),
    DECLARE_PROPERTY( RippleEffect, PROPERTY_FLOAT,  WaveLength  ),
    DECLARE_PROPERTY( RippleEffect, PROPERTY_FLOAT,  NumWaves    ),
    DECLARE_PROPERTY( RippleEffect, PROPERTY_COLOR,  Color       ),
    NULL
};
DECLARE_PROPERTY_SET( RippleEffect, s_propertyTable );





//----------------------------------------------------------------------------
// Class data
//----------------------------------------------------------------------------
UINT32                  RippleEffect::s_NumInstances                = NULL;

bool                    RippleEffect::s_RippleShadersLoaded         = false;
GLuint                  RippleEffect::s_RippleShaderProgram         = 0;
HShader                 RippleEffect::s_hRippleShader;

const char*             RippleEffect::s_RippleVertexShaderName      = "/app/shaders/RippleEffect_vs.glsl";
const char*             RippleEffect::s_RippleFragmentShaderName    = "/app/shaders/RippleEffect_fs.glsl";

GLint                   RippleEffect::s_uProjectionMatrix           = 0;
GLint                   RippleEffect::s_uModelViewMatrix            = 0;
GLint                   RippleEffect::s_uTexture                    = 0;
GLint                   RippleEffect::s_uGlobalColor                = 0;
GLint                   RippleEffect::s_uOrigin                     = 0;
GLint                   RippleEffect::s_uAmplitude                  = 0;
GLint                   RippleEffect::s_uTime                       = 0;
GLint                   RippleEffect::s_uRadius                     = 0;
GLint                   RippleEffect::s_uHalfWaveLength             = 0;
GLint                   RippleEffect::s_uNumWaves                   = 0;
GLint                   RippleEffect::s_uColor                      = 0;

const UINT8             RippleEffect::DEFAULT_ROWS                  = 15;
const UINT8             RippleEffect::DEFAULT_COLUMNS               = 15;
const UINT32            RippleEffect::DEFAULT_PADDING               = 20;
const float             RippleEffect::DEFAULT_AMPLITUDE             = 2.0;
const float             RippleEffect::DEFAULT_ORIGIN_X              = 0.5;
const float             RippleEffect::DEFAULT_ORIGIN_Y              = 0.5;
const float             RippleEffect::DEFAULT_SPEED                 = 4.0;
const float             RippleEffect::DEFAULT_RADIUS                = 0.0;
const float             RippleEffect::DEFAULT_WAVE_LENGTH           = 20.0;
const float             RippleEffect::DEFAULT_NUM_WAVES             = 1.0;
const Color             RippleEffect::DEFAULT_COLOR                 = Color::Clear();
const float             RippleEffect::MAX_AMPLITUDE                 = 100.0;
const float             RippleEffect::MAX_SPEED                     = 100.0;



//----------------------------------------------------------------------------
// Object methods
//----------------------------------------------------------------------------


RippleEffect::RippleEffect() :
    m_pRippledVertices(NULL),
    m_numRippledVertices(0),
    m_rows(DEFAULT_ROWS),
    m_columns(DEFAULT_COLUMNS),
    m_padding(DEFAULT_PADDING),
    m_fWaveLength(DEFAULT_WAVE_LENGTH),
    m_fNumWaves(DEFAULT_NUM_WAVES),
    m_fAmplitude(DEFAULT_AMPLITUDE),
    m_fSpeed(DEFAULT_SPEED),
    m_fRadius(DEFAULT_RADIUS),
    m_color(DEFAULT_COLOR),
    m_origin(0,0)
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "RippleEffect( %4d )", m_ID);

    ATOMIC_INCREMENT( RippleEffect::s_NumInstances );


    // This effect needs render-to-texture.  The result is then presented on-screen in ::EndFrame()
    m_needsRenderTarget = true;
    m_isPostEffect      = true;


    m_name = "RippleEffect";
    memset(&m_frame, 0, sizeof(m_frame));
}


RippleEffect::~RippleEffect()
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "\t~RippleEffect( %4d )", m_ID);

    SAFE_ARRAY_DELETE(m_pRippledVertices);

    // Delete the shaders when the last instance is freed
    if (0 == ATOMIC_DECREMENT( RippleEffect::s_NumInstances ))
    {
        RETAILMSG(ZONE_INFO, "~RippleEffect: last instance deleted, freeing Ripple shaders");
        Deinit();
    }
}



RippleEffect*
RippleEffect::Clone() const
{
    return new RippleEffect(*this);
}


RippleEffect::RippleEffect( const RippleEffect& rhs ) : 
    OpenGLESEffect(),
    m_pRippledVertices(NULL),
    m_numRippledVertices(0),
    m_rows(DEFAULT_ROWS),
    m_columns(DEFAULT_COLUMNS),
    m_padding(DEFAULT_PADDING),
    m_fWaveLength(DEFAULT_WAVE_LENGTH),
    m_fNumWaves(DEFAULT_NUM_WAVES),
    m_fAmplitude(DEFAULT_AMPLITUDE),
    m_fSpeed(DEFAULT_SPEED),
    m_fRadius(DEFAULT_RADIUS),
    m_color(DEFAULT_COLOR),
    m_origin(0,0)
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "RippleEffect( %4d ) (copy ctor)", m_ID);

    ATOMIC_INCREMENT( RippleEffect::s_NumInstances );

    m_name = "RippleEffect";
    memset(&m_frame, 0, sizeof(m_frame));

    *this = rhs;
}


RippleEffect& 
RippleEffect::operator=( const RippleEffect& rhs )
{
    // Avoid self-assignment.
    if (this == &rhs)
        return *this;
    

    // Copy base class (is there a cleaner syntax?)
    OpenGLESEffect::operator=(rhs);


    // SHALLOW COPY:
    m_numRippledVertices            = rhs.m_numRippledVertices;
    m_rows                          = rhs.m_rows;
    m_columns                       = rhs.m_columns;
    m_padding                       = rhs.m_padding;
    m_fAmplitude                    = rhs.m_fAmplitude;
    m_fSpeed                        = rhs.m_fSpeed;
    m_fRadius                       = rhs.m_fRadius;
    m_fWaveLength                   = rhs.m_fWaveLength;
    m_fNumWaves                     = rhs.m_fNumWaves;
    m_origin                        = rhs.m_origin;
    m_color                         = rhs.m_color;
    

    // DEEP COPY: vertices
    if (rhs.m_pRippledVertices)
    {
        SAFE_ARRAY_DELETE(m_pRippledVertices);
        m_pRippledVertices = new Vertex[ rhs.m_numRippledVertices ];
        memcpy(m_pRippledVertices, rhs.m_pRippledVertices, sizeof(Vertex)*rhs.m_numRippledVertices);
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
RippleEffect::DrawTriangleStrip( Vertex *pVertices, UINT32 numVertices )
{
    return RippleEffect::Draw( pVertices, numVertices, PRIMITIVE_TYPE_TRIANGLE_STRIP );
}


RESULT 
RippleEffect::DrawTriangleList( Vertex *pVertices, UINT32 numVertices )
{
    return RippleEffect::Draw( pVertices, numVertices, PRIMITIVE_TYPE_TRIANGLE_LIST );
}



RESULT 
RippleEffect::Draw( Vertex *pVertices, UINT32 numVertices, PRIMITIVE_TYPE primitiveType  )
{
    DEBUGMSG(ZONE_SHADER, "RippleEffect[%d]::DrawTriangleStrip()", m_ID);

    RESULT     rval          = S_OK;
    Rectangle  sourceRect;
    Rectangle  textureRect;
    Util::GetBoundingRect  ( pVertices, numVertices, &sourceRect  );
    Util::GetTextureMapping( pVertices, numVertices, &textureRect );



    // TODO: reinit if our source frame changes, i.e. the object
    // we're rippling is being scaled.
    if (!m_pRippledVertices || m_frameChanged)
    {
        CHR(InitScratchSurfaces( sourceRect, textureRect ));
        m_frameChanged = false;
    }

    // Assign values to shader parameters.
    // TODO: move to BeginPass()?
    VERIFYGL(glUseProgram(s_RippleShaderProgram));
    VERIFYGL(glUniformMatrix4fv(s_uProjectionMatrix, 1, GL_FALSE, (GLfloat*)m_projectionMatrix.Pointer()));
    VERIFYGL(glUniformMatrix4fv(s_uModelViewMatrix,  1, GL_FALSE, (GLfloat*)m_modelViewMatrix.Pointer()));
    VERIFYGL(glUniform1i(s_uTexture,        0));
    VERIFYGL(glUniform4f(s_uGlobalColor,    m_globalColor.floats.r, m_globalColor.floats.g, m_globalColor.floats.b, m_globalColor.floats.a));
    VERIFYGL(glUniform1f(s_uAmplitude,      m_fAmplitude));
    VERIFYGL(glUniform1f(s_uTime,           m_fSpeed * GameTime.GetTimeDouble()));
    VERIFYGL(glUniform1f(s_uRadius,         m_fRadius));
    VERIFYGL(glUniform1f(s_uHalfWaveLength, m_fWaveLength/2.0f));
    VERIFYGL(glUniform1f(s_uNumWaves,       m_fNumWaves));
    VERIFYGL(glUniform2f(s_uOrigin,         m_origin.x, m_origin.y));
    VERIFYGL(glUniform4f(s_uColor,          m_color.floats.r, m_color.floats.g, m_color.floats.b, m_color.floats.a));
    
    // Draw the texture mapped onto the rippling mesh.
    VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_POSITION,       3, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &m_pRippledVertices->x));
    VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_DIFFUSE,        4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(Vertex), &m_pRippledVertices->color));
    VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_TEXTURE_COORD0, 2, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &m_pRippledVertices->texCoord[0]));

    switch (primitiveType)
    {
        // NOTE: for this effect we ignore the passed-in vertices and draw a triangle list (GL_TRIANGLES) not a triangle strip (GL_TRIANGLE_STRIP).
        case PRIMITIVE_TYPE_TRIANGLE_STRIP:
        case PRIMITIVE_TYPE_TRIANGLE_LIST:
            VERIFYGL(glDrawArrays(GL_TRIANGLES, 0, m_numRippledVertices));
            break;
        default:
            // TODO: might it ever be useful to apply Ripple to a field of point sprites?
            DEBUGCHK(0);
    }

Exit:
    return rval;
}



void
RippleEffect::SetPadding( UINT32 padding )
{
    m_padding = padding;
}


UINT32
RippleEffect::GetPadding()
{
    return m_padding;
}



void
RippleEffect::SetRows( UINT8 rows )
{
    m_rows = rows;
}


UINT8
RippleEffect::GetRows()
{
    return m_rows;
}




void
RippleEffect::SetColumns( UINT8 columns )
{
    m_columns = columns;
}


UINT8
RippleEffect::GetColumns()
{
    return m_columns;
}





void
RippleEffect::SetAmplitude( float amplitude )
{
    m_fAmplitude = MIN(amplitude, MAX_AMPLITUDE);
}


float
RippleEffect::GetAmplitude()
{
    return m_fAmplitude;
}



void
RippleEffect::SetSpeed( float speed )
{
    m_fSpeed = MIN(speed, MAX_SPEED);
}


float
RippleEffect::GetSpeed()
{
    return m_fSpeed;
}



void
RippleEffect::SetRadius( float radius )
{
    m_fRadius = MAX(radius, 0.0f);
}


float
RippleEffect::GetRadius()
{
    return m_fRadius;
}



void
RippleEffect::SetWaveLength( float waveLength )
{
    m_fWaveLength = MAX(waveLength, 1.0f);
}


float
RippleEffect::GetWaveLength()
{
    return m_fWaveLength;
}



void
RippleEffect::SetNumWaves( float numWaves )
{
    m_fNumWaves = MAX(numWaves, 1.0f);
}


float
RippleEffect::GetNumWaves()
{
    return m_fNumWaves;
}



void
RippleEffect::SetColor( const Color& color )
{
    m_color = color;
}


Color
RippleEffect::GetColor()
{
    return m_color;
}



void
RippleEffect::SetOrigin( const vec2& origin )
{
    m_origin = origin;
}


vec2
RippleEffect::GetOrigin()
{
    return m_origin;
}



RESULT 
RippleEffect::InitScratchSurfaces( Rectangle& sourceRect, Rectangle& textureRect )
{
    RESULT rval = S_OK;

    // We render an animated mesh in place of the triangle mesh sent down from the caller.
    //
    // Create the new mesh with the same position and dimensions (plus optional padding), 
    // and a reasonable number of number of triangles (so the ripples look fluid).
    
    Rectangle paddedRect = sourceRect;
    paddedRect.x        -= m_padding;
    paddedRect.y        -= m_padding;
    paddedRect.width    += m_padding*2;
    paddedRect.height   += m_padding*2;
    
//    Rectangle paddedTextureRect = textureRect;
//    paddedTextureRect.x  -= sourceRect.width - paddedRect.width;
      

    float uStart    = textureRect.x;
    float uEnd      = textureRect.x + textureRect.width;
    float vStart    = textureRect.y;
    float vEnd      = textureRect.y + textureRect.height;
    
    SAFE_ARRAY_DELETE(m_pRippledVertices);
    
    CHR(Util::CreateTriangleList( &paddedRect, m_rows, m_columns, &m_pRippledVertices, &m_numRippledVertices, uStart, uEnd, vStart, vEnd  ));

    // TODO: create a VBO.

Exit:
    return rval;
}



//----------------------------------------------------------------------------
// Class methods
//----------------------------------------------------------------------------

RippleEffect* 
RippleEffect::CreateInstance()
{
    DEBUGMSG(ZONE_RENDER, "RippleEffect::CreateInstance()");

    RESULT        rval      = S_OK;
    RippleEffect* pEffect   = NULL;

    if (Platform::IsOpenGLES2())
    {
        if (!s_RippleShadersLoaded)
        {
            CHR(Init());
        }

        CPREx(pEffect = new RippleEffect(), E_OUTOFMEMORY);
    }
    else 
    {
        RETAILMSG(ZONE_WARN, "Device doesn't support RippleEffect.");
    }

Exit:
    return pEffect;
}



RESULT 
RippleEffect::Init()
{
    RETAILMSG(ZONE_RENDER, "RippleEffect::Init()");

    RESULT rval = S_OK;

    if (s_RippleShadersLoaded)
    {
        return S_OK;
    }

    CHR(OpenGLESEffect::Init());

    CHR(ShaderMan.CreateShader( "RippleEffect", s_RippleVertexShaderName, s_RippleFragmentShaderName, &s_hRippleShader ));
    
    // Save uniform handles.
    s_RippleShaderProgram = ShaderMan.GetShaderProgramID( s_hRippleShader );
    VERIFYGL(glUseProgram(s_RippleShaderProgram));

    VERIFYGL(s_uModelViewMatrix     = glGetUniformLocation(s_RippleShaderProgram, "uMatModelView"));
    VERIFYGL(s_uProjectionMatrix    = glGetUniformLocation(s_RippleShaderProgram, "uMatProjection"));
    VERIFYGL(s_uTexture             = glGetUniformLocation(s_RippleShaderProgram, "uTexture"));
    VERIFYGL(s_uGlobalColor         = glGetUniformLocation(s_RippleShaderProgram, "uGlobalColor"));
    VERIFYGL(s_uAmplitude           = glGetUniformLocation(s_RippleShaderProgram, "uAmplitude"));
    VERIFYGL(s_uTime                = glGetUniformLocation(s_RippleShaderProgram, "uTime"));
    VERIFYGL(s_uRadius              = glGetUniformLocation(s_RippleShaderProgram, "uRadius"));
    VERIFYGL(s_uOrigin              = glGetUniformLocation(s_RippleShaderProgram, "uOrigin"));
    VERIFYGL(s_uHalfWaveLength      = glGetUniformLocation(s_RippleShaderProgram, "uHalfWaveLength"));
    VERIFYGL(s_uNumWaves            = glGetUniformLocation(s_RippleShaderProgram, "uNumWaves"));
    VERIFYGL(s_uColor               = glGetUniformLocation(s_RippleShaderProgram, "uColor"));
    
    s_RippleShadersLoaded = true;

Exit:
    return rval;
}



RESULT 
RippleEffect::Deinit()
{
    RETAILMSG(ZONE_RENDER, "RippleEffect::Deinit()");

    RESULT rval = S_OK;

    CHR(ShaderMan.Release( s_hRippleShader ));
    s_RippleShaderProgram  = 0;
    s_RippleShadersLoaded  = false;

Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: RippleEffect::Deinit(): failed to free shaders");
    }

    return rval;
}



HShader
RippleEffect::GetShader()
{
    return s_hRippleShader;
}



IProperty*
RippleEffect::GetProperty( const string& propertyName ) const
{
    return s_properties.Get( this, propertyName );
}


} // END namespace Z

