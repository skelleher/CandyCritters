#pragma once

#include <map>
#include <vector>
#include <string>
#include <list>
#include "Types.hpp"
#include "Handle.hpp"
#include "Property.hpp"
#include "Log.hpp"


using std::map;
using std::vector;
using std::string;
using std::list;


namespace Z
{


template<typename TYPE>
class ResourceManager
{
public:
    // Singleton
    static ResourceManager<TYPE>& Instance();
    
    virtual RESULT          Init            ( );
    virtual RESULT          Add             ( IN const string& name, IN TYPE* pResource, INOUT Handle<TYPE>* pHandle = NULL );
    
    // TODO: this is so common, we should just return the Handle (NULL for failure).
    virtual RESULT          Get             ( IN const string& name, INOUT Handle<TYPE>* pHandle );
    virtual RESULT          GetCopy         ( IN const string& name, INOUT Handle<TYPE>* pHandle );
    
    
    virtual RESULT          AddRef          ( IN Handle<TYPE> handle );
    virtual RESULT          Release         ( IN Handle<TYPE> handle );
    virtual RESULT          Remove          ( IN Handle<TYPE> handle );
    virtual RESULT          Release         ( IN TYPE* pResource, IN Handle<TYPE> handle = NULL_HANDLE );
    virtual RESULT          Remove          ( IN TYPE* pResource, IN Handle<TYPE> handle = NULL_HANDLE );
    
    // TODO: this is so common, we should just return the value (empty/0 for failure)
    virtual RESULT          GetName         ( IN Handle<TYPE> handle, INOUT string*       pName      ) const;
    virtual RESULT          GetObjectID     ( IN Handle<TYPE> handle, INOUT OBJECT_ID*    pObjectID  ) const;
    virtual UINT32          GetRefCount     ( IN Handle<TYPE> handle ) const;

    virtual IProperty*      GetProperty     ( IN Handle<TYPE> handle, IN const string& propertyName  ) const;
    
    virtual UINT32          Count           ( );
    virtual RESULT          Shutdown        ( );

    virtual bool            ValidHandle     ( Handle<TYPE>, bool bDeletedHandleIsValid = false ) const;
    
    virtual void            Print           ( );
    
protected:
    ResourceManager<TYPE>();
    ResourceManager<TYPE>( const ResourceManager<TYPE>& rhs );
    ResourceManager<TYPE>& operator=( const ResourceManager<TYPE>& rhs );
    virtual ~ResourceManager<TYPE>();
    
    virtual Handle<TYPE>    CreateHandle    ( UINT32 index );
//    virtual bool            ValidHandle     ( Handle<TYPE>, bool bDeletedHandleIsValid = false ) const;

    virtual TYPE*           GetObjectPointer( IN const Handle<TYPE> handle, bool addref = false ) const;
    
protected:
    typedef map<std::string, Handle<TYPE> >     NameToHandleMap;    // 1:1      - returns first valid handle for the name
    typedef map<Handle<TYPE>, std::string>      HandleToNameMap;    // many:1   - many handles may map to the same name
    typedef vector<Handle<TYPE> >               HandleList;
    typedef vector<TYPE*>                       ResourceList;
    
    // The "typename" keyword is required when declaring an iterator on a nested template,
    // such as std::map<const char*, Handle<TYPE> >
    // See Question #1 in the C++ Templates FAQ
    typedef typename NameToHandleMap::iterator  NameToHandleMapIterator;
    typedef typename HandleToNameMap::iterator  HandleToNameMapIterator;
    typedef typename HandleList::iterator       HandleListIterator;
    typedef typename ResourceList::iterator     ResourceListIterator;
    
    NameToHandleMap                             m_nameToHandleMap;
    HandleToNameMap                             m_handleToNameMap;
    HandleList                                  m_validHandleList;
    
    ResourceList                                m_resourceList;
    list<UINT32>                                m_freeSlotList;
    
    
    static ResourceManager<TYPE>*               s_pInstance;
    static const char*                          s_pResourceManagerName;
};

} // END namespace Z


#pragma mark ResourceManager Template Implementation

#include "Object.hpp"


