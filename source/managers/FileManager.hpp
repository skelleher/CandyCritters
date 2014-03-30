#pragma once

#include <stdio.h>
#include <string>
#include "ResourceManager.hpp"
#include "Handle.hpp"
#include "Types.hpp"

using std::string;

    

namespace Z
{



//
// Map an application-relative file handle
// onto the operating system's actual file
//
class OSFile : virtual public Object 
{
public:
    FILE*   pFILE;
    string  relativeFilename;
    string  absoluteFilename;
    bool    bOpenedForRead;
    bool    bOpenedForWrite;
    
public:
    OSFile( const string& filename ) { m_name = filename; }
protected:
    OSFile();
    OSFile( const OSFile& rhs );
};

typedef Handle<OSFile> HFile;


// 
// Filesystem roots for read-only resources
// and persistent writable storage
//
#define RESOURCE "/app/"
#define STORAGE  "/user/"


class FileManager: public ResourceManager<OSFile>
{
public:
    typedef enum
    {
        READ = 0,
        WRITE,
        APPEND,
        READWRITE,
    } FILE_MODE;
 
public:
    static  FileManager& Instance();
    
    RESULT          OpenFile        ( IN const string& pFilename, INOUT HFile* pHandle, IN FILE_MODE mode = READ             );
    RESULT          GetFileSize     ( IN HFile handle, INOUT UINT32* pFilesize                                               );
    RESULT          ReadFile        ( IN HFile handle, INOUT BYTE* pBuffer, IN UINT32 numBytes, INOUT UINT32* pBytesRead     );
    RESULT          ReadLine        ( IN HFile handle, INOUT BYTE* pBuffer, IN UINT32 numBytes, INOUT UINT32* pBytesRead     );
    RESULT          WriteFile       ( IN HFile handle, INOUT BYTE* pBuffer, IN UINT32 numBytes, INOUT UINT32* pBytesWritten  );
    RESULT          CloseFile       ( IN HFile handle                                                                        );
    
    RESULT          GetAbsolutePath ( IN HFile handle,           INOUT string* pAbsolutePath                                 );
    RESULT          GetAbsolutePath ( IN const string& filename, INOUT string* pAbsolutePath                                 );

protected:
    FileManager();
    FileManager( const FileManager& rhs );
    FileManager& operator=( const FileManager& rhs );
    virtual ~FileManager();
    
    
};

#define FileMan ((FileManager&)FileManager::Instance())



} // END namespace Z


