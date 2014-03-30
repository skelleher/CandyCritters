/*
 *  SpriteManager.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 10/3/10.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include "Engine.hpp"
#include "SpriteManager.hpp"
#include "FileManager.hpp"
#include "Log.hpp"
#include "Handle.hpp"
#include "Settings.hpp"
#include "Platform.hpp"
#include "Types.hpp"
#include "Image.hpp"
#include "Util.hpp"



namespace Z 
{



SpriteManager& 
SpriteManager::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new SpriteManager();
    }
    
    return static_cast<SpriteManager&>(*s_pInstance);
}


SpriteManager::SpriteManager() :
    m_inSpriteBatch(false)
{
    RETAILMSG(ZONE_VERBOSE, "SpriteManager()");
    
    s_pResourceManagerName = "SpriteManager";
}


SpriteManager::~SpriteManager()
{
    RETAILMSG(ZONE_VERBOSE, "\t~SpriteManager()");
    
    // TODO: release m_spriteBatch and m_vertexList;
    
}



RESULT
SpriteManager::Init( IN const string& settingsFilename )
{
    RETAILMSG(ZONE_INFO, "SpriteManager::Init( %s )", settingsFilename.c_str());

    RESULT rval = S_OK;
    char   path[MAX_PATH];
    
    //
    // Create a Settings object and load the file.
    //
    Settings mySettings;
    if ( FAILED(mySettings.Read( settingsFilename )) )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: SpriteManager::Init( %s ): failed to load settings file", settingsFilename.c_str() );
        return E_UNEXPECTED;
    }
    

    //
    // Create each Sprite.
    //
    UINT32 numSprites = mySettings.GetInt("/Sprites.NumSprites");

    for (int i = 0; i < numSprites; ++i)
    {
        sprintf(path, "/Sprites/Sprite%d", i);
        //DEBUGMSG(ZONE_INFO, "Loading [%s]", path);

        Sprite *pSprite = NULL;
        CreateSprite( &mySettings, path, &pSprite );
        if (!pSprite)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: SpriteManager::Init( %s ): failed to create Sprite", path);
            // Continue loading other Sprites rather than aborting.
            continue;
        }
        
        //DEBUGMSG(ZONE_SPRITE, "Created Sprite [%s]", pSprite->GetName().c_str());
        CHR(Add(pSprite->GetName(), pSprite));
    }
    
Exit:
    if (FAILED(rval))
    {
        DEBUGCHK(0);
    }
           
    return rval;
}



RESULT
SpriteManager::Shutdown()
{
    RESULT rval = S_OK;
    
    // Release all SpriteBatches
    SpriteBatchMapIterator ppSpriteBatch;
    for (ppSpriteBatch = m_spriteBatchMap.begin(); ppSpriteBatch != m_spriteBatchMap.end(); ++ppSpriteBatch)
    {
        SpriteBatch* pSpriteBatch = ppSpriteBatch->second;
        
        DEBUGCHK( pSpriteBatch );
        
        pSpriteBatch->numSprites    = 0;
        pSpriteBatch->numVertices   = 0;
        
        // Free the batched sprites
        BatchedSprites* pBatchedSprites = &pSpriteBatch->sprites;
        BatchedSpritesIterator ppBatchedSprite;
        for (ppBatchedSprite = pBatchedSprites->begin(); ppBatchedSprite != pBatchedSprites->end(); ++ppBatchedSprite)
        {
            BatchedSprite* pBatchedSprite = *ppBatchedSprite;
            DEBUGCHK(pBatchedSprite);
            delete pBatchedSprite;
        }
        
        pSpriteBatch->sprites.clear();
        SAFE_ARRAY_DELETE(pSpriteBatch->pVertices);
        
        //SAFE_DELETE(pSpriteBatch);
        m_spriteBatchMap.erase( ppSpriteBatch );
        
        //        RETAILMSG(ZONE_SPRITE | ZONE_VERBOSE, "SpriteManager::BeginBatch(): cleared previous SpriteBatch [%s]", pSpriteBatch->textureName.c_str());
        RETAILMSG(ZONE_SPRITE | ZONE_VERBOSE, "SpriteManager::BeginBatch(): cleared previous SpriteBatch 0x%x", pSpriteBatch);
    }
    
    DEBUGCHK(m_spriteBatchMap.size() == 0);
    
Exit:
    return rval;
}




RESULT
SpriteManager::CreateFromFile( IN    const  string&     filename, 
                               INOUT        HSprite*    pHandle,
                               IN    const  Color&      color )
{
    RESULT      rval    = S_OK;
    HSprite     hSprite;
    HTexture    hTexture;
    TextureInfo info;
    Rectangle   spriteRect;
    char        spriteName[MAX_PATH];
    
    if (filename.length() == 0 || !pHandle)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: SpriteManager::CreateFromFile(): must pass filename and pointer to HSprite.");
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    sprintf(spriteName, "SPRITE:%s", filename.c_str());


    // If a sprite has already been created from this file, return a handle to it.
    if (SUCCEEDED(Get( spriteName, &hSprite )))
    {
        RETAILMSG(ZONE_ERROR, "SpriteManager::CreateFromFile( \"%s\" ): sprite already loaded.", filename.c_str());
        if (pHandle)
        {
            *pHandle = hSprite;
        }
        
        rval = E_ALREADY_EXISTS;
        goto Exit;
    }
    
    

    // Load Texture.
    CHR(TextureMan.CreateFromFile( filename, &hTexture ));

    CHR(TextureMan.GetInfo( hTexture, &info ));
    spriteRect.x        = 0;
    spriteRect.y        = 0;
    spriteRect.width    = info.widthPixels;
    spriteRect.height   = info.heightPixels;

    // Create Sprite.
    CHR(CreateFromTexture(spriteName, hTexture, spriteRect, &hSprite, color ));

    *pHandle = hSprite;

Exit:
    return rval;
}



RESULT
SpriteManager::CreateFromTexture( IN    const  string&     name, 
                                  IN    const  string&     textureName,
                                  IN    const  Rectangle&  spriteRect,
                                  INOUT        HSprite*    pHandle,
                                  IN    const  Color&      color )
{
    RESULT rval = S_OK;
    HTexture hTexture;

    CHR(TextureMan.Get( textureName, &hTexture ));
    CHR(CreateFromTexture( name, hTexture, spriteRect, pHandle, color ))

Exit:
    hTexture.Release();
    return rval;
}



RESULT
SpriteManager::CreateFromTexture( IN    const  string&     name, 
                                  IN    const  HTexture&   hTexture,
                                  IN    const  Rectangle&  spriteRect,
                                  INOUT        HSprite*    pHandle,
                                  IN    const  Color&      color )
{
    RESULT rval = S_OK;

    HSprite     hSprite;
    Sprite*     pSprite = NULL;
    
    
    //
    // Allocate Sprite
    //
    pSprite = new Sprite;
    CPREx(pSprite, E_OUTOFMEMORY);
    
    if ("" == name) 
    {
        char randomName[MAX_NAME];
        sprintf(randomName, "Sprite_%X", (unsigned int)Platform::Random());
        
        CHR(pSprite->Init ( randomName, hTexture, spriteRect, color ));
    } 
    else
    {
        CHR(pSprite->Init ( name, hTexture, spriteRect, color ));
    }
    
    CHR(Add(pSprite->GetName(), pSprite, &hSprite));
    
    if (pHandle)
    {
        *pHandle = hSprite;
    }
    

Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: SpriteManager::Create(name: \"%s\", hTexture: 0x%x) failed: 0x%x",
                  name.c_str(), 
                  (UINT32)hTexture, 
                  rval);
        
        // TODO: free the object and handle.
        DEBUGCHK(0);
    }
    else 
    {
        DEBUGMSG(ZONE_SPRITE, "SpriteManager::Create( %4d, name: \"%-16s\" hTexture: 0x%x)",
                 pSprite->GetID(),
                 name.c_str(),
                 (UINT32)hTexture) 
    }
    
    
    return rval;
}



RESULT
SpriteManager::SetSpriteFrame( IN HSprite hSprite, UINT8 frame )
{
    RESULT rval = S_OK;
    
    Sprite* pSprite = GetObjectPointer( hSprite );
    if (pSprite)
    {
        DEBUGMSG(ZONE_SPRITE | ZONE_VERBOSE, "SpriteManager::SetSpriteFrame( %s, %d )", 
                 pSprite->GetName().c_str(), frame);
        
        pSprite->SetSpriteFrame( frame );
    }
    else 
    {
        rval = E_FAIL;
    }

    
Exit:
    return rval;
}


#pragma mark -
#pragma mark IDrawable

// IDrawable
RESULT
SpriteManager::SetVisible( IN HSprite hSprite, bool isVisible )
{
    RESULT rval = S_OK;
    
    Sprite* pSprite = GetObjectPointer( hSprite );
    if (pSprite)
    {
        DEBUGMSG(ZONE_SPRITE | ZONE_VERBOSE, "SpriteManager::SetVisible( %s, %d )", 
                 pSprite->GetName().c_str(), isVisible);
        
        pSprite->SetVisible( isVisible );
    }
    else 
    {
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}




RESULT
SpriteManager::SetPosition( IN HSprite hSprite, const vec3& vPos )
{
    RESULT rval = S_OK;
    
    Sprite* pSprite = GetObjectPointer( hSprite );
    if (pSprite)
    {
        DEBUGMSG(ZONE_SPRITE | ZONE_VERBOSE, "SpriteManager::SetPosition( %s, (%d, %d, %d) )", 
                 pSprite->GetName().c_str(), vPos.x, vPos.y, vPos.z);
        
        pSprite->SetPosition( vPos );
    }
    else 
    {
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}



RESULT
SpriteManager::SetRotation( IN HSprite hSprite, const vec3& vRotationDegrees )
{
    RESULT rval = S_OK;
    
    Sprite* pSprite = GetObjectPointer( hSprite );
    if (pSprite)
    {
        DEBUGMSG(ZONE_SPRITE | ZONE_VERBOSE, "SpriteManager::SetRotation( %s, (%d, %d, %d) )", 
                 pSprite->GetName().c_str(), vRotationDegrees.x, vRotationDegrees.y, vRotationDegrees.z);
        
        pSprite->SetRotation( vRotationDegrees );
    }
    else 
    {
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}



RESULT
SpriteManager::SetScale( IN HSprite hSprite, float scale )
{
    RESULT rval = S_OK;
    
    Sprite* pSprite = GetObjectPointer( hSprite );
    if (pSprite)
    {
        DEBUGMSG(ZONE_SPRITE | ZONE_VERBOSE, "SpriteManager::SetScale( %s, %4.4f )", 
                 pSprite->GetName().c_str(), scale);
        
        pSprite->SetScale( scale );
    }
    else 
    {
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}



RESULT
SpriteManager::SetOpacity( IN HSprite hSprite, float opacity )
{
    RESULT rval = S_OK;
    
    Sprite* pSprite = GetObjectPointer( hSprite );
    if (pSprite)
    {
        DEBUGMSG(ZONE_SPRITE | ZONE_VERBOSE, "SpriteManager::SetOpacity( %s, %4.4f )", 
                 pSprite->GetName().c_str(), opacity);
        
        pSprite->SetOpacity( opacity );
    }
    else 
    {
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}



RESULT
SpriteManager::SetEffect( IN HSprite hSprite, HEffect hEffect )
{
    RESULT rval = S_OK;
    
    Sprite* pSprite = GetObjectPointer( hSprite );
    if (pSprite)
    {
#ifdef DEBUG    
        string name;
        Effects.GetName( hEffect, &name );
        DEBUGMSG(ZONE_SPRITE | ZONE_VERBOSE, "SpriteManager::SetEffect( %s, %s )", 
                 pSprite->GetName().c_str(), name.c_str());
#endif        
        pSprite->SetEffect( hEffect );
    }
    else 
    {
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}



RESULT
SpriteManager::SetColor( IN HSprite hSprite, const Color& color )
{
    RESULT rval = S_OK;
    
    Sprite* pSprite = GetObjectPointer( hSprite );
    if (pSprite)
    {
        pSprite->SetColor( color );
    }
    else 
    {
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}



RESULT
SpriteManager::SetShadow( IN HSprite hSprite, bool hasShadow )
{
    RESULT rval = S_OK;
    
    Sprite* pSprite = GetObjectPointer( hSprite );
    if (pSprite)
    {
        DEBUGMSG(ZONE_SPRITE | ZONE_VERBOSE, "SpriteManager::SetShadow( %s, %d )", 
                 pSprite->GetName().c_str(), hasShadow);
        
        pSprite->SetShadow( hasShadow );
    }
    else 
    {
        rval = E_BAD_HANDLE;
    }

Exit:
    return rval;
}





bool
SpriteManager::GetVisible( IN HSprite hSprite )
{
    bool rval = false;

    Sprite* pSprite = GetObjectPointer( hSprite );
    if (pSprite)
    {
        DEBUGMSG(ZONE_SPRITE | ZONE_VERBOSE, "SpriteManager::GetVisible( %s )", 
                 pSprite->GetName().c_str());
        
        rval = pSprite->GetVisible();
    }

Exit:
    return rval;
}



vec3
SpriteManager::GetPosition( IN HSprite hSprite )
{
    Sprite* pSprite = GetObjectPointer( hSprite );
    if (pSprite)
    {
        DEBUGMSG(ZONE_SPRITE | ZONE_VERBOSE, "SpriteManager::GetPosition( %s )", 
                 pSprite->GetName().c_str());
        
        return pSprite->GetPosition();
    }
    else 
    {
        return vec3(0,0,0);
    }
}



vec3
SpriteManager::GetRotation( IN HSprite hSprite )
{
    Sprite* pSprite = GetObjectPointer( hSprite );
    if (pSprite)
    {
        DEBUGMSG(ZONE_SPRITE | ZONE_VERBOSE, "SpriteManager::GetRotation( %s )", 
                 pSprite->GetName().c_str());
        
        return pSprite->GetRotation();
    }
    else 
    {
        return vec3(0,0,0);
    }
}



float
SpriteManager::GetScale( IN HSprite hSprite )
{
    float rval = 0.0;

    Sprite* pSprite = GetObjectPointer( hSprite );
    if (pSprite)
    {
        DEBUGMSG(ZONE_SPRITE | ZONE_VERBOSE, "SpriteManager::GetScale( %s )", 
                 pSprite->GetName().c_str());
        
        rval = pSprite->GetScale();
    }

Exit:
    return rval;
}



float
SpriteManager::GetOpacity( IN HSprite hSprite )
{
    float rval = 0.0;

    Sprite* pSprite = GetObjectPointer( hSprite );
    if (pSprite)
    {
        DEBUGMSG(ZONE_SPRITE | ZONE_VERBOSE, "SpriteManager::GetOpacity( %s )", 
                 pSprite->GetName().c_str());
        
        rval = pSprite->GetOpacity();
    }

Exit:
    return rval;
}

    

HEffect
SpriteManager::GetEffect( IN HSprite hSprite )
{

    Sprite* pSprite = GetObjectPointer( hSprite );
    if (pSprite)
    {
        DEBUGMSG(ZONE_SPRITE | ZONE_VERBOSE, "SpriteManager::GetEffect( %s )", 
                 pSprite->GetName().c_str());
        
        return pSprite->GetEffect();
    }
    else 
    {
        return HEffect::NullHandle();
    }
}



AABB
SpriteManager::GetBounds( IN HSprite hSprite )
{
    Sprite* pSprite = GetObjectPointer( hSprite );
    if (pSprite)
    {
        DEBUGMSG(ZONE_SPRITE | ZONE_VERBOSE, "SpriteManager::GetBounds( %s )", 
                 pSprite->GetName().c_str());
        
        return pSprite->GetBounds();
    }
    else 
    {
        return AABB();  // 0,0,0
    }
}



bool
SpriteManager::GetShadow( IN HSprite hSprite )
{
    bool rval = false;

    Sprite* pSprite = GetObjectPointer( hSprite );
    if (pSprite)
    {
        DEBUGMSG(ZONE_SPRITE | ZONE_VERBOSE, "SpriteManager::GetShadow( %s )", 
                 pSprite->GetName().c_str());
        
        rval = pSprite->GetShadow();
    }

Exit:
    return rval;
}




RESULT
SpriteManager::CreateSprite( IN Settings* pSettings, IN const string& settingsPath, INOUT Sprite** ppSprite )
{
    RESULT rval = S_OK;
    string name;
    
    if (!pSettings || "" == settingsPath || !ppSprite)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: SpriteManager::CreateSprite(): invalid argument");
        return E_INVALID_ARG;
    }

    *ppSprite = NULL;
    
    Sprite *pSprite = new Sprite();
    CPR(pSprite);

    name = pSettings->GetString( settingsPath + ".Name" );
    CHR(pSprite->Init( name, pSettings, settingsPath ));

    // Caller must AddRef()
    *ppSprite = pSprite;
    
Exit:
    if (FAILED(rval))
    {
        SAFE_DELETE(pSprite);
    }
    
    return rval;
}



#pragma mark -
#pragma mark Draw

RESULT
SpriteManager::BeginBatch()
{
    RESULT rval = S_OK;

    SpriteBatchMapIterator ppSpriteBatch;

    RETAILMSG(ZONE_SPRITE | ZONE_VERBOSE, "SpriteManager::BeginBatch()");
    
    if (m_inSpriteBatch)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: SpriteManager::BeginBatch(): nested calls not allowed (call ::EndBatch() first)");
        rval = E_INVALID_OPERATION;
        goto Exit;
    }
    
    m_inSpriteBatch  = true;
    m_zOrderForBatch = 0;
    
    // Reset all SpriteBatches
    for (ppSpriteBatch = m_spriteBatchMap.begin(); ppSpriteBatch != m_spriteBatchMap.end(); /*++ppSpriteBatch*/)
    {
        SpriteBatch* pSpriteBatch = ppSpriteBatch->second;
        
        DEBUGCHK( pSpriteBatch );
        
        if (pSpriteBatch->numVertices == 0)
        {
            ++ppSpriteBatch;
            continue;
        }
            
        pSpriteBatch->numSprites    = 0;
        pSpriteBatch->numVertices   = 0;

        // Free the batched sprites
        BatchedSprites* pBatchedSprites = &pSpriteBatch->sprites;
        BatchedSpritesIterator ppBatchedSprite;
        for (ppBatchedSprite = pBatchedSprites->begin(); ppBatchedSprite != pBatchedSprites->end(); ++ppBatchedSprite)
        {
            BatchedSprite* pBatchedSprite = *ppBatchedSprite;
            DEBUGCHK(pBatchedSprite);
            delete pBatchedSprite;
        }
        
        pSpriteBatch->sprites.clear();
        SAFE_ARRAY_DELETE(pSpriteBatch->pVertices);
        
#ifdef DEBUG
        DEBUGMSG(ZONE_SPRITE | ZONE_VERBOSE, "SpriteManager::BeginBatch(): cleared SpriteBatch 0x%x Effect \"%s\" texture \"%s\"", 
            pSpriteBatch, pSpriteBatch->hEffect.GetName().c_str(), pSpriteBatch->hTexture.GetName().c_str());
#endif

        //
        // Free any SpriteBatches if their effect/texture is no longer valid 
        //
        if (pSpriteBatch->hEffect.IsDangling()   ||
            pSpriteBatch->hTexture.IsDangling())
        {
            // See "The C++ Standard Template Library" by Josuttis, pg. 205
            m_spriteBatchMap.erase( ppSpriteBatch++ );
            delete pSpriteBatch;
        }
        else 
        {
            ++ppSpriteBatch;
        }

    }
    