namespace Z
{


//----------------------------------------------------------------------------
// Static data
//----------------------------------------------------------------------------

template<typename TYPE>
ResourceManager<TYPE>* ResourceManager<TYPE>::s_pInstance = NULL;

template<typename TYPE>
const char* ResourceManager<TYPE>::s_pResourceManagerName = "ResourceManager";  // Subclasses should provide a name



//----------------------------------------------------------------------------
// Static methods
//----------------------------------------------------------------------------


template<typename TYPE>
ResourceManager<TYPE>& 
ResourceManager<TYPE>::Instance()
{
    // Create a *local* static instance; defers creation until the first call to Instance()
    //    static ResourceManager<TYPE> instance;
    //    return instance;
    
    if (!s_pInstance) 
    {
        s_pInstance = new ResourceManager<TYPE>;
    }
    
    return *s_pInstance;
}


//----------------------------------------------------------------------------
// Protected methods
//----------------------------------------------------------------------------
template<typename TYPE>
ResourceManager<TYPE>::ResourceManager()
{
}


template<typename TYPE>
ResourceManager<TYPE>::~ResourceManager()
{
    Shutdown();
}


template<typename TYPE>
Handle<TYPE> 
ResourceManager<TYPE>::CreateHandle( UINT32 index )
{
    Handle<TYPE> handle;
    
    handle.Init( index, this );
    
    return handle;
}


template<typename TYPE>
bool
ResourceManager<TYPE>::ValidHandle( Handle<TYPE> handle, bool bDeletedHandleIsValid ) const
{
    bool    rval    = true;
    UINT32  index   = handle.GetIndex();
    UINT32  token   = handle.GetToken();
    
    // TODO: consider setting the token and index to known safe values
    // as soon as a Validity check fails; protects less-than-robust ResourceManagers
    // from indexing into a table and getting garbage memory.
    
    
    if ( m_validHandleList.empty()      ||
         handle.IsNull()                ||
         index > m_resourceList.size()  ||
         index > m_validHandleList.size()   
        )
    {
        return false;
    }
    
    // Compare the caller's token to our private copy
    Handle<TYPE> privateHandle = m_validHandleList[ index ];
    
    // Deleted handles are valid, but only for ResourceManager::CloseHandle().
    if ( !bDeletedHandleIsValid && privateHandle.IsDeleted() )
    {
        RETAILMSG(ZONE_WARN, "WARNING: %s::ValidHandle(): resource for 0x%x has been deleted", s_pResourceManagerName, (UINT32)handle);
        return false;
    }
    
    // Check for "forged" handles, where the token does not match our private copy.
    if ( !privateHandle.IsDeleted() &&
          token != privateHandle.GetToken() )
    {
        return false;
    }
    
    return rval;
}



//----------------------------------------------------------------------------
// Public methods
//----------------------------------------------------------------------------

template<typename TYPE>
RESULT
ResourceManager<TYPE>::Init()
{
    // Override with your class-specific version, if you want
    // to do anything extra like initialize a cache.
    
    return S_OK;
}


template<typename TYPE>
RESULT      
ResourceManager<TYPE>::Add( IN const string& name, IN TYPE* pResource, INOUT Handle<TYPE>* pHandle )
{
    RESULT                  rval                = S_OK;
    UINT32                  index;
    Handle<TYPE>            handle;
    NameToHandleMapIterator pExistingResource;
    
    if ("" == name || !pResource /* NULL handle is OK; maybe caller doesn't need one */ )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: %s::Add( \"%s\", 0x%x ): invalid arg", s_pResourceManagerName, name.c_str(), (void*)pResource);
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    if (pResource)
    {
        // Convert name to lower case
        string name_lc = name;
        transform(name_lc.begin(), name_lc.end(), name_lc.begin(), tolower );

        //
        // If the Resource is already held, just return a new handle to it
        // Don't allow replacement of resources with an existing name
        //
        pExistingResource = m_nameToHandleMap.find( name_lc );
        if ( pExistingResource != m_nameToHandleMap.end() )
        {
            //
            // Return copy of existing handle.
            // This makes it cheap to test handles for validity - only one to compare against.
            // Also means when a handle is closed, all COPIES of it become invalid.
            //
            ////handle = pExistingResource->second;
            
            RETAILMSG(ZONE_ERROR, "ERROR: %s::Add( \"%s\", 0x%x ): name already exists", 
                      s_pResourceManagerName, name_lc.c_str(), (void*)pResource);
            rval = E_ACCESS_DENIED;
            DEBUGCHK(0);
            goto Exit;
        }
        else 
        {
            if ( !m_freeSlotList.empty() )
            {
                index = m_freeSlotList.back();
                m_freeSlotList.pop_back();
            }
            else
            {
                index  = m_resourceList.size();
            }
            
            //
            // Create a new, original, handle
            //
            handle = CreateHandle( index );
            
            // 
            // Save: the resource and name->handle mapping.
            //
            if ( index == m_resourceList.size() )
            {
                m_resourceList.push_back( pResource );
                m_validHandleList.push_back( handle );
            }
            else 
            {
                m_resourceList   [ index ] = pResource;
                m_validHandleList[ index ] = handle;
            }

            DEBUGMSG(ZONE_RESOURCE, "%s::Add( index: %d \"%s\" 0x%x )", s_pResourceManagerName, index, name_lc.c_str(), (UINT32)handle);
            m_nameToHandleMap.insert( typename std::pair< std::string,  Handle<TYPE> >( name_lc, handle  ) );
            m_handleToNameMap.insert( typename std::pair< Handle<TYPE>, std::string  >( handle,  name_lc ) );

            // Take a reference to all Objects we're holding.
            SAFE_ADDREF_TEMPLATE_TYPE(pResource);
        }

        //
        // Increase ref count if returning a handle to the caller.
        // Do this even when returning copy of handle for a pre-existing resource.
        //
        if (pHandle)
        {
            *pHandle = handle;
/////            SAFE_ADDREF_TEMPLATE_TYPE(pResource);
        }
    }
    
    
    
Exit:
    return rval;
}



template<typename TYPE>
RESULT
ResourceManager<TYPE>::Remove( IN TYPE* pResource, IN Handle<TYPE> hResource )
{
    RESULT  rval            = S_OK;
    string  resourceName    = "";
    TYPE*   pMyCopy         = NULL;
    Object* pObject         = NULL;
    
    HandleToNameMapIterator pHandleToName;
    NameToHandleMapIterator pNameToHandle;
 
     
    // If we weren't passed a corresponding handle for the resource, search for it.
    // This is the case when a resource wants to delete itself (it has a "this" pointer,
    // but not necessarily a handle to itself).
    if (hResource.IsNull())
    {
        ResourceListIterator ppResource;
        ppResource = find( m_resourceList.begin(), m_resourceList.end(), pResource );
        
        if (ppResource != m_resourceList.end())
        {
            // Is this iterator subtraction always valid?  
            // In our case m_resourceList is a Vector, so it should be safe.
            UINT32 index    = (UINT32)(ppResource - m_resourceList.begin());
            hResource       = m_validHandleList[ index ];

            DEBUGCHK( index == hResource.GetIndex() );
        }
    }


    if (!ValidHandle(hResource))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: %s::Remove( 0x%x ): invalid handle.", s_pResourceManagerName, (UINT32)hResource);
        DEBUGCHK(0);
        rval = E_BAD_HANDLE;
        goto Exit;
    }


