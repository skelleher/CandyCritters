#include "Time.hpp"

namespace Z
{

   
Time* Time::s_pGlobalTime = NULL;



Time& 
Time::Instance()
{
    if (!s_pGlobalTime)
    {
        s_pGlobalTime = new Time();
    }
    
    return *s_pGlobalTime;
}


Time::Time() : 
    m_isPaused(false),
    m_speed(1.0),
    m_startTime(0),
    m_currentTime(0),
    m_previousTime(0)
{
    m_previousTime = m_startTime = Platform::GetTickCount();
    m_currentTime  = 0;
}


void 
Time::Pause()
{
    m_isPaused      = true;
    m_previousTime  = Platform::GetTickCount();
}



void 
Time::Resume()
{
    m_isPaused      = false;
    m_previousTime  = Platform::GetTickCount();
}
    
    

// Return game time in milliseconds since start.
UINT64 
Time::GetTime()
{
    if (m_isPaused)
    {
        return m_currentTime * m_speed;
    }

    UINT64 current = Platform::GetTickCount();
    

    m_currentTime += current - m_previousTime;
    m_previousTime = current;

    return m_currentTime * m_speed;
}


// Return game time in seconds since start.
double 
Time::GetTimeDouble()
{
    UINT64 milliseconds = GetTime();
    double rval         = (double)milliseconds/1000.0;

    return rval;
}
    

void 
Time::SetSpeed( double speed )
{
    m_speed = speed;
}


double 
Time::GetSpeed( )
{
    return m_speed;
}




} // END namespace Z
