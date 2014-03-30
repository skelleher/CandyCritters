#pragma once

#include "ResourceManager.hpp"
#include "Handle.hpp"
#include "Object.hpp"
#include "Settings.hpp"
#include "GameObject.hpp"
#include "Layer.hpp"

#include <string>
using std::string;


namespace Z
{



//=============================================================================
//
// LayerManager
//
//=============================================================================

class LayerManager : public ResourceManager<Layer>
{
public:
    static  LayerManager& Instance();
    
    virtual RESULT  Init            ( IN const string& settingsFilename );
    
    RESULT          CreateLayer     ( IN const string& name, INOUT HLayer*          phLayer          );
    RESULT          AddToLayer      ( IN HLayer hLayer,      IN HGameObject         hGameObject      );
    RESULT          AddToLayer      ( IN HLayer hLayer,      IN HSprite             hSprite          );
    RESULT          AddToLayer      ( IN HLayer hLayer,      IN HLayer              hChildLayer      );
    RESULT          AddToLayer      ( IN HLayer hLayer,      IN HParticleEmitter    hParticleEmitter );
    RESULT          RemoveFromLayer ( IN HLayer hLayer,      IN HGameObject         hGameObject      );
    RESULT          RemoveFromLayer ( IN HLayer hLayer,      IN HSprite             hSprite          );
    RESULT          RemoveFromLayer ( IN HLayer hLayer,      IN HLayer              hChildLayer      );
    RESULT          RemoveFromLayer ( IN HLayer hLayer,      IN HParticleEmitter    hParticleEmitter );
    RESULT          Clear           ( IN HLayer hLayer );
    
    RESULT          Draw            ( );
    RESULT          Draw            ( IN HLayer hLayer, IN const mat4& matParentWorld = mat4::Identity() );
    

    RESULT          Show            ( IN HLayer hLayer, IN HStoryboard hStoryboard = HStoryboard::NullHandle(), IN HEffect hEffect = HEffect::NullHandle() );
    RESULT          Hide            ( IN HLayer hLayer, IN HStoryboard hStoryboard = HStoryboard::NullHandle(), IN HEffect hEffect = HEffect::NullHandle() );
    

	// IDrawable
    // TODO: are handles worth the annoyance of having to expose every object method like this?!
    RESULT          SetVisible      ( IN HLayer hLayer, bool        isVisible        );
    RESULT          SetPosition     ( IN HLayer hLayer, const vec3& vPos             );
    RESULT          SetRotation     ( IN HLayer hLayer, const vec3& vRotationDegrees );
    RESULT          SetScale        ( IN HLayer hLayer, float       scale            );
    RESULT          SetOpacity      ( IN HLayer hLayer, float       opacity          );
    RESULT          SetEffect       ( IN HLayer hLayer, HEffect     hEffect          );
    RESULT          SetShadow       ( IN HLayer hLayer, bool        shadowEnabled    );

    bool            GetVisible      ( IN HLayer hLayer                               );
    vec3            GetPosition     ( IN HLayer hLayer                               );
    vec3            GetRotation     ( IN HLayer hLayer                               );
    float           GetScale        ( IN HLayer hLayer                               );
    float           GetOpacity      ( IN HLayer hLayer                               );
    HEffect         GetEffect       ( IN HLayer hLayer                               );
    AABB            GetBounds       ( IN HLayer hLayer                               );
    bool            GetShadow       ( IN HLayer hLayer                               );



protected:
    LayerManager();
    LayerManager( const LayerManager& rhs );
    LayerManager& operator=( const LayerManager& rhs );
    virtual ~LayerManager();
 
protected:
    RESULT          CreateLayer( IN const Settings* pSettings, IN const string& settingsPath, INOUT Layer** ppLayer );
    RESULT          Draw       ( IN Layer* pLayer, IN const mat4& matParentWorld = mat4::Identity() );
};

#define LayerMan ((LayerManager&)LayerManager::Instance())
#define Layers   ((LayerManager&)LayerManager::Instance())



} // END namespace Z