    //
    // Validate the pointer/handle refers to a resource I'm managing.
    //
    pMyCopy = m_resourceList[ hResource.GetIndex() ];
    if (pMyCopy != pResource)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: %s::Remove( 0x%x ): pResource not found in m_resourceList.", s_pResourceManagerName, (UINT32)pResource);
        DEBUGCHK(0);
        rval = E_NOT_FOUND;
        goto Exit;
    }


    //
    // Remove the resource from our list.
    //
    m_resourceList[ hResource.GetIndex() ] = NULL;
    
/**
    // Mark the handle as "deleted".
    // From now on only ::CloseHandle() may be called on it.
    // The Resource will NOT be freed until ::CloseHandle() is called
    // for all open handles!!
    m_validHandleList[ hResource.GetIndex() ].Delete();
**/    

    //
    // To keep Add() quick, we reuse slots in m_resourceList/m_validHandleList.
    //
    m_freeSlotList.push_back( hResource.GetIndex() );
    

    //
    // Mark the handle as "deleted".
    // Do this immediately, to prevent any dereference to it that may occur during pObject->Release() (e.g. circular dereference between parent and child objects)
    //
    m_validHandleList[ hResource.GetIndex() ].Delete();

    
    //
    // If resource is an Object, decrease the ref count.
    //
