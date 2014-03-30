#pragma once

#include "Log.hpp"
#include "Object.hpp"
#include "Macros.hpp"
#include "Color.hpp"

#include <string>
#include <map>
using std::string;
using std::map;


namespace Z
{

//
// Legal types for data that may be encapsulated in a Property.
//
typedef enum
{
    PROPERTY_UNKNOWN = 0,
    PROPERTY_FLOAT,
    PROPERTY_UINT32,
    PROPERTY_BOOL,
    PROPERTY_VEC2,
    PROPERTY_VEC3,
    PROPERTY_VEC4,
    PROPERTY_IVEC2,
    PROPERTY_IVEC3,
    PROPERTY_IVEC4,
    PROPERTY_COLOR,
} PropertyType;


//
// Given a name, return the property type.
// Name is not case-sensitive.
// E.g. "float" => PROPERTY_FLOAT.
//
static PropertyType PropertyTypeFromName( IN const string& name );




//
// Wraps a piece of data (storage, setter, and getter) into a unit that can be read / written.
// Useful for binding Animations to object properties, like GameObject::GetScale() / GameObject::SetScale().
//
// This is the abstract base class for Property<type>, which we need so
// we can declare arrays of and pointers to Properties.
//
class IProperty
{
public:
    virtual PropertyType    GetType     ( ) const               = 0;

    virtual float           GetFloat    ( ) const               = 0;
    virtual UINT32          GetInteger  ( ) const               = 0;
    virtual bool            GetBool     ( ) const               = 0;
    virtual vec2            GetVec2     ( ) const               = 0;
    virtual vec3            GetVec3     ( ) const               = 0;
    virtual vec4            GetVec4     ( ) const               = 0;
    virtual ivec2           GetIVec2    ( ) const               = 0;
    virtual ivec3           GetIVec3    ( ) const               = 0;
    virtual ivec4           GetIVec4    ( ) const               = 0;
    virtual Color           GetColor    ( ) const               = 0;

    virtual void            SetFloat    ( float  val )          = 0;
    virtual void            SetInteger  ( UINT32 val )          = 0;
    virtual void            SetBool     ( bool   val )          = 0;
    virtual void            SetVec2     ( const vec2&  val )    = 0;
    virtual void            SetVec3     ( const vec3&  val )    = 0;
    virtual void            SetVec4     ( const vec4&  val )    = 0;
    virtual void            SetIVec2    ( const ivec2& val )    = 0;
    virtual void            SetIVec3    ( const ivec3& val )    = 0;
    virtual void            SetIVec4    ( const ivec4& val )    = 0;
    virtual void            SetColor    ( const Color& val )    = 0;
    
    virtual bool            IsNull      ( ) const               = 0;
    virtual IProperty*      Clone       ( ) const               = 0;
    virtual void            SetObject   ( Object* pObject )     = 0;    // HACK: for creating a Property from a template, then binding it to an object instance.  Make FRIEND of PropertySet.
    
    virtual ~IProperty() {};
};



template<typename TYPE>
class Property : virtual public IProperty
{
public:
    // Function pointers for Class::GetFoo() and Class::SetFoo()
    typedef void*   (TYPE::*OBJECT_GET_METHOD)();
    typedef void    (TYPE::*OBJECT_SET_METHOD)(void*);

public:
    // TODO: deprecate the default ctor?  Only useful in creating a NULL Property
    Property<TYPE>() { m_pObject = NULL; m_pGetMethod = NULL; m_pSetMethod = NULL; }
    Property<TYPE>( TYPE* pObject, PropertyType type, OBJECT_GET_METHOD pGetter, OBJECT_SET_METHOD pSetter );
    virtual ~Property<TYPE>();
    virtual IProperty* Clone() const;

    virtual PropertyType GetType ( ) const;


    virtual void    SetObject   ( Object* pObject );


    // Get and Set the value this Property encapsulates.
    virtual float   GetFloat    ( ) const;
    virtual UINT32  GetInteger  ( ) const;
    virtual bool    GetBool     ( ) const;
    virtual vec2    GetVec2     ( ) const;
    virtual vec3    GetVec3     ( ) const;
    virtual vec4    GetVec4     ( ) const;
    virtual ivec2   GetIVec2    ( ) const;
    virtual ivec3   GetIVec3    ( ) const;
    virtual ivec4   GetIVec4    ( ) const;
    virtual Color   GetColor    ( ) const;

