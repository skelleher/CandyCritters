#include "OpenGLESEffect.hpp"
#include "MorphEffect.hpp"
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
    DECLARE_PROPERTY( MorphEffect, PROPERTY_UINT32, Rows                ),
    DECLARE_PROPERTY( MorphEffect, PROPERTY_UINT32, Columns             ),
    DECLARE_PROPERTY( MorphEffect, PROPERTY_VEC3,   Origin              ),

    DECLARE_PROPERTY( MorphEffect, PROPERTY_VEC3,   Curve0p0            ),
    DECLARE_PROPERTY( MorphEffect, PROPERTY_VEC3,   Curve0p1            ),
    DECLARE_PROPERTY( MorphEffect, PROPERTY_VEC3,   Curve0p2            ),
    DECLARE_PROPERTY( MorphEffect, PROPERTY_VEC3,   Curve0p3            ),

    DECLARE_PROPERTY( MorphEffect, PROPERTY_VEC3,   Curve1p0            ),
    DECLARE_PROPERTY( MorphEffect, PROPERTY_VEC3,   Curve1p1            ),
    DECLARE_PROPERTY( MorphEffect, PROPERTY_VEC3,   Curve1p2            ),
    DECLARE_PROPERTY( MorphEffect, PROPERTY_VEC3,   Curve1p3            ),

    NULL
};
DECLARE_PROPERTY_SET( MorphEffect, s_propertyTable );





//----------------------------------------------------------------------------
// Class data
//----------------------------------------------------------------------------
UINT32                  MorphEffect::s_NumInstances                 = NULL;

bool                    MorphEffect::s_MorphShadersLoaded           = false;
GLuint                  MorphEffect::s_MorphShaderProgram           = 0;
HShader                 MorphEffect::s_hMorphShader;

const char*             MorphEffect::s_MorphVertexShaderName        = "/app/shaders/MorphEffect_vs.glsl";
const char*             MorphEffect::s_MorphFragmentShaderName      = "/app/shaders/MorphEffect_fs.glsl";

GLint                   MorphEffect::s_uProjectionMatrix            = 0;
GLint                   MorphEffect::s_uModelViewMatrix             = 0;
GLint                   MorphEffect::s_uTexture                     = 0;
GLint                   MorphEffect::s_uGlobalColor                 = 0;
GLint                   MorphEffect::s_uOrigin                      = 0;

// Bezier curve 0
GLint                   MorphEffect::s_curve0point0Loc              = 0;
GLint                   MorphEffect::s_curve0point1Loc              = 0;
GLint                   MorphEffect::s_curve0point2Loc              = 0;
GLint                   MorphEffect::s_curve0point3Loc              = 0;

// Bezier curve 1
GLint                   MorphEffect::s_curve1point0Loc              = 0;
GLint                   MorphEffect::s_curve1point1Loc              = 0;
GLint                   MorphEffect::s_curve1point2Loc              = 0;
GLint                   MorphEffect::s_curve1point3Loc              = 0;

GLint                   MorphEffect::s_uSourceWidth                 = 0;
GLint                   MorphEffect::s_uSourceHeight                = 0;

// Checkerboard pattern for debugging
GLint                   MorphEffect::s_uCheckerboard                = 0;

const UINT8             MorphEffect::DEFAULT_ROWS                   = 20;
const UINT8             MorphEffect::DEFAULT_COLUMNS                = 20;
const vec3              MorphEffect::DEFAULT_ORIGIN                 = vec3( 0.5, 0.5, 0.0 );

const float             MorphEffect::DEFAULT_WIDTH                  = 640.0;
const float             MorphEffect::DEFAULT_HEIGHT                 = 1136.0;
const float             MorphEffect::DEFAULT_HEIGHT_3_5             = 960.0;



//----------------------------------------------------------------------------
// Object methods
//----------------------------------------------------------------------------


