#include "PathFind.hpp"
#include "Log.hpp"
#include "Macros.hpp"
#include <math.h>



namespace Z
{


    
PathFinder::PathFinder( PathGraph* pPathGraph ) :
    m_pPathGraph(pPathGraph),
    m_pCustomHeuristic(NULL),
    m_bShowPathFinding(false),
    m_pDisplayCallback(NULL),
    m_pClearCallback(NULL),
    m_fHeuristicWeight(1.0f),
    m_bPathSmoothing(false),
    m_pCurrentConnection(NULL),
    m_pStartConnection(NULL)
{
    if (!m_pPathGraph)
    {
        RETAILMSG(ZONE_ERROR, "PathFinder() needs a PathGraph* when instantiated\n");
        DEBUGCHK(0);
    }
}



PathFinder::~PathFinder()
{
}



WORLD_POSITION
PathFinder::NodeIDToWorldPosition( NodeID nodeID )
{
    WORLD_POSITION rval(0,0,0);

    rval.y = 0;
    rval.z = nodeID / GRID_COLUMNS;
    rval.x = nodeID % GRID_COLUMNS;

    return rval;
}



NodeID
PathFinder::WorldPositionToNodeID( WORLD_POSITION pos )
{
    NodeID rval = ((unsigned int)pos.z) * GRID_COLUMNS + (unsigned int)pos.x;

    return rval;
}



void
PathFinder::SetHeuristicWeight( float fWeight )
{
    if (fWeight >= 0.0)
        m_fHeuristicWeight = fWeight;
}



float 
PathFinder::Heuristic( NodeID srcID, NodeID goalID )
{
    if (m_pCustomHeuristic)
       return m_pCustomHeuristic( srcID, goalID ) * m_fHeuristicWeight;


    WORLD_POSITION srcPos  = NodeIDToWorldPosition( srcID  );
    WORLD_POSITION goalPos = NodeIDToWorldPosition( goalID );

    // Return the Euclidian distance between two points.
    // Works better in large open areas without obstacles.
    float x = (float)(goalPos.x - srcPos.x);
    float y = (float)(goalPos.y - srcPos.y);
    
    return sqrt( x*x + y*y ) * m_fHeuristicWeight;
}



void
PathFinder::SetPathSmoothing( bool fSmoothPath )
{
    m_bPathSmoothing = fSmoothPath;
}



void
PathFinder::SetShowPathFinding( bool fShowPathFinding )
{
    m_bShowPathFinding = fShowPathFinding;
}



WaypointList*
PathFinder::FindPath( WORLD_POSITION src, WORLD_POSITION goal, UINT32 maxCells )
{
    if (!m_pPathGraph)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: PathFinder::FindPath(): m_pPathGraph is NULL\n");
        return NULL;
    }
    
    NodeID srcID    = WorldPositionToNodeID( src  );
    NodeID goalID   = WorldPositionToNodeID( goal );
    //    RETAILMSG(ZONE_PATHFINDER, ("FindPath: (%.2f, %.2f, %.2f) [%d] -> (%.2f, %.2f, %.2f) [%d]\n", 
    //        src.x,  src.y,  src.z,   srcID, 
    //        goal.x, goal.y, goal.z,  goalID );
    
    
    // Find the list of connections from Cell A to Cell B
    WaypointList *pWaypointList = FindPathAStar( srcID, goalID, maxCells );
    
    if (m_bPathSmoothing)
        pWaypointList = SmoothWaypointList( pWaypointList );
    
    if (pWaypointList)
    {
	    for( WaypointList::iterator pWaypoint = pWaypointList->begin(); pWaypoint != pWaypointList->end(); pWaypoint++ )
	    {
            if (m_bShowPathFinding && m_pDisplayCallback)
            {
                WORLD_POSITION temp = *pWaypoint;
                temp.x -= 0.5;
                temp.z -= 0.5;
                m_pDisplayCallback( temp, PATH_NODE_WAYPOINT );
            }
        }
    }
    else
    {
        if (m_bShowPathFinding && m_pClearCallback)
            m_pClearCallback();
    }
    
    return pWaypointList;
}



