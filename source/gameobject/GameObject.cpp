#include "GameObject.hpp"
#include "Util.hpp"
#include "StoryboardManager.hpp"
#include "StateMachine.hpp"
#include "Engine.hpp"

namespace Z 
{


    
// ============================================================================
//
//  GameObject Implementation
//
// ============================================================================



//----------------------------------------------------------------------------
// We expose the following named properties for animation and scripting.
//----------------------------------------------------------------------------
static const NamedProperty s_propertyTable[] =
{
    DECLARE_PROPERTY( GameObject, PROPERTY_VEC3,  Position  ),
    DECLARE_PROPERTY( GameObject, PROPERTY_VEC3,  Rotation  ),
    DECLARE_PROPERTY( GameObject, PROPERTY_FLOAT, Scale     ),
    DECLARE_PROPERTY( GameObject, PROPERTY_FLOAT, Opacity   ),
    DECLARE_PROPERTY( GameObject, PROPERTY_COLOR, Color     ),
    DECLARE_PROPERTY( GameObject, PROPERTY_UINT32, SpriteFrame ),
//    DECLARE_PROPERTY( GameObject, PROPERTY_BOOL,  Visible   ),    // asserts that SetVisible() is NULL ??
    NULL,
};
DECLARE_PROPERTY_SET( GameObject, s_propertyTable );



#pragma mark GameObject Implementation

GameObject::GameObject( GO_TYPE type ) :
    m_type(type),
    m_isMarkedForDeletion(false),
    m_vWorldPosition(0,0,0),
    m_vRotation(0,0,0),
    m_fScale(1.0f),
    m_fOpacity(1.0f),
    m_color(Color::White()),
    m_isVisible(true),
    m_hasShadow(false),
    m_spriteFrame(0),
    m_pStateMachineManager(NULL)
{
    DEBUGMSG(ZONE_GAMEOBJECT | ZONE_VERBOSE, "\tnew GameObject( %4d )", m_ID);
    
    m_matWorld = mat4::Identity();
    
    m_bounds.SetMin( vec3( 0.0f, 0.0f, 0.0f ) );
    m_bounds.SetMax( vec3( 0.0f, 0.0f, 0.0f ) );
}




GameObject::~GameObject()
{
    RETAILMSG(ZONE_OBJECT, "\t~GameObject( %4d, \"%s\" )", m_ID, m_name.c_str());

    //
    // Free all components
    //
    SpriteMan.Release ( m_hSprite           );
    MeshMan.Release   ( m_hMesh             );
    EffectMan.Release ( m_hEffect           );
    //Particles.Release ( m_hParticleEmitter  );
    
    for (HGameObjectListIterator pHGameObject = m_hGameObjectChildren.begin(); pHGameObject != m_hGameObjectChildren.end(); ++pHGameObject)
    {
        GOMan.Release( *pHGameObject );
    }

    for (HSpriteListIterator pHSprite = m_hSpriteChildren.begin(); pHSprite != m_hSpriteChildren.end(); ++pHSprite)
    {
        SpriteMan.Release( *pHSprite );
    }

    for (HParticleEmitterListIterator pHParticleEmitter = m_hParticleEmitterChildren.begin(); pHParticleEmitter != m_hParticleEmitterChildren.end(); ++pHParticleEmitter)
    {
        Particles.Stop( *pHParticleEmitter );
        Particles.Release( *pHParticleEmitter );
    }


    SAFE_DELETE(m_pStateMachineManager);
}



RESULT
GameObject::Init( const string& name, const Settings* pSettings, const string& settingsPath )
{
    RESULT      rval        = S_OK;
    
    
    RETAILMSG(ZONE_GAMEOBJECT, "GameObject[%4d]::Init( \"%s\" )", m_ID, name.c_str());
    
    m_name = name;
    
    if (!pSettings)
    {
        //RETAILMSG(ZONE_ERROR, "ERROR: GameObject::Init(): NULL settings");
        //rval = E_INVALID_ARG;
        goto Exit;
    }
    
    
    // Create the GO components
    m_name      = name;


    if (!m_hSprite.IsNull())
    {
        m_bounds = SpriteMan.GetBounds( m_hSprite );
    }

    
Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObject[%4d]::Init( \"%s\" ): failed", m_ID, name.c_str());
    }
    return rval;
}



