//
//  Level.hpp
//  Critters
//
//  Created by Sean Kelleher on 4/17/11.
//  Copyright 2011 Sean Kelleher. All rights reserved.
//

#pragma once
#include "Types.hpp"

namespace Z
{
    

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



} // END namespace Z
