#include "ColumnState.hpp"
#include "Log.hpp"
#include "SpriteManager.hpp"
#include "LayerManager.hpp"
#include "GameObjectManager.hpp"
#include "TouchInput.hpp"
#include "Engine.hpp"
#include "StoryboardManager.hpp"
#include "GameState.hpp"
#include "ColumnBrickMap.hpp"
#include "SoundManager.hpp"
#include "Level.hpp"
#include "RippleEffect.hpp" 
#include "Achievements.hpp"
#include "BoxedVariable.hpp"

#include <stddef.h> // for sprintf()




namespace Z
{


extern UINT32   g_totalScore;
extern UINT32   g_scoreToLevelUp;
extern UINT32   g_gameNumLinesCleared;
extern UINT32   g_gameNumBlocksCleared;
extern UINT32   g_gameNumChains;
extern UINT64   g_startGameTime;
extern UINT32   g_highestLevel;
extern UINT32   g_highestLevelEver;

extern BoxedVariable g_scoreScale;


//
// This StateMachine controls the behavior of the Column currently in-play.
//

// If player clears N blocks in one move, they earn a vertical bomb.
// If player causes N chains in one move, they earn a radial bomb.
const UINT8     BLOCKS_TO_EARN_VERTICAL_BOMB    = 7;
const UINT8     CHAINS_TO_EARN_RADIAL_BOMB      = 2;



// Lazy programmer: I treat StateMachines like a big script.  Variables everywhere.
static UINT8    g_numLines                = 0;
static UINT8    g_longestLine             = 0;
static UINT8    g_numChains               = 0;
static INT32    g_numDroppingBricks       = 0;
static UINT32   g_currentDropDurationMS   = 0;
static bool     g_levelledUp              = false;    // user has levelled up, display a storyboard when done dropping blocks.

static HGameObjectSet g_smashedBricks;


// Map of the playing field, which tracks each placed Brick.
// Used for collision detection.
ColumnBrickMap* g_pGameMap = NULL;


static HLayer           hPlayScreenBackground;
static HLayer           hPlayScreenGrid;
static HLayer           hPlayScreenColumn;
static HLayer           hPlayScreenForeground;
static HLayer           hPlayScreenAlerts;

static HGameObject      hColumn;
static HGameObject      hColumnBricks[3];
static TouchEvent       lastTouchEvent;
static TouchEvent       startTouchEvent;

static HGameObjectSet*  pClearedBricks = NULL;

static HEffect          hRippleEffect;


static HStoryboard      hCameraShakeStoryboard;
static HStoryboard      hCameraZoomStoryboard;
static HStoryboard      hSmashCameraShakeStoryboard;
static HStoryboard      hDropColumnStoryboard;
static HStoryboard      hAwesomeRippleStoryboard;
static HStoryboard      hUpdateScoreStoryboard;

static HSound           hSlideLeft;
static HSound           hSlideRight;
static HSound           hSlideDown;
static HSound           hRotate;
static HSound           hDrop;
static HSound           hClink;
static HSound           hScore1;
static HSound           hScore2;
static HSound           hScore3;
static HSound           hChain1;
static HSound           hChain2;
static HSound           hChain3;
static HSound           hSmashBlock;
static HSound           hMegaChainSound;


//Add new states here
enum StateName 
{
    STATE_Initialize = 0,
    STATE_Paused,
    STATE_ResumeFromPause,
    STATE_TutorialMode,
    STATE_CreateColumn,
    STATE_DropColumn,
    STATE_LandColumn,
    STATE_DropAllBricks,
    STATE_SmashBricks,
    STATE_CheckForCompletedLines,
    STATE_ShowCompletedLines,
    STATE_ClearCompletedLines,
    STATE_LevelUp,
    STATE_GameOver,
    STATE_Idle,
};
StateName g_stateWhenUnpaused       = STATE_DropColumn;


//Add new substates here
enum SubstateName 
{
	SUBSTATE_WaitForDroppingBricks
};



ColumnState::ColumnState( HGameObject &hGameObject ) :
    StateMachine( hGameObject )
{
}

ColumnState::~ColumnState( void )
{
    hPlayScreenBackground.Release();
    hPlayScreenGrid.Release();
    hPlayScreenColumn.Release();
    hPlayScreenForeground.Release();
    hPlayScreenAlerts.Release();

//    hBackgroundSprite.Release();

    hRippleEffect.Release();

    hCameraShakeStoryboard.Release();
    hCameraZoomStoryboard.Release();
    hSmashCameraShakeStoryboard.Release();
    hDropColumnStoryboard.Release();
    hAwesomeRippleStoryboard.Release();
    hUpdateScoreStoryboard.Release();

    hSlideLeft.Release();
    hSlideRight.Release();
    hSlideDown.Release();
    hRotate.Release();
    hDrop.Release();
    hClink.Release();
    hScore1.Release();
    hScore2.Release();
    hScore3.Release();
    hChain1.Release();
    hChain2.Release();
    hChain3.Release();
    hSmashBlock.Release();
    hMegaChainSound.Release();

    for (int i = 0; i < NUM_BRICKS_PER_COLUMN; ++i)
    {
        TouchScreen.RemoveListener( hColumnBricks[i], MSG_TouchBegin  );
        TouchScreen.RemoveListener( hColumnBricks[i], MSG_TouchUpdate );
        TouchScreen.RemoveListener( hColumnBricks[i], MSG_TouchEnd    );

        hColumnBricks[i].Release();
    }
    
    hColumn.Release();

//    DEBUGCHK(0);
}



//=============================================================================
//
// This function is called when a brick has finished dropping due to gravity.
//
//=============================================================================
void
ColumnState::DropBrickCallback( void* pContext )
{
    g_numDroppingBricks--;
    SoundMan.Play( hClink );
}


void
ColumnState::ScoreRippleDoneCallback( void* pContext )
{
    LayerMan.SetEffect( hPlayScreenGrid, HEffect::NullHandle() );
    LayerMan.SetEffect( hPlayScreenForeground, HEffect::NullHandle() );
}



RESULT
ColumnState::CreateColumn( WORLD_POSITION position, BrickType* pBrickTypes, UINT8 numTypes )
{
    RESULT          rval                = S_OK;
    WORLD_POSITION  pos                 = position;
    UINT8           numSpecialBricks    = 0;


    // 
    // Create column with specified brick types.
    //
    if (pBrickTypes && numTypes)
    {
        for (int i = 0; i < numTypes; ++i)
        {
            CHR(CreateBrickOfType( &pBrickTypes[i], pos, &hColumnBricks[i] ));
            pos.y -= COLUMN_BRICK_HEIGHT;
        }
        
        g_pGameMap->Update();
        
        goto Exit;
    }
    

    //
    // Roll dice to select the brick types.
    // 
    // TODO: clean this up!
    for (int i = 0; i < NUM_BRICKS_PER_COLUMN; ++i)
    {
        // Only roll for special bricks on the last block in the column.
        // Ensures that bomb blocks are on the bottom, where they do good even
        // if the player doesn't flick them downwards.
        if (i == NUM_BRICKS_PER_COLUMN-1)
        {
            // Roll the dice for each special brick available on this level.
            // We need to roll for all of them to give each an equal chance,
            // otherwise the first one in the list will tend to be chosen more often.
            BrickType* specialBrickCandidates[3];
            for (int j = 0; j < 3; ++j)
            {
                BrickType* pBrickType = &g_specialBrickTypes[j];
                
                // Is this brick type available in the current level?
                if (g_pLevel->specialBrickTypes & pBrickType->goType)
                {
                    if (Platform::RandomDouble(0.0, 1.0) <= pBrickType->probability)
                    {
                        specialBrickCandidates[ numSpecialBricks++ ] = pBrickType;
                    }
                }
            }

            // Of the special bricks we rolled for, pick one at random.
            // Should ensure a fair distribution?
            if (numSpecialBricks)
            {
                UINT8 index = Platform::Random() % numSpecialBricks;
                BrickType* pBrickType = specialBrickCandidates[index];
                CreateBrickOfType( pBrickType, pos, &hColumnBricks[i] );
                
                pos.y -= COLUMN_BRICK_HEIGHT;
                
//                HStoryboard hFlashBombStoryboard;
//                StoryboardMan.GetCopy( "FlashStone", &hFlashBombStoryboard );
//                StoryboardMan.BindTo( hFlashBombStoryboard, hColumnBricks[i] );
//                StoryboardMan.Start( hFlashBombStoryboard );
//                // HORRIBLE HACK:
//                // Storyboard loops forever; stop it when its target is deleted.
//                // TODO: some robust way for objects to notify listeners when they are deleted.
//                g_loopingStoryboards.insert( std::pair< HGameObject, HStoryboard >( hColumnBricks[i], hFlashBombStoryboard ) );

                continue;
            }
        }
                
        // Pick a brick at random.
        BrickType* pBrickType = &g_brickTypes[Platform::Random() % g_pLevel->numBrickTypes];

        // TODO: prevent 3-of-a-kind; annoys some players.

        CreateBrickOfType( pBrickType, pos, &hColumnBricks[i] );

        pos.y -= COLUMN_BRICK_HEIGHT;
    }

    g_pGameMap->Update();

Exit:
    return rval;
}



RESULT
ColumnState::CreateBrickOfType( BrickType* pBrickType, WORLD_POSITION position, HGameObject* pHGameObject )
{
    RESULT      rval = S_OK;
    HGameObject handle;
    char        brickName[MAX_NAME];

    CPR(pBrickType);
    CPR(pHGameObject);
    CPR(pBrickType->spriteName);

    sprintf( brickName, "%s_%X", pBrickType->spriteName, (unsigned int)Platform::Random());

    CHR(GOMan.Create(   brickName,                                      // IN:  name for this GameObject
                        &handle,                                        // OUT: handle
                        pBrickType->spriteName,                         // IN:  sprite name
                        "",                                             // IN:  mesh name
                        "",                                             // IN:  effect name
                        pBrickType->behaviorName,                       // IN:  behavior name
                        pBrickType->goType,                             // IN:  GO_TYPE
                        position,                                       // IN:  world position
                        1.0f,                                           // IN:  opacity,
                        Color::White(),                                 // IN:  color,
                        true                                            // IN:  hasShadow
                    ));

    // Add Brick to the screen (does not take a reference)
    LayerMan.AddToLayer( hPlayScreenColumn, handle );
    
    // Add Brick to the gameMap (does not take a reference)
//    DEBUGMSG(ZONE_MAP, "Adding [%s] 0x%x to game map", brickName, (UINT32)handle);
//    g_pGameMap->AddGameObject( handle );

    // TEST:
//    {
//        HParticleEmitter hSwirl;
//        Particles.GetCopy("Swirl", &hSwirl);
//        Particles.Start(hSwirl);
//        GameObjects.AddChild(handle, hSwirl);
//    }

    // Each brick registers for tap notifications:
    TouchScreen.AddListener( handle, MSG_TouchBegin  );
    TouchScreen.AddListener( handle, MSG_TouchUpdate );
    TouchScreen.AddListener( handle, MSG_TouchEnd    );
    

    *pHGameObject = handle;

Exit:
    return rval;
}



//=============================================================================
//
// Touch Handling method: tap/gesture on column, or tap/gesture anywhere on screen
//
//=============================================================================
bool
ColumnState::WasColumnTouched( TouchEvent* pTouchEvent )
{
    bool rval = false;

    if ( !pTouchEvent )
    {
        goto Exit;
    }

    // Test each brick directly.
    // TODO: GameObjects that container children, with correct bounding box.
    for (int i = 0; i < NUM_BRICKS_PER_COLUMN; ++i)
    {
        AABB spriteBounds = GOMan.GetBounds( hColumnBricks[i] );
        
        // Sometimes players think they've tapped the Column but they missed by a few pixels, 
        // and it frustrates them.
        // So expand the bounding-box to handle near-misses.
        spriteBounds *= 1.5;
        
        if (Log::IsZoneEnabled( ZONE_TOUCH ))
        {
            DebugRender.Quad( spriteBounds );
        }
        
        if ( spriteBounds.Intersects( pTouchEvent->point ))
        {
//            DEBUGMSG(ZONE_INFO, "ColumnState::WasColumnTouched() = true: (%2.2f, %2.2f)", pTouchEvent->point.x, pTouchEvent->point.y);
            rval   = true;
            break;
        }
    }

Exit:
    return rval;
}



RESULT
ColumnState::HandleGameObjectTapped( MSG_Object* pMsgObject )
{
    RESULT          rval    = S_OK;
    HGameObject     handle;
    int             numSmashed;

    if (!pMsgObject)
    {
        return E_NULL_POINTER;
    }

    if (pMsgObject->IsDataValid() && pMsgObject->IsPointerData())
    {
        handle = *(HGameObject*)pMsgObject->GetPointerData();
        DEBUGCHK(!handle.IsNull());
    }

    WORLD_POSITION  pos     = GOMan.GetPosition( handle );
    GO_TYPE         type    = GOMan.GetType( handle );


    // Mask off the type bits we care about
    type = (GO_TYPE)((UINT32)type & (UINT32)(GO_TYPE_VERTICAL_BOMB|GO_TYPE_RADIAL_BOMB));


    // If the GO is a bomb, explode it.
    if (type & GO_TYPE_VERTICAL_BOMB || type & GO_TYPE_RADIAL_BOMB)
    {
//        // Shock wave.
//        HParticleEmitter hEmitter;
//        switch (type)
//        {
//            case GO_TYPE_VERTICAL_BOMB:
//                Particles.GetCopy( "shock", &hEmitter );
//                break;
//            case GO_TYPE_RADIAL_BOMB:
//                Particles.GetCopy( "shock", &hEmitter );
//                break;
//            default:
//                break;
//        }
//
//        WORLD_POSITION emitterPos = pos;
//        emitterPos.x += COLUMN_BRICK_WIDTH/2.0f;
//        Particles.SetPosition( hEmitter, emitterPos );
//        LayerMan.AddToLayer( hPlayScreenForeground, hEmitter );
//        Particles.Start( hEmitter );

//        bool chainReaction = pMsgObject->GetName() == MSG_Detonate ? true : false;
        bool chainReaction = false;
        bool wasFlung = true;
        numSmashed = SmashBricks(pos, type, wasFlung, chainReaction);  // TODO: only set true if user tapped and held the bomb for certain duration.


        // Update score based on numSmashed.
        // TODO: clean up the scoring and put it all in one place.
        UINT8 brickScore = 0;
        switch (numSmashed) 
        {
            case 0:
                brickScore = 0;
                break;
            case 1:
            case 2:
            case 3:
                brickScore = 10;
                break;
            case 4:
                brickScore = 15;
                break;
            case 5:
                brickScore = 30;
                break;
            case 6:
                brickScore = 100;
                break;
            default:
                brickScore = 200;
                break;
        };
        
        UINT32 score = numSmashed * brickScore * g_pLevel->scoreMultiplier * g_difficultyMultiplier;
        g_totalScore += score;
        SendMsgToStateMachine( MSG_UpdateScore );    // TODO: extend MsgRouter to accept data for broadcast messages!

        //
        // Drop the column in play (HACK: just because STATE_SmashBricks currently can't go back to STATE_Drop).
        //
        for (int i = 0; i < NUM_BRICKS_PER_COLUMN; ++i)
        {
            g_pGameMap->AddGameObject ( hColumnBricks[i] );
            
            LayerMan.AddToLayer     ( hPlayScreenGrid,   hColumnBricks[i] );
            LayerMan.RemoveFromLayer( hPlayScreenColumn, hColumnBricks[i] );
        }

        //
        // Smash the game objects affected by the bomb.
        //
        if (GetState() != STATE_SmashBricks )
        {
            ChangeState( STATE_SmashBricks );
        }
    }

Exit:
    return rval;
}



//
// TODO: this needs to be a continuous gesture recognizer class,
// smart enough to consume TouchEvents and emit one or more GestureEvents.
// TODO: this needs to be consolidated in the StateMachine base class.
//
RESULT 
ColumnState::HandleTouch( MSG_Name msgName, IN TouchEvent* pTouchEvent )
{
    RESULT rval = S_OK;
    static bool   columnDragged = false;
    static bool   columnDropped = false;
    
    if (!pTouchEvent)
    {
        return E_NULL_POINTER;
    }
    

    Point2D startTouchPoint = startTouchEvent.point;
    Point2D lastTouchPoint  = lastTouchEvent.point;
    Point2D touchPoint      = pTouchEvent->point;

    // Determine direction and speed of the finger movement.
    // We're looking for sideways and downward swipe gestures.
    // We need to handle the case where user swipes sideways, then downwards, without lifting the finger.
    // Swiping down at a certain speed causes the game piece to be dropped.
  
    vec2   totalGestureVector       = vec2( touchPoint.x - startTouchPoint.x, touchPoint.y - startTouchPoint.y );
    
    vec2   swipeVector              = vec2( touchPoint.x - lastTouchPoint.x,  touchPoint.y - lastTouchPoint.y  );
    vec2   swipeVectorNormalized    = swipeVector.Normalized();

    vec2   swipeHorizontal          = vec2( touchPoint.x - lastTouchPoint.x, 0 );
    float  swipeHorizontalVelocity  = swipeHorizontal.Length();     // TODO: take into account the deltaT between updates!  Handles dropped messages!
    
    vec2   swipeVertical            = vec2( 0, touchPoint.y - lastTouchPoint.y );
    float  swipeVerticalVelocity    = swipeVertical.Length();       // TODO: take into account the deltaT between updates!  Handles dropped messages!

    vec2   totalSwipeVertical       = vec2( 0, touchPoint.y - startTouchPoint.y );


    if (Log::IsZoneEnabled( ZONE_TOUCH ))
    {
        WORLD_POSITION pos = WORLD_POSITION( touchPoint.x, touchPoint.y, 0.0 );
    
        DebugRender.Quad( pos, Color::Red(), 32.0 );

        DebugRender.Line( pos, vec3(0,1,0),  Color::Red(), 1.0f, 128.0f, 10.0f );
        DebugRender.Line( pos, vec3(0,-1,0), Color::Red(), 1.0f, 192.0f, 10.0f );
    }            
    
    
    switch ( msgName )
    {
        case MSG_TouchBegin:
        {
            RETAILMSG(ZONE_TOUCH, ">>>> Column: MSG_TouchBegin");

            startTouchEvent = *pTouchEvent;
            memset(&lastTouchEvent, 0, sizeof(lastTouchEvent));
            
            columnDragged  = false;
            columnDropped  = false;
        }
        break;
        
        case MSG_TouchUpdate:
        {
            RETAILMSG(ZONE_TOUCH, "\tColumn: MSG_TouchUpdate (H %2.0f  V %2.0f)", swipeHorizontalVelocity, swipeVerticalVelocity);
            
            // HACK HACK: use the first brick to determine game piece column.
            AABB  spriteBounds  = GOMan.GetBounds( hColumnBricks[0] );
            float cellWidth     = spriteBounds.GetWidth();
            float columnX       = cellWidth/2.0f + GOMan.GetPosition( hColumnBricks[0] ).x - GAME_GRID_LEFT;  // CENTER of sprite
            float tapX          = pTouchEvent->point.x                                     - GAME_GRID_LEFT;
            int   oldColumn     = columnX / cellWidth;
            int   newColumn     = tapX    / cellWidth;

            // Test for drop gesture (rapid swipe downwards, within N degrees of the Y axis).
            if ( swipeVerticalVelocity >= DROP_VELOCITY && DEGREES(fabs(swipeVectorNormalized.Angle(vec2(0, -1)))) < SWIPE_AXIS_THRESHOLD_DEGREES )
            {
                columnDropped = true;
                SendMsgToState( MSG_ColumnDropToBottom );
            }
            // Test for drag down gesture (slow swipe downwards, within N degrees of the Y axis)
            else if ( totalSwipeVertical.Length() > DROP_LENGTH && DEGREES(fabs(swipeVectorNormalized.Angle(vec2(0, -1)))) < SWIPE_AXIS_THRESHOLD_DEGREES )
            {
                columnDragged = true;
            
                // Reset the gesture so we don't spew back-to-back drop messages.
                startTouchEvent = *pTouchEvent;
                
                if ( !(CollisionCheck() & COLLISION_BELOW) )
                {
                    SoundMan.Play( hSlideDown );
                    ChangeState( STATE_DropColumn );
                }
            }
            else if ( swipeVerticalVelocity < DROP_VELOCITY_HINT && fabs(swipeHorizontalVelocity) > 0 /*&& swipeHorizontalVelocity > swipeVerticalVelocity*/ )
            {
                int columnOffset = newColumn - oldColumn;
                
                if ( columnOffset != 0 )
                {
                    DEBUGMSG(ZONE_TOUCH, "\tcolumnOffset = %d (%d -> %d), swipeVector = (%2.2f, %2.2f): %2.2f rad", 
                        columnOffset, oldColumn, newColumn,
                        swipeVectorNormalized.x, swipeVectorNormalized.y,
                        swipeVectorNormalized.Dot(vec2(1,0)));
                }
            
                // Move column left or right, but only after the swipe gesture has
                // caught up with or passed the column's location.
                // Prevents the column from snapping back towards the finger in the opposite
                // direction of the swipe.
                if (swipeVectorNormalized.Dot(vec2(1,0)) < 0 && columnOffset < 0)
                {
                    columnDragged = true;
                    //SendMsgToState( MSG_ColumnLeft );  // Can't do this if we've enabled duplicate messages in the msgrouter. 
                    for (int i = 0; i < abs(columnOffset); ++i)
                    {
                        MoveColumnLeft();
                    }
                }
                else if (swipeVectorNormalized.Dot(vec2(1,0)) > 0 && columnOffset > 0)
                {
                    columnDragged = true;
                    //SendMsgToState( MSG_ColumnRight );
                    for (int i = 0; i < abs(columnOffset); ++i)
                    {
                        MoveColumnRight();
                    }
                }
            }
        }       
        break;
        
        case MSG_TouchEnd:
        {
            RETAILMSG(ZONE_TOUCH, "<<<< Column: MSG_TouchEnd");

            // TODO: test the duration from TouchBegin to TouchEnd; debounce so we don't send repated MSG_ColumnRotates.
            
            // If user tapped on screen - but did not perform a swipe - rotate the column.
            if (!columnDragged && !columnDropped && totalGestureVector.Length() < TAP_MOVEMENT_CUTOFF)
            {
                SendMsgToState( MSG_ColumnRotate );
                //RotateColumn();
            }

            memset(&startTouchEvent, 0, sizeof(startTouchEvent));
            memset(&lastTouchEvent,  0, sizeof(lastTouchEvent));

            columnDragged  = false;
            columnDropped  = false;
        }
        break;
        
        default:
            ;
        
    }; // END switch( msgName )
    
    
    lastTouchEvent  = *pTouchEvent;


Exit:
    return rval;
}


bool
ColumnState::CheckForLevelUp( )
{
    if (g_isTutorialMode)
        return false;
        

    if (g_levelledUp)
        return true;


    // Has the player advanced to next level?
    if (g_totalScore > g_scoreToLevelUp)
    {
        g_pLevel++;
        if (!g_pLevel->level)
        {
            // Don't advance once we've reached the end of the levels array.
            g_pLevel--;
            return false;
        }
        else
        {
            // Display "Level Up!" stuff later, when we're done dropping blocks.
            g_levelledUp = true;
            
            g_scoreToLevelUp = g_totalScore + g_pLevel->scoreToLevelUp;
        }
        
        g_highestLevel      = MAX(g_highestLevel, g_pLevel->level);
        g_highestLevelEver  = MAX(g_highestLevelEver, g_highestLevel);
        
        UserSettings.SetInt( "/UserPrefs.HighestLevel", g_highestLevelEver );
        UserSettings.Write();
        
        return true;
    }
    
    return false;
}



float
ColumnState::TimeUntilDrop()
{
    float delay = 0.0f;
    
    switch (g_difficulty)
    {
        case 1:     // EASY
            delay = g_pLevel->secondsPerMove * 1.5f;
            break;

        case 2:     // MEDIUM
            delay = g_pLevel->secondsPerMove;
            break;

        case 3:     // HARD
            delay = g_pLevel->secondsPerMove * 0.5f;
            break;

        case 4:     // CHALLENGE
            delay = g_pLevel->secondsPerMove * 0.35f;
            break;

        default:
            delay = g_pLevel->secondsPerMove;
            DEBUGCHK(0);
            break;
    }

    return delay;
}



//=============================================================================
//
// Test bottom-most brick for collision with neighbors.
//
//=============================================================================
ColumnState::COLLISION_RESULT
ColumnState::CollisionCheck( )
{
    COLLISION_RESULT rval = COLLISION_NONE;
    
    WORLD_POSITION worldPos;
    MAP_POSITION   mapPos;
    
    worldPos = GOMan.GetPosition( hColumnBricks[2] );
    g_pGameMap->WorldToMapPosition( worldPos, &mapPos  );

    MAP_POSITION cellLeft ( mapPos.x-1, mapPos.y   );
    MAP_POSITION cellRight( mapPos.x+1, mapPos.y   );
    MAP_POSITION cellBelow( mapPos.x,   mapPos.y-1 );
    
    GO_TYPE value = (GO_TYPE)(0);
    if ( (value = g_pGameMap->GetValue( cellLeft )) != 0 || cellLeft.x < 0 )
    {
        DEBUGMSG(ZONE_COLLISION, "ColumnState::CollisionCheck(): COLLISION_LEFT value: %d", value);
        rval = (COLLISION_RESULT)(rval | COLLISION_LEFT);
    }

    if ( (value = g_pGameMap->GetValue( cellRight )) != 0 || cellRight.x >= g_pGameMap->GetWidth() )
    {
        DEBUGMSG(ZONE_COLLISION, "ColumnState::CollisionCheck(): COLLISION_RIGHT value: %d", value);
        rval = (COLLISION_RESULT)(rval | COLLISION_RIGHT);
    }

    if ( (value = g_pGameMap->GetValue( cellBelow )) != 0 || cellBelow.y < 0 )
    {
        DEBUGMSG(ZONE_COLLISION, "ColumnState::CollisionCheck(): COLLISION_BELOW value: %d", value);
        rval = (COLLISION_RESULT)(rval | COLLISION_BELOW);
    }
     
Exit:
    return rval;
}



//=============================================================================
//
// Find all contiguous bricks on the map given a minimum line length.
// Return them in a set.
//
//=============================================================================
HGameObjectSet*
ColumnState::FindContiguousBricks( UINT32 minCount, OUT UINT8* pLongestLine, OUT UINT8* pNumLines )
{
    HGameObjectSet*         pAllBricks  = NULL;
    HGameObjectSetIterator  pHBrick;
    MAP_POSITION            position;
    UINT32                  numLines        = 0;
    UINT32                  maxLineLength   = 0;

    // Make sure the map is current (animations may have moved bricks).
    g_pGameMap->Update();


    for (UINT32 row = 0; row < g_pGameMap->GetHeight(); ++row)
    {
        for (UINT32 col = 0; col < g_pGameMap->GetWidth(); ++col)
        {
            position.x = col;
            position.y = row;
            
            // Check for contiguous runs of the current block in all directions
            GO_TYPE brickType = g_pGameMap->GetValue( position );
        
            if (0 == brickType || brickType & GO_TYPE_UNKNOWN)
            {
                continue;
            }
        
            for (UINT32 dir = 0; dir < g_pGameMap->numDirections; ++dir)
            {
                // TODO: game grid size is fixed; much cheaper to use a static array of HGameObjects
                // instead of allocating/deleting lists.
                HGameObjectSet* pBricks = FindContiguousBricks( brickType, position, g_pGameMap->directions[dir], minCount );
                
                if (!pBricks)
                {
                    continue;
                }
                
                
                if (!pAllBricks && pBricks->size() != 0)
                {
                    pAllBricks = new HGameObjectSet;
                }
                
                numLines++;
                maxLineLength = MAX(maxLineLength, pBricks->size());
                
                
                for (pHBrick = pBricks->begin(); pHBrick != pBricks->end(); ++pHBrick)
                {
                    pAllBricks->insert( *pHBrick );
                }
                
                delete pBricks;
            }
        }
    }
    
    if (pNumLines)
    {
        // Since we're not optimized about it, every line always gets counted twice
        // (front to back and back to front).
        *pNumLines = numLines/2;
    }
    
    if (pLongestLine)
    {
        *pLongestLine = maxLineLength;
    }
    
    return pAllBricks;
}



//=============================================================================
//
// Find contiguous bricks given a starting point, direction, type, and min count.
// Return them as a set.
//
//=============================================================================
HGameObjectSet*
ColumnState::FindContiguousBricks( GO_TYPE brickType, MAP_POSITION position, ivec2 direction, UINT32 minCount )
{
    HGameObjectSet*         pAllBricks = NULL;
    HGameObjectListIterator pHBrick;
    UINT32                  numBricks  = 0;

    if ( (0 == brickType || GO_TYPE_UNKNOWN == brickType)   ||
         (position.x > g_pGameMap->GetWidth())                  || 
         (position.y > g_pGameMap->GetHeight())                 ||
         (!direction.x && !direction.y) )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ColumnState::FindContiguousBricks(): invalid argument.");
        goto Exit;
    }
    
