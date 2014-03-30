#pragma once

#include "Types.hpp"
#include "Object.hpp"
#include "Platform.hpp"
#include "EventSource.hpp"

#include <set>
#include <map>
using std::set;
using std::map;


namespace Z
{


typedef enum
{
    ACCELEROMETER_EVENT_VECTOR   = 0,
    ACCELEROMETER_EVENT_SHAKE
} ACCELEROMETER_EVENT_TYPE;



struct AccelerometerEvent
{
    UINT32                      id;
    ACCELEROMETER_EVENT_TYPE    type;
    UINT64                      timestamp;
    vec3                        vector;
};



//
// Source of Accelerometer events.
//
// Registers with OS for Accelerometer notifications, and delivers them to Accelerometer listeners.
// This means asynch OS notifications will be queued to the listeners and processed inline 
// with the other game updates.
//
// Sends MSG_AccelerometerBegin, MSG_AccelerometerUpdate, and MSG_AccelerometerEnd.  
// Each has an associated AccelerometerEvent in msg->GetPointerData().
//

class Accelerometer : virtual public Object, public IEventSource
{
public:
    static Accelerometer&  GetDefaultAccelerometer( );

    // IEventSource
//    virtual RESULT      AddListener         ( OBJECT_ID id, MSG_Name msg );
//    virtual RESULT      RemoveListener      ( OBJECT_ID id, MSG_Name msg );
    virtual RESULT      AddListener         ( HGameObject hListener, MSG_Name msg );
    virtual RESULT      RemoveListener      ( HGameObject hListener, MSG_Name msg );


    // The native Accelerometer handler should call these methods to inject Accelerometeres
    // into ZEngine.
    // NOTE: these methods must be thread-safe!
    RESULT              DeliverEvent        ( AccelerometerEvent* pAccelerometerEvent    );

protected:
    RESULT              SendToListeners     ( AccelerometerEvent* pAccelerometerEvent    );

protected:
    Accelerometer();
    Accelerometer( const Accelerometer& rhs );
    Accelerometer& operator=( const Accelerometer& rhs );
    virtual ~Accelerometer();

protected:
    static Accelerometer*  s_pDefaultAccelerometer;
    

    typedef std::set<OBJECT_ID>                     EventListenerList;
    typedef std::map<MSG_Name, EventListenerList>   MsgToListenersMap;
    
    typedef EventListenerList::iterator             EventListenerListIterator;
    typedef MsgToListenersMap::iterator             MsgToListenersMapIterator;

    MsgToListenersMap   m_msgToListenersMap;
};


} // END namespace Z


