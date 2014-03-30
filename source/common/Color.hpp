#pragma once

#include "Types.hpp"

namespace Z
{

   
class Color
{
public:
    Color();
    Color( float r, float g, float b, float a );
    Color( vec4& );
    Color( UINT32 rgba );
    
    operator vec4()     const;
    operator UINT32()   const;
    
    Color& operator*=(float rhs);
    Color& operator+=(float rhs);
    Color& operator+=(const Color& rhs);
    Color  operator+(const Color& rhs) const;
    Color  operator-(const Color& rhs) const;

    // HACK: place the floats array first
    // so that parameter coercion from vec4& -> Color& "just works."
    // The keyframe animation system relies on this rather than
    // define a separate Color keyframe type.
    struct
    {
        float r;
        float g;
        float b;
        float a;
    } floats;

public:
    static Color Clear()    { return Color(0.0f, 0.0f,  0.0f, 0.0f); }
    static Color Red()      { return Color(1.0f, 0.0f,  0.0f, 1.0f); }
    static Color Green()    { return Color(0.0f, 1.0f,  0.0f, 1.0f); }
    static Color Blue()     { return Color(0.0f, 0.0f,  1.0f, 1.0f); }
    static Color Yellow()   { return Color(1.0f, 1.0f,  0.0f, 1.0f); }
    static Color Purple()   { return Color(1.0f, 0.0f,  1.0f, 1.0f); }
    static Color Orange()   { return Color(1.0f, 0.6f,  0.14f,1.0f); }
    static Color Amber()    { return Color(1.0f, 0.77f, 0.0f, 1.0f); }
    static Color Gold()     { return Color(1.0f, 0.96f, 0.6f, 1.0f); }
    static Color Violet()   { return Color(0.0f, 0.65f, 0.3f, 1.0f); }
    static Color Pink()     { return Color(1.0f, 0.56f, 0.98f,1.0f); }
    static Color LightBlue(){ return Color(0.43f,0.81f, 0.96f,1.0f); }
    static Color White()    { return Color(1.0f, 1.0f,  1.0f, 1.0f); }
    static Color Black()    { return Color(0.0f, 0.0f,  0.0f, 1.0f); }
    static Color Gray()     { return Color(0.22, 0.71,  0.07, 1.0f); }
};


} // END namespace Z