    virtual void    SetFloat    ( float  val );
    virtual void    SetInteger  ( UINT32 val );
    virtual void    SetBool     ( bool   val );
    virtual void    SetVec2     ( const vec2&  val );
    virtual void    SetVec3     ( const vec3&  val );
    virtual void    SetVec4     ( const vec4&  val );
    virtual void    SetIVec2    ( const ivec2& val );
    virtual void    SetIVec3    ( const ivec3& val );
    virtual void    SetIVec4    ( const ivec4& val );
    virtual void    SetColor    ( const Color& val );

    virtual bool    IsNull      ( ) const;

//    static  Property<TYPE>   NullProperty();

protected:
    Property<TYPE>( const Property<TYPE>& rhs );
    Property<TYPE>& operator=( const Property<TYPE>& rhs );


protected:
    TYPE*           m_pObject;
    PropertyType    m_propertyType;

    typedef float   (TYPE::*OBJECT_GET_FLOAT_METHOD)();
    typedef void    (TYPE::*OBJECT_SET_FLOAT_METHOD)(float);

    typedef UINT32  (TYPE::*OBJECT_GET_INTEGER_METHOD)();
    typedef void    (TYPE::*OBJECT_SET_INTEGER_METHOD)(UINT32);

    typedef bool    (TYPE::*OBJECT_GET_BOOL_METHOD)();
    typedef void    (TYPE::*OBJECT_SET_BOOL_METHOD)(bool);

    typedef vec2    (TYPE::*OBJECT_GET_VEC2_METHOD)();
    typedef void    (TYPE::*OBJECT_SET_VEC2_METHOD)(const vec2&);

    typedef vec3    (TYPE::*OBJECT_GET_VEC3_METHOD)();
    typedef void    (TYPE::*OBJECT_SET_VEC3_METHOD)(const vec3&);

    typedef vec4    (TYPE::*OBJECT_GET_VEC4_METHOD)();
    typedef void    (TYPE::*OBJECT_SET_VEC4_METHOD)(const vec4&);

    typedef ivec2   (TYPE::*OBJECT_GET_IVEC2_METHOD)();
    typedef void    (TYPE::*OBJECT_SET_IVEC2_METHOD)(const ivec2&);

    typedef ivec3   (TYPE::*OBJECT_GET_IVEC3_METHOD)();
    typedef void    (TYPE::*OBJECT_SET_IVEC3_METHOD)(const ivec3&);

    typedef ivec4   (TYPE::*OBJECT_GET_IVEC4_METHOD)();
    typedef void    (TYPE::*OBJECT_SET_IVEC4_METHOD)(const ivec4&);

    typedef Color   (TYPE::*OBJECT_GET_COLOR_METHOD)();
    typedef void    (TYPE::*OBJECT_SET_COLOR_METHOD)(const Color&);
    
    union 
    {
        OBJECT_GET_METHOD           m_pGetMethod;
        OBJECT_GET_FLOAT_METHOD     m_pGetFloatMethod;
        OBJECT_GET_INTEGER_METHOD   m_pGetIntegerMethod;
        OBJECT_GET_BOOL_METHOD      m_pGetBoolMethod;
        OBJECT_GET_VEC2_METHOD      m_pGetVec2Method;
        OBJECT_GET_VEC3_METHOD      m_pGetVec3Method;
        OBJECT_GET_VEC4_METHOD      m_pGetVec4Method;
        OBJECT_GET_IVEC2_METHOD     m_pGetIVec2Method;
        OBJECT_GET_IVEC3_METHOD     m_pGetIVec3Method;
        OBJECT_GET_IVEC4_METHOD     m_pGetIVec4Method;
        OBJECT_GET_COLOR_METHOD     m_pGetColorMethod;
    };

