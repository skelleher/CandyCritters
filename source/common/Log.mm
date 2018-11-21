/*
 *  Log.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 9/4/10.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#import <Foundation/Foundation.h>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include "Log.hpp"
#include "Errors.hpp"
#include "Macros.hpp"
#include "Platform.hpp" // for ::GetTickCount()
#include "msg.hpp"      // for MSG_Object


namespace Z
{

// Keep N copies of the log file so we can investigate crashes
const int NUM_LOGS_TO_BACKUP = 3;


//
// Static Data
//
bool         Log::s_initialized     = false;
FILE*        Log::s_pFile           = NULL;
std::string  Log::s_filename("");
ZONE_MASK    Log::s_zoneMask        = ZONE_ERROR | ZONE_WARN | ZONE_INFO;

ZONE_MAPPING Log::ZoneMapping[] = 
{
    { "ZONE_ERROR",         ZONE_ERROR          },
    { "ZONE_WARN",          ZONE_WARN           },
    { "ZONE_INFO",          ZONE_INFO           },
    { "ZONE_SETTINGS",      ZONE_SETTINGS       },
    { "ZONE_HANDLE",        ZONE_HANDLE         },
    { "ZONE_RESOURCE",      ZONE_RESOURCE       },
    { "ZONE_RENDER",        ZONE_RENDER         },
    { "ZONE_OBJECT",        ZONE_OBJECT         },
    { "ZONE_FILE",          ZONE_FILE           },
    { "ZONE_TEXTURE",       ZONE_TEXTURE        },
    { "ZONE_SHADER",        ZONE_SHADER         },
    { "ZONE_SPRITE",        ZONE_SPRITE         },
    { "ZONE_MESH",          ZONE_MESH           },
    { "ZONE_ANIMATION",     ZONE_ANIMATION      },
    { "ZONE_STORYBOARD",    ZONE_STORYBOARD     },
    { "ZONE_GAMEOBJECT",    ZONE_GAMEOBJECT     },
    { "ZONE_MESSAGE",       ZONE_MESSAGE        },
    { "ZONE_STATEMACHINE",  ZONE_STATEMACHINE   },
    { "ZONE_LAYER",         ZONE_LAYER          },
    { "ZONE_PATHFINDER",    ZONE_PATHFINDER     },
    { "ZONE_TOUCH",         ZONE_TOUCH          },
    { "ZONE_MAP",           ZONE_MAP            },
    { "ZONE_COLLISION",     ZONE_COLLISION      },
    { "ZONE_ACCELEROMETER", ZONE_ACCELEROMETER  },
    { "ZONE_FONT",          ZONE_FONT           },
    { "ZONE_SOUND",         ZONE_SOUND          },
    { "ZONE_NETWORK",       ZONE_NETWORK        },
    { "ZONE_PARTICLES",     ZONE_PARTICLES      },
    
    { "ZONE_ANALYTICS",     ZONE_ANALYTICS      },
    { "ZONE_PERF",          ZONE_PERF           },
    { "ZONE_VERBOSE",       ZONE_VERBOSE        },
    0,
};



RESULT
Log::OpenWithFilename( IN const string& filename )
{
    RESULT rval = S_OK;
//    string path;
//    Platform::GetPathToPersistantStorage(&path);
//    path = path + filename;
    
    if (filename.length() == 0)
    {
        Print(ZONE_ERROR, "Log:openWithFilename: empty filename");
        return E_FILE_NOT_FOUND;
    }

    if (s_initialized)
    {
        Print(ZONE_WARN, "WARN: Log::Open() called more than once; ignoring.");
        CBR(E_ACCESS_DENIED);
    }

    
    BackupFileIfItExists( filename );
    
//    if (![ExoUtilities isDebuggerAttached])
//    {
//        [self redirectNSLogToFile:path];
//    }
    
    rval = SetFilename( filename );

    s_initialized = true;

Exit:
    return rval;
}



void
Log::BackupFileIfItExists( IN const string& logfilename )
{
    NSFileManager* fileManager = [NSFileManager defaultManager];
    NSError* error;
    
    
    NSString* filename  = [NSString stringWithUTF8String:logfilename.c_str()];
    NSString* basename  = [filename stringByDeletingPathExtension];
    NSString* extension = [filename pathExtension];
    
    // Rotate any existing files, deleting the oldest
    for (int i = NUM_LOGS_TO_BACKUP-1; i >= 1; --i)
    {
        NSString* curFilename  = [NSString stringWithFormat:@"%@_%d.%@", basename, i,   extension];
        NSString* nextFilename = [NSString stringWithFormat:@"%@_%d.%@", basename, i+1, extension];
        
        if ([fileManager fileExistsAtPath:curFilename])
        {
            [fileManager removeItemAtPath:nextFilename error:&error];
            [fileManager moveItemAtPath:curFilename toPath:nextFilename error:&error];
        }
    }
    
    NSString* backupFilename = [NSString stringWithFormat:@"%@_1.%@", basename, extension];
    [fileManager removeItemAtPath:backupFilename error:&error];
    [fileManager moveItemAtPath:filename toPath:backupFilename error:&error];
}



RESULT
Log::Close()
{
    RESULT rval = S_OK;
    
    {
        char timestring[128];
        time_t result = time(NULL);
        strftime(timestring, sizeof(timestring), "%c", localtime(&result));
        
        Print(ZONE_INFO, "Log Closed %s\n", timestring);
    }
    
    fflush(s_pFile);
    fclose(s_pFile);

    s_pFile = NULL;
    
    return rval;
}



RESULT 
Log::SetFilename( IN const string& logfilename )
{
    RESULT  rval    = S_OK;
    
    if (0 == logfilename.size() || logfilename == "")
    {
        CHR(E_INVALID_ARG);
    }
    
    
    if ( s_filename.length() != 0 )
    {
        Print(ZONE_INFO, "Log::SetFile(%s): closing previous log [%s]\n", logfilename.c_str(), s_filename.c_str());
        fflush(s_pFile);
        fclose(s_pFile);
    }
    
    s_pFile = fopen( logfilename.c_str(), "w" );

    if (NULL == s_pFile)
    {
        DEBUGCHK(0);
    }
    else 
    {
        s_filename = logfilename;
    }

      
    {
        char timestring[128];
        time_t result = time(NULL);
        strftime(timestring, sizeof(timestring), "%c", localtime(&result));
    
        Print(ZONE_INFO, "Log [%s]",     logfilename.c_str());
        Print(ZONE_INFO, "Created %s\n", timestring);
    }

Exit:    
    return rval;
}



const std::string&
Log::GetFilename( )
{
    return s_filename;
}



void
Log::Print( ZONE_MASK zone, IN const std::string* format, ... )
{
    assert(format);
    if (zone == (s_zoneMask & zone))
    {
        char buf[1024]; 
        va_list vargs;
        va_start(vargs, format);
        vsprintf(buf, format->c_str(), vargs);
        
        if (s_pFile)
        {
            // For ZONE_INFO and ZONE_ERROR, print a timestamp.
            if ( zone & (ZONE_INFO | ZONE_ERROR) )
            {
                char timestring[128];
                time_t result = time(NULL);
                strftime(timestring, sizeof(timestring), "%c", localtime(&result));
                
                fprintf( s_pFile, "[%s] %s\n", timestring, buf ); 
                printf("[%s] %s\n", timestring, buf);
            }
            else 
            {
                fprintf( s_pFile, "%s\n", buf ); 
                printf("%s\n", buf);
            }
            
            fflush ( s_pFile );
        }
        
        va_end(vargs);
    }
}



void
Log::Print( ZONE_MASK zone, IN const char *format, ... )
{
    if (zone == (s_zoneMask & zone))
    {
        char buf[1024]; 
        va_list vargs;
        va_start(vargs, format);
        vsprintf(buf, format, vargs); 

        if (s_pFile)
        {
            /*
            // For ZONE_INFO and ZONE_ERROR, print a timestamp.
            if ( zone & (ZONE_INFO | ZONE_ERROR) )
            {
                char timestring[128];
                time_t result = time(NULL);
                strftime(timestring, sizeof(timestring), "%c", localtime(&result));
                
                fprintf( s_pFile, "[%s] %s\n", timestring, buf ); 
                printf("[%s] %s\n", timestring, buf);
            }
            else 
            {
                fprintf( s_pFile, "%s\n", buf ); 
                printf("%s\n", buf);
            }
            */
            
            UINT64 tickcount    = Platform::GetTickCount();
            UINT64 hours        = tickcount / (1000*60*60);
            UINT64 minutes      = tickcount % (1000*60*60) / (1000*60);
            UINT64 seconds      = tickcount % (1000*60)    / (1000);
            UINT64 milliseconds = tickcount % (1000);
            
   
