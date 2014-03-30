#include "SkyState.hpp"
#include "Log.hpp"
#include "Engine.hpp"
#include "GameState.hpp"


namespace Z
{



//
// This StateMachine controls the game sky: day/night transitions, stars, clouds, etc.
//

static HLayer           hRootLayer;
static HLayer           hSkyLayer;
static HLayer           hSunBeamsLayer;
static HLayer           hPlayScreenColumn;
static HLayer           hPlayScreenBackground;
static HLayer           hPlayScreenForeground;
static HTexture         hSkyTexture;
static HSprite          hSkySprite;
static HEffect          hGradientEffect;
static HParticleEmitter hCloudEmitter;
//static HParticleEmitter hStarfieldEmitter;
static HStoryboard      hGradientEffectStoryboard;
static HStoryboard      hFadeOutStoryboard;
static HStoryboard      hFadeInStoryboard;
static HStoryboard      hHideBackgroundStoryboard;
static HStoryboard      hRevealBackgroundStoryboard;



//Add new states here
enum StateName {
    STATE_Initialize,
    STATE_Sunrise,
    STATE_Morning,
    STATE_Afternoon,
    STATE_Evening,
    STATE_Sunset,
    STATE_Night,
    STATE_Idle
};


//Add new substates here
enum SubstateName {
	//empty
};



SkyState::SkyState( HGameObject &hGameObject ) :
    StateMachine( hGameObject )
{
}

SkyState::~SkyState( void )
{
}


void SkyState::SkyDoneFadingOutCallback( void* context )
{
    LayerMan.RemoveFromLayer( hSkyLayer, hSkySprite );
    hSkySprite.Release();
    hSkyTexture.Release();

    Rectangle skyRect;
    Platform::GetScreenRect(&skyRect);

    Textures.CreateFromFile( Util::ResolutionSpecificFilename(g_pLevel->backgroundFilename), &hSkyTexture );
    Sprites.CreateFromTexture( "", hSkyTexture, skyRect, &hSkySprite );
    LayerMan.AddToLayer( hSkyLayer, hSkySprite );
    Sprites.SetRotation( hSkySprite, vec3(0, -90, 0) );
    Storyboards.BindTo( hRevealBackgroundStoryboard, hSkySprite );
    Storyboards.Start( hRevealBackgroundStoryboard );
}



bool SkyState::States( State_Machine_Event event, MSG_Object * msg, int state, int substate )
{
BeginStateMachine

	//Global message responses
	OnMsg( MSG_Reset )
		ResetStateMachine();
        RETAILMSG(ZONE_STATEMACHINE, "SkyState: MSG_Reset");

    OnMsg( MSG_LevelUp )
        // Fade out old background, fade in new background
        Callback callback( SkyState::SkyDoneFadingOutCallback, (void*)true );
        Storyboards.BindTo( hHideBackgroundStoryboard, hSkySprite );
        Storyboards.CallbackOnFinished( hHideBackgroundStoryboard, callback );
        Storyboards.Start( hHideBackgroundStoryboard );

    OnMsg( MSG_NewGame )
        SkyDoneFadingOutCallback(NULL);
    
    

    ///////////////////////////////////////////////////////////////
	DeclareState( STATE_Initialize )

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "SkyState: STATE_Initialize");

            // Leak leak
            LayerMan.Get        ( "RootLayer",      &hRootLayer         );
            LayerMan.Get        ( "SkyLayer",       &hSkyLayer          );
            LayerMan.Get        ( "SunBeamsLayer",  &hSunBeamsLayer     );

