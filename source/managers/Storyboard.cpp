#include "StoryboardManager.hpp"
#include "Storyboard.hpp"
#include "GameObjectManager.hpp"
#include "EffectManager.hpp"
#include "LayerManager.hpp"


namespace Z
{



//=============================================================================
//
// Multiple Animations are grouped together in an Storyboard,
// which controls all animated properties of a single object.
//
//=============================================================================

Storyboard::Storyboard() :
    m_interpolatorType(INTERPOLATOR_TYPE_UNKNOWN),
    m_isBoundToTarget(false),
    m_numAnimations(0),
    m_numFinishedAnimations(0),
    m_pAnimationBindings(NULL),
    m_pCallbackOnFinished(NULL),
    m_pObject(NULL),
    m_startTimeMS(0),
    m_durationMS(0),
    m_isStarted(false),
    m_isPaused(false),
    m_autoRepeat(false),
    m_autoReverse(false),
    m_releaseTargetOnFinish(true),
    m_deleteOnFinish(true),
    m_relativeToCurrentState(false)
{
}


Storyboard::~Storyboard()
{
    DEBUGMSG(ZONE_STORYBOARD, "\t~Storyboard( %4d, \"%s\" 0x%x, numTargets %d = 0x%x)", m_ID, m_name.c_str(), (UINT32)m_hGameObject, m_numAnimations, m_pAnimationBindings);

    StoryboardMan.Stop( this );
    UnBind();
  
    SAFE_ARRAY_DELETE(m_pAnimationBindings);
}


// TODO: implement the copy ctor and assignment operator like the others.
Storyboard*
Storyboard::Clone() const
{
    RESULT rval = S_OK;
    
    Storyboard* pClone = new Storyboard(*this);

    // Duplicate m_hGameObject, if non-NULL, so the refcount will be increased.
    // We're being paranoid; Storyboards should only be cloned from "templates,"
    // which should NOT be bound to an actual GameObject.
    if (!m_hGameObject.IsNull())
    {
        pClone->m_hGameObject = m_hGameObject;
        CHR( GOMan.AddRef( m_hGameObject ));
    }
    
    // DEEP COPY: m_pAnimationBindings
    pClone->m_pAnimationBindings = new AnimationBinding[m_numAnimations];
    DEBUGCHK(pClone->m_pAnimationBindings);

    
    // DEEP COPY: duplicate the AnimationBindings
    for (int i = 0; i < m_numAnimations; ++i)
    {
        // Clone the AnimationBinding.
        pClone->m_pAnimationBindings[i] = m_pAnimationBindings[i];
    
        // Clone the Animation. 
        AnimationBinding* pClonedAnimationBinding = &pClone->m_pAnimationBindings[i];
        if (FAILED(AnimationMan.GetCopy( pClonedAnimationBinding->m_animationName, &pClonedAnimationBinding->m_hAnimation )))    // Overwrite original handle with new handle (copy of the Animation)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: StoryBoard::Clone( \"%s\" ): failed to copy Animation track %d \"%s\".", 
                m_name.c_str(), i, pClonedAnimationBinding->m_animationName.c_str());
                
//            DEBUGCHK(0);
            
            continue;
        }
    }
    

    // DEEP COPY: m_pCallbackOnFinished
    if (m_pCallbackOnFinished)
    {
        pClone->m_pCallbackOnFinished = m_pCallbackOnFinished->Clone();
    }
    
    
    // Give it a new name with a random suffix
    char instanceName[MAX_PATH];
    sprintf(instanceName, "%s_%X", m_name.c_str(), (unsigned int)Platform::Random());
    pClone->m_name = string(instanceName);
 
Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Storyboard::Clone() failed! rval = 0x%x", rval);
    }
    
    return pClone;
}