RESULT
GameObject::SetEffect( HEffect hEffect )
{
    RESULT rval = S_OK;

    if (m_hEffect != hEffect)
    {
        IGNOREHR(EffectMan.Release(m_hEffect));
        m_hEffect = hEffect;
        
        SpriteMan.SetEffect( m_hSprite, m_hEffect );
        MeshMan.SetEffect  ( m_hMesh,   m_hEffect );
    }
    
    for (HSpriteListIterator pHSprite = m_hSpriteChildren.begin(); pHSprite != m_hSpriteChildren.end(); ++pHSprite)
    {
        SpriteMan.SetEffect( *pHSprite, m_hEffect );
    }

    for (HGameObjectListIterator pHGameObject = m_hGameObjectChildren.begin(); pHGameObject != m_hGameObjectChildren.end(); ++pHGameObject)
    {
        GOMan.SetEffect( *pHGameObject, m_hEffect );
    }

Exit:
    return rval;
}



RESULT
GameObject::SetSprite( HSprite hSprite )
{
    RESULT rval = S_OK;

    if (m_hSprite != hSprite)
    {
        IGNOREHR(SpriteMan.Release(m_hSprite));
        m_hSprite = hSprite;
        CHR(SpriteMan.AddRef( m_hSprite ));
        
        m_bounds = SpriteMan.GetBounds( m_hSprite );
    }

Exit:
    return rval;
}



RESULT
GameObject::SetSpriteFrame( UINT8 frame )
{
    RESULT rval = S_OK;
    
    m_spriteFrame = frame;
    
    CHR(SpriteMan.SetSpriteFrame( m_hSprite, frame ));
    
Exit:
    return rval;
}


RESULT
GameObject::SetMesh( HMesh hMesh )
{
    RESULT rval = S_OK;
    
    if (m_hMesh != hMesh)
    {
        IGNOREHR(MeshMan.Release( m_hMesh ));
        m_hMesh = hMesh;
        CHR(MeshMan.AddRef( m_hMesh ));
    }
        
Exit:
    return rval;
}



RESULT
GameObject::AddChild( HSprite hSprite )
{
    RESULT rval = S_OK;

    HSpriteListIterator pHSprite;
    pHSprite = find( m_hSpriteChildren.begin(), m_hSpriteChildren.end(), hSprite );
    
    if (pHSprite == m_hSpriteChildren.end())
    {
        m_hSpriteChildren.push_back( hSprite );
        
        CHR(SpriteMan.AddRef( hSprite ));
        
        // TODO: update bounding box to include children?
    }
    
Exit:
    return rval;
}



RESULT
GameObject::AddChild( HGameObject hGameObject )
{
    RESULT rval = S_OK;

    HGameObjectListIterator pHGameObject;
    pHGameObject = find( m_hGameObjectChildren.begin(), m_hGameObjectChildren.end(), hGameObject );
    

    if (pHGameObject == m_hGameObjectChildren.end())
    {
        m_hGameObjectChildren.push_back( hGameObject );
        
        CHR(GOMan.AddRef( hGameObject ));
        
        // TODO: update bounding box to include children?
    }
    
Exit:
    return rval;
}



RESULT
GameObject::AddChild( HParticleEmitter hParticleEmitter )
{
    RESULT rval = S_OK;

    HParticleEmitterListIterator pHParticleEmitter;
    pHParticleEmitter = find( m_hParticleEmitterChildren.begin(), m_hParticleEmitterChildren.end(), hParticleEmitter );
    

    if (pHParticleEmitter == m_hParticleEmitterChildren.end())
    {
        m_hParticleEmitterChildren.push_back( hParticleEmitter );
        
        CHR(Particles.AddRef( hParticleEmitter ));
        
        // TODO: update bounding box to include children?
    }
    
Exit:
    return rval;
}



RESULT
GameObject::GetChildren( INOUT HParticleEmitterList* pList  )
{
    RESULT                       rval            = S_OK;
    HParticleEmitterList         children;
    HParticleEmitterListIterator pHParticleEmitter;
    
    if (!pList)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObject[%4d]::GetChildren( NULL )", m_ID );
        rval = E_NULL_POINTER;
        goto Exit;
    }


    for( pHParticleEmitter = m_hParticleEmitterChildren.begin(); pHParticleEmitter != m_hParticleEmitterChildren.end(); ++pHParticleEmitter )
    {
        HParticleEmitter hChild = *pHParticleEmitter;
        pList->push_back( hChild );
    }
    
Exit:
    return rval;
}



RESULT
GameObject::GetChildren( INOUT HGameObjectList* pList  )
{
    RESULT                  rval            = S_OK;
    HGameObjectList         children;
    HGameObjectListIterator pHGameObject;
    
    if (!pList)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObject[%4d]::GetChildren( NULL )", m_ID );
        rval = E_NULL_POINTER;
        goto Exit;
    }


    for( pHGameObject = m_hGameObjectChildren.begin(); pHGameObject != m_hGameObjectChildren.end(); ++pHGameObject )
    {
        HGameObject hChild = *pHGameObject;
        pList->push_back( hChild );
    }
    