    union
    {
        OBJECT_SET_METHOD           m_pSetMethod;
        OBJECT_SET_FLOAT_METHOD     m_pSetFloatMethod;
        OBJECT_SET_INTEGER_METHOD   m_pSetIntegerMethod;
        OBJECT_SET_BOOL_METHOD      m_pSetBoolMethod;
        OBJECT_SET_VEC2_METHOD      m_pSetVec2Method;
        OBJECT_SET_VEC3_METHOD      m_pSetVec3Method;
        OBJECT_SET_VEC4_METHOD      m_pSetVec4Method;
        OBJECT_SET_IVEC2_METHOD     m_pSetIVec2Method;
        OBJECT_SET_IVEC3_METHOD     m_pSetIVec3Method;
        OBJECT_SET_IVEC4_METHOD     m_pSetIVec4Method;
        OBJECT_SET_COLOR_METHOD     m_pSetColorMethod;
    };
};


//-----------------------------------------------------------------------------
// Global functions
//-----------------------------------------------------------------------------
#pragma mark -
#pragma mark Global Functions


struct PROPERTY_NAME_TO_TYPE
{
    const char*   name;
    PropertyType  type;
};

static PROPERTY_NAME_TO_TYPE nameToType[] =
{
    "float",    PROPERTY_FLOAT,
    "uint32",   PROPERTY_UINT32,
    "bool",     PROPERTY_BOOL,
    "vec2",     PROPERTY_VEC2,
    "vec3",     PROPERTY_VEC3,
    "vec4",     PROPERTY_VEC4,
    "ivec2",    PROPERTY_IVEC2,
    "ivec3",    PROPERTY_IVEC3,
    "ivec4",    PROPERTY_IVEC4,
    "color",    PROPERTY_COLOR,
};


static
PropertyType PropertyTypeFromName( IN const string& name )
{
    PropertyType rval = PROPERTY_UNKNOWN;
    

    for (int i = 0; i < ARRAY_SIZE(nameToType); ++i)
    {
        if ( !strcasecmp( name.c_str(), nameToType[i].name ) )
        {
            rval = nameToType[i].type;
            break;
        }
    }

    return rval;
}


static
const char* PropertyNameFromType( IN PropertyType type )
{
    const char* rval = "unknown";

    for (int i = 0; i < ARRAY_SIZE(nameToType); ++i)
    {
        if ( type == nameToType[i].type )
        {
            rval = nameToType[i].name;
            break;
        }
    }

    return rval;
}






//-----------------------------------------------------------------------------
// Property Static Class Methods
//-----------------------------------------------------------------------------
#pragma mark -
#pragma mark Property static class methods




//-----------------------------------------------------------------------------
// Object Methods
//-----------------------------------------------------------------------------
#pragma mark -
#pragma mark Property Object Methods


template<typename TYPE>
PropertyType
Property<TYPE>::GetType() const
{
    return m_propertyType;
}



template<typename TYPE>
Property<TYPE>::Property( TYPE* pObject, PropertyType type, OBJECT_GET_METHOD pGetter, OBJECT_SET_METHOD pSetter ) :
    m_pObject(pObject),
    m_propertyType(type),
    m_pGetMethod(pGetter),
    m_pSetMethod(pSetter)
{
    if (m_pObject)
    {
        DEBUGMSG(ZONE_OBJECT, "Property( %s.%s )", m_pObject->GetName().c_str(), PropertyNameFromType(m_propertyType) );
    }
    
    SAFE_ADDREF_TEMPLATE_TYPE(m_pObject);
    
    DEBUGCHK(pGetter);
    DEBUGCHK(pSetter);
}


template<typename TYPE>
Property<TYPE>::~Property()
{
    if (m_pObject)
    {
        DEBUGMSG(ZONE_OBJECT, "\t~Property( %s.%s )", m_pObject->GetName().c_str(), PropertyNameFromType(m_propertyType) );
    }

    m_pGetMethod = NULL;
    m_pSetMethod = NULL;

    SAFE_RELEASE_TEMPLATE_TYPE( m_pObject );
    m_pObject = NULL;
}




template<typename TYPE>
IProperty*
Property<TYPE>::Clone() const
{
    return static_cast<IProperty*>( new Property<TYPE>(*this) );
}


template<typename TYPE>
Property<TYPE>::Property( const Property<TYPE>& rhs ) : 
    m_pObject(NULL),
    m_pGetMethod(NULL),
    m_pSetMethod(NULL)
{
    *this = rhs;
}



template<typename TYPE>
Property<TYPE>& 
Property<TYPE>::operator=( const Property<TYPE>& rhs )
{
    // Avoid self-assignment.
    if (this == &rhs)
        return *this;
    
    SAFE_ADDREF_TEMPLATE_TYPE( rhs.m_pObject );
    SAFE_RELEASE_TEMPLATE_TYPE(m_pObject);

    // SHALLOW COPY:
    m_pObject           = rhs.m_pObject;
    m_propertyType      = rhs.m_propertyType;
    m_pGetMethod        = rhs.m_pGetMethod;
    m_pSetMethod        = rhs.m_pSetMethod;


    if (m_pObject)
    {
        DEBUGMSG(ZONE_OBJECT, "Property( %s.%s ) copy", m_pObject->GetName().c_str(), PropertyNameFromType(m_propertyType) );
    }
    
    return *this;
}




//-----------------------------------------------------------------------------
// Property Instance Methods
//-----------------------------------------------------------------------------

// Internal use only; users of Property should never use this macro.
#ifndef CALL_METHOD
#define CALL_METHOD(pObject, pMethod)  ((*pObject).*(pMethod)) 
#endif


template<typename TYPE>
void
Property<TYPE>::SetObject( Object* pObject )
{
    if (!pObject)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::SetObject(): argument cannot be NULL");
        return;
    }
    