//    Object* pObject = static_cast<Object*>(pResource);
    pObject = dynamic_cast<Object*>(pMyCopy);
    if (pObject)
    {
        try 
        {
            if (pObject->GetRefCount() > 1)
            {
                RETAILMSG(ZONE_ERROR, "ERROR: %s::Remove( %d: %s ): %d open handles; will not be deleted (yet)!",
                          s_pResourceManagerName, pObject->GetID(), pObject->GetName().c_str(), pObject->GetRefCount() );
            }

            pObject->Release();
        } 
        catch (...) 
        {
            RETAILMSG(ZONE_ERROR, "ERROR: %s::Remove( 0x%x ): attempt to free previously-deleted object.", s_pResourceManagerName, (UINT32)pResource);
            DEBUGCHK(0);
        }
    }


    //
    // TODO: we should associate a list of names and handles with every object,
    // so that removal can occur in constant time, without scanning the lists?
    //
     
    //
    // Remove the handle->name mapping.
    //
    pHandleToName = m_handleToNameMap.find( hResource );
    if (pHandleToName != m_handleToNameMap.end()) 
    {
        resourceName = pHandleToName->second;
        m_handleToNameMap.erase( pHandleToName );
    }
    else
    {
        RETAILMSG(ZONE_ERROR, "ERROR: %s::Remove( 0x%x ): not found in m_handleToNameMap", s_pResourceManagerName, (UINT32)hResource);
        rval = E_UNEXPECTED;
        DEBUGCHK(0);
    }
     


    //
    // Remove the name->handle mapping.
    //
    pNameToHandle = m_nameToHandleMap.find( resourceName );
    if (pNameToHandle != m_nameToHandleMap.end()) 
    {
        hResource = pNameToHandle->second;
        m_nameToHandleMap.erase( pNameToHandle );
    }
    else
    {
        RETAILMSG(ZONE_ERROR, "ERROR: %s::Remove( \"%s\" ): not found in m_nameToHandleMap", s_pResourceManagerName, resourceName.c_str());
        rval = E_UNEXPECTED;
        DEBUGCHK(0);
    }
    
    

    DEBUGMSG(ZONE_RESOURCE, "%s::Remove( slot: %d handle: 0x%x \"%s\" )", s_pResourceManagerName, hResource.GetIndex(), (UINT32)hResource, resourceName.c_str());


    DEBUGMSG(ZONE_RESOURCE, "%s::Remove(): returned slot %d to free list", 
             s_pResourceManagerName, hResource.GetIndex() );
    
    
    DEBUGMSG(ZONE_RESOURCE, "%s::ValidHandleList: %d ResourceList: %d NameToHandleMap: %d HandleToNameMap: %d",
            s_pResourceManagerName,
            m_validHandleList.size(),
            m_resourceList.size(),
            m_nameToHandleMap.size(),
            m_handleToNameMap.size());


Exit:
    return rval;
}




template<typename TYPE>
RESULT
ResourceManager<TYPE>::Remove( IN const Handle<TYPE> handle )
{
    RESULT rval = S_OK;
    
    
//    if ( !ValidHandle(handle, true) )
//    {
//        RETAILMSG(ZONE_ERROR, "ERROR: %s::Remove( %d ): invalid handle", s_pResourceManagerName, (UINT32)handle);
//        return E_BAD_HANDLE;
//    }
  

    TYPE* pResource = GetObjectPointer( handle );
    if (!pResource)
    {
        RETAILMSG(ZONE_ERROR, "%s::Remove(): handle 0x%x not found", s_pResourceManagerName, (UINT32)handle);
        rval = E_NOT_FOUND;
        goto Exit;
    }

    DEBUGCHK( SUCCEEDED(Remove( pResource )) );

Exit:
    /*
    DEBUGCHK( m_validHandleList.size() == m_nameToHandleMap.size() &&
            m_validHandleList.size() == m_handleToNameMap.size() &&
            m_validHandleList.size() == m_resourceList.size()       );
    */
    
    DEBUGCHK( m_validHandleList.size() == m_resourceList.size() );
    
    return rval;
}