    // Scan for GameObjects of matching type in the indicated direction.
    while( (brickType & g_pGameMap->GetValue(position)) == brickType )
    {
        WORLD_POSITION worldPos;
        g_pGameMap->MapToWorldPosition( position, &worldPos );
#ifdef DEBUG        
//        DebugRender.Line( worldPos, vec3(direction.x, direction.y, 0), Color::Black(), 1.0f, 32.0f, 10.0f );
#endif        
    
        // Do NOT delete pBricks.  It's a naked pointer to the game map's internal list for that cell.
        // WARNING: if one GameObject in the cell matched, but there are others, this will return ALL of them.
        HGameObjectList* pBricks = g_pGameMap->GetGameObjects( position );
        DEBUGCHK(pBricks && pBricks->size() != 0);
        
        if (!pAllBricks)
        {
            pAllBricks = new HGameObjectSet;
        }
        
        
        for (pHBrick = pBricks->begin(); pHBrick != pBricks->end(); ++pHBrick)
        {
            pAllBricks->insert( *pHBrick );
        }
    
        // TODO: mark bricks as visited?
        // The problem with four or five in a row is that they get counted as multiple lines:
        // the greater line, and the lines within.
    
        // Move to next cell of the map.
        numBricks++;
        position += direction;

        // Stop if we hit the edge of the map.
        if (position.x >= g_pGameMap->GetWidth() || position.y >= g_pGameMap->GetHeight())
        {
            break;
        }
    }
    