Exit:
    return rval;
}



RESULT SpriteManager::EndBatch()
{
    RESULT rval = S_OK;
    SpriteBatchMapIterator ppSpriteBatch;

    // TODO: sort SpriteBatches before render?
    
    //
    // For each SpriteBatch, submit the vertices.
    //
    for (ppSpriteBatch = m_spriteBatchMap.begin(); ppSpriteBatch != m_spriteBatchMap.end(); ++ppSpriteBatch)
    {
        SpriteBatch*    pSpriteBatch = ppSpriteBatch->second;
        UINT32          index        = 0;
        
        DEBUGCHK( pSpriteBatch );
        
        if ( 0 == pSpriteBatch->numSprites )
            continue;
            
        
        IGNOREHR(Renderer.PushEffect  ( pSpriteBatch->hEffect     ));
        IGNOREHR(Renderer.SetTexture  ( 0, pSpriteBatch->hTexture ));
            
        if (!pSpriteBatch->pVertices)
        {
            // Allocate a vertex array.
            pSpriteBatch->pVertices = new Vertex[ pSpriteBatch->numVertices ];
            if (!pSpriteBatch->pVertices)
            {
                RETAILMSG(ZONE_ERROR, "ERROR: SpriteManager::EndBatch(): out of memory allocating %d vertices", pSpriteBatch->numVertices);
                rval = E_OUTOFMEMORY;
                goto Exit;
            }
        }
        
        //
        // For every BatchedSprite, add its transformed vertices to the batch vertex array.
        //
        BatchedSprites* pBatchedSprites = &pSpriteBatch->sprites;
        BatchedSpritesIterator ppBatchedSprite;
        for (ppBatchedSprite = pBatchedSprites->begin(); ppBatchedSprite != pBatchedSprites->end(); ++ppBatchedSprite)
        {
            BatchedSprite*  pBatchedSprite = *ppBatchedSprite;
            Sprite*         pSprite        = pBatchedSprite->pSprite;
            Vertex*         pVertices      = &pSpriteBatch->pVertices[index];
            
            DEBUGCHK( pSprite );

            Vertex*     pSpriteVertices;
            UINT32      numSpriteVertices;
            CHR(pSprite->GetVertices( &pSpriteVertices, &numSpriteVertices ));
            DEBUGCHK( numSpriteVertices >= VERTS_PER_SPRITE );
            memcpy( pVertices, pSpriteVertices, sizeof(Vertex) * numSpriteVertices );
            index += numSpriteVertices;

            //
            // Transform the quad based on position, scale, rotation.
            //
            mat4 modelview;

            // Scale and rotate around center point.
            // Don't use pSprite->GetBounds() - that reflects the current scale and we want the pre-scaled dimensions.
            // Compute a rect from the vertices instead.
            Rectangle spriteRect;
            Util::GetBoundingRect( pSpriteVertices, numSpriteVertices, &spriteRect );
            vec3 rotationPoint = vec3( spriteRect.width * 0.5f, spriteRect.height * 0.5f, 0.0f);
            
            modelview = mat4::Translate( -rotationPoint.x, -rotationPoint.y, 0.0f );
    
            modelview *= mat4::Scale    ( pBatchedSprite->scale );
            modelview *= mat4::RotateX  ( pBatchedSprite->rotation.x );
            modelview *= mat4::RotateY  ( pBatchedSprite->rotation.y );
            modelview *= mat4::RotateZ  ( pBatchedSprite->rotation.z );

            modelview *= mat4::Translate( rotationPoint.x, rotationPoint.y, 0.0f );

            modelview *= mat4::Translate( pBatchedSprite->position.x, pBatchedSprite->position.y, pBatchedSprite->position.z );
            modelview *= pBatchedSprite->matWorldParent;
            
            for (int i = 0; i < numSpriteVertices; ++i)
            {
                Vertex* pVertex  = &pVertices[i];
                vec4    position = vec4( pVertex->x, pVertex->y, pVertex->z, 1.0f );

                //
                // Scale/Rotate/Translate the vertex
                //
                position = modelview * position;
                
                pVertex->x = position.x;
                pVertex->y = position.y;
                pVertex->z = position.z;

                // Premultiplied Alpha
                pVertex->r = (BYTE) (((float)pVertex->r) * pBatchedSprite->opacity);
                pVertex->g = (BYTE) (((float)pVertex->g) * pBatchedSprite->opacity);
                pVertex->b = (BYTE) (((float)pVertex->b) * pBatchedSprite->opacity);
                pVertex->a = (BYTE) (255.0f * pBatchedSprite->opacity);
            }
        }


        //
        // Submit the vertices
        //
#ifdef DEBUG
        string effectName, textureName;
        if (!pSpriteBatch->hEffect.IsNull())
            Effects.GetName( pSpriteBatch->hEffect,   &effectName   );
        TextureMan.GetName ( pSpriteBatch->hTexture,  &textureName  );
        DEBUGMSG(ZONE_SPRITE, "SpriteManager::EndBatch(): submitting SpriteBatch Effect \"%s\" texture \"%s\" zOrder %d %d Sprite(s) %d vertices", 
            effectName.c_str(), textureName.c_str(), pSpriteBatch->zOrder, pSpriteBatch->numSprites, pSpriteBatch->numVertices);
#endif
      
        CHR(Renderer.SetModelViewMatrix( GameCamera.GetViewMatrix() ));  
        CHR(Renderer.DrawTriangleList( pSpriteBatch->pVertices, pSpriteBatch->numVertices ));
        
        IGNOREHR(Renderer.PopEffect( ));
    } // END draw sprite batch
    
    
    
Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: SpriteManager::EndBatch(): rval = 0x%x", rval);
        DEBUGCHK(0);
    }

    Renderer.EnableAlphaTest( false );

    m_inSpriteBatch  = false;
    m_zOrderForBatch = 0;
    
    return rval;
}



