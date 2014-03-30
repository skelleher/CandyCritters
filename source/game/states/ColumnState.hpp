#pragma once
#include "StateMachine.hpp"
#include "GameObjectManager.hpp"
#include "TouchInput.hpp"
#include "GameState.hpp"

namespace Z
{


//
// This StateMachine controls the behavior of the Column currently in-play.
//

class ColumnState : public StateMachine
{
public:
	ColumnState( HGameObject &hGameObject );
	~ColumnState( void );

private:
	virtual bool States( State_Machine_Event event, MSG_Object * msg, int state, int substate );
    
    
    // Return values for CollisionCheck()
    enum COLLISION_RESULT
    {
        COLLISION_NONE  = 0,
        COLLISION_LEFT  = BIT0,
        COLLISION_RIGHT = BIT1,
        COLLISION_BELOW = BIT2,
    };

    // TODO: all methods can be static
    RESULT              CreateColumn        ( WORLD_POSITION position, BrickType* pBrickTypes = NULL, UINT8 numTypes = 0);
    RESULT              CreateBrickOfType   ( BrickType* pBrickType, WORLD_POSITION position, HGameObject* pHGameObject );

    COLLISION_RESULT    CollisionCheck      ( );
    HGameObjectSet*     FindContiguousBricks( UINT32 minCount, OUT UINT8* pLongestLine, OUT UINT8* pNumLines );              // Caller must free the set
    HGameObjectSet*     FindContiguousBricks( GO_TYPE brickType, MAP_POSITION position, ivec2 direction, UINT32 minCount );  // Caller must free the set
    UINT32              UpdateScore         ( IN HGameObjectSet* pBricks, UINT8 longestLine, UINT8 numLines, UINT8 numChains );
    UINT32              DropAllBricks       ( );
    HStoryboard         CreateDropAnimation ( IN HGameObject hTarget, WORLD_POSITION start, WORLD_POSITION end, UINT64 durationMS, bool deleteOnFinish );
    void                DropColumnToBottom  ( );
    bool                MoveColumnLeft      ( );
    bool                MoveColumnRight     ( );
    void                RotateColumn        ( );
    bool                CheckForLevelUp     ( );

    float               TimeUntilDrop       ( );

    bool                WasColumnTouched    ( TouchEvent* pTouchEvent );
    RESULT              HandleTouch         ( MSG_Name msgName, IN TouchEvent* pTouchEvent );
    RESULT              HandleGameObjectTapped( MSG_Object* pMsgObject );
    
    UINT8               CheckForSmashedBricks( bool wasFlung = false );
    UINT8               SmashBricks         ( const WORLD_POSITION& pos, GO_TYPE bombType, bool wasFlung = false, bool chainReaction = false );
    GO_TYPE             BombInColumn        ( HGameObject* pHBricks, UINT8 numHBricks );

    static void         DropBrickCallback   ( void* pContext );
    static void         ScoreRippleDoneCallback ( void* pContext );
};



} // END namespace Z