    // If we found contiguous bricks, but not enough to satisfy the search, don't return them.
    if (numBricks < minCount)
    {
        SAFE_DELETE(pAllBricks);
        pAllBricks = NULL;
    }
    
Exit:    
    return pAllBricks;
}



//=============================================================================
//
// Find all hanging bricks on the map, and start them dropping to the ground.
//
// Method returns immediately, but hanging bricks will have drop animations 
// bound to them.  When the animations complete, the bricks will be deleted.
//
// Updates: global variable numDroppingBricks.
//
//=============================================================================
UINT32
ColumnState::DropAllBricks()
{
    UINT32 numDroppedBricks = 0;
    UINT32 dstRow           = 0;
    UINT32 srcRow           = 1;
    UINT32 width            = g_pGameMap->GetWidth();
    UINT32 height           = g_pGameMap->GetHeight();
    UINT32 dropLength       = 0;
    

    // Make sure the map is current (animations may have moved bricks).
    g_pGameMap->Update();

    
    // Reset the blocks-are-dropping timer.
    g_currentDropDurationMS = 0;


    for (UINT32 column = 0; column < width; ++column)
    {
        for (dstRow = 0, srcRow = 1; srcRow < height; ++srcRow, ++dstRow)
        {
            // Scan for an empty cell
            if (g_pGameMap->GetNumberOfObjects( column, dstRow ) > 0)
            {
                continue;
            }
            else 
            {
                // Found an empty cell; scan for first non-empty cell up above.
                while (srcRow < height && g_pGameMap->GetNumberOfObjects( column, srcRow ) <= 0)
                {
                    ++srcRow;
                }
            
                // If all cells above this one are empty, continue to next column.
                if (srcRow >= height)
                {
                    continue;
                }
    
                DEBUGCHK(g_pGameMap->GetNumberOfObjects( column, dstRow ) <= 0);
                
                // Drop all bricks in this cell and above
                while ( srcRow < height )
                {
                    // Drop all Bricks in the cell.
                    HGameObjectList* pHGameObjects = g_pGameMap->GetGameObjects( column, srcRow );
                    HGameObjectListIterator pHGameObject;
                    for (pHGameObject = pHGameObjects->begin(); pHGameObject != pHGameObjects->end(); ++pHGameObject)
                    {
                        // What is the furthest length that a block must drop?
                        // Used to wait for drop animations to complete, before
                        // checking for contiguous blocks again.
                        dropLength    = srcRow - dstRow;

                        //
                        // Create a drop animation.
                        //
                        
                        // BUG BUG: is it a problem to make these relative?
                        // What if a block is already dropping when we kick off the animation?
                        // It's end position would be WRONG.
                        // TODO: apply floor function to position before binding animation.
  
                        // Add a few milliseconds to each drop animation so the bricks land at different times, 
                        // creating a nice audio/visual effect.
                        UINT64      durationMS      = dropLength * BLOCK_DROP_SPEED_MS + (Platform::Random() % 200);
                        HStoryboard hDropStoryboard = CreateDropAnimation( *pHGameObject, vec3(0,0,0), vec3(0, -COLUMN_BRICK_HEIGHT*(float)dropLength, 0), durationMS, true );
                        StoryboardMan.Start  ( hDropStoryboard );
                        
                        g_currentDropDurationMS = MAX(g_currentDropDurationMS, durationMS);
                        
                        numDroppedBricks++;
                    }
                
                    srcRow++;
                    dstRow++;
                }
            } // END for each GameObject
        } // END for each row
    } // END for each column


    if (numDroppedBricks > 0)
    {
        DEBUGMSG(ZONE_INFO, "Dropped %d bricks", numDroppedBricks);
    }

    g_numDroppingBricks += numDroppedBricks;

    return numDroppedBricks;
}



HStoryboard
ColumnState::CreateDropAnimation( HGameObject hTarget, WORLD_POSITION start, WORLD_POSITION end, UINT64 durationMS, bool deleteOnFinish )
{
    KeyFrame keyFrames[2];
    keyFrames[0].SetTimeMS(0);
    keyFrames[0].SetVec3Value( start );
    keyFrames[1].SetTimeMS( durationMS );
    keyFrames[1].SetVec3Value( end   );


    HAnimation  hDropAnimation;
    HStoryboard hDropStoryboard;
    AnimationMan.CreateAnimation( "BrickPosition", "Position", PROPERTY_VEC3, INTERPOLATOR_TYPE_QUADRATIC_IN, KEYFRAME_TYPE_VEC3, &keyFrames[0], 2, true, &hDropAnimation );
 
    if (deleteOnFinish)
    {
        Callback droppedCallback( ColumnState::DropBrickCallback );
        AnimationMan.CallbackOnFinished( hDropAnimation, droppedCallback );
    }
    
    StoryboardMan.CreateStoryboard( "", &hDropAnimation, 1, false, false, true, true, false, &hDropStoryboard );
    AnimationMan.Release( hDropAnimation );

    StoryboardMan.BindTo ( hDropStoryboard, hTarget );
    
    return hDropStoryboard;
}


