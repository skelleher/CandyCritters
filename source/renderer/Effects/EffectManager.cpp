#include "EffectManager.hpp"
#include "PerfTimer.hpp"
#include "Util.hpp"


namespace Z
{



RESULT
EffectManager::Init( IN const string& settingsFilename )
{
    RETAILMSG(ZONE_INFO, "EffectManager::Init( %s )", settingsFilename.c_str());

    s_pResourceManagerName = "EffectManager";

    RESULT rval = S_OK;
    char   path[MAX_PATH];
    
    //
    // Create a Settings object and load the file.
    //
    Settings mySettings;
    if ( FAILED(mySettings.Read( settingsFilename )) )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: EffectManager::Init( %s ): failed to load settings file", settingsFilename.c_str() );
        return E_UNEXPECTED;
    }
    

    //
    // Create each Effect.
    //
    UINT32 numEffects = mySettings.GetInt("/Effects.NumEffects");

    for (int i = 0; i < numEffects; ++i)
    {
        sprintf(path, "/Effects/Effect%d", i);

        IEffect *pEffect = NULL;
        CreateEffect( &mySettings, path, &pEffect );
        if (!pEffect)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: EffectManager::Init( %s ): failed to create Effect", path);
            // Continue loading other Effects rather than aborting.
            continue;
        }
        
        CHR(Add(pEffect->GetName(), pEffect));
    }
    
Exit:
    return rval;
}



RESULT
EffectManager::CreateEffect( IN Settings* pSettings, IN const string& settingsPath, INOUT IEffect** ppEffect )
{
    RESULT rval = S_OK;
    string name;
    
    if (!pSettings || "" == settingsPath || !ppEffect)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: EffectManager::CreateEffect(): invalid argument");
        return E_INVALID_ARG;
    }
    
    name = pSettings->GetString( settingsPath + ".Name" );
    IEffect* pEffect = Create( name );
    CPR(pEffect);

    // TODO: init Effects here, or defer until they are used?
    // Don't want to waste resources allocating frame buffers and textures for "template" effects,
    // but is a good idea to pre-create the shaders.

    // Caller must AddRef()
    *ppEffect = pEffect;
    
Exit:    
    return rval;
}



RESULT
EffectManager::PrewarmEffects( )
{
    RESULT rval = S_OK;
    
    ResourceListIterator ppEffect;
    for (ppEffect = m_resourceList.begin(); ppEffect != m_resourceList.end(); ++ppEffect)
    {
        IEffect* pEffect = *ppEffect;
        
        if (pEffect)
        {
            IGNOREHR(PrewarmEffect( pEffect ));
        }
    }
    
Exit:
    return rval;
}



RESULT
EffectManager::PrewarmEffect( IN IEffect* pEffect )
{
    RESULT rval = S_OK;
    
    CPR(pEffect);

    static PerfTimer timer;
    timer.Start();

    static Rectangle rect = { 0,0,32,32 };
    static Vertex    pVertices[4];
    static bool      initialized = false;
    
    if (!initialized)
    {
        CHR(Util::CreateTriangleStrip( &rect, pVertices ));
        initialized = true;
    }
    

    //
    // Render a single small quad.
    // This forces the Effect to compile its shaders and create scratch surfaces,
    // ensuring it is instantly ready the first time it's used for real.
    // Otherwise, the user may see a delay (especially bad when showing/hiding a Scene).
    //
    CHR(pEffect->BeginFrame( rect ));
    CHR(pEffect->DrawTriangleStrip(pVertices, 4));
    CHR(pEffect->EndFrame( true ));
    
    
    timer.Stop();
    
    RETAILMSG(ZONE_SHADER, "EffectManager::PrewarmEffect( \"%s\" ): %4.2f ms",
             pEffect->GetName().c_str(),
             timer.ElapsedMilliseconds());
    
Exit:
    return rval;
}



IEffect* 
EffectManager::Create( IN const string& EffectName )
{
    IEffect*           pEffect  = NULL;
    EffectFactoryItem* pFactory = NULL;
    int                i        = 0;
    
    do 
    {
        pFactory = &s_EffectFactories[i++];
    
        if (EffectName == pFactory->name)
        {
            DEBUGMSG(ZONE_RESOURCE, "EffectManager::Create( \"%s\" )", EffectName.c_str());
            
            pEffect = pFactory->factoryMethod();
            break;
        }
    } while (pFactory->factoryMethod != NULL);
    
    if (!pEffect)
    {
        RETAILMSG(ZONE_ERROR, "EffectManager::Create( \"%s\" ): not found", EffectName.c_str());
    }
    
    return pEffect;
}



HShader
EffectManager::GetShader( IN const HEffect handle )
{
    HShader rval;
    
    IEffect* pEffect = GetObjectPointer( handle );
    if (pEffect)
    {
        DEBUGMSG(ZONE_SHADER | ZONE_VERBOSE, "EffectManager::GetShader( %s )", 
                 pEffect->GetName().c_str());
        
        rval = pEffect->GetShader();
    }
    
Exit:
    return rval;
}


GLuint
EffectManager::GetShaderProgramID( IN const HEffect handle )
{
    GLuint rval = 0xFFFFFFFF;
    
    IEffect* pEffect = GetObjectPointer( handle );
    if (pEffect)
    {
        DEBUGMSG(ZONE_SHADER | ZONE_VERBOSE, "EffectManager::GetShader( %s )", 
                 pEffect->GetName().c_str());
        
        HShader hShader = pEffect->GetShader();
        rval = ShaderMan.GetShaderProgramID( hShader );
    }
    
Exit:
    return rval;
}