RESULT 
Storyboard::Init( IN const string& name, IN HAnimation* pHAnimations, UINT8 numAnimations, bool autoRepeat, bool autoReverse, bool releaseTargetOnFinish, bool deleteOnFinish, bool isRelative )
{
    RESULT rval = S_OK;
    
    if (!pHAnimations || !numAnimations)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Storyboard::Init(): must pass pHAnimations, numAnimations.");
        rval = E_INVALID_DATA;
        goto Exit;
    }
    

    if (name == "")
    {
        char randomName[MAX_NAME];
        sprintf(randomName, "Storyboard_%X", (unsigned int)Platform::Random());
        m_name = randomName;
    }
    else 
    {
        m_name = name;
    }

    
    
    m_autoRepeat                = autoRepeat;
    m_autoReverse               = autoReverse;
    m_releaseTargetOnFinish     = releaseTargetOnFinish;
    m_deleteOnFinish            = deleteOnFinish;
    m_relativeToCurrentState    = isRelative;
    

    m_numAnimations             = numAnimations;
    m_pAnimationBindings        = new AnimationBinding[ m_numAnimations ];
    if (!m_pAnimationBindings)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Storyboard::Init(): out of memory");
        rval = E_OUTOFMEMORY;
        goto Exit;
    }

    
    for (int i = 0; i < m_numAnimations; ++i)
    {
        m_durationMS = MAX(m_durationMS, AnimationMan.GetDurationMS( *pHAnimations ));

        //
        // Create the AnimationBinding
        //
        AnimationBinding *pAnimationBinding = &m_pAnimationBindings[i];
        
        string name;
        CHR(AnimationMan.GetName( *pHAnimations, &name ));
        rval = CreateAnimationBinding( name, pAnimationBinding );
        if (FAILED(rval))
        {
            RETAILMSG(ZONE_ERROR, "ERROR: Storyboard::Init( %s ): failed to create AnimationBinding", m_name.c_str());
            
            // Continue loading other Animations rather than aborting.
            continue;
        }
        
        pHAnimations++;
        
        // Override the Animation properties with our own.
        // Redundant but err on the side of consistency.
        AnimationMan.SetAutoRepeat            ( pAnimationBinding->m_hAnimation, m_autoRepeat             );
        AnimationMan.SetAutoReverse           ( pAnimationBinding->m_hAnimation, m_autoReverse            );
        AnimationMan.SetDeleteOnFinish        ( pAnimationBinding->m_hAnimation, m_deleteOnFinish         );
//        AnimationMan.SetRelativeToCurrentState( pAnimationBinding->m_hAnimation, m_relativeToCurrentState );

        
        DEBUGMSG(ZONE_STORYBOARD | ZONE_VERBOSE, "Storyboard [%s] += [%s]", 
                 m_name.c_str(), pAnimationBinding->m_animationName.c_str() );

    }
    

    RETAILMSG(ZONE_STORYBOARD, "Storyboard[%4d]: \"%-32s\" %d animation tracks %d msec",
          m_ID, m_name.c_str(), m_numAnimations, m_durationMS);
    

Exit:
    return rval;
}



