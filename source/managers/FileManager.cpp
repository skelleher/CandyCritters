#include "FileManager.hpp"

#include <sys/stat.h> // for fstat()
#include <string>
#include "Platform.hpp"

using std::string;


namespace Z 
{



FileManager& 
FileManager::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new FileManager();
    }
    
    return static_cast<FileManager&>(*s_pInstance);
}


FileManager::FileManager()
{
    RETAILMSG(ZONE_INFO, "FileManager created.");
    
    s_pResourceManagerName = "FileManager";
}


FileManager::~FileManager()
{
    RETAILMSG(ZONE_INFO, "FileManager destroyed.");
}





RESULT
FileManager::OpenFile( IN const string& filename, INOUT HFile* pHandle, IN FILE_MODE mode )
{
    RESULT      rval    = S_OK;
    OSFile*     pOSFile;
    HFile       hFile;
    string      filename_lc;
    string      absoluteFilename;
    const char* pStartOfPath;
    const char* pEndOfPath;
    
    
    if (!pHandle)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::OpenFile( \"%s\", 0x%x, %d ): NULL handle pointer",
                  filename.c_str(), pHandle, mode);
        rval = E_NULL_POINTER;
        goto Exit;
    }

    if ("" == filename)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::OpenFile( \"%s\", 0x%x, %d ): filename is empty string",
                  filename.c_str(), (UINT32)*pHandle, mode);
        rval = E_INVALID_ARG;
        goto Exit;
    }
    

    const char* pMode;
    switch (mode)
    {
        case READ:
            pMode = "rb";
            break;
        case WRITE:
            pMode = "wb";
            break;
        case APPEND:
            pMode = "ab+";
            break;
        case READWRITE:
            pMode = "wb+";
            break;
        default:
            RETAILMSG(ZONE_ERROR, "ERROR: FileManager::OpenFile(): invalid mode %d", mode);
            rval = E_INVALID_ARG;
            goto Exit;
    }

    
    // Create local, writable copy of filename for substitutions.
    filename_lc = filename;
    
    
    //
    // Convert the relative path to an absolute path
    //
    if (0 == filename_lc.find( RESOURCE ))
    {
        // Resource files are read-only on iOS devices
        if (mode == WRITE || mode == READWRITE)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: FileManager::OpenFile(): file \"%s\" can only be opened for READ",
                      filename_lc.c_str());
            rval = E_ACCESS_DENIED;
            goto Exit;
        }
        
        // Strip off the virtual root folder
        filename_lc.replace(0, strlen(RESOURCE), "");

        CHR(Platform::GetPathForResource( filename_lc, &absoluteFilename ));
    }
    else if (0 == filename_lc.find( STORAGE ))
    {
        // Strip off the virtual root folder
        filename_lc.replace(0, strlen(STORAGE), "");

        string pathToPersistantStore;
        CHR(Platform::GetPathToPersistantStorage( &pathToPersistantStore ));
        absoluteFilename = pathToPersistantStore + "/" + filename_lc;
    }
    else
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::OpenFile(): Invalid path \"%s\"", filename.c_str());
        rval = E_INVALID_ARG;
        goto Exit;
    }

    //
    // Check if the file was previously opened and return existing handle
    //

    if (SUCCEEDED(Get( filename, &hFile )))
    {
        *pHandle = hFile;
        goto Exit;
    }

    
    //
    // Newly-opened file
    //
    
    DEBUGMSG(ZONE_FILE | ZONE_VERBOSE, "FileManager::OpenFile(): creating OSFile \"%s\"", filename.c_str());
    pOSFile = new OSFile( filename );
    DEBUGCHK(pOSFile);
    pOSFile->relativeFilename = filename_lc;
    pOSFile->absoluteFilename = absoluteFilename;
    

    //
    // Create the directory, if needed
    //
    pStartOfPath  = absoluteFilename.c_str();
    pEndOfPath    = strrchr(pStartOfPath, '/');
    if (pEndOfPath - pStartOfPath > 0)
    {
        struct stat filestats;
        string path;
        
        path.append( pStartOfPath, pEndOfPath - pStartOfPath );
        const char* pPath = path.c_str();
        
        int error = stat(pPath, &filestats);
        
        if (error || !(filestats.st_mode & S_IFDIR))
        {
            // Directory does not exist; create it
            error = mkdir(pPath, S_IRWXU);
            if (error)
            {
                RETAILMSG(ZONE_ERROR, "ERROR: FileManager::OpenFile(): can't create directory \"%s\", error: %d",
                      pPath, error);
                rval = E_ACCESS_DENIED;
                goto Exit;
            }
            else
            {
                RETAILMSG(ZONE_FILE | ZONE_VERBOSE, "FileManager::OpenFile(): created directory \"%s\"", pPath); 
            }
        }
    }

    
    //
    // Open the file
    //
    pOSFile->pFILE = fopen(absoluteFilename.c_str(), pMode);
    if (!pOSFile->pFILE)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::OpenFile(): can't open [%s] for mode [%s]",
                  absoluteFilename.c_str(), pMode);
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    switch (mode)
    {
        case READ:
            pOSFile->bOpenedForRead     = true;
            break;
        case WRITE:
        case APPEND:
            pOSFile->bOpenedForWrite    = true;
            break;
        case READWRITE:
            pOSFile->bOpenedForRead     = true;
            pOSFile->bOpenedForWrite    = true;           
    };

