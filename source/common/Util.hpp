#pragma once

#include "Types.hpp"
#include "IRenderer.hpp"

#include <string>
using std::string;


namespace Z
{


class Util
{
public:
    // Compute a 2D bounding rectangle for the given triangle strip.
    static RESULT GetBoundingRect      ( IN const Vertex *pVertices, IN UINT32 numVertices, OUT Rectangle* pRect );

    // Compute an AABB for the given triangle strip.
    static RESULT GetAABB              ( IN const Vertex *pVertices, IN UINT32 numVertices, OUT AABB* pAABB       );

    // Determine the min and max texture coordinates for the given triangle strip.
    static RESULT GetTextureMapping    ( IN const Vertex *pVertices, IN UINT32 numVertices, OUT Rectangle* pRect );

    // Generate a triangle strip for the given rectangle.
    // Strip will have four vertices, forming a quadrilateral.
    // Texture coordinates will begin at [uOffset, vOffset]  (modify if using a TextureAtlas).
    // Caller must provide a buffer of adequate size - sizeof(Vertex)*rows*columns*4.
    static RESULT CreateTriangleStrip  ( IN const Rectangle* pRect, INOUT Vertex *pVertices, float uStart = 0.0f, float uEnd = 1.0f, float vStart = 0.0f, float vEnd = 1.0f, IN const Color& color = Color::White() );

    // Generate a triangle strip for the given rectangle.
    // Strip will have four vertices, forming a quadrilateral.
    // Texture coordinates will begin at [uOffset, vOffset]  (modify if using a TextureAtlas).
    // Caller must free the returned buffer when done.
    static RESULT CreateTriangleStrip  ( IN const Rectangle* pRect, OUT Vertex **ppVertices, OUT UINT32* pNumVertices, float uStart = 0.0f, float uEnd = 1.0f, float vStart = 0.0f, float vEnd = 1.0f, IN const Color& color = Color::White() );

    // Generate a triangle list for the given rectangle.
    // Strip will have N vertices, forming a quadrilateral.
    // Texture coordinates will begin at [uOffset, vOffset]  (modify if using a TextureAtlas).
    // Caller must provide a buffer of adequate size - sizeof(Vertex)*rows*columns*6.
    static RESULT CreateTriangleList  ( IN const Rectangle* pRect, UINT32 rows, UINT32 columns, INOUT Vertex *pVertices, float uStart = 0.0f, float uEnd = 1.0f, float vStart = 0.0f, float vEnd = 1.0f, IN const Color& color = Color::White() );

    // Generate a triangle list for the given rectangle.
    // Strip will have N vertices, forming a quadrilateral.
    // Texture coordinates will begin at [uOffset, vOffset]  (modify if using a TextureAtlas).
    // Caller must free the returned buffer when done.
    static RESULT CreateTriangleList  ( IN const Rectangle* pRect, UINT32 rows, UINT32 columns, OUT Vertex **ppVertices, OUT UINT32* pNumVertices, float uStart = 0.0f, float uEnd = 1.0f, float vStart = 0.0f, float vEnd = 1.0f, IN const Color& color = Color::White() );

/*
    // Generate a triangle strip for a cube of the given width/height.
    // Caller must provide a buffer of adequate size - sizeof(Vertex)*8.
    static RESULT CreateCubeMesh      ( float width, float height, INOUT Vertex *pVertices );

    
    // Generate a triangle strip for a cube of the given width/height.
    // Caller must free the returned buffer when done.
    static RESULT CreateCubeMesh      ( float width, float height, OUT Vertex **ppVertices, OUT UINT32* pNumVertices );
*/    

    // Are two floats "equal" (within EPSILON of each other, 
    // which is the smallest representable floating point value).
    static inline bool CompareFloats    ( float  a, float  b ) { float  delta = (float) (a) - (float) (b); return -FLT_EPSILON <= delta && delta <= FLT_EPSILON; }
    static inline bool CompareDoubles   ( double a, double b ) { double delta = (double)(a) - (double)(b); return -DBL_EPSILON <= delta && delta <= DBL_EPSILON; }

    static inline bool CompareRectangles( const Rectangle& a, const Rectangle& b )      { return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height; }


    // Search a string for key/value pairs of the form key=value.
    // Assumes value is terminated by white space or EOL.
    // Caller can provide a custom separator string.
    static string   FindValueForKey     ( IN const string& searchString, IN const string& key, IN const string& separator = "=" );
    static int      FindIntValueForKey  ( IN const string& searchString, IN const string& key, IN const string& separator = "=" );
    static float    FindFloatValueForKey( IN const string& searchString, IN const string& key, IN const string& separator = "=" );
    
    
    static inline bool IsPowerOfTwo     ( UINT32 value ) 
    { 
        if ( (value > 0) && ((value & (value - 1)) == 0) ) 
        { 
            return true; 
        } 
        else 
        { 
            return false; 
        }  
    }
    
    
    static inline UINT32 RoundUpToPowerOfTwo ( UINT32 value )
    {
        // http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
        
        value--;
        value |= value >> 1;
        value |= value >> 2;
        value |= value >> 4;
        value |= value >> 8;
        value |= value >> 16;
        value++;
        
        return value;
    }
    
    static string ResolutionSpecificFilename( IN const string& baseFilename );
    
};


} // END namespace Z

