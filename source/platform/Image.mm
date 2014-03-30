#import  <Foundation/NSData.h>
#include <CoreGraphics/CGImage.h>
#include "Image.hpp"
#include "Log.hpp"
#include "Macros.hpp"
#include "FileManager.hpp"


namespace Z
{



RESULT
Image::ConvertPNGToRGBA( IN BYTE* pInputBuffer, IN UINT32 numBytesIn, INOUT BYTE** ppOutputBuffer, OUT UINT32* pNumBytesOut )
{
    RESULT              rval            = S_OK;
    CGImageRef          cgImage         = nil;
    CGDataProviderRef   cgDataProvider  = nil;
    CFDataRef           cfRawPixelData  = nil;
    
    // Validate params
    if (!pInputBuffer || !ppOutputBuffer || !pNumBytesOut)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Image::ConvertPNGToRGBA( 0x%x, %d, 0x%x, 0x%x ): NULL pointer",
                  pInputBuffer, numBytesIn, ppOutputBuffer, pNumBytesOut);
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    // Init in case we fail and return early.
    *ppOutputBuffer = NULL;
    *pNumBytesOut   = 0;
    
    
    cgDataProvider  = CGDataProviderCreateWithData( NULL, pInputBuffer, numBytesIn, NULL );
    // This step decompresses the .PNG file into a cgImage.
    cgImage         = CGImageCreateWithPNGDataProvider( cgDataProvider, NULL, false, kCGRenderingIntentDefault );
    
    
    switch (CGImageGetAlphaInfo(cgImage))
    {
        case kCGImageAlphaNone:
            DEBUGMSG(ZONE_TEXTURE, "\tkCGImageAlphaNone");
            break;
        case kCGImageAlphaLast:
            DEBUGMSG(ZONE_TEXTURE, "\tkCGImageAlphaLast");
            break;
        case kCGImageAlphaFirst:
            DEBUGMSG(ZONE_TEXTURE, "\tkCGImageAlphaFirst");
            break;
        case kCGImageAlphaPremultipliedLast:
            DEBUGMSG(ZONE_TEXTURE, "\tkCGImageAlphaPremultipliedLast");
            break;
        case kCGImageAlphaPremultipliedFirst:
            DEBUGMSG(ZONE_TEXTURE, "\tkCGImageAlphaPremultipliedFirst");
            break;
        case kCGImageAlphaNoneSkipLast:
            DEBUGMSG(ZONE_TEXTURE, "\tkCGImageAlphaNoneSkipLast");
            break;
        case kCGImageAlphaNoneSkipFirst:
            DEBUGMSG(ZONE_TEXTURE, "\tkCGImageAlphaNoneSkipFirst");
            break;
        default:
            DEBUGMSG(ZONE_TEXTURE, "\tUnknown alpha");
            break;
    }
    
    DEBUGMSG(ZONE_TEXTURE, "\t%d x %d",    CGImageGetWidth(cgImage), CGImageGetHeight(cgImage));
    DEBUGMSG(ZONE_TEXTURE, "\tStride: %d", CGImageGetBytesPerRow(cgImage));
    DEBUGMSG(ZONE_TEXTURE, "\tBPP:    %d", CGImageGetBitsPerPixel(cgImage));
    DEBUGMSG(ZONE_TEXTURE, "\tBPC:    %d", CGImageGetBitsPerComponent(cgImage));
    
