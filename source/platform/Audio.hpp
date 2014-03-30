#pragma once

#include "Types.hpp"
#import <OpenAL/al.h>
#import <OpenAL/alc.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AudioToolbox/ExtendedAudioFile.h>

#include <string>
using std::string;


namespace Z
{

    
class Audio
{
public:
    static RESULT   GetOpenALDataFromFile( IN const string& filename, OUT ALsizei *pDataSize, INOUT ALvoid** ppData, OUT ALenum *pDataFormat, OUT ALsizei* pSampleRate );

protected:
    Audio();
    Audio( const Audio& rhs );
    Audio& operator=( const Audio& rhs );
};


} // END namespace Z
