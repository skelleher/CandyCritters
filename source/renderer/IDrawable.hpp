#pragma once

#include "Matrix.hpp"
#include "IEffect.hpp"

namespace Z
{



class IDrawable
{
public:
    virtual RESULT      SetVisible  ( bool          isVisible        )  = 0;
    virtual RESULT      SetPosition ( const vec3&   vPos             )  = 0;
    virtual RESULT      SetRotation ( const vec3&   vRotationDegrees )  = 0;
    virtual RESULT      SetScale    ( float         scale            )  = 0;
    virtual RESULT      SetOpacity  ( float         opacity          )  = 0;
    virtual RESULT      SetEffect   ( HEffect       hEffect          )  = 0;
    virtual RESULT      SetColor    ( const Color&  color            )  = 0;
    virtual RESULT      SetShadow   ( bool          hasShadow        )  = 0;
    

    virtual bool        GetVisible  ( )                                 = 0;
    virtual vec3        GetPosition ( )                                 = 0;
    virtual vec3        GetRotation ( )                                 = 0;
    virtual float       GetScale    ( )                                 = 0;
    virtual float       GetOpacity  ( )                                 = 0;
    virtual HEffect     GetEffect   ( )                                 = 0;
    virtual AABB        GetBounds   ( )                                 = 0;
    virtual Color       GetColor    ( )                                 = 0;
    virtual bool        GetShadow   ( )                                 = 0;
    
    virtual RESULT      Draw        ( const mat4&   matParentWorld )  = 0;

    virtual ~IDrawable() {};
};



} // END namespace Z
