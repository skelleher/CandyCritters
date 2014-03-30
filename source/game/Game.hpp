#pragma once

#include "Types.hpp"
#include "GameObject.hpp"
#include "RenderContext.hpp"


namespace Z
{



//
// This class wraps a game of Columns.
// These are the entry points to be called from Objective-C on startup/shutdown.
//
class Game
{
public:
    static  RESULT  Start   ( );
    static  RESULT  Stop    ( );
    static  RESULT  Pause   ( );
    static  RESULT  Resume  ( );
    
    static  RESULT  LoadUserPreferences ( );
    static  RESULT  SaveUserPreferences ( );
};


} // END namespace Z


