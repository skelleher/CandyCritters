/* Copyright Steve Rabin, 2007. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright Steve Rabin, 2007"
 */


//namespace Z
//{



//These message names are processed inside msg.h

REGISTER_MESSAGE_NAME(MSG_NULL)							//Reserved message name
REGISTER_MESSAGE_NAME(MSG_CHANGE_STATE_DELAYED)			//Reserved message name
REGISTER_MESSAGE_NAME(MSG_CHANGE_SUBSTATE_DELAYED)		//Reserved message name

//Add new messages here
REGISTER_MESSAGE_NAME(MSG_Reset)
REGISTER_MESSAGE_NAME(MSG_EngineReady)
REGISTER_MESSAGE_NAME(MSG_NewGame)
REGISTER_MESSAGE_NAME(MSG_PauseGame)
REGISTER_MESSAGE_NAME(MSG_UnpauseGame)
REGISTER_MESSAGE_NAME(MSG_LevelUp)
REGISTER_MESSAGE_NAME(MSG_BeginPlay)
REGISTER_MESSAGE_NAME(MSG_GameOver)
REGISTER_MESSAGE_NAME(MSG_Shutdown)
REGISTER_MESSAGE_NAME(MSG_UpdateScore)
REGISTER_MESSAGE_NAME(MSG_DialogConfirmed)
REGISTER_MESSAGE_NAME(MSG_DialogCancelled)


// Touch
REGISTER_MESSAGE_NAME(MSG_TouchBegin)
REGISTER_MESSAGE_NAME(MSG_TouchUpdate)
REGISTER_MESSAGE_NAME(MSG_TouchEnd)
REGISTER_MESSAGE_NAME(MSG_GameObjectWasTapped)

// Accelerometer
REGISTER_MESSAGE_NAME(MSG_AccelerometerEvent)

// Game Screens
REGISTER_MESSAGE_NAME(MSG_GameScreenTest)
REGISTER_MESSAGE_NAME(MSG_GameScreenHome)
REGISTER_MESSAGE_NAME(MSG_GameScreenMenu)
REGISTER_MESSAGE_NAME(MSG_GameScreenTutorial)
REGISTER_MESSAGE_NAME(MSG_GameScreenLevel)
REGISTER_MESSAGE_NAME(MSG_GameScreenPlay)
REGISTER_MESSAGE_NAME(MSG_GameScreenCredits)
REGISTER_MESSAGE_NAME(MSG_GameScreenConfirmQuit)
REGISTER_MESSAGE_NAME(MSG_GameScreenPrevious)

// Tutorial
REGISTER_MESSAGE_NAME(MSG_TutorialPopulateWithCritters)
REGISTER_MESSAGE_NAME(MSG_TutorialShowTitle)
REGISTER_MESSAGE_NAME(MSG_TutorialShowText)
REGISTER_MESSAGE_NAME(MSG_TutorialCreateColumn)
REGISTER_MESSAGE_NAME(MSG_TutorialDropColumn)
REGISTER_MESSAGE_NAME(MSG_TutorialShowFinger)
REGISTER_MESSAGE_NAME(MSG_TutorialHideFinger)
REGISTER_MESSAGE_NAME(MSG_TutorialShowFingerLeft)
REGISTER_MESSAGE_NAME(MSG_TutorialShowFingerRight)
REGISTER_MESSAGE_NAME(MSG_TutorialShowFingerRotate)
REGISTER_MESSAGE_NAME(MSG_TutorialShowFingerSwipeDown)
REGISTER_MESSAGE_NAME(MSG_TutorialClearScreen)
REGISTER_MESSAGE_NAME(MSG_TutorialFinished)


REGISTER_MESSAGE_NAME(MSG_RemoveEffect)
REGISTER_MESSAGE_NAME(MSG_GameOverBubbleUp)
REGISTER_MESSAGE_NAME(MSG_HidePlayScreen)
REGISTER_MESSAGE_NAME(MSG_ShowGameOverScene)
REGISTER_MESSAGE_NAME(MSG_SpawnCritter)
REGISTER_MESSAGE_NAME(MSG_KillCritter)
REGISTER_MESSAGE_NAME(MSG_MoveCritterLeft)
REGISTER_MESSAGE_NAME(MSG_MoveCritterRight)
REGISTER_MESSAGE_NAME(MSG_MoveCritterWithinColumn)
REGISTER_MESSAGE_NAME(MSG_ScareCritter)
REGISTER_MESSAGE_NAME(MSG_ExciteCritter)
REGISTER_MESSAGE_NAME(MSG_SadCritter)
REGISTER_MESSAGE_NAME(MSG_HappyCritter)
REGISTER_MESSAGE_NAME(MSG_DropCritterToBottom)
REGISTER_MESSAGE_NAME(MSG_Fireworks)
REGISTER_MESSAGE_NAME(MSG_Firework)
REGISTER_MESSAGE_NAME(MSG_BeginStoryboard)
REGISTER_MESSAGE_NAME(MSG_SmashBrick)
REGISTER_MESSAGE_NAME(MSG_Detonate)


// Game controls
REGISTER_MESSAGE_NAME(MSG_ColumnLeft)
REGISTER_MESSAGE_NAME(MSG_ColumnRight)
REGISTER_MESSAGE_NAME(MSG_ColumnRotate)
REGISTER_MESSAGE_NAME(MSG_ColumnDropOneUnit)
REGISTER_MESSAGE_NAME(MSG_ColumnDropToBottom)



//REGISTER_MESSAGE_NAME(MSG_Fire_Weapon)
//REGISTER_MESSAGE_NAME(MSG_Stop_Fire_Weapon)
//REGISTER_MESSAGE_NAME(MSG_Take_Damage)
//REGISTER_MESSAGE_NAME(MSG_HUD_Stop_Showing_Damage)

//REGISTER_MESSAGE_NAME(MSG_Scan_For_Player)
//REGISTER_MESSAGE_NAME(MSG_Saw_Player)

//REGISTER_MESSAGE_NAME(MSG_Collision)

//REGISTER_MESSAGE_NAME(MSG_Set_Goal)
//REGISTER_MESSAGE_NAME(MSG_Arrived_At_Waypoint)
//REGISTER_MESSAGE_NAME(MSG_Arrived_At_Destination)


//Unit test messages
REGISTER_MESSAGE_NAME(MSG_UnitTestMessage)
REGISTER_MESSAGE_NAME(MSG_UnitTestMessage2)
REGISTER_MESSAGE_NAME(MSG_UnitTestMessage3)
REGISTER_MESSAGE_NAME(MSG_UnitTestMessage4)
REGISTER_MESSAGE_NAME(MSG_UnitTestMessage5)
REGISTER_MESSAGE_NAME(MSG_UnitTestMessage6)
REGISTER_MESSAGE_NAME(MSG_UnitTestMessage7)
REGISTER_MESSAGE_NAME(MSG_UnitTestMessage8)
REGISTER_MESSAGE_NAME(MSG_UnitTestMessage9)
REGISTER_MESSAGE_NAME(MSG_UnitTestBroken)
REGISTER_MESSAGE_NAME(MSG_UnitTestPing)
REGISTER_MESSAGE_NAME(MSG_UnitTestAck)
REGISTER_MESSAGE_NAME(MSG_UnitTestDone)
REGISTER_MESSAGE_NAME(MSG_UnitTestTimer)



//} // END namespace Z
