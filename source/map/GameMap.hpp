
#pragma once
#include "GameObjectManager.hpp"
#include "Types.hpp"
#include "Log.hpp"

// TEST:
#include "DebugRenderer.hpp"


namespace Z
{


template<typename TYPE>
class GameMap
{
public:
    GameMap          ( UINT32 width, UINT32 height, float cellWidth = 1.0f, float worldOriginX = 0.0f, float worldOriginY = 0.0f, float worldOriginZ = 0.0f, bool verticalMap = false );
    virtual ~GameMap ( );

    //------------------------------------------------------------------------
    // Update the value of every cell in the GameMap, 
    // based on contained GameObjects.
    // Must be called at least once after populating the map.
    //------------------------------------------------------------------------
    virtual void        Update              ( /* TODO: max timeslice in milliseconds before method must return */ );
    virtual void        Render              ( )  = 0;
    virtual void        Print               ( )  = 0;

    //------------------------------------------------------------------------
    // Link to another map
    // Lets us add its values to our own every time ::Update() is called.
    //
    // Use this to combine two or more maps into a super-map
    // e.g. Openness + Influence == places to attack from (or avoid)
    //------------------------------------------------------------------------
    void                Link                ( GameMap* pMap );
    void                Unlink              ( GameMap* pMap );

    void                EnableDebugDisplay  ( bool enabled );

    void                WorldToMapPosition  ( IN const WORLD_POSITION& worldPos,     INOUT MAP_POSITION*   pMapPos   );
    void                WorldToMapPosition  ( IN float x, IN float y, IN float z,    INOUT MAP_POSITION*   pMapPos   );
    void                MapToWorldPosition  ( IN const MAP_POSITION& mapPos,         INOUT WORLD_POSITION* pWorldPos, IN bool centerInCell = true );
    void                MapToWorldPosition  ( IN UINT32 x, IN UINT32 y,              INOUT WORLD_POSITION* pWorldPos, IN bool centerInCell = true );

    //------------------------------------------------------------------------
    // Populate the GameMap
    //------------------------------------------------------------------------
    void                AddGameObject       ( IN HGameObject hGameObject, IN UINT32 x, IN UINT32 y   );
    void                AddGameObject       ( IN HGameObject hGameObject, IN const MAP_POSITION& pos );
    void                AddGameObject       ( IN HGameObject hGameObject );

    void                RemoveGameObject    ( IN HGameObject hGameObject, IN UINT32 x, IN UINT32 y   );
    void                RemoveGameObject    ( IN HGameObject hGameObject, IN const MAP_POSITION& pos );
    void                RemoveGameObject    ( IN HGameObject hGameObject );

	void                Clear               ( );


    //------------------------------------------------------------------------
    // Query the GameMap
    //------------------------------------------------------------------------
    HGameObject         FindClosest         ( IN const MAP_POSITION& pos, IN UINT32 radius, IN TYPE type );
    HGameObject         FindClosest         ( IN const MAP_POSITION& pos, IN const ivec2& direction, IN UINT32 distance, IN TYPE type );

    RESULT              FindAll             ( IN const MAP_POSITION& pos, IN UINT32 radius, IN TYPE type, INOUT HGameObjectSet* pList );
    RESULT              FindAll             ( IN const MAP_POSITION& pos, IN const ivec2& direction, IN UINT32 distance, IN TYPE type, INOUT HGameObjectSet* pList );

    inline UINT32       GetNumberOfObjects  ( IN const MAP_POSITION& pos )   { return GetNumberOfObjects( pos.x, pos.y ); }
    inline UINT32       GetNumberOfObjects  ( UINT32 x, UINT32 y )           { return m_pGameObjectsGrid[ y*m_gridWidth + x ].size(); }
 

    //------------------------------------------------------------------------
    // Direct access
    //------------------------------------------------------------------------
    inline UINT32       GetWidth            ( )  { return m_gridWidth;   }
    inline UINT32       GetHeight           ( )  { return m_gridHeight;  }
    
    inline TYPE*            GetArrayOfValues      ( )  { return m_pValuesGrid; }
//    inline HGameObjectList* GetArrayOfGameObjects ( )  { return m_pGameObjectsGrid; }
    inline HGameObjectList  GetListOfAllGameObjects  ( )  { return m_gameObjectList; }


