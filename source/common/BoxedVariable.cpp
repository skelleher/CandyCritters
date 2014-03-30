/*
 *  BoxedVariable.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 11/27/12.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

// BoxedVariable wraps a primitive variable, allowing us to treat it like an object (specifically an IProperty).
// For example, this lets us bind an animation to a float:
//
//  float scale = 1.0f;
//  BoxedVariable bv( PROPERTY_FLOAT, &scale );
//  Animation.BindTo( hAnimation, bv );
//

#include "BoxedVariable.hpp"


namespace Z
{

//-----------------------------------------------------------------------------
// Object Methods
//-----------------------------------------------------------------------------
#pragma mark -
#pragma mark BoxedVariable methods

BoxedVariable::BoxedVariable( PropertyType type, void* pVariable )
{
    if (pVariable)
    {
        m_pVariable     = pVariable;
        m_propertyType  = type;
    }
    else
    {
        m_pVariable     = NULL;
        m_propertyType  = PROPERTY_UNKNOWN;
    }
}


float
BoxedVariable::GetFloat() const
{
    // TODO: install top-level signal handlers for UNIX exceptions like EXC_BAD_ACCESS.
    // They won't be handled by try/catch.
    // E.G. PLCrashReporter.
    
    DEBUGCHK(m_propertyType == PROPERTY_FLOAT);
    return *static_cast<float*>(m_pVariable);
}



void
BoxedVariable::SetFloat( float val )
{
    DEBUGCHK(m_propertyType == PROPERTY_FLOAT);
    *static_cast<float*>(m_pVariable) = val;
}



UINT32
BoxedVariable::GetInteger() const
{
    DEBUGCHK(m_propertyType == PROPERTY_UINT32);
    return *static_cast<UINT32*>(m_pVariable);
}



void
BoxedVariable::SetInteger( UINT32 val )
{
    DEBUGCHK(m_propertyType == PROPERTY_UINT32);
    *static_cast<UINT32*>(m_pVariable) = val;
}



bool
BoxedVariable::GetBool() const
{
    DEBUGCHK(m_propertyType == PROPERTY_BOOL);
    return *static_cast<bool*>(m_pVariable);
}



void
BoxedVariable::SetBool( bool val )
{
    DEBUGCHK(m_propertyType == PROPERTY_BOOL);
    *static_cast<bool*>(m_pVariable) = val;
}



vec2
BoxedVariable::GetVec2() const
{
    DEBUGCHK(m_propertyType == PROPERTY_VEC2);
    return *static_cast<vec2*>(m_pVariable);
}



void
BoxedVariable::SetVec2( const vec2& val )
{
    DEBUGCHK(m_propertyType == PROPERTY_VEC2);
    *static_cast<vec2*>(m_pVariable) = val;
}



vec3
BoxedVariable::GetVec3() const
{
    DEBUGCHK(m_propertyType == PROPERTY_VEC3);
    return *static_cast<vec3*>(m_pVariable);
}



void
BoxedVariable::SetVec3( const vec3& val )
{
    DEBUGCHK(m_propertyType == PROPERTY_VEC3);
    *static_cast<vec3*>(m_pVariable) = val;
}



vec4
BoxedVariable::GetVec4() const
{
    DEBUGCHK(m_propertyType == PROPERTY_VEC4);
    return *static_cast<vec4*>(m_pVariable);
}



void
BoxedVariable::SetVec4( const vec4& val )
{
    DEBUGCHK(m_propertyType == PROPERTY_VEC4);
    *static_cast<vec4*>(m_pVariable) = val;
}



ivec2
BoxedVariable::GetIVec2() const
{
    DEBUGCHK(m_propertyType == PROPERTY_IVEC2);
    return *static_cast<ivec2*>(m_pVariable);
}



void
BoxedVariable::SetIVec2( const ivec2& val )
{
    DEBUGCHK(m_propertyType == PROPERTY_IVEC2);
    *static_cast<ivec2*>(m_pVariable) = val;
}



ivec3
BoxedVariable::GetIVec3() const
{
    DEBUGCHK(m_propertyType == PROPERTY_IVEC3);
    return *static_cast<ivec3*>(m_pVariable);
}



void
BoxedVariable::SetIVec3( const ivec3& val )
{
    DEBUGCHK(m_propertyType == PROPERTY_IVEC3);
    *static_cast<ivec3*>(m_pVariable) = val;
}



ivec4
BoxedVariable::GetIVec4() const
{
    DEBUGCHK(m_propertyType == PROPERTY_IVEC4);
    return *static_cast<ivec4*>(m_pVariable);
}



void
BoxedVariable::SetIVec4( const ivec4& val )
{
    DEBUGCHK(m_propertyType == PROPERTY_IVEC4);
    *static_cast<ivec4*>(m_pVariable) = val;
}



Color
BoxedVariable::GetColor() const
{
    DEBUGCHK(m_propertyType == PROPERTY_COLOR);
    return *static_cast<Color*>(m_pVariable);
}



void
BoxedVariable::SetColor( const Color& val )
{
    DEBUGCHK(m_propertyType == PROPERTY_COLOR);
    *static_cast<Color*>(m_pVariable) = val;
}



bool
BoxedVariable::IsNull( ) const
{
    return !m_pVariable || m_propertyType == PROPERTY_UNKNOWN;
}


IProperty*
BoxedVariable::GetProperty ( const string& name ) const
{
    // BoxedVariable is a single IProperty; just return pointer-to-self regardless of requested property name.
//    return static_cast<IProperty*>(this);

    return (IProperty*)this;
}



IProperty*
BoxedVariable::Clone( ) const
{
    return static_cast<IProperty*>( new BoxedVariable(*this) );
}



void
BoxedVariable::SetObject( Object* pObject )
{
    DEBUGCHK(0);    // not implemented
}



BoxedVariable::BoxedVariable( const BoxedVariable& rhs ) :
    m_pVariable(NULL),
    m_propertyType(PROPERTY_UNKNOWN)
{
    *this = rhs;
}



BoxedVariable& 
BoxedVariable::operator=( const BoxedVariable& rhs )
{
    // Avoid self-assignment.
    if (this == &rhs)
        return *this;
    
    // SHALLOW COPY:
    m_propertyType  = rhs.m_propertyType;
    m_pVariable     = rhs.m_pVariable;

    return *this;
}



PropertyType
BoxedVariable::GetType( ) const
{
    return m_propertyType;
}




} // END namespace Z
