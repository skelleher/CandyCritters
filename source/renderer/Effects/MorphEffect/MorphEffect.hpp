#pragma once


#include "OpenGLESEffect.hpp"


namespace Z
{



class MorphEffect : public OpenGLESEffect
{
//----------------------------------------------------------------------------
// Object methods
//----------------------------------------------------------------------------
public:
    virtual MorphEffect* Clone() const;
    virtual ~MorphEffect();

    virtual RESULT Draw                     ( IN Vertex *pVertices, UINT32 numVertices, PRIMITIVE_TYPE type );
    virtual RESULT DrawTriangleStrip        ( IN Vertex *pVertices, UINT32 numVertices                      );
    virtual RESULT DrawTriangleList         ( IN Vertex *pVertices, UINT32 numVertices                      );

    // Custom Effect Properties
    virtual void    SetRows                 ( UINT8  rows        );
    virtual void    SetColumns              ( UINT8  columns     );
    virtual void    SetOrigin               ( const vec3& origin );

    virtual void    SetCurve0p0             ( const vec3& point  );
    virtual void    SetCurve0p1             ( const vec3& point  );
    virtual void    SetCurve0p2             ( const vec3& point  );
    virtual void    SetCurve0p3             ( const vec3& point  );

    virtual void    SetCurve1p0             ( const vec3& point  );
    virtual void    SetCurve1p1             ( const vec3& point  );
    virtual void    SetCurve1p2             ( const vec3& point  );
    virtual void    SetCurve1p3             ( const vec3& point  );


    virtual UINT8   GetRows                 ( );
    virtual UINT8   GetColumns              ( );
    virtual vec3    GetOrigin               ( );

    virtual vec3    GetCurve0p0             ( );
    virtual vec3    GetCurve0p1             ( );
    virtual vec3    GetCurve0p2             ( );
    virtual vec3    GetCurve0p3             ( );
    
    virtual vec3    GetCurve1p0             ( );
    virtual vec3    GetCurve1p1             ( );
    virtual vec3    GetCurve1p2             ( );
    virtual vec3    GetCurve1p3             ( );
    
    virtual HShader GetShader               ( );

    virtual IProperty* GetProperty          ( const string& name ) const;

protected:
    MorphEffect();
    MorphEffect( const MorphEffect& rhs );
    MorphEffect& operator=( const MorphEffect& rhs );

    virtual RESULT InitScratchSurfaces  ( Rectangle& sourceRect, Rectangle& textureRect );




//----------------------------------------------------------------------------
// Object data
//----------------------------------------------------------------------------
protected:
    Vertex*         m_pMorphedVertices;
    UINT32          m_numMorphedVertices;


    UINT8           m_rows;
    UINT8           m_columns;
    vec3            m_origin;       // TODO: automatically animate the curves to collapse onto m_origin.

    vec3            m_curve0[4];
    vec3            m_curve1[4];
    

//----------------------------------------------------------------------------
// Class methods
//----------------------------------------------------------------------------
public:
    static MorphEffect*    CreateInstance  ( );

protected:
    static RESULT           Init            ( );
    static RESULT           Deinit          ( );


//----------------
// Class data
//----------------
protected:
    static UINT32                   s_NumInstances;

    // Shaders
    static bool                     s_MorphShadersLoaded;
    static GLuint                   s_MorphShaderProgram;
    static HShader                  s_hMorphShader;

    static const char*              s_MorphVertexShaderName;
    static const char*              s_MorphFragmentShaderName;

    // Shader parameters
    static GLint                    s_uProjectionMatrix;
    static GLint                    s_uModelViewMatrix;
    static GLint                    s_uTexture;
    static GLint                    s_uGlobalColor;
    static GLint                    s_uOrigin;

    static GLint                    s_curve0point0Loc;
    static GLint                    s_curve0point1Loc;
    static GLint                    s_curve0point2Loc;
    static GLint                    s_curve0point3Loc;
    
    static GLint                    s_curve1point0Loc;
    static GLint                    s_curve1point1Loc;
    static GLint                    s_curve1point2Loc;
    static GLint                    s_curve1point3Loc;
    
    static GLint                    s_uSourceWidth;
    static GLint                    s_uSourceHeight;
    
    static GLint                    s_uCheckerboard;
    
    static const UINT8              DEFAULT_ROWS;
    static const UINT8              DEFAULT_COLUMNS;
    static const vec3               DEFAULT_ORIGIN;


    // TEST TEST
    static const float              DEFAULT_WIDTH;
    static const float              DEFAULT_HEIGHT;
    static const float              DEFAULT_HEIGHT_3_5;


    //
    // Exported Properties for animation/scripting
    //
    static       PropertySet        s_properties;
    
};



} // END namespace Z
