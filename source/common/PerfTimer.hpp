#pragma once

#include <mach/mach_time.h>
#include "Types.hpp"


namespace Z
{

    
class PerfTimer
{
public:
    PerfTimer();
    virtual ~PerfTimer();
    
    void    Start();
    void    Stop();
    double  ElapsedSeconds();
    double  ElapsedMilliseconds();
    
protected:
    mach_timebase_info_data_t   m_machTimebaseInfo;
    uint64_t                    m_startTick;
    uint64_t                    m_stopTick;
};


} // END namespace Z
