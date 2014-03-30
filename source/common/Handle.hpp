#pragma once

#include "Types.hpp"

#include <string>
using std::string;


namespace Z
{

// Forward declarations.
template <typename T> class ResourceManager;
class IProperty;


//
// Hat-tip to Scott Bilas for his Handle implementation.
//

template <typename TYPE>
class Handle
{
public:
    Handle();
    ~Handle();  // non-virtual to keep sizeof(Handle) 8 bytes instead of 12

    
    void            Init       ( UINT32 index, ResourceManager<TYPE>* pResourceManager = NULL );
    
    UINT32          GetHandle  ( )    const;
    UINT32          GetIndex   ( )    const;
    UINT32          GetToken   ( )    const;
    bool            IsNull     ( )    const;
    bool            IsDeleted  ( )    const;
    bool            IsDangling ( )    const;
    bool            IsValid    ( )    const;
    RESULT          Delete     ( );
    
    // Be sure to delete the IProperty when done.
    // TODO: don't alloc in this method?
    IProperty*      GetProperty( const std::string& name ) const;
    
    const string&   GetName    ( ) const;
    OBJECT_ID       GetID      ( ) const;
    UINT32          GetRefCount( ) const;
    RESULT          AddRef     ( ) const;
    RESULT          Release    ( ) const;
    
    
    operator unsigned long() const;
    bool operator != ( Handle<TYPE> rhs )  const;
    bool operator == ( Handle<TYPE> rhs )  const;
    
    
    static UINT32       NumHandles();
    static Handle<TYPE> NullHandle();
    static Handle<TYPE> DeletedHandle();
    
protected:
//    Handle( const Handle& rhs );
//    Handle& operator=( const Handle& rhs );

    enum
    {
        // Sizes to use for bit fields
        INDEX_BITS = 16,
        TOKEN_BITS = 16,
        
        INDEX_MASK  = 0xFFFF0000,
        TOKEN_MASK  = 0x0000FFFF,
        
        // Sizes to compare against for asserting dereferences
        MAX_INDEX = ( 1 << INDEX_BITS)  - 1,
        MAX_TOKEN = ( 1 << TOKEN_BITS)  - 2,

        // This token indicates a handle that has been deleted.
        // ::CloseHandle() is the only valid operation on it.
        DELETED_HANDLE_TOKEN = MAX_TOKEN +1
    };
    
    union 
    {
        UINT32      m_handle;

        struct 
        {
            UINT32  m_index:INDEX_BITS;
            UINT32  m_token:TOKEN_BITS;
        };
    };


    // All Handles are created by a ResourceManager.
    // We could avoid the backlink (and save 4 bytes!)
    // except that its useful for animation and scripting 
    // to call hFoo.GetProperty()
    ResourceManager<TYPE>*  m_pResourceManager;
    
    
    static  UINT32  s_nextToken;
    static  UINT32  s_numHandles;
};

#define NULL_HANDLE     (Handle<TYPE>::NullHandle())
#define DELETED_HANDLE  (Handle<TYPE>::DeletedHandle())

} // END namespace Z


#include "Handle.cpp"


