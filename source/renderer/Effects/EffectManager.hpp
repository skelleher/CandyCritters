#pragma once

#include "IEffect.hpp"
#include "ShaderManager.hpp"
#include "Log.hpp"


namespace Z
{


template<typename TYPE>
class EffectFactory
{
public:
    static TYPE* Create()
    {
        //return new TYPE();
        return TYPE::CreateInstance();
    }


protected:
    EffectFactory();
    EffectFactory( const EffectFactory& rhs );
    EffectFactory& operator=( const EffectFactory& rhs );
    virtual ~EffectFactory();

};



class EffectManager : public ResourceManager<IEffect>
{
public:
    RESULT  Init                 ( IN const string& settingsFilename );
    RESULT  PrewarmEffects       ( );

    bool    IsPostEffect         ( IN HEffect handle );

    RESULT  Enable               ( IN HEffect handle );
    RESULT  BeginFrame           ( IN HEffect handle, IN const Rectangle& frame );
    RESULT  EndFrame             ( IN HEffect handle, bool present );

    RESULT  SetModelViewMatrix   ( IN HEffect handle, IN const mat4& modelView  );
    RESULT  SetProjectionMatrix  ( IN HEffect handle, IN const mat4& projection );
    RESULT  SetTexture           ( IN HEffect handle, UINT8 textureUnit, HTexture hTexture  );
    RESULT  SetTexture           ( IN HEffect handle, UINT8 textureUnit, UINT32   textureID );
    RESULT  SetGlobalColor       ( IN HEffect handle, IN const Color& color );                      // Everything drawn is multiplied by this color. Useful for setting global opacity.

    RESULT  DrawTriangleStrip    ( IN HEffect handle, IN Vertex *pVertices, UINT32 numVertices );
    RESULT  DrawTriangleList     ( IN HEffect handle, IN Vertex *pVertices, UINT32 numVertices );
//    RESULT DrawPointSprites     ( IN Vertex *pVertices, UINT32 numVertices, float fScale = 8.0f );
//    RESULT DrawLines            ( IN Vertex *pVertices, UINT32 numVertices, float fWidth = 1.0f );

    HShader GetShader           ( IN HEffect handle );
    GLuint  GetShaderProgramID  ( IN HEffect handle );
    RESULT  GetPointer          ( IN HEffect handle, INOUT IEffect** ppIEffect );

protected:
    EffectManager();
    EffectManager( const EffectManager& rhs );
    EffectManager& operator=( const EffectManager& rhs );
    virtual ~EffectManager();


protected:
    RESULT   CreateEffect    ( IN Settings* pSettings, IN const string& settingsPath, INOUT IEffect** ppEffect );
    IEffect* Create          ( IN const string& EffectName );
    RESULT   PrewarmEffect   ( IN IEffect* pEffect );



protected:
    typedef IEffect*(*EFFECT_FACTORY_METHOD)();
    typedef struct 
    {
        const char*             name;
        EFFECT_FACTORY_METHOD   factoryMethod;
    } EffectFactoryItem;


    static EffectFactoryItem s_EffectFactories[];
};

// TODO: rename ALL ResourceManagers to use simpler nameing: Textures, Sounds, Sprites, etc.
//#define Effects   ((EffectManager&)EffectManager::Instance())
#define EffectMan ((EffectManager&)EffectManager::Instance())


} // END namespace Z