MorphEffect::MorphEffect() :
    m_pMorphedVertices(NULL),
    m_numMorphedVertices(0),
    m_rows(DEFAULT_ROWS),
    m_columns(DEFAULT_COLUMNS),
    m_origin(DEFAULT_ORIGIN)
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "MorphEffect( %4d )", m_ID);

    ATOMIC_INCREMENT( MorphEffect::s_NumInstances );


    // This effect needs render-to-texture.  The result is then presented on-screen in ::EndFrame()
    m_needsRenderTarget = true;
    m_isPostEffect      = true;
    

    // TEST TEST: default curve control points.
    float height = DEFAULT_HEIGHT;
    if (!Platform::IsWidescreen()) {
        height = DEFAULT_HEIGHT_3_5;
    }
    
    m_curve0[0] =   vec3( 0.0f,                           height,           0.0f );     // Top
    m_curve0[1] =   vec3( (DEFAULT_WIDTH/2.0f) + 40.0f,   0.0f,             0.0f );     // Control 2
    m_curve0[2] =   vec3( (DEFAULT_WIDTH/2.0f) - 40.0f,   0.0f,             0.0f );     // Control 1
    m_curve0[3] =   vec3( (DEFAULT_WIDTH/2.0f) - 40.0f,   0.0f,             0.0f );     // Bottom
    
    m_curve1[0] =   vec3( DEFAULT_WIDTH,                   height,          0.0f );     // Top
    m_curve1[1] =   vec3( (DEFAULT_WIDTH/2.0f) + 20.0f,    0.0f,            0.0f );     // Control 2
    m_curve1[2] =   vec3( (DEFAULT_WIDTH/2.0f) - 20.0f,    0.0f,            0.0f );     // Control 1
    m_curve1[3] =   vec3( (DEFAULT_WIDTH/2.0f) - 40.0f,    0.0f,            0.0f );     // Bottom


    m_name = "MorphEffect";
    memset(&m_frame, 0, sizeof(m_frame));
}


MorphEffect::~MorphEffect()
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "\t~MorphEffect( %4d )", m_ID);

    SAFE_ARRAY_DELETE(m_pMorphedVertices);

    // Delete the shaders when the last instance is freed
    if (0 == ATOMIC_DECREMENT( MorphEffect::s_NumInstances ))
    {
        RETAILMSG(ZONE_INFO, "~MorphEffect: last instance deleted, freeing Morph shaders");
        Deinit();
    }
}



MorphEffect*
MorphEffect::Clone() const
{
    return new MorphEffect(*this);
}


MorphEffect::MorphEffect( const MorphEffect& rhs ) : 
    OpenGLESEffect(),
    m_pMorphedVertices(NULL),
    m_numMorphedVertices(0),
    m_rows(DEFAULT_ROWS),
    m_columns(DEFAULT_COLUMNS),
    m_origin(DEFAULT_ORIGIN)
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "MorphEffect( %4d ) (copy ctor)", m_ID);

    ATOMIC_INCREMENT( MorphEffect::s_NumInstances );

    m_name = "MorphEffect";
    memset(&m_frame, 0, sizeof(m_frame));

    *this = rhs;
}