    SAFE_RELEASE_TEMPLATE_TYPE(m_pObject);
    m_pObject = dynamic_cast<TYPE*>(pObject);
    SAFE_ADDREF_TEMPLATE_TYPE(m_pObject);
}



template<typename TYPE>
float
Property<TYPE>::GetFloat() const
{
    float rval;

    // TODO: install top-level signal handlers for UNIX exceptions like EXC_BAD_ACCESS.
    // They won't be handled by try/catch.
    // E.G. PLCrashReporter.
    
    DEBUGCHK(m_pGetMethod);
    DEBUGCHK(m_propertyType == PROPERTY_FLOAT);
    
    try 
    {
        rval = (float) CALL_METHOD(m_pObject, m_pGetFloatMethod)();
    } 
    catch (...) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::GetFloat( %x, %x ): access violation.", m_pObject, m_pGetMethod);
        rval = 0.0f;
    }
    
    // Test for NaN
    if (rval != rval)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::GetFloat( 0x%x, 0x%x ): is NaN.", m_pObject, m_pGetMethod);
        rval = 0.0f;
    }
    
    return rval;
}



template<typename TYPE>
void
Property<TYPE>::SetFloat( float val )
{
    DEBUGCHK(m_pSetMethod);
    DEBUGCHK(m_propertyType == PROPERTY_FLOAT);

    try 
    {
        CALL_METHOD(m_pObject, m_pSetFloatMethod)( val );
    } 
    catch (...) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::SetFloat( 0x%x, 0x%x ): access violation.", m_pObject, m_pSetMethod);
    }
}



template<typename TYPE>
UINT32
Property<TYPE>::GetInteger() const
{
    UINT32 rval;

    DEBUGCHK(m_pGetMethod);
    DEBUGCHK(m_propertyType == PROPERTY_UINT32);
    
    try 
    {
        rval = (UINT32) CALL_METHOD(m_pObject, m_pGetIntegerMethod)();
    } 
    catch (...) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::GetInteger( %x, %x ): access violation.", m_pObject, m_pGetMethod);
        rval = 0;
    }
    
    return rval;
}



template<typename TYPE>
void
Property<TYPE>::SetInteger( UINT32 val )
{
    DEBUGCHK(m_pSetMethod);
    DEBUGCHK(m_propertyType == PROPERTY_UINT32);

    try 
    {
        CALL_METHOD(m_pObject, m_pSetIntegerMethod)( val );
    } 
    catch (...) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::SetInteger( 0x%x, 0x%x ): access violation.", m_pObject, m_pSetMethod);
    }
}



template<typename TYPE>
bool
Property<TYPE>::GetBool() const
{
    UINT32 rval;

    DEBUGCHK(m_pGetMethod);
    DEBUGCHK(m_propertyType == PROPERTY_BOOL);
    
    try 
    {
        rval = (UINT32) CALL_METHOD(m_pObject, m_pGetBoolMethod)();
    } 
    catch (...) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::GetBool( %x, %x ): access violation.", m_pObject, m_pGetMethod);
        rval = 0;
    }
    
    return rval;
}



template<typename TYPE>
void
Property<TYPE>::SetBool( bool val )
{
    DEBUGCHK(m_pSetMethod);
    DEBUGCHK(m_propertyType == PROPERTY_BOOL);

    try 
    {
        CALL_METHOD(m_pObject, m_pSetBoolMethod)( val );
    } 
    catch (...) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::SetBool( 0x%x, 0x%x ): access violation.", m_pObject, m_pSetMethod);
    }
}



