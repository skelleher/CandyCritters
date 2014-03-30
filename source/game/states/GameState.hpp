#pragma once
#include "StateMachine.hpp"
#include "Level.hpp"


namespace Z
{

// TEST TEST
#define FAT_MODE


//
// This StateMachine controls a game of Columns
//

class GameState : public StateMachine
{
public:
	GameState( HGameObject &hGameObject );
	~GameState( void );

private:
	virtual bool States( State_Machine_Event event, MSG_Object * msg, int state, int substate );
    
    
};


extern UINT32 g_totalScore;
extern UINT32 g_highScore;
extern bool   g_newHighScore;
extern UINT32 g_difficulty;
extern float  g_difficultyMultiplier;

extern UINT32 g_level;
extern Level  g_levels[];
extern Level* g_pLevel;
extern UINT32 g_highestLevel;
extern UINT32 g_highestLevelEver;
extern UINT32 g_numLevels;

// HACK HACK until we have UIControls working.
extern bool   g_showScore;

// HACK
extern bool   g_isTutorialMode;

    
// Types of brick: red, yellow, blue, stone, bomb, etc.    
struct BrickType
{
    const char*     spriteName;
    const char*     behaviorName;
    GO_TYPE         goType;
    float           probability;
};
extern BrickType g_brickTypes[];
extern BrickType g_specialBrickTypes[];
extern UINT32    g_numBrickTypes;
extern UINT32    g_numSpecialBrickTypes;


  
// Custom GameObject types
const GO_TYPE GO_TYPE_BLUE_BRICK            = (GO_TYPE_USER);
const GO_TYPE GO_TYPE_RED_BRICK             = ((GO_TYPE)(GO_TYPE_USER << 1));
const GO_TYPE GO_TYPE_GREEN_BRICK           = ((GO_TYPE)(GO_TYPE_USER << 2));
const GO_TYPE GO_TYPE_ORANGE_BRICK          = ((GO_TYPE)(GO_TYPE_USER << 3));
const GO_TYPE GO_TYPE_PURPLE_BRICK          = ((GO_TYPE)(GO_TYPE_USER << 4));
const GO_TYPE GO_TYPE_PINK_BRICK            = ((GO_TYPE)(GO_TYPE_USER << 5));
const GO_TYPE GO_TYPE_BROWN_BRICK           = ((GO_TYPE)(GO_TYPE_USER << 6));
const GO_TYPE GO_TYPE_VERTICAL_BOMB         = ((GO_TYPE)(GO_TYPE_USER << 7));
const GO_TYPE GO_TYPE_RADIAL_BOMB           = ((GO_TYPE)(GO_TYPE_USER << 8));



// How long it takes for a block to drop one row, when the block(s) beneath it have been erased (i.e. gravity).
const UINT32 BLOCK_DROP_SPEED_MS        = 75;

const UINT32 NUM_BRICKS_PER_COLUMN      = 3;

const float  COLUMN_BRICK_HEIGHT        = 80.0f;
const float  COLUMN_BRICK_WIDTH         = 80.0f;
const float  SCREEN_HEIGHT              = 1136.0f;
const float  SCREEN_HEIGHT_3_5          = 960.0f;
const float  SCREEN_WIDTH               = 640.0f;
const int    GAME_GRID_NUM_COLUMNS      = 7;
const int    GAME_GRID_NUM_ROWS         = 13;
const int    GAME_GRID_NUM_ROWS_3_5     = 11;
const float  GAME_GRID_BOTTOM           = 80.0f;
const float  GAME_GRID_BOTTOM_3_5       = 64.0f;
const float  GAME_GRID_TOP              = GAME_GRID_BOTTOM + (COLUMN_BRICK_HEIGHT * GAME_GRID_NUM_ROWS+1);
const float  GAME_GRID_TOP_3_5          = GAME_GRID_BOTTOM_3_5 + (COLUMN_BRICK_HEIGHT * GAME_GRID_NUM_ROWS_3_5+1);
const float  GAME_GRID_LEFT             = (SCREEN_WIDTH - (COLUMN_BRICK_WIDTH*GAME_GRID_NUM_COLUMNS))/2.0f;
const float  GAME_GRID_RIGHT            = GAME_GRID_LEFT + (COLUMN_BRICK_WIDTH * GAME_GRID_NUM_COLUMNS);

class  ColumnBrickMap;
extern ColumnBrickMap* g_pGameMap;


// TODO: make all this configurable.
const float  DROP_VELOCITY                  = 80.0f;    // Swipe down at this speed drops the block
const float  DROP_VELOCITY_HINT             = 20.0f;    // Swipe down at this speed means user might be starting a drop; ignore sideways movement!
const float  DROP_LENGTH                    = COLUMN_BRICK_HEIGHT;
const float  SWIPE_AXIS_THRESHOLD_DEGREES   = 30.0f;    // Gestures must be within this many degrees of a cardinal axis to count as a swipe in that direction. Blackberry uses 45 degrees.
const float  TAP_MOVEMENT_CUTOFF            = 50.0f;    // Finger must move less than N pixels to count as a tap.  Prevents slow/incomplete swipe from triggering a tap.

} // END namespace Z