void
ColumnState::DropColumnToBottom()
{
    RETAILMSG(ZONE_STATEMACHINE, "ColumnState: MSG_ColumnDropToBottom");
    
    UINT8 dropLength = 0;
    
    // Move Column down until it colides.
    while ( ! (CollisionCheck() & COLLISION_BELOW) )
    {
        WORLD_POSITION brickPos = GOMan.GetPosition( hColumnBricks[0] );
        
        brickPos.y -= COLUMN_BRICK_HEIGHT;
        GOMan.SetPosition( hColumnBricks[0], brickPos ); brickPos.y -= COLUMN_BRICK_HEIGHT;
        GOMan.SetPosition( hColumnBricks[1], brickPos ); brickPos.y -= COLUMN_BRICK_HEIGHT;
        GOMan.SetPosition( hColumnBricks[2], brickPos );
        
        dropLength++;
        
        g_pGameMap->Update();
    }
    
    // Create GameObject with storyboard and start.
    // When the storyboard ends, it will delete itself and the GameObject.
    vec3 position = GOMan.GetPosition( hColumnBricks[0] );
    
    // TODO: replace "Score10" with a textbox containing the actual drop score.
    GOMan.Create( "Score10", "ScoreBrick", hPlayScreenAlerts, position, NULL, 1.0f, Color::Gold() );
    
    // Award points for dropping blocks early.
    g_totalScore += dropLength * 10 * g_difficultyMultiplier;
    
//    if (dropLength > 0)
//        SendMsgToStateMachine( MSG_UpdateScore );
    
    SoundMan.Play( hDrop );
    StoryboardMan.Start( hDropColumnStoryboard );
    
    // Drop "swoosh"
    HGameObject handle;
    position = GOMan.GetPosition( hColumnBricks[2] );
    GOMan.Create( "swoosh_drop", "DropColumnSwoosh", hPlayScreenForeground, position, &handle );

    // Critter makes a face when moved.
    for (int i = 0; i < NUM_BRICKS_PER_COLUMN; ++i)
    {
        GOMan.SendMessageFromSystem(hColumnBricks[i], MSG_DropCritterToBottom);
    }
    
    
    // Dust cloud centered under column
    // TODO: add as child of Column?
    position.x += COLUMN_BRICK_WIDTH/2.0f;
    HParticleEmitter hEmitter;
    Particles.GetCopy( "DustCloud", &hEmitter );
    Particles.SetPosition( hEmitter, position );
    LayerMan.AddToLayer( hPlayScreenForeground, hEmitter );
    Particles.Start( hEmitter );

    
    CheckForSmashedBricks( true );
}    



GO_TYPE
ColumnState::BombInColumn( HGameObject* pHBricks, UINT8 numHBricks )
{
    GO_TYPE rval      = GO_TYPE_UNKNOWN;
    GO_TYPE brickType = GO_TYPE_UNKNOWN;
    
    for (UINT8 i = 0; i < numHBricks; ++i)
    {
        brickType = GOMan.GetType( pHBricks[i] );
        if ( brickType & (GO_TYPE_VERTICAL_BOMB | GO_TYPE_RADIAL_BOMB) )
        {
            // Return the bomb type; mask off GO_TYPE_SPRITE and other bits.
            rval = (GO_TYPE)(brickType & (GO_TYPE_VERTICAL_BOMB | GO_TYPE_RADIAL_BOMB));
            break;
        }
    }
    
    return rval;
}



UINT8
ColumnState::CheckForSmashedBricks( bool wasFlung )
{
    UINT8 numSmashed = 0;

    //
    // Smash bricks if a bomb has landed.
    //
    GO_TYPE bombType = BombInColumn( hColumnBricks, ARRAY_SIZE(hColumnBricks) );
    if (bombType != GO_TYPE_UNKNOWN)
    {
        // Find the bottom-most bomb.
        WORLD_POSITION pos;
        for (int i = 0; i < NUM_BRICKS_PER_COLUMN; ++i)
        {
            if (GOMan.GetType(hColumnBricks[i]) & (GO_TYPE_VERTICAL_BOMB | GO_TYPE_RADIAL_BOMB))
            {
                pos = GOMan.GetPosition(hColumnBricks[i]);
                break;
            }
        }

        numSmashed = SmashBricks( pos, bombType, wasFlung );
    }


    // Update score based on numSmashed.
    // TODO: clean up the scoring and put it all in one place.
    UINT8 brickScore = 0;
    switch (numSmashed) 
    {
        case 0:
            brickScore = 0;
            break;
        case 1:
        case 2:
        case 3:
            brickScore = 10;
            break;
        case 4:
            brickScore = 15;
            break;
        case 5:
            brickScore = 30;
            break;
        case 6:
            brickScore = 100;
            break;
        default:
            brickScore = 200;
            break;
    }
    
    g_totalScore += numSmashed * brickScore * g_pLevel->scoreMultiplier * g_difficultyMultiplier;
    
    if (numSmashed > 0)
        SendMsgToStateMachine( MSG_UpdateScore );


    return numSmashed;
}


// TODO: move all of this into the Bomb state machine!!!!

UINT8
ColumnState::SmashBricks( const WORLD_POSITION& pos, GO_TYPE bombType, bool wasFlung, bool chainReaction )
{
    UINT8 numSmashed = 0;
    
    // Find the set of bricks to smash.
    HGameObjectSet  smashedBricks;
    MAP_POSITION    mapPos;
    
    // Do not include current position in the search; look down one cell.
    g_pGameMap->WorldToMapPosition(pos, &mapPos);
    

    switch (bombType)
    {
        case GO_TYPE_VERTICAL_BOMB:
        {
            // bombs flicked downards do much more damage.
            if (wasFlung)
            {
                // Clear all game pieces above and below the brick
                g_pGameMap->FindAll(mapPos, ivec2(0,-1), GAME_GRID_NUM_ROWS, GO_TYPE_ANY, &smashedBricks);
                g_pGameMap->FindAll(mapPos, ivec2(0, 1), GAME_GRID_NUM_ROWS, GO_TYPE_ANY, &smashedBricks);

                if (chainReaction)
                {
                    // Clear all game pieces left and right of the brick
                    g_pGameMap->FindAll(mapPos, ivec2(-1,0), GAME_GRID_NUM_COLUMNS, GO_TYPE_ANY, &smashedBricks);
                    g_pGameMap->FindAll(mapPos, ivec2( 1,0), GAME_GRID_NUM_COLUMNS, GO_TYPE_ANY, &smashedBricks);
                }
            }
            else
            {
                g_pGameMap->FindAll(mapPos, ivec2(0,-1), 4, GO_TYPE_ANY, &smashedBricks);
            }
        }
        break;

        case GO_TYPE_RADIAL_BOMB:
        {
            // bombs flicked downards do much more damage.
            if (wasFlung)
            {
                g_pGameMap->FindAll(mapPos, 3.0, GO_TYPE_ANY, &smashedBricks);
            }
            else
            {
                g_pGameMap->FindAll(mapPos, 1.0, GO_TYPE_ANY, &smashedBricks);
            }
        }
        break;
    
        default:
            RETAILMSG(ZONE_ERROR, "ERROR: ColumnState::SmashBricks(): unknown bombType %d", (UINT32)bombType);
            DEBUGCHK(0);
    }
    

    DEBUGMSG(ZONE_INFO, "SmashBrick: %d bricks", smashedBricks.size());


    // Queue the blocks for smashing one after another (feels better than smashing simultaneously).
    HGameObjectSetIterator pHBrick;
    for (pHBrick = smashedBricks.begin(); pHBrick != smashedBricks.end(); ++pHBrick)
    {
        HGameObject hBrick = *pHBrick;
        
        g_smashedBricks.insert( hBrick );
        
        numSmashed++;
    }


    if (numSmashed > 0)
    {
        g_pGameMap->Update();
    }

Exit:
    return numSmashed;
}


bool
ColumnState::MoveColumnLeft()
{
    RETAILMSG(ZONE_STATEMACHINE, "ColumnState: MoveColumnLeft");
    
    // Don't slide left if there's a block in the way
    if (CollisionCheck() & COLLISION_LEFT)
    {
        return false;
    }

    
    // HACK HACK: for now move each Brick directly
    WORLD_POSITION pos;
    for (int i = 0; i < NUM_BRICKS_PER_COLUMN; ++i)
    {
        pos = GOMan.GetPosition( hColumnBricks[i] );
        pos.x -= COLUMN_BRICK_WIDTH;
        pos.x = CLAMP(pos.x, GAME_GRID_LEFT, GAME_GRID_RIGHT);
        GOMan.SetPosition( hColumnBricks[i], pos );

        // Critter makes a face when moved.
        GOMan.SendMessageFromSystem(hColumnBricks[i], MSG_MoveCritterLeft);
    }

    // Move the Column GO, even though it's an invisible container for the bricks.
    GOMan.SetPosition( hColumn, pos );

    g_pGameMap->Update();
    
    SoundMan.Play( hSlideLeft );
    
    return true;
}


bool
ColumnState::MoveColumnRight()
{
    RETAILMSG(ZONE_STATEMACHINE, "ColumnState: MoveColumnRight");
    
    // Don't slide right if there's a block in the way
    if (CollisionCheck() & COLLISION_RIGHT)
    {
        return false;
    }
        
    // HACK HACK: for now move each Brick directly
    WORLD_POSITION pos;
    for (int i = 0; i < NUM_BRICKS_PER_COLUMN; ++i)
    {
        pos = GOMan.GetPosition( hColumnBricks[i] );
        pos.x += COLUMN_BRICK_WIDTH;
        pos.x = CLAMP(pos.x, GAME_GRID_LEFT, GAME_GRID_RIGHT);
        GOMan.SetPosition( hColumnBricks[i], pos );

        // Critter makes a face when moved.
        GOMan.SendMessageFromSystem(hColumnBricks[i], MSG_MoveCritterRight);
    }

    // TEST: move the Column GO, even though it's an invisible container for the bricks.
    GOMan.SetPosition( hColumn, pos );

    g_pGameMap->Update();
    
    SoundMan.Play( hSlideRight );

    return true;
}


void
ColumnState::RotateColumn()
{
    RETAILMSG(ZONE_STATEMACHINE, "ColumnState: MSG_ColumnRotate");
    
    WORLD_POSITION pos = GOMan.GetPosition( hColumnBricks[0] );
    GOMan.SetPosition( hColumnBricks[0], GOMan.GetPosition( hColumnBricks[1] ));
    GOMan.SetPosition( hColumnBricks[1], GOMan.GetPosition( hColumnBricks[2] ));
    GOMan.SetPosition( hColumnBricks[2], pos );
    
    HGameObject hGO  = hColumnBricks[2];
    hColumnBricks[2] = hColumnBricks[1];
    hColumnBricks[1] = hColumnBricks[0];
    hColumnBricks[0] = hGO;
    
    g_pGameMap->Update();
    
    pos = GOMan.GetPosition( hColumnBricks[1] );
    

    // Rotate "glow" sprite
    // TODO: set anchor point at center of sprite.

    // Create and bind a storyboard; storyboard will delete the sprite when it's finished.
    HGameObject handle;
//    GOMan.Create( "swoosh_rotate", "RotateColumnSwoosh", hPlayScreenGrid, pos, &handle);  // TODO: whenever we get a handle, it should have +1 refcount/need release!
    GOMan.Create( "critters_highlight", "RotateColumnSwoosh", hPlayScreenGrid, pos, &handle, 1.0, Color::Gold());  // TODO: whenever we get a handle, it should have +1 refcount/need release!

    // Center the swoosh on the column.
    GOMan.SetScale( handle, 1.5f );
    AABB bounds = GOMan.GetBounds(handle);
    pos.x -= bounds.GetWidth()/2;
    pos.y -= bounds.GetHeight()/2;
    pos.x += COLUMN_BRICK_WIDTH/2;
    pos.y += COLUMN_BRICK_HEIGHT/2;
 
    GOMan.SetPosition( handle, pos );

    for (int i = 0; i < NUM_BRICKS_PER_COLUMN; ++i)
    {
        // Critter makes a face when moved.
        GOMan.SendMessageFromSystem(hColumnBricks[i], MSG_MoveCritterWithinColumn);
    }


//    HParticleEmitter hEmitter;
//    Particles.GetCopy( "Bubbles", &hEmitter );
//    Particles.SetPosition( hEmitter, pos );
//    LayerMan.AddToLayer( hPlayScreenForeground, hEmitter );
//    Particles.Start( hEmitter );

    SoundMan.Play( hRotate );
}



