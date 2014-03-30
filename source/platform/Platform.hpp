#pragma once

#include "Types.hpp"
#include "Errors.hpp"
#include <string>

using std::string;
    
namespace Z
{



class Platform
{
public:
    static  const char* GetBuildInfo                ( );

    static  const char* GetDeviceType               ( );
    static  const char* GetDeviceUDID               ( );
    static  const char* GetOSName                   ( );
    static  const char* GetOSVersion                ( );
    
    static  RESULT      GetPathToPersistantStorage  ( INOUT string* pPathname );
    static  RESULT      GetPathToApplicationFolder  ( INOUT string* pPathname );
    static  RESULT      GetPathForResource          ( IN    const string& pResourceFilename, INOUT string* pPathToResource );
    
    static  UINT32      GetAvailableMemory          ( );
    static  UINT32      GetUsedMemory               ( );
    static  UINT32      GetProcessUsedMemory        ( );
    
    static  RESULT      GetScreenRect               ( INOUT Rectangle* pScreenRect );
    static  RESULT      GetScreenRectPoints         ( INOUT Rectangle* pScreenRect );
    static  RESULT      GetScreenRectCamera         ( INOUT Rectangle* pScreenRect );   // TODO: move this to Camera.
    static  float       GetScreenScaleFactor        ( );
 
    static  UINT64      GetTickCount                ( );
    static  RESULT      Sleep                       ( IN UINT32 milliseconds );

    static  UINT32      Random                      ( );
    static  UINT32      Random                      ( IN UINT32 min, IN UINT32 max );
    static  double      RandomDouble                ( );
    static  double      RandomDouble                ( IN double min, IN double max );
    static  bool        CoinToss                    ( ) { return Platform::Random(0,1) ? true : false; }
    
    static  RESULT      Vibrate                     ( IN UINT32 milliseconds = 100 );
    
    static  bool        IsDebuggerAttached          ( );
    static  bool        IsDevice                    ( );
    static  bool        IsSimulator                 ( );
    static  bool        IsIPhone3G                  ( );
    static  bool        IsIPhone3GS                 ( );
    static  bool        IsIPhone4                   ( );
    static  bool        IsIPad                      ( );

    static  bool        IsOpenGLES1                 ( );
    static  bool        IsOpenGLES2                 ( );
    
    static  bool        IsWidescreen                ( );
    
    static  void        LogAnalyticsEvent           ( IN const string& event  /* TODO: dictionary of attributes */ );
    
protected:
    //
    // Singleton
    //
    Platform();
    Platform(const Platform& rhs);
    Platform& operator=(const Platform& rhs);
    virtual ~Platform();
    
};



} // END namespace Z
