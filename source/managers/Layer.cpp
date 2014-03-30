/*
 *  Layer.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 11/4/10.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include "Layer.hpp"
#include "Util.hpp"
#include "SpriteManager.hpp"
#include "GameObjectManager.hpp"
#include "StoryboardManager.hpp"
#include "ParticleManager.hpp"
#include "Engine.hpp"


namespace Z 
{

//----------------------------------------------------------------------------
// We expose the following named properties for animation and scripting.
//----------------------------------------------------------------------------
static const NamedProperty s_propertyTable[] =
{
    DECLARE_PROPERTY( Layer, PROPERTY_VEC3,  Position  ),
    DECLARE_PROPERTY( Layer, PROPERTY_VEC3,  Rotation  ),
    DECLARE_PROPERTY( Layer, PROPERTY_FLOAT, Scale     ),
    DECLARE_PROPERTY( Layer, PROPERTY_FLOAT, Opacity   ),
//    DECLARE_PROPERTY( Layer, PROPERTY_BOOL,  Visible   ),    // asserts that SetVisible() is NULL ??
    NULL,
};
DECLARE_PROPERTY_SET( Layer, s_propertyTable );



// ============================================================================
//
//  Layer Implementation
//
// ============================================================================


#pragma mark Layer Implementation

Layer::Layer() :
    m_isVisible(true),
    m_fScale(1.0f),
    m_fOpacity(1.0f),
    m_color(Color::White()),
    m_isShadowEnabled(false)
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "Layer( %4d )", m_ID);

    m_bounds.SetMin( vec3( 0.0f, 0.0f, 0.0f ) );
    m_bounds.SetMax( vec3( 0.0f, 0.0f, 0.0f ) );
}


Layer::~Layer()
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "\t~Layer( %4d )", m_ID);
    

    SpriteListIterator      pHSprite;
    for (pHSprite = m_spriteList.begin(); pHSprite != m_spriteList.end(); ++pHSprite)
    {
        HSprite hSprite = *pHSprite;
        SpriteMan.Release( hSprite );
    }
    m_spriteList.clear();
    
    GameObjectListIterator  pHGameObject;
    for (pHGameObject = m_gameObjectList.begin(); pHGameObject != m_gameObjectList.end(); ++pHGameObject)
    {
        HGameObject hGameObject = *pHGameObject;
        GOMan.Release( hGameObject );
    }
    m_gameObjectList.clear();
    
    ParticleEmitterListIterator  pHParticleEmitter;
    for (pHParticleEmitter = m_particleEmitterList.begin(); pHParticleEmitter != m_particleEmitterList.end(); ++pHParticleEmitter)
    {
        HParticleEmitter hParticleEmitter = *pHParticleEmitter;
        Particles.Stop   ( hParticleEmitter );
        Particles.Release( hParticleEmitter );
    }
    m_particleEmitterList.clear();
    

    if ( !m_hEffect.IsNull() )
    {
        EffectMan.Release(m_hEffect);
    }
}


/*
Layer*
Layer::Clone() const
{
    Layer* pLayerClone = new Layer(*this);
    
    // Give it a new name with a random suffix
    char instanceName[MAX_NAME];
    sprintf(instanceName, "%s_%X", m_name.c_str(), (unsigned int)Platform::Random());
    pLayerClone->m_name = string(instanceName);
    
    // Reset state
//    pLayerClone->m_isVisible    = true;
//    pLayerClone->m_fScale       = 1.0;
    
    return pLayerClone;
}
*/


RESULT
Layer::Init( const string& name )
{
    RESULT      rval        = S_OK;
    
    RETAILMSG(ZONE_OBJECT, "Layer[%4d]::Init( %s )", m_ID, name.c_str());
    
    m_name      = name;

    Rectangle screenRect;
    Platform::GetScreenRect( &screenRect );
    m_bounds.SetMin(vec3( 0.0f, 0.0f, 0.0f ));
    m_bounds.SetMax(vec3( screenRect.width, screenRect.height, 0.0f ));


    RETAILMSG(ZONE_LAYER, "Layer[%4d]: \"%-32s\"", m_ID, m_name.c_str());
    
Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Layer::Init( \"%s\" ): failed", name.c_str());
    }
    return rval;
}