//=============================================================================
//
// Updates the score based on number of contiguous bricks, longest line,
// number of lines, and number of chains.
//
// Method returns immediately, but scoring animations are started for each brick.
// The scoring animations will delete themselves when finished.
//
//=============================================================================
UINT32
ColumnState::UpdateScore( HGameObjectSet* pBricks, UINT8 longestLine, UINT8 numLines, UINT8 numChains )
{
    UINT32  score       = 0;
    UINT8   brickScore  = 0;
    UINT8   numBricks   = 0;

    if (!pBricks)
    {
        DEBUGCHK(0);
        return 0;
    }


    //
    // Compute the score.
    //
    
    numBricks = pBricks->size();

    switch (longestLine) 
    {
        case 3:
            brickScore = 10;
            break;
        case 4:
            brickScore = 15;
            break;
        case 5:
            brickScore = 30;
            break;
        case 6:
            brickScore = 100;
            break;
        default:
            DEBUGCHK(0);
            break;
    }
    

    if (numChains > 0 && brickScore < 50)
    {
        brickScore = 50;
    }

    if (numBricks > 6 && brickScore < 100)
    {
        brickScore = 100;
    }

    score = brickScore * numBricks;
    
    for (int i = 0; i < numChains; ++i)
    {
        score += 1000;
    }
    

    HGameObject    hBrick   = *(pBricks->begin());
    WORLD_POSITION position = GOMan.GetPosition( hBrick );

    // Score multiplier per level
    score *= g_pLevel->scoreMultiplier;

    // Difficulty multiplier
    score *= g_difficultyMultiplier;
    
    // 
    // Alert the user if they beat their previous high score.
    //
    if ( !g_newHighScore && g_totalScore > g_highScore && !g_isTutorialMode)
    {
        // TODO: custom High Score sound, particles, maybe a separate state.

        // Don't show alert the very first time; not fun to get "high score" on move one.
        if (g_highScore != 0)
        {
            // StarBurst
            HParticleEmitter hEmitter;
            Particles.GetCopy( "StarBurst", &hEmitter );
            Particles.SetPosition( hEmitter, position );
            LayerMan.AddToLayer( hPlayScreenForeground, hEmitter );
            Particles.Start( hEmitter );


//            for (int i = 0; i < 20; ++i)
//            {
//                float delay = Platform::RandomDouble(0.1f, 1.0f);
//                SendMsgDelayedToStateMachine(delay, MSG_Fireworks);
//            }

            position = WORLD_POSITION( 0.0, 0.0, 0.0 );
            SoundMan.Play( hChain1 );
            GOMan.Create( "highscore", "HighScore", hPlayScreenAlerts, position, NULL, 1.0f, Color::Red(), true );
            
            #ifdef USE_OPENFEINT
            // Submit score to OpenFeint.
            // TODO: is this always a fast return?!
            OpenFeint.SubmitHighScore( g_highScore );
            #endif
        }

        g_highScore     = g_totalScore;
        g_newHighScore  = true;
    }
    

    //
    // Update stats.
    //
    g_gameNumLinesCleared   += numLines;
    g_gameNumChains         += numChains;
    g_gameNumBlocksCleared  += numBricks;


    RETAILMSG(ZONE_INFO, "*** UpdateScore(): %d bricks, longestLine: %d numLines: %d numChains: %d SCORE = %d TOTAL SCORE = %d", 
        pBricks->size(), longestLine, numLines, numChains, score, g_totalScore);

    
    return score;
}



//=============================================================================
//
// States govering the Column lifecycle, movement, etc.
//
//=============================================================================
bool ColumnState::States( State_Machine_Event event, MSG_Object * msg, int state, int substate )
{
BeginStateMachine

	//Global message responses
	OnMsg( MSG_Reset )
		ResetStateMachine();
        DEBUGMSG(ZONE_STATEMACHINE, "ColumnState: MSG_Reset");

    OnMsg( MSG_UpdateScore )
        // Animate the on-screen score when it changes
        if (Storyboards.IsStopped( hUpdateScoreStoryboard ))
        {
            Storyboards.Start( hUpdateScoreStoryboard );
        }
    

    OnMsg( MSG_Firework )
        Rectangle screen;
        Platform::GetScreenRect(&screen);
        int x = Platform::Random(0, screen.width);
        int y = Platform::Random(0, screen.height);
        vec3 position(x, y, 0);
    
        HParticleEmitter hSparksEmitter;
        Particles.GetCopy( "Sparks", &hSparksEmitter );
        Particles.SetPosition( hSparksEmitter, position );
        LayerMan.AddToLayer( hPlayScreenForeground, hSparksEmitter );
        Particles.Start( hSparksEmitter );

    OnMsg( MSG_Fireworks )
        for (int i = 0; i < 50; ++i)
        {
            float delay = Platform::RandomDouble(0.25f, 3.5f);
            SendMsgDelayedToStateMachine(delay, MSG_Firework);
        }


    OnMsg( MSG_TutorialCreateColumn )
        //ChangeState( STATE_CreateColumn );
        WORLD_POSITION position;
        if (Platform::IsWidescreen()) {
            position = WORLD_POSITION( GAME_GRID_LEFT + COLUMN_BRICK_WIDTH*3, GAME_GRID_TOP, 0 );
        } else {
            position = WORLD_POSITION( GAME_GRID_LEFT + COLUMN_BRICK_WIDTH*3, GAME_GRID_TOP_3_5, 0 );
        }
        BrickType* pBrickTypes = (BrickType*)msg->GetPointerData();
        CreateColumn( position, pBrickTypes, 3 );
        ChangeState( STATE_TutorialMode );
        

    OnMsg( MSG_GameOver )
//        // Move in-play blocks to the grid layer so they get cleaned up properly.
//        for (int i = 0; i < NUM_BRICKS_PER_COLUMN; ++i)
//        {
//            g_pGameMap->AddGameObject( hColumnBricks[i] );
//        
//            LayerMan.AddToLayer     ( hPlayScreenGrid,   hColumnBricks[i] );
//            LayerMan.RemoveFromLayer( hPlayScreenColumn, hColumnBricks[i] );
//        }
        ChangeState( STATE_GameOver );


    OnMsg( MSG_KillCritter )
        HGameObjectList gobs = g_pGameMap->GetListOfAllGameObjects();
        HGameObjectListIterator phGOB;
    
        for (phGOB = gobs.begin(); phGOB != gobs.end(); ++phGOB)
        {
        //
        // Create drop animation.
        //
        WORLD_POSITION  startPos,    endPos;
        MAP_POSITION    startMapPos, endMapPos;
        
        HGameObject hGOB = *phGOB;
        
        startPos = GOMan.GetPosition( hGOB );
        endPos   = startPos;
        
        if (Platform::IsWidescreen()) {
            endPos.y = GAME_GRID_BOTTOM;
        } else {
            endPos.y = GAME_GRID_BOTTOM_3_5;
        }
        
        // Add some random-ness so they pile up
        endPos.x += Platform::RandomDouble(-COLUMN_BRICK_WIDTH, COLUMN_BRICK_WIDTH);
        //endPos.y += Platform::RandomDouble(0.0, (startPos.y - endPos.y)/(GAME_GRID_NUM_ROWS*COLUMN_BRICK_WIDTH) * COLUMN_BRICK_HEIGHT);
        
        
        g_pGameMap->WorldToMapPosition( startPos, &startMapPos );
        g_pGameMap->WorldToMapPosition( endPos,   &endMapPos   );
        
        UINT64 dropDurationMS  = (startMapPos.y - endMapPos.y) * BLOCK_DROP_SPEED_MS + (Platform::Random() % 500);

        KeyFrame keyFrames[2];
        keyFrames[0].SetTimeMS(0);
        keyFrames[0].SetVec3Value( startPos );
        keyFrames[1].SetTimeMS( dropDurationMS );
        keyFrames[1].SetVec3Value( endPos   );

        HAnimation  hDropAnimation;
        HStoryboard hDropStoryboard;
        AnimationMan.CreateAnimation( "", "Position", PROPERTY_VEC3, INTERPOLATOR_TYPE_QUADRATIC_IN, KEYFRAME_TYPE_VEC3, &keyFrames[0], 2, false, &hDropAnimation );

        StoryboardMan.CreateStoryboard( "",                     // Storyboard name
                                        &hDropAnimation,        // HAnimations[]
                                        1,                      // numAnimations
                                        false,                  // autoRepeat
                                        false,                  // autoReverse
                                        true,                   // releaseTargetOnFinish
                                        true,                   // deleteOnFinish
                                        false,                  // isRelative
                                        &hDropStoryboard        // HStoryboard
                                       );
        
        StoryboardMan.BindTo( hDropStoryboard, hGOB );
        StoryboardMan.Start( hDropStoryboard );
        AnimationMan.Release( hDropAnimation );

        // Play a thump sound on landing.
        g_numDroppingBricks++;
        Callback callback( ColumnState::DropBrickCallback );
        StoryboardMan.CallbackOnFinished( hDropStoryboard, callback );
        }
    

    #pragma mark MSG_GameObjectWasTapped
    OnMsg( MSG_GameObjectWasTapped )
        RETAILMSG(ZONE_STATEMACHINE, "ColumnState: MSG_GameObjectWasTapped");
        if (GetState() != STATE_SmashBricks) // HACK
        {
            HandleGameObjectTapped( msg );
        }
        

    #pragma mark MSG_Detonate
    OnMsg( MSG_Detonate )
        RETAILMSG(ZONE_STATEMACHINE, "ColumnState: MSG_Detonate");
        {
            HandleGameObjectTapped( msg );
            HGameObject handle = *(HGameObject*)msg->GetPointerData();
            handle.Release();
        }
        

    #pragma mark MSG_PauseGame
    OnMsg( MSG_PauseGame )
        RETAILMSG(ZONE_STATEMACHINE, "ColumnState: MSG_PauseGame");
        if (GetState() != STATE_Paused)
        {
            if (GetState() != STATE_ResumeFromPause)
            {
                g_stateWhenUnpaused = (Z::StateName)GetState(); // UGH
            }

            RETAILMSG(ZONE_INFO, "Pausing ColumnState; resume will change to state %d", g_stateWhenUnpaused);
            ChangeState( STATE_Paused );
        }
                
    #pragma mark MSG_NewGame
    OnMsg( MSG_NewGame )
        // TODO: move this to GameScreens!

        RETAILMSG(ZONE_STATEMACHINE, "ColumnState: MSG_NewGame");

        //
        // Reset stats.
        //
        g_pLevel                = &g_levels[ g_level ];
        g_gameNumBlocksCleared  = 0;
        g_gameNumLinesCleared   = 0;
        g_gameNumChains         = 0;
        g_startGameTime         = GameTime.GetTime();
        g_numDroppingBricks     = 0;

        g_scoreToLevelUp       = g_levels[ g_level ].scoreToLevelUp;

        g_highestLevelEver      = UserSettings.GetInt( "/UserPrefs.HighestLevel", g_highestLevelEver );

        // TODO: move this to a dedicated STATE; it is unsafe to delete GameObjects
        // in a msg handler.
        
        // Remove any bricks hanging out from a previous game.
        HGameObjectList list = g_pGameMap->GetListOfAllGameObjects();
        HGameObjectListIterator phGO;
        for (phGO = list.begin(); phGO != list.end(); ++phGO)
        {
            LayerMan.RemoveFromLayer( hPlayScreenGrid,   *phGO );
            LayerMan.RemoveFromLayer( hPlayScreenColumn, *phGO );
            GOMan.Remove( *phGO );
        }

        LayerMan.Clear( hPlayScreenColumn     );
        LayerMan.Clear( hPlayScreenGrid       );

        g_pGameMap->Clear();

        
        float delay = StoryboardMan.GetDurationMS( "FadeInBackground" )/1000.0f;
        delay      += StoryboardMan.GetDurationMS( "NewGame"          )/1000.0f;
        ChangeStateDelayed( delay, STATE_CreateColumn );


    #pragma mark MSG_GameScreenHome
    OnMsg( MSG_GameScreenHome )
        // Release any bricks that were in play from previous game.
        for (int i = 0; i < NUM_BRICKS_PER_COLUMN; ++i)
        {
            LayerMan.RemoveFromLayer( hPlayScreenColumn, hColumnBricks[i] );
            GOMan.Release( hColumnBricks[i] );
        }



    // NOTE: we have global handlers for some movement messages so that these messages aren't 
    // DROPPED if they happen during a state change.
    //
    // E.G. the delayed transition from STATE_DropColumn -> STATE_DropColumn.
    // Even though it's the same state, this counts as a state transition, and messages delivered in the
    // interrim will NOT be received.  I think this may be a bug in the StateMachine scoping rules.    
    //
    // Handling these globally ensures that column drag and rotate operations can be started at any time.
    // Without this the user may try to drag a column and find it unresponsive.
    //
    // However, MSG_TouchEnd is still processed only in STATE_DropColumn.  
    // So rotates are prevented if MSG_TouchEnd happens at an "illegal" time, such as after the column has landed.
    #pragma mark MSG_TouchBegin
    OnMsg( MSG_TouchBegin )
        HandleTouch( MSG_TouchBegin, (TouchEvent*)msg->GetPointerData() );


    ///////////////////////////////////////////////////////////////
    #pragma mark -
    #pragma mark STATE_Initialize
	DeclareState( STATE_Initialize )

        // TODO: move much of this into a game screen controller.
    
		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "ColumnState: STATE_Initialize");