            LayerMan.Get        ( "PlayScreenColumn",      &hPlayScreenColumn    );
            LayerMan.Get        ( "PlayScreenForeground",  &hPlayScreenForeground);
            LayerMan.Get        ( "PlayScreenBackground",  &hPlayScreenBackground);

//            Particles.GetCopy   ( "Starfield",      &hStarfieldEmitter  );
            Particles.GetCopy   ( "Clouds",         &hCloudEmitter      );
            Storyboards.GetCopy ( "FadeOutSky",     &hFadeOutStoryboard );
            Storyboards.GetCopy ( "FadeInSky",      &hFadeInStoryboard  );
            Storyboards.GetCopy ( "FadeOutBackground",  &hHideBackgroundStoryboard );
            Storyboards.GetCopy ( "FadeInBackground",   &hRevealBackgroundStoryboard  );

    
    
    
            // Load the background sky sprite
            // TODO: this should be done at level up and exported to SkyState so SkyState can tint it.
            // Or, just handle level-up messages here to keep Sky all in one place.
            hSkyTexture.Release();
            RESULT rval = Textures.CreateFromFile( Util::ResolutionSpecificFilename(g_pLevel->backgroundFilename), &hSkyTexture );
            if (FAILED(rval))
            {
                Textures.Get( "SkyTextureEvening", &hSkyTexture );
            }

            Rectangle skyRect;
            Platform::GetScreenRect(&skyRect);

            Sprites.CreateFromTexture( "SkySprite", hSkyTexture, skyRect, &hSkySprite );
            LayerMan.AddToLayer( hSkyLayer, hSkySprite );

			ChangeState( STATE_Afternoon );
	


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Sunrise )

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "SkyState: STATE_Sunrise");

            GenerateSunriseSky();
            
            float duration = Storyboards.GetDurationMS( hGradientEffectStoryboard ) / 1000.0f;
            if (duration > 0)
            {
                ChangeStateDelayed( duration, STATE_Morning );
            }
            else
            {
                ChangeStateDelayed( 60.0f, STATE_Morning );
            }
            
            
//        OnFrameUpdate
//            DebugRender.Text( "STATE_Sunrise" );



	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Morning )

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "SkyState: STATE_Morning");

            GenerateMorningSky();
            
            float duration = Storyboards.GetDurationMS( hGradientEffectStoryboard ) / 1000.0f;
            if (duration > 0)
            {
                ChangeStateDelayed( duration, STATE_Afternoon );
            }
            else
            {
                ChangeStateDelayed( 60.0f, STATE_Afternoon );
            }
            
            
//        OnFrameUpdate
//            DebugRender.Text( "STATE_Morning" );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Afternoon )

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "SkyState: STATE_Afternoon");

            GenerateAfternoonSky();
            
            float duration = Storyboards.GetDurationMS( hGradientEffectStoryboard ) / 1000.0f;
            if (duration > 0)
            {
                ChangeStateDelayed( duration, STATE_Evening );
            }
            else
            {
                ChangeStateDelayed( 60.0f, STATE_Evening );
            }

//        OnFrameUpdate
//            DebugRender.Text( "STATE_Afternoon" );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Evening )

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "SkyState: STATE_Evening");

            GenerateEveningSky();
            
            float duration = Storyboards.GetDurationMS( hGradientEffectStoryboard ) / 1000.0f;
            if (duration > 0)
            {
                ChangeStateDelayed( duration, STATE_Sunset );
            }
            else
            {
                ChangeStateDelayed( 60.0f, STATE_Sunset );
            }


//        OnFrameUpdate
//            DebugRender.Text( "STATE_Evening" );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Sunset )

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "SkyState: STATE_Sunset");

            GenerateSunsetSky();
            
            float duration = Storyboards.GetDurationMS( hGradientEffectStoryboard ) / 1000.0f;
            if (duration > 0)
            {
                ChangeStateDelayed( duration, STATE_Night );
            }
            else
            {
                ChangeStateDelayed( 60.0f, STATE_Night );
            }