    inline HGameObjectList* GetGameObjects( UINT32 x, IN UINT32 y )
                        {
                            if (x >= m_gridWidth || y >= m_gridHeight)
                            {
                                //DEBUGMSG(ZONE_WARN, "ERROR: GameMap::GetValue(%d,%d) out of range", x, y);
                                return NULL;
                            }
                        
                            UINT32 cellIndex = x + (y * m_gridWidth);
                            return &m_pGameObjectsGrid[ cellIndex ];
                        }

    inline HGameObjectList* GetGameObjects( IN const MAP_POSITION& mapPos )
                        {
                            return GetGameObjects( mapPos.x, mapPos.y );
                        }


    inline void         SetValue ( UINT32 x, UINT32 y, TYPE value )
                        {
                            if (x >= m_gridWidth || y >= m_gridHeight) 
                            {
                                RETAILMSG(ZONE_ERROR, "ERROR: GameMap::SetValue(%d,%d) out of range", x, y);
                                return;
                            }

                            UINT32 cellIndex = x + (y * m_gridWidth);
                            m_pValuesGrid[cellIndex] = value;
                        }

    inline void         SetValue ( MAP_POSITION mapPos, TYPE value )
                        {
                            SetValue( mapPos.x, mapPos.y, value );
                        }

    inline void         SetValue ( WORLD_POSITION worldPos, TYPE value )
                        {
                            MAP_POSITION mapPos;
                            WorldToMapPosition( worldPos, &mapPos );

                            SetValue( mapPos.x, mapPos.y, value );
                        }




    inline TYPE         GetValue ( UINT32 x, UINT32 y )
                        {
                            if (x >= m_gridWidth || y >= m_gridHeight) 
                            {
                                //DEBUGMSG(ZONE_WARN, "WARNING: GameMap::GetValue( %d, %d ): out of bounds", x, y);
                                return (TYPE)(0);
                            }

                            UINT32 cellIndex = x + (y * m_gridWidth);
                            return m_pValuesGrid[ cellIndex ];
                        }

    inline TYPE         GetValue ( MAP_POSITION  mapPos )
                        {
                            return GetValue( mapPos.x, mapPos.y );
                        }

    inline float        GetValue ( WORLD_POSITION worldPos )
                        {
                            MAP_POSITION mapPos;
                            WorldToMapPosition( worldPos, &mapPos );

                            return GetValue( mapPos );
                        }

public:
    static const ivec2 directions[];
    static const int   numDirections;


protected:
    typedef std::list<GameMap*>         GameMapList;
    typedef std::list<HGameObject>      GameObjectList;

    // The "typename" keyword is required when declaring an iterator on a nested template,
    // such as std::map<const char*, Handle<TYPE> >
    // See Question #1 in the C++ Templates FAQ
    typedef typename GameObjectList::iterator    GameObjectListIterator;
    typedef typename GameMapList::iterator       GameMapListIterator;

protected:
    UINT32                  m_gridWidth;
    UINT32                  m_gridHeight;
    float                   m_fCellWidth;
    WORLD_POSITION          m_worldOrigin;
    bool                    m_isVertical;

    TYPE*                   m_pValuesGrid;                  // Grid of computed values for each cell in the map (e.g. influence, visibility, visited)
    HGameObjectList*        m_pGameObjectsGrid;             // Grid of HGameObjects for each cell in the map.
    GameObjectList          m_gameObjectList;           
    GameMapList             m_linkedMapList;

