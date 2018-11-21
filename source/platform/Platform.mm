/*
 *  Platform.cpp
 *  OpenGL
 *
 *  Created by Sean Kelleher on 9/4/10.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include "Platform.hpp"
#include "Log.hpp"

// For NSArray, NSString, etc.
#import <Foundation/Foundation.h>

// For CGRect/UIScreen
#import <UIKit/UIKit.h>

// For ::GetAvailableRam()
#include <mach/mach.h>
#include <mach/mach_host.h>

// For ::GetTickCount()
#include <mach/mach_time.h>

// For ::IsDebuggerAttached()
// For ::Is<Device>()
// For sysctlbyname() / sysctl()
#include <sys/types.h>
#include <sys/sysctl.h>


// For GlobalSettings.GetInt("/Settings.bUseOpenGLES1")
#include "Settings.hpp"

// For AudioServicesPlaySystemSound  [vibration on iOS]
#import <AudioToolbox/AudioToolbox.h>


// TODO: absolute need to move this to the Metrics system.
// For Localytics
#import "LocalyticsSession.h"


namespace Z
{


const char*
Platform::GetBuildInfo()
{
    NSString* buildInfo = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"BuildNumber"];
    
    return [buildInfo UTF8String];
}



const char* 
Platform::GetDeviceType()
{
    UIDevice* device = [UIDevice currentDevice];

    return [device.model UTF8String];
}



const char* 
Platform::GetDeviceUDID()
{
    NSUUID* uuid = [[UIDevice currentDevice] identifierForVendor];
    
    return [[uuid UUIDString] UTF8String];
}


    
const char*
Platform::GetOSName()
{
    UIDevice* device = [UIDevice currentDevice];

    return [device.systemName UTF8String];
}



const char*
Platform::GetOSVersion()
{
    UIDevice* device = [UIDevice currentDevice];

    return [device.systemVersion UTF8String];
}



RESULT
Platform::GetPathToPersistantStorage( INOUT string *pPathname )
{
    RESULT rval = S_OK;
    
    if (!pPathname)
    {
        DEBUGMSG(ZONE_ERROR, "ERROR: Platform::GetPathToPersistantStorage(): NULL pointer");
        rval = E_NULL_POINTER;
        goto Exit;
    }
    else 
    {
        //
        // Get the path to this application's Documents folder
        //
        NSArray  *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *documentsDirectory = [paths objectAtIndex:0];
    
        // Copy the NSString into a std::string.
        // The NSString will be released when the method returns.
        pPathname->assign( [documentsDirectory UTF8String] );
    }
    
Exit:    
    return rval;
}



RESULT
Platform::GetPathToApplicationFolder( INOUT string *pPathname )
{
    RESULT rval = S_OK;
    
    if (!pPathname)
    {
        DEBUGMSG(ZONE_ERROR, "ERROR: Platform::GetPathToApplicationFolder(): NULL pointer");
        rval = E_NULL_POINTER;
        goto Exit;
    }
    else 
    {
        pPathname->assign( "" );
    }
    
Exit:    
    return rval;
}



RESULT
Platform::GetPathForResource( IN const string& resourceFilename, OUT string* pPathToResource )
{
    RESULT rval = S_OK;

    if ( "" == resourceFilename)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Platform::GetPathForResource(): empty string");
        rval = E_INVALID_ARG;
        goto Exit;
    }

    if (!pPathToResource)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Platform::GetPathForResource(): NULL pointer");
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    
    {
        char    resourceFilenameCopy[MAX_PATH];
        strcpy( &resourceFilenameCopy[0], resourceFilename.c_str() );
        
        // Split the filename into its path/file components by the hack
        // of inserting a '#' character there.
        NSString *directory;
        NSString *filename;
        NSString *extension;
        
        char* pEndOfPath = strrchr(resourceFilenameCopy, '/');
        if (pEndOfPath)
        {
            *pEndOfPath = '#';
            directory   = [NSString stringWithCString: strtok(resourceFilenameCopy, "#")  encoding:NSUTF8StringEncoding];
            filename    = [NSString stringWithCString: strtok(NULL, ".")  encoding:NSUTF8StringEncoding];
            extension   = [NSString stringWithCString: strtok(NULL, ".")  encoding:NSUTF8StringEncoding];
        }
        else 
        {
            directory   = nil;
            filename    = [NSString stringWithCString: strtok(resourceFilenameCopy, ".")  encoding:NSUTF8StringEncoding];
            extension   = [NSString stringWithCString: strtok(NULL, ".")  encoding:NSUTF8StringEncoding];
        }

        
        NSString* path          = [[NSBundle mainBundle] pathForResource: filename ofType: extension inDirectory: directory];

        if (path)
        {
            *pPathToResource = [path UTF8String];
        }
        else 
        {
            *pPathToResource = "";
            rval = E_FILE_NOT_FOUND;
        }
    }
    
Exit:
    
    return rval;
}



UINT64
Platform::GetTickCount()
{
    mach_timebase_info_data_t   m_machTimebaseInfo;
    mach_timebase_info( &m_machTimebaseInfo );

    uint64_t time = mach_absolute_time();
    
    UINT64 milliseconds = time * (double)(((double)m_machTimebaseInfo.numer/(double)m_machTimebaseInfo.denom) / 1000000.0);
    
    return milliseconds;
}



RESULT
Platform::Sleep( UINT32 milliseconds )
{
    if (!milliseconds)
        return S_OK;
    

    timespec sleeptime = {0,0};
        
    time_t seconds      = milliseconds / 1000;
    sleeptime.tv_sec    = seconds;
    sleeptime.tv_nsec   = (milliseconds - (seconds * 1000)) * 1000000L;
    
    if (0 != nanosleep( &sleeptime, NULL ))
    {
        return E_FAIL;
    }
    
    return S_OK;
}



UINT32
Platform::Random()
{
    // arc4random self-initializes, so no need to seed it.
    return arc4random();
}


UINT32
Platform::Random( UINT32 min, UINT32 max )
{
    return min + arc4random_uniform(max+1);
}


double
Platform::RandomDouble()
{
    // arc4random self-initializes, so no need to seed it.
    return (double) (double(arc4random()) / double(RAND_MAX));
}


double
Platform::RandomDouble( double min, double max )
{
    UINT32 range = MAX(1, (UINT32)(max*100.0 - min*100.0));
    double rval  = ((double)(Platform::Random() % range))/100.0;
    
    return rval + min;
}



RESULT
Platform::Vibrate( UINT32 milliseconds )
{
    // No control over duration in iOS. :-(  LAME.
	AudioServicesPlaySystemSound( kSystemSoundID_Vibrate );
    
    return S_OK;
}


UINT32
Platform::GetProcessUsedMemory( )
{
    task_basic_info         info;
    kern_return_t           rval = 0;
    mach_port_t             task = mach_task_self();
    mach_msg_type_number_t  tcnt = TASK_BASIC_INFO_COUNT;
    task_info_t             tptr = (task_info_t) &info;

    memset(&info, 0, sizeof(info));

    rval = task_info(task, TASK_BASIC_INFO, tptr, &tcnt);
    if ( rval != KERN_SUCCESS )
    {
        return 0;
    }
    
    return info.resident_size;
}


UINT32
Platform::GetUsedMemory( )
{
    mach_port_t host_port;
    mach_msg_type_number_t host_size;
    vm_size_t pagesize;
        
    host_port = mach_host_self();
    host_size = sizeof(vm_statistics_data_t) / sizeof(integer_t);
    host_page_size(host_port, &pagesize);        
        
    vm_statistics_data_t vm_stat;
        
    if (host_statistics(host_port, HOST_VM_INFO, (host_info_t)&vm_stat, &host_size) != KERN_SUCCESS)
    {
        RETAILMSG(ZONE_ERROR, "Platform::GetAvailableMemory(): failed to fetch vm statistics");
        return 0;
    }
    
    /* Stats in bytes */ 
    natural_t mem_used = (vm_stat.active_count +
                          vm_stat.inactive_count +
                          vm_stat.wire_count) * pagesize;
    
    return mem_used;
}