Exit:
    return rval;
}



RESULT
GameObject::GetChildren( INOUT HSpriteList* pList  )
{
    RESULT                  rval            = S_OK;
    HSpriteList             children;
    HSpriteListIterator     pHSprite;
    
    if (!pList)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObject[%4d]::GetChildren( NULL )", m_ID );
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    for( pHSprite = m_hSpriteChildren.begin(); pHSprite != m_hSpriteChildren.end(); ++pHSprite )
    {
        HSprite hChild = *pHSprite;
        pList->push_back( hChild );
    }
    
Exit:
    return rval;
}



RESULT
GameObject::SetStateMachineManager( StateMachineManager* pStateMachineManager )
{
    RESULT rval = S_OK;
    
    if (!pStateMachineManager)
    {
        RETAILMSG(ZONE_ERROR, "GameObject[%4d]::SetStateMachineManager( NULL )", m_ID);
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    SAFE_DELETE(m_pStateMachineManager);
    m_pStateMachineManager = pStateMachineManager;
    
Exit:
    return rval;
}



RESULT
GameObject::CreateStateMachineManager()
{
    RESULT rval = S_OK;
    
    if (m_pStateMachineManager != NULL)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameObject[%4d]::CreateStateMachineManager(): already has a StateMachineManager", m_ID);
        rval = E_INVALID_OPERATION;
        goto Exit;
    }
    
	m_pStateMachineManager = new StateMachineManager( *this );
    if (!m_pStateMachineManager)
    {
        rval = E_OUTOFMEMORY;
        goto Exit;
    }
    
Exit:
    return rval;
}



RESULT
GameObject::Update( UINT64 elapsedMS )
{
    RESULT rval = S_OK;

    //
    // Update all our components
    //
    if(m_pStateMachineManager)
	{
		m_pStateMachineManager->Update();
	}

Exit:
    return rval;
}


#pragma mark -
#pragma mark IDrawable

RESULT
GameObject::Draw( const mat4& matParentWorld )
{
    RESULT  rval = S_OK;
    
    //
    // Skip drawing GameObjects w/ zero opacity
    //
    if (Util::CompareFloats( m_fOpacity, 0.0f ))
        return rval;

    mat4 world;
    world *= mat4::Scale    ( m_fScale      );
    world *= mat4::RotateX  ( m_vRotation.x ); 
    world *= mat4::RotateY  ( m_vRotation.y ); 
    world *= mat4::RotateZ  ( m_vRotation.z ); 
    world *= mat4::Translate( m_vWorldPosition.x, m_vWorldPosition.y, m_vWorldPosition.z );

    world *= matParentWorld;


    //
    // Draw current Sprite
    //
    if (!m_hSprite.IsNull())
    {
        CHR(SpriteMan.DrawSprite( m_hSprite, world ));
    }
    
    
    //
    // Draw child GameObjects
    //
    for (HGameObjectListIterator pHGameObject = m_hGameObjectChildren.begin(); pHGameObject != m_hGameObjectChildren.end(); ++pHGameObject)
    {
        GameObject* pGO = NULL;
        CHR(GOMan.GetGameObjectPointer( *pHGameObject, &pGO ));
        if (pGO)
        {
            CHR(pGO->Draw( world ));
        }
    }


    // 
    // Draw child Sprites
    //
    for (HSpriteListIterator pHSprite = m_hSpriteChildren.begin(); pHSprite != m_hSpriteChildren.end(); ++pHSprite)
    {
        CHR(SpriteMan.DrawSprite( *pHSprite, world ));
    }


    // 
    // Draw child ParticleEmitters
    //
    for (HParticleEmitterListIterator pHParticleEmitter = m_hParticleEmitterChildren.begin(); pHParticleEmitter != m_hParticleEmitterChildren.end(); /*++pHParticleEmitter*/)
    {
        if (FAILED(Particles.Draw( *pHParticleEmitter, world )))
        {
            // ParticleEmitter probably deleted itself when finished; remove it
            pHParticleEmitter = m_hParticleEmitterChildren.erase( pHParticleEmitter );
        }
        else
        {
            ++pHParticleEmitter;
        }
    }

    
Exit:    
    return rval;
}



RESULT
GameObject::SetVisible( bool isVisible )
{ 
    RESULT rval = S_OK;
    
    m_isVisible = isVisible;
    
Exit:
    return rval;
}



RESULT
GameObject::SetPosition( const vec3& vPos )
{ 
    RESULT rval = S_OK;

    m_vWorldPosition  = vPos;
    

    // Update bounding box
    vec3 minPoint, maxPoint;
    minPoint = maxPoint = vPos;

    maxPoint.x += m_bounds.GetWidth();
    maxPoint.y += m_bounds.GetHeight();
    maxPoint.z += m_bounds.GetDepth();
    
    m_bounds = AABB(minPoint, maxPoint);
   
Exit:    
    return rval;
}