RESULT
SpriteManager::DrawSprite( IN HSprite hSprite, IN const mat4& matParentWorld )
{
    RESULT rval = S_OK;
    
    Sprite* pSprite = GetObjectPointer( hSprite );

    if (pSprite)
    {
        vec3  position  = pSprite->GetPosition();
        vec3  rotation  = pSprite->GetRotation();
        float scale     = pSprite->GetScale();
        float opacity   = pSprite->GetOpacity();
        
        CHR(DrawSprite( hSprite, 
                        position.x, 
                        position.y,
                        position.z,
                        opacity,
                        scale,
                        rotation.x,
                        rotation.y,
                        rotation.z,
                        matParentWorld
                       ));
    }
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: SpriteManager::DrawSprite( 0x%x ): object not found", (UINT32)hSprite);
        rval = E_INVALID_ARG;
        goto Exit;
    }

Exit:
    return rval;
}



RESULT
SpriteManager::DrawSprite( IN HSprite hSprite, IN const WORLD_POSITION& pos, float opacity, float scale, float rotateX, float rotateY, float rotateZ, IN const mat4& matParentWorld )
{
    return DrawSprite( hSprite, pos.x, pos.y, pos.z, opacity, scale, rotateX, rotateY, rotateZ, matParentWorld );
}