UINT32
Platform::GetAvailableMemory( )
{
    mach_port_t host_port;
    mach_msg_type_number_t host_size;
    vm_size_t pagesize;
        
    host_port = mach_host_self();
    host_size = sizeof(vm_statistics_data_t) / sizeof(integer_t);
    host_page_size(host_port, &pagesize);        
        
    vm_statistics_data_t vm_stat;
        
    if (host_statistics(host_port, HOST_VM_INFO, (host_info_t)&vm_stat, &host_size) != KERN_SUCCESS)
    {
        RETAILMSG(ZONE_ERROR, "Platform::GetAvailableMemory(): failed to fetch vm statistics");
        return 0;
    }
    
    /* Stats in bytes */ 
    natural_t mem_free = vm_stat.free_count * pagesize;

    
    return mem_free;
}



RESULT
Platform::GetScreenRectPoints( INOUT Rectangle* pScreenRect )
{
    if (!pScreenRect)
    {
        return E_NULL_POINTER;
    }

    // Cache these values instead of calling the OS every time.
    static Rectangle screenRect = { 0 };
    
    if ( !screenRect.width || !screenRect.height )
    {
        // Get the Device's screen size.
        CGRect frame = [[UIScreen mainScreen] bounds];
        screenRect.x      = 0;
        screenRect.y      = 0;
        screenRect.width  = CGRectGetWidth(frame);
        screenRect.height = CGRectGetHeight(frame);


        DEBUGMSG(ZONE_INFO, "Platform::GetScreenRectPoints(): %2.2f x %2.2f", screenRect.width, screenRect.height);
    }

    *pScreenRect = screenRect;
        
Exit:
    return S_OK;
}



