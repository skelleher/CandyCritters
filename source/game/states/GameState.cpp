#include "GameState.hpp"
#include "Log.hpp"
#include "Color.hpp"
#include "BoxedVariable.hpp"

// TEST:
#include "test.hpp"



namespace Z
{

UINT32  g_totalScore            = 0;
UINT32  g_highScore             = 0;
UINT32  g_scoreToLevelUp        = 0;
UINT32  g_difficulty            = 2;
float   g_difficultyMultiplier  = 1.0f;
bool    g_newHighScore          = false;
bool    g_showTutorial          = true;
UINT32  g_gameNumChains         = 0;
UINT32  g_gameNumLinesCleared   = 0;
UINT32  g_gameNumBlocksCleared  = 0;
UINT32  g_highestLevel          = 1;
UINT32  g_highestLevelEver      = 1;
UINT64  g_startGameTime         = 0;

// HACK HACK until we have UIControls working.
bool    g_showScore = false;

// HACK:
bool    g_isTutorialMode = false;


/*
struct Level
{
    UINT8   level;
    UINT8   numBrickTypes;
    float   secondsPerMove;
    float   landDelaySeconds;
    float   scoreMultiplier;
    Color   backgroundColor;
    UINT32  scoreToLevelUp;
    const char* backgroundFilename;

    GO_TYPE specialBrickTypes;
};
*/
Level g_levels[] = 
{
    { 1,  3, 0.5f,   0.25f, 1.0f, Color::White(), 1700,     "/app/textures/background3.png",   (GO_TYPE) 0 },
    { 2,  3, 0.45f,  0.25f, 1.1f, Color::White(), 4000,     "/app/textures/background1.png",   (GO_TYPE) 0 },
    { 3,  4, 0.4f,   0.25f, 1.2f, Color::White(), 8000,     "/app/textures/background4.png",   (GO_TYPE) 0 },
    { 4,  4, 0.4f,   0.25f, 1.3f, Color::White(), 14000,    "/app/textures/background2.png",   (GO_TYPE) 0 },
    { 5,  5, 0.35f,  0.25f, 1.4f, Color::White(), 20000,    "/app/textures/background5.png",   (GO_TYPE) (GO_TYPE_VERTICAL_BOMB)     },
    { 6,  5, 0.35f,  0.25f, 1.5f, Color::White(), 28000,    "/app/textures/background6.png",   (GO_TYPE) 0 },
    { 7,  6, 0.35f,  0.25f, 1.6f, Color::White(), 38000,    "/app/textures/background7.png",   (GO_TYPE) (GO_TYPE_VERTICAL_BOMB)     },
    { 8,  6, 0.35f,  0.25f, 1.7f, Color::White(), 50000,    "/app/textures/background8.png",   (GO_TYPE) (GO_TYPE_RADIAL_BOMB)   },
    { 9,  6, 0.30f,  0.25f, 1.8f, Color::White(), 65000,    "/app/textures/background9.png",   (GO_TYPE) (GO_TYPE_RADIAL_BOMB)   },
    { 10, 6, 0.30f,  0.30f, 1.9f, Color::White(), 80000,    "/app/textures/background10.png",  (GO_TYPE) (GO_TYPE_VERTICAL_BOMB | GO_TYPE_RADIAL_BOMB) },
    { 11, 6, 0.25f,  0.30f, 2.0f, Color::White(), 95000,    "/app/textures/background11.png",  (GO_TYPE) (GO_TYPE_RADIAL_BOMB)   },

    { 12, 6, 0.20f,  0.30f, 2.0f, Color::White(), 105000,   "/app/textures/background12.png",  (GO_TYPE) (GO_TYPE_VERTICAL_BOMB)     },
    { 13, 6, 0.20f,  0.30f, 2.0f, Color::White(), 125000,   "/app/textures/background13.png",  (GO_TYPE) (GO_TYPE_RADIAL_BOMB)   },
    { 14, 6, 0.15f,  0.30f, 2.0f, Color::White(), 145000,   "/app/textures/background14.png",  (GO_TYPE) (GO_TYPE_VERTICAL_BOMB)     },
    { 15, 6, 0.15f,  0.30f, 2.0f, Color::White(), 170000,   "/app/textures/background15.png",  (GO_TYPE) (GO_TYPE_RADIAL_BOMB)   },

    { 0 }
};

UINT32 g_level = 0;
Level* g_pLevel = &g_levels[g_level];
UINT32 g_numLevels = ARRAY_SIZE(g_levels);


BrickType g_brickTypes[] = 
{
    { "red",        "CharacterState",   GO_TYPE_RED_BRICK,              1.0 },
    { "purple",     "CharacterState",   GO_TYPE_PURPLE_BRICK,           1.0 },
    { "orange",     "CharacterState",   GO_TYPE_ORANGE_BRICK,           1.0 },
    { "brown",      "CharacterState",   GO_TYPE_BROWN_BRICK,            1.0 },
    { "green",      "CharacterState",   GO_TYPE_GREEN_BRICK,            1.0 },
    { "pink",       "CharacterState",   GO_TYPE_PINK_BRICK,             1.0 },
    { "blue",       "CharacterState",   GO_TYPE_BLUE_BRICK,             1.0 },
    { 0 }
};
UINT32 g_numBrickTypes = ARRAY_SIZE(g_brickTypes)-1;


BrickType g_specialBrickTypes[] = 
{
    { "lightning",  "BombState", GO_TYPE_VERTICAL_BOMB,   0.15  },
    { "bomb",       "BombState", GO_TYPE_RADIAL_BOMB,     0.10  },
    { 0 }
};
UINT32 g_numSpecialBrickTypes = ARRAY_SIZE(g_specialBrickTypes)-1;


//
// This StateMachine controls a game of Columns
//
// TODO: move EVERYTHING out of GameScreens/ColumnState and into this class.
//


//Add new states here
enum StateName {
    STATE_Initialize,
    STATE_NewGame,
    STATE_PlayGame,
    STATE_PauseGame,
    STATE_Test,
    STATE_Idle
};

//Add new substates here
enum SubstateName {
	//empty
};



GameState::GameState( HGameObject &hGameObject ) :
    StateMachine( hGameObject )
{
}

GameState::~GameState( void )
{
}


bool GameState::States( State_Machine_Event event, MSG_Object * msg, int state, int substate )
{
BeginStateMachine

	//Global message responses
	OnMsg( MSG_Reset )
		ResetStateMachine();
        RETAILMSG(ZONE_STATEMACHINE, "GameState: MSG_Reset");

    OnMsg( MSG_NewGame )
        ChangeState( STATE_NewGame );


    ///////////////////////////////////////////////////////////////
    #pragma mark STATE_Initialize
	DeclareState( STATE_Initialize )

		OnEnter
            ChangeState( STATE_Test );
	


    ///////////////////////////////////////////////////////////////
    #pragma mark STATE_Test
	DeclareState( STATE_Test )

		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "GameState: STATE_Test");
        
			ChangeState( STATE_Idle );

	
    ///////////////////////////////////////////////////////////////
    #pragma mark STATE_NewGame
	DeclareState( STATE_NewGame )
        OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "GameState: STATE_NewGame");
            
            // TODO: flourish sound, particles, "Go!" billboard, etc.
            g_totalScore    = 0;
            g_newHighScore  = false;
            
            ChangeState( STATE_PlayGame );
    
        OnExit
            RETAILMSG(ZONE_STATEMACHINE, "GameState: STATE_NewGame: Exit");


    
	///////////////////////////////////////////////////////////////
	DeclareState( STATE_PlayGame )
        OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "GameState: STATE_PlayGame");
    
        OnExit
            RETAILMSG(ZONE_STATEMACHINE, "GameState: STATE_PlayGame: Exit");
    
    

	///////////////////////////////////////////////////////////////
	DeclareState( STATE_Idle )
		OnEnter
            RETAILMSG(ZONE_STATEMACHINE, "GameState: STATE_Idle");

        OnFrameUpdate

EndStateMachine
}



} // END namespace Z



