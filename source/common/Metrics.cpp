
#include "Metrics.hpp"
#include "Log.hpp"
#include "Macros.hpp"

   
namespace Z
{


Metric Metrics::s_metrics[ MAX_METRIC_ID ] =
{
    { METRIC_RUNTIME_SECONDS,   METRIC_UINT64,  0 },
    { METRIC_FPS_AVG,           METRIC_DOUBLE,  0 },
    { METRIC_FPS_LOW,           METRIC_DOUBLE,  0 },
    { METRIC_FPS_HIGH,          METRIC_DOUBLE,  0 },
    { METRIC_RAM_LOW,           METRIC_UINT64,  0 },
    { METRIC_RAM_HIGH,          METRIC_UINT64,  0 },
    { METRIC_TEXTURE_CHANGES,   METRIC_UINT64,  0 },
};


void* 
Metrics::Get( METRIC_ID id )
{
    if (id >= MAX_METRIC_ID)
    {
        RETAILMSG(ZONE_INFO, "ERROR: Metrics[%d]: invalid METRIC_ID", id);
        return NULL;
    }
    
    switch ( s_metrics[id].type )
    {
        case METRIC_UINT64:
            return &s_metrics[id].value.m_uint64;
            break;
        case METRIC_INT64:
            return &s_metrics[id].value.m_int64;
            break;
        case METRIC_DOUBLE:
            return &s_metrics[id].value.m_double;
            break;
        default:
            return NULL;
    }
}



void
Metrics::Set( METRIC_ID id, void* pValue )
{
    if (id >= MAX_METRIC_ID)
    {
        RETAILMSG(ZONE_INFO, "ERROR: Metrics[%d]: invalid METRIC_ID", id);
        return;
    }
    
    if (!pValue)
    {
        RETAILMSG(ZONE_INFO, "ERROR: Metrics[%d]: NULL pValue", id);
        return;
    }
    
    
    Metric* pMetric = &s_metrics[ id ];
    DEBUGCHK(pMetric);
    switch (pMetric->type)
    {
        case METRIC_UINT64:
            pMetric->value.m_uint64  = *(UINT64*)pValue;
            break;
        case METRIC_INT64:
            pMetric->value.m_int64   = *(INT64*)pValue;
            break;
        case METRIC_DOUBLE:
            pMetric->value.m_double  = *(double*)pValue;
            break;
    }
}


} // END namespace Z

    