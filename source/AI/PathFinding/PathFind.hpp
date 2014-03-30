#pragma once

#include "PathGraph.hpp"
#include "Vector.hpp"
#include "Types.hpp"
#include <list>
#include <set>

using std::list;
using std::multiset;


namespace Z
{



typedef enum 
{
    PATH_NODE_OPEN = 0,
    PATH_NODE_CLOSED,
    PATH_NODE_SRC,
    PATH_NODE_DST,
    PATH_NODE_WAYPOINT
} PATH_NODE_TYPE;


typedef std::list<WORLD_POSITION> WaypointList;
typedef void  (*DISPLAYPATHCALLBACK)( WORLD_POSITION pos, PATH_NODE_TYPE type );
typedef void  (*CLEARPATHCALLBACK)  ( void );
typedef float (*CUSTOMHEURISTIC)( NodeID src, NodeID dst );

typedef multiset<Connection*, ConnectionSorter>  OpenListType;
typedef multiset<Connection*>                    ClosedListType;

// TODO: dynamic sizing for path-finding grids
#define GRID_COLUMNS    25
#define GRID_ROWS       25


class PathFinder
{
public:
    PathFinder( PathGraph* pPathGraph );
    ~PathFinder();

    WORLD_POSITION  NodeIDToWorldPosition( NodeID         id );
    NodeID          WorldPositionToNodeID( WORLD_POSITION pos );
    WaypointList*   FindPath( WORLD_POSITION src, WORLD_POSITION goal, UINT32 maxCells = UINT_MAX );  // FREE the list when done!!

    void            SetHeuristicWeight( float fHeuristicWeight        );
    void            SetHeuristic      ( CUSTOMHEURISTIC heuristic     );    // Provide custom heuristic, tuned for the current map
    void            SetShowPathFinding( bool fShowPathFinding         );
    void            SetDisplayCallback( DISPLAYPATHCALLBACK callback  );
    void            SetClearCallback  ( CLEARPATHCALLBACK   callback  );
    void            SetPathSmoothing  ( bool fSmoothPath );

protected:
    WaypointList*   FindPathAStar( NodeID srcID, NodeID goalID, UINT32 maxCells = UINT_MAX );
    float           Heuristic( NodeID src, NodeID goalID );
    WORLD_POSITION  CatmullRom( WORLD_POSITION p0, WORLD_POSITION p1, WORLD_POSITION p2, WORLD_POSITION p3, float u );
    WaypointList*   SmoothWaypointList( WaypointList* pWaypointList );

    void            CleanUp();

    // TEST
    bool            IsListSorted( OpenListType* );

protected:
    CUSTOMHEURISTIC         m_pCustomHeuristic;     // Optional; may be set by the client (default is Euclidian distance)
    float                   m_fHeuristicWeight;
    bool                    m_bShowPathFinding;
    DISPLAYPATHCALLBACK     m_pDisplayCallback;
    CLEARPATHCALLBACK       m_pClearCallback;
    bool                    m_bPathSmoothing;
    PathGraph*              m_pPathGraph;

    // Connections will be sorted on insertion to Open List (by estimated cost to goal)
    // Closed list does not need any particular sorting, so we leave it to use default (sort by value of Connection*)
    // Make them class members so we can persist the path-finding state
    // across multiple calls to FindPath() [ time-slice the cost over multiple frames ]
    OpenListType            m_OpenList;
    ClosedListType          m_ClosedList;
    Connection*             m_pCurrentConnection;
    Connection*             m_pStartConnection;
};



} // END namespace Z



