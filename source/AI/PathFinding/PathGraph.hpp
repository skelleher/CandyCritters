#pragma once

#include <map>
using std::map;


namespace Z
{


    
typedef unsigned int NodeID;

struct Connection
{
    float           fCost;
    float           fCostSoFar;
    float           fEstimatedCostToGoal;

    Connection*     pParentConnection;
    NodeID          parentNodeID;
    NodeID          destinationNodeID;

    // One connection is cheaper than another if the estimated-cost-to-goal is less
    bool operator<(const Connection &rhs)  { return (this->fEstimatedCostToGoal < rhs.fEstimatedCostToGoal ? true : false); }
};



// Functor used to sort Connections when inserted into STL containers
class ConnectionSorter
{
public:
    bool operator() (const Connection* lhs, const Connection* rhs) const
    {
        // Sort by fEstimateCostToGoal
        // If equal, fallback on sorting by address in memory
        // This ENSURES that nodes with identical cost will never be seen as "equal"
        // Otherwise multiset.erase( node ) will erase ALL NODES with identical cost!

        if (lhs && rhs && lhs->fEstimatedCostToGoal < rhs->fEstimatedCostToGoal)
        {
            return true;
        }
        else if (lhs && rhs && lhs->fEstimatedCostToGoal == rhs->fEstimatedCostToGoal)
        {
            if ( lhs < rhs)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        return false;
    }
};



// Our game levels are a 2D square grid: each cell can have at most 8 connections
#define MAX_CONNECTIONS     8
struct ConnectionList
{
    unsigned int    numConnections;
    Connection      connections[MAX_CONNECTIONS];
};
typedef map<NodeID, ConnectionList*> NodeConnectionsMap;



class PathGraph
{
public:
    PathGraph();
    ~PathGraph();

    void        GetConnections( NodeID nodeID, ConnectionList** ppConnectionList );
    void        AddConnections( NodeID nodeID, ConnectionList*  pConnectionList, unsigned int numConnections );

private:
    NodeConnectionsMap m_MapConnectionLists;
};



} // END namespace Z


    
