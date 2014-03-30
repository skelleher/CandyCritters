#pragma once

#include <stdio.h>
#include <string>
#include "Types.hpp"

using std::string;


namespace Z
{

// Forward declaration
class MSG_Object;



#define RETAILMSG(zone, format, ...)  \
    Z::Log::Print(zone, format, ##__VA_ARGS__);


#ifdef DEBUG
    #define DEBUGMSG(zone, format, ...)  \
        Z::Log::Print(zone, format, ##__VA_ARGS__);
#else
    #define DEBUGMSG(zone, format, ...) 
#endif



typedef UINT32 ZONE_MASK;

const ZONE_MASK ZONE_ERROR          = BIT0;
const ZONE_MASK ZONE_WARN           = BIT1;
const ZONE_MASK ZONE_INFO           = BIT2;
const ZONE_MASK ZONE_SETTINGS       = BIT3;
const ZONE_MASK ZONE_HANDLE         = BIT4;
const ZONE_MASK ZONE_RESOURCE       = BIT5;
const ZONE_MASK ZONE_RENDER         = BIT6;
const ZONE_MASK ZONE_OBJECT         = BIT7;
const ZONE_MASK ZONE_FILE           = BIT8;
const ZONE_MASK ZONE_TEXTURE        = BIT9;
const ZONE_MASK ZONE_SHADER         = BIT10;
const ZONE_MASK ZONE_SPRITE         = BIT11;
const ZONE_MASK ZONE_MESH           = BIT12;
const ZONE_MASK ZONE_ANIMATION      = BIT13;
const ZONE_MASK ZONE_STORYBOARD     = BIT14;
const ZONE_MASK ZONE_GAMEOBJECT     = BIT15;
const ZONE_MASK ZONE_MESSAGE        = BIT16;
const ZONE_MASK ZONE_STATEMACHINE   = BIT17;
const ZONE_MASK ZONE_LAYER          = BIT18;
const ZONE_MASK ZONE_PATHFINDER     = BIT19;
const ZONE_MASK ZONE_TOUCH          = BIT20;
const ZONE_MASK ZONE_MAP            = BIT21;
const ZONE_MASK ZONE_COLLISION      = BIT22;
const ZONE_MASK ZONE_ACCELEROMETER  = BIT23;
const ZONE_MASK ZONE_FONT           = BIT24;
const ZONE_MASK ZONE_SOUND          = BIT25;
const ZONE_MASK ZONE_NETWORK        = BIT26;
const ZONE_MASK ZONE_PARTICLES      = BIT27;

const ZONE_MASK ZONE_ANALYTICS      = BIT29;
const ZONE_MASK ZONE_PERF           = BIT30;
const ZONE_MASK ZONE_VERBOSE        = BIT31;


typedef struct
{
    const char* name;
    ZONE_MASK   zone;
} ZONE_MAPPING;


class Log
{
public:
    static RESULT           OpenWithFilename( IN const string& filename );
    static RESULT           Close();
//    static RESULT           SetFilename( IN const string& filename );
    static const string&    GetFilename( );

    static void             SetZoneMask  ( ZONE_MASK zoneMask );
    static ZONE_MASK        GetZoneMask  ( );
    static bool             IsZoneEnabled( ZONE_MASK zoneMask );
    static void             EnableZone   ( ZONE_MASK zoneMask );
    static void             DisableZone  ( ZONE_MASK zoneMask );
    
    
    static void             Print( ZONE_MASK zone, IN const char* format, ... );
    static void             Print( ZONE_MASK zone, IN const std::string& format, ... );
    static void             Print( IN const char* format, ... );
    
    static void             LogStateMachineEvent( OBJECT_ID id, const string& name, MSG_Object* msg, const string& statename, const string& substatename, const string& eventmsgname, bool handled );
    static void             LogStateMachineStateChange( OBJECT_ID id, const string& name, unsigned int state, int substate );
    
public:
    static ZONE_MAPPING ZoneMapping[32 + 1]; // NULL-terminated
    
protected:
    //
    // Singleton
    //
    Log();
    Log(const Log& rhs);
    Log& operator=(const Log& rhs);
    virtual ~Log();
    
    static RESULT           SetFilename( IN const string& filename );
    static void             BackupFileIfItExists( IN const string& filename );
    
protected:
    static bool          s_initialized;
    static std::string   s_filename;
    static FILE         *s_pFile;
    static ZONE_MASK     s_zoneMask;
};


} // END namespace Z