MorphEffect& 
MorphEffect::operator=( const MorphEffect& rhs )
{
    // Avoid self-assignment.
    if (this == &rhs)
        return *this;
    

    // Copy base class (is there a cleaner syntax?)
    OpenGLESEffect::operator=(rhs);


    // SHALLOW COPY:
    m_numMorphedVertices            = rhs.m_numMorphedVertices;
    m_rows                          = rhs.m_rows;
    m_columns                       = rhs.m_columns;
    m_origin                        = rhs.m_origin;


    // DEEP COPY: bezier curve points
    memcpy(m_curve0, rhs.m_curve0, sizeof(m_curve0));
    memcpy(m_curve1, rhs.m_curve1, sizeof(m_curve1));


    // DEEP COPY: vertices
    if (rhs.m_pMorphedVertices)
    {
        SAFE_ARRAY_DELETE(m_pMorphedVertices);
        m_pMorphedVertices = new Vertex[ rhs.m_numMorphedVertices ];
        memcpy(m_pMorphedVertices, rhs.m_pMorphedVertices, sizeof(Vertex)*rhs.m_numMorphedVertices);
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
MorphEffect::DrawTriangleStrip( Vertex *pVertices, UINT32 numVertices )
{
    return MorphEffect::Draw( pVertices, numVertices, PRIMITIVE_TYPE_TRIANGLE_STRIP );
}


RESULT 
MorphEffect::DrawTriangleList( Vertex *pVertices, UINT32 numVertices )
{
    return MorphEffect::Draw( pVertices, numVertices, PRIMITIVE_TYPE_TRIANGLE_LIST );
}



RESULT 
MorphEffect::Draw( Vertex *pVertices, UINT32 numVertices, PRIMITIVE_TYPE primitiveType  )
{
    DEBUGMSG(ZONE_SHADER, "MorphEffect[%d]::DrawTriangleStrip()", m_ID);

    RESULT     rval          = S_OK;
    Rectangle  sourceRect;
    Rectangle  textureRect;
    Util::GetBoundingRect  ( pVertices, numVertices, &sourceRect  );
    Util::GetTextureMapping( pVertices, numVertices, &textureRect );

    // Reinit if our source frame changes, i.e. the object
    // we're rippling is being scaled.
    if (!m_pMorphedVertices || m_frameChanged)
    {
        CHR(InitScratchSurfaces( sourceRect, textureRect ));
        m_frameChanged = false;
    }


    // Assign values to shader parameters.
    // TODO: move to BeginPass()?
    VERIFYGL(glUseProgram(s_MorphShaderProgram));
    VERIFYGL(glUniformMatrix4fv(s_uProjectionMatrix, 1, GL_FALSE, (GLfloat*)m_projectionMatrix.Pointer()));
    VERIFYGL(glUniformMatrix4fv(s_uModelViewMatrix,  1, GL_FALSE, (GLfloat*)m_modelViewMatrix.Pointer()));
    VERIFYGL(glUniform1i(s_uTexture, 0));
    VERIFYGL(glUniform4f(s_uGlobalColor, m_globalColor.floats.r, m_globalColor.floats.g, m_globalColor.floats.b, m_globalColor.floats.a));
    VERIFYGL(glUniform3fv(s_uOrigin, 1, (const float*)m_origin));
    
//    VERIFYGL(glUniform1i(s_uCheckerboard,   (GLint)m_bUseCheckerboard));

    VERIFYGL(glUniform1f(s_uSourceWidth,    sourceRect.width));
    VERIFYGL(glUniform1f(s_uSourceHeight,   sourceRect.height));

    
    VERIFYGL(glUniform3fv(s_curve0point0Loc, 1, (const float*)m_curve0[0]));
    VERIFYGL(glUniform3fv(s_curve0point1Loc, 1, (const float*)m_curve0[1]));
    VERIFYGL(glUniform3fv(s_curve0point2Loc, 1, (const float*)m_curve0[2]));
    VERIFYGL(glUniform3fv(s_curve0point3Loc, 1, (const float*)m_curve0[3]));

    VERIFYGL(glUniform3fv(s_curve1point0Loc, 1, (const float*)m_curve1[0]));
    VERIFYGL(glUniform3fv(s_curve1point1Loc, 1, (const float*)m_curve1[1]));
    VERIFYGL(glUniform3fv(s_curve1point2Loc, 1, (const float*)m_curve1[2]));
    VERIFYGL(glUniform3fv(s_curve1point3Loc, 1, (const float*)m_curve1[3]));


    // Draw the texture mapped onto the morphing mesh.
    VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_POSITION,       3, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &m_pMorphedVertices->x));
    VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_DIFFUSE,        4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(Vertex), &m_pMorphedVertices->color));
    VERIFYGL(glVertexAttribPointer(ATTRIBUTE_VERTEX_TEXTURE_COORD0, 2, GL_FLOAT,         GL_FALSE, sizeof(Vertex), &m_pMorphedVertices->texCoord[0]));

    switch (primitiveType)
    {
        // NOTE: for this effect we ignore the passed-in vertices and draw a triangle list (GL_TRIANGLES) not a triangle strip (GL_TRIANGLE_STRIP).
        case PRIMITIVE_TYPE_TRIANGLE_STRIP:
        case PRIMITIVE_TYPE_TRIANGLE_LIST:
            VERIFYGL(glDrawArrays(GL_TRIANGLES, 0, m_numMorphedVertices));
            break;
        default:
            // TODO: might it ever be useful to apply Morph to a field of point sprites?
            DEBUGCHK(0);
    }