template<typename TYPE>
RESULT    
ResourceManager<TYPE>::Get( IN const string& name, INOUT Handle<TYPE>* pHandle )
{
    DEBUGMSG(ZONE_RESOURCE, "%s::Get( \"%s\" )", s_pResourceManagerName, name.c_str());
    
    RESULT                  rval    = S_OK;
    string                  name_lc = name;
    Handle<TYPE>            handle;
    NameToHandleMapIterator i;

    if ("" == name)
    {
        rval = E_NOT_FOUND;
        goto Exit;
    }

    if (!pHandle)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: %s::Get( \"%s\"m 0x%x ): NULL pointer", s_pResourceManagerName, name.c_str(), pHandle);
        rval = E_NULL_POINTER;
        goto Exit;
    }
    *pHandle = NULL_HANDLE;

    
    // Convert name to lower case
    transform(name_lc.begin(), name_lc.end(), name_lc.begin(), tolower );
    

    // Using an iterator with find() has a benefit:
    // if the name isn't found, no blank element will be
    // created for that name (as would happen with vector[pName])
    i = m_nameToHandleMap.find(name_lc);
    if (i == m_nameToHandleMap.end()) 
    {
        RETAILMSG(ZONE_VERBOSE, "ERROR: %s::Get( \"%s\" ) is not a valid resource name", s_pResourceManagerName, name_lc.c_str());
        rval = E_NOT_FOUND;
        goto Exit;
    } 
    else 
    {
        //
        // Return the existing handle.
        // NOTE: there is NEVER more than ONE handle point to an object.
        // This makes it very fast to test a handle for validity/forgery (compare it to our private copy).
        // However this also means that if a handle is closed, all copies of that handle are invalidated.
        // In other words, don't close a handle unless the resource it points to is being deleted.
        //
        handle = i->second;     // TODO: what if handle was previously deleted?  Will it be found here?  Need to undelete?
        
        if ( !ValidHandle(handle) )
        {
//            if (handle.IsDeleted())
//            {
//                // Create a new handle for the resource
//                Handle newHandle;
//                newHandle.Init( handle.GetIndex() );
//                handle = newHandle;
//            }
//            else 
//            {
                std::string name = i->first;
                RETAILMSG(ZONE_ERROR, "ERROR: %s::Get( \"%s\" ): found invalid handle in m_nameToHandleMap", s_pResourceManagerName, name_lc.c_str());
                rval = E_UNEXPECTED;
                DEBUGCHK(0);
                goto Exit;
//            }
        }
                
        //
        // Increase ref count on the corresponding resource.
        //
        TYPE*   pResource   = m_resourceList[ handle.GetIndex() ];
        SAFE_ADDREF_TEMPLATE_TYPE(pResource);
    }
    
    *pHandle = handle;
    
Exit:
    return rval;
}



template<typename TYPE>
RESULT    
ResourceManager<TYPE>::GetCopy( IN const string& name, INOUT Handle<TYPE>* pHandle )
{
    RESULT          rval = S_OK;
    Handle<TYPE>    hResourceTemplate;
    TYPE*           pResourceTemplate;
    Handle<TYPE>    hResourceInstance;
    TYPE*           pResourceInstance;
    
    DEBUGMSG(ZONE_RESOURCE, "%s::GetCopy( \"%s\" )", s_pResourceManagerName, name.c_str());
    
    if (!pHandle)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: %s::GetCopy( \"%s\"m 0x%x ): NULL pointer", s_pResourceManagerName, name.c_str(), pHandle);
        rval = E_NULL_POINTER;
        goto Exit;
    }
    *pHandle = NULL_HANDLE;


    //
    // Get the original resource to use as a template (will not AddRef)
    //
    CHR(ResourceManager<TYPE>::Get( name, &hResourceTemplate ));
    pResourceTemplate = GetObjectPointer( hResourceTemplate );
    DEBUGCHK(pResourceTemplate);
    
    //
    // Clone the resource (template) to create a new instance
    //
    // TODO: what if the resource doesn't implement Clone()?
    pResourceInstance = dynamic_cast<TYPE*>(pResourceTemplate->Clone());
    
    
    // Add the copy to our resource list
    CHR(Add(pResourceInstance->GetName(), pResourceInstance, &hResourceInstance));
    
    // Release our reference to the resource COPY.
    // (taken when we added it to the ResourceList).
    // We want it to go away when the caller closes their handle!
    // Only resource originals/templates should remain until shutdown.
    /////SAFE_RELEASE(pResourceInstance);
    
    // return the new handle
    *pHandle = hResourceInstance;
    CHR(Release( hResourceTemplate ));
    