RESULT
SpriteManager::DrawSprite( IN HSprite hSprite, float x, float y, float z, float opacity, float scale, float rotateX, float rotateY, float rotateZ, IN const mat4& matParentWorld )
{
    RESULT rval = S_OK;
    
    // Batch up the draw calls and issue them sorted by texture atlas, w/
    // orthographic projection and no depth test, at end of frame.
    
    Sprite* pSprite = GetObjectPointer( hSprite );
    if (pSprite)
    {
        DEBUGMSG(ZONE_SPRITE | ZONE_VERBOSE, "SpriteManager::DrawSprite( %s, [%4.2f, %4.2f, %4.2f] %4.2f, %4.2f, (%4.2f, %4.2f, %4.2f) )", 
                 pSprite->GetName().c_str(),
                 x, y, z, opacity, scale, rotateX, rotateY, rotateZ);

        if (false == pSprite->GetVisible())
        {
            return S_OK;
        }


        //
        // Draw the Sprite immediately, instead of batching?
        //
        if (!m_inSpriteBatch /*|| !pSprite->IsBackedByTextureAtlas()*/ )
        {
            // If the Sprite has a NULL Effect, inherit the Effect that is currently set on the renderer.
            // But only if that Effect is not PostProc.
            HEffect hSpriteEffect = pSprite->GetEffect();
            if ( hSpriteEffect.IsNull() )
            {
                HEffect hParentEffect;
                CHR(Renderer.GetEffect( &hParentEffect ));
                hSpriteEffect = hParentEffect;
                
                if ( !hParentEffect.IsNull() && !EffectMan.IsPostEffect( hParentEffect ))
                    pSprite->SetEffect( hSpriteEffect );
            }


            // TODO: enable alpha blend only if opacity < 1.0 or sprite has per-pixel alpha channel.

            vec3 position(x, y, z);
            vec3 rotation(rotateX, rotateY, rotateZ);
            
            CHR(pSprite->SetOpacity ( opacity        ));
            CHR(pSprite->SetScale   ( scale          ));
            CHR(pSprite->SetPosition( position       ));
            CHR(pSprite->SetRotation( rotation       ));
            CHR(pSprite->Draw       ( matParentWorld ));
            
            goto Exit;
        }

        
        //
        // Has a SpriteBatch already been created for this Effect / texture atlas combo?
        // If not, create one.
        //
        SpriteBatch*    pSpriteBatch    = NULL;
        HEffect         hSpriteEffect   = pSprite->GetEffect();;
        HTexture        hSpriteTexture;
        TextureInfo     textureInfo;

        //
        // If the Sprite has a NULL Effect, batch it with whichever Effect is currently set on the renderer.
        //
        if ( hSpriteEffect.IsNull() )
        {
            HEffect hParentEffect;
            CHR(Renderer.GetEffect( &hParentEffect ));

            if ( !hParentEffect.IsNull() && !EffectMan.IsPostEffect( hParentEffect ))
            {
                hSpriteEffect = hParentEffect;
            }
            else
            {
                Effects.Get( "DefaultEffect", &hParentEffect );
                hSpriteEffect = hParentEffect;
            }
        }


        CHR(pSprite->GetTexture( &hSpriteTexture ));
        CHR(TextureMan.GetInfo( hSpriteTexture, &textureInfo ));
        
        
        // TODO: we need a rich key for sorting on Layer/Effect/Texture/opacity etc.
        // See Christer Ericson's article: http://realtimecollisiondetection.net/blog/?p=86 
        SpriteBatchKey key;
        key.hEffect   = hSpriteEffect;
        key.textureID = textureInfo.textureID;
        
        SpriteBatchMapIterator ppSpriteBatchMap = m_spriteBatchMap.find( key );
        if (ppSpriteBatchMap == m_spriteBatchMap.end())
        {
            // Haven't encountered this combination of Effect and texture atlas before; allocate a new SpriteBatch for it.
            pSpriteBatch                    = new SpriteBatch();
            pSpriteBatch->hEffect           = hSpriteEffect;
            pSpriteBatch->hTexture          = hSpriteTexture;
            pSpriteBatch->textureID         = textureInfo.textureID;
            pSpriteBatch->numSprites        = 0;
            pSpriteBatch->numVertices       = 0;
            pSpriteBatch->pVertices         = NULL;
            pSpriteBatch->zOrder            = key.zOrder;
            
            m_spriteBatchMap.insert( std::make_pair(key, pSpriteBatch) );
            
    #ifdef DEBUG            
            DEBUGMSG(ZONE_SPRITE, "Created SpriteBatch for Effect \"%s\" texture \"%s\"", 
                hSpriteEffect.GetName().c_str(), hSpriteTexture.GetName().c_str());
    #endif            
        }
        else 
        {
            pSpriteBatch = ppSpriteBatchMap->second;
        }


        //
        // Add Sprite to SpriteBatch
        //
        BatchedSprite* pBatchedSprite   = new BatchedSprite();            // TODO: replace with a FrameAllocator!!
        pBatchedSprite->pSprite         = pSprite;
        pBatchedSprite->position        = vec3(x, y, z);
        pBatchedSprite->opacity         = opacity;
        pBatchedSprite->scale           = scale;
        pBatchedSprite->rotation        = vec3(rotateX, rotateY, rotateZ);
        pBatchedSprite->matWorldParent  = matParentWorld * GameCamera.GetViewMatrix();
        
        pSpriteBatch->sprites.push_back( pBatchedSprite );
        pSpriteBatch->numSprites++;

        UINT32  numVertices = 0;
        Vertex* pVertices   = NULL;
        CHR(pSprite->GetVertices(&pVertices, &numVertices));
        pSpriteBatch->numVertices += numVertices;
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
//  Sprite Implementation
//
// ============================================================================

#pragma mark -
#pragma mark Sprite Implementation


//----------------------------------------------------------------------------
// We expose the following named properties for animation and scripting.
//----------------------------------------------------------------------------
static const NamedProperty s_propertyTable[] =
{
    DECLARE_PROPERTY( Sprite, PROPERTY_VEC3,   Position     ),
    DECLARE_PROPERTY( Sprite, PROPERTY_VEC3,   Rotation     ),
    DECLARE_PROPERTY( Sprite, PROPERTY_FLOAT,  Scale        ),
    DECLARE_PROPERTY( Sprite, PROPERTY_FLOAT,  Opacity      ),
    DECLARE_PROPERTY( Sprite, PROPERTY_COLOR,  Color        ),
    DECLARE_PROPERTY( Sprite, PROPERTY_UINT32, SpriteFrame  ),
    //    DECLARE_PROPERTY( Sprite, PROPERTY_BOOL,  Visible   ),    // asserts that SetVisible() is NULL ??
    NULL,
};
DECLARE_PROPERTY_SET( Sprite, s_propertyTable );






Sprite::Sprite() :
    m_width(0.0f),
    m_height(0.0f),
    m_frame(0),
    m_numFrames(0),
    m_fScale(1.0),
    m_fOpacity(1.0),
    m_isVisible(true),
    m_color(Color::White()),
    m_hasShadow(false),
    m_isBackedByTextureAtlas(false)
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "Sprite( %4d )", m_ID);
    
    memset(&m_vertices, 0, sizeof(m_vertices));
    
    m_bounds.SetMin( vec3( 0.0f, 0.0f, 0.0f ) );
    m_bounds.SetMax( vec3( 0.0f, 0.0f, 0.0f ) );
}