WaypointList*
PathFinder::FindPathAStar( NodeID srcID, NodeID goalID, UINT32 maxCells )
{
    if (!m_pPathGraph)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: PathFinder::FindPathAStar(): m_pPathGraph is NULL\n");
        return NULL;
    }

    WaypointList* pWaypointList     = NULL;
    float         fExpectedCost;
    bool          bFoundGoal        = false;

    UINT32 numCells = 0;

    WORLD_POSITION srcPos  = NodeIDToWorldPosition( srcID );
    WORLD_POSITION goalPos = NodeIDToWorldPosition( goalID );
    fExpectedCost          = Heuristic( srcID, goalID );

    // Add the starting point to open list
    if (!m_pCurrentConnection)
    {
        // Finding a new path; as good a place as any to erase all debug visualization
        // for the previous path.
        if (m_bShowPathFinding && m_pClearCallback)
            m_pClearCallback();

        m_pStartConnection = new Connection();
        m_pStartConnection->destinationNodeID       = srcID;
        m_pStartConnection->fCost                   = 0.0f;
        m_pStartConnection->fCostSoFar              = 0.0f;
        m_pStartConnection->fEstimatedCostToGoal    = fExpectedCost;
        m_pStartConnection->pParentConnection       = NULL;
        m_pStartConnection->parentNodeID            = -1;

        m_pCurrentConnection = m_pStartConnection;
        m_OpenList.insert( m_pStartConnection );
    }
    

    while( !m_OpenList.empty() )
    {
        
        // TEST TEST: validate that the list is sorted!
        //if (!IsListSorted( &m_OpenList ) )
        //{
        //    RETAILMSG(ZONE_ERROR, "ERROR: m_OpenList is not sorted!\n");
        //}


        // Choose "cheapest" potential connection; first item in OpenList
        m_pCurrentConnection = *(m_OpenList.begin());
        if (!m_pCurrentConnection)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: PathFinder::FindPathAStar(): m_pCurrentConnection is NULL");
            break;
        }
        

        RETAILMSG(ZONE_PATHFINDER | ZONE_VERBOSE, "Current Connection: %d -> %d (cost=%f) (costSoFar=%f) (estimatedCostToGoal=%f)\n", 
             m_pCurrentConnection->parentNodeID,
             m_pCurrentConnection->destinationNodeID,
             m_pCurrentConnection->fCost,
             m_pCurrentConnection->fCostSoFar,
             m_pCurrentConnection->fEstimatedCostToGoal);

        // Is this connection our goal?
        if (m_pCurrentConnection->destinationNodeID == goalID)
        {
            // Done!
            bFoundGoal = true;
            break;
        }

        //
        // foreach visibleConnection (m_pCurrentConnection)
        //
        ConnectionList* pConnectionList;
        m_pPathGraph->GetConnections( m_pCurrentConnection->destinationNodeID, &pConnectionList );
        if (!pConnectionList)
        {
            // No connections from here; should never happen
            // Starting point may be inside a wall, or off the map; clean up and return
            RETAILMSG(ZONE_ERROR, "ERROR: pathfinding found an unreachable node. Giving up.\n");
            goto Exit;
        }

        for (unsigned int i = 0; i < pConnectionList->numConnections; ++i)
        {
            Connection* pVisibleConnection = &pConnectionList->connections[ i ];
            float fCostForThisConnection = m_pCurrentConnection->fCostSoFar + pVisibleConnection->fCost;

            // if visibleConnection on closed list
            if (m_ClosedList.find( pVisibleConnection ) != m_ClosedList.end())
            {
                if ( pVisibleConnection->fCostSoFar <= fCostForThisConnection )
                {
                    // current connection not any cheaper; skip
                    continue;
                }
                else
                {
                    m_ClosedList.erase( pVisibleConnection );
                    
                    DEBUGMSG(ZONE_PATHFINDER | ZONE_VERBOSE, "-Closed List: %d -> %d (cost=%f) (costSoFar=%f) (estimatedCostToGoal=%f)\n", 
                        m_pCurrentConnection->parentNodeID,
                        m_pCurrentConnection->destinationNodeID,
                        m_pCurrentConnection->fCost,
                        m_pCurrentConnection->fCostSoFar,
                        m_pCurrentConnection->fEstimatedCostToGoal);
                }
            } 

            OpenListType::iterator ppConnection;
            ppConnection = m_OpenList.find( pVisibleConnection );
            if ( ppConnection != m_OpenList.end() && *ppConnection == pVisibleConnection )
            {
                // If visible connection is still cheaper than current connection
                if (pVisibleConnection->fCostSoFar <= fCostForThisConnection)
                {
                    continue;  // still viable; keep on the open list
                }
            }
            else
            {
                //
                // First time visiting this connection
                //

                // Store cost-so-far to visibleConnection
                pVisibleConnection->fCostSoFar = fCostForThisConnection;

                // Store parent to visibleConnection
                pVisibleConnection->parentNodeID      = m_pCurrentConnection->destinationNodeID;
                pVisibleConnection->pParentConnection = m_pCurrentConnection;

                // Store estimate-total-cost-from-here to visibleConnection
                pVisibleConnection->fEstimatedCostToGoal = fCostForThisConnection + Heuristic( pVisibleConnection->destinationNodeID, goalID );


                // Add to open list
                m_OpenList.insert( pVisibleConnection );
                
                DEBUGMSG(ZONE_PATHFINDER | ZONE_VERBOSE, "+Open List: %d -> %d (cost=%f) (costSoFar=%f) (estimatedCostToGoal=%f)\n", 
                    pVisibleConnection->parentNodeID,
                    pVisibleConnection->destinationNodeID,
                    pVisibleConnection->fCost,
                    pVisibleConnection->fCostSoFar,
                    pVisibleConnection->fEstimatedCostToGoal);

                // TEST TEST: validate that the list is sorted!
                //if (!IsListSorted( &m_OpenList ) )
                //{
                //    RETAILMSG(ZONE_ERROR, "ERROR: m_OpenList is not sorted!\n");
                //}
            }
        } // END considering each connection from current node


        // Done sorting all connections from current node into open (viable) and closed (too expensive)
        // remove m_pCurrentConnection from open list
        m_OpenList.erase( m_pCurrentConnection );
        DEBUGMSG(ZONE_PATHFINDER | ZONE_VERBOSE, "-Open List:   %d -> %d (cost=%f) (costSoFar=%f) (estimatedCostToGoal=%f)\n", 
            m_pCurrentConnection->parentNodeID,
            m_pCurrentConnection->destinationNodeID,
            m_pCurrentConnection->fCost,
            m_pCurrentConnection->fCostSoFar,
            m_pCurrentConnection->fEstimatedCostToGoal);

        // put m_pCurrentConnection on closed list
        m_ClosedList.insert( m_pCurrentConnection );
        DEBUGMSG(ZONE_PATHFINDER | ZONE_VERBOSE, "+Closed List: %d -> %d (cost=%f) (costSoFar=%f) (estimatedCostToGoal=%f)\n", 
            m_pCurrentConnection->parentNodeID,
            m_pCurrentConnection->destinationNodeID,
            m_pCurrentConnection->fCost,
            m_pCurrentConnection->fCostSoFar,
            m_pCurrentConnection->fEstimatedCostToGoal);


        // If we've run too long, abort the search
        // Will resume where we left off on the next call to PathFindAStar().
        if (++numCells >= maxCells)
            break;
    } // END while( open list )

