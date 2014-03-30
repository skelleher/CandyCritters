/*
 * Object is the abstract base class for all entities that require lifetime management and composition.
 * Concrete subclasses of Object include Texture, Sprite, GameObject, etc.
 * A GameObject is composed of one or more Object subclasses.
 *
 */

#pragma once
#include "Types.hpp"

// TEST TEST:
#include "msg.hpp"

#include <string>
using std::string;


namespace Z
{

// Forward declaration
class IProperty;

    
class IObject
{
public:
    virtual UINT32          AddRef      ( )                         = 0;
    virtual UINT32          Release     ( )                         = 0;
    virtual OBJECT_ID       GetID       ( ) const                   = 0;
    virtual UINT32          GetRefCount ( ) const                   = 0;
    virtual const string&   GetName     ( ) const                   = 0;
    virtual IObject*        Clone       ( ) const                   = 0;

    virtual IProperty*      GetProperty ( const string& name ) const = 0;

/*    
    virtual RESULT          AddListener    ( IObject* pListener, MSG_Name msg ) = 0;
    virtual RESULT          RemoveListener ( IObject* pListener, MSG_Name msg ) = 0;
    virtual RESULT          HandleMessage  ( MSG_Name msg, void* pData )        = 0;
*/

    virtual ~IObject()  {};
};


//
// By default Objects are created with a ref count of zero
// and must be retained by their creator.
// Subclasses should not change this behavior.
//
class Object : virtual public IObject
{
public:
    // IObject
    virtual UINT32          AddRef();
    virtual UINT32          Release();
    virtual OBJECT_ID       GetID()                 const;
    virtual UINT32          GetRefCount()           const;
    virtual const string&   GetName()               const;
    virtual IObject*        Clone()                 const;

    virtual IProperty*      GetProperty( const string& name ) const;

/*
    virtual RESULT          AddListener    ( IObject* pListener, MSG_Name msg );
    virtual RESULT          RemoveListener ( IObject* pListener, MSG_Name msg );
    virtual RESULT          HandleMessage  ( MSG_Name msg, void* pData );
*/

public:
    // FOR TESTING:
    static Object* Create();
    bool operator == ( Object& rhs ) const;


public:
    static  UINT32          Count()     { return s_numObjects; }

    
protected:
    // Disallow construction of Objects.  Can only construct subclasses.
    Object();
    Object(const Object& rhs);
    Object& operator=(const Object& rhs);
    virtual ~Object();
    
protected:
    UINT32  m_ID;
    UINT32  m_refCount;
    string  m_name;     // TODO: char[64]?  string hash?
    
    // TODO: msg queue, created on demand in AddListener().

protected:
    static  OBJECT_ID   s_nextAvailableID;
    static  UINT32      s_numObjects;
};

const OBJECT_ID INVALID_OBJECT_ID = 0;
const OBJECT_ID SYSTEM_OBJECT_ID  = 1;


} // END namespace Z