RESULT
Storyboard::Init( IN const string& name, IN const Settings* pSettings, IN const string& settingsPath )
{
    RESULT rval = S_OK;
    char   path[MAX_PATH];
    string interpolator;
 
    m_name  = name;
    
    if (!pSettings)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Storyboard::Init(): NULL settings");
        rval = E_INVALID_ARG;
        goto Exit;
    }


    m_autoRepeat                = pSettings->GetBool( settingsPath + ".AutoRepeat",         false );
    m_autoReverse               = pSettings->GetBool( settingsPath + ".AutoReverse",        false );
    m_releaseTargetOnFinish     = pSettings->GetBool( settingsPath + ".ReleaseOnFinish",    false );
    m_deleteOnFinish            = pSettings->GetBool( settingsPath + ".DeleteOnFinish",     false );
    m_relativeToCurrentState    = pSettings->GetBool( settingsPath + ".RelativeToObject",   false );


    // TODO: this needs to be a lookup table in Animation.
    interpolator = pSettings->GetString( settingsPath + ".Interpolator",   "" );
    if ("Linear" == interpolator)
    {
        m_interpolatorType  = INTERPOLATOR_TYPE_LINEAR;
    }
    else if ("QuadraticEaseIn" == interpolator)
    {
        m_interpolatorType  = INTERPOLATOR_TYPE_QUADRATIC_IN;
    }
    else if ("QuadraticEaseOut" == interpolator)
    {
        m_interpolatorType  = INTERPOLATOR_TYPE_QUADRATIC_OUT;
    }
    else if ("QuadraticEaseInOut" == interpolator)
    {
        m_interpolatorType  = INTERPOLATOR_TYPE_QUADRATIC_INOUT;
    }
    else if ("ElasticIn" == interpolator)
    {
        m_interpolatorType  = INTERPOLATOR_TYPE_ELASTIC_IN;
    }


    m_numAnimations = pSettings->GetInt( settingsPath + ".NumAnimations" );
    if (0 == m_numAnimations)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Storyboard::Init(): zero Animations defined; check XML");
        rval = E_INVALID_ARG;
        goto Exit;
    }

    m_pAnimationBindings = new AnimationBinding[ m_numAnimations ];
    if (!m_pAnimationBindings)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Storyboard::Init(): out of memory");
        rval = E_OUTOFMEMORY;
        goto Exit;
    }
    
    for (int i = 0; i < m_numAnimations; ++i)
    {
        //
        // Create the Animation
        //
        sprintf(path, "%s/Animation%d", settingsPath.c_str(), i);
        Animation *pAnimation = NULL;
        
        RESULT rval = AnimationMan.CreateAnimation( pSettings, path, &pAnimation );
        if (!pAnimation || FAILED(rval))
        {
            RETAILMSG(ZONE_ERROR, "ERROR: Storyboard::Init( %s ): failed to create Animation", path);
            
            // Continue loading other Animations rather than aborting.
            continue;
        }
        m_durationMS = MAX(m_durationMS, pAnimation->GetDurationMS());

        
        // TODO: do this before creating the animation, so that
        // the Animation can override settings inherited from the Storyboard.
        pAnimation->SetAutoRepeat            ( m_autoRepeat             );
        pAnimation->SetAutoReverse           ( m_autoReverse            );
        pAnimation->SetDeleteOnFinish        ( m_deleteOnFinish         );
//        pAnimation->SetRelativeToCurrentState( m_relativeToCurrentState );
        
        if (m_interpolatorType != INTERPOLATOR_TYPE_UNKNOWN)
        {
            pAnimation->SetInterpolatorType( m_interpolatorType );
        }
        
//        DEBUGMSG(ZONE_STORYBOARD, "Created Animation [%s]", pAnimation->GetName().c_str());
        CHR(AnimationMan.Add(pAnimation->GetName(), pAnimation));
        
        //
        // Create the AnimationBinding
        //
        AnimationBinding *pAnimationBinding = &m_pAnimationBindings[i];
        
        string animationName         = pAnimation->GetName();
        
        rval = CreateAnimationBinding( animationName, pAnimationBinding );
        if (FAILED(rval))
        {
            RETAILMSG(ZONE_ERROR, "ERROR: Storyboard::Init( %s ): failed to create AnimationBinding", path);
            // Continue loading other Animations rather than aborting.
            continue;
        }
        
        
        DEBUGMSG(ZONE_STORYBOARD | ZONE_VERBOSE, "Storyboard [%s] += [%s]", 
                 m_name.c_str(), pAnimationBinding->m_animationName.c_str() );

    }
    

    RETAILMSG(ZONE_STORYBOARD, "Storyboard[%4d]: \"%-32s\" %d animation tracks, %d msec",
          m_ID, m_name.c_str(), m_numAnimations, m_durationMS);
    

Exit:
    return rval;
}



RESULT
Storyboard::Update( UINT64 elapsedMS )
{
    RESULT rval = S_OK;
    
    if (!m_isStarted || m_isPaused)
        return S_OK;
    
    //DEBUGMSG(ZONE_STORYBOARD | ZONE_VERBOSE, "Storyboard::Update( \"%s\" )", m_name.c_str());
    
    // Update each Animation
    for (int i = 0; i < m_numAnimations; ++i)
    {
        AnimationBinding* pAnimationBinding = &m_pAnimationBindings[i];
        DEBUGCHK(pAnimationBinding);
        
        if (pAnimationBinding->m_hasFinished)
            continue;
        
        if (FAILED(AnimationMan.Update( pAnimationBinding->m_hAnimation, elapsedMS )))
        {
            // If an Animation failed to Update, it has probably deleted itself after finishing.
            pAnimationBinding->m_hasFinished = true;
            m_numFinishedAnimations++;
        }
    }

    // If all Animations are finished, let StoryboardManager know we're done ( and may be stopped / deleted ).
    if (m_numFinishedAnimations == m_numAnimations)
    {
        DEBUGMSG(ZONE_STORYBOARD, "Storyboard::Update( \"%s\" ): END - all Animations done", m_name.c_str());
        rval = E_NOTHING_TO_DO;
    }
    
Exit:
    return rval;
}