template<typename TYPE>
vec2
Property<TYPE>::GetVec2() const
{
    vec2 rval;

    DEBUGCHK(m_pGetMethod);
    DEBUGCHK(m_propertyType == PROPERTY_VEC2);
    
    try 
    {
        rval = (vec2) CALL_METHOD(m_pObject, m_pGetVec2Method)();
    } 
    catch (...) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::GetVec2( %x, %x ): access violation.", m_pObject, m_pGetMethod);
        rval = vec2(0,0);
    }
   
    return rval;
}



template<typename TYPE>
void
Property<TYPE>::SetVec2( const vec2& val )
{
    DEBUGCHK(m_pSetMethod);
    DEBUGCHK(m_propertyType == PROPERTY_VEC2);

    try 
    {
        CALL_METHOD(m_pObject, m_pSetVec2Method)( val );
    } 
    catch (...) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::SetVec2( 0x%x, 0x%x ): access violation.", m_pObject, m_pSetMethod);
    }
}



template<typename TYPE>
vec3
Property<TYPE>::GetVec3() const
{
    vec3 rval;

    DEBUGCHK(m_pGetMethod);
    DEBUGCHK(m_propertyType == PROPERTY_VEC3);
    
    try 
    {
        rval = (vec3) CALL_METHOD(m_pObject, m_pGetVec3Method)();
    } 
    catch (...) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::GetVec3( %x, %x ): access violation.", m_pObject, m_pGetMethod);
        rval = vec3(0,0,0);
    }
    
    // Test for NaN
    if (rval != rval)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::GetVec3( 0x%x, 0x%x ): is NaN.", m_pObject, m_pGetMethod);
        rval = vec3(0,0,0);
    }
    
    return rval;
}



template<typename TYPE>
void
Property<TYPE>::SetVec3( const vec3& val )
{
    DEBUGCHK(m_pSetMethod);
    DEBUGCHK(m_propertyType == PROPERTY_VEC3);

    try 
    {
        CALL_METHOD(m_pObject, m_pSetVec3Method)( val );
    } 
    catch (...) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::SetVec3( 0x%x, 0x%x ): access violation.", m_pObject, m_pSetMethod);
    }
}



template<typename TYPE>
vec4
Property<TYPE>::GetVec4() const
{
    vec4 rval;

    DEBUGCHK(m_pGetMethod);
    DEBUGCHK(m_propertyType == PROPERTY_VEC4);
    
    try 
    {
        rval = (vec4) CALL_METHOD(m_pObject, m_pGetVec4Method)();
    } 
    catch (...) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::GetVec3( %x, %x ): access violation.", m_pObject, m_pGetMethod);
        rval = vec4(0,0,0,0);
    }
    
    // Test for NaN
    if (rval != rval)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::GetVec4( 0x%x, 0x%x ): is NaN.", m_pObject, m_pGetMethod);
        rval = vec4(0,0,0,0);
    }
    
    return rval;
}



template<typename TYPE>
void
Property<TYPE>::SetVec4( const vec4& val )
{
    DEBUGCHK(m_pSetMethod);
    DEBUGCHK(m_propertyType == PROPERTY_VEC4);

    try 
    {
        CALL_METHOD(m_pObject, m_pSetVec4Method)( val );
    } 
    catch (...) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::SetVec4( 0x%x, 0x%x ): access violation.", m_pObject, m_pSetMethod);
    }
}



template<typename TYPE>
ivec2
Property<TYPE>::GetIVec2() const
{
    ivec2 rval;

    DEBUGCHK(m_pGetMethod);
    DEBUGCHK(m_propertyType == PROPERTY_IVEC2);
    
    try 
    {
        rval = (ivec2) CALL_METHOD(m_pObject, m_pGetIVec2Method)();
    } 
    catch (...) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::GetIVec2( %x, %x ): access violation.", m_pObject, m_pGetMethod);
        rval = ivec2(0,0);
    }
    
    return rval;
}



template<typename TYPE>
void
Property<TYPE>::SetIVec2( const ivec2& val )
{
    DEBUGCHK(m_pSetMethod);
    DEBUGCHK(m_propertyType == PROPERTY_IVEC2);

    try 
    {
        CALL_METHOD(m_pObject, m_pSetIVec2Method)( val );
    } 
    catch (...) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::SetIVec2( 0x%x, 0x%x ): access violation.", m_pObject, m_pSetMethod);
    }
}