RESULT
Layer::AddGameObject( IN const HGameObject hGameObject )
{
    RESULT rval = S_OK;
    
    if ( std::find( m_gameObjectList.begin(), m_gameObjectList.end(), hGameObject ) != m_gameObjectList.end() )
        return E_ALREADY_EXISTS;


    // Push to back so newer items draw on top of older items.
    m_gameObjectList.push_back( hGameObject );

//    CHR(GOMan.AddRef( hGameObject ));

Exit:
    return rval;
}



RESULT
Layer::AddSprite( IN const HSprite hSprite )
{
    RESULT rval = S_OK;

    if ( std::find( m_spriteList.begin(), m_spriteList.end(), hSprite ) != m_spriteList.end() )
        return E_ALREADY_EXISTS;

    // Push to back so newer items draw on top of older items.
    m_spriteList.push_back( hSprite );

//    CHR(SpriteMan.AddRef( hSprite ));

Exit:
    return rval;
}



RESULT
Layer::AddLayer( IN const HLayer hChildLayer )
{
    RESULT rval = S_OK;

    if ( std::find( m_layerList.begin(), m_layerList.end(), hChildLayer ) != m_layerList.end() )
        return E_ALREADY_EXISTS;

     // Push to back so newer items draw on top of older items.
     m_layerList.push_back( hChildLayer );

//    CHR(LayerMan.AddRef( hChildLayer ));

    // Child layers inherit our Effect if they don't already have one.
    HEffect hChildEffect = LayerMan.GetEffect( hChildLayer );
    if ( hChildEffect.IsNull() && !m_hEffect.IsNull() )
    {
        IGNOREHR(LayerMan.SetEffect( hChildLayer, m_hEffect ));
    }

Exit:
    return rval;
}



RESULT
Layer::AddParticleEmitter( IN const HParticleEmitter hParticleEmitter )
{
    RESULT rval = S_OK;

    if ( std::find( m_particleEmitterList.begin(), m_particleEmitterList.end(), hParticleEmitter ) != m_particleEmitterList.end() )
        return E_ALREADY_EXISTS;

    // Push to back so newer items draw on top of older items.
    m_particleEmitterList.push_back( hParticleEmitter );

//    CHR(Particles.AddRef( hParticleEmitter ));

Exit:
    return rval;
}




RESULT
Layer::RemoveGameObject( IN const HGameObject hGameObject )
{
    RESULT rval = S_OK;

    GameObjectListIterator pHGameObject = std::find( m_gameObjectList.begin(), m_gameObjectList.end(), hGameObject );
    if (pHGameObject != m_gameObjectList.end())
    {
        m_gameObjectList.erase( pHGameObject );
        
//        CHR(GOMan.Release( *pHGameObject ));
    }
    else
    {
        rval = E_NOT_FOUND;
    }

Exit:
    return rval;
}



RESULT
Layer::RemoveLayer( IN const HLayer hLayer )
{
    RESULT rval = S_OK;

    LayerListIterator pHLayer = std::find( m_layerList.begin(), m_layerList.end(), hLayer );
    if (pHLayer != m_layerList.end())
    {
        m_layerList.erase( pHLayer );
        
//        CHR(LayerMan.Release( *pHLayer ));
    }
    else
    {
        rval = E_NOT_FOUND;
    }

Exit:
    return rval;
}



RESULT
Layer::RemoveSprite( IN const HSprite hSprite )
{
    RESULT rval = S_OK;

    SpriteListIterator pHSprite = std::find( m_spriteList.begin(), m_spriteList.end(), hSprite );
    if (pHSprite != m_spriteList.end())
    {
        m_spriteList.erase( pHSprite );
        
//        CHR(SpriteMan.Release( *pHSprite ));
    }
    else
    {
        rval = E_NOT_FOUND;
    }

Exit:
    return rval;
}



RESULT
Layer::RemoveParticleEmitter( IN const HParticleEmitter hParticleEmitter )
{
    RESULT rval = S_OK;

    ParticleEmitterListIterator pHParticleEmitter = std::find( m_particleEmitterList.begin(), m_particleEmitterList.end(), hParticleEmitter );
    if (pHParticleEmitter != m_particleEmitterList.end())
    {
        m_particleEmitterList.erase( pHParticleEmitter );
        
//        CHR(Particles.Release( *pHParticleEmitter ));
    }
    else
    {
        rval = E_NOT_FOUND;
    }

Exit:
    return rval;
}