RESULT
Storyboard::BindTo( HGameObject hGameObject )       // TODO: bind to multiple GOs?
{
    RESULT rval = S_OK;
    
    DEBUGMSG(ZONE_STORYBOARD, "BIND Storyboard \"%s\" -> GO \"%s\"", m_name.c_str(), hGameObject.GetName().c_str());

    if (IsStarted() || IsPaused())
    {
        StoryboardMan.Stop( this );
    }

    // Take a reference to the GameObject.
    CHR(GOMan.AddRef( hGameObject ));
    if (!m_hGameObject.IsNull())
    {
        CHR(GOMan.Release( m_hGameObject ));
    }
    m_hGameObject = hGameObject;


    // Bind each Animation track to the GameObject.
    for (int i = 0; i < m_numAnimations; ++i)
    {
        AnimationBinding*   pAnimationBinding   = &m_pAnimationBindings[i];
        IProperty*          pProperty           = hGameObject.GetProperty( pAnimationBinding->m_propertyName ); // TODO: should this NOT allocate a new IProperty?  Great LEAK potential!

        if (!pProperty)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: Storyboard::BindTo( \"%s\" ): GameObject \"%s\" [%4d] does not expose Property \"%s\"",
                m_name.c_str(), hGameObject.GetName().c_str(), hGameObject.GetID(), pAnimationBinding->m_propertyName.c_str());
            
            //DEBUGCHK(0);
            
            continue;
        }

        CHR(AnimationMan.BindTo( pAnimationBinding->m_hAnimation, *pProperty ));
        pAnimationBinding->m_isBound = true;
        
        SAFE_DELETE(pProperty);
    }
        
    m_isBoundToTarget = true;
    
Exit:
    return rval;
}



RESULT
Storyboard::BindTo( HEffect hEffect )
{
    RESULT rval = S_OK;

    DEBUGMSG(ZONE_STORYBOARD, "BIND Storyboard \"%s\" -> Effect \"%s\"", m_name.c_str(), hEffect.GetName().c_str());
    
    if (IsStarted() || IsPaused())
    {
        StoryboardMan.Stop( this );
    }

    // Take a reference to the Effect.
    CHR(EffectMan.AddRef( hEffect ));
    if (!m_hEffect.IsNull())
    {
        CHR(EffectMan.Release( m_hEffect ));
    }
    m_hEffect = hEffect;
    

    // Bind each Animation track to the Effect.
    for (int i = 0; i < m_numAnimations; ++i)
    {
        AnimationBinding*   pAnimationBinding   = &m_pAnimationBindings[i];
        IProperty*          pProperty           = hEffect.GetProperty( pAnimationBinding->m_propertyName );

        if (!pProperty)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: Storyboard::BindTo( \"%s\" ): Effect \"%s\" [%4d] does not expose Property \"%s\"",
                m_name.c_str(), hEffect.GetName().c_str(), hEffect.GetID(), pAnimationBinding->m_propertyName.c_str());
                
//            continue;
            rval = E_INVALID_OPERATION;
            goto Exit;
        }

        CHR(AnimationMan.BindTo( pAnimationBinding->m_hAnimation, *pProperty ));
        pAnimationBinding->m_isBound = true;

        SAFE_DELETE(pProperty);
    }
    
    m_isBoundToTarget = true;
    
Exit:
    return rval;
}



