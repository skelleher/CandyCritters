#pragma once

/*
 *  ColumnBrickMap.hpp
 *  Critters
 *
 *  Created by Sean Kelleher on 1/29/11.
 *  Copyright 2011 Sean Kelleher. All rights reserved.
 *
 */

#include "Game.hpp"
#include "GameMap.hpp"
#include "Log.hpp"
#include "DebugRenderer.hpp"


namespace Z
{


class ColumnBrickMap : public GameMap<GO_TYPE>
{
public:
    ColumnBrickMap          ( UINT32 width, UINT32 height, float cellWidth = 1.0f, float worldOriginX = 0.0f, float worldOriginY = 0.0f, float worldOriginZ = 0.0f, bool verticalMap = false );
    virtual ~ColumnBrickMap ( );

    //------------------------------------------------------------------------
    // Update the value of every cell in the GameMap, 
    // based on contained GameObjects.
    // Must be called at least once after populating the map.
    //------------------------------------------------------------------------
    virtual void        Update  ( /* TODO: max timeslice in milliseconds before method must return */ );
    virtual void        Render  ( );
    virtual void        Print   ( );
};



} // END namespace Z

