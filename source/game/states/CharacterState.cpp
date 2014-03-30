#include "CharacterState.hpp"
#include "Log.hpp"
#include "Engine.hpp"
#include "ColumnBrickMap.hpp"
#include "GameState.hpp"


namespace Z
{


// TEST: simple drop-to-bottom-of-screen death animation.
class  ColumnBrickMap;
extern ColumnBrickMap* g_pGameMap;

//
// Static data.
//
HGameObject CharacterState::s_hColumnController;



//
// This StateMachine controls the game characters: their facial expressions.
//

enum SpriteFaceIndex {
    SPRITE_FACE_BLINK1 = 0,
    SPRITE_FACE_BLINK2,
    SPRITE_FACE_IDLE,
    SPRITE_FACE_SAD1,
    SPRITE_FACE_SAD2,
    SPRITE_FACE_SCARED1,
    SPRITE_FACE_SCARED2,
    SPRITE_FACE_SLEEP1,
    SPRITE_FACE_SLEEP2,
    SPRITE_FACE_DEAD,
    SPRITE_FACE_HAPPY = SPRITE_FACE_BLINK1,
};


//Add new states here
enum StateName {
    STATE_Initialize,
    STATE_Idle,
    STATE_Blink,
    STATE_Sleep,
    STATE_Happy,
    STATE_Squint,
    STATE_Sad,
    STATE_Scared,
    STATE_Excited,
    STATE_Dead,
    STATE_MoveLeft,
    STATE_MoveRight,
    STATE_MoveWithinColumn
};


//Add new substates here
enum SubstateName {
	//empty
};



//=============================================================================
//
// This function is called when a Critter has fallen to bottom of screen.
//
//=============================================================================
void
CharacterState::DropCritterCallback( void* pContext )
{
    static HSound hClink;
    
    if (hClink.IsNull())
    {
        Sounds.GetCopy( "Clink", &hClink );
    }
    
    bool playSound = (bool)pContext;
    if (playSound)
    {
        Sounds.Play( hClink );
    }
}



CharacterState::CharacterState( HGameObject &hGameObject ) :
    StateMachine( hGameObject ),
    m_pGameObject(NULL),
    m_tapBegan(false)
{
}


CharacterState::~CharacterState( void )
{
}




//
// TODO: this needs to be a continuous gesture recognizer class,
// smart enough to consume TouchEvents and emit one or more GestureEvents.
// TODO: this needs to be consolidated in the StateMachine base class.
//
RESULT 
CharacterState::HandleTouch( MSG_Name msgName, IN TouchEvent* pTouchEvent )
{
    RESULT rval = S_OK;
    
    if (!pTouchEvent)
    {
        return E_NULL_POINTER;
    }
    
    if (s_hColumnController.IsNull())
    {
        GOMan.Get( "ColumnController", &s_hColumnController );
    }
    
    

    Point2D touchPoint      = pTouchEvent->point;

    if (Log::IsZoneEnabled( ZONE_TOUCH ))
    {
        WORLD_POSITION pos = WORLD_POSITION( touchPoint.x, touchPoint.y, 0.0 );
    
        DebugRender.Quad( pos, Color::Red(), 32.0 );

        DebugRender.Line( pos, vec3(0,1,0),  Color::Red(), 1.0f, 128.0f, 10.0f );
        DebugRender.Line( pos, vec3(0,-1,0), Color::Red(), 1.0f, 192.0f, 10.0f );
    }            
    
    //
    // TODO: we need a tap recognizer so GameObjects don't need to do this low-level stuff.
    // One central place to test touch events against bounding boxes.
    //
    switch ( msgName )
    {
        case MSG_TouchBegin:
        {
            m_startTouchEvent = *pTouchEvent;
            memset(&m_lastTouchEvent, 0, sizeof(m_lastTouchEvent));

            // Was our GameObject tapped?
            AABB spriteBounds = GOMan.GetBounds( m_hOwner );
            spriteBounds *= 1.5;
            
            if (spriteBounds.Intersects( touchPoint ))
            {
                m_tapBegan = true;
            }
        }
        break;
        
        case MSG_TouchEnd:
        {
            if (m_tapBegan /* && duration-of-tap > threshhold */)
            {
                AABB spriteBounds = GOMan.GetBounds( m_hOwner );
                spriteBounds *= 1.5;

                if (spriteBounds.Intersects( touchPoint ))
                {
                    GOMan.SendMessageFromSystem( s_hColumnController, MSG_GameObjectWasTapped, &m_hOwner );
                }

                m_tapBegan = false;
            }
        }
        break;
        
        default:
            ;
        
    }; // END switch( msgName )
    
    
    m_lastTouchEvent  = *pTouchEvent;


Exit:
    return rval;
}





bool CharacterState::States( State_Machine_Event event, MSG_Object * msg, int state, int substate )
{
BeginStateMachine

	//Global message responses
	OnMsg( MSG_Reset )
		ResetStateMachine();
        RETAILMSG(ZONE_STATEMACHINE, "CharacterState: MSG_Reset");

    OnMsg( MSG_KillCritter )
        ChangeState( STATE_Dead );

    OnMsg( MSG_MoveCritterLeft )
        if (GetState() != STATE_MoveLeft)
        {
            ChangeState( STATE_MoveLeft );
        }

    OnMsg( MSG_MoveCritterRight )
        if (GetState() != STATE_MoveRight)
        {
            ChangeState( STATE_MoveRight );
        }

    OnMsg( MSG_MoveCritterWithinColumn )
        ChangeState( STATE_MoveWithinColumn );

    OnMsg( MSG_DropCritterToBottom )
//        ChangeState( STATE_Happy );
        ChangeState( STATE_Squint );

    OnMsg( MSG_ScareCritter )
        ChangeState( STATE_Scared );

    OnMsg( MSG_ExciteCritter )
        ChangeState( STATE_Excited );

    OnMsg( MSG_SadCritter )
        // Add some randomness when critters die at Game Over.
        float delay = Platform::RandomDouble( 0.01, 0.5 );
        ChangeStateDelayed( delay, STATE_Sad );

    OnMsg( MSG_HappyCritter )
        ChangeState( STATE_Happy );


    OnMsg( MSG_TouchBegin )
        HandleTouch( MSG_TouchBegin,  (TouchEvent*)msg->GetPointerData() );

    OnMsg( MSG_TouchUpdate )
        HandleTouch( MSG_TouchUpdate, (TouchEvent*)msg->GetPointerData() );

    OnMsg( MSG_TouchEnd )
        HandleTouch( MSG_TouchEnd,    (TouchEvent*)msg->GetPointerData() );

        

    ///////////////////////////////////////////////////////////////
	DeclareState( STATE_Initialize )

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "CharacterState: STATE_Initialize");