RESULT
Storyboard::BindTo( HLayer hLayer )
{
    RESULT rval = S_OK;
    
    DEBUGMSG(ZONE_STORYBOARD, "BIND Storyboard \"%s\" -> Layer \"%s\"", m_name.c_str(), hLayer.GetName().c_str());
    
    if (IsStarted() || IsPaused())
    {
        StoryboardMan.Stop( this );
    }

    // Take a reference to the Layer.
    CHR(LayerMan.AddRef( hLayer ));
    if (!m_hLayer.IsNull())
    {
        CHR(LayerMan.Release( m_hLayer ));
    }
    m_hLayer = hLayer;
    

    // Bind each Animation track to the Layer.
    for (int i = 0; i < m_numAnimations; ++i)
    {
        AnimationBinding*   pAnimationBinding   = &m_pAnimationBindings[i];
        IProperty*          pProperty           = hLayer.GetProperty( pAnimationBinding->m_propertyName );

        if (!pProperty)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: Storyboard::BindTo( \"%s\" ): Layer \"%s\" [%4d] does not expose Property \"%s\"",
                m_name.c_str(), hLayer.GetName().c_str(), hLayer.GetID(), pAnimationBinding->m_propertyName.c_str());
                
//            continue;
            rval = E_INVALID_OPERATION;
            goto Exit;
        }

        CHR(AnimationMan.BindTo( pAnimationBinding->m_hAnimation, *pProperty ));
        pAnimationBinding->m_isBound = true;

        SAFE_DELETE(pProperty);
    }
    
    m_isBoundToTarget = true;
    
Exit:
    return rval;
}



RESULT
Storyboard::BindTo( HSprite hSprite )
{
    RESULT rval = S_OK;
    
    DEBUGMSG(ZONE_STORYBOARD, "BIND Storyboard \"%s\" -> Sprite \"%s\"", m_name.c_str(), hSprite.GetName().c_str());

    if (IsStarted() || IsPaused())
    {
        StoryboardMan.Stop( this );
    }
    
    // Take a reference to the Sprite.
    CHR(SpriteMan.AddRef( hSprite ));
    if (!m_hSprite.IsNull())
    {
        CHR(SpriteMan.Release( m_hSprite ));
    }
    m_hSprite = hSprite;

    
    // Bind each Animation track to the Sprite.
    for (int i = 0; i < m_numAnimations; ++i)
    {
        AnimationBinding*   pAnimationBinding   = &m_pAnimationBindings[i];
        IProperty*          pProperty           = hSprite.GetProperty( pAnimationBinding->m_propertyName );
        
        if (!pProperty)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: Storyboard::BindTo( \"%s\" ): Sprite \"%s\" [%4d] does not expose Property \"%s\"",
                      m_name.c_str(), hSprite.GetName().c_str(), hSprite.GetID(), pAnimationBinding->m_propertyName.c_str());
            
//            continue;
            rval = E_INVALID_OPERATION;
            goto Exit;
        }
        
        CHR(AnimationMan.BindTo( pAnimationBinding->m_hAnimation, *pProperty ));
        pAnimationBinding->m_isBound = true;
        
        SAFE_DELETE(pProperty);
    }
    
    m_isBoundToTarget = true;
    
Exit:
    return rval;
}



RESULT
Storyboard::BindTo( HParticleEmitter hParticleEmitter )
{
    RESULT rval = S_OK;
    
    
    if (hParticleEmitter.IsNull() || hParticleEmitter.IsDangling() || hParticleEmitter.IsDeleted())
    {
        return E_BAD_HANDLE;
    }
    
    
    DEBUGMSG(ZONE_STORYBOARD, "BIND Storyboard \"%s\" -> ParticleEmitter \"%s\"", m_name.c_str(), hParticleEmitter.GetName().c_str());

    if (IsStarted() || IsPaused())
    {
        StoryboardMan.Stop( this );
    }
    
    // Take a reference to the Emitter.
    CHR(ParticleMan.AddRef( hParticleEmitter ));
    if (!m_hParticleEmitter.IsNull())
    {
        CHR(ParticleMan.Release( m_hParticleEmitter ));
    }
    m_hParticleEmitter = hParticleEmitter;

    
    // Bind each Animation track to the ParticleEmitter.
    for (int i = 0; i < m_numAnimations; ++i)
    {
        AnimationBinding*   pAnimationBinding   = &m_pAnimationBindings[i];
        IProperty*          pProperty           = hParticleEmitter.GetProperty( pAnimationBinding->m_propertyName );
        
        if (!pProperty)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: Storyboard::BindTo( \"%s\" ): ParticleEmitter \"%s\" [%4d] does not expose Property \"%s\"",
                      m_name.c_str(), hParticleEmitter.GetName().c_str(), hParticleEmitter.GetID(), pAnimationBinding->m_propertyName.c_str());
            
//            continue;
            rval = E_INVALID_OPERATION;
            goto Exit;
        }
        
        CHR(AnimationMan.BindTo( pAnimationBinding->m_hAnimation, *pProperty ));
        pAnimationBinding->m_isBound = true;
        
        SAFE_DELETE(pProperty);
    }
    
    m_isBoundToTarget = true;
    