RESULT
Layer::Clear()
{
    RESULT rval = S_OK;
    
    DEBUGMSG(ZONE_LAYER, "Layer::Clear( %4d \"%s\" )", m_ID, m_name.c_str());

/*
    for (GameObjectListIterator pHGameObject = m_gameObjectList.begin(); pHGameObject != m_gameObjectList.end(); ++pHGameObject)
    {
        IGNOREHR(GOMan.Release( *pHGameObject ));
    }

    for (SpriteListIterator pHSprite = m_spriteList.begin(); pHSprite != m_spriteList.end(); ++pHSprite)
    {
        IGNOREHR(SpriteMan.Release( *pHSprite ));
    }
*/
    m_gameObjectList.clear();
    m_spriteList.clear();
///    m_layerList.clear();
    m_particleEmitterList.clear();

Exit:
    return rval;
}



RESULT      
Layer::Show( IN HEffect hEffect, IN HStoryboard hStoryboard )
{
    RESULT rval = S_OK;
    
    DEBUGMSG(ZONE_INFO, "Layer::Show( \"%s\" )", m_name.c_str());
    
    // Don't change our Effect if hEffect is NULL; we might already have an Effect bound.
    if ( !hEffect.IsNull() && m_hEffect != hEffect)
    {
        EffectMan.AddRef( hEffect );
        EffectMan.Release( m_hEffect );
        m_hEffect = hEffect;
    }
    
    if (m_hStoryboard != hStoryboard)
    {
        StoryboardMan.Stop( m_hStoryboard );
        StoryboardMan.AddRef( hStoryboard );
        StoryboardMan.Release( m_hStoryboard );
        m_hStoryboard = hStoryboard;
    }
    
    SetVisible( true );
    
    // Should we show the scene with a transition storyboard?
    if (!hStoryboard.IsNull())
    {
        // Should the storyboard control the Layer the Layer's Effect?
        if (!hEffect.IsNull())
        {
            CHR(StoryboardMan.BindTo( hStoryboard, hEffect ));
        }
        else
        {
            CHR(StoryboardMan.BindTo( hStoryboard, this ));
        }
        
        Callback callback( Layer::OnDoneShowing, this );
        CHR(StoryboardMan.CallbackOnFinished( hStoryboard, callback ));
        CHR(StoryboardMan.Start( hStoryboard ));
    }
    
//    m_isAnimating = true;
    
Exit:    
    
    return rval;
}


RESULT
Layer::Hide( IN HEffect hEffect, IN HStoryboard hStoryboard )
{
    RESULT rval = S_OK;
    
    DEBUGMSG(ZONE_INFO, "Layer::Hide( \"%s\" )", m_name.c_str());
    

    // Don't change our Effect if hEffect is NULL; we might already have an Effect bound.
    if ( !hEffect.IsNull() && m_hEffect != hEffect)
    {
        EffectMan.AddRef( hEffect );
        EffectMan.Release( m_hEffect );
        m_hEffect = hEffect;
    }
    
    if (m_hStoryboard != hStoryboard)
    {
        StoryboardMan.Stop( m_hStoryboard );
        StoryboardMan.AddRef( hStoryboard );
        StoryboardMan.Release( m_hStoryboard );
        m_hStoryboard = hStoryboard;
    }
    
    
    // Should we show the scene with a transition storyboard?
    if (!hStoryboard.IsNull())
    {
        // Should the storyboard control the Layer the Layer's Effect?
        if (!hEffect.IsNull())
        {
            CHR(StoryboardMan.BindTo( hStoryboard, hEffect ));
        }
        else
        {
            CHR(StoryboardMan.BindTo( hStoryboard, this ));
        }
        
        Callback callback( Layer::OnDoneHiding, this );
        CHR(StoryboardMan.CallbackOnFinished( hStoryboard, callback ));
        CHR(StoryboardMan.Start( hStoryboard ));
    }
    
    
//    m_isAnimating = true;
    
Exit:
    
    return rval;
}



void
Layer::OnDoneShowing(void* context)
{
    Layer*  pLayer  = (Layer*)context;
    
//    pLayer->m_isAnimating = false;
    StoryboardMan.Stop( pLayer->m_hStoryboard );
    
    pLayer->SetVisible( true );
    
    RETAILMSG(ZONE_INFO, "Layer::Show( \"%s\" ): DONE", pLayer->GetName().c_str());
}


