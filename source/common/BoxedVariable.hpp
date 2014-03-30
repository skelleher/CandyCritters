#pragma once

#include "Property.hpp"


namespace Z
{

#pragma mark -
#pragma mark BoxedVariable

// BoxedVariable wraps a primitive variable, allowing us to treat it like an object (specifically an IProperty).
// For example, this lets us bind an animation to a float:
//
//  float scale = 1.0f;
//  BoxedVariable bv( PROPERTY_FLOAT, &scale );
//  Animation.BindTo( hAnimation, bv );
//
class BoxedVariable : virtual public IProperty
{
public:
    BoxedVariable( PropertyType type, void* pVariable );
    virtual ~BoxedVariable() {}

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

    virtual IProperty*      GetProperty ( const string& name ) const;
    virtual bool            IsNull      ( ) const;
    virtual IProperty*      Clone       ( ) const;
    virtual void            SetObject   ( Object* pObject );
    virtual PropertyType    GetType     ( ) const;

protected:
    BoxedVariable( const BoxedVariable& rhs );
    BoxedVariable& operator=( const BoxedVariable& rhs );

protected:
    PropertyType    m_propertyType;
    void*           m_pVariable;
};

} // END namespace Z