Exit:
    return rval;
}




RESULT
Storyboard::BindTo( IProperty* pProperty )
{
    RESULT rval = S_OK;
    AnimationBinding* pAnimationBinding = &m_pAnimationBindings[0];
    
    if (!pProperty)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Storyboard::BindTo(): pProperty is NULL.");
        DEBUGCHK(0);
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    if (m_numAnimations > 1)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Storyboard::BindTo( IProperty ): Storyboard has more than one Animation, which to choose?.");
        DEBUGCHK(0);
        rval = E_INVALID_OPERATION;
        goto Exit;
    }

    DEBUGMSG(ZONE_STORYBOARD, "BIND Storyboard \"%s\" -> IProperty \"%s\"", m_name.c_str(), pAnimationBinding->m_propertyName.c_str());

    if (IsStarted() || IsPaused())
    {
        StoryboardMan.Stop( this );
    }

    CHR(AnimationMan.BindTo( pAnimationBinding->m_hAnimation, *pProperty ));
    pAnimationBinding->m_isBound = true;

    m_isBoundToTarget = true;
    
Exit:
    return rval;
}



RESULT
Storyboard::BindTo( Object* pObject )
{
    RESULT rval = S_OK;
    
    if (!pObject)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Storyboard::BindTo(): pObject is NULL.");
        DEBUGCHK(0);
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    DEBUGMSG(ZONE_STORYBOARD, "BIND Storyboard \"%s\" -> Object \"%s\"", m_name.c_str(), pObject->GetName().c_str());

    if (IsStarted() || IsPaused())
    {
        StoryboardMan.Stop( this );
    }

    pObject->AddRef();
    if (m_pObject)
    {
        m_pObject->Release();
    }
    m_pObject = pObject;


    // Bind each Animation track to the Object.
    for (int i = 0; i < m_numAnimations; ++i)
    {
        AnimationBinding*   pAnimationBinding   = &m_pAnimationBindings[i];
        IProperty*          pProperty           = m_pObject->GetProperty( pAnimationBinding->m_propertyName );

        if (!pProperty)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: Storyboard::BindTo( \"%s\" ): Object \"%s\" [%4d] does not expose Property \"%s\"",
                m_name.c_str(), m_pObject->GetName().c_str(), m_pObject->GetID(), pAnimationBinding->m_propertyName.c_str());
                
//            continue;
            rval = E_INVALID_OPERATION;
            goto Exit;
        }

        CHR(AnimationMan.BindTo( pAnimationBinding->m_hAnimation, *pProperty ));
        pAnimationBinding->m_isBound = true;

        SAFE_DELETE(pProperty);
    }
    
    m_isBoundToTarget = true;
    
Exit:
    return rval;
}



RESULT
Storyboard::UnBind( )
{
    RESULT rval = S_OK;
    
    if (!m_isBoundToTarget)
        return S_OK;
        
    
    // Un-bind each Animation track from the target.
    // Decrements the refcount once for each Animation.
    for (int i = 0; i < m_numAnimations; ++i)
    {
        AnimationBinding*   pAnimationBinding   = &m_pAnimationBindings[i];

        IGNOREHR(AnimationMan.BindTo( pAnimationBinding->m_hAnimation, NULL ));

        pAnimationBinding->m_isBound = false;
    }
    
    //
    // Release the target object.
    //
    SAFE_RELEASE(m_pObject);
    GOMan.Release       ( m_hGameObject      ); m_hGameObject       = HGameObject::NullHandle();
    EffectMan.Release   ( m_hEffect          ); m_hEffect           = HEffect::NullHandle();
    LayerMan.Release    ( m_hLayer           ); m_hLayer            = HLayer::NullHandle();
    SpriteMan.Release   ( m_hSprite          ); m_hSprite           = HSprite::NullHandle();
    ParticleMan.Release ( m_hParticleEmitter ); m_hParticleEmitter  = HParticleEmitter::NullHandle();


Exit:
    m_isBoundToTarget = false;

    return rval;
}



