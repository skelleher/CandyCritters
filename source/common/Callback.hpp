#pragma once

/*
 *  Callback.hpp
 *  Critters
 *
 *  Created by Sean Kelleher on 2/22/11.
 *  Copyright 2011 Sean Kelleher. All rights reserved.
 *
 */


namespace Z
{


//
// Wraps a function - can be static, or a method on an object.
//
// This is the abstract base class for Callbacks, so that 
// we can declare Callback pointers.
//
class ICallback
{
public:
    virtual void        Invoke      ( ) const       = 0;
    virtual bool        IsNull      ( ) const       = 0;
    virtual ICallback*  Clone       ( ) const       = 0;
    
    virtual ~ICallback() {};
};


//
// Callback wraps a function or static class method.  
// Function must not be an object method.
// Function must return void and take a void* context.
//
class Callback : virtual public ICallback
{
    typedef void (*FUNCTION)(void*);

public:
    Callback( FUNCTION function = NULL, void* context = NULL )
    { 
        m_pFunction = function;
        m_pContext  = context;
        
        DEBUGMSG(ZONE_OBJECT, "Callback( 0x%x -> 0x%x )", this, m_pFunction);
    }
    
    virtual ~Callback()                 
    { 
        DEBUGMSG(ZONE_OBJECT, "\t~Callback( 0x%x -> 0x%x )", this, m_pFunction);

        m_pFunction = NULL;
        m_pContext  = NULL;
    }


    virtual ICallback*  Clone() const
    {
        Callback* pClone = new Callback( m_pFunction, m_pContext );
        
        return pClone;
    }
    

    virtual bool IsNull() const
    { 
        return m_pFunction ? false : true; 
    }

    virtual void Invoke() const
    { 
        if (!m_pFunction)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: Callback::Invoke( 0x%x ): NULL m_pFunction.", this);
            return;
        }
    
        m_pFunction( m_pContext );        
    }
    

protected:
    FUNCTION m_pFunction;
    void*    m_pContext;
};



//
// Callback wraps an object method.
// Method must return void and take a void* context.
//
template<typename TYPE>
class ObjectCallback : virtual public ICallback
{
    typedef void    (TYPE::*OBJECT_METHOD)(void*);


public:
    ObjectCallback<TYPE>( TYPE* pObject, OBJECT_METHOD method, void* context = NULL )
    {
        m_pObject  = pObject;
        m_pMethod  = method;
        m_pContext = context;
        
        // Retain the object, if it supports ref-counting.
        Object* pRefCountObject = dynamic_cast<Object*> (pObject);
        if (pRefCountObject)
        {
            pRefCountObject->AddRef();
        }
    }
    
    
    ObjectCallback<TYPE>( const ICallback& rhs )
    {
        const ObjectCallback<TYPE>* pCallback = dynamic_cast< const ObjectCallback<TYPE>* > (&rhs);
        
        if (pCallback)
        {
            m_pMethod       = pCallback->m_pMethod;
            m_pContext      = pCallback->m_pContext;

            m_pObject       = pCallback->m_pObject;

            // Retain the object, if it supports ref-counting.
            Object* pRefCountObject = dynamic_cast<Object*> (m_pObject);
            if (pRefCountObject)
            {
                pRefCountObject->AddRef();
            }
        }
        else 
        {
            DEBUGCHK(0);
        }
    }



    virtual ~ObjectCallback<TYPE>()
    {
        m_pMethod  = NULL;
        m_pContext = NULL;

        if (!m_pObject)
            return;

        // Release the object, if it supports ref-counting.
        Object* pRefCountObject = dynamic_cast<Object*> (m_pObject);
        if (pRefCountObject)
        {
            pRefCountObject->Release();
        }

        m_pObject  = NULL;
    }



    virtual ICallback* Clone()
    {
        ObjectCallback* pClone = new ObjectCallback( m_pObject, m_pMethod, m_pContext );
        
        return pClone;
    }



    virtual bool IsNull()
    {
        return (m_pObject && m_pMethod) ? false : true;
    }


    virtual void Invoke()
    {
        #ifndef CALL_METHOD
        #define CALL_METHOD(pObject, pMethod)  ((*pObject).*(pMethod)) 
        #endif

        if (!m_pObject || !m_pMethod)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: ObjectCallback::Invoke( 0x%x ): NULL object or method.", this);
            return;
        }
            
        CALL_METHOD(m_pObject, m_pMethod)(m_pContext);
    }
    

protected:
    TYPE*           m_pObject;
    OBJECT_METHOD   m_pMethod;
    void*           m_pContext;
};



} // END namespace Z