            // Get a handle to the GameObject's sprite so we can animate it.
            // NOTE that we animate the sprite not the GO, because the GO
            // may be moving (dropping, rotating) and the Sprite will  
            // always draw relative to the GO's scale/position/rotation.
            GameObjects.GetGameObjectPointer( m_hOwner, &m_pGameObject );
            DEBUGCHK(m_pGameObject);
            m_hSprite = m_pGameObject->GetSprite();
            DEBUGCHK(!m_hSprite.IsNull());

			ChangeState( STATE_Idle );
	


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Idle )

		OnEnter
            //RETAILMSG(ZONE_STATEMACHINE, "CharacterState: STATE_Idle");
            m_pGameObject->SetSpriteFrame( SPRITE_FACE_IDLE );

            float delay = Platform::RandomDouble(5.0f, 30.0f);
            int nextState = Platform::Random(1, 3);

            switch (nextState)
            {
                case 1:
                    ChangeStateDelayed( delay, STATE_Blink );
                    break;
                case 2:
                    ChangeStateDelayed( delay, STATE_Happy );
                    break;
                case 3:
                    ChangeStateDelayed( delay, STATE_Sleep );
                    break;
                default:
                    ChangeStateDelayed( delay, STATE_Scared );
                    break;
            }

//        OnFrameUpdate
//            if (Log::IsZoneEnabled( ZONE_STATEMACHINE ))
//            {
//                DebugRender.Text(m_pGameObject->GetPosition(), "Idle");
//            }


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Happy )

		OnEnter
            m_pGameObject->SetSpriteFrame( SPRITE_FACE_HAPPY );

            HStoryboard hStoryboard;
            StoryboardMan.GetCopy( "WiggleCharacter", &hStoryboard );
            StoryboardMan.BindTo ( hStoryboard, m_hSprite );
            StoryboardMan.Start  ( hStoryboard );

            // TEST:
            float delay = Storyboards.GetDurationMS( hStoryboard )/1000.0f;
            delay += Platform::RandomDouble(0.3f, 0.5f);
            ChangeStateDelayed( delay, STATE_Idle );

        OnFrameUpdate
            if (Log::IsZoneEnabled( ZONE_STATEMACHINE ))
            {
                DebugRender.Text(m_pGameObject->GetPosition(), "Happy");
            }
    

	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Squint )
		OnEnter
            //m_pGameObject->SetSpriteFrame( SPRITE_FACE_BLINK2 );

            HStoryboard hStoryboard;
            StoryboardMan.GetCopy( "Squint", &hStoryboard );
            StoryboardMan.BindTo ( hStoryboard, m_hSprite );
            StoryboardMan.Start  ( hStoryboard );

            float delay = Storyboards.GetDurationMS( hStoryboard )/1000.0f;
            delay += Platform::RandomDouble(0.0f, 0.2f);
            ChangeStateDelayed( delay, STATE_Idle );

        OnFrameUpdate
            if (Log::IsZoneEnabled( ZONE_STATEMACHINE ))
            {
                DebugRender.Text(m_pGameObject->GetPosition(), "Squint");
            }



	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Blink )

		OnEnter
            //RETAILMSG(ZONE_STATEMACHINE, "CharacterState: STATE_Blink");
            m_pGameObject->SetSpriteFrame( SPRITE_FACE_BLINK1 );

            HStoryboard hStoryboard;
            StoryboardMan.GetCopy( "Blink", &hStoryboard );
            StoryboardMan.BindTo ( hStoryboard, m_hSprite );
            StoryboardMan.Start  ( hStoryboard );

            float delay = Storyboards.GetDurationMS( hStoryboard )/1000.0f;
            delay += Platform::RandomDouble(0.3f, 0.5f);
            ChangeStateDelayed( delay, STATE_Idle );
    
        OnFrameUpdate
            if (Log::IsZoneEnabled( ZONE_STATEMACHINE ))
            {
                DebugRender.Text(m_pGameObject->GetPosition(), "Blink");
            }
    

	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Sleep )

		OnEnter
            //RETAILMSG(ZONE_STATEMACHINE, "CharacterState: STATE_Sleep");
            m_pGameObject->SetSpriteFrame( SPRITE_FACE_SLEEP1 );

            HStoryboard hStoryboard;
            StoryboardMan.GetCopy( "Sleep", &hStoryboard );
            StoryboardMan.BindTo ( hStoryboard, m_hSprite );
            StoryboardMan.Start  ( hStoryboard );

            float delay = Storyboards.GetDurationMS( hStoryboard )/1000.0f;
            delay += Platform::RandomDouble(20.0f, 30.0f);
            ChangeStateDelayed( delay, STATE_Idle );

        OnFrameUpdate
            if (Log::IsZoneEnabled( ZONE_STATEMACHINE ))
            {
                DebugRender.Text(m_pGameObject->GetPosition(), "Sleep");
            }


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Sad )

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "CharacterState: STATE_Sad");
            m_pGameObject->SetSpriteFrame( SPRITE_FACE_SAD1 );

            if (Platform::CoinToss())
            {
                SoundMan.Play("Awwww");
            }

            HStoryboard hStoryboard;
            StoryboardMan.GetCopy( "WiggleCharacter2", &hStoryboard );
            StoryboardMan.BindTo ( hStoryboard, m_hSprite );
            StoryboardMan.Start  ( hStoryboard );

        OnFrameUpdate
            if (Log::IsZoneEnabled( ZONE_STATEMACHINE ))
            {
                DebugRender.Text(m_pGameObject->GetPosition(), "Sad");
            }


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Scared )

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "CharacterState: STATE_Scared");
            m_pGameObject->SetSpriteFrame( STATE_Scared );

            HStoryboard hStoryboard;
            StoryboardMan.GetCopy( "WiggleCharacter3", &hStoryboard );
            StoryboardMan.BindTo ( hStoryboard, m_hSprite );
            StoryboardMan.Start  ( hStoryboard );


            float delay = Storyboards.GetDurationMS( hStoryboard )/1000.0f;
            delay += Platform::RandomDouble(0.3f, 0.5f);
            ChangeStateDelayed( delay, STATE_Idle );

        OnFrameUpdate
            if (Log::IsZoneEnabled( ZONE_STATEMACHINE ))
            {
                DebugRender.Text(m_pGameObject->GetPosition(), "Scared");
            }


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Excited )

		OnEnter
            //RETAILMSG(ZONE_STATEMACHINE, "CharacterState: STATE_Excited");
            m_pGameObject->SetSpriteFrame( SPRITE_FACE_SCARED1 );

            HStoryboard hStoryboard;
            StoryboardMan.GetCopy( "WiggleCharacter", &hStoryboard );
            StoryboardMan.BindTo ( hStoryboard, m_hSprite );
            StoryboardMan.Start  ( hStoryboard );

            // TEST:
            float delay = Storyboards.GetDurationMS( hStoryboard )/1000.0f;
            delay += Platform::RandomDouble(0.3f, 0.5f);
            ChangeStateDelayed( delay, STATE_Idle );

        OnFrameUpdate
            if (Log::IsZoneEnabled( ZONE_STATEMACHINE ))
            {
                DebugRender.Text(m_pGameObject->GetPosition(), "Excited");
            }


    ///////////////////////////////////////////////////////////////
	DeclareState( STATE_Dead )

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "CharacterState: STATE_Dead");

            m_pGameObject->SetSpriteFrame( SPRITE_FACE_DEAD );

            // Bind a death animation to the Sprite.
            HStoryboard hDeadCharacterStoryboard;
            Storyboards.GetCopy( "DeadCharacter", &hDeadCharacterStoryboard );
            Storyboards.BindTo( hDeadCharacterStoryboard, m_hSprite );
            Storyboards.Start( hDeadCharacterStoryboard );

        OnFrameUpdate
            if (Log::IsZoneEnabled( ZONE_STATEMACHINE ))
            {
                DebugRender.Text(m_pGameObject->GetPosition(), "Dead");
            }


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_MoveLeft )

		OnEnter
            HStoryboard hStoryboard;
            StoryboardMan.GetCopy( "MoveCharacterLeft", &hStoryboard );
            StoryboardMan.BindTo ( hStoryboard, m_hSprite );
            StoryboardMan.Start  ( hStoryboard );

            float delay = Storyboards.GetDurationMS( hStoryboard )/1000.0f;
            ChangeStateDelayed( delay, STATE_Idle );

        OnFrameUpdate
            if (Log::IsZoneEnabled( ZONE_STATEMACHINE ))
            {
                DebugRender.Text(m_pGameObject->GetPosition(), "<<<");
            }



	///////////////////////////////////////////////////////////////
	DeclareState( STATE_MoveRight )

		OnEnter
            m_pGameObject->SetSpriteFrame( SPRITE_FACE_BLINK2 );

            HStoryboard hStoryboard;
            StoryboardMan.GetCopy( "MoveCharacterRight", &hStoryboard );
            StoryboardMan.BindTo ( hStoryboard, m_hSprite );
            StoryboardMan.Start  ( hStoryboard );

            float delay = Storyboards.GetDurationMS( hStoryboard )/1000.0f;
            ChangeStateDelayed( delay, STATE_Idle );

        OnFrameUpdate
            if (Log::IsZoneEnabled( ZONE_STATEMACHINE ))
            {
                DebugRender.Text(m_pGameObject->GetPosition(), ">>>");
            }
    

	///////////////////////////////////////////////////////////////
	DeclareState( STATE_MoveWithinColumn )

        OnMsg( MSG_BeginStoryboard )
            HStoryboard hStoryboard;
            StoryboardMan.GetCopy( "MoveCharacterWithinColumn", &hStoryboard );
            StoryboardMan.BindTo ( hStoryboard, m_hSprite );
            StoryboardMan.Start  ( hStoryboard );
            float delay = Storyboards.GetDurationMS( hStoryboard )/1000.0f;
            ChangeStateDelayed( delay, STATE_Idle );

		OnEnter
            m_pGameObject->SetSpriteFrame( SPRITE_FACE_HAPPY );

            //float delay = Platform::RandomDouble(0.1f, 0.25f);
            float delay = 0.1f;
            SendMsgDelayedToState(delay, MSG_BeginStoryboard);

        OnFrameUpdate
            if (Log::IsZoneEnabled( ZONE_STATEMACHINE ))
            {
                DebugRender.Text(m_pGameObject->GetPosition(), "(())");
            }
    

EndStateMachine
}



} // END namespace Z



