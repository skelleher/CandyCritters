#include "TouchInput.hpp"
#include "GameObjectManager.hpp"


namespace Z
{


// Static data
TouchInput*  TouchInput::s_pDefaultTouchInput = NULL;


TouchInput::TouchInput()
{
    DEBUGMSG(ZONE_OBJECT | ZONE_VERBOSE, "TouchInput( %4d )", m_ID);
}


TouchInput::~TouchInput()
{
    DEBUGMSG(ZONE_OBJECT | ZONE_VERBOSE, "\t~TouchInput( %4d )", m_ID);
    
    // TODO: clean up our listeners.
    
    DEBUGCHK(0);
}



// In the future we may support dual touch screens.
TouchInput&
TouchInput::GetDefaultTouchInput( )
{
    if (!s_pDefaultTouchInput)
    {
        s_pDefaultTouchInput = new TouchInput();
    }
    
    s_pDefaultTouchInput->AddRef();
    return *s_pDefaultTouchInput;
}


/*
// IEventSource
RESULT
TouchInput::AddListener( OBJECT_ID id, MSG_Name msg )
{
    RESULT rval = S_OK;
    
    // Get the list of listeners for this event (create a list if need be).
    EventListenerList& listeners = m_msgToListenersMap[ msg ];

    // Add the listener (to a set, guaranteeing listeners can only be added once)
    listeners.insert( id );
    
    return rval;
}



RESULT
TouchInput::RemoveListener( OBJECT_ID id, MSG_Name msg )
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
TouchInput::AddListener( HGameObject hListener, MSG_Name msg )
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

/*
RESULT
TouchInput::RemoveListener( HGameObject hListener )
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
*/


RESULT
TouchInput::RemoveListener( HGameObject hListener, MSG_Name msg )
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



// The native touch handler should call these methods to inject touches
// into ZEngine.
// NOTE: these methods must be thread-safe!
RESULT
TouchInput::BeginTouch( TouchEvent* pTouchEvent )
{
    DEBUGMSG(ZONE_TOUCH | ZONE_VERBOSE, "TouchInput::BeginTouch( 0x%x, %4.2f x %4.2f )", 
        pTouchEvent->id, pTouchEvent->point.x, pTouchEvent->point.y);
    
    return SendToListeners( pTouchEvent );
}



RESULT
TouchInput::UpdateTouch( TouchEvent* pTouchEvent )
{
    DEBUGMSG(ZONE_TOUCH | ZONE_VERBOSE, "TouchInput::UpdateTouch( 0x%x, %4.2f x %4.2f )", 
        pTouchEvent->id, pTouchEvent->point.x, pTouchEvent->point.y);
    
    return SendToListeners( pTouchEvent );
}



RESULT
TouchInput::EndTouch( TouchEvent* pTouchEvent )
{
    DEBUGMSG(ZONE_TOUCH | ZONE_VERBOSE, "TouchInput::EndTouch( 0x%x, %4.2f x %4.2f )", 
        pTouchEvent->id, pTouchEvent->point.x, pTouchEvent->point.y);
    
    return SendToListeners( pTouchEvent );
}



RESULT
TouchInput::SendToListeners( TouchEvent* pTouchEvent )
{
    // TODO: take a critical section here

    RESULT                      rval                = S_OK;
    MSG_Name                    msg                 = MSG_NULL;
    MsgToListenersMapIterator   pEventListeners;

    if (!pTouchEvent)
    {
        rval = E_NULL_POINTER;
        goto Exit;
    }


    //
    // Convert from iOS coordinates to ZEngine coordinates.
    // iOS puts (0,0) at the upper-left corner.  
    // ZEngine puts (0,0) at the lower-left corner, and also scales the screen.
    //
    // TODO: add an IRenderer method to do this?
    //
    {
        Point2D deviceCoords = pTouchEvent->point;
        Point2D engineCoords;
        
        float     worldScaleFactor;
        float     screenScaleFactor;
        Rectangle screenRect;


        //
        // Convert from points to pixels.
        //
        Platform::GetScreenRect ( &screenRect );
        screenScaleFactor = Platform::GetScreenScaleFactor(); 
    
        engineCoords.x = deviceCoords.x * screenScaleFactor;
        engineCoords.y = screenRect.height - (deviceCoords.y * screenScaleFactor);
    
        worldScaleFactor = GlobalSettings.GetFloat("/Settings.fWorldScaleFactor", 1.0f); 
        engineCoords.x    /= worldScaleFactor;
        engineCoords.y    /= worldScaleFactor;
    
        RETAILMSG(ZONE_TOUCH | ZONE_VERBOSE, "TouchInput: convert from device (%2.2f x %2.2f) to Zengine (%2.2f x %2.2f), screenRect %2.2f x %2.2f, screenScaleFactor %2.2f, worldScaleFactor %2.2f",
            deviceCoords.x, deviceCoords.y,
            engineCoords.x, engineCoords.y,
            screenRect.width,
            screenRect.height,
            screenScaleFactor,
            worldScaleFactor);
            
        pTouchEvent->point = engineCoords;
    }
    
    switch (pTouchEvent->type)
    {
        case TOUCH_EVENT_BEGIN:
            msg = MSG_TouchBegin;
            break;
        case TOUCH_EVENT_UPDATE:
            msg = MSG_TouchUpdate;
            break;
        case TOUCH_EVENT_END:
            msg = MSG_TouchEnd;
            break;
        default:
            RETAILMSG(ZONE_ERROR, "ERROR: TouchInput::SendToListeners(): invalid touchEventType %d",
                pTouchEvent->type);
    }


    // Get listeners for this event
    pEventListeners = m_msgToListenersMap.find( msg );

    if (pEventListeners != m_msgToListenersMap.end())
    {
        // Notify the listener(s)
        EventListenerList& listeners = pEventListeners->second;
        
        EventListenerListIterator ppListener;
        for (ppListener = listeners.begin(); ppListener != listeners.end(); )
        {
            OBJECT_ID eventListenerGOID = *ppListener;
        
            DEBUGMSG(ZONE_TOUCH | ZONE_VERBOSE, "TouchInput::SendToListeners(): touch: %d goid: %d",
                pTouchEvent->type, eventListenerGOID);
            
            // Send a MSG_TouchXXX, along with the TouchEvent*
            rval = GOMan.SendMessageFromSystem( eventListenerGOID, msg, pTouchEvent );
            if (FAILED(rval))
            {
                // Object may have deleted itself without unregistering; remove it from the list.
                ppListener = listeners.erase( ppListener );
            }
            else
            {
                ++ppListener;
            }
        }
    }
    
Exit:
    return rval;
}


} // END namespace Z