template<typename TYPE>
ivec3
Property<TYPE>::GetIVec3() const
{
    ivec3 rval;

    DEBUGCHK(m_pGetMethod);
    DEBUGCHK(m_propertyType == PROPERTY_IVEC3);
    
    try 
    {
        rval = (ivec3) CALL_METHOD(m_pObject, m_pGetIVec3Method)();
    } 
    catch (...) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::GetIVec3( %x, %x ): access violation.", m_pObject, m_pGetMethod);
        rval = ivec3(0,0,0);
    }
    
    // Test for NaN
    if (rval != rval)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::GetIVec3( 0x%x, 0x%x ): is NaN.", m_pObject, m_pGetMethod);
        rval = ivec3(0,0,0);
    }
    
    return rval;
}



template<typename TYPE>
void
Property<TYPE>::SetIVec3( const ivec3& val )
{
    DEBUGCHK(m_pSetMethod);
    DEBUGCHK(m_propertyType == PROPERTY_IVEC3);

    try 
    {
        CALL_METHOD(m_pObject, m_pSetIVec3Method)( val );
    } 
    catch (...) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::SetIVec3( 0x%x, 0x%x ): access violation.", m_pObject, m_pSetMethod);
    }
}



template<typename TYPE>
ivec4
Property<TYPE>::GetIVec4() const
{
    ivec4 rval;

    DEBUGCHK(m_pGetMethod);
    DEBUGCHK(m_propertyType == PROPERTY_IVEC3);
    
    try 
    {
        rval = (ivec4) CALL_METHOD(m_pObject, m_pGetIVec4Method)();
    } 
    catch (...) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::GetIVec4( %x, %x ): access violation.", m_pObject, m_pGetMethod);
        rval = ivec4(0,0,0,0);
    }
    
    // Test for NaN
    if (rval != rval)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::GetIVec4( 0x%x, 0x%x ): is NaN.", m_pObject, m_pGetMethod);
        rval = ivec4(0,0,0,0);
    }
    
    return rval;
}



template<typename TYPE>
void
Property<TYPE>::SetIVec4( const ivec4& val )
{
    DEBUGCHK(m_pSetMethod);
    DEBUGCHK(m_propertyType == PROPERTY_IVEC4);

    try 
    {
        CALL_METHOD(m_pObject, m_pSetIVec4Method)( val );
    } 
    catch (...) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::SetIVec4( 0x%x, 0x%x ): access violation.", m_pObject, m_pSetMethod);
    }
}



template<typename TYPE>
Color
Property<TYPE>::GetColor() const
{
    Color rval;

    DEBUGCHK(m_pGetMethod);
    DEBUGCHK(m_propertyType == PROPERTY_COLOR);
    
    try 
    {
        rval = (Color) CALL_METHOD(m_pObject, m_pGetColorMethod)();
    } 
    catch (...) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::GetColor( %x, %x ): access violation.", m_pObject, m_pGetMethod);
        rval = Color(0,0,0,0);
    }
    
    return rval;
}



template<typename TYPE>
void
Property<TYPE>::SetColor( const Color& val )
{
    DEBUGCHK(m_pSetMethod);
    DEBUGCHK(m_propertyType == PROPERTY_COLOR);

    try 
    {
        CALL_METHOD(m_pObject, m_pSetColorMethod)( val );
    } 
    catch (...) 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Property::SetColor( 0x%x, 0x%x ): access violation.", m_pObject, m_pSetMethod);
    }
}



template<typename TYPE>
bool
Property<TYPE>::IsNull( ) const
{
    return !m_pObject || !m_pGetMethod || !m_pSetMethod;
}


/*
template <typename TYPE>
Property<TYPE>
Property<TYPE>::NullProperty()
{
    Property<TYPE> nullProperty;

    // TODO: point to a static method that prints an error and DEBUGCHKs?
    nullProperty.m_pObject      = NULL;
    nullProperty.m_pGetMethod   = NULL;
    nullProperty.m_pGetMethod   = NULL;
    
    return nullProperty;
}
*/





#pragma mark -
#pragma mark PropertySet

