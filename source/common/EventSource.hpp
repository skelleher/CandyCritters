#pragma once

#include "msg.hpp"
#include "GameObject.hpp"

namespace Z
{



class IEventSource
{
public:
//    virtual RESULT AddListener      ( OBJECT_ID id, MSG_Name msg ) = 0;
//    virtual RESULT RemoveListener   ( OBJECT_ID id, MSG_Name msg ) = 0;
    virtual RESULT AddListener      ( HGameObject hListener, MSG_Name msg ) = 0;
    virtual RESULT RemoveListener   ( HGameObject hListener, MSG_Name msg ) = 0;

    virtual ~IEventSource() {};
};



} // END namespace Z