RESULT
GameObject::SetRotation( const vec3& vRotationDegrees )
{ 
    RESULT rval = S_OK;

    m_vRotation = vRotationDegrees;

    // TODO: update bounding box

Exit:    
    return rval;
}



RESULT
GameObject::SetScale( float scale )
{ 
    RESULT rval = S_OK;

    m_fScale = scale;


    // TODO: update bounding box
    
Exit:    
    return rval;
}



RESULT
GameObject::SetOpacity( float opacity )
{ 
    RESULT rval = S_OK;

    m_fOpacity = CLAMP(opacity, 0.0, 1.0); 

    if (!m_hSprite.IsNull())
    {
        CHR(SpriteMan.SetOpacity( m_hSprite, m_fOpacity ));
    }
    
    if (!m_hMesh.IsNull())
    {
        CHR(MeshMan.SetOpacity( m_hMesh, m_fOpacity ));
    }

    for (HSpriteListIterator pHSprite = m_hSpriteChildren.begin(); pHSprite != m_hSpriteChildren.end(); ++pHSprite)
    {
        SpriteMan.SetOpacity( *pHSprite, m_fOpacity );
    }

    for (HGameObjectListIterator pHGameObject = m_hGameObjectChildren.begin(); pHGameObject != m_hGameObjectChildren.end(); ++pHGameObject)
    {
        GOMan.SetOpacity( *pHGameObject, m_fOpacity );
    }

    
Exit:    
    return rval;
}



RESULT
GameObject::SetColor( const Color& color )
{ 
    RESULT rval = S_OK;

    m_color = color;

    if (!m_hSprite.IsNull())
    {
        CHR(SpriteMan.SetColor( m_hSprite, m_color ));
    }
    
    if (!m_hMesh.IsNull())
    {
        CHR(MeshMan.SetColor( m_hMesh, m_color ));
    }

    for (HSpriteListIterator pHSprite = m_hSpriteChildren.begin(); pHSprite != m_hSpriteChildren.end(); ++pHSprite)
    {
        SpriteMan.SetColor( *pHSprite, m_color );
    }

    for (HGameObjectListIterator pHGameObject = m_hGameObjectChildren.begin(); pHGameObject != m_hGameObjectChildren.end(); ++pHGameObject)
    {
        GOMan.SetColor( *pHGameObject, m_color );
    }

    
Exit:    
    return rval;
}



RESULT
GameObject::SetShadow( bool hasShadow )
{ 
    RESULT rval = S_OK;

    m_hasShadow = hasShadow;

    if (!m_hSprite.IsNull())
    {
        CHR(SpriteMan.SetShadow( m_hSprite, m_hasShadow ));
    }
    
    if (!m_hMesh.IsNull())
    {
        CHR(MeshMan.SetShadow( m_hMesh, m_hasShadow ));
    }

    for (HSpriteListIterator pHSprite = m_hSpriteChildren.begin(); pHSprite != m_hSpriteChildren.end(); ++pHSprite)
    {
        SpriteMan.SetShadow( *pHSprite, m_hasShadow );
    }

    for (HGameObjectListIterator pHGameObject = m_hGameObjectChildren.begin(); pHGameObject != m_hGameObjectChildren.end(); ++pHGameObject)
    {
        GOMan.SetShadow( *pHGameObject, m_hasShadow );
    }

    
Exit:    
    return rval;
}



AABB
GameObject::GetBounds( )
{
    AABB bounds;

    if (!m_hMesh.IsNull())
    {
        bounds = MeshMan.GetBounds( m_hMesh );
    }
    else if (!m_hSprite.IsNull())
    {
        bounds = SpriteMan.GetBounds( m_hSprite );
    }
    else 
    {
        bounds = AABB( vec3(0,0,0), vec3(0,0,0) );
    }

    // Transform the bounding box by the GameObject's current position/scale/rotation.
    // TODO: pass world matrix to GetBounds( )?
    mat4 world;
    world *= mat4::Scale    ( m_fScale      );
    world *= mat4::RotateX  ( m_vRotation.x ); 
    world *= mat4::RotateY  ( m_vRotation.y ); 
    world *= mat4::RotateZ  ( m_vRotation.z ); 
    world *= mat4::Translate( m_vWorldPosition.x, m_vWorldPosition.y, m_vWorldPosition.z );

    bounds.Update( world );

    return bounds;
}



IProperty*
GameObject::GetProperty( const string& propertyName ) const
{
    return s_properties.Get( this, propertyName );
}




} // END namespace Z

