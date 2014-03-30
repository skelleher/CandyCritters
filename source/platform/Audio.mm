#include "Audio.hpp"
#include "Log.hpp"
#include "FileManager.hpp"

#import <Foundation/NSString.h>
#import <Foundation/NSURL.h>


namespace Z
{



RESULT
Audio::GetOpenALDataFromFile( IN const string& filename, OUT ALsizei *pNumBytes, INOUT ALvoid** ppData, OUT ALenum *pDataFormat, OUT ALsizei* pSampleRate )
{
    RESULT                          rval = S_OK;

    string                          filename_lc;
    string                          absoluteFilename;

    CFURLRef                        url;
    OSStatus                        err = noErr;	
    SInt64                          numFrames = 0;
    AudioStreamBasicDescription     format;
    UINT32                          propertySize = 0;
    ExtAudioFileRef                 fileRef = NULL;
    BYTE*                           pData = NULL;
    UINT32                          numBytes;
    AudioStreamBasicDescription     streamDesc;


    if (filename == "")
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Audio::GetOpenALDataFromFile(): must specify filename.");
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    if (!pNumBytes || !ppData || !pDataFormat || !pSampleRate)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Audio::GetOpenALDataFromFile(): argument cannot be NULL.");
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    
    // Strip off the virtual root folder.
    // Convert the app-relative filename to an absolute filename.
    // Convert to a URL.
    filename_lc = filename;
    filename_lc.replace(0, strlen(RESOURCE), "");
    CHR(Platform::GetPathForResource( filename_lc, &absoluteFilename ));
    url = (CFURLRef)[NSURL fileURLWithPath:[NSString stringWithCString:absoluteFilename.c_str() encoding:NSUTF8StringEncoding ]];


    //
    // Open the file with AudioToolbox.
    //
    err = ExtAudioFileOpenURL(url, &fileRef);
    if(err) 
    { 
        RETAILMSG(ZONE_ERROR, "ERROR: Audio::GetOpenALDataFromFile(): ExtAudioFileOpenURL(): error = %d\n", err);
        rval = E_FAIL;
        goto Exit; 
    }
	
    //
    // Get the source format.
    //
    propertySize = sizeof(format);
    err = ExtAudioFileGetProperty(fileRef, kExtAudioFileProperty_FileDataFormat, &propertySize, &format);
    if(err) 
    { 
        RETAILMSG(ZONE_ERROR, "ERROR: Audio::GetOpenALDataFromFile(): ExtAudioFileGetProperty(): error = %d\n", err);
        rval = E_FAIL;
        goto Exit; 
    }

	if (format.mChannelsPerFrame > 2)
    { 
        RETAILMSG(ZONE_ERROR, "ERROR: Audio::GetOpenALDataFromFile(): unsupported channel count %d", format.mChannelsPerFrame);
        rval = E_FAIL;
        goto Exit; 
    }
    

    //
	// Set the output format to 16 bit signed integer (native-endian) data.
	// Maintain the channel count and sample rate of the original source format.
    //
	streamDesc.mSampleRate          = format.mSampleRate;
	streamDesc.mChannelsPerFrame    = format.mChannelsPerFrame;

	streamDesc.mFormatID            = kAudioFormatLinearPCM;
	streamDesc.mBytesPerPacket      = 2 * streamDesc.mChannelsPerFrame;
	streamDesc.mFramesPerPacket     = 1;
	streamDesc.mBytesPerFrame       = 2 * streamDesc.mChannelsPerFrame;
	streamDesc.mBitsPerChannel      = 16;
	streamDesc.mFormatFlags         = kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger;
	
    //
	// Tell AudioToolbox to convert the file to the output (client) format we just specified.
    //
	err = ExtAudioFileSetProperty(fileRef, kExtAudioFileProperty_ClientDataFormat, sizeof(streamDesc), &streamDesc);
    if(err) 
    { 
        RETAILMSG(ZONE_ERROR, "ERROR: Audio::GetOpenALDataFromFile(): ExtAudioFileSetProperty(): error = %d\n", err);
        rval = E_FAIL;
        goto Exit; 
    }
	
    
    //
	// How many frames in the converted file?
    //
    propertySize = sizeof(numFrames);
	err = ExtAudioFileGetProperty(fileRef, kExtAudioFileProperty_FileLengthFrames, &propertySize, &numFrames);
    if(err) 
    { 
        RETAILMSG(ZONE_ERROR, "ERROR: Audio::GetOpenALDataFromFile(): ExtAudioFileGetProperty(): error = %d\n", err);
        rval = E_FAIL;
        goto Exit; 
    }
	
    
    //
	// Read all the data into memory
    //
	numBytes = numFrames * streamDesc.mBytesPerFrame;
	pData = new BYTE[numBytes];
    if (!pData)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Audio::GetOpenALDataFromFile(): out of memory");
        rval = E_OUTOFMEMORY;
        goto Exit;
    }
    
	if (pData)
	{
        // Create an AudioBufferList with one slot (large enough to hold the entire file).
        // We only care about the .mData member, which we'll return to the caller.
		AudioBufferList pBufferList;
		pBufferList.mNumberBuffers                = 1;
		pBufferList.mBuffers[0].mDataByteSize     = numBytes;
		pBufferList.mBuffers[0].mNumberChannels   = streamDesc.mChannelsPerFrame;
		pBufferList.mBuffers[0].mData             = pData;
		
		// Read the data into our AudioBufferList
		err = ExtAudioFileRead(fileRef, (UINT32*)&numFrames, &pBufferList);
		if(err == noErr)
		{
			*pNumBytes      = (ALsizei)numBytes;
			*pDataFormat    = (streamDesc.mChannelsPerFrame > 1) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
			*pSampleRate    = (ALsizei)streamDesc.mSampleRate;
		}
		else 
		{ 
            SAFE_ARRAY_DELETE(pData);
            printf("MyGetOpenALAudioData: ExtAudioFileRead FAILED, Error = %ld\n", err); goto Exit;
		}	
	}
	
    
    // Call must free.
    *ppData = pData;
    

Exit:
	if (fileRef) 
    {
        ExtAudioFileDispose(fileRef);
    }
    
	return rval;
}



} // END namespace Z