    switch (CGColorSpaceGetModel(CGImageGetColorSpace(cgImage)))
    {
        case kCGColorSpaceModelUnknown:
            DEBUGMSG(ZONE_TEXTURE, "\tkCGColorSpaceModelUnknown");
            break;
        case kCGColorSpaceModelMonochrome:
            DEBUGMSG(ZONE_TEXTURE, "\tkCGColorSpaceModelMonochrome");
            break;
        case kCGColorSpaceModelRGB:
            DEBUGMSG(ZONE_TEXTURE, "\tkCGColorSpaceModelRGB");
            break;
        case kCGColorSpaceModelCMYK:
            DEBUGMSG(ZONE_TEXTURE, "\tkCGColorSpaceModelCMYK");
            break;
        case kCGColorSpaceModelLab:
            DEBUGMSG(ZONE_TEXTURE, "\tkCGColorSpaceModelLab");
            break;
        case kCGColorSpaceModelDeviceN:
            DEBUGMSG(ZONE_TEXTURE, "\tkCGColorSpaceModelDeviceN");
            break;
        case kCGColorSpaceModelIndexed:
            DEBUGMSG(ZONE_TEXTURE, "\tkCGColorSpaceModelIndexed");
            break;
        case kCGColorSpaceModelPattern:
            DEBUGMSG(ZONE_TEXTURE, "\tkCGColorSpaceModelPattern");
            break;
    }

    cfRawPixelData  = CGDataProviderCopyData(CGImageGetDataProvider(cgImage));
    *pNumBytesOut   = CFDataGetLength( cfRawPixelData );

    // Allocate a buffer to hold the pixels and return to caller.
    // The caller may have provided a buffer to us.
    if (NULL == *ppOutputBuffer)
    {
        *ppOutputBuffer = new BYTE[*pNumBytesOut];
    }
    CFDataGetBytes(cfRawPixelData, CFRangeMake(0, *pNumBytesOut), *ppOutputBuffer);

Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Image::ConvertPNGToRGBA() failed. rval = %d", rval);
    }

    CGDataProviderRelease(cgDataProvider);
    CGImageRelease(cgImage);
    if (cfRawPixelData)
        CFRelease(cfRawPixelData);
    
    return rval;
}



RESULT
Image::GetImageProperties( IN const string& filename, INOUT ImageProperties* pImageProperties )
{
    RESULT              rval                = S_OK;
    string              filename_lc         = filename;
    string              absoluteFilename;
    CGImageRef          pngImageRef         = NULL;
    CGDataProviderRef   pngProviderRef      = NULL;
    HFile               hFile;
    
    
    if ( !filename.length() || !pImageProperties)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Image::GetProperties( \"\", 0x%x ): invalid argument", pImageProperties);
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    
    // Convert to lower case.
    transform(filename_lc.begin(), filename_lc.end(), filename_lc.begin(), tolower );

    if ( !filename_lc.find(".png"))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Image::GetProperties( \"%s\" ): only .PNG files supported for now", filename.c_str());
        DEBUGCHK(0);
    }
    

    // Convert app-relative filename to absolute filename.
    CHR(FileMan.OpenFile( filename, &hFile ));
    CHR(FileMan.GetAbsolutePath( hFile, &absoluteFilename ));

    
    // Create a PNG data provider.
    pngProviderRef = CGDataProviderCreateWithFilename( absoluteFilename.c_str() );
    if (!pngProviderRef)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Image::GetProperties( \"%s\" ): not a valid .PNG file", filename.c_str());
        rval = E_BAD_FILE_FORMAT;
        goto Exit;
    }
    
    // Create a CGImage.
    pngImageRef = CGImageCreateWithPNGDataProvider(pngProviderRef, NULL, true, kCGRenderingIntentDefault);
    if (!pngImageRef)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Image::GetProperties( \"%s\" ): not a valid .PNG file", filename.c_str());
        rval = E_BAD_FILE_FORMAT;
        goto Exit;
    }
    
   
    // Get the properties.
    pImageProperties->width         = CGImageGetWidth(pngImageRef);
    pImageProperties->height        = CGImageGetHeight(pngImageRef);
    pImageProperties->stride        = CGImageGetBytesPerRow(pngImageRef);
    pImageProperties->bytesPerPixel = CGImageGetBitsPerPixel(pngImageRef) / 8;
    pImageProperties->numBytes      = pImageProperties->height * pImageProperties->stride * pImageProperties->bytesPerPixel;
    
    
Exit:
    if ( !hFile.IsNull() )
        FileMan.CloseFile( hFile );

    if ( pngProviderRef )
        CGDataProviderRelease( pngProviderRef );
        
    if ( pngImageRef )
        CGImageRelease( pngImageRef );
        
    return rval;
}


} // END namespace Z