Sprite::~Sprite()
{
    RETAILMSG(ZONE_OBJECT, "\t~Sprite( %4d )", m_ID);
    
    for (int i = 0; i < m_numFrames; ++i)
    {
        TextureMan.Release( m_hTextures[i] );
    }
    
    EffectMan.Release ( m_hEffect );
    
    memset(&m_vertices, 0, sizeof(m_vertices));
}


// TODO: need proper copy constructor and assignment operation; see Animation.

Sprite*
Sprite::Clone() const
{
    RESULT  rval         = S_OK;
    Sprite* pSpriteClone = new Sprite(*this);

    // Give it a new name with a random suffix
    char instanceName[MAX_NAME];
    sprintf(instanceName, "%s_%X", m_name.c_str(), (unsigned int)Platform::Random());
    pSpriteClone->m_name = string(instanceName);
    
    pSpriteClone->m_color = m_color;
    pSpriteClone->m_frame = 0;
    pSpriteClone->m_isBackedByTextureAtlas = m_isBackedByTextureAtlas;

    // Take new reference to Effect.
    pSpriteClone->m_hEffect = m_hEffect;
    if ( !m_hEffect.IsNull() )
    {
        CHR(EffectMan.AddRef( m_hEffect ));
    }

    // Take new reference to Texture.
    for (int i = 0; i < m_numFrames; ++i)
    {
        pSpriteClone->m_hTextures[i] = m_hTextures[i];
        CHR(TextureMan.AddRef( pSpriteClone->m_hTextures[i] ));
    }

Exit:    
    return pSpriteClone;
}