Exit:
    return rval;
}


void
MorphEffect::SetRows( UINT8 rows )
{
    m_rows = rows;
}


UINT8
MorphEffect::GetRows()
{
    return m_rows;
}




void
MorphEffect::SetColumns( UINT8 columns )
{
    m_columns = columns;
}


UINT8
MorphEffect::GetColumns()
{
    return m_columns;
}



void
MorphEffect::SetOrigin( const vec3& origin )
{
    m_origin = origin;
}


vec3
MorphEffect::GetOrigin()
{
    return m_origin;
}


#pragma mark -
#pragma mark Curve 1
void
MorphEffect::SetCurve0p0( const vec3& point )
{
    m_curve0[0] = point;
}

void
MorphEffect::SetCurve0p1( const vec3& point )
{
    m_curve0[1] = point;
}

void
MorphEffect::SetCurve0p2( const vec3& point )
{
    m_curve0[2] = point;
}

void
MorphEffect::SetCurve0p3( const vec3& point )
{
    m_curve0[3] = point;
}


vec3
MorphEffect::GetCurve0p0()
{
    return m_curve0[0];
}

vec3
MorphEffect::GetCurve0p1()
{
    return m_curve0[1];
}

vec3
MorphEffect::GetCurve0p2()
{
    return m_curve0[2];
}

vec3
MorphEffect::GetCurve0p3()
{
    return m_curve0[3];
}



#pragma mark -
#pragma mark Curve 1
void
MorphEffect::SetCurve1p0( const vec3& point )
{
    m_curve1[0] = point;
}

void
MorphEffect::SetCurve1p1( const vec3& point )
{
    m_curve1[1] = point;
}

void
MorphEffect::SetCurve1p2( const vec3& point )
{
    m_curve1[2] = point;
}

void
MorphEffect::SetCurve1p3( const vec3& point )
{
    m_curve1[3] = point;
}


vec3
MorphEffect::GetCurve1p0()
{
    return m_curve1[0];
}

vec3
MorphEffect::GetCurve1p1()
{
    return m_curve1[1];
}

vec3
MorphEffect::GetCurve1p2()
{
    return m_curve1[2];
}

vec3
MorphEffect::GetCurve1p3()
{
    return m_curve1[3];
}





RESULT 
MorphEffect::InitScratchSurfaces( Rectangle& sourceRect, Rectangle& textureRect )
{
    RESULT rval = S_OK;

    // We render an animated mesh in place of the triangle mesh sent down from the caller.
    //
    // Create the new mesh with the same position and dimensions, 
    // and a reasonable number of number of triangles (so the Morph looks fluid).
    
    float uStart    = textureRect.x;
    float uEnd      = textureRect.x + textureRect.width;
    float vStart    = textureRect.y;
    float vEnd      = textureRect.y + textureRect.height;
    
    SAFE_ARRAY_DELETE(m_pMorphedVertices);
    
    CHR(Util::CreateTriangleList( &sourceRect, m_rows, m_columns, &m_pMorphedVertices, &m_numMorphedVertices, uStart, uEnd, vStart, vEnd  ));

    // TODO: create a VBO.

Exit:
    return rval;
}



//----------------------------------------------------------------------------
// Class methods
//----------------------------------------------------------------------------