void
Layer::OnDoneHiding(void* context)
{
    Layer*  pLayer  = (Layer*)context;
    
//    pLayer->m_isAnimating = false;
    StoryboardMan.Stop( pLayer->m_hStoryboard );
    
    pLayer->SetVisible( false );
    
    RETAILMSG(ZONE_INFO, "Layer::Hide( \"%s\" ): DONE", pLayer->GetName().c_str());
}





//
// IDrawable
// 

RESULT  
Layer::Draw( const mat4& matParentWorld )
{
    RESULT  rval = S_OK;
    mat4    world;
    LayerListIterator  ppHLayer;
  
    //
    // Skip drawing Layers w/ zero opacity.
    // Will skip any child layers as well.
    //
    if ( !m_isVisible || Util::CompareFloats(m_fOpacity, 0.0f))
        return rval;


    //
    // Render optional shadow pass.
    //
    if (m_isShadowEnabled)
    {    
        DrawShadowPass( matParentWorld );
    }


    //
    // Apply an Effect, if set.
    // TODO: if SpriteBatching is enabled, this is WASTEFUL.
    // However, we can't know this.
    // Move all batching into a wrapper class on top of IRenderer,
    // and let it avoid needless Effect calls.
    //
    CHR(Renderer.PushEffect( m_hEffect ));

    //
    // Set the Renderer's "global color" so that everything we draw inherits the Layer's opacity.  
    //
    CHR(Renderer.SetGlobalColor( Color(1, 1, 1, m_fOpacity) ));

    CHR(DrawContents( matParentWorld ));

    // Draw child Layers last.
    // Do it after our SpriteBatch so that order is maintained.
    for (ppHLayer = m_layerList.begin(); ppHLayer != m_layerList.end(); )
    {
        HLayer hLayer = *ppHLayer;
        if ( hLayer.IsNull() || hLayer.IsDangling() )
        {
            // Bad handle (or the referenced object has since been deleted).
            // Remove it from our draw list.
            ppHLayer = m_layerList.erase( ppHLayer );
            continue;
        }
        
        // Children should inherit our Effect unless they have one set explicitly.
        HEffect hChildEffect = LayerMan.GetEffect( hLayer );
        if (hChildEffect.IsNull() && !m_hEffect.IsNull())
        {
            LayerMan.SetEffect( hLayer, m_hEffect );
        }
        
        if (FAILED(LayerMan.Draw(hLayer, world )))
        {
            // Layer may have deleted itself; remove from our list.
            ppHLayer = m_layerList.erase( ppHLayer );
        }
        else 
        {
            ++ppHLayer;
        }
    }
    
Exit:    
    CHR(Renderer.SetGlobalColor( Color::White() ));
    IGNOREHR(Renderer.PopEffect());

    return rval;
}



RESULT
Layer::DrawShadowPass( const mat4& matParentWorld )
{
    RESULT  rval = S_OK;
    static HEffect hShadowEffect;

    if (hShadowEffect.IsNull())
    {
        CHR(EffectMan.Get( "DropShadowEffect", &hShadowEffect ));
    }


    if (GetNumSprites() > 0 || GetNumGameObjects() > 0)
    {
        CHR(Renderer.PushEffect( hShadowEffect ));
        IGNOREHR(DrawContents( matParentWorld, true ));
        CHR(Renderer.PopEffect());
    }
    
Exit:
    return rval;
}