RESULT
Platform::GetScreenRect( INOUT Rectangle* pScreenRect )
{
    if (!pScreenRect)
    {
        return E_NULL_POINTER;
    }

    // Cache these values instead of calling the OS every time.
    static Rectangle screenRect = { 0 };
    
    if ( !screenRect.width || !screenRect.height )
    {
        
        float scale = 1.0f;
        if (!Platform::IsIPhone3G())    // TODO: should be an iOS version check instead?
        {
            scale = [[UIScreen mainScreen] scale];
        }
        
        // Get the Device's screen size.
        CGRect frame = [[UIScreen mainScreen] bounds];
        screenRect.x      = 0;
        screenRect.y      = 0;
        screenRect.width  = CGRectGetWidth(frame)     * scale;
        screenRect.height = CGRectGetHeight(frame)    * scale;


        DEBUGMSG(ZONE_INFO, "Platform::GetScreenRect(): %2.2f x %2.2f", screenRect.width, screenRect.height);
    }

    *pScreenRect = screenRect;
        
Exit:
    return S_OK;
}



RESULT
Platform::GetScreenRectCamera( INOUT Rectangle* pScreenRect )
{
    if (!pScreenRect)
    {
        return E_NULL_POINTER;
    }
    
    // Cache these values instead of calling the OS every time.
    static Rectangle screenRectScaled = { 0 };
    
    if ( !screenRectScaled.width || !screenRectScaled.height )
    {
        GetScreenRect( &screenRectScaled );
        
        float worldScale = GlobalSettings.GetFloat("/Settings.fWorldScaleFactor", 1.0f);
        
        screenRectScaled.x        /= worldScale;
        screenRectScaled.y        /= worldScale;
        screenRectScaled.width    /= worldScale;
        screenRectScaled.height   /= worldScale;
        
        DEBUGMSG(ZONE_INFO, "Platform::GetScreenRectCamera(): %2.2f x %2.2f", screenRectScaled.width, screenRectScaled.height);
    }
    
    *pScreenRect = screenRectScaled;
    
Exit:
    return S_OK;
}