MorphEffect* 
MorphEffect::CreateInstance()
{
    DEBUGMSG(ZONE_RENDER, "MorphEffect::CreateInstance()");

    RESULT        rval      = S_OK;
    MorphEffect* pEffect   = NULL;

    if (Platform::IsOpenGLES2())
    {
        if (!s_MorphShadersLoaded)
        {
            CHR(Init());
        }

        CPREx(pEffect = new MorphEffect(), E_OUTOFMEMORY);
    }
    else 
    {
        RETAILMSG(ZONE_WARN, "Device doesn't support MorphEffect.");
    }

Exit:
    return pEffect;
}



RESULT 
MorphEffect::Init()
{
    RETAILMSG(ZONE_RENDER, "MorphEffect::Init()");

    RESULT rval = S_OK;

    if (s_MorphShadersLoaded)
    {
        return S_OK;
    }

    CHR(OpenGLESEffect::Init());

    CHR(ShaderMan.CreateShader( "MorphEffect", s_MorphVertexShaderName, s_MorphFragmentShaderName, &s_hMorphShader ));
    
    // Save uniform handles.
    s_MorphShaderProgram = ShaderMan.GetShaderProgramID( s_hMorphShader );
    VERIFYGL(glUseProgram(s_MorphShaderProgram));

    VERIFYGL(s_uModelViewMatrix     = glGetUniformLocation(s_MorphShaderProgram, "uMatModelView"));
    VERIFYGL(s_uProjectionMatrix    = glGetUniformLocation(s_MorphShaderProgram, "uMatProjection"));
    VERIFYGL(s_uTexture             = glGetUniformLocation(s_MorphShaderProgram, "uTexture"));
    VERIFYGL(s_uGlobalColor         = glGetUniformLocation(s_MorphShaderProgram, "uGlobalColor"));
    VERIFYGL(s_uOrigin              = glGetUniformLocation(s_MorphShaderProgram, "uOrigin"));
    
    VERIFYGL(s_curve0point0Loc      = glGetUniformLocation(s_MorphShaderProgram, "c0p0"));
    VERIFYGL(s_curve0point1Loc      = glGetUniformLocation(s_MorphShaderProgram, "c0p1"));
    VERIFYGL(s_curve0point2Loc      = glGetUniformLocation(s_MorphShaderProgram, "c0p2"));
    VERIFYGL(s_curve0point3Loc      = glGetUniformLocation(s_MorphShaderProgram, "c0p3"));

    VERIFYGL(s_curve1point0Loc      = glGetUniformLocation(s_MorphShaderProgram, "c1p0"));
    VERIFYGL(s_curve1point1Loc      = glGetUniformLocation(s_MorphShaderProgram, "c1p1"));
    VERIFYGL(s_curve1point2Loc      = glGetUniformLocation(s_MorphShaderProgram, "c1p2"));
    VERIFYGL(s_curve1point3Loc      = glGetUniformLocation(s_MorphShaderProgram, "c1p3"));

    VERIFYGL(s_uSourceWidth         = glGetUniformLocation(s_MorphShaderProgram, "uSourceWidth"));
    VERIFYGL(s_uSourceHeight        = glGetUniformLocation(s_MorphShaderProgram, "uSourceHeight"));
    
    VERIFYGL(s_uCheckerboard        = glGetUniformLocation(s_MorphShaderProgram, "uCheckerboard"));
        
    s_MorphShadersLoaded = true;

Exit:
    return rval;
}



RESULT 
MorphEffect::Deinit()
{
    RETAILMSG(ZONE_RENDER, "MorphEffect::Deinit()");

    RESULT rval = S_OK;

    CHR(ShaderMan.Release( s_hMorphShader ));
    s_MorphShaderProgram  = 0;
    s_MorphShadersLoaded  = false;

Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: MorphEffect::Deinit(): failed to free shaders");
    }

    return rval;
}



HShader
MorphEffect::GetShader()
{
    return s_hMorphShader;
}



IProperty*
MorphEffect::GetProperty( const string& propertyName ) const
{
    return s_properties.Get( this, propertyName );
}


} // END namespace Z