//-----------------------------------------------------------------------------
//
// PropertySet - a container for an object's named Properties.
//
// Works in conjunction with hObject->GetProperty( "name" ).
// This enables binding of Animations, Storyboards, and scripts to object handles.
//
// To create a PropertySet, you should:
//  1) declare a static NamedProperty array. It must be NULL-terminated.
//  2) DECLARE_PROPERTY_SET( YourClass, propertyTable ).
//  3) do this once per class.
//
//  *** 
//  *** When the PropertySet is constructed, it will iterate over s_propertyTable to        *** 
//  *** initialize itself.  Thereafter, anyone may call YourObject.GetProperty( "name" ).   *** 
//  *** 
//
// static const NamedProperty s_propertyTable[] =
// {
//     DECLARE_PROPERTY( GameObject, PROPERTY_VEC3,  Position  ),
//     DECLARE_PROPERTY( GameObject, PROPERTY_VEC3,  Rotation  ),
//     DECLARE_PROPERTY( GameObject, PROPERTY_FLOAT, Scale     ),
//     DECLARE_PROPERTY( GameObject, PROPERTY_FLOAT, Opacity   ),
//     NULL,
// };
// DECLARE_PROPERTY_SET( GameObject, s_propertyTable );
// 
// 
//-----------------------------------------------------------------------------


// Making C++ look like PERL
#define DECLARE_PROPERTY( CLASS, TYPE, PROPERTYNAME )  \
    { #PROPERTYNAME,      new Property< CLASS >( NULL, TYPE, (Property<CLASS>::OBJECT_GET_METHOD)&CLASS::Get##PROPERTYNAME,   (Property<CLASS>::OBJECT_SET_METHOD)&CLASS::Set##PROPERTYNAME  )  } 


#define DECLARE_PROPERTY_SET( CLASS, propertyTable ) \
    PropertySet CLASS::s_properties( propertyTable, #CLASS )

// TODO: possible DECLARE_PROPERTY_SET() macro that constructs the entire table, ensuring it is named properly and NULL-terminated?


typedef struct 
{
    const char* name;
    IProperty*  pIProperty;
} NamedProperty;



class PropertySet
{
public:
    // Automatically initialize the PropertySet when it is constructed (usually as a static class member).
    PropertySet( IN const NamedProperty s_propertyTable[], IN const string& classname = "" ) 
    {
        m_classname = classname;
        
        this->AddProperties( s_propertyTable );
    }


    UINT32 Count()
    {
        return m_properties.size();
    }


    RESULT AddProperties( IN const NamedProperty* pProperties )
    {
        RESULT rval = S_OK;
        
        if (pProperties)
        {
            const NamedProperty* pProperty = &pProperties[0];
            
            while (pProperty && pProperty->name)
            {
                // TEST:
                //printf("Register Property \"%s\" : \"%s\"\n", m_classname.c_str(), pProperty->name);
                
                m_properties.insert( std::pair< std::string, IProperty* >( pProperty->name, pProperty->pIProperty ) );
                ++pProperty;
            }
        }
        
    Exit:
        return rval;
    }
    
    
    IProperty* Get( IN const Object* pObject, const string& propertyName )
    {
        if (!pObject)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: Properties::Get(): pObject is NULL");
            return NULL;
        }

        // TEST:
        DEBUGMSG(ZONE_OBJECT | ZONE_VERBOSE, "PropertSet::Get( \"%s\" \"%s\", \"%s\" )", m_classname.c_str(), pObject->GetName().c_str(), propertyName.c_str());

        PropertyMapIterator ppTemplate = m_properties.find( propertyName );

        if (ppTemplate == m_properties.end())
        {
            RETAILMSG(ZONE_ERROR, "ERROR: Object[%4d] named \"%s\" has no Property \"%s\"", pObject->GetID(), pObject->GetName().c_str(), propertyName.c_str() );
    //        return Property<Object>::NullProperty();
            return NULL;
        }

        IProperty* pTemplate = ppTemplate->second;
        IProperty* pProperty = pTemplate->Clone();  // TODO: it's counter-intuitive and dangerous that a Property is something allocated, which the caller needs to remember to delete.  Need to make these members of the object.

        DEBUGCHK(pProperty);
        pProperty->SetObject( const_cast<Object*>(pObject) );
        
        return pProperty;
    }


protected:
    PropertySet( );
    PropertySet( const PropertySet& rhs );
    PropertySet& operator=( const PropertySet& rhs );

protected:
    typedef map<string, IProperty*> PropertyMap;
    typedef PropertyMap::iterator   PropertyMapIterator;

    string      m_classname;
    PropertyMap m_properties;
};


} // END namespace Z

