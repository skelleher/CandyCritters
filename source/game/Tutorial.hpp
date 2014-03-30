//
//  Tutorial.hpp
//  Critters
//
//  Created by Sean Kelleher on 5/27/13.
//
//

#pragma once
#include "msg.hpp"

typedef struct TutorialAction
{
    float       timestamp;
    MSG_Name    message;
    const char* dialogText;
} TutorialAction;


extern TutorialAction g_tutorialActions[];
extern uint32_t       g_numTutorialActions;
extern uint32_t       g_tutorialActionsIndex;
extern double         g_tutorialStartTime;
