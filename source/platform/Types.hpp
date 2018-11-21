#pragma once

#include <stdint.h>
#include <stddef.h>
#include <float.h>
#include <algorithm>
#include <assert.h>
#include <list>
#include <limits>
#include "Errors.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"


#ifndef UINT_MAX
#define UINT_MAX std::numeric_limits<unsigned int>::max()
#endif

namespace Z
{

#ifndef MIN
#define MIN(x, y)   ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x, y)   ((x) > (y) ? (x) : (y))
#endif


typedef struct
{
    float   x;
    float   y;
} Point2D;



typedef struct
{
    float   x;
    float   y;
    float   z;
} Point3D;



typedef struct
{
    float   x;
    float   y;
    float   width;
    float   height;
} Rectangle;



// Axis-Aligned Bounding Box
class AABB
{
public:
    AABB()                        { m_minPoint = m_maxPoint = vec3(0,0,0); }
    AABB( vec3  min, vec3  max )  { m_minPoint = min; m_maxPoint = max; }
    virtual ~AABB()               {}
    
    inline void          SetMin     ( const vec3& min ) { m_minPoint = min; }
    inline void          SetMax     ( const vec3& max ) { m_maxPoint = max; };
    
    inline const float   GetWidth   ()  const   { return m_maxPoint.x - m_minPoint.x; }
    inline const float   GetHeight  ()  const   { return m_maxPoint.y - m_minPoint.y; }
    inline const float   GetDepth   ()  const   { return m_maxPoint.z - m_minPoint.z; }
    inline const vec3    GetMin     ()  const   { return m_minPoint; }
    inline const vec3    GetMax     ()  const   { return m_maxPoint; }
    inline const vec3    GetCenter  ()  const   { vec3 c;  
                                                  c.x = m_minPoint.x + GetWidth()/2.0f; 
                                                  c.y = m_minPoint.y + GetHeight()/2.0f; 
                                                  c.z = m_minPoint.z + GetDepth()/2.0f; 
                                                  return c; 
                                                }
    inline const float   GetRadius  ()  const   { return MAX(GetWidth(), GetHeight())/2.0f; }
    
    
    void  Update ( const mat4& orientationMatrix )
    {
        // Multiply the min and max points by the orientation matrix
        vec4 newMin;
        vec4 newMax;
        
        newMin = orientationMatrix * vec4(m_minPoint.x, m_minPoint.y, m_minPoint.z, 1.0); 
        newMax = orientationMatrix * vec4(m_maxPoint.x, m_maxPoint.y, m_maxPoint.z, 1.0); 
        
        SetMin( vec3(newMin.x, newMin.y, newMin.z) );
        SetMax( vec3(newMax.x, newMax.y, newMax.z) );
    }
    
    
    bool Intersects ( const Point3D point ) const
    {
        if (point.x < m_minPoint.x || 
            point.y < m_minPoint.y ||
            point.z < m_minPoint.z ||
            point.x > m_maxPoint.x ||
            point.y > m_maxPoint.y ||
            point.z > m_maxPoint.z)
        {
            return false;
        }
        
        return true;
    }


    bool Intersects ( const Point2D point ) const
    {
        if (point.x < m_minPoint.x || 
            point.y < m_minPoint.y ||
            point.x > m_maxPoint.x ||
            point.y > m_maxPoint.y)
        {
            return false;
        }
        
        return true;
    }

    
    bool Intersects ( const AABB bounds ) const
    {
        vec3 boundsMin = bounds.GetMin();
        vec3 boundsMax = bounds.GetMax();
    
        // See Real-Time Collision Detection p. 79
        if (m_maxPoint.x < boundsMin.x || m_minPoint.x > boundsMax.x) return false;
        if (m_maxPoint.y < boundsMin.y || m_minPoint.y > boundsMax.y) return false;
        if (m_maxPoint.z < boundsMin.z || m_minPoint.z > boundsMax.z) return false;

        return true;
    }
    
    
    void operator*=(float f)
    {
        if (f <= 0.0f)
            return;

        float halfWidth  = GetWidth()/2.0f;
        float halfHeight = GetHeight()/2.0f;

        vec3 center = GetCenter();
        
        m_minPoint.x = center.x - (halfWidth  * f);
        m_minPoint.y = center.y - (halfHeight * f);
        m_maxPoint.x = center.x + (halfWidth  * f);
        m_maxPoint.y = center.y + (halfHeight * f);
    }

    
protected:
    vec3 m_minPoint;
    vec3 m_maxPoint;
};



typedef vec3  WORLD_POSITION;
typedef ivec2 MAP_POSITION;
typedef std::list<WORLD_POSITION>   WorldPositionList;
typedef std::list<MAP_POSITION>     MapPositionList;
typedef WorldPositionList::iterator WorldPositionListIterator;
typedef MapPositionList::iterator   MapPositionListIterator;


#ifdef __APPLE__
//#if defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
    typedef uint64_t        UINT64;
    typedef unsigned long   UINT32;
    typedef unsigned short  UINT16;
    typedef unsigned char   UINT8;

    typedef int64_t         INT64;
    typedef long            INT32;
    typedef short           INT16;
    typedef char            INT8;

    typedef unsigned char   BYTE;
    typedef unsigned short  WCHAR;

    // INT32 instead of UINT32 so we can use OSAtomicIncrement32().
    typedef INT32           OBJECT_ID;
#else
    #error Compiling for unknown platform in Types.hpp
#endif // __APPLE__


const int MAX_PATH = 2048;
const int MAX_NAME = 64;

//
// Function parameter hints, e.g.
// func(IN param1, IN param2, OUT param3);
//
#define IN
#define OUT
#define INOUT


#define BIT0                            0x00000001
#define BIT1                            0x00000002
#define BIT2                            0x00000004
#define BIT3                            0x00000008
#define BIT4                            0x00000010
#define BIT5                            0x00000020
#define BIT6                            0x00000040
#define BIT7                            0x00000080
#define BIT8                            0x00000100
#define BIT9                            0x00000200
#define BIT10                           0x00000400
#define BIT11                           0x00000800
#define BIT12                           0x00001000
#define BIT13                           0x00002000
#define BIT14                           0x00004000
#define BIT15                           0x00008000
#define BIT16                           0x00010000
#define BIT17                           0x00020000
#define BIT18                           0x00040000
#define BIT19                           0x00080000
#define BIT20                           0x00100000
#define BIT21                           0x00200000
#define BIT22                           0x00400000
#define BIT23                           0x00800000
#define BIT24                           0x01000000
#define BIT25                           0x02000000
#define BIT26                           0x04000000
#define BIT27                           0x08000000
#define BIT28                           0x10000000
#define BIT29                           0x20000000
#define BIT30                           0x40000000
#define BIT31                           0x80000000


} // END namespace Z