//            fprintf( s_pFile, "[%lld] %s\n", tickcount, buf ); 
//            printf("[%lld] %s\n", tickcount, buf);
            fprintf( s_pFile, "[%lld:%02lld:%02lld:%03lld] %s\n", hours, minutes, seconds, milliseconds, buf ); 
            printf("[%lld:%02lld:%02lld:%03lld] %s\n", hours, minutes, seconds, milliseconds, buf);

            fflush ( s_pFile );
        }
        
        va_end(vargs);
    }
}



void
Log::LogStateMachineEvent( OBJECT_ID id, const string& name, MSG_Object* msg, const string& statename, const string& substatename, const string& eventmsgname, bool handled )
{
    RETAILMSG(ZONE_STATEMACHINE, "Event id:%d name:\"%s\" msg:\"%s\" state:\"%s\", substate:\"%s\", eventmsg:\"%s\", handle:%d", 
              (UINT32)id, name.c_str(), msg->GetName(), statename.c_str(), substatename.c_str(), eventmsgname.c_str(), handled);
}



void
Log::LogStateMachineStateChange( OBJECT_ID id, const string& name, unsigned int state, int substate )
{
    RETAILMSG(ZONE_STATEMACHINE, "State id:%d name:\"%s\" state:%d substate:%d",
              (UINT32)id, name.c_str(), state, substate);
}



void
Log::SetZoneMask( ZONE_MASK zoneMask )
{
    RETAILMSG(ZONE_VERBOSE, "Log::SetZoneMask( 0x%x )", zoneMask);
    s_zoneMask = zoneMask;
}



ZONE_MASK
Log::GetZoneMask()
{
    return s_zoneMask;
}



bool
Log::IsZoneEnabled( ZONE_MASK zone )
{
    return (s_zoneMask & zone) == zone ? true : false;
}




void
Log::EnableZone( ZONE_MASK zone )
{
    s_zoneMask |= zone;
}



void
Log::DisableZone( UINT32 zone )
{
    s_zoneMask &= ~zone;
}



} // END namespace Z