RESULT
Sprite::Init( const string& name, const Settings* pSettings, const string& settingsPath )
{
    RESULT      rval        = S_OK;
    string      textureName;
    Rectangle   rect;
    TextureInfo textureInfo;
    
    
    RETAILMSG(ZONE_OBJECT, "Sprite[%4d]::Init( %s )", m_ID, name.c_str());
    
    if (!pSettings)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Sprite::Init(): NULL settings");
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    
    // Set the members
    m_name      = name;
    m_numFrames = pSettings->GetInt   ( settingsPath + ".NumFrames"  );
    m_width     = pSettings->GetFloat ( settingsPath + ".Width"      );
    m_height    = pSettings->GetFloat ( settingsPath + ".Height"     );
    m_color     = pSettings->GetColor ( settingsPath + ".Color", Color::White() );

    m_bounds.SetMin(vec3( 0.0f, 0.0f, 0.0f ));
    m_bounds.SetMax(vec3( m_width, m_height, 0.0f ));
    
    
    CBR(m_numFrames     > 0);
    CBR(m_name.length() > 0);
    CBR(m_width         > 0);
    CBR(m_height        > 0);
    
    // TODO: dynamically allocate sprite animation frames
    DEBUGCHK(m_numFrames <= MAX_SPRITE_FRAMES);
    m_numFrames = MIN(m_numFrames, MAX_SPRITE_FRAMES);
    
    char path[MAX_PATH];
    for (int i = 0; i < m_numFrames; ++i)
    {
        sprintf(path, "%s/Frame%d", settingsPath.c_str(), i);
        
        textureName = pSettings->GetString( string(path) + ".Texture"    );
        rval = TextureMan.Get    ( textureName, &m_hTextures[i] );
        
        if (FAILED(rval))
        {
            RETAILMSG(ZONE_ERROR, "Sprite[%4d]: \"%s\", texture not found: \"%s\"", m_ID, m_name.c_str(), textureName.c_str());
            continue;
        }
        
        CHR(TextureMan.GetInfo( m_hTextures[i], &textureInfo ));

        // Take a reference to the texture.
        m_hTextures[i].AddRef();
        
        // Create the textured quad
        rect.x      = 0;
        rect.y      = 0;
        rect.width  = m_width;
        rect.height = m_height;
        
        CHR(Util::CreateTriangleList( &rect,
                                      1,
                                      1,
                                      &m_vertices[i][0], 
                                      textureInfo.uStart, 
                                      textureInfo.uEnd, 
                                      textureInfo.vStart, 
                                      textureInfo.vEnd,
                                      m_color ));
        
    }
    

    CHR(TextureMan.GetInfo( m_hTextures[0], &textureInfo ));
    m_isBackedByTextureAtlas = textureInfo.isBackedByTextureAtlas;
    

    RETAILMSG(ZONE_SPRITE, "Sprite[%4d]: \"%-32s\" %d x %d frames: %d", m_ID, m_name.c_str(), m_width, m_height, m_numFrames);

Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Sprite::Init( \"%s\" ): failed", name.c_str());
    }
    return rval;
}



