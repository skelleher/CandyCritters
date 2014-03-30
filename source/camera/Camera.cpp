#include "Engine.hpp"
#include "Camera.hpp"
#include "Log.hpp"
#include "Settings.hpp"


namespace Z
{


//----------------------------------------------------------------------------
// We expose the following named properties for animation and scripting.
//----------------------------------------------------------------------------
static const NamedProperty s_propertyTable[] =
{
    DECLARE_PROPERTY( Camera, PROPERTY_VEC3,  Position  ),
    DECLARE_PROPERTY( Camera, PROPERTY_VEC3,  LookAt    ),
    DECLARE_PROPERTY( Camera, PROPERTY_VEC3,  Up        ),
    NULL,
};
DECLARE_PROPERTY_SET( Camera, s_propertyTable );




Camera::Camera() :
    m_mode(CAMERA_MODE_PERSPECTIVE),
    m_fFov(50.0f),
    m_fAspectRatio(1.0f),
    m_fNearPlane(1.0f),
    m_fFarPlane(5000.0f)
{
    m_mWorld        = mat4::Identity();
    m_mView         = mat4::Identity();
    m_mProjection   = mat4::Identity();

    m_vPosition     = vec3(0,0,0);
    m_vUp           = vec3(0,1,0);
    m_vRight        = vec3(1,0,0);
    m_vLookAt       = vec3(0,0,-1);

    UpdateViewMatrix();
    UpdateWorldMatrix();
    UpdateProjectionMatrix();
    
    m_name = "Camera";
}


Camera::~Camera()
{
}



void
Camera::SetMode( CameraMode mode )
{
    m_mode = mode;
}



void
Camera::SetProjection( float fov, float aspectRatio, float nearPlane, float farPlane )
{
    m_fFov          = fov;
    m_fAspectRatio  = aspectRatio;
    m_fNearPlane    = nearPlane;
    m_fFarPlane     = farPlane;
   
    UpdateProjectionMatrix();
}



void
Camera::SetPosition( const vec3& position )
{
    m_vPosition = position;
    UpdateViewMatrix();
    UpdateWorldMatrix();
}



void    
Camera::SetLookAt( const vec3& lookAt   )
{
    // Prevent setting a senseless LookAt point.
    if (m_vPosition == lookAt)
    {
        return;
    }

    m_vLookAt = lookAt;
    
    UpdateViewMatrix();
    UpdateWorldMatrix();
}



void
Camera::SetUp( const vec3& up )
{
    m_vUp = up;
    m_vUp.Normalize();

    UpdateViewMatrix();
    UpdateWorldMatrix();
}




void
Camera::UpdateWorldMatrix()
{
    // TODO: inverse of the view matrix
/*
To do so, just invert the camera's position (by negating all 3 coordinates) and its orientation (by negating the roll, pitch, and yaw angles, and adjusting them to be in their proper ranges), and then compute the matrix using the same algorithm.
*/
/**
    vec3 zaxis = vec3(-m_vPosition.x, -m_vPosition.y, -m_vPosition.z) - m_vLookAt;    zaxis.Normalize();
    vec3 xaxis = m_vUp.Cross( zaxis );    m_vRight.Normalize();
    vec3 yaxis = zaxis.Cross( m_vRight );    m_vUp.Normalize();
    
    float x = -m_vRight.Dot( m_vPosition );
    float y = -m_vUp.Dot   ( m_vPosition );
    float z = -zaxis.Dot   ( m_vPosition );
    
    m_mView.x   = vec4( m_vRight.x, m_vUp.x, zaxis.x, 0.0f );
    m_mView.y   = vec4( m_vRight.y, m_vUp.y, zaxis.y, 0.0f );
    m_mView.z   = vec4( m_vRight.z, m_vUp.z, zaxis.z, 0.0f );
    m_mView.w   = vec4( x,          y,       z,       1.0f );
**/
}


void
Camera::UpdateViewMatrix()
{
/*
 // Left-Hand coordinate system (Direct3D)
 zaxis = normal(At - Eye)
 xaxis = normal(cross(Up, zaxis))
 yaxis = cross(zaxis, xaxis)
 
 xaxis.x           yaxis.x           zaxis.x          0
 xaxis.y           yaxis.y           zaxis.y          0
 xaxis.z           yaxis.z           zaxis.z          0
 -dot(xaxis, eye)  -dot(yaxis, eye)  -dot(zaxis, eye)  1
 
*/
    vec3 zaxis = -m_vLookAt;                zaxis.Normalize();
    vec3 xaxis = m_vUp.Cross( zaxis );      xaxis.Normalize();
    vec3 yaxis = zaxis.Cross( xaxis );      yaxis.Normalize();
    
    
    m_vRight = xaxis;
    m_vUp    = yaxis;
    m_vAhead = zaxis;
    
    float x = -xaxis.Dot( m_vPosition );
    float y = -yaxis.Dot( m_vPosition );
    float z = -zaxis.Dot( m_vPosition );
    
    m_mView.x   = vec4( xaxis.x, yaxis.x, zaxis.x, 0.0f );
    m_mView.y   = vec4( xaxis.y, yaxis.y, zaxis.y, 0.0f );
    m_mView.z   = vec4( xaxis.z, yaxis.z, zaxis.z, 0.0f );
    m_mView.w   = vec4( x,       y,       z,       1.0f );
}


void
Camera::UpdateProjectionMatrix()
{
    float width      = (float)Renderer.GetWidth();
    float height     = (float)Renderer.GetHeight();
    float halfWidth  = width/2.0;
    float halfHeight = height/2.0;
                
    // Do we need to scale the scene? (e.g. for a low-res device like iPhone 3)
    mat4 scale;
    float scaleFactor = GlobalSettings.GetFloat("/Settings.fWorldScaleFactor", 0.0f);
    if (scaleFactor && m_mode != CAMERA_MODE_ORTHOGRAPHIC)
    {
        RETAILMSG(ZONE_WARN, "/Settings.fWorldScaleFactor = %2.2f, scaling scene.", scaleFactor);
        scale    = mat4::Scale( scaleFactor );
    }


    switch (m_mode)
    {
        case CAMERA_MODE_PERSPECTIVE:
            m_mProjection   = scale * mat4::Translate( -halfWidth, -halfHeight, -halfHeight ) * mat4::Frustum( -halfWidth/halfHeight, halfWidth/halfHeight, -halfHeight/halfHeight, halfHeight/halfHeight, m_fNearPlane, m_fFarPlane );
            break;
        case CAMERA_MODE_ORTHOGRAPHIC:
            m_mProjection   = scale * mat4::Translate( -halfWidth, -halfHeight, -halfHeight ) * mat4::Ortho( -halfWidth, halfWidth, -halfHeight, halfHeight,  m_fNearPlane, m_fFarPlane );
            break;
    }
}

    
IProperty*
Camera::GetProperty( const string& propertyName ) const
{
    return s_properties.Get( this, propertyName );
}


} // END namespace Z
