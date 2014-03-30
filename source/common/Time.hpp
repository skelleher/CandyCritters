#pragma once

#include "Platform.hpp"
#include "Log.hpp"

namespace Z
{


class Time
{
public:
    static Time& Instance ( );                  // Creates and starts the Timer on first call.

    Time();
    ~Time() {};

    void    Pause         ( );
    void    Resume        ( );
    UINT64  GetTime       ( );                  // Return time in milliseconds since start.
    double  GetTimeDouble ( );                  // Return time in seconds since start.
    void    SetSpeed      ( double speed );
    double  GetSpeed      ( );
    
protected:
    bool    m_isPaused;
    double  m_speed;
    UINT64  m_startTime;
    UINT64  m_currentTime;
    UINT64  m_previousTime;
    
protected:
    static  Time*   s_pGlobalTime;
};

#define GameTime ((Z::Time&)Z::Time::Instance())

} // END namespace Z

