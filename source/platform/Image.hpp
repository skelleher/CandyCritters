#pragma once

#include "Types.hpp"

#include <string>
using std::string;


namespace Z
{


struct ImageProperties
{
    UINT32  width;
    UINT32  height;
    UINT32  stride;
    UINT32  bytesPerPixel;
    UINT32  numBytes;
};

    
class Image
{
public:
    // Caller may provide ppOutputBuffer.  If NULL, method will allocate and return one which the caller must delete.
    static RESULT   ConvertPNGToRGBA    ( IN BYTE* pInputBuffer, IN UINT32 numBytesIn, INOUT BYTE** ppOutputBuffer, OUT UINT32* pNumBytesOut );
    static RESULT   GetImageProperties  ( IN const string& filename, INOUT ImageProperties* pImageProperties );
    
protected:
    Image();
    Image( const Image& rhs );
    Image& operator=( const Image& rhs );
};


} // END namespace Z
