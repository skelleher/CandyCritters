#include "Accelerometer.hpp"
#include "GameObjectManager.hpp"


namespace Z
{


// Static data
Accelerometer*  Accelerometer::s_pDefaultAccelerometer = NULL;


Accelerometer::Accelerometer()
{
    DEBUGMSG(ZONE_OBJECT | ZONE_VERBOSE, "Accelerometer( %4d )", m_ID);
}


Accelerometer::~Accelerometer()
{
    DEBUGMSG(ZONE_OBJECT | ZONE_VERBOSE, "\t~Accelerometer( %4d )", m_ID);
    
    // TODO: clean up our listeners
    
    DEBUGCHK(0);
}



// In the future we may support dual Accelerometer screens.
Accelerometer&
Accelerometer::GetDefaultAccelerometer( )
{
    if (!s_pDefaultAccelerometer)
    {
        s_pDefaultAccelerometer = new Accelerometer();
    }
    
    s_pDefaultAccelerometer->AddRef();
    return *s_pDefaultAccelerometer;
}


/*
// IEventSource
RESULT
Accelerometer::AddListener( OBJECT_ID id, MSG_Name msg )
{
    RESULT rval = S_OK;
    
    // Get the list of listeners for this event (create a list if need be).
    EventListenerList& listeners = m_msgToListenersMap[ msg ];

    // Add the listener (to a set, guaranteeing listeners can only be added once)
    listeners.insert( id );
    
    return rval;
}



RESULT
Accelerometer::RemoveListener( OBJECT_ID id, MSG_Name msg )
{
    RESULT rval = S_OK;
    
    // Get the list of listeners for this event.
    MsgToListenersMapIterator pEventListeners;
    pEventListeners = m_msgToListenersMap.find( msg );

    if (pEventListeners != m_msgToListenersMap.end())
    {
        // Remove the listener
        EventListenerList& listeners = pEventListeners->second;
        
        EventListenerListIterator ppListener;
        ppListener = find( listeners.begin(), listeners.end(), id );
        if (ppListener != listeners.end())
        {
            listeners.erase( ppListener );
        }
    }

    return rval;
}
*/



// IEventSource
RESULT
Accelerometer::AddListener( HGameObject hListener, MSG_Name msg )
{
    RESULT rval = S_OK;
 
    // Get the list of listeners for this event (create a list if need be).
    EventListenerList& listeners = m_msgToListenersMap[ msg ];

    // Get the listener's unique OBJECT_ID.
    OBJECT_ID id = INVALID_OBJECT_ID;
    CHR(GOMan.GetObjectID( hListener, &id ));
          
    // Add the listener (to a set, guaranteeing listeners can only be added once)
    listeners.insert( id );

Exit:    
    return rval;
}



RESULT
Accelerometer::RemoveListener( HGameObject hListener, MSG_Name msg )
{
    RESULT rval = S_OK;

    // Get the list of listeners for this event.
    MsgToListenersMapIterator pEventListeners;
    pEventListeners = m_msgToListenersMap.find( msg );

    if (pEventListeners != m_msgToListenersMap.end())
    {
        // Remove the listener
        EventListenerList& listeners = pEventListeners->second;
        
        // Get the listener's unique OBJECT_ID.
        OBJECT_ID id = INVALID_OBJECT_ID;
        CHR(GOMan.GetObjectID( hListener, &id ));
          
        EventListenerListIterator ppListener;
        ppListener = find( listeners.begin(), listeners.end(), id );
        if (ppListener != listeners.end())
        {
            listeners.erase( ppListener );
        }
    }

Exit:    
    return rval;
}



// The native Accelerometer handler should call this method to inject AccelerometerEvents.
// into ZEngine.
// NOTE: these methods must be thread-safe!
RESULT
Accelerometer::DeliverEvent( AccelerometerEvent* pAccelerometerEvent )
{
    DEBUGMSG(ZONE_ACCELEROMETER, "Accelerometer::DeliverEvent( id: %d type: %d (%4.4f, %4.4f, %4.4f) )", 
        pAccelerometerEvent->id, 
        pAccelerometerEvent->type,
        pAccelerometerEvent->vector.x, 
        pAccelerometerEvent->vector.y,
        pAccelerometerEvent->vector.z);
    
    return SendToListeners( pAccelerometerEvent );
}



RESULT
Accelerometer::SendToListeners( AccelerometerEvent* pAccelerometerEvent )
{
    // TODO: take a critical section here

    RESULT                      rval                = S_OK;
    MSG_Name                    msg                 = MSG_AccelerometerEvent;
    MsgToListenersMapIterator   pEventListeners;

    if (!pAccelerometerEvent)
    {
        rval = E_NULL_POINTER;
        goto Exit;
    }


    // Get listeners for this event
    pEventListeners = m_msgToListenersMap.find( msg );

    if (pEventListeners != m_msgToListenersMap.end())
    {
        // Notify the listener(s)
        EventListenerList& listeners = pEventListeners->second;
        
        EventListenerListIterator ppListener;
        for (ppListener = listeners.begin(); ppListener != listeners.end(); ++ppListener)
        {
            OBJECT_ID eventListenerGOID = *ppListener;
        
            DEBUGMSG(ZONE_ACCELEROMETER | ZONE_VERBOSE, "Accelerometer::SendToListeners(): Accelerometer: %d goid: 0x%x",
                pAccelerometerEvent->type, eventListenerGOID);
            
            // Send a MSG_AccelerometerEvent, along with the AccelerometerEvent*
            CHR(GOMan.SendMessageFromSystem( eventListenerGOID, msg, pAccelerometerEvent ));
        }
    }
    
Exit:
    return rval;
}


} // END namespace Z

