/*
 *  Object.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 9/4/10.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include "Object.hpp"
#include "Macros.hpp"
#include "Property.hpp"


namespace Z
{



//
// Static Data
//
OBJECT_ID Object::s_nextAvailableID   = SYSTEM_OBJECT_ID + 1;
UINT32    Object::s_numObjects        = 0;


// TEST TEST:
Object*
Object::Create()
{
    return new Object();
}


bool 
Object::operator==( Object& rhs ) const
{  
    return ( m_ID == rhs.GetID() );  
}




//
// Methods
//
Object::Object()
{
    m_ID        =  ATOMIC_INCREMENT(s_nextAvailableID);
    m_refCount  = 0;
#ifdef DEBUG
    m_name      = "Object";
#endif
    
    ATOMIC_INCREMENT(s_numObjects);
    
    DEBUGMSG(ZONE_OBJECT, "\tnew Object( %4d )", m_ID);
}


Object::Object( const Object& rhs )
{
    m_ID        =  ATOMIC_INCREMENT(s_nextAvailableID);
    m_refCount  = 0;
#ifdef DEBUG
    m_name      = rhs.m_name + "CLONE";
#endif
    
    ATOMIC_INCREMENT(s_numObjects);

    DEBUGMSG(ZONE_OBJECT, "\tObject( %4d \"%s\" is clone of %4d \"%s\" )", 
        m_ID, 
        m_name.c_str(),
        rhs.GetID(),
        rhs.GetName().c_str());
}


Object& 
Object::operator=( const Object& rhs )
{
    // Disallow copying of Objects, unless the subclass implements it.
    DEBUGCHK(0);
    
    return *this;
}



Object::~Object()
{
    if (0 != m_refCount)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Object %d \"%s\" deleted with %d dangling refs!", m_ID, m_name.c_str(), m_refCount);
        DEBUGCHK(0);
    }
    
    ATOMIC_DECREMENT(s_numObjects);

    DEBUGMSG(ZONE_OBJECT, "\t~Object( %4d, \"%s\" )", m_ID, m_name.c_str());
}


IObject*
Object::Clone() const
{
    RETAILMSG(ZONE_ERROR, "Attempt to clone Object %d \"%s\"; which did not implement virtual Object* Clone()", m_ID, m_name.c_str());
    DEBUGCHK(0);
    
    return NULL;
}



//
// IObject
//

UINT32
Object::AddRef()
{
    return ATOMIC_INCREMENT(m_refCount);
}



UINT32
Object::Release()
{
    if (0 == ATOMIC_DECREMENT(m_refCount))
    {
        DEBUGMSG(ZONE_OBJECT | ZONE_VERBOSE, "Object [%4d, \"%s\"] released; deleting", m_ID, m_name.c_str());
        delete this;
        return 0;
    }
    
    return m_refCount;
}



OBJECT_ID
Object::GetID() const
{
    return m_ID;
}


UINT32
Object::GetRefCount() const
{
    return m_refCount;
}


const string&
Object::GetName() const
{
    return m_name;
}



IProperty*
Object::GetProperty( const string& propertyName ) const
{
    // For now Object exposes no Properties.
    // Subclasses may override.

    RETAILMSG(ZONE_ERROR, "ERROR: Object[%4d] named \"%s\" has no Property \"%s\"", m_ID, m_name.c_str(), propertyName.c_str() );
    RETAILMSG(ZONE_ERROR, "Did you forget to implement ::GetProperty() in a subclass?");
    DEBUGCHK(0);
    
    return NULL;
}



/*

//
// PROBLEM: invoking pointers is just too dangerous.
// We would probably need an ObjectManager / HObject to protect
// all access, which is a whole can of worms.
// Even trapping null pointer deref isn't enough, since we could
// invoke a garbage pointer.
//

RESULT
Object::AddListener    ( IObject* pListener, MSG_Name msg )
{
    RESULT rval = S_OK;
    
Exit:
    return rval;
}



RESULT
Object::RemoveListener ( IObject* pListener, MSG_Name msg )
{
    RESULT rval = S_OK;
    
Exit:
    return rval;
}



RESULT
Object::HandleMessage  ( MSG_Name msg, void* pData )
{
    RESULT rval = S_OK;
    
Exit:
    return rval;
}
*/


} // END namespace Z
