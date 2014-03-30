/*
 *  Color.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 9/4/10.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include "Color.hpp"


namespace Z
{


Color::Color()
{
    Color( 1.0f, 1.0f, 1.0f, 1.0f);
}


/*
Color::Color( BYTE  r, BYTE  g, BYTE  b, BYTE  a )
{
    bytes.r = r;
    bytes.g = g;
    bytes.b = b;
    bytes.a = a;
    
    floats.r = (float)(r/255.0);
    floats.g = (float)(g/255.0);
    floats.b = (float)(b/255.0);
    floats.a = (float)(a/255.0);
}
*/


Color::Color( float r, float g, float b, float a )
{
    floats.r    = r;
    floats.g    = g;
    floats.b    = b;
    floats.a    = a;
/*
    bytes.r     = (BYTE)(r * 255.0);
    bytes.g     = (BYTE)(g * 255.0);
    bytes.b     = (BYTE)(b * 255.0);
    bytes.a     = (BYTE)(a * 255.0);
*/
}

    

Color::Color( vec4& v )
{
    floats.r    = v.x;
    floats.g    = v.y;
    floats.b    = v.z;
    floats.a    = v.w;
/*
    bytes.r     = (BYTE)(floats.r * 255.0);
    bytes.g     = (BYTE)(floats.g * 255.0);
    bytes.b     = (BYTE)(floats.b * 255.0);
    bytes.a     = (BYTE)(floats.a * 255.0);
*/    
}


Color::Color( UINT32 rgba )
{
    floats.r = (rgba & 0xFF000000) >> 24;
    floats.g = (rgba & 0x00FF0000) >> 16;
    floats.b = (rgba & 0x0000FF00) >> 8;
    floats.a = (rgba & 0x000000FF);
}


Color::operator vec4() const
{
    return vec4( floats.r, floats.g, floats.b, floats.a );
}
    
/*
Color::operator UINT32() const
{
    return bytes.r << 24 | bytes.g << 16 | bytes.b << 8 | bytes.a;
}
*/


Color::operator UINT32() const
{
//    return bytes.a << 24 | bytes.b << 16 | bytes.g << 8 | bytes.r;

    return (BYTE)(floats.a * 255.0) << 24 | (BYTE)(floats.b * 255.0) << 16 | (BYTE)(floats.g * 255.0) << 8 | (BYTE)(floats.r * 255.0);
}

Color&
Color::operator*=( float f )
{
    floats.r *= f;
    floats.g *= f;
    floats.b *= f;
    floats.a *= f;
    
    return *this;
}


Color&
Color::operator+=( float f )
{
    floats.r += f;
    floats.g += f;
    floats.b += f;
    floats.a += f;
    
    return *this;
}


Color&
Color::operator+=( const Color& rhs )
{
    floats.r += rhs.floats.r;
    floats.g += rhs.floats.g;
    floats.b += rhs.floats.b;
    floats.a += rhs.floats.a;
    
    return *this;
}


Color
Color::operator+(const Color& rhs) const
{
    Color rval;
    
    rval.floats.r = floats.r + rhs.floats.r;
    rval.floats.g = floats.g + rhs.floats.g;
    rval.floats.b = floats.b + rhs.floats.b;
    rval.floats.a = floats.a + rhs.floats.a;
    
    return rval;
}


Color
Color::operator-(const Color& rhs) const
{
    Color rval;
    
    rval.floats.r = floats.r - rhs.floats.r;
    rval.floats.g = floats.g - rhs.floats.g;
    rval.floats.b = floats.b - rhs.floats.b;
    rval.floats.a = floats.a - rhs.floats.a;
    
    return rval;
}

    


} // END namespace Z