float
Platform::GetScreenScaleFactor()
{
        if([[UIScreen mainScreen] respondsToSelector: NSSelectorFromString(@"scale")])
        {
            return [[UIScreen mainScreen] scale];
        }
        else 
        {
            return 1.0f;
        }
}



bool
Platform::IsDebuggerAttached()
{
    // Returns true if the current process is being debugged (either
    // running under the debugger or has a debugger attached post facto).
    int                 mib[4];
    struct kinfo_proc   info;
    size_t              size;

    // Initialize the flags so that, if sysctl fails for some bizarre
    // reason, we get a predictable result.
    info.kp_proc.p_flag = 0;

    // Initialize mib, which tells sysctl the info we want, in this case
    // we're looking for information about a specific process ID.
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();

    // Call sysctl.
    size = sizeof(info);
    sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);

    // We're being debugged if the P_TRACED flag is set.
    return ( (info.kp_proc.p_flag & P_TRACED) != 0 );
}



bool
Platform::IsDevice()
{
    bool rval;
    
#if TARGET_IPHONE_SIMULATOR
    rval = false;
#else
    rval = true;
#endif
    
    return rval;
}



bool
Platform::IsSimulator()
{
    bool rval;
    
#if TARGET_IPHONE_SIMULATOR
    rval = true;
#else
    rval = false;
#endif
    
    return rval;
}



bool
Platform::IsIPhone3G()
{
    size_t size = MAX_PATH;
    char  devicetype[MAX_PATH];
    sysctlbyname("hw.machine", devicetype, &size, NULL, 0);
    
    if (!strcmp(devicetype, "iPhone1,2"))
    {
        return true;
    }
    else 
    {
        return false;
    }
}



bool
Platform::IsIPhone3GS()
{
    size_t size = MAX_PATH;
    char  devicetype[MAX_PATH];
    sysctlbyname("hw.machine", devicetype, &size, NULL, 0);
    
    if (!strcmp(devicetype, "iPhone2,1"))
    {
        return true;
    }
    else 
    {
        return false;
    }
}



bool
Platform::IsIPhone4()
{
    size_t size = MAX_PATH;
    char  devicetype[MAX_PATH];
    sysctlbyname("hw.machine", devicetype, &size, NULL, 0);
    
    if (!strcmp(devicetype, "iPhone3,1"))
    {
        return true;
    }
    else 
    {
        return false;
    }
}



bool
Platform::IsIPad()
{
    size_t size = MAX_PATH;
    char  devicetype[MAX_PATH];
    sysctlbyname("hw.machine", devicetype, &size, NULL, 0);
    
    if (!strcmp(devicetype, "iPad1,1") ||   // iPad WiFi
        !strcmp(devicetype, "iPad1,2"))     // iPad 3G
    {
        return true;
    }
    else 
    {
        return false;
    }
}



bool
Platform::IsOpenGLES1()
{
    static bool s_IsOpenGLES1 = false;
    static bool s_haveChecked = false;
    
    if (!s_haveChecked)
    {
        s_IsOpenGLES1 = GlobalSettings.GetInt("/Settings.bUseOpenGLES1");
        s_haveChecked = true;
    }

    return s_IsOpenGLES1;
}



bool
Platform::IsOpenGLES2()
{
    static bool s_IsOpenGLES2 = false;
    static bool s_haveChecked = false;
    
    if (!s_haveChecked)
    {
        s_IsOpenGLES2 = !GlobalSettings.GetInt("/Settings.bUseOpenGLES1");
        s_haveChecked = true;
    }
    
    return s_IsOpenGLES2;
}
    


void
Platform::LogAnalyticsEvent( IN const string& event /* TODO: dictionary of attributes */ )
{
    if (0 == event.length())
    {
        return;
    }
    
    NSString* string = [NSString stringWithCString:event.c_str() encoding:NSUTF8StringEncoding];
    [[LocalyticsSession sharedLocalyticsSession] tagEvent:string];
}



bool
Platform::IsWidescreen()
{
    return fabs( (double)[[UIScreen mainScreen] bounds].size.height - (double)568 ) < DBL_EPSILON;
}


} // END namespace Z