RESULT
Sprite::Init( IN const string& name, IN const HTexture& hTexture, IN const Rectangle& spriteRect, IN const Color& color )
{
    RESULT      rval        = S_OK;
    TextureInfo textureInfo;
    
    CBR( !hTexture.IsNull() );
    m_hTextures[0] = hTexture;
    
    RETAILMSG(ZONE_OBJECT, "Sprite[%4d]::Init( %s )", m_ID, name.c_str());

    CHR(TextureMan.GetInfo( hTexture, &textureInfo ));
    
    // Take a reference to the texture.
    hTexture.AddRef();
    
    // Set the members
    m_name      = name;
    m_numFrames = 1;
    m_width     = spriteRect.width;
    m_height    = spriteRect.height;
    m_color     = color;
    
    m_isBackedByTextureAtlas = textureInfo.isBackedByTextureAtlas;
    
    m_bounds.SetMin(vec3( 0.0f, 0.0f, 0.0f ));
    m_bounds.SetMax(vec3( m_width, m_height, 0.0f ));
    
    
    //
    // Create a textured quad for the Sprite.
    // For now, we only support a single frame in this convenience method.
    // Multi-frame Sprites should be defined offline with XML.
    //
    CHR(Util::CreateTriangleList( &spriteRect,
                                  1,
                                  1,
                                  &m_vertices[0][0], 
                                  textureInfo.uStart, 
                                  textureInfo.uEnd, 
                                  textureInfo.vStart, 
                                  textureInfo.vEnd,
                                  m_color ));
    
    RETAILMSG(ZONE_SPRITE, "Sprite[%4d]: \"%-32s\" %d x %d frames: %d", m_ID, m_name.c_str(), m_width, m_height, m_numFrames);
    
Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Sprite::Init( \"%s\" hTexture: 0x%x ): failed", name.c_str(), (UINT32)hTexture);
    }
    return rval;
}