    bool                    m_debugDisplay;
};





template<typename TYPE>
const ivec2 GameMap<TYPE>::directions[] =
{
    ivec2( -1,  1 ),   // NorthWest
    ivec2(  0,  1 ),   // North
    ivec2(  1,  1 ),   // NorthEast
    ivec2( -1,  0 ),   // West
    ivec2(  1,  0 ),   // East
    ivec2( -1, -1 ),   // SouthWest
    ivec2(  0, -1 ),   // South
    ivec2(  1, -1 ),   // SouthEast
};

template<typename TYPE>
const int GameMap<TYPE>::numDirections = ARRAY_SIZE(directions);





#pragma mark class MapPositionSorter

// Sort functor used when inserting MAP_POSITIONs into a std::set.
class MapPositionSorter
{
public:
    // Sort MAP_POSITIONs by X and Y coordinates.
    // We assume all MAP_POSITIONs are positive (in NE quadrant of Cartesian grid).
    // This satisfies the requirements of std::set / std::multiset.
    // See The C++ Standard Template Library by Nicolai Josuttis p176. 
    bool operator() (const MAP_POSITION& lhs, const MAP_POSITION& rhs) const
    {
        UINT64 lhsKey = (UINT64)(lhs.y) << 32 | lhs.x;
        UINT64 rhsKey = (UINT64)(rhs.y) << 32 | rhs.x;

        if (lhsKey < rhsKey)
        {
            return true;
        }
        
        return false;
    }

};





#pragma mark GameMap Template Implementation

template<typename TYPE>
GameMap<TYPE>::GameMap( UINT32 width, UINT32 height, float cellWidth, float worldOriginX, float worldOriginY, float worldOriginZ, bool isVertical ) :
    m_gridWidth       (width),
    m_gridHeight      (height),
    m_fCellWidth      (cellWidth),
    m_worldOrigin     (worldOriginX, worldOriginY, worldOriginZ),
    m_isVertical      (isVertical),
    m_debugDisplay    (false)
{
    // We only support integral values in each cell of the GameMap.
    DEBUGCHK( sizeof(TYPE) <= 4 );

    m_pValuesGrid       = new TYPE[ width * height ];
    m_pGameObjectsGrid  = new HGameObjectList[ width * height ];
    
    memset( m_pValuesGrid, 0, width * height * sizeof(TYPE) );    // TODO: this is bad if TYPE is non-integral.
}



template<typename TYPE>
GameMap<TYPE>::~GameMap()
{
    SAFE_ARRAY_DELETE ( m_pValuesGrid       );
    SAFE_ARRAY_DELETE ( m_pGameObjectsGrid  );
}



template<typename TYPE>
void
GameMap<TYPE>::Update( /* TODO: max timeslice in milliseconds before method must return */ )
{
    RESULT rval = S_OK;
    
    DEBUGMSG(ZONE_MAP, "GameMap::Update()");
    memset( m_pValuesGrid,      0, m_gridWidth*m_gridHeight*sizeof(TYPE)     );

    // TODO: UGH, this won't scale well for large maps.
    for (UINT32 i = 0; i < m_gridWidth*m_gridHeight; ++i)
    {
        m_pGameObjectsGrid[ i ].clear();
    }

    //
    // DEFAULT IMPLEMENTATION:
    //
    // Update each cell's list of GameObjects, based on their current position 
    // (they may have moved since being added to the map).
    //
    // Do not update the "value" of each cell, since only the subclass knows
    // what that should be.
    //
    GameObjectListIterator pHGameObject;
    for (pHGameObject = m_gameObjectList.begin(); pHGameObject != m_gameObjectList.end(); /*++pHGameObject*/)
    {
        HGameObject     hGameObject = *pHGameObject;
        GameObject*     pGO;
        MAP_POSITION    mapPos;
        
//        CHR( GOMan.GetGameObjectPointer( hGameObject, &pGO ));
//        CPR( pGO );
        rval = GOMan.GetGameObjectPointer( hGameObject, &pGO );
        if (FAILED(rval))
        {
            // GameObject somehow deletd itself without the map being notified.
            // Remove it from our list.
            pHGameObject = m_gameObjectList.erase( pHGameObject );
            rval = S_OK;
            continue;
        }
    
        WorldToMapPosition( GOMan.GetPosition( hGameObject ), &mapPos );
        
        if ( mapPos.x < 0 || mapPos.x > m_gridWidth-1 || mapPos.y < 0 || mapPos.y > m_gridHeight-1)
        {
            string name;
            IGNOREHR(GOMan.GetName( hGameObject, &name ));
            RETAILMSG(ZONE_WARN, "GameMap::Update(): GameObject \"%s\" has moved off the map (%d, %d).", name.c_str(), mapPos.x, mapPos.y);
            pHGameObject++;
            continue;
        }

        UINT32 cellIndex = (mapPos.y * m_gridWidth) + mapPos.x;
        m_pGameObjectsGrid[ cellIndex ].push_front( hGameObject );
        
        pHGameObject++;
    }
    
Exit:
    return;
}





template<typename TYPE>
void
GameMap<TYPE>::Link( IN GameMap* pMap )
{
    m_linkedMapList->push_front( pMap );
}


template<typename TYPE>
void
GameMap<TYPE>::Unlink( IN GameMap* pMap )
{
    m_linkedMapList->remove( pMap );
}


template<typename TYPE>
void
GameMap<TYPE>::WorldToMapPosition( IN const WORLD_POSITION& worldPos, INOUT MAP_POSITION* pMapPos )
{
    WorldToMapPosition( worldPos.x, worldPos.y, worldPos.z, pMapPos );
}



template<typename TYPE>
void
GameMap<TYPE>::WorldToMapPosition( IN float world_x, IN float world_y, IN float world_z, INOUT MAP_POSITION* pMapPos )
{
    if (!pMapPos)
        return;

    pMapPos->x = (UINT32) ((world_x - m_worldOrigin.x) / m_fCellWidth);
    
    if (m_isVertical)
    {
        pMapPos->y = (UINT32) ((world_y - m_worldOrigin.y) / m_fCellWidth);
    }
    else 
    {
        pMapPos->y = (UINT32) ((world_z - m_worldOrigin.z) / m_fCellWidth);
    }
}



template<typename TYPE>
void
GameMap<TYPE>::MapToWorldPosition( IN const MAP_POSITION& mapPos, INOUT WORLD_POSITION* pWorldPos, IN bool centerInCell )
{
    MapToWorldPosition( mapPos.x, mapPos.y, pWorldPos, centerInCell );
}



template<typename TYPE>
void
GameMap<TYPE>::MapToWorldPosition( IN UINT32 grid_x, IN UINT32 grid_y, INOUT WORLD_POSITION* pWorldPos, IN bool centerInCell )
{
    if (!pWorldPos)
        return;

    pWorldPos->x = m_worldOrigin.x + (float) (grid_x * m_fCellWidth);
    
    if (m_isVertical)
    {
        pWorldPos->y = m_worldOrigin.y + (float) (grid_y * m_fCellWidth);
        pWorldPos->z = 0.0f;
    }
    else 
    {
        pWorldPos->y = 0.0f;
        pWorldPos->z = m_worldOrigin.z + (float) (grid_y * m_fCellWidth);
    }
    
    // Return a point in the center of the grid cell, as opposed to the corner?
    if (centerInCell)
    {
        pWorldPos->x += (m_fCellWidth / 2);

        if (m_isVertical)
        {
            pWorldPos->y += (m_fCellWidth / 2);
        }
        else
        {
            pWorldPos->z += (m_fCellWidth / 2);
        }
    }
}



template<typename TYPE>
void
GameMap<TYPE>::AddGameObject( IN HGameObject hGameObject )
{
    WORLD_POSITION worldPos;
    MAP_POSITION   mapPos;

    if (hGameObject.IsNull())
        return;

    worldPos = GOMan.GetPosition( hGameObject );
    WorldToMapPosition( worldPos, &mapPos );
    AddGameObject( hGameObject, mapPos );
}


template<typename TYPE>
void
GameMap<TYPE>::AddGameObject( IN HGameObject hGameObject, IN const MAP_POSITION& mapPos )
{
    AddGameObject( hGameObject, mapPos.x, mapPos.y );
}


template<typename TYPE>
void
GameMap<TYPE>::AddGameObject( IN HGameObject hGameObject, IN UINT32 x, IN UINT32 y )
{
    if (x >= m_gridWidth || y >= m_gridHeight) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameMap::AddGameObject(%d, %d) out of range",  x, y);
        return;
    }