/*    
    if (FAILED(GetHandle( filename, &hFile )))
    {
        // Create handle for the file, and save in our lookup tables
        CHR(Add( filename, pOSFile, &hFile ));
    }
*/

    // Create handle for the file, and save in our lookup tables
    CHR(Add( filename, pOSFile, &hFile ));
    *pHandle = hFile;
    
Exit:
    return rval;
}



RESULT
FileManager::GetFileSize( IN HFile handle, INOUT UINT32* pFilesize)
{
    RESULT rval             = S_OK;
    OSFile* pOSFile         = NULL;
    struct stat filestats;
    
    if (!pFilesize)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::GetFileSize( 0x%x, 0x%x ): NULL pointer",
                  (UINT32)handle, pFilesize);
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    if (!ValidHandle(handle))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::GetFileSize( 0x%x ): invalid handle", (UINT32)handle);
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    
    pOSFile = GetObjectPointer( handle );
    if (!pOSFile)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::GetFileSize( 0x%x ): file not found", (UINT32)handle);
        rval = E_FILE_NOT_FOUND;
        goto Exit;
    }

    if (0 != stat(pOSFile->absoluteFilename.c_str(), &filestats))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::GetFileSize( 0x%x ): fstat() failed", (UINT32)handle);
        rval = E_UNEXPECTED;
        goto Exit;
    }
    

    *pFilesize = (UINT32)filestats.st_size;
    
Exit:
    return rval;
}



RESULT
FileManager::GetAbsolutePath( IN const string& filename, INOUT string* pAbsolutePath )
{
    RESULT rval = S_OK;
    string absoluteFilename;

    if (!pAbsolutePath)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::GetAbsolutePath(): NULL pointer");
        return E_NULL_POINTER;
    }
    

    // Create local, writable copy of filename for substitutions.
    string filename_lc = filename;

    //
    // Convert the relative path to an absolute path
    //
    if (0 == filename_lc.find( RESOURCE ))
    {
        // Strip off the virtual root folder
        filename_lc.replace(0, strlen(RESOURCE), "");

        CHR(Platform::GetPathForResource( filename_lc, &absoluteFilename ));
    }
    else if (0 == filename_lc.find( STORAGE ))
    {
        // Strip off the virtual root folder
        filename_lc.replace(0, strlen(STORAGE), "");

        string pathToPersistantStore;
        CHR(Platform::GetPathToPersistantStorage( &pathToPersistantStore ));
        absoluteFilename = pathToPersistantStore + "/" + filename_lc;
    }
    else
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::GetAbsolutePath(): Invalid path \"%s\"", filename.c_str());
        rval = E_INVALID_ARG;
        goto Exit;
    }

    *pAbsolutePath = absoluteFilename;
    
