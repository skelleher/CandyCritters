//
//  Tutorial.cpp
//  Critters
//
//  Created by Sean Kelleher on 5/27/13.
//
//

#include "Tutorial.hpp"


TutorialAction g_tutorialActions[] =
{
    { 1.00f, MSG_TutorialPopulateWithCritters, NULL },
    
    { 2.00f, MSG_TutorialShowText, "Let's learn how to play!" },
    
    { 3.00f, MSG_TutorialCreateColumn, NULL },
    { 3.50f, MSG_TutorialDropColumn, NULL },
    { 4.00f, MSG_TutorialDropColumn, NULL },
    { 4.50f, MSG_TutorialDropColumn, NULL },

    { 4.75f, MSG_TutorialShowText, "Swipe Critters left and right" },
    { 5.00f, MSG_TutorialShowFinger, NULL },

    { 6.0f,  MSG_TutorialShowFingerLeft, NULL },
    { 6.25f, MSG_TutorialShowFingerLeft, NULL },
    { 6.5f,  MSG_TutorialShowFingerLeft, NULL },
    { 6.75f, MSG_TutorialShowFingerRight, NULL },
    { 7.0f,  MSG_TutorialShowFingerRight, NULL },
    { 7.25f, MSG_TutorialShowFingerRight, NULL },
    { 7.50f, MSG_TutorialShowFingerRight, NULL },
    { 7.75f, MSG_TutorialShowFingerRight, NULL },
    { 8.00f, MSG_TutorialShowFingerLeft, NULL },
    { 8.25f, MSG_TutorialShowFingerLeft, NULL },

    { 9.00f, MSG_TutorialShowText, "Tap to change their order" },

    { 10.0f, MSG_TutorialShowFingerRotate, NULL },
    { 10.5f, MSG_TutorialShowFingerRotate, NULL },
    { 11.0f, MSG_TutorialShowFingerRotate, NULL },

    { 12.0f, MSG_TutorialShowText, "Swipe down to drop quickly" },
    { 13.0f, MSG_TutorialShowFingerSwipeDown, NULL },

    { 14.0f, MSG_TutorialHideFinger, NULL },
    { 16.5f, MSG_TutorialClearScreen, NULL },
    { 16.5f, MSG_TutorialShowText, "Let's play!" },
    { 16.75f, MSG_TutorialFinished, NULL },
};

uint32_t g_numTutorialActions   = ARRAY_SIZE(g_tutorialActions);
uint32_t g_tutorialActionsIndex = 0;
double   g_tutorialStartTime    = 0.0f;