    UINT32 cellIndex = (y * m_gridWidth) + x;
    m_pGameObjectsGrid[ cellIndex ].push_front( hGameObject );

    m_gameObjectList.push_back( hGameObject );
}


template<typename TYPE>
void
GameMap<TYPE>::RemoveGameObject( IN HGameObject hGameObject )
{
    WORLD_POSITION worldPos;
    MAP_POSITION   mapPos;

    if (hGameObject.IsNull())
        return;

    worldPos = GOMan.GetPosition( hGameObject);
    WorldToMapPosition( worldPos, &mapPos );
    RemoveGameObject( hGameObject, mapPos );
}


template<typename TYPE>
void
GameMap<TYPE>::RemoveGameObject( IN HGameObject hGameObject, IN const MAP_POSITION& mapPos )
{
    RemoveGameObject( hGameObject, mapPos.x, mapPos.y );
}


template<typename TYPE>
void
GameMap<TYPE>::RemoveGameObject( IN HGameObject hGameObject, IN UINT32 x, IN UINT32 y )
{
    if (x >= m_gridWidth || y >= m_gridHeight) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: GameMap::RemoveGameObject(%d, %d) out of range", x, y);
        return;
    }

    UINT32 cellIndex = (y * m_gridWidth) + x;
    m_pGameObjectsGrid[ cellIndex ].remove( hGameObject );

    m_gameObjectList.remove( hGameObject );
}