Exit:
    //
    // Display the Open and Closed lists for debugging
    //
    if (m_bShowPathFinding && m_pDisplayCallback)
    {
        ClosedListType::iterator ppClosedConnection;
        for (ppClosedConnection = m_ClosedList.begin(); ppClosedConnection != m_ClosedList.end(); ++ppClosedConnection)
        {
            Connection* pConnection = *ppClosedConnection;
            m_pDisplayCallback( NodeIDToWorldPosition( pConnection->destinationNodeID ), PATH_NODE_CLOSED );
        }

        OpenListType::iterator ppOpenConnection;
        for (ppOpenConnection = m_OpenList.begin(); ppOpenConnection != m_OpenList.end(); ++ppOpenConnection)
        {
            Connection* pConnection = *ppOpenConnection;
            m_pDisplayCallback( NodeIDToWorldPosition( pConnection->destinationNodeID ), PATH_NODE_OPEN );
        }

    }

    // If we got here, the open list is empty, or we ran out of time for this frame
//    if (m_pCurrentConnection->destinationNodeID != goalID)
    if (!bFoundGoal && m_OpenList.empty() )
    {
        if ( !m_OpenList.empty() )
        {
            DEBUGMSG(ZONE_PATHFINDER | ZONE_VERBOSE, "Halting search until next frame.\n");
        }
        else
        {
            DEBUGMSG(ZONE_PATHFINDER, "Found NO path to goalID!\n");
            CleanUp();
        }
    }
    else
    {
        DEBUGMSG(ZONE_PATHFINDER, "Found path to goalID.\n");


        // Walk back the connection list
        // Convert each connection into a Waypoint
        pWaypointList = new WaypointList();
//        while( m_pCurrentConnection )
        while( m_pCurrentConnection && m_pCurrentConnection->fCost > 0.0f )
        {
            pWaypointList->push_front( NodeIDToWorldPosition( m_pCurrentConnection->destinationNodeID ) );
            
            m_pCurrentConnection = m_pCurrentConnection->pParentConnection;
        }

        CleanUp();
    }

    return pWaypointList;
}