//        OnFrameUpdate
//            DebugRender.Text( "STATE_Sunset" );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Night )

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "SkyState: STATE_Night");

            GenerateNightSky();
            
            float duration = Storyboards.GetDurationMS( hGradientEffectStoryboard ) / 1000.0f;
            if (duration > 0)
            {
                ChangeStateDelayed( duration, STATE_Sunrise );
            }
            else
            {
                ChangeStateDelayed( 60.0f, STATE_Sunrise );
            }

//        OnFrameUpdate
//            DebugRender.Text( "STATE_Night" );


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Idle )

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "SkyState: STATE_Idle");

        OnFrameUpdate

EndStateMachine
}



RESULT
SkyState::GenerateSunriseSky()
{
    RESULT rval = S_OK;
    
    
    //
    // Clear the previous sky contents.
    //
    CHR(LayerMan.Clear( hSkyLayer ));
    hGradientEffectStoryboard.Release();
    hGradientEffect.Release();
    
    LayerMan.AddToLayer( hSkyLayer, hSkySprite );


    //
    // Resume casting shadows.
    //
    LayerMan.SetShadow( hPlayScreenColumn,     true );
    LayerMan.SetShadow( hPlayScreenBackground, true );
    
    //
    // Apply animating gradient (sunrise/sunset).
    // 
    IGNOREHR(Effects.GetCopy( "GradientEffect", &hGradientEffect ));
//    CHR(Sprites.SetEffect( hSkySprite, hGradientEffect ));
    CHR(Storyboards.GetCopy( "SunriseGradient", &hGradientEffectStoryboard ));
    CHR(Storyboards.BindTo( hGradientEffectStoryboard, hGradientEffect ));
    CHR(Storyboards.Start( hGradientEffectStoryboard ));

/*
    //
    // Sun rays.
    //
    {
    HSprite hSunburst;
    Rectangle spriteRect = { 0, 0, 960, 1280 };
    Sprites.CreateFromTexture( "", "sunburst", spriteRect, &hSunburst );
    Sprites.SetOpacity( hSunburst, 0.7f );
    Layers.AddToLayer( hSunBeamsLayer, hSunburst );

    HStoryboard hSunburstStoryboard;
    Storyboards.GetCopy( "Sunburst", &hSunburstStoryboard );
    Storyboards.BindTo( hSunburstStoryboard, hSunburst );
    Storyboards.Start( hSunburstStoryboard );
//    hSunburstStoryboard.Release();    // this deletes; make it a global and release on state exit.
    }
*/

    //
    // Fade out starfield emitter, then stop it.
    //
//    if (hStarfieldEmitter.IsNull() || hStarfieldEmitter.IsDangling())
//    {
//        Particles.GetCopy( "Starfield",  &hStarfieldEmitter  );
//    }
    {
//    Callback onFadeOutDone( &SkyState::HideEmitterCallback, &hStarfieldEmitter );
//    Storyboards.CallbackOnFinished( hFadeOutStoryboard, onFadeOutDone );
//    Storyboards.BindTo( hFadeOutStoryboard, hStarfieldEmitter );
//    Storyboards.Start( hFadeOutStoryboard );
    }


    //
    // Fade in clouds emitter.
    //
//    if (hCloudEmitter.IsNull() || hCloudEmitter.IsDangling())
//    {
//       Particles.GetCopy( "Clouds",     &hCloudEmitter     );
//    }
//    CHR(Particles.Start( hCloudEmitter ));
//    Storyboards.BindTo( hFadeInStoryboard, hCloudEmitter );
//    Storyboards.Start( hFadeInStoryboard );
//
//    CHR(LayerMan.AddToLayer( hSunBeamsLayer, hCloudEmitter    ));

    
Exit:
    return rval;
}



