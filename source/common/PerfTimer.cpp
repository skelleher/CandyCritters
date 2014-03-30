/*
 *  PerfTimer.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 9/6/10.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 *
 *  This class implements a high-resolution performance timer.
 *  The class is so small that we call MACH directly, rather
 *  than going through the Platform:: abstraction layer.
 *
 *  Resolution is dependent on the underlying hardware/OS.
 */



#include "PerfTimer.hpp"


namespace Z
{


PerfTimer::PerfTimer() :
    m_startTick(0),
    m_stopTick(0)
{
    // Save information about the high-resolution timer (e.g. ticks per millisecond)
    mach_timebase_info( &m_machTimebaseInfo );
}



PerfTimer::~PerfTimer()
{
}



void
PerfTimer::Start()
{
    m_startTick = mach_absolute_time();
    m_stopTick  = 0;
}



void    
PerfTimer::Stop()
{
    m_stopTick = mach_absolute_time();
}



double  
PerfTimer::ElapsedSeconds()
{
    double elapsedTicks;
    
    if (m_stopTick)
    {
        elapsedTicks = m_stopTick - m_startTick;
    }
    else
    {
        elapsedTicks = mach_absolute_time() - m_startTick;
    }
    
    double seconds = elapsedTicks * (((double)m_machTimebaseInfo.numer/(double)m_machTimebaseInfo.denom) / 1000000000.0);
    
    return seconds;
}


double  
PerfTimer::ElapsedMilliseconds()
{
    double elapsedTicks;
    
    if (m_stopTick)
    {
        elapsedTicks = m_stopTick - m_startTick;
    }
    else
    {
        elapsedTicks = mach_absolute_time() - m_startTick;
    }
    
    double milliseconds = elapsedTicks * (((double)m_machTimebaseInfo.numer/(double)m_machTimebaseInfo.denom) / 1000000.0);
    
    return milliseconds;
}

  
  
} // END namespace Z