//            if (Platform::IsWidescreen()) {
//                g_pGameMap = new ColumnBrickMap(
//                                     GAME_GRID_NUM_COLUMNS,     // columns
//                                     GAME_GRID_NUM_ROWS+1,      // rows
//                                     COLUMN_BRICK_WIDTH,        // cell width
//                                     GAME_GRID_LEFT,            // X offset in world coordinates
//                                     GAME_GRID_BOTTOM,          // Y offset in world coordinates
//                                     0.0,                       // Z offset in world coordinates
//                                     true                       // isVertical
//                                   );
//            } else {
//                g_pGameMap = new ColumnBrickMap(
//                                     GAME_GRID_NUM_COLUMNS,     // columns
//                                     GAME_GRID_NUM_ROWS_3_5+1,  // rows
//                                     COLUMN_BRICK_WIDTH,        // cell width
//                                     GAME_GRID_LEFT,            // X offset in world coordinates
//                                     GAME_GRID_BOTTOM_3_5,      // Y offset in world coordinates
//                                     0.0,                       // Z offset in world coordinates
//                                     true                       // isVertical
//                                   );
//            }

            LayerMan.Get    ( "PlayScreenBackground",   &hPlayScreenBackground );
            LayerMan.Get    ( "PlayScreenGrid",         &hPlayScreenGrid       );
            LayerMan.Get    ( "PlayScreenColumn",       &hPlayScreenColumn     );
            LayerMan.Get    ( "PlayScreenForeground",   &hPlayScreenForeground );
            LayerMan.Get    ( "PlayScreenAlerts",       &hPlayScreenAlerts     );

            SoundMan.Get    ( "SlideLeft",              &hSlideLeft         );
            SoundMan.Get    ( "SlideRight",             &hSlideRight        );
            SoundMan.Get    ( "SlideDown",              &hSlideDown         );
            SoundMan.Get    ( "Rotate",                 &hRotate            );
            SoundMan.Get    ( "Drop",                   &hDrop              );
            SoundMan.Get    ( "Clink",                  &hClink             );
            SoundMan.Get    ( "Score1",                 &hScore1            );
            SoundMan.Get    ( "Score2",                 &hScore2            );
            SoundMan.Get    ( "Score3",                 &hScore3            );
            SoundMan.Get    ( "Chain1",                 &hChain1            );
            SoundMan.Get    ( "Chain2",                 &hChain2            );
            SoundMan.Get    ( "Chain3",                 &hChain3            );
            SoundMan.Get    ( "SmashBlock",             &hSmashBlock        );
            SoundMan.Get    ( "AwesomeExplosion",       &hMegaChainSound    );

            // Get a handle to our parent GameObject, so we can get/set the position,
            // but do NOT retain it.  Children should never retain parents, else
            // the parent will never be freed.
            GOMan.Get       ( m_owner->GetName(),       &hColumn            );
            GOMan.Release   ( hColumn );

            EffectMan.GetCopy( "RippleEffect", &hRippleEffect );

            StoryboardMan.GetCopy( "DelayedCameraShake",    &hCameraShakeStoryboard      );
            StoryboardMan.GetCopy( "SmashCameraShake",      &hSmashCameraShakeStoryboard );
            StoryboardMan.GetCopy( "CameraZoom",            &hCameraZoomStoryboard       );

            StoryboardMan.GetCopy( "DropColumn", &hDropColumnStoryboard );
            StoryboardMan.BindTo ( hDropColumnStoryboard, hPlayScreenGrid );    // Not a typo; we move the entire layer down rather than each piece of the column.

            Storyboards.GetCopy( "UpdateScore", &hUpdateScoreStoryboard );
            Storyboards.BindTo( hUpdateScoreStoryboard, &g_scoreScale );
    

            if (Log::IsZoneEnabled(ZONE_MAP))
            {
                g_pGameMap->EnableDebugDisplay( true );
            }

            ChangeState( STATE_Idle );
	

	///////////////////////////////////////////////////////////////
    #pragma mark STATE_Paused
    DeclareState( STATE_Paused )

        #pragma mark MSG_UnpauseGame
        OnMsg( MSG_UnpauseGame )
            RETAILMSG(ZONE_STATEMACHINE, "ColumnState: MSG_UnpauseGame");
            // Go to STATE_ResumeFromPause so the player can collect his/her-self.
            // Will return to the current state of the game after a delay.
            ///////g_stateWhenUnpaused = (Z::StateName)GetState();
            ChangeState( STATE_ResumeFromPause );

        OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "ColumnState: STATE_Paused");
            
        OnExit
            RETAILMSG(ZONE_STATEMACHINE, "ColumnState: STATE_Paused: Exit");

    
	///////////////////////////////////////////////////////////////
    #pragma mark STATE_ResumeFromPause
    DeclareState( STATE_ResumeFromPause )

        OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "ColumnState: STATE_ResumeFromPause");
            if (g_stateWhenUnpaused == STATE_CreateColumn)
            {
                ChangeStateDelayed(1.0f, STATE_DropColumn);
            }
            else if (g_stateWhenUnpaused == STATE_Idle)
            {
                ChangeStateDelayed(1.0f, STATE_CheckForCompletedLines);
            }
            else
            {
                ChangeStateDelayed(1.0f, g_stateWhenUnpaused);
            }
    
    
    ///////////////////////////////////////////////////////////////
    #pragma mark STATE_CreateColumn
	DeclareState( STATE_CreateColumn )

        #pragma mark MSG_TouchUpdate
        OnMsg( MSG_TouchUpdate )
            HandleTouch( MSG_TouchUpdate, (TouchEvent*)msg->GetPointerData() );
        
        #pragma mark MSG_TouchEnd
        OnMsg( MSG_TouchEnd )
            HandleTouch( MSG_TouchEnd, (TouchEvent*)msg->GetPointerData() );

//        #pragma mark MSG_ColumnDropOneUnit
//        OnMsg( MSG_ColumnDropOneUnit )
//            ChangeState( STATE_DropColumn );

        #pragma mark MSG_ColumnLeft
        OnMsg( MSG_ColumnLeft )
            RETAILMSG(ZONE_INFO, "ColumnState: MSG_ColumnLeft: STATE_CreateColumn");
            MoveColumnLeft();
        
        
        #pragma mark MSG_ColumnRight
        OnMsg( MSG_ColumnRight )
            RETAILMSG(ZONE_INFO, "ColumnState: MSG_ColumnRight: STATE_CreateColumn");
            MoveColumnRight();
        
        
        #pragma mark MSG_ColumnRotate
        OnMsg( MSG_ColumnRotate )
            RETAILMSG(ZONE_INFO, "ColumnState: MSG_ColumnRotate: STATE_CreateColumn");
            RotateColumn();
        

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "ColumnState: STATE_CreateColumn");

            g_longestLine       = 0;
            g_numLines          = 0;
            g_numChains         = 0;
//            g_numBlocksCleared  = 0;

            // HACK: reset any current gesture when a column is created, so that previous swipe doesn't carry over.
            if (lastTouchEvent.timestamp != 0)
            {
                GOMan.SendMessageFromSystem( MSG_TouchEnd, NULL );
            }

            // HACK HACK: in some rare circumstances a column is left dangling on-screen; forcibly clear it here
            for (int i = 0; i < NUM_BRICKS_PER_COLUMN; ++i)
            {
                if (SUCCEEDED(LayerMan.RemoveFromLayer( hPlayScreenColumn, hColumnBricks[i] )))
                {
                    GOMan.Remove( hColumnBricks[i] );
                }
            }

            WORLD_POSITION position;
            if (Platform::IsWidescreen()) {
                position = WORLD_POSITION( GAME_GRID_LEFT + COLUMN_BRICK_WIDTH*3, GAME_GRID_TOP, 0 );
            } else {
                position = WORLD_POSITION( GAME_GRID_LEFT + COLUMN_BRICK_WIDTH*3, GAME_GRID_TOP_3_5, 0 );
            }
    
            CreateColumn( position, NULL, 3 );
            
            
            // We must check for completed lines BEFORE dropping the column,
            // for the rare occasion when a column is created at the top of the pile
            // but matches what's below it.  Otherwise the column will materialize,
            // and then it's game over (the player feels cheated out of the last match).
            pClearedBricks = FindContiguousBricks( 3, &g_longestLine, &g_numLines );
            
            if (pClearedBricks && pClearedBricks->size() > 0)
            {
                ChangeState( STATE_ShowCompletedLines );
            }
            else
            {
                ChangeStateDelayed( TimeUntilDrop(), STATE_DropColumn );
            }


    ///////////////////////////////////////////////////////////////
    #pragma mark STATE_TutorialMode
	DeclareState( STATE_TutorialMode )

        #pragma mark MSG_ColumnDropOneUnit
        OnMsg( MSG_ColumnDropOneUnit );
            RETAILMSG(ZONE_INFO, "TutorialMode: MSG_ColumnDropOneUnit");

            // HACK HACK: for now move each Brick directly
            WORLD_POSITION pos;
            for (int i = 0; i < NUM_BRICKS_PER_COLUMN; ++i)
            {
                pos = GOMan.GetPosition( hColumnBricks[i] );
                pos.y -= COLUMN_BRICK_HEIGHT;
                GOMan.SetPosition( hColumnBricks[i], pos );
            }
            
            // TEST: move the Column GO.  TODO: make bricks CHILDREN of the Column.
            GOMan.SetPosition( hColumn, pos );