RESULT
SkyState::GenerateMorningSky()
{
    RESULT rval = S_OK;
    

    //
    // Clear the previous sky contents.
    //
    CHR(LayerMan.Clear( hSkyLayer ));
    hGradientEffectStoryboard.Release();
    hGradientEffect.Release();
    
    LayerMan.AddToLayer( hSkyLayer, hSkySprite );


    //
    // Apply animating gradient (sunrise/sunset).
    // 
    IGNOREHR(Effects.GetCopy( "GradientEffect", &hGradientEffect ));
//    CHR(Sprites.SetEffect( hSkySprite, hGradientEffect ));
    CHR(Storyboards.GetCopy( "MorningToAfternoonGradient", &hGradientEffectStoryboard ));
    CHR(Storyboards.BindTo( hGradientEffectStoryboard, hGradientEffect ));
    CHR(Storyboards.Start( hGradientEffectStoryboard ));


    //
    // Create a particle emitter (clouds, stars, etc.).
    //
//    if (hCloudEmitter.IsNull() || hCloudEmitter.IsDangling())
//    {
//       Particles.GetCopy( "Clouds", &hCloudEmitter );
//    }
//    CHR(Particles.Start( hCloudEmitter ));
//    CHR(LayerMan.AddToLayer( hSunBeamsLayer, hCloudEmitter    ));
//
//
//    //
//    // Fade in clouds emitter.
//    //
//    Storyboards.BindTo( hFadeInStoryboard, hCloudEmitter );
//    Storyboards.Start( hFadeInStoryboard );

Exit:
    return rval;
}



RESULT
SkyState::GenerateAfternoonSky()
{
    RESULT rval = S_OK;
    
    //
    // Clear the previous sky contents.
    //
    CHR(LayerMan.Clear( hSkyLayer ));
    CHR(LayerMan.Clear( hSunBeamsLayer ));
    hGradientEffectStoryboard.Release();
    hGradientEffect.Release();
    
    LayerMan.AddToLayer( hSkyLayer, hSkySprite );

    //
    // Apply animating gradient (sunrise/sunset).
    // 
    IGNOREHR(Effects.GetCopy( "GradientEffect", &hGradientEffect ));
//    CHR(Sprites.SetEffect( hSkySprite, hGradientEffect ));
    CHR(Storyboards.GetCopy( "AfternoonToEveningGradient", &hGradientEffectStoryboard ));
    CHR(Storyboards.BindTo( hGradientEffectStoryboard, hGradientEffect ));
    CHR(Storyboards.Start( hGradientEffectStoryboard ));

    //
    // Create a particle emitter (clouds, stars, etc.).
    //
//    if (!Particles.IsStarted( hCloudEmitter ))
//    {
//        CHR(Particles.Start( hCloudEmitter ));
//    }
//    CHR(LayerMan.AddToLayer( hSkyLayer, hCloudEmitter    ));

Exit:
    return rval;
}



RESULT
SkyState::GenerateEveningSky()
{
    RESULT rval = S_OK;
    
    Callback onFadeOutDone( &SkyState::HideEmitterCallback, &hCloudEmitter );

    //
    // Clear the previous sky contents.
    //
    CHR(LayerMan.Clear( hSkyLayer ));
    hGradientEffectStoryboard.Release();
    hGradientEffect.Release();
    
    LayerMan.AddToLayer( hSkyLayer, hSkySprite );

    //
    // Apply animating gradient (sunrise/sunset).
    // 
    IGNOREHR(Effects.GetCopy( "GradientEffect", &hGradientEffect ));
//    CHR(Sprites.SetEffect( hSkySprite, hGradientEffect ));
    CHR(Storyboards.GetCopy( "EveningToSunsetGradient", &hGradientEffectStoryboard ));
    CHR(Storyboards.BindTo( hGradientEffectStoryboard, hGradientEffect ));
    CHR(Storyboards.Start( hGradientEffectStoryboard ));

    //
    // Create a particle emitter (clouds, stars, etc.).
    //
//    if (!Particles.IsStarted( hCloudEmitter ))
//    {
//        CHR(Particles.Start( hCloudEmitter ));
//    }
/////    CHR(LayerMan.AddToLayer( hSkyLayer, hCloudEmitter    ));


Exit:
    return rval;
}



