#include "BombState.hpp"
#include "Log.hpp"
#include "Engine.hpp"
#include "ColumnBrickMap.hpp"
#include "GameState.hpp"


namespace Z
{

//
// Static data.
//
HGameObject BombState::s_hColumnController;

HStoryboard hAttentionStoryboard;
HStoryboard hBlinkStoryboard;


//
// This StateMachine controls a bomb.
//

enum SpriteIndex {
    SPRITE_IDLE    = 0,
    SPRITE_BLINK,
};


//Add new states here
enum StateName {
    STATE_Initialize,
    STATE_Touched,
    STATE_Idle,
    STATE_Blink,
    STATE_Attention,
    STATE_Explode,
    STATE_Dead,
};


//Add new substates here
enum SubstateName {
	//empty
};



BombState::BombState( HGameObject &hGameObject ) :
    StateMachine( hGameObject ),
    m_pGameObject(NULL),
    m_tapBegan(false)
{
}


BombState::~BombState( void )
{
}




//
// TODO: this needs to be a continuous gesture recognizer class,
// smart enough to consume TouchEvents and emit one or more GestureEvents.
// TODO: this needs to be consolidated in the StateMachine base class.
//
RESULT 
BombState::HandleTouch( MSG_Name msgName, IN TouchEvent* pTouchEvent )
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
            DEBUGMSG(ZONE_INFO, ">>>>> BombState[%d] MSG_TouchBegin", m_ID);

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
            DEBUGMSG(ZONE_INFO, "<<<<< BombState[%d] MSG_TouchEnd", m_ID);

            if (m_tapBegan /* && duration-of-tap > threshhold */)
            {
                AABB spriteBounds = GOMan.GetBounds( m_hOwner );
                spriteBounds *= 1.5;

                if (spriteBounds.Intersects( touchPoint ))
                {
                    DEBUGMSG(ZONE_INFO, "BombState[%d] was tapped", m_ID);
                    GOMan.SendMessageFromSystem( s_hColumnController, MSG_ColumnDropToBottom );
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





bool BombState::States( State_Machine_Event event, MSG_Object * msg, int state, int substate )
{
BeginStateMachine

	//Global message responses
	OnMsg( MSG_Reset )
		ResetStateMachine();
        RETAILMSG(ZONE_STATEMACHINE, "BombState: MSG_Reset");

    OnMsg( MSG_TouchBegin )
        HandleTouch( MSG_TouchBegin,  (TouchEvent*)msg->GetPointerData() );

    OnMsg( MSG_TouchUpdate )
        HandleTouch( MSG_TouchUpdate, (TouchEvent*)msg->GetPointerData() );

    OnMsg( MSG_TouchEnd )
        HandleTouch( MSG_TouchEnd,    (TouchEvent*)msg->GetPointerData() );

    OnMsg( MSG_KillCritter )
        ChangeState( STATE_Dead );

    OnMsg( MSG_Detonate )
        GOMan.SendMessageFromSystem( s_hColumnController, MSG_Detonate, &m_hOwner );

    

    ///////////////////////////////////////////////////////////////
	DeclareState( STATE_Initialize )

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "BombState: STATE_Initialize");

            // Get a handle to the GameObject's sprite so we can animate it.
            // NOTE that we animate the sprite not the GO, because the GO
            // may be moving (dropping, rotating) and the Sprite will  
            // always draw relative to the GO's scale/position/rotation.
            GameObjects.GetGameObjectPointer( m_hOwner, &m_pGameObject );
            DEBUGCHK(m_pGameObject);
            m_hSprite = m_pGameObject->GetSprite();
            DEBUGCHK(!m_hSprite.IsNull());

			ChangeState( STATE_Attention );
	


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Idle )

		OnEnter
            m_pGameObject->SetSpriteFrame( 0 );

            int idleState = Platform::Random(0,1);
    
            switch (idleState)
            {
                case 0:
                    ChangeStateDelayed(30.0f, STATE_Attention);
                    break;
                    
                case 1:
                    ChangeStateDelayed(20.0f, STATE_Blink);
                    break;
                    
                default:
                    ChangeStateDelayed(20.0f, STATE_Blink);
                
            }

        OnFrameUpdate
            if (Log::IsZoneEnabled( ZONE_STATEMACHINE ))
            {
                DebugRender.Text(m_pGameObject->GetPosition(), "Idle");
            }



	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Touched )

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "BombState: STATE_Touched");



	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Explode )

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "BombState: STATE_Explode");



	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Blink )

		OnEnter
            Storyboards.GetCopy("BombBlink",     &hBlinkStoryboard);
            Storyboards.BindTo ( hBlinkStoryboard, m_hSprite );
            Storyboards.Start  ( hBlinkStoryboard );

            float delay = Storyboards.GetDurationMS( hBlinkStoryboard )/1000.0f;
            ChangeStateDelayed( delay, STATE_Idle );

        OnExit
            Storyboards.Stop( hBlinkStoryboard );
            //hBlinkStoryboard.Release();

        OnFrameUpdate
            if (Log::IsZoneEnabled( ZONE_STATEMACHINE ))
            {
                DebugRender.Text(m_pGameObject->GetPosition(), "Blink");
            }


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Attention )

		OnEnter
            Storyboards.GetCopy("BombAttention", &hAttentionStoryboard);
            Storyboards.BindTo ( hAttentionStoryboard, m_hSprite );
            Storyboards.Start  ( hAttentionStoryboard );

            float delay = Storyboards.GetDurationMS( hAttentionStoryboard )/1000.0f;
            ChangeStateDelayed( delay, STATE_Idle );

        OnExit
            Storyboards.Stop( hAttentionStoryboard );
            //hAttentionStoryboard.Release();

        OnFrameUpdate
            if (Log::IsZoneEnabled( ZONE_STATEMACHINE ))
            {
                DebugRender.Text(m_pGameObject->GetPosition(), "!!!!");
            }


	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Dead )

		OnEnter
            Storyboards.Stop( hAttentionStoryboard );
            Storyboards.Stop( hBlinkStoryboard );
            Sprites.SetVisible(m_hSprite, false);   // TODO: we really should explode bombs on game over and give the player points



EndStateMachine
}



} // END namespace Z



