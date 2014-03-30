#include "PathGraph.hpp"
#include "Log.hpp"


namespace Z
{



PathGraph::~PathGraph()
{
    RETAILMSG(ZONE_PATHFINDER, "\t~PathGraph(): freeing connections\n");

	// Delete all connection lists
	NodeConnectionsMap::iterator ppConnectionList;
    for (ppConnectionList = m_MapConnectionLists.begin(); ppConnectionList != m_MapConnectionLists.end(); ++ppConnectionList)
	{
        // Get ConnectionList for this NodeID
        ConnectionList* pConnectionList = ppConnectionList->second; 
		delete pConnectionList;
	}
}




void
PathGraph::GetConnections( NodeID nodeID, ConnectionList** ppConnectionList )
{
    *ppConnectionList = m_MapConnectionLists[ nodeID ];
}



void
PathGraph::AddConnections( NodeID nodeID, ConnectionList* pConnectionList, unsigned int numConnections )
{
    m_MapConnectionLists.insert( std::make_pair(nodeID, pConnectionList) );
}
    
    

} // END namespace Z