RESULT
Sprite::GetVertices( INOUT Vertex** ppVertices, OUT UINT32* pNumVertices )
{
    RESULT rval = S_OK;
    
    if (!ppVertices || !pNumVertices)
    {
        return E_NULL_POINTER;
    }
    
    *ppVertices     = &m_vertices[m_frame][0];
    *pNumVertices   = VERTS_PER_SPRITE;
    
Exit:
    return rval;
}



RESULT
Sprite::GetTexture( INOUT HTexture* phTexture )
{
    RESULT rval = S_OK;
    
    if (!phTexture)
    {
        return E_NULL_POINTER;
    }
    
    *phTexture = m_hTextures[m_frame];
    
Exit:
    return rval;
}


RESULT
Sprite::SetSpriteFrame( UINT8 frame )
{
    if (frame < m_numFrames)
    {
        m_frame = frame;
        return S_OK;
    }
    else
    {
        return E_INVALID_ARG;
    }
}



//
// IDrawable
//
RESULT  
Sprite::SetOpacity( float opacity )
{
    RESULT rval = S_OK;
    
    opacity = CLAMP(opacity, 0.0, 1.0);
    m_fOpacity = opacity;
    
Exit:
    return rval;
}




RESULT  
Sprite::SetColor( const Color& color )
{
    RESULT rval = S_OK;
    
    m_color = color;
    
    //
    // Hack hack; set the shader's ambient color instead.
    // Only do this on OpenGL ES 1.x platforms.
    //
    for (int frame = 0; frame < m_numFrames; ++frame)
    {
        for (int i = 0; i < VERTS_PER_SPRITE; ++i)
        {
            m_vertices[frame][i].color = color;
        }
    }
    
Exit:
    return rval;
}




RESULT
Sprite::SetEffect( HEffect hEffect )
{
    RESULT rval = S_OK;

    if (m_hEffect != hEffect)
    {
        IGNOREHR(Effects.Release(m_hEffect));
        m_hEffect = hEffect;
    }
    
Exit:
    return rval;
}



void
Sprite::UpdateBoundingBox()
{
    mat4    modelview;
    vec4    min, max;

    min = vec4( 0, 0, 0, 1 ); 
    max = vec4( m_width, m_height, 0, 1 ); 
    
    //
    // Transform the quad based on position, scale.
    //
    modelview  = mat4::Scale    ( m_fScale );
    modelview *= mat4::RotateX  ( m_vRotation.x );
    modelview *= mat4::RotateY  ( m_vRotation.y );
    modelview *= mat4::RotateZ  ( m_vRotation.z );
    modelview *= mat4::Translate( m_vWorldPosition.x, m_vWorldPosition.y, m_vWorldPosition.z );

    // apply to min/max
    min = modelview * min;
    max = modelview * max;
    
    // Update bounding box
    m_bounds.SetMin( vec3( min.x, min.y, min.z ) );
    m_bounds.SetMax( vec3( max.x, max.y, max.z ) );
}



RESULT  
Sprite::Draw( const mat4& matParentWorld )
{
    RESULT      rval                = S_OK;
    mat4        modelview;
    
    //
    // Skip drawing sprites w/ zero opacity.
    //
    if (Util::CompareFloats(m_fOpacity, 0.0f))
        return rval;
    
    //
    // Transform the quad based on position, scale.
    //
    
    // Scale and rotate around center point!
    Rectangle spriteRect;
    uint32_t numVertices = MAX_SPRITE_FRAMES * VERTS_PER_SPRITE;
    Util::GetBoundingRect( &m_vertices[0][0], numVertices, &spriteRect );
    vec3 rotationPoint = vec3( spriteRect.width * 0.5f, spriteRect.height * 0.5f, 0.0f);
    
    modelview = mat4::Translate( -rotationPoint.x, -rotationPoint.y, 0.0f );

    modelview *= mat4::Scale( m_fScale );
    modelview *= mat4::RotateX( m_vRotation.x );
    modelview *= mat4::RotateY( m_vRotation.y );
    modelview *= mat4::RotateZ( m_vRotation.z );

    modelview *= mat4::Translate( rotationPoint.x, rotationPoint.y, 0.0f );

    modelview *= mat4::Translate( m_vWorldPosition.x, m_vWorldPosition.y, m_vWorldPosition.z );

    modelview *= matParentWorld;

    //
    // HACK: duplicate vertices so we can apply pre-multiplied Alpha to them,
    // without losing the original RGB values.
    // Keeps things consistent when animating vertex opacity with a texture
    // that already had per-pixel opacity.
    // We do basically the same thing in SpriteBatch.
    //
    Vertex vertices[m_numFrames][VERTS_PER_SPRITE];
    memcpy(&vertices, m_vertices, sizeof(Vertex)*VERTS_PER_SPRITE*m_numFrames);
    for (int frame = 0; frame < m_numFrames; ++frame)
    {
        for (int i = 0; i < VERTS_PER_SPRITE; ++i)
        {
            Vertex* pOldVertex = &m_vertices[frame][i];
            Vertex* pNewVertex = &vertices[frame][i];
        
            // Premultiplied Alpha
            pNewVertex->r = (BYTE) (((float)pOldVertex->r) * m_fOpacity);
            pNewVertex->g = (BYTE) (((float)pOldVertex->g) * m_fOpacity);
            pNewVertex->b = (BYTE) (((float)pOldVertex->b) * m_fOpacity);
            pNewVertex->a = (BYTE) (255.0f * m_fOpacity);
        }
    }


    CHR(Renderer.PushEffect( m_hEffect ));
    CHR(Renderer.SetModelViewMatrix( modelview ));
    CHR(Renderer.SetTexture( 0, m_hTextures[m_frame] ));

    CHR(Renderer.DrawTriangleList( &vertices[m_frame][0], VERTS_PER_SPRITE ));
    
Exit:    
    IGNOREHR(Renderer.PopEffect());
    return rval;
}





IProperty*
Sprite::GetProperty( const string& propertyName ) const
{
    return s_properties.Get( this, propertyName );
}




} // END namespace Z