RESULT
Storyboard::CallbackOnFinished( const ICallback& callback )
{
    RESULT rval = S_OK;
    
    if (callback.IsNull())
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Storyboard::CallbackOnFinished( \"%s\", 0x%x ): Callback is NULL",
                  m_name.c_str(), (UINT32)&callback);
        
        SAFE_DELETE(m_pCallbackOnFinished);
        
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    SAFE_DELETE(m_pCallbackOnFinished);
    m_pCallbackOnFinished = callback.Clone();
    
Exit:
    return rval;
}





RESULT
Storyboard::Start( )
{
    RESULT rval = S_OK;

    DEBUGMSG(ZONE_STORYBOARD, "START Storyboard \"%s\"", m_name.c_str());

    m_isStarted             = true;
    m_isPaused              = false;
    m_numFinishedAnimations = 0;

    // Start each Animation
    for (int i = 0; i < m_numAnimations; ++i)
    {
        AnimationBinding* pAnimation = &m_pAnimationBindings[i];
        DEBUGCHK(pAnimation);
        
        pAnimation->m_hasFinished = false;
        
        CHR(AnimationMan.Start( pAnimation->m_hAnimation ));
    }
    
Exit:
    return rval;
}



RESULT
Storyboard::Stop( )
{
    RESULT rval = S_OK;

    DEBUGMSG(ZONE_STORYBOARD, "STOP Storyboard \"%s\"", m_name.c_str());


    if (!m_isStarted && !m_isPaused)
    {
        RETAILMSG(ZONE_WARN, "WARNING: Storyboard \"%s\" already stopped.", m_name.c_str());
        return E_INVALID_OPERATION;
    }


    m_isStarted             = false;
    m_isPaused              = false;
    m_numFinishedAnimations = 0;
    
    // Stop each Animation
    for (int i = 0; i < m_numAnimations; ++i)
    {
        AnimationBinding* pTarget = &m_pAnimationBindings[i];
        DEBUGCHK(pTarget);
        
        IGNOREHR(AnimationMan.Stop( pTarget->m_hAnimation ));
    }

    if (m_releaseTargetOnFinish)
    {
        CHR(UnBind());
    }

    if (m_pCallbackOnFinished)
    {
        m_pCallbackOnFinished->Invoke();
    }

    if (m_deleteOnFinish && m_refCount > 0)
    {
        DEBUGMSG(ZONE_STORYBOARD, "Storyboard::Stop(): \"%s\" DELETE on finish.", m_name.c_str());
        StoryboardMan.ReleaseOnNextFrame( this );
    }

Exit:
    return rval;
}



RESULT
Storyboard::Pause( )
{
    RESULT rval = S_OK;
    
    if (!m_isStarted)
        return E_INVALID_OPERATION;
    
    DEBUGMSG(ZONE_STORYBOARD, "PAUSE Storyboard \"%s\"", m_name.c_str());

    m_isPaused  = false;
    
    // Pause each Animation
    for (int i = 0; i < m_numAnimations; ++i)
    {
        AnimationBinding* pTarget = &m_pAnimationBindings[i];
        DEBUGCHK(pTarget);
        
        CHR(AnimationMan.Pause( pTarget->m_hAnimation ));
    }

Exit:
    return rval;
}



RESULT
Storyboard::CreateAnimationBinding( IN const string& animationName, INOUT AnimationBinding* pAnimationBinding )
{
    RESULT          rval            = S_OK;
    HAnimation      hAnimation;
    
    if ("" == animationName || !pAnimationBinding)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Storyboard::CreateAnimationBinding(): invalid arg");
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    //
    // Get handle to the Animation
    //
    CHR(AnimationMan.GetCopy( animationName, &hAnimation  ));

    pAnimationBinding->m_hAnimation      = hAnimation;
    pAnimationBinding->m_animationName   = animationName;
    pAnimationBinding->m_propertyType    = AnimationMan.GetPropertyType( hAnimation );
    pAnimationBinding->m_propertyName    = AnimationMan.GetPropertyName( hAnimation );
    
Exit:
    if (FAILED(rval))
        RETAILMSG(ZONE_ERROR, "ERROR: Storyboard::CreateAnimationBinding() failed.");
    
    
    return rval;
}
    
    
/*    
const StoryboardInfo&
Storyboard::GetInfo()
{
    StoryboardInfo info;

    return info;
}
*/

} // END namespace Z



