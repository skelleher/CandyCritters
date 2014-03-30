/*
 *  Handle.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 9/10/10.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include "Handle.hpp"
#include "Object.hpp"
#include "Log.hpp"
#include "Macros.hpp"
#include "Platform.hpp"
#include "ResourceManager.hpp"


namespace Z
{



template <typename TYPE>
UINT32 Handle<TYPE>::s_nextToken = 0;

template <typename TYPE>
UINT32 Handle<TYPE>::s_numHandles = 0;



template <typename TYPE>
Handle<TYPE>::Handle() :
    m_handle(0),
    m_pResourceManager(NULL)
{
    ATOMIC_INCREMENT(s_numHandles);
}


template <typename TYPE>
Handle<TYPE>::~Handle()
{
    // TODO: decrement the refcount/assert refcount == 0?
    // Handles should behave like smart pointers; when they go out of scope,
    // release the object they refer to.

    ATOMIC_DECREMENT(s_numHandles);
}



template <typename TYPE>
void
Handle<TYPE>::Init( UINT32 index, ResourceManager<TYPE>* pResourceManager )
{
    if ( !IsNull() )
    {
        // Don't allow reinitializing a handle
        RETAILMSG(ZONE_ERROR, "ERROR: Handle::Init( %d ) on existing Handle( %d, %d )",
                  index, m_index, m_token);
        return;
    }

/*
    if (!pResourceManager)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Handle::Init( %d ): pResourceManager cannot be NULL");
        return;
    }
*/

    DEBUGCHK(pResourceManager);

    m_pResourceManager = pResourceManager;


    DEBUGCHK (index < MAX_INDEX);

    m_index = index;
    m_token = Handle::TOKEN_MASK & Platform::Random();

    // Once in a blue moon Random() == DELETED_HANDLE_TOKEN, so try again.
    if (m_token == DELETED_HANDLE_TOKEN)
    {
        m_token = Handle::TOKEN_MASK & Platform::Random();
    }

    RETAILMSG(ZONE_HANDLE, "Handle::Init( index = %d, token = %d )", m_index, m_token);
}



template <typename TYPE>
UINT32  
Handle<TYPE>::GetHandle() const
{
    return m_handle;
}



template <typename TYPE>
UINT32
Handle<TYPE>::GetIndex() const
{
    return m_index;
}



template <typename TYPE>
UINT32
Handle<TYPE>::GetToken() const
{
    return m_token;
}



template <typename TYPE>
bool
Handle<TYPE>::IsNull() const
{
    return !m_handle;

/*
    if (!m_handle || IsDeleted() || IsDangling() )
    {
        return true;
    }
    else
    {
        return false;
    }
*/
}



template <typename TYPE>
bool
Handle<TYPE>::IsDeleted() const
{
    return (m_token == DELETED_HANDLE_TOKEN) ? true : false;
}



template <typename TYPE>
bool
Handle<TYPE>::IsDangling() const
{
    if (m_handle && GetID() == INVALID_OBJECT_ID)
    {
        return true;
    }
    else 
    {
        return false;
    }
}



template <typename TYPE>
bool
Handle<TYPE>::IsValid() const
{
    if (m_pResourceManager)
    {
        return m_pResourceManager->ValidHandle( *this );
    }
    else
    {
        return false;
    }
}



template <typename TYPE>
RESULT
Handle<TYPE>::Delete()
{
    m_token = DELETED_HANDLE_TOKEN;
    return S_OK;
}



template <typename TYPE>
IProperty*
Handle<TYPE>::GetProperty( IN const std::string& name ) const
{
    IProperty* rval = NULL;

    if (m_pResourceManager)
    {
       rval = m_pResourceManager->GetProperty( *this, name );
    }
//    else 
//    {
//        DEBUGCHK(0);
//    }

    
    return rval;
}


template <typename TYPE>
const string&
Handle<TYPE>::GetName( ) const
{
    static string rval = "";

    if (m_pResourceManager)
    {
        m_pResourceManager->GetName( *this, &rval );
    }
    
    return rval;
}



template <typename TYPE>
OBJECT_ID
Handle<TYPE>::GetID( ) const
{
    OBJECT_ID rval = INVALID_OBJECT_ID;

    if (m_pResourceManager)
    {
        m_pResourceManager->GetObjectID( *this, &rval );
    }
    
    return rval;
}



template <typename TYPE>
UINT32
Handle<TYPE>::GetRefCount( ) const
{
    UINT32 rval = 0;

    if (m_pResourceManager)
    {
        rval = m_pResourceManager->GetRefCount( *this );
    }
    
    return rval;
}



template <typename TYPE>
RESULT
Handle<TYPE>::AddRef( ) const
{
    RESULT rval = S_OK;
    
    if (m_pResourceManager)
    {
        rval = m_pResourceManager->AddRef( *this );
    }
    else 
    {
        DEBUGCHK(0);
    }

    return rval;
}



template <typename TYPE>
RESULT
Handle<TYPE>::Release( ) const
{
    RESULT rval = S_OK;
    
    if (m_pResourceManager)
    {
        rval = m_pResourceManager->Release( *this );
    }
//    else 
//    {
//        DEBUGCHK(0);
//    }
    
    return rval;
}




template <typename TYPE>
Handle<TYPE>::operator unsigned long() const
{
    return (unsigned long)m_handle;
//    return (( ((UINT32)m_index) << INDEX_BITS) & INDEX_MASK) | ( ((UINT32)m_token) & TOKEN_MASK);
}



template <typename TYPE>
bool 
Handle<TYPE>::operator != ( Handle<TYPE> rhs ) const
{  
    return ( this->GetHandle() != rhs.GetHandle() );  
}



template <typename TYPE>
bool 
Handle<TYPE>::operator == ( Handle<TYPE> rhs ) const
{  
    return ( this->GetHandle() == rhs.GetHandle() );  
}


template <typename TYPE>
UINT32
Handle<TYPE>::NumHandles()
{
    return s_numHandles;
}


template <typename TYPE>
Handle<TYPE>
Handle<TYPE>::NullHandle()
{
    Handle nullHandle;
    
    nullHandle.m_index = 0;
    nullHandle.m_token = 0;
    
    return nullHandle;
}



template <typename TYPE>
Handle<TYPE>
Handle<TYPE>::DeletedHandle()
{
    Handle deletedHandle;
    
    deletedHandle.m_index = 0;
    deletedHandle.m_token = DELETED_HANDLE_TOKEN;
    
    return deletedHandle;
}


} // END namespace Z