RESULT
SkyState::GenerateSunsetSky()
{
    RESULT rval = S_OK;
    
    Callback onFadeOutDone( &SkyState::HideEmitterCallback, &hCloudEmitter );

    //
    // Stop casting shadows.
    //
///    LayerMan.SetShadow( hPlayScreenColumn,     false );
//    LayerMan.SetShadow( hPlayScreenForeground, false );
////    LayerMan.SetShadow( hPlayScreenBackground, false );

    
    //
    // Clear the previous sky contents.
    //
    CHR(LayerMan.Clear( hSkyLayer ));
    hGradientEffectStoryboard.Release();
    hGradientEffect.Release();
    
    LayerMan.AddToLayer( hSkyLayer, hSkySprite );

    //
    // Apply animating gradient (sunrise/sunset).
    // 
    IGNOREHR(Effects.GetCopy( "GradientEffect", &hGradientEffect ));
//    CHR(Sprites.SetEffect( hSkySprite, hGradientEffect ));
    CHR(Storyboards.GetCopy( "SunsetGradient", &hGradientEffectStoryboard ));
    CHR(Storyboards.BindTo( hGradientEffectStoryboard, hGradientEffect ));
    CHR(Storyboards.Start( hGradientEffectStoryboard ));

    //
    // Create a particle emitter (clouds, stars, etc.).
    //
//    CHR(Particles.Start( hStarfieldEmitter ));
//    CHR(LayerMan.AddToLayer( hSkyLayer, hStarfieldEmitter ));
//    CHR(LayerMan.AddToLayer( hSkyLayer, hCloudEmitter    ));

    //
    // Fade out Clouds emitter, then stop it.
    //
    Storyboards.CallbackOnFinished( hFadeOutStoryboard, onFadeOutDone );
    Storyboards.BindTo( hFadeOutStoryboard, hCloudEmitter );
    Storyboards.Start( hFadeOutStoryboard );

    //
    // Fade in starfield emitter.
    //
//    Storyboards.BindTo( hFadeInStoryboard, hStarfieldEmitter );
//    Storyboards.Start( hFadeInStoryboard );
    

Exit:
    return rval;
}



RESULT
SkyState::GenerateNightSky()
{
    RESULT rval = S_OK;
   
    //
    // Clear the previous sky contents.
    //
    CHR(LayerMan.Clear( hSkyLayer ));
    hGradientEffectStoryboard.Release();
    hGradientEffect.Release();
    
    LayerMan.AddToLayer( hSkyLayer, hSkySprite );

    //
    // Apply animating gradient (sunrise/sunset).
    // 
    IGNOREHR(Effects.GetCopy( "GradientEffect", &hGradientEffect ));
//    CHR(Sprites.SetEffect( hSkySprite, hGradientEffect ));
    CHR(Storyboards.GetCopy( "NightToSunriseGradient", &hGradientEffectStoryboard ));
    CHR(Storyboards.BindTo( hGradientEffectStoryboard, hGradientEffect ));
    CHR(Storyboards.Start( hGradientEffectStoryboard ));

    //
    // Create a particle emitter (clouds, stars, etc.).
    //
//    if (!Particles.IsStarted( hStarfieldEmitter ))
//    {
//        CHR(Particles.Start( hStarfieldEmitter ));
//    }
//    CHR(LayerMan.AddToLayer( hSkyLayer, hStarfieldEmitter ));

Exit:
    return rval;
}



    
void   
SkyState::HideEmitterCallback( void* pContext )
{
    if (!pContext)
        return;

    HParticleEmitter hEmitter = *(HParticleEmitter*)pContext;
    
    Particles.Stop( hEmitter );
    //hEmitter.Release();
    
    //*(HParticleEmitter*)pContext = HParticleEmitter::NullHandle();
}


} // END namespace Z



