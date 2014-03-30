#pragma once

#include "Types.hpp"

namespace Z
{

   
typedef enum 
{
    METRIC_RUNTIME_SECONDS = 0,
    METRIC_FPS_AVG,
    METRIC_FPS_LOW,
    METRIC_FPS_HIGH,
    METRIC_RAM_LOW,
    METRIC_RAM_HIGH,
    METRIC_TEXTURE_CHANGES,
    
    MAX_METRIC_ID
} METRIC_ID;


typedef enum 
{
    METRIC_UINT64 = 0,
    METRIC_INT64,
    METRIC_DOUBLE
} METRIC_TYPE;


typedef union 
{
    UINT64  m_uint64;
    INT64   m_int64;
    double  m_double;
} MetricValue;


typedef struct 
{
    METRIC_ID   id;
    METRIC_TYPE type;
    MetricValue value;
} Metric;


class Metrics
{
public:
    static void* Get( METRIC_ID id );    
    static void  Set( METRIC_ID id, void* pValue );
    
protected:
    Metrics();
    Metrics( const Metrics& rhs );
    Metrics& operator=( const Metrics& rhs );
    virtual ~Metrics();
    
protected:
    static Metric s_metrics[ MAX_METRIC_ID ];
};


} // END namespace Z