template<typename TYPE>
void
GameMap<TYPE>::Clear()
{
    // TODO: Bad bad if TYPE is non-integral, e.g. a class.

    memset( m_pValuesGrid, 0, m_gridHeight * m_gridWidth * sizeof(TYPE) );
    
    for (UINT32 row = 0; row < m_gridHeight; ++row)
    {
        for (UINT32 col = 0; col < m_gridWidth; ++col)
        {
            UINT32 cellIndex = (row*m_gridWidth) + col;
            m_pGameObjectsGrid[ cellIndex ].clear();
        }
    }
    
    m_gameObjectList.clear();
}


/*
template <typename TYPE>
void
GameMap<TYPE>::SetDisplayCallback( IN DISPLAYMAPCALLBACK callback  )
{
    m_pDisplayCallback = callback;
}


template<typename TYPE>
void
GameMap<TYPE>::SetClearCallback( IN CLEARMAPCALLBACK callback  )
{
    m_pClearCallback = callback;
}
*/


template<typename TYPE>
void
GameMap<TYPE>::EnableDebugDisplay( IN bool enabled )
{
    m_debugDisplay = enabled;

//    if (!m_debugDisplay && m_pClearCallback)
//        m_pClearCallback();
}



template<typename TYPE>
HGameObject
GameMap<TYPE>::FindClosest( IN const MAP_POSITION& pos, IN UINT32 radius, IN TYPE type )
{
    HGameObject rval;
    
    DEBUGCHK(0);

Exit:
    return rval;
}



template<typename TYPE>
HGameObject
GameMap<TYPE>::FindClosest( IN const MAP_POSITION& pos, IN const ivec2& direction, IN UINT32 distance, IN TYPE type )
{
    HGameObject rval;
    
    DEBUGCHK(0);

Exit:
    return rval;
}



