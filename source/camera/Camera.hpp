#pragma once

#include "Matrix.hpp"
#include "Property.hpp"

    
namespace Z
{


    
typedef enum
{
    CAMERA_MODE_PERSPECTIVE,
    CAMERA_MODE_ORTHOGRAPHIC
} CameraMode;


class Camera : virtual public Object
{
public:
    Camera();
    ~Camera();

    virtual void    SetMode             ( CameraMode mode );
    virtual void    SetProjection       ( float fov, float aspectRatio, float nearPlane, float farPlane );
    virtual void    SetPosition         ( const vec3& position );
    virtual void    SetLookAt           ( const vec3& lookAt   );
    virtual void    SetUp               ( const vec3& up       );
    
    const vec3      GetPosition         ( )     const { return m_vPosition;        }
    const vec3      GetLookAt           ( )     const { return m_vLookAt;          }
    const vec3      GetUp               ( )     const { return m_vUp;              }
    const mat4&     GetViewMatrix       ( )     const { return m_mView;            }
    const mat4&     GetProjectionMatrix ( )     const { return m_mProjection;      } 

    const vec3&     GetRightVector      ( )     const { return (vec3&)m_mView.x;  } 
    const vec3&     GetUpVector         ( )     const { return (vec3&)m_mView.y;  }
    const vec3&     GetAheadVector      ( )     const { return (vec3&)m_mView.z;  }
    const vec3&     GetEyePt            ( )     const { return (vec3&)m_mView.w;  }


    virtual IProperty*  GetProperty ( const string& name ) const;


protected:
    void            UpdateViewMatrix      ( );
    void            UpdateWorldMatrix     ( );
    void            UpdateProjectionMatrix( );
    
protected:
    CameraMode  m_mode;
    
    float       m_fFov;
    float       m_fAspectRatio;
    float       m_fNearPlane;
    float       m_fFarPlane;
    
    vec3        m_vPosition;
    vec3        m_vLookAt;
    vec3        m_vUp;
    vec3        m_vRight;
    vec3        m_vAhead;
    
    mat4        m_mProjection;
    mat4        m_mView;
    mat4        m_mWorld;           // World matrix (inverse of the view matrix)
    
    

    //
    // Exported Properties for animation/scripting
    //
    static       PropertySet        s_properties;
};

    

} // END namespace Z