Exit:
    return rval;
}



RESULT
FileManager::GetAbsolutePath( IN HFile handle, INOUT string* pAbsolutePath )
{
    RESULT  rval    = S_OK;
    OSFile* pOSFile = NULL;
    

    if (!pAbsolutePath)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::GetAbsolutePath( 0x%x, 0x%x ): NULL pointer",
                  (UINT32)handle, pAbsolutePath);
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    if (!ValidHandle(handle))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::GetAbsolutePath( 0x%x ): invalid handle", (UINT32)handle);
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    
    pOSFile = GetObjectPointer( handle );
    if (!pOSFile)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::GetAbsolutePath( 0x%x ): file not found", (UINT32)handle);
        rval = E_FILE_NOT_FOUND;
        goto Exit;
    }
    
    *pAbsolutePath = pOSFile->absoluteFilename;
    
Exit:
    return rval;
}



RESULT  
FileManager::ReadFile( IN HFile handle, INOUT BYTE* pBuffer, IN UINT32 numBytes, INOUT UINT32* pBytesRead )
{
    RESULT  rval        = S_OK;
    OSFile* pOSFile     = NULL;
    int     result      = 0;
    
    
    if (!pBuffer || !pBytesRead)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::ReadFile( 0x%x, 0x%x, %d, 0x%x ): NULL pointer",
                  (UINT32)handle, pBuffer, numBytes, pBytesRead);
        rval = E_NULL_POINTER;
        goto Exit;
    }
        
    if (!ValidHandle(handle))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::ReadFile( 0x%x, 0x%x, %d, 0x%x ): invalid handle",
                  (UINT32)handle, pBuffer, numBytes, pBytesRead);
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    

    *pBytesRead = 0;
    pOSFile     = GetObjectPointer( handle );    // Won't addref
    if (!pOSFile)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::ReadFile( 0x%x, 0x%x, %d, 0x%x ): file not found",
                  (UINT32)handle, pBuffer, numBytes, pBytesRead);
        rval = E_FILE_NOT_FOUND;
        goto Exit;
    }
    
    
    if (0 == numBytes)
    {
        goto Exit;
    }


    fflush( pOSFile->pFILE );
    result = fread ( pBuffer, numBytes, 1, pOSFile->pFILE ); 
    if (1 != result)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::ReadFile( 0x%x, 0x%x, %d, 0x%x ): fread() returned %d",
                  (UINT32)handle, pBuffer, numBytes, pBytesRead, result);
        rval = E_UNEXPECTED;
        goto Exit;
    }
    
    *pBytesRead = numBytes;

Exit:
    return rval;
}



RESULT  
FileManager::WriteFile( IN HFile handle, INOUT BYTE* pBuffer, IN UINT32 numBytes, INOUT UINT32* pBytesWritten )
{
    RESULT  rval        = S_OK;
    OSFile* pOSFile     = NULL;
    int     result      = 0;
    
    
    if (!pBuffer || !pBytesWritten)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::WriteFile( 0x%x, 0x%x, %d, 0x%x ): NULL pointer",
                  (UINT32)handle, pBuffer, numBytes, pBytesWritten);
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    if (!ValidHandle(handle))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::WriteFile( 0x%x, 0x%x, %d, 0x%x ): invalid handle",
                  (UINT32)handle, pBuffer, numBytes, pBytesWritten);
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    
    
    *pBytesWritten = 0;
    pOSFile        = GetObjectPointer( handle ); // won't addref
    if (!pOSFile)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::WriteFile( 0x%x, 0x%x, %d, 0x%x ): file not found",
                  (UINT32)handle, pBuffer, numBytes, pBytesWritten);
        rval = E_FILE_NOT_FOUND;
        goto Exit;
    }
    
    
    if (0 == numBytes)
    {
        goto Exit;
    }
    
    result = fwrite( pBuffer, numBytes, 1, pOSFile->pFILE );
    if (1 != result)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::WriteFile( 0x%x, 0x%x, %d, 0x%x ): fwrite() error",
                  (UINT32)handle, pBuffer, numBytes, pBytesWritten, result);
        rval = E_UNEXPECTED;
        goto Exit;
    }
    
    fflush(pOSFile->pFILE);
    
    
    *pBytesWritten = numBytes;
    
