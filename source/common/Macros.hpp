#pragma once

#include <libkern/OSAtomic.h>
#include <assert.h>
#include <TargetConditionals.h>
#include "Log.hpp"
#include "Platform.hpp"

namespace Z
{


    
#define ARRAY_SIZE(a)           ( sizeof((a)) / sizeof((a)[0]) )


#define SAFE_DELETE(x)          { if ((x) != NULL) { delete (x); (x) = NULL; } }

#define SAFE_ARRAY_DELETE(x)    { if ((x) != NULL) { delete[] (x); (x) = NULL; } }

#define SAFE_FREE(x)            { if ((x) != NULL) { free (x); (x) = NULL; } }

#define SAFE_RELEASE(x)         { if ((x) != NULL) { (x)->Release(); (x) = NULL; } }

#define SAFE_RELEASE_NO_NULL(x) { if ((x) != NULL) { (x)->Release(); } }

//#define SAFE_ADDREF(x)          { if (x != NULL) { (x)->AddRef(); } }

#define SAFE_ADDREF_TEMPLATE_TYPE(x)  \
{ \
    if (x) \
    { \
        IObject* pOBJ = dynamic_cast<IObject*>((x)); \
        if (pOBJ) \
        { \
            pOBJ->AddRef(); \
        } \
    } \
}


#define SAFE_RELEASE_TEMPLATE_TYPE(x)  \
{ \
    if (x) \
    { \
        IObject* pOBJ = dynamic_cast<IObject*>((x)); \
        if (pOBJ) \
        { \
            pOBJ->Release(); \
        } \
    } \
}



#ifndef MIN
#define MIN(x, y)   ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x, y)   ((x) > (y) ? (x) : (y))
#endif

#ifndef CLAMP
#define CLAMP(x, min, max)  (MIN(max, MAX(x, min)))
#endif


#ifndef PI
#define PI 3.1415926
#endif

#define RADIANS( x )    ((x) * (PI/180.0))
#define DEGREES( x )    ((x) * (180.0/PI))


#ifdef DEBUG
    #define VERIFYGL(x) \
    { \
        (x);  \
        GLenum error = glGetError(); \
        if (error != GL_NO_ERROR) \
        { \
            Z::Log::Print(Z::ZONE_ERROR, "error 0x%x [%s:%d]: ", error, __FILE__, __LINE__); \
            Z::Log::Print(Z::ZONE_ERROR, "[%s]", #x); \
            DEBUGCHK(0); \
            goto Exit; \
        } \
    }


    #define VERIFYEGL(x) \
    { \
        (x);  \
        GLenum error = eglGetError(); \
        if (error != EGL_NO_ERROR) \
        { \
            Z::Log::Print(Z::ZONE_ERROR, "error 0x%x [%s:%d]: ", error, __FILE__, __LINE__); \
            Z::Log::Print(Z::ZONE_ERROR, "[%s]", #x); \
            DEBUGCHK(0); \
            goto Exit; \
        } \
    }


    #define VERIFYAL(x) \
    { \
        (x);  \
        ALenum error = alGetError(); \
        if (error != AL_NO_ERROR) \
        { \
            Z::Log::Print(Z::ZONE_ERROR, "error 0x%x [%s:%d]: ", error, __FILE__, __LINE__); \
            Z::Log::Print(Z::ZONE_ERROR, "[%s]", #x); \
            DEBUGCHK(0); \
            goto Exit; \
        } \
    }


    #define IGNOREGL(x) \
    { \
        (x);  \
        GLenum error = glGetError(); \
        if (error != GL_NO_ERROR) \
        { \
            Z::Log::Print(Z::ZONE_ERROR, "error 0x%x [%s:%d]: ", error, __FILE__, __LINE__); \
            Z::Log::Print(Z::ZONE_ERROR, "[%s]", #x); \
        } \
    }


    #define IGNOREEGL(x) \
    { \
        (x);  \
        GLenum error = eglGetError(); \
        if (error != EGL_NO_ERROR) \
        { \
            Z::Log::Print(Z::ZONE_ERROR, "error 0x%x [%s:%d]: ", error, __FILE__, __LINE__); \
            Z::Log::Print(Z::ZONE_ERROR, "[%s]", #x); \
        } \
    }


    #define IGNOREAL(x) \
    { \
        (x);  \
        ALenum error = alGetError(); \
        if (error != AL_NO_ERROR) \
        { \
            Z::Log::Print(Z::ZONE_ERROR, "error 0x%x [%s:%d]: ", error, __FILE__, __LINE__); \
            Z::Log::Print(Z::ZONE_ERROR, "[%s]", #x); \
        } \
    }



#else
    #define VERIFYGL(x)     x
    #define VERIFYEGL(x)    x
    #define VERIFYAL(x)     x

    #define IGNOREGL(x)     x
    #define IGNOREEGL(x)    x
    #define IGNOREAL(x)     x
#endif



#define ASSERTMSG( x, string )  \
    { \
        if (!(x)) \
        { \
            Z::Log::Print(Z::ZONE_ERROR, (string)); \
            DEBUGCHK(x); \
        } \
    }



#if TARGET_CPU_ARM
    #define SOFTWARE_INTERRUPT(signal) \
        __asm__ __volatile__ ("mov r0, %0\nmov r1, %1\nmov.w r12, #37\nswi 128\n" : : "r" (getpid()), "r" (signal) : "r12", "r0", "r1", "cc")

//    #ifdef DEBUG        
    #ifndef SHIPBUILD
        #define DEBUG_BREAK() \
        { \
            do \
            { \
                Z::Log::Print(Z::ZONE_INFO, "- DEBUG_BREAK -"); \
                int trapSignal = Z::Platform::IsDebuggerAttached() ? SIGINT : SIGSTOP; \
                SOFTWARE_INTERRUPT(trapSignal); \
                if (trapSignal == SIGSTOP) \
                { \
                    SOFTWARE_INTERRUPT(SIGINT); \
                } \
            } \
            while (false); \
        }
    #else
        #define DEBUG_BREAK()
    #endif
   
#elif TARGET_CPU_X86
    #define DEBUG_BREAK() \
    { \
        Z::Log::Print(Z::ZONE_INFO, "- DEBUG_BREAK -"); \
        do \
        { \
            int trapSignal = Z::Platform::IsDebuggerAttached() ? SIGINT : SIGSTOP; \
            __asm__ __volatile__ ("pushl %0\npushl %1\npush $0\nmovl %2, %%eax\nint $0x80\nadd $12, %%esp" : : "g" (trapSignal), "g" (getpid()), "n" (37) : "eax", "cc"); \
        } while (false); \
    }

#else
    #define DEBUG_BREAK() assert(0)
#endif


#define DEBUGCHK(x)  \
{ \
    if (!(x)) \
    { \
        Z::Log::Print(Z::ZONE_ERROR, "DEBUGCHK [%s:%d]: ", __FILE__, __LINE__); \
        Z::Log::Print(Z::ZONE_ERROR, "[%s]", #x); \
        DEBUG_BREAK(); \
    } \
}


#ifdef __APPLE__
    #define ATOMIC_INCREMENT(x)         OSAtomicIncrement32( ((volatile int32_t*)&(x)) )
    #define ATOMIC_DECREMENT(x)         OSAtomicDecrement32( ((volatile int32_t*)&(x)) )
    #define ATOMIC_EXCHANGE(x,y)        ((x) = (y))
#elif

    #error You need to implement atomic operations for this platform.

    #define ATOMIC_INCREMENT(x)     (++(x))
    #define ATOMIC_DECREMENT(x)     (--(x))
    #define ATOMIC_EXCHANGE(x,y)    ((x) = (y))
#endif // __APPLE__


} // END namespace Z