/*            
            g_pGameMap->Update();

            if ( CollisionCheck() & COLLISION_BELOW )
            {
                SoundMan.Play( hClink );
            }
*/

        #pragma mark MSG_ColumnDropToBottom
        OnMsg( MSG_ColumnDropToBottom );
            RETAILMSG(ZONE_INFO, "TutorialMode: MSG_ColumnDropToBottom");
            DropColumnToBottom();
            ChangeState( STATE_LandColumn );
            

        #pragma mark MSG_ColumnLeft
        OnMsg( MSG_ColumnLeft )
            RETAILMSG(ZONE_INFO, "TutorialMode: MSG_ColumnLeft");
            MoveColumnLeft();

        
        #pragma mark MSG_ColumnRight
        OnMsg( MSG_ColumnRight )
            RETAILMSG(ZONE_INFO, "TutorialMode: MSG_ColumnRight");
            MoveColumnRight();
            
        
        #pragma mark MSG_ColumnRotate
        OnMsg( MSG_ColumnRotate )
            RETAILMSG(ZONE_INFO, "TutorialMode: MSG_ColumnRotate");
            RotateColumn();
            

        ///////////////////////////////////////////////////////////////
		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "ColumnState: STATE_TutorialMode");
            g_numLines          = 0;
            g_numChains         = 0;
            g_totalScore        = 0;
            g_numDroppingBricks = 0;
    
    
        ///////////////////////////////////////////////////////////////
		OnExit
            RETAILMSG(ZONE_STATEMACHINE, "ColumnState: STATE_TutorialMode: Exit");



    ///////////////////////////////////////////////////////////////
    #pragma mark STATE_DropColumn
	DeclareState( STATE_DropColumn )

        #pragma mark MSG_TouchUpdate
        OnMsg( MSG_TouchUpdate )
            HandleTouch( MSG_TouchUpdate, (TouchEvent*)msg->GetPointerData() );

        #pragma mark MSG_TouchEnd
        OnMsg( MSG_TouchEnd )
            HandleTouch( MSG_TouchEnd, (TouchEvent*)msg->GetPointerData() );


        #pragma mark MSG_ColumnDropToBottom
        OnMsg( MSG_ColumnDropToBottom );
            DropColumnToBottom();
            ChangeState( STATE_LandColumn );
            

        #pragma mark MSG_ColumnLeft
        OnMsg( MSG_ColumnLeft )
            MoveColumnLeft();

        
        #pragma mark MSG_ColumnRight
        OnMsg( MSG_ColumnRight )
            MoveColumnRight();
            
        
        #pragma mark MSG_ColumnRotate
        OnMsg( MSG_ColumnRotate )
            RotateColumn();
            
        
        ///////////////////////////////////////////////////////////////
		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "ColumnState: STATE_DropColumn");

            g_pGameMap->Update();
            
            // Check for collision.
            if ( CollisionCheck() & COLLISION_BELOW )
            {
//                //
//                // Check for GAME OVER
//                //
//                if ( GOMan.GetPosition( hColumnBricks[0] ).y >= GAME_GRID_TOP )
//                {
//                    UINT64 endGameTime = GameTime.GetTime();
//                
//                    RETAILMSG(ZONE_INFO, "------------------------------------");
//                    RETAILMSG(ZONE_INFO, "---          GAME OVER           ---");
//                    RETAILMSG(ZONE_INFO, "------------------------------------");
//                    RETAILMSG(ZONE_INFO, "Score:   %d", g_totalScore);
//                    RETAILMSG(ZONE_INFO, "Level:   %d", g_pLevel->level);
//                    RETAILMSG(ZONE_INFO, "Minutes: %2.2f", (double)(endGameTime - g_startGameTime)/60000.0 );
//                    g_pGameMap->Print();
//                    
//                    GOMan.SendMessageFromSystem( MSG_GameOver );
//                    ChangeState( STATE_Idle );
//                }
//                else 
//                {
                    ChangeStateDelayed( g_pLevel->landDelaySeconds, STATE_LandColumn );  // Give player a sporting chance to rotate after the column landed.
//                }
            }
            else 
            {
/**   
                WORLD_POSITION startPos, endPos;
                for (int i = 0; i < NUM_BRICKS_PER_COLUMN; ++i)
                {
                    startPos = vec3(0, 0, 0);
                    endPos   = vec3(0, -COLUMN_BRICK_HEIGHT, 0);
                    
                    HStoryboard hDropAnimation = CreateDropAnimation( hColumnBricks[i], startPos, endPos, DEFAULT_SECONDS_PER_MOVE*1000, false );
                    
                    StoryboardMan.Start(hDropAnimation);
                }
**/

                // HACK HACK: for now move each Brick directly
                WORLD_POSITION pos;
                for (int i = 0; i < NUM_BRICKS_PER_COLUMN; ++i)
                {
                    pos = GOMan.GetPosition( hColumnBricks[i] );
                    pos.y -= COLUMN_BRICK_HEIGHT;
                    GOMan.SetPosition( hColumnBricks[i], pos );
                }
                
                // TEST: move the Column GO, even though it's an invisible container for the bricks.
                GOMan.SetPosition( hColumn, pos );

                g_pGameMap->Update();

                if ( CollisionCheck() & COLLISION_BELOW )
                {
                    SoundMan.Play( hClink );
                }

               ChangeStateDelayed( TimeUntilDrop(), STATE_DropColumn );
            }


        OnFrameUpdate
            // If ZONE_MAP is enabled, the g_gameMap contents will be rendered as an overlay.
            g_pGameMap->Render();
    


           

    ///////////////////////////////////////////////////////////////
    #pragma mark STATE_LandColumn
	DeclareState( STATE_LandColumn )

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "ColumnState: STATE_LandColumn");
  
            // Move blocks to GameGrid layer.
            // GameMap and Layers do not take references to the GameObjects.
            for (int i = 0; i < NUM_BRICKS_PER_COLUMN; ++i)
            {
                g_pGameMap->AddGameObject( hColumnBricks[i] );

                LayerMan.AddToLayer     ( hPlayScreenGrid,       hColumnBricks[i] );
                LayerMan.RemoveFromLayer( hPlayScreenColumn,     hColumnBricks[i] );
            }
            g_pGameMap->Update();
    
  
            // TODO: change appearance or particle effect?          
            ChangeStateDelayed( 0.1f, STATE_CheckForCompletedLines );



    ///////////////////////////////////////////////////////////////
    #pragma mark STATE_CheckForCompletedLines
    DeclareState( STATE_CheckForCompletedLines )
    
        OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "ColumnState: STATE_CheckForCompletedLines");

            DEBUGCHK(g_numDroppingBricks == 0);
            g_pGameMap->Print();

            if ( CheckForSmashedBricks( false ) > 0 )
            {
                ChangeState( STATE_SmashBricks );
            }
            else
            {
                //
                // Find continguous Bricks, remove from the screen/map.
                //
                pClearedBricks = FindContiguousBricks( 3, &g_longestLine, &g_numLines );
                
                if (pClearedBricks && pClearedBricks->size() > 0)
                {
                    ChangeState( STATE_ShowCompletedLines );
                }
                else
                {
                    ChangeStateDelayed( (float)BLOCK_DROP_SPEED_MS/1000.0f, STATE_DropAllBricks );
                }
            }


    ///////////////////////////////////////////////////////////////
    #pragma mark STATE_ShowCompletedLines
    DeclareState( STATE_ShowCompletedLines )
        
        OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "ColumnState: STATE_ShowCompletedLines");

            if (pClearedBricks)
            {
                // Update the score.
                g_totalScore += UpdateScore( pClearedBricks, g_longestLine, g_numLines, g_numChains );
                SendMsgToStateMachine( MSG_UpdateScore );

                HGameObjectSetIterator pHBrick;
                for (pHBrick = pClearedBricks->begin(); pHBrick != pClearedBricks->end(); ++pHBrick)
                {
                    HGameObject hBrick = *pHBrick;
                    
                    // Move bricks to foreground layer.
                    // Do it in this order to ensure refcount doesn't drop below 0.
                    LayerMan.AddToLayer     ( hPlayScreenColumn, hBrick );
                    LayerMan.RemoveFromLayer( hPlayScreenGrid,   hBrick );
                    
                    // Add overlay Sprite to the Brick.
                    HSprite hOverlay;
                    SpriteMan.GetCopy( "block_overlay", &hOverlay );
                    GOMan.AddChild( hBrick, hOverlay );
                    SpriteMan.Release( hOverlay );
                    
                    // Bind FlashBrick to contiguous Bricks.
                    // Will self-delete when finished.
                    HStoryboard hFlashStoryboard;
                    StoryboardMan.GetCopy( "FlashBrick", &hFlashStoryboard );
                    StoryboardMan.BindTo( hFlashStoryboard, hBrick );
                    StoryboardMan.Start( hFlashStoryboard );
                    
                    // Matched critters smile.
                    GOMan.SendMessageFromSystem( hBrick, MSG_HappyCritter );
                }
                
                int numBricks = pClearedBricks->size();
                switch (numBricks)
                {
                    case 3:
                        SoundMan.Play( hScore1 );
                        break;
                    case 4:
                        SoundMan.Play( hScore2 );
                        break;
                    // 5 or more
                    default:
                        SoundMan.Play( hScore3 );
                        
                        //
                        // TODO: compute bounding box for the all bricks, 
                        // start the ripple in the center.
                        //
                        
//                        HGameObject hBrick  = *pClearedBricks->begin();
//                        vec3        origin  = GOMan.GetPosition( hBrick );
//                        IProperty*  pOrigin = hRippleEffect.GetProperty( "Origin" );
//                        if (pOrigin)
//                        {
//                            pOrigin->SetVec2( vec2(origin.x, origin.y) );
//                            SAFE_DELETE(pOrigin);
//                            
//                            LayerMan.SetEffect( hPlayScreenGrid, hRippleEffect );
//                            LayerMan.SetEffect( hPlayScreenForeground, hRippleEffect );
//
//                            StoryboardMan.GetCopy( "AwesomeRipple", &hAwesomeRippleStoryboard );
//                            Callback callback( ColumnState::ScoreRippleDoneCallback );
//                            StoryboardMan.CallbackOnFinished( hAwesomeRippleStoryboard, callback );
//                            StoryboardMan.BindTo( hAwesomeRippleStoryboard, hRippleEffect );
//                            StoryboardMan.Start( hAwesomeRippleStoryboard );
//                        }

                        //
                        // Emit some sparks.
                        //
                        HGameObjectSetIterator pHBrick;
                        for (pHBrick = pClearedBricks->begin(); pHBrick != pClearedBricks->end(); ++pHBrick)
                        {
                            HGameObject hBrick = *pHBrick;
                            WORLD_POSITION position = GOMan.GetPosition( hBrick );
                            position.y += COLUMN_BRICK_WIDTH/2;
                            position.x += COLUMN_BRICK_HEIGHT/2;
                            HParticleEmitter hExplosion;
                            Particles.GetCopy( "Sparks", &hExplosion );
                            Particles.SetPosition( hExplosion, position );
                            Particles.Start( hExplosion );
                            Layers.AddToLayer( hPlayScreenForeground, hExplosion );
                        }
                        break;
                }
                
                if (g_numChains >= 1)
                {
                    // HACK: don't restart camera shake if its already running.
                    // It's relative to camera's current possition, so multiple calls to BindTo
                    // will leave the camera off-axis.
                    if (!StoryboardMan.IsStarted( hCameraShakeStoryboard ))
                    {
                        StoryboardMan.BindTo( hCameraShakeStoryboard, &GameCamera );
                        StoryboardMan.Start ( hCameraShakeStoryboard );
                    }
                }
                else if (!StoryboardMan.IsStarted( hCameraZoomStoryboard ))
                {
                    // Briefly zoom the camera in/out
                    StoryboardMan.BindTo( hCameraZoomStoryboard, &GameCamera );
                    StoryboardMan.Start ( hCameraZoomStoryboard );
                }
            }
            
            ChangeStateDelayed( 0.3f, STATE_ClearCompletedLines );



    ///////////////////////////////////////////////////////////////
    #pragma mark STATE_ClearCompletedLines
    DeclareState( STATE_ClearCompletedLines )
        
        OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "ColumnState: STATE_ClearCompletedLines");

            // Player may earn a bomb when clearing blocks.
            WORLD_POSITION minPos(FLT_MAX,FLT_MAX,FLT_MAX);
            WORLD_POSITION maxPos(0,0,0);

            if (pClearedBricks)
            {
                HGameObjectSetIterator pHBrick;
                for (pHBrick = pClearedBricks->begin(); pHBrick != pClearedBricks->end(); ++pHBrick)
                {
                    HGameObject hBrick = *pHBrick;
                    
                    // HORRIBLE HACK: Stop any looping storyboard bound to the GameObject.
                    // Otherwise the storyboard and GO will never be freed.
//                    {
//                    GameObjectToStoryboardMapIterator pMapping = g_loopingStoryboards.find( hBrick );
//                    if (pMapping != g_loopingStoryboards.end()) 
//                    {
//                        HStoryboard handle = pMapping->second;
//                        StoryboardMan.Stop( handle );
//                        
//                        g_loopingStoryboards.erase( pMapping );
//                    }
//                    }
                
                    // Remove from the game map
                    g_pGameMap->RemoveGameObject( hBrick );
                    
                    // Bind EraseBrick to contiguous Bricks.
                    // Will self-delete when finished, and take the Brick with it.
                    // LayerMan will update itself to reflect this.
                    HStoryboard hEraseStoryboard;
                    StoryboardMan.GetCopy( "EraseBrick", &hEraseStoryboard );
                    StoryboardMan.BindTo( hEraseStoryboard, hBrick );
                    StoryboardMan.Start( hEraseStoryboard );

                    WORLD_POSITION pos = GOMan.GetPosition( hBrick );
                    minPos = MIN(pos, minPos);
                    maxPos = MAX(pos, maxPos);


                    GOMan.Release( hBrick );
                }
                

                //
                // Replaced cleared blocks with a bomb, if player earned one.
                //
                if (pClearedBricks->size() >= BLOCKS_TO_EARN_VERTICAL_BOMB || g_numChains >= CHAINS_TO_EARN_RADIAL_BOMB)
                {
                    HGameObject    hBomb;
                    WORLD_POSITION bombPos = minPos;
                    
                    if (pClearedBricks->size() >= BLOCKS_TO_EARN_VERTICAL_BOMB)
                    {
                        CreateBrickOfType( &g_specialBrickTypes[ 0 ], bombPos, &hBomb); // 1 == vertical bomb.
                    }
                    else
                    {
                        CreateBrickOfType( &g_specialBrickTypes[ 1 ], bombPos, &hBomb); // 0 == radial bomb.
                    }
                    
                    g_pGameMap->AddGameObject( hBomb );
                    LayerMan.AddToLayer( hPlayScreenGrid, hBomb );
                    LayerMan.RemoveFromLayer( hPlayScreenColumn, hBomb);
                    g_pGameMap->Update();
                }

                //
                // Kick off some notifications and effects when scoring.
                //

                HGameObject    hBrick   = *(pClearedBricks->begin());
                WORLD_POSITION position = GOMan.GetPosition( hBrick );
                UINT32         numBricks = pClearedBricks->size();

                if (g_numChains >= 1 || numBricks >= 6)
                {
                    // TODO: special sound
                    // TODO: particle effects

                    IProperty* pOrigin = hRippleEffect.GetProperty( "Origin" );
                    if (pOrigin)
                    {
                        pOrigin->SetVec2( vec2(position.x, position.y) );
                        SAFE_DELETE(pOrigin);
                        
                        LayerMan.SetEffect( hPlayScreenGrid, hRippleEffect );
                        LayerMan.SetEffect( hPlayScreenForeground, hRippleEffect );
                        
                        StoryboardMan.GetCopy( "AwesomeRipple", &hAwesomeRippleStoryboard );
                        Callback callback( ColumnState::ScoreRippleDoneCallback );
                        StoryboardMan.CallbackOnFinished( hAwesomeRippleStoryboard, callback );
                        StoryboardMan.BindTo( hAwesomeRippleStoryboard, hRippleEffect );
                        StoryboardMan.Start( hAwesomeRippleStoryboard );
                    }
                }


                HGameObject hAlert;
                switch (g_numChains)
                {
                    case 1:
                    {
                        SoundMan.Play( hChain1 );
                        GOMan.Create( "chain", "Chain", hPlayScreenAlerts, position, &hAlert, 1.0f, Color::LightBlue(), true );
                    }
                    break;
                        
                    case 2:
                    {
                        SoundMan.Play( hChain2 );
                        GOMan.Create( "chain", "Chain", hPlayScreenAlerts, position, &hAlert, 1.0f, Color::Gold(), true );
                    }
                    break;
                        
                    case 3:
                    {
                        SoundMan.Play( hChain3 );
                        GOMan.Create( "chain", "Chain", hPlayScreenAlerts, position, &hAlert, 1.0f, Color::Violet(), true );
                    }
                    break;
                }
                
                if (numBricks >= 5)
                {
                    SoundMan.Play( hChain1 );
                    
                    HGameObject hAlert;
                    GOMan.Create( "nice", "Nice", hPlayScreenAlerts, position, &hAlert, 1.0f, Color::Gold(), true );

//                    // TEST:
//                    {
//                        HEffect hEffect;
//                        Effects.GetCopy( "GradientEffect", &hEffect );  // TODO: who release this and when?!
//
//                        IProperty* pStartColor = hEffect.GetProperty("StartColor");
//                        IProperty* pEndColor   = hEffect.GetProperty("EndColor");
//                        DEBUGCHK(pStartColor && pEndColor);
//                        pStartColor->SetColor( Color::Gold() );
//                        pEndColor->SetColor  ( Color::White() );
//
//                        //delete pStartColor;
//                        //delete pEndColor;
//
//                        GOMan.SetEffect(hAlert, hEffect);
//                        //hEffect.Release();
//                    }

                }

                g_numChains++;
            
                delete pClearedBricks;
                pClearedBricks = NULL;
            }
            
            // Brief delay to allow user to bask in the clearing bricks
            ChangeStateDelayed( 0.15f, STATE_DropAllBricks );




    ///////////////////////////////////////////////////////////////
    #pragma mark STATE_DropAllBricks
	DeclareState( STATE_DropAllBricks )

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "ColumnState: STATE_DropAllBricks");
            
            DropAllBricks();
            if (g_numDroppingBricks > 0)
            {
                ChangeSubstate( SUBSTATE_WaitForDroppingBricks );
            }
            else
            {
                //
                // Check for GAME OVER (any column has been filled to top of screen)
                //
                bool gameOver = false;
                for (int i = 0; i < GAME_GRID_NUM_COLUMNS; ++i)
                {
                    int top;
                    if (Platform::IsWidescreen()) {
                        top = GAME_GRID_NUM_ROWS;
                    } else {
                        top = GAME_GRID_NUM_ROWS_3_5;
                    }
                
                    if (g_pGameMap->GetValue(i, top) != 0)
                    {
                        gameOver = true;
                        break;
                    }
                }
                
                if (gameOver)
                {
                    UINT64 endGameTime = GameTime.GetTime();
                
                    RETAILMSG(ZONE_INFO, "------------------------------------");
                    RETAILMSG(ZONE_INFO, "---          GAME OVER           ---");
                    RETAILMSG(ZONE_INFO, "------------------------------------");
                    RETAILMSG(ZONE_INFO, "Score:   %d", g_totalScore);
                    RETAILMSG(ZONE_INFO, "Level:   %d", g_pLevel->level);
                    RETAILMSG(ZONE_INFO, "Minutes: %2.2f", (double)(endGameTime - g_startGameTime)/60000.0 );
                    g_pGameMap->Print();
                    
                    GOMan.SendMessageFromSystem( MSG_GameOver );
                    ChangeState( STATE_Idle );
                }

                if ( CheckForLevelUp() == true )
                {
                    ChangeStateDelayed( 0.1f, STATE_LevelUp );
                }
                else
                {
                    if (g_isTutorialMode)
                    {
                        ChangeStateDelayed( 0.1f, STATE_TutorialMode );
                    }
                    else
                    {
                        ChangeStateDelayed( 0.1f, STATE_CreateColumn );
                    }
                }
            }
            
        DeclareSubstate( SUBSTATE_WaitForDroppingBricks )
            OnEnter
                RETAILMSG(ZONE_STATEMACHINE, "ColumnState: SUBSTATE_WaitForDroppingBricks");
            
            OnFrameUpdate
                if (g_numDroppingBricks <= 0)
                {
                    g_pGameMap->Update();

                    // HACK: check again; sometimes g_numDroppingBricks goes to zero
                    // even though some bricks are still hanging in thin air. :-/
                    DropAllBricks();
                    if (g_numDroppingBricks <= 0)
                    {
                        g_pGameMap->Update();
                        ChangeState( STATE_CheckForCompletedLines );
                    }
                }

    
    ///////////////////////////////////////////////////////////////
    #pragma mark STATE_SmashBricks
	DeclareState( STATE_SmashBricks )

        OnMsg( MSG_SmashBrick )
            if (g_smashedBricks.size() <= 0)
            {
                StopTimer( MSG_SmashBrick );
                {
                    ChangeState( STATE_DropAllBricks );
                }
            }
            else
            {
                HGameObject hBrick = *g_smashedBricks.begin();
                g_smashedBricks.erase( g_smashedBricks.begin() );
                
                WORLD_POSITION pos = GOMan.GetPosition(hBrick);

                // Move bricks to foreground layer
                // Do it in this order to ensure refcount doesn't drop below 0.
                LayerMan.AddToLayer     ( hPlayScreenColumn, hBrick );
                LayerMan.RemoveFromLayer( hPlayScreenGrid,   hBrick );

                // Smash brick.
                HStoryboard hSmashStoryboard;
                StoryboardMan.GetCopy( "SmashBrick", &hSmashStoryboard );
                StoryboardMan.BindTo( hSmashStoryboard, hBrick );
                StoryboardMan.Start( hSmashStoryboard );

                // Explosion.  TODO: particle effect instead of sprite animation!
                HSprite hExplosionSprite;
                SpriteMan.GetCopy( "explosion", &hExplosionSprite );
                SpriteMan.SetPosition( hExplosionSprite, pos ); 
                LayerMan.AddToLayer( hPlayScreenColumn, hExplosionSprite );
                
                HStoryboard hExplosionStoryboard;
                StoryboardMan.GetCopy( "Explosion", &hExplosionStoryboard );
                StoryboardMan.BindTo( hExplosionStoryboard, hExplosionSprite );
                StoryboardMan.Start( hExplosionStoryboard );
                hExplosionSprite.Release();

                // Smash sound.
                SoundMan.Play( hSmashBlock );
                
                // Shake camera.
                StoryboardMan.Start( hSmashCameraShakeStoryboard );

                // Animate score.
                SendMsgToStateMachine( MSG_UpdateScore );

                // TODO: start a particle system for block debris.

                // Remove from the game map
                g_pGameMap->RemoveGameObject( hBrick );

                // If the piece smashed was itself a bomb, detonate it
//                GO_TYPE type = GOMan.GetType( hBrick );
//                if (type & GO_TYPE_VERTICAL_BOMB || type & GO_TYPE_RADIAL_BOMB)
//                {
//                    GOMan.SendMessageFromSystem(hBrick, MSG_Detonate);
//                }
//                else
                {
                    hBrick.Release();
                }
            }

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "ColumnState: STATE_SmashBricks");
            
            // NOTE: bind ONCE; this records the current camera position.
            // Multiple bindings could cause the camera to drift, since this storyboard is relative-to-starting position.
            // Can start the storyboard as often as we like, it will return camera to original position before restarting.
            StoryboardMan.BindTo ( hSmashCameraShakeStoryboard, &GameCamera );
            SetTimerState( 0.05f, MSG_SmashBrick );
            

        OnExit
            RETAILMSG(ZONE_STATEMACHINE, "ColumnState: STATE_SmashBricks: Exit");
//            if (g_smashedBricks.size() > 0)
//            {
//                DEBUGCHK(0);
//            }
            g_pGameMap->Update();


    
    ///////////////////////////////////////////////////////////////
    #pragma mark STATE_LevelUp
	DeclareState( STATE_LevelUp )
    
        // Broadcast MSG_LevelUp, then wait for a MSG_BeginPlay
        // after LevelUp has completed (see GameScreens.mm).
    
        OnMsg( MSG_BeginPlay )
            ChangeState( STATE_DropAllBricks );

    
        OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "ColumnState: STATE_LevelUp");
            SendMsgBroadcast( MSG_LevelUp );
            g_levelledUp = false;
        
        
    
	///////////////////////////////////////////////////////////////
    #pragma mark STATE_Idle
	DeclareState( STATE_Idle )

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "ColumnState: STATE_Idle");

        OnFrameUpdate


    DeclareState( STATE_GameOver )
    
        OnEnter

        OnExit
    


EndStateMachine
}



} // END namespace Z