Exit:
    return rval;
}



template<typename TYPE>
RESULT
ResourceManager<TYPE>::Release( IN Handle<TYPE> handle )
{
    RESULT  rval      = S_OK;
    TYPE   *pResource = NULL;
    
    if (handle.IsNull())
    {
        goto Exit;
    }
    
    // Is the handle valid?  It's legal to call Release() for
    // Resources that were previously ::Removed()'d
    if (!handle.IsNull() && !ValidHandle(handle, true /* bDeletedHandleIsValid */))
    {
        RETAILMSG(ZONE_ERROR | ZONE_VERBOSE, "ERROR: %s::Release(): handle 0x%x is not valid.", s_pResourceManagerName, (UINT32)handle);
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    
    pResource = m_resourceList[ handle.GetIndex() ];
    
    if (!pResource)
    {
        RETAILMSG(ZONE_ERROR | ZONE_VERBOSE, "ERROR: %s::Release(): handle 0x%x is not valid.", s_pResourceManagerName, (UINT32)handle);
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    CHR(Release( pResource, handle ));
    
Exit:
    return rval;
}



template<typename TYPE>
RESULT
ResourceManager<TYPE>::AddRef( IN Handle<TYPE> handle )
{
    RESULT  rval        = S_OK;
    TYPE   *pResource   = NULL;
    
    if (!ValidHandle(handle))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: %s::AddRef(): handle 0x%x is not valid.", s_pResourceManagerName, (UINT32)handle);
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    
    pResource = m_resourceList[ handle.GetIndex() ];
    
    if (!pResource)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: %s::AddRef(): handle 0x%x is not valid.", s_pResourceManagerName);
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    
    
    //
    // Increase ref count on the corresponding resource.
    //
    {
        //Object* pObject = dynamic_cast<Object*>(pResource);
        IObject* pObject = dynamic_cast<IObject*>(pResource);
        if (pObject)
        {
            pObject->AddRef();
        }
    }
    
Exit:
    return rval;
}



template<typename TYPE>
RESULT
ResourceManager<TYPE>::Release( IN TYPE* pResource, IN Handle<TYPE> handle )
{
    RESULT rval = S_OK;
    
    CPREx(pResource, E_NULL_POINTER);

    //
    // Decrease ref count on the corresponding resource.
    //
    {
        //Object* pObject = dynamic_cast<Object*>(pResource);
        IObject* pObject = dynamic_cast<IObject*>(pResource);
        if (pObject)
        {
            UINT32 refcount = pObject->GetRefCount();
            if (0 == refcount)
            {
                RETAILMSG(ZONE_ERROR, "ERROR: %s::Release( 0x%x ): DOUBLE RELEASE.", s_pResourceManagerName, (UINT32) handle);
                DEBUGCHK(0);
            }
            
            if (1 == refcount)
            {
                Remove( pResource, handle );
            }
            else 
            {
                pObject->Release();
            }

        }
    }

Exit:     
    return rval;
}


/*
template<typename TYPE>
RESULT
ResourceManager<TYPE>::CloneHandle( IN Handle<TYPE> handle, INOUT Handle<TYPE>* pNewHandle )
{
    RESULT  rval        = S_OK;
    TYPE   *pResource   = NULL;
    
    if (!pNewHandle)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: %s::CloneHandle(): NULL ptr.", s_pResourceManagerName);
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    if (!ValidHandle(handle))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: %s::CloneHandle(): handle 0x%x is not valid.", s_pResourceManagerName, (UINT32)handle);
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    
    pResource = m_resourceList[ handle.GetIndex() ];
    
    if (!pResource)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: %s::CloneHandle(): handle 0x%x is not valid.", s_pResourceManagerName);
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    
    
    //
    // Increase ref count on the corresponding resource.
    //
    {
        Object* pObject = dynamic_cast<Object*>(pResource);
        if (pObject)
        {
            pObject->AddRef();
        }
    }
    
    // Return the duplicated handle.
    // TODO: we may want to simply get a NEW handle to the resource.
    // Duplication saves space in the lookup table, but may introduce bugs?
    *pNewHandle = handle;
    
Exit:
    return rval;
}
*/


template<typename TYPE>
RESULT
ResourceManager<TYPE>::GetName( IN Handle<TYPE> handle, INOUT string* pName ) const
{
    RESULT  rval      = S_OK;
    TYPE   *pResource = NULL;
    
    if (!pName)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: %s::GetName(): NULL pointer", s_pResourceManagerName);
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    if (!ValidHandle(handle))
    {
        if (handle.IsNull())
        {
            *pName = "NULL";
        }
        else
        {
            RETAILMSG(ZONE_ERROR, "ERROR: %s::GetName(): handle 0x%x is not valid.", s_pResourceManagerName, (UINT32)handle);
            *pName = "BAD HANDLE";
        }
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    

    pResource = m_resourceList[ handle.GetIndex() ];
    if (!pResource)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: %s::GetName(): handle 0x%x is not valid.", s_pResourceManagerName);
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    
    
    {
        //Object* pObject = dynamic_cast<Object*>(pResource);
        IObject* pObject = dynamic_cast<IObject*>(pResource);
        if (pObject)
        {
            *pName = pObject->GetName();
        }
    }
    
    
Exit:
    return rval;
}



template<typename TYPE>
RESULT
ResourceManager<TYPE>::GetObjectID( IN Handle<TYPE> handle, INOUT OBJECT_ID* pObjectID ) const
{
    RESULT  rval      = S_OK;
    TYPE   *pResource = NULL;
    
    if (!pObjectID)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: %s::GetObjectID(): NULL pointer", s_pResourceManagerName);
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    if (!ValidHandle(handle))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: %s::GetObjectID(): handle 0x%x is not valid.", s_pResourceManagerName, (UINT32)handle);
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    

    pResource = m_resourceList[ handle.GetIndex() ];
    if (!pResource)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: %s::GetObjectID(): handle 0x%x is not valid.", s_pResourceManagerName);
        rval = E_BAD_HANDLE;
        goto Exit;
    }
    
    
    {
        //Object* pObject = dynamic_cast<Object*>(pResource);
        IObject* pObject = dynamic_cast<IObject*>(pResource);
        if (pObject)
        {
            *pObjectID = pObject->GetID();
        }
    }
    
    
Exit:
    return rval;
}



template<typename TYPE>
UINT32
ResourceManager<TYPE>::GetRefCount( IN Handle<TYPE> handle ) const
{
    UINT32  rval      = 0;
    TYPE   *pResource = NULL;
    
    
    if (!ValidHandle(handle))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: %s::GetRefCount(): handle 0x%x is not valid.", s_pResourceManagerName, (UINT32)handle);
        goto Exit;
    }
    

    pResource = m_resourceList[ handle.GetIndex() ];
    if (!pResource)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: %s::GetRefCount(): handle 0x%x is not valid.", s_pResourceManagerName);
        goto Exit;
    }
    
    
    {
        //Object* pObject = dynamic_cast<Object*>(pResource);
        IObject* pObject = dynamic_cast<IObject*>(pResource);
        if (pObject)
        {
            rval = pObject->GetRefCount();
        }
    }

    
Exit:
    return rval;
}