// HACK HACK: callers need to set custom Effect properties,
// and here our handle-based system breaks down entirely.
// Allow the caller to fetch a raw pointer and cast it to the correct type.
// TODO: SmartPointers, QueryInterface!!!!
RESULT
EffectManager::GetPointer( IN HEffect handle, INOUT IEffect** ppEffect )
{
    RESULT   rval    = S_OK;
    IEffect* pEffect = NULL;
    
    if (!ppEffect)
    {
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    
    pEffect = GetObjectPointer( handle );
    if (!pEffect)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: EffectManager::GetPointer(): bad handle");
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    
    // We have NOT increased the Effect's refcount!
    // Caller already holds a ref via the handle.
    //
    // Any access via the pointer is NOT thread-safe!
    *ppEffect = pEffect;
    
Exit:
    return rval;
}



bool
EffectManager::IsPostEffect( IN HEffect handle )
{
    bool rval = false;
    
    IEffect* pEffect = GetObjectPointer( handle );
    if (pEffect)
    {
        rval = pEffect->IsPostEffect();
    }
    
Exit:
    return rval;
}



RESULT
EffectManager::Enable( IN HEffect handle )
{
    RESULT rval = S_OK;
    
    IEffect* pEffect = GetObjectPointer( handle );
    if (pEffect)
    {
        rval = pEffect->Enable( );
    }
    else
    {
        rval = E_BAD_HANDLE;
    }
    
Exit:
    return rval;
}



RESULT
EffectManager::BeginFrame( IN HEffect handle, IN const Rectangle& frame )
{
    RESULT rval = S_OK;
    
    IEffect* pEffect = GetObjectPointer( handle );
    if (pEffect)
    {
        rval = pEffect->BeginFrame( frame );
    }
    else
    {
        rval = E_BAD_HANDLE;
    }
    
Exit:
    return rval;
}



RESULT
EffectManager::EndFrame( IN HEffect handle, bool present  )
{
    RESULT rval = S_OK;
    
    IEffect* pEffect = GetObjectPointer( handle );
    if (pEffect)
    {
        rval = pEffect->EndFrame( present );
    }
    else
    {
        rval = E_BAD_HANDLE;
    }
    
Exit:
    return rval;
}



RESULT
EffectManager::SetModelViewMatrix( IN HEffect handle, IN const mat4& modelView  )
{
    RESULT rval = S_OK;
    
    IEffect* pEffect = GetObjectPointer( handle );
    if (pEffect)
    {
        rval = pEffect->SetModelViewMatrix( modelView );
    }
    else
    {
        rval = E_BAD_HANDLE;
    }
    
Exit:
    return rval;
}



RESULT
EffectManager::SetProjectionMatrix( IN HEffect handle, IN const mat4& projection )
{
    RESULT rval = S_OK;
    
    IEffect* pEffect = GetObjectPointer( handle );
    if (pEffect)
    {
        rval = pEffect->SetProjectionMatrix( projection );
    }
    else
    {
        rval = E_BAD_HANDLE;
    }
    
Exit:
    return rval;
}



RESULT
EffectManager::SetTexture( IN HEffect handle, UINT8 textureUnit, UINT32 textureID )
{
    RESULT rval = S_OK;
    
    IEffect* pEffect = GetObjectPointer( handle );
    if (pEffect)
    {
        rval = pEffect->SetTexture( textureUnit, textureID );
    }
    else
    {
        rval = E_BAD_HANDLE;
    }
    
Exit:
    return rval;
}



RESULT
EffectManager::SetGlobalColor( IN HEffect handle, IN const Color& color )
{
    RESULT rval = S_OK;
    
    IEffect* pEffect = GetObjectPointer( handle );
    if (pEffect)
    {
        rval = pEffect->SetGlobalColor( color );
    }
    else
    {
        rval = E_BAD_HANDLE;
    }
    
Exit:
    return rval;
}



RESULT
EffectManager::SetTexture( IN HEffect handle, UINT8 textureUnit, HTexture hTexture )
{
    RESULT rval = S_OK;
    
    IEffect* pEffect = GetObjectPointer( handle );
    if (pEffect)
    {
        rval = pEffect->SetTexture( textureUnit, hTexture );
    }
    else
    {
        rval = E_BAD_HANDLE;
    }
    
Exit:
    return rval;
}



RESULT
EffectManager::DrawTriangleStrip( IN HEffect handle, IN Vertex *pVertices, UINT32 numVertices  )
{
    RESULT rval = S_OK;
    
    IEffect* pEffect = GetObjectPointer( handle );
    if (pEffect)
    {
        rval = pEffect->DrawTriangleStrip( pVertices, numVertices );
    }
    else
    {
        rval = E_BAD_HANDLE;
    }
    
Exit:
    return rval;
}




RESULT
EffectManager::DrawTriangleList( IN HEffect handle, IN Vertex *pVertices, UINT32 numVertices  )
{
    RESULT rval = S_OK;
    
    IEffect* pEffect = GetObjectPointer( handle );
    if (pEffect)
    {
        rval = pEffect->DrawTriangleList( pVertices, numVertices );
    }
    else
    {
        rval = E_BAD_HANDLE;
    }
    
Exit:
    return rval;
}




} // END namespace Z