RESULT
Layer::DrawContents( const mat4& matParentWorld, bool shadowPass )
{
    RESULT rval = S_OK;
    mat4   world;

    world *= mat4::Scale( m_fScale );
    world *= mat4::RotateX( m_vRotation.x ); 
    world *= mat4::RotateY( m_vRotation.y ); 
    world *= mat4::RotateZ( m_vRotation.z ); 
    world *= mat4::Translate( m_vWorldPosition.x, m_vWorldPosition.y, m_vWorldPosition.z );

    world *= matParentWorld;


    //
    // Render all items in this Layer
    //
    // We need a seperate SpriteBatch per Layer to ensure proper Z-order.
    // Rendering all Layers in a single SpriteBatch could cause Sprites to render out of order (sorted by shader),
    // meaning Sprites could disappear behind their background.
    //
    //

    SpriteListIterator          ppHSprite;
    GameObjectListIterator      ppHGameObject;
    ParticleEmitterListIterator ppHParticleEmitter;

    CHR(SpriteMan.BeginBatch());

    // Draw Sprites first; the Layer probably has a large opaque Sprite for a background.
    for (ppHSprite = m_spriteList.begin(); ppHSprite != m_spriteList.end(); /*++ppHSprite*/)
    {
        HSprite hSprite = *ppHSprite;
        if ( hSprite.IsNull() )
        {
            RETAILMSG(ZONE_WARN, "WARNING: NULL HSprite added to Layer \"%s\"; ignoring.", m_name.c_str());
            ppHSprite = m_spriteList.erase( ppHSprite );
            continue;
        }
        
        if (shadowPass && !SpriteMan.GetShadow( hSprite ))
        {
            ++ppHSprite;
            continue;
        }
        
        if (FAILED(SpriteMan.DrawSprite( hSprite, world )))
        {
            // Sprite may have deleted itself; remove from our list.
            ppHSprite = m_spriteList.erase( ppHSprite );
        }
        else 
        {
            ++ppHSprite;
        }
    }

    // Draw GameObjects on top of any background Sprites.
    for (ppHGameObject = m_gameObjectList.begin(); ppHGameObject != m_gameObjectList.end(); /*++ppHGameObject*/)
    {
        HGameObject hGameObject = *ppHGameObject;
        if( hGameObject.IsNull() )
        {
            RETAILMSG(ZONE_WARN, "WARNING: NULL HGameObject added to Layer \"\"; ignoring.", m_name.c_str());
            ppHGameObject = m_gameObjectList.erase( ppHGameObject );
            continue;
        }

        if (shadowPass && !GOMan.GetShadow( hGameObject ))
        {
            ++ppHGameObject;
            continue;
        }
            
        if (FAILED(GOMan.Draw( hGameObject, world )))
        {
            // GameObject may have deleted itself; remove from our list.
            ppHGameObject = m_gameObjectList.erase( ppHGameObject );
        }
        else 
        {
            ++ppHGameObject;
        }
    }
    
    CHR(SpriteMan.EndBatch());  // TODO: must move batching into Renderer, not Sprites-only.


    // Draw ParticleEmitters on top of everything.
    for (ppHParticleEmitter = m_particleEmitterList.begin(); ppHParticleEmitter != m_particleEmitterList.end(); /*++ppHParticleEmitter*/)
    {
        HParticleEmitter hParticleEmitter = *ppHParticleEmitter;
        if( hParticleEmitter.IsNull() )
        {
            RETAILMSG(ZONE_WARN, "WARNING: NULL HParticleEmitter added to Layer \"\"; ignoring.", m_name.c_str());
            ppHParticleEmitter = m_particleEmitterList.erase( ppHParticleEmitter );
            continue;
        }

        if (FAILED(Particles.Draw( hParticleEmitter, world )))
        {
            // ParticleEmitter may have deleted itself; remove from our list.
            ppHParticleEmitter = m_particleEmitterList.erase( ppHParticleEmitter );
        }
        else 
        {
            ++ppHParticleEmitter;
        }
    }

//    CHR(SpriteMan.EndBatch());
        
Exit:
    return rval;
}




RESULT
Layer::SetEffect( HEffect hEffect )
{
    RESULT rval = S_OK;

    if (m_hEffect != hEffect)
    {
        if (!m_hEffect.IsNull())
            CHR(Effects.Release(m_hEffect));
            
        m_hEffect = hEffect;
        
        if (!m_hEffect.IsNull())
            CHR(Effects.AddRef( m_hEffect ));
            
        
        // Set Effect on child layers.    
        LayerListIterator pHChildLayer;
        for (pHChildLayer = m_layerList.begin(); pHChildLayer != m_layerList.end(); ++pHChildLayer)
        {
            IGNOREHR(LayerMan.SetEffect( *pHChildLayer, m_hEffect ));
        }
    }
    
Exit:
    return rval;
}



RESULT
Layer::SetShadow( bool enabled )
{
    RESULT rval = S_OK;

    m_isShadowEnabled = enabled;
    
    // Set on child layers.
    LayerListIterator pHChildLayer;
    for (pHChildLayer = m_layerList.begin(); pHChildLayer != m_layerList.end(); ++pHChildLayer)
    {
        IGNOREHR(LayerMan.SetShadow( *pHChildLayer, enabled ));
    }
    
Exit:
    return rval;
}




IProperty*
Layer::GetProperty( const string& propertyName ) const
{
    return s_properties.Get( this, propertyName );
}




} // END namespace Z


