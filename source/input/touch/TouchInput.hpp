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
    TOUCH_EVENT_BEGIN   = 0,
    TOUCH_EVENT_UPDATE,
    TOUCH_EVENT_END,
    TOUCH_EVENT_CANCEL,
} TOUCH_EVENT_TYPE;


struct TouchEvent
{
    UINT32              id;
    TOUCH_EVENT_TYPE    type;
    UINT64              timestamp;
    Point2D             point;
};



//
// Source of touch and multi-touch inputs.
//
// Registers with OS for touch notifications, and delivers them to touch listeners.
// This means asynch OS notifications will be queued to the listeners and processed inline 
// with the other game updates.
//
// Sends MSG_TouchBegin, MSG_TouchUpdate, and MSG_TouchEnd.  
// Each has an associated TouchEvent in msg->GetPointerData().
//

class TouchInput : virtual public Object, public IEventSource
{
public:
    // In the future we may support dual touch screens.
    static TouchInput&  GetDefaultTouchInput( );

    // IEventSource
//    virtual RESULT      AddListener         ( OBJECT_ID id, MSG_Name msg );
//    virtual RESULT      RemoveListener      ( OBJECT_ID id, MSG_Name msg );
    virtual RESULT      AddListener         ( HGameObject hListener, MSG_Name msg );    // TODO: either TouchInput, or some proxy class, should dispatch based on listener AABB
    virtual RESULT      RemoveListener      ( HGameObject hListener, MSG_Name msg );
//    virtual RESULT      RemoveListener      ( HGameObject hListener );


    // The native touch handler should call these methods to inject touches
    // into ZEngine.
    // NOTE: these methods must be thread-safe!
    RESULT              BeginTouch          ( TouchEvent* pTouchEvent    );
    RESULT              UpdateTouch         ( TouchEvent* pTouchEvent    );
    RESULT              EndTouch            ( TouchEvent* pTouchEvent    );

protected:
    RESULT              SendToListeners     ( TouchEvent* pTouchEvent    );

protected:
    TouchInput();
    TouchInput( const TouchInput& rhs );
    TouchInput& operator=( const TouchInput& rhs );
    virtual ~TouchInput();

protected:
    static TouchInput*  s_pDefaultTouchInput;
    

    typedef std::set<OBJECT_ID>                     EventListenerList;
    typedef std::map<MSG_Name, EventListenerList>   MsgToListenersMap;
    
    typedef EventListenerList::iterator             EventListenerListIterator;
    typedef MsgToListenersMap::iterator             MsgToListenersMapIterator;

    MsgToListenersMap   m_msgToListenersMap;
};


} // END namespace Z


