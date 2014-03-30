/*
 *  ColumnBrickMap.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 1/29/11.
 *  Copyright 2011 Sean Kelleher. All rights reserved.
 *
 */

#include "ColumnBrickMap.hpp"
#include "GameState.hpp"


namespace Z
{


ColumnBrickMap::ColumnBrickMap( UINT32 width, UINT32 height, float cellWidth, float worldOriginX, float worldOriginY, float worldOriginZ, bool verticalMap ) :
    GameMap<GO_TYPE>( width, height, cellWidth, worldOriginX, worldOriginY, worldOriginZ, verticalMap )
{
}


ColumnBrickMap::~ColumnBrickMap()
{
}



void
ColumnBrickMap::Update( /* TODO: max timeslice in milliseconds before method must return */ )
{
    RESULT rval = S_OK;
    
    DEBUGMSG(ZONE_MAP, "ColumnBrickMap::Update()");

    // Update the position of every GameObject in the map.
    GameMap<GO_TYPE>::Update();
    
////    DebugRender.Reset();

    // For each GameObject, update its containing cell's GO_TYPE.
    GameObjectListIterator pGameObject;
    for (pGameObject = m_gameObjectList.begin(); pGameObject != m_gameObjectList.end(); ++pGameObject)
    {
        HGameObject     hGameObject = *pGameObject;
        GameObject*     pGO;
        GO_TYPE         type;
        MAP_POSITION    mapPos;
        
        CHR( GOMan.GetGameObjectPointer( hGameObject, &pGO ));
        CPR( pGO );
        type = pGO->GetType();
    
        WorldToMapPosition( GOMan.GetPosition( hGameObject ), &mapPos );
        UINT32 cellIndex = (mapPos.y * m_gridWidth) + mapPos.x;
        m_pValuesGrid     [ cellIndex ] = type;
    }

    
    if (Log::IsZoneEnabled( ZONE_MAP ))
    {
        Print();
    }
    
Exit:
    return;
}



void
ColumnBrickMap::Render()
{
    if (!m_debugDisplay)
    {
        return;
    }

    // Render the map as transparent quads on top of the game.
    for (int y = m_gridHeight-1; y >= 0; --y)
    {
        for (UINT32 x = 0; x < m_gridWidth; ++x)
        {
            GO_TYPE cellValue = m_pValuesGrid[ y*m_gridWidth + x ];

            WORLD_POSITION worldPos;
            MapToWorldPosition(x, y, &worldPos);
            
            switch (cellValue) 
            {
                case GO_TYPE_UNKNOWN:
                    DebugRender.Quad( worldPos, Color::Black(), 0.2f, 16.0f );
                    break;
                case GO_TYPE_ORANGE_BRICK:
                    DebugRender.Quad( worldPos, Color::Orange(), 0.2f, 32.0f );
                    break;
                case GO_TYPE_BLUE_BRICK:
                    DebugRender.Quad( worldPos, Color::Blue(), 0.2f, 32.0f );
                    break;
                case GO_TYPE_GREEN_BRICK:
                    DebugRender.Quad( worldPos, Color::Green(), 0.2f, 32.0f );
                    break;
                case GO_TYPE_RED_BRICK:
                    DebugRender.Quad( worldPos, Color::Red(), 0.2f, 32.0f );
                    break;
                case GO_TYPE_PURPLE_BRICK:
                    DebugRender.Quad( worldPos, Color::Purple(), 0.2f, 32.0f );
                    break;
                case GO_TYPE_PINK_BRICK:
                    DebugRender.Quad( worldPos, Color::Pink(), 0.2f, 32.0f );
                    break;
                case GO_TYPE_BROWN_BRICK:
                    DebugRender.Quad( worldPos, Color::White(), 0.2f, 32.0f );
                    break;
                case GO_TYPE_RADIAL_BOMB:
                    DebugRender.Quad( worldPos, Color::Gray(), 0.2f, 32.0f );
                    break;
                case GO_TYPE_VERTICAL_BOMB:
                    DebugRender.Quad( worldPos, Color::Gray(), 0.2f, 32.0f );
                    break;
                default:
                    break;
            }
        }
    }
}



void
ColumnBrickMap::Print()
{
    if (!Log::IsZoneEnabled(ZONE_MAP)) {
        return;
    }

    RETAILMSG(ZONE_MAP, "---------- ColumnBrickMap ----------");
    RETAILMSG(ZONE_MAP, "%d GameObjects", m_gameObjectList.size());

    for (int y = m_gridHeight-1; y >= 0; --y)
    {
        char buf[MAX_PATH];
        char* p = &buf[0];
        for (UINT32 x = 0; x < m_gridWidth; ++x)
        {
            GO_TYPE cellValue = m_pValuesGrid[ y*m_gridWidth + x ];

            WORLD_POSITION worldPos;
            MapToWorldPosition(x, y, &worldPos);
            
            switch (cellValue) 
            {
                case 0:
                case GO_TYPE_UNKNOWN:
                    *p++ = '.';
                    break;
                case GO_TYPE_ORANGE_BRICK:
                    *p++ = 'O';
                    break;
                case GO_TYPE_BLUE_BRICK:
                    *p++ = 'B';
                    break;
                case GO_TYPE_GREEN_BRICK:
                    *p++ = 'G';
                    break;
                case GO_TYPE_RED_BRICK:
                    *p++ = 'R';
                    break;
                case GO_TYPE_PURPLE_BRICK:
                    *p++ = 'P';
                    break;
                case GO_TYPE_PINK_BRICK:
                    *p++ = 'p';
                    break;
                case GO_TYPE_BROWN_BRICK:
                    *p++ = 'b';
                    break;
                case GO_TYPE_RADIAL_BOMB:
                    *p++ = '@';
                    break;
                case GO_TYPE_VERTICAL_BOMB:
                    *p++ = 'v';
                    break;
                default:
                    *p++ = '?';
                    break;
            }
        }
        *p = '\0';
        RETAILMSG(ZONE_MAP, "%2d: %s", y, buf);
    }
    RETAILMSG(ZONE_MAP, "-------------------------------------");
}



} // END namespace Z