Exit:
    return rval;
}



RESULT  
FileManager::ReadLine( IN HFile handle, INOUT BYTE* pBuffer, IN UINT32 numBytes, INOUT UINT32* pBytesRead )
{
    RESULT  rval        = S_OK;
    OSFile* pOSFile     = NULL;
    BYTE*   pResult     = NULL;
    
    
    if (!pBuffer || !pBytesRead)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::ReadLine( 0x%x, 0x%x, %d, 0x%x ): NULL pointer",
                  (UINT32)handle, pBuffer, numBytes, pBytesRead);
        rval = E_NULL_POINTER;
        goto Exit;
    }
        
    if (!ValidHandle(handle))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::ReadLine( 0x%x, 0x%x, %d, 0x%x ): invalid handle",
                  (UINT32)handle, pBuffer, numBytes, pBytesRead);
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    

    *pBytesRead = 0;
    pOSFile     = GetObjectPointer( handle );    // Won't addref
    if (!pOSFile)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::ReadLine( 0x%x, 0x%x, %d, 0x%x ): file not found",
                  (UINT32)handle, pBuffer, numBytes, pBytesRead);
        rval = E_FILE_NOT_FOUND;
        goto Exit;
    }
    
    
    if (0 == numBytes)
    {
        goto Exit;
    }


//    fflush( pOSFile->pFILE );
    pResult = (BYTE*)fgets( (char*)pBuffer, numBytes, pOSFile->pFILE );
    if (!pResult)
    {
        rval = E_EOF;
        goto Exit;
    }
    
    *pBytesRead = numBytes;

Exit:
    return rval;
}



RESULT  
FileManager::CloseFile( IN HFile handle )
{
    RESULT  rval        = S_OK;
    OSFile* pOSFile     = NULL;

    if (!ValidHandle(handle))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::CloseFile( 0x%x ): invalid handle", (UINT32)handle);
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    
    pOSFile = GetObjectPointer( handle );    // no addref
    if (!pOSFile)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::CloseFile( 0x%x ): file not found", (UINT32)handle);
        rval = E_FILE_NOT_FOUND;
        goto Exit;
    }

    if (EOF == fclose(pOSFile->pFILE))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FileManager::CloseFile( \"%s\" ): fclose() error", pOSFile->relativeFilename.c_str());
        rval = E_UNEXPECTED;
        goto Exit;
    }

    //SAFE_RELEASE(pOSFile);
    
//    if (pOSFile->GetRefCount() == 1)
//    {
//        RETAILMSG(ZONE_FILE | ZONE_VERBOSE, "FileManager::CloseFile(): closing last handle; file will be freed");
//        CHR(CloseHandle( handle ));
//    }
//    else 
//    {
//        SAFE_RELEASE(pOSFile);
//    }

    /*
    // Do NOT close the handle.  It will invalidate all existing copies of the handle.
    // Releasing pOSFile is enough; when it's ref count goes to zero, the existing handles
    // will become invalid.
    */
    
    CHR(Release( handle ));

Exit:
    //SAFE_RELEASE(pOSFile);
    return rval;
}



} // END namespace Z