void
PathFinder::SetDisplayCallback( DISPLAYPATHCALLBACK callback )
{
    m_pDisplayCallback = callback;
}


void
PathFinder::SetClearCallback( CLEARPATHCALLBACK callback )
{
    m_pClearCallback = callback;
}


void
PathFinder::CleanUp()
{
    // m_pStartConnection was allocated to kick-start the algorithm; free it.
    // The other connections are owned by the PathGraph and persist until
    // we unload or modify the level.
    SAFE_DELETE( m_pStartConnection );

    m_pStartConnection   = NULL;
    m_pCurrentConnection = NULL;

    m_OpenList.clear();
    m_ClosedList.clear();
}



WORLD_POSITION
PathFinder::CatmullRom( WORLD_POSITION p0, WORLD_POSITION p1, WORLD_POSITION p2, WORLD_POSITION p3, float u )
{
	float u_squared = u*u;
	float u_cubed = u_squared*u;

	vec3 result = p0 * (-0.5f*u_cubed + u_squared - 0.5f*u) +
		                p1 * (1.5f*u_cubed - 2.5f*u_squared + 1.0f) +
						p2 * (-1.5f*u_cubed + 2.0f*u_squared + 0.5f*u) +
						p3 * (0.5f*u_cubed - 0.5f*u_squared);

	return result;
}


WaypointList*
PathFinder::SmoothWaypointList( WaypointList* pWaypointList )
{
    if (!pWaypointList)
        return NULL;

	if( pWaypointList->size() > 2 )
	{
		WORLD_POSITION newPoint0( 0.0f, 0.0f, 0.0f );
		WORLD_POSITION newPoint1( 0.0f, 0.0f, 0.0f );

		WaypointList oldList;
		for( WaypointList::iterator i = pWaypointList->begin(); i != pWaypointList->end(); i++ )
		{
			oldList.push_back( *i );
		}
		pWaypointList->clear();

		WORLD_POSITION p0, p1, p2, p3;

		//First
		WaypointList::iterator iter = oldList.begin();
		p0 = p1 = p2 = *iter;
		iter++;
		p3 = *iter;
		iter++;

		const int divisions = 5;
		while( iter != oldList.end() )
		{
			p0 = p1;
			p1 = p2;
			p2 = p3;
			p3 = *iter;

			for( int count=1; count<divisions+1; count++)
			{
				vec3 newPoint = CatmullRom( p0, p1, p2, p3, (float)count/(float)divisions );
				if( pWaypointList->size() > 1 )
				{
					vec3 lastDir = newPoint1 - newPoint0;
					vec3 curDir = newPoint - newPoint1;
                    
                    lastDir.Normalize();
                    curDir.Normalize();
                    float dot = lastDir.Dot( curDir );
                    
					if( dot > 0.99f )
					{
						pWaypointList->pop_back();
					}
				}
				newPoint0 = newPoint1;
				newPoint1 = newPoint;
				pWaypointList->push_back( newPoint );
			}
			iter++;
		}

		//Last
		p0 = p1;
		p1 = p2;
		p2 = p3;
		p3 = p3;	//Redundant, but put here to make it clear what is intended
		
		for( int count=1; count<divisions+1; count++)
		{
			WORLD_POSITION newPoint = CatmullRom( p0, p1, p2, p3, (float)count/(float)divisions );
			if( pWaypointList->size() > 1 )
			{
				vec3 lastDir = newPoint1 - newPoint0;
				vec3 curDir = newPoint - newPoint1;
                
                lastDir.Normalize();
                curDir.Normalize();
                float dot = lastDir.Dot( curDir );
                
				if( dot > 0.99f )
				{
					pWaypointList->pop_back();
				}
			}
			newPoint0 = newPoint1;
			newPoint1 = newPoint;
			pWaypointList->push_back( newPoint );
		}
	}

    return pWaypointList;
}


// TEST TEST
bool
PathFinder::IsListSorted( OpenListType* pOpenList )
{
    Connection* pA;
    Connection* pB;

    OpenListType::iterator ppConnection;
    for (ppConnection = pOpenList->begin(); ppConnection != pOpenList->end(); /* ++ppConnection */)
    {
        pA = *ppConnection;
        ++ppConnection;

        if (ppConnection == pOpenList->end())
            return true;

        pB = *ppConnection;

        if ( *pB < *pA)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: Open list not sorted\n");
            return false;
        }
    }

    return true;
}



} // END namespace Z



