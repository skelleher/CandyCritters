/*
 *  MeshManager.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 10/6/10.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include "Engine.hpp"
#include "MeshManager.hpp"
#include "FileManager.hpp"
#include "Log.hpp"
#include "Handle.hpp"
#include "Settings.hpp"
#include "Platform.hpp"
#include "Types.hpp"
#include "Util.hpp"



namespace Z
{


   
MeshManager& 
MeshManager::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new MeshManager();
    }
    
    return static_cast<MeshManager&>(*s_pInstance);
}


MeshManager::MeshManager()
{
    RETAILMSG(ZONE_VERBOSE, "MeshManager()");
    
    s_pResourceManagerName = "MeshManager";
}


MeshManager::~MeshManager()
{
    RETAILMSG(ZONE_VERBOSE, "\t~MeshManager()");
}



RESULT
MeshManager::Init( IN const string& settingsFilename )
{
    RETAILMSG(ZONE_INFO, "MeshManager::Init( %s )", settingsFilename.c_str());

    RESULT rval = S_OK;
    char   path[MAX_PATH];
    
    //
    // Create a Settings object and load the file.
    //
    Settings mySettings;
    if ( FAILED(mySettings.Read( settingsFilename )) )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: MeshManager::Init( %s ): failed to load settings file", settingsFilename.c_str() );
        return E_UNEXPECTED;
    }
    

    //
    // Create each Mesh.
    //
    UINT32 numMeshs = mySettings.GetInt("/Meshes.NumMeshes");

    for (int i = 0; i < numMeshs; ++i)
    {
        sprintf(path, "/Meshes/Mesh%d", i);
        //DEBUGMSG(ZONE_INFO, "Loading [%s]", path);

        Mesh *pMesh = NULL;
        CreateMesh( &mySettings, path, &pMesh );
        if (!pMesh)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: MeshManager::Init( %s ): failed to create Mesh", path);
            // Continue loading other Meshs rather than aborting.
            continue;
        }
        
        DEBUGMSG(ZONE_MESH, "Created Mesh [%s]", pMesh->GetName().c_str());
        CHR(Add(pMesh->GetName(), pMesh));
    }
    
Exit:
    return rval;
}



RESULT
MeshManager::CreateMesh( IN Settings* pSettings, IN const string& settingsPath, INOUT Mesh** ppMesh )
{
    RESULT rval = S_OK;
    string name;
    
    if (!pSettings || "" == settingsPath || !ppMesh)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: MeshManager::CreateMesh(): invalid argument");
        return E_INVALID_ARG;
    }
    
    Mesh *pMesh = new Mesh();
    CPR(pMesh);

    name = pSettings->GetString( settingsPath + ".Name" );
    pMesh->Init( name, pSettings, settingsPath );

    // Caller must AddRef()
    *ppMesh = pMesh;
    
Exit:    
    return rval;
}



RESULT
MeshManager::DrawMesh( IN HMesh hMesh, mat4& matWorld )
{
    RESULT rval = S_OK;
    
    // TODO: batch up the draw calls and issue them sorted by atlas?
    
    Mesh* pMesh = GetObjectPointer( hMesh );
    if (pMesh)
    {
        DEBUGMSG(ZONE_MESH | ZONE_VERBOSE, "MeshManager::DrawMesh( %s )", 
                 pMesh->GetName().c_str());
        
        CHR(pMesh->Draw( matWorld ));
    }
    
Exit:
    return rval;
}


//
// IDrawable
//
RESULT
MeshManager::SetVisible( IN HMesh hMesh, bool isVisible )
{
    RESULT rval = S_OK;
    
    Mesh* pMesh = GetObjectPointer( hMesh );
    if (pMesh)
    {
        DEBUGMSG(ZONE_MESH | ZONE_VERBOSE, "MeshManager::SetVisible( %s, %d )", 
                 pMesh->GetName().c_str(), isVisible);
        
        pMesh->SetVisible( isVisible );
    }
    else 
    {
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}




RESULT
MeshManager::SetPosition( IN HMesh hMesh, const vec3& vPos )
{
    RESULT rval = S_OK;
    
    Mesh* pMesh = GetObjectPointer( hMesh );
    if (pMesh)
    {
        DEBUGMSG(ZONE_MESH | ZONE_VERBOSE, "MeshManager::SetPosition( %s, (%d, %d, %d) )", 
                 pMesh->GetName().c_str(), vPos.x, vPos.y, vPos.z);
        
        pMesh->SetPosition( vPos );
    }
    else 
    {
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}



RESULT
MeshManager::SetRotation( IN HMesh hMesh, const vec3& vRotationDegrees )
{
    RESULT rval = S_OK;
    
    Mesh* pMesh = GetObjectPointer( hMesh );
    if (pMesh)
    {
        DEBUGMSG(ZONE_MESH | ZONE_VERBOSE, "MeshManager::SetRotation( %s, (%d, %d, %d) )", 
                 pMesh->GetName().c_str(), vRotationDegrees.x, vRotationDegrees.y, vRotationDegrees.z);
        
        pMesh->SetRotation( vRotationDegrees );
    }
    else 
    {
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}



RESULT
MeshManager::SetScale( IN HMesh hMesh, float scale )
{
    RESULT rval = S_OK;
    
    Mesh* pMesh = GetObjectPointer( hMesh );
    if (pMesh)
    {
        DEBUGMSG(ZONE_MESH | ZONE_VERBOSE, "MeshManager::SetScale( %s, %4.4f )", 
                 pMesh->GetName().c_str(), scale);
        
        pMesh->SetScale( scale );
    }
    else 
    {
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}



RESULT
MeshManager::SetOpacity( IN HMesh hMesh, float opacity )
{
    RESULT rval = S_OK;
    
    Mesh* pMesh = GetObjectPointer( hMesh );
    if (pMesh)
    {
        DEBUGMSG(ZONE_MESH | ZONE_VERBOSE, "MeshManager::SetOpacity( %s, %4.4f )", 
                 pMesh->GetName().c_str(), opacity);
        
        pMesh->SetOpacity( opacity );
    }
    else 
    {
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}



RESULT
MeshManager::SetColor( IN HMesh hMesh, const Color& color )
{
    RESULT rval = S_OK;
    
    Mesh* pMesh = GetObjectPointer( hMesh );
    if (pMesh)
    {
        pMesh->SetColor( color );
    }
    else 
    {
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}



RESULT
MeshManager::SetEffect( IN HMesh hMesh, HEffect hEffect )
{
    RESULT rval = S_OK;
    
    Mesh* pMesh = GetObjectPointer( hMesh );
    if (pMesh)
    {
#ifdef DEBUG
        string name;
        Effects.GetName( hEffect, &name );
        DEBUGMSG(ZONE_MESH | ZONE_VERBOSE, "MeshManager::SetEffect( %s, %s )", 
                 pMesh->GetName().c_str(), name.c_str());
#endif        
        pMesh->SetEffect( hEffect );
    }
    else 
    {
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}




RESULT
MeshManager::SetShadow( IN HMesh hMesh, bool hasShadow )
{
    RESULT rval = S_OK;
    
    Mesh* pMesh = GetObjectPointer( hMesh );
    if (pMesh)
    {
        DEBUGMSG(ZONE_MESH | ZONE_VERBOSE, "MeshManager::SetShadow( %s, %d )", 
                 pMesh->GetName().c_str(), hasShadow);
        
        pMesh->SetShadow( hasShadow );
    }
    else 
    {
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}





bool
MeshManager::GetVisible( IN HMesh hMesh )
{
    bool rval = false;

    Mesh* pMesh = GetObjectPointer( hMesh );
    if (pMesh)
    {
        DEBUGMSG(ZONE_MESH | ZONE_VERBOSE, "MeshManager::GetVisible( %s )", 
                 pMesh->GetName().c_str());
        
        rval = pMesh->GetVisible();
    }

Exit:
    return rval;
}



vec3
MeshManager::GetPosition( IN HMesh hMesh )
{
    Mesh* pMesh = GetObjectPointer( hMesh );
    if (pMesh)
    {
        DEBUGMSG(ZONE_MESH | ZONE_VERBOSE, "MeshManager::GetPosition( %s )", 
                 pMesh->GetName().c_str());
        
        return pMesh->GetPosition();
    }
    else 
    {
        return vec3(0,0,0);
    }
}



vec3
MeshManager::GetRotation( IN HMesh hMesh )
{
    Mesh* pMesh = GetObjectPointer( hMesh );
    if (pMesh)
    {
        DEBUGMSG(ZONE_MESH | ZONE_VERBOSE, "MeshManager::GetRotation( %s )", 
                 pMesh->GetName().c_str());
        
        return pMesh->GetRotation();
    }
    else 
    {
        return vec3(0,0,0);
    }
}



float
MeshManager::GetScale( IN HMesh hMesh )
{
    float rval = 0.0;

    Mesh* pMesh = GetObjectPointer( hMesh );
    if (pMesh)
    {
        DEBUGMSG(ZONE_MESH | ZONE_VERBOSE, "MeshManager::GetScale( %s )", 
                 pMesh->GetName().c_str());
        
        rval = pMesh->GetScale();
    }

Exit:
    return rval;
}



float
MeshManager::GetOpacity( IN HMesh hMesh )
{
    float rval = 0.0;

    Mesh* pMesh = GetObjectPointer( hMesh );
    if (pMesh)
    {
        DEBUGMSG(ZONE_MESH | ZONE_VERBOSE, "MeshManager::GetOpacity( %s )", 
                 pMesh->GetName().c_str());
        
        rval = pMesh->GetOpacity();
    }

Exit:
    return rval;
}

    

HEffect
MeshManager::GetEffect( IN HMesh hMesh )
{

    Mesh* pMesh = GetObjectPointer( hMesh );
    if (pMesh)
    {
        DEBUGMSG(ZONE_SPRITE | ZONE_VERBOSE, "MeshManager::GetEffect( %s )", 
                 pMesh->GetName().c_str());
        
        return pMesh->GetEffect();
    }
    else 
    {
        return HEffect::NullHandle();
    }
}



AABB
MeshManager::GetBounds( IN HMesh hMesh )
{

    Mesh* pMesh = GetObjectPointer( hMesh );
    if (pMesh)
    {
        DEBUGMSG(ZONE_MESH | ZONE_VERBOSE, "MeshManager::GetBounds( %s )", 
                 pMesh->GetName().c_str());
        
        return pMesh->GetBounds();
    }
    else 
    {
        return AABB();  // 0,0,0
    }
}



bool
MeshManager::GetShadow( IN HMesh hMesh )
{
    bool rval = false;

    Mesh* pMesh = GetObjectPointer( hMesh );
    if (pMesh)
    {
        DEBUGMSG(ZONE_MESH | ZONE_VERBOSE, "MeshManager::GetShadow( %s )", 
                 pMesh->GetName().c_str());
        
        rval = pMesh->GetShadow();
    }

Exit:
    return rval;
}





// ============================================================================
//
//  Private classes; other systems only see Handles to these
//
// ============================================================================




// ============================================================================
//
//  Mesh Implementation
//
// ============================================================================


#pragma mark Mesh Implementation

Mesh::Mesh() :
//    m_hTexture(),   // Handles are null by default
    m_pVertices(NULL),
    m_vWorldPosition(0.0f, 0.0f, 0.0f),
    m_vRotation(0.0f, 0.0f, 0.0f),
    m_fScale(1.0f),
    m_fOpacity(1.0f),
    m_isVisible(true),
    m_color(Color::White()),
    m_hasShadow(false)
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "Mesh( %4d )", m_ID);
    
    m_matWorld = mat4::Identity();
    
    m_bounds.SetMin( vec3(0,0,0) );
    m_bounds.SetMax( vec3(0,0,0) );
}



Mesh::~Mesh()
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "\t~Mesh( %4d )", m_ID);
    
    TextureMan.Release( m_hTexture );
    
    // TODO: free m_pVertices;
}



RESULT
Mesh::Init( const string& name, const Settings* pSettings, const string& settingsPath )
{
    RESULT      rval        = S_OK;
    string      textureName;
    //Rectangle   rect;
    
    
    RETAILMSG(ZONE_OBJECT, "Mesh[%4d]::Init( %s )", m_ID, name.c_str());
    
    if (!pSettings)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Mesh::Init(): NULL settings");
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    
    // Set the members
    m_name      = name;
    textureName = pSettings->GetString( settingsPath + ".Texture"    );
    CHR(TextureMan.Get( textureName, &m_hTexture ));

    // TODO: load the vertices into a VBO
    
    RETAILMSG(ZONE_MESH, "Mesh[%4d]: \"%-32s\"", m_ID, m_name.c_str());
    
Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Mesh::Init( \"%s\" ): failed", name.c_str());
    }
    return rval;
}


//
// IDrawable
//

RESULT
Mesh::Draw( const mat4& matParentWorld )
{
    RESULT  rval = S_OK;
    mat4    modelview;

    //
    // Set the texture
    //
    CHR(Renderer.SetTexture( 0, m_hTexture ));

//    CHR(Renderer.SetModelViewMatrix( modelview ));
//    CHR(Renderer.DrawTriangleStrip( m_vertices, 4 ));
    
Exit:    
    return rval;
}



//
// IDrawable
//
RESULT  
Mesh::SetOpacity( float opacity )
{
    opacity = CLAMP(opacity, 0.0, 1.0);
    m_fOpacity = opacity;

    return S_OK;
}



RESULT
Mesh::SetEffect( HEffect hEffect )
{
    RESULT rval = S_OK;

    if (m_hEffect != hEffect)
    {
        IGNOREHR(EffectMan.Release(m_hEffect));
        m_hEffect = hEffect;
    }
    
Exit:
    return rval;
}





void
Mesh::UpdateBoundingBox()
{
/*
    mat4    modelview;

    //
    // Transform the mesh based on scale, rotation, and translation.
    //
    modelview  = mat4::Scale    ( m_fScale );
    modelview *= mat4::RotateX  ( m_vRotation.x );
    modelview *= mat4::RotateY  ( m_vRotation.y );
    modelview *= mat4::RotateZ  ( m_vRotation.z );
    modelview *= mat4::Translate( m_vWorldPosition.x, m_vWorldPosition.y, m_vWorldPosition.z );

    // 
    // Apply to each vertex.
    // Save the the min/max vertices for our AABB.
    //
    for (int i = 0; i < m_numVertices; ++i)
    {
        vec4 vertex = modelview * pVertices[i];
        
        // save min/max 
        
    }
    
    // Update bounding box
    m_bounds.SetMin( vec3( min.x, min.y, min.z ) );
    m_bounds.SetMax( vec3( max.x, max.y, max.z ) );
*/

    DEBUGCHK(0);
}



} // END namespace Z