template<typename TYPE>
RESULT
GameMap<TYPE>::FindAll( IN const MAP_POSITION& startPos, IN UINT32 radius, IN TYPE type, INOUT HGameObjectSet* pFoundObjectsSet )
{
    RESULT          rval       = S_OK;

    typedef std::set<MAP_POSITION, MapPositionSorter>  MapPositionSet;
    MapPositionSet openList, closedList;
    MAP_POSITION   currentCell;
    
    CPR(pFoundObjectsSet);
    
    // openlist = start pos
    // while (openlist.notempty)
    //   current cell =  openlist.head
    //   pList        += all GOs in current cell
    //
    //  foreach (neighbor) in current cell's neighbors
    //    if (neighbor distance from start) < radius
    //      if (neighbor not already visted)
    //        openlist += neighbor


    openList.insert( startPos );

    while ( !openList.empty() )
    {
        currentCell = *openList.begin(); 
        
        openList.erase(openList.begin());
        closedList.insert(currentCell);

//        DEBUGMSG(ZONE_INFO, "moved [%d,%d] to closedList. Open: %d Closed: %d", currentCell.x, currentCell.y, openList.size(), closedList.size());
        
        
/*        
        {
            MapPositionSet::iterator pMapPos;
            pMapPos = closedList.find( currentCell );
            DEBUGMSG(ZONE_INFO, "closedList.find([%d,%d]) = 0x%x", currentCell.x, currentCell.y, (UINT32)&(*pMapPos));
            DEBUGCHK(pMapPos != closedList.end());

            DEBUGMSG(ZONE_INFO, "--- ClosedList ---");
            for (pMapPos = closedList.begin(); pMapPos != closedList.end(); ++pMapPos)
            {
                MAP_POSITION pos = *pMapPos;
                DEBUGMSG(ZONE_INFO, "[%d,%d] ", pos.x, pos.y);
            }
            DEBUGMSG(ZONE_INFO, "--- ---------- ---");
        }
*/
        
        // Add all GOs in current cell that match the search type.
        HGameObjectList* pHGameObjects = GetGameObjects( currentCell );
        if (pHGameObjects)
        {
//            DEBUGMSG(ZONE_INFO, "FindAll( %d,%d ): %d GOs", currentCell.x, currentCell.y, pHGameObjects->size());

            HGameObjectListIterator pHGameObject;
            for (pHGameObject = pHGameObjects->begin(); pHGameObject != pHGameObjects->end(); ++pHGameObject)
            {
                HGameObject hGO = *pHGameObject;
                if (GOMan.GetType(hGO) & type)
                {
//                    DEBUGMSG(ZONE_INFO, "hGO 0x%x matches type %d", (UINT32)hGO, type);
                    pFoundObjectsSet->insert( hGO );
                }
            }
        }
        
        
        //
        // Add non-visited neighbor cells to the openList.
        //
        for (UINT32 dir = 0; dir < numDirections; ++dir)
        {
            MAP_POSITION neighborCell = currentCell + directions[dir];
            
            float distance = ivec2(neighborCell - startPos).Length();

            // Add neighbor cells to the openList if they are within the radius and have not been visited previously.
            if (distance <= (float)radius)
            {
                if (closedList.find( neighborCell ) == closedList.end())
                {
                    openList.insert( neighborCell );
//                    DEBUGMSG(ZONE_INFO, "openList += [%d, %d] count: %d", neighborCell.x, neighborCell.y, openList.size());
                }
                else
                {
//                    DEBUGMSG(ZONE_INFO, "neighborCell[%d,%d] already in closedList", neighborCell.x, neighborCell.y);
                }
            }
        }
        
    } // while ( !openList.empty() )

Exit:
    return rval;
}



template<typename TYPE>
RESULT
GameMap<TYPE>::FindAll( IN const MAP_POSITION& pos, IN const ivec2& direction, IN UINT32 distance, IN TYPE type, INOUT HGameObjectSet* pFoundObjectsSet )
{
    RESULT       rval       = S_OK;
    MAP_POSITION currentPos = pos;
    
    CPR(pFoundObjectsSet);
    
    while(distance)
    {
        //DEBUGMSG(ZONE_INFO, "FindAll( %d,%d ) distance: %d", currentPos.x, currentPos.y, distance);
        distance--;

//        {
//        WORLD_POSITION worldPos;
//        MapToWorldPosition(pos, &worldPos);
//        DebugRender.Quad( worldPos, Color::Red(), 0.2f, 64.0f );
//        }

        // Get list of GOs in cell.
        HGameObjectList* pHGameObjects = GetGameObjects( currentPos );
        if (!pHGameObjects)
            continue;
        
        //DEBUGMSG(ZONE_INFO, "FindAll( %d,%d ): %d GOs", currentPos.x, currentPos.y, pHGameObjects->size());


        // For each GO
        HGameObjectListIterator pHGameObject;
        for (pHGameObject = pHGameObjects->begin(); pHGameObject != pHGameObjects->end(); ++pHGameObject)
        {
            HGameObject hGO = *pHGameObject;
            if (GOMan.GetType(hGO) & type)
            {
                //DEBUGMSG(ZONE_INFO, "hGO 0x%x matches type %d", (UINT32)hGO, type);
                pFoundObjectsSet->insert( hGO );
            }
        }
        
        currentPos += direction;
    }


Exit:
    return rval;
}



} // END namespace Z