template<typename TYPE>
IProperty*
ResourceManager<TYPE>::GetProperty( IN Handle<TYPE> handle, IN const string& propertyName ) const
{
    IProperty*  rval      = NULL;
    TYPE*       pResource = NULL;
    
    if (!ValidHandle(handle))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: %s::GetProperty(): handle 0x%x is not valid.", s_pResourceManagerName, (UINT32)handle);
        goto Exit;
    }
    

    pResource = m_resourceList[ handle.GetIndex() ];
    if (!pResource)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: %s::GetProperty(): handle 0x%x is not valid.", s_pResourceManagerName);
        goto Exit;
    }
    
    
    {
/*
        IObject* pIObject = dynamic_cast<IObject*>(pResource);
        if (pIObject)
        {
            rval = pIObject->GetProperty( propertyName );
        }
*/
        //Object* pObject = dynamic_cast<Object*>(pResource);
        IObject* pObject = dynamic_cast<IObject*>(pResource);
        if (pObject)
        {
            rval = pObject->GetProperty( propertyName );
        }
    }
    
    
Exit:
    return rval;
}




template<typename TYPE>
TYPE*    
ResourceManager<TYPE>::GetObjectPointer( IN Handle<TYPE> handle, bool addref ) const
{
    TYPE   *pResource = NULL;
    
    if (!ValidHandle(handle))
    {
//        if (!handle.IsNull())
//        {
//            RETAILMSG(ZONE_ERROR, "ERROR: %s::GetObjectPointer(): handle 0x%x is invalid.", s_pResourceManagerName, (UINT32)handle);
//        }
            
        goto Exit;
    }
    

    pResource = m_resourceList[ handle.GetIndex() ];
    if (!pResource)
    {
        RETAILMSG(ZONE_ERROR | ZONE_VERBOSE, "ERROR: %s::GetObjectPointer(): handle 0x%x is invalid.", s_pResourceManagerName);
        goto Exit;
    }
    
    
    //
    // Increase ref count on the resource.
    //
    if (addref)
    {
        SAFE_ADDREF_TEMPLATE_TYPE(pResource);
    }
    
    
Exit:
    return pResource;
}



template<typename TYPE>
UINT32    
ResourceManager<TYPE>::Count()
{
    return m_resourceList.size() - m_freeSlotList.size();
}



template<typename TYPE>
RESULT    
ResourceManager<TYPE>::Shutdown()
{
    DEBUGMSG( ZONE_RESOURCE, "%s::Shutdown()", s_pResourceManagerName);
    
    RESULT rval         = S_OK;


    /*
     UINT32 numResources = 0;
     
     ResourceMap::iterator ppResource;
     for (ppResource = m_resourceMap.begin(); ppResource != m_resourceMap.end();)
     {
     string    ResourceName   = ppResource->first;
     TYPE*     pResource      = ppResource->second;
     DEBUGMSG( RESOURCE | VERBOSE, "freeing Resource [%s]\n", ResourceName.c_str());
     
     // Remove it from the map
     ppResource = m_ResourceMap.erase( ppResource );
     delete pResource;
     
     numResources++;
     }
     
     */

Exit:
    return rval;
}



template<typename TYPE>
void
ResourceManager<TYPE>::Print()
{
    RETAILMSG(ZONE_INFO, "%s: %d resources (%d handles) %d free slots",
              s_pResourceManagerName, m_resourceList.size(), m_validHandleList.size(), m_freeSlotList.size());

    RETAILMSG(ZONE_INFO, "NameToHandleMap: %d", m_nameToHandleMap.size());

    RETAILMSG(ZONE_INFO, "HandleToNameMap: %d", m_handleToNameMap.size());
 
    
    RETAILMSG(ZONE_INFO, "Resources:");
    NameToHandleMapIterator ppHandle;
    for (ppHandle = m_nameToHandleMap.begin(); ppHandle != m_nameToHandleMap.end(); ++ppHandle)
    {
        string          resourceName    = ppHandle->first;
        Handle<TYPE>    handle          = ppHandle->second;

        TYPE*           pResource       = m_resourceList[ handle.GetIndex() ];
        IObject*        pObject         = static_cast<IObject*>(pResource);

        UINT32 refcount = 0;
        if (pObject)
        {
            refcount = pObject->GetRefCount();
        }
        
        RETAILMSG(ZONE_INFO, "handle: 0x%08x pResource: 0x%x refcount: %3d \"%s\"", 
                  (UINT32)handle,
                  pResource,
                  refcount,
                  resourceName.c_str()); 
    }
    RETAILMSG(ZONE_INFO, "\n\n");
}


} // END namespace Z

