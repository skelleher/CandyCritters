/*
 *  Util.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 10/5/10.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include "Util.hpp"
#include "Log.hpp"
#include "Macros.hpp"

#include <string>
using std::string;


namespace Z
{

   
RESULT 
Util::GetBoundingRect( IN const Vertex *pVertices, IN UINT32 numVertices, IN Rectangle* pRect )
{
    RESULT  rval    = S_OK;
    const Vertex* pVertex;
    float   minX;
    float   minY;
    float   maxX;
    float   maxY;
    
    
    CPREx(pRect, E_NULL_POINTER);
    
    if (!numVertices || !pVertices)
    {
        pRect->x        = 0;
        pRect->y        = 0;
        pRect->width    = 0;
        pRect->height   = 0;
        
        CHR(E_INVALID_ARG);
    }
    
    
    //
    // Find the min/max X and Y coordinates.
    //
    minX = FLT_MAX;
    minY = FLT_MAX;
    maxX = 0;
    maxY = 0;
    
    pVertex = pVertices;
    for (UINT32 i = 0; i < numVertices; ++i)
    {
        minX = MIN( minX, pVertex->x );
        minY = MIN( minY, pVertex->y );
        maxX = MAX( maxX, pVertex->x );
        maxY = MAX( maxY, pVertex->y );
        
        pVertex++;
    }
    
    //
    // Compute the bounding rectangle.
    //
    pRect->x        = minX;
    pRect->y        = minY;
    pRect->width    = maxX - minX;
    pRect->height   = maxY - minY;
    
Exit:
    return rval;
}



// Compute an AABB for the given triangle strip.
RESULT
Util::GetAABB( IN const Vertex *pVertices, IN UINT32 numVertices, OUT AABB* pAABB )
{
    RESULT  rval    = S_OK;
    const Vertex* pVertex;
    float   minX;
    float   minY;
    float   minZ;
    float   maxX;
    float   maxY;
    float   maxZ;
    
    
    CPREx(pAABB, E_NULL_POINTER);
    
    if (!numVertices || !pVertices)
    {
        pAABB->SetMin( vec3(0,0,0) );
        pAABB->SetMax( vec3(0,0,0) );
        CHR(E_INVALID_ARG);
    }
    
    
    //
    // Find the min/max coordinates.
    //
    minX = FLT_MAX;
    minY = FLT_MAX;
    minZ = FLT_MAX;
    maxX = 0;
    maxY = 0;
    maxZ = 0;
    
    pVertex = pVertices;
    for (UINT32 i = 0; i < numVertices; ++i)
    {
        minX = MIN( minX, pVertex->x );
        minY = MIN( minY, pVertex->y );
        minZ = MIN( minZ, pVertex->z );
        maxX = MAX( maxX, pVertex->x );
        maxY = MAX( maxY, pVertex->y );
        maxZ = MAX( maxZ, pVertex->z );
        
        pVertex++;
    }
    
    pAABB->SetMin( vec3(minX, minY, minZ) );
    pAABB->SetMax( vec3(maxX, maxY, maxZ) );
    
Exit:
    return rval;
}



// Determine the min and max texture coordinates for the given triangle strip.
RESULT 
Util::GetTextureMapping( IN const Vertex *pVertices, IN UINT32 numVertices, OUT Rectangle* pRect )
{
    RESULT  rval    = S_OK;
    const Vertex* pVertex;
    float   uStart;
    float   vStart;
    float   uEnd;
    float   vEnd;
    
    
    CPREx(pRect, E_NULL_POINTER);
    
    if (!numVertices || !pVertices)
    {
        pRect->x        = 0;
        pRect->y        = 0;
        pRect->width    = 0;
        pRect->height   = 0;
        
        CHR(E_INVALID_ARG);
    }
    
    
    //
    // Find the min/max U and V coordinates.
    //
    uStart = FLT_MAX;
    vStart = FLT_MAX;
    uEnd = 0;
    vEnd = 0;
    
    pVertex = pVertices;
    for (UINT32 i = 0; i < numVertices; ++i)
    {
        uStart = MIN( uStart, pVertex->texCoord[0] );
        vStart = MIN( vStart, pVertex->texCoord[1] );
        uEnd   = MAX( uEnd,   pVertex->texCoord[0] );
        vEnd   = MAX( vEnd,   pVertex->texCoord[1] );
        
        pVertex++;
    }
    
    //
    // Compute the bounding rectangle.
    //
    pRect->x        = uStart;
    pRect->y        = vStart;
    pRect->width    = uEnd - uStart;
    pRect->height   = vEnd - vStart;


    // The min/max approach above results in texcoords increasing left to right and bottom to top.
    // This is wrong if pVertices is a top-down or right-left mesh.
    // Flip the X and/or Y axis if needed.
    if (pVertices[0].texCoord[0] > pVertices[numVertices-1].texCoord[0])
    {
        pRect->x        = uEnd;
        pRect->width    = -1 * pRect->width;
    }
    
#ifdef __APPLE__
    // Remember: on iPhone the Y axis is inverted (draws from bottom to top)
    if (pVertices[0].texCoord[1] < pVertices[numVertices-1].texCoord[1])
    {
        pRect->y        = vEnd;
        pRect->height   = -1 * pRect->height;
    }
#else
    if (pVertices[0].texCoord[1] > pVertices[numVertices-1].texCoord[1])
    {
        pRect->y        = vEnd;
        pRect->height   = -1 * pRect->height;
    }
#endif
    
    
Exit:
    return rval;
}


// Generate a triangle strip for the given rectangle.
// Caller must free the returned buffer when done.
RESULT 
Util::CreateTriangleStrip( IN const Rectangle* pRect, OUT Vertex **ppVertices, OUT UINT32* pNumVertices, float uStart, float uEnd, float vStart, float vEnd, IN const Color& color )
{
    RESULT rval = S_OK;
    
    CPREx(pRect,        E_NULL_POINTER);
    CPREx(ppVertices,   E_NULL_POINTER);
    CPREx(pNumVertices, E_NULL_POINTER);
    
    
    try 
    {
        UINT32  numVertices = 4;
        Vertex* pVertices = new Vertex[numVertices];
        
        DEBUGCHK(pVertices);
        
        rval = CreateTriangleStrip( pRect, pVertices, uStart, uEnd, vStart, vEnd, color );
        
        *pNumVertices   = numVertices;
        *ppVertices     = pVertices;
    }
    catch (...)
    {
        RETAILMSG(ZONE_ERROR, "Util::CreateTriangleStrip(): exception occured.");
        return E_FAIL;
    }
    
    
Exit:
    return rval;
}



// Generate a triangle strip for the given rectangle.
// Caller must provide a buffer of adequate size - sizeof(Vertex)*rows*columns*4.
RESULT 
Util::CreateTriangleStrip( IN const Rectangle* pRect, INOUT Vertex *pVertices, float uStart, float uEnd, float vStart, float vEnd, IN const Color& color  )
{
    RESULT      rval        = S_OK;
    float       width;
    float       height;
    float       x;
    float       y;
    
    CPREx(pRect,        E_NULL_POINTER);
    CPREx(pVertices,    E_NULL_POINTER);
   
    
    width       = pRect->width;
    height      = pRect->height;
    x           = pRect->x;
    y           = pRect->y;
        
    
    try 
    {
        //
        // Generate a triangle strip/quadrilateral
        //
        Vertex* pVert0 = &pVertices[ 0 ];
        Vertex* pVert1 = &pVertices[ 1 ];
        Vertex* pVert2 = &pVertices[ 2 ];
        Vertex* pVert3 = &pVertices[ 3 ];
        
        pVert0->x   = x;            pVert0->y   = y;            pVert0->z = 0.0f;
        pVert1->x   = x;            pVert1->y   = y + height;   pVert1->z = 0.0f;
        pVert2->x   = x + width;    pVert2->y   = y;            pVert2->z = 0.0f;
        pVert3->x   = x + width;    pVert3->y   = y + height;   pVert3->z = 0.0f;
        
        pVert0->color = color;
        pVert1->color = color;
        pVert2->color = color;
        pVert3->color = color;
        
#ifdef __APPLE__
        // Remember: on iPhone the Y axis is inverted (draws from bottom to top)
        // So Y texture coordinates must be inverted!
        pVert0->u0 = uStart;    pVert0->v0  = vEnd;
        pVert1->u0 = uStart;    pVert1->v0  = vStart;
        pVert2->u0 = uEnd;      pVert2->v0  = vEnd;
        pVert3->u0 = uEnd;      pVert3->v0  = vStart;
#else        
        pVert0->u0 = uStart;    pVert0->v0  = vStart;
        pVert1->u0 = uStart;    pVert1->v0  = vEnd;
        pVert2->u0 = uEnd;      pVert2->v0  = vStart;
        pVert3->u0 = uEnd;      pVert3->v0  = vEnd;
#endif
    }
    catch (...)
    {
        RETAILMSG(ZONE_ERROR, "Util::CreateTriangleStrip(): exception occured.");
        return E_FAIL;
    }
    
Exit:
    return rval;
}





// Generate a triangle list for the given rectangle.
// Caller must free the returned buffer when done.
RESULT 
Util::CreateTriangleList( IN const Rectangle* pRect,  UINT32 rows, UINT32 columns, OUT Vertex **ppVertices, OUT UINT32* pNumVertices, float uStart, float uEnd, float vStart, float vEnd, IN const Color& color )
{
    RESULT rval = S_OK;
    
    CPREx(pRect,        E_NULL_POINTER);
    CPREx(ppVertices,   E_NULL_POINTER);
    CPREx(pNumVertices, E_NULL_POINTER);
    CBREx(rows > 0,     E_INVALID_ARG);
    CBREx(columns > 0,  E_INVALID_ARG);
    
    
    try 
    {
        UINT32  numVertices = 6 * rows * columns;
        Vertex* pVertices   = new Vertex[numVertices];
        
        DEBUGCHK(pVertices);
        
        rval = CreateTriangleList( pRect, rows, columns, pVertices, uStart, uEnd, vStart, vEnd, color );
        
        *pNumVertices   = numVertices;
        *ppVertices     = pVertices;
    }
    catch (...)
    {
        RETAILMSG(ZONE_ERROR, "Util::CreateTriangleStrip(): exception occured.");
        return E_FAIL;
    }
    
    
Exit:
    return rval;
}


// Generate a triangle list for the given rectangle.
// Caller must provide a buffer of adequate size - sizeof(Vertex)*rows*columns*6.
RESULT 
Util::CreateTriangleList( IN const Rectangle* pRect, UINT32 rows, UINT32 columns, INOUT Vertex *pVertices, float uStart, float uEnd, float vStart, float vEnd, IN const Color& color )
{
    RESULT      rval        = S_OK;
    float       width;
    float       height;
    float       cellWidth;
    float       cellHeight;
    float       x;
    float       y;
    float       uWidth;
    float       vHeight;
    
    CPREx(pRect,        E_NULL_POINTER);
    CPREx(pVertices,    E_NULL_POINTER);
    CBREx(rows > 0,     E_INVALID_ARG);
    CBREx(columns > 0,  E_INVALID_ARG);
    
    
    width       = pRect->width;
    height      = pRect->height;
    cellWidth   = width/columns;
    cellHeight  = height/rows;
    x           = pRect->x;
    y           = pRect->y;
    
    uWidth      = uEnd - uStart;
    vHeight     = vEnd - vStart;
    
    
    try 
    {
        int i = 0;
        
        //
        // Generate a triangle list with the given number
        // of rows and columns.
        //
        for (UINT32 row = 0; row < rows; ++row)
        {
            for (UINT32 col = 0; col < columns; ++col)
            {
                Vertex* pVert0 = &pVertices[ i++ ];
                Vertex* pVert1 = &pVertices[ i++ ];
                Vertex* pVert2 = &pVertices[ i++ ];
                Vertex* pVert3 = &pVertices[ i++ ];
                Vertex* pVert4 = &pVertices[ i++ ];
                Vertex* pVert5 = &pVertices[ i++ ];
                pVert0->x   = x + (cellWidth * col);         pVert0->y = y + (cellHeight * row);      pVert0->z = 0.0f;
                pVert1->x   = x + (cellWidth * (col+1));     pVert1->y = y + (cellHeight * row);      pVert1->z = 0.0f;
                pVert2->x   = x + (cellWidth * col);         pVert2->y = y + (cellHeight * (row+1));  pVert2->z = 0.0f;
                pVert3->x   = x + (cellWidth * col);         pVert3->y = y + (cellHeight * (row+1));  pVert3->z = 0.0f;
                pVert4->x   = x + (cellWidth * (col+1));     pVert4->y = y + (cellHeight * row);      pVert4->z = 0.0f;
                pVert5->x   = x + (cellWidth * (col+1));     pVert5->y = y + (cellHeight * (row+1));  pVert5->z = 0.0f;
                
                pVert0->color = color;
                pVert1->color = color;
                pVert2->color = color;
                pVert3->color = color;
                pVert4->color = color;
                pVert5->color = color;

#ifdef __APPLE__

                // Remember: on iPhone the Y axis is inverted (draws from bottom to top)
                // So Y texture coordinates must be inverted!
                pVert0->u0 = uStart + uWidth/columns * col;      pVert0->v0  = vEnd - vHeight/rows * row;
                pVert1->u0 = uStart + uWidth/columns * (col+1);  pVert1->v0  = vEnd - vHeight/rows * row;
                pVert2->u0 = uStart + uWidth/columns * col;      pVert2->v0  = vEnd - vHeight/rows * (row+1);
                pVert3->u0 = uStart + uWidth/columns * col;      pVert3->v0  = vEnd - vHeight/rows * (row+1);
                pVert4->u0 = uStart + uWidth/columns * (col+1);  pVert4->v0  = vEnd - vHeight/rows * row;
                pVert5->u0 = uStart + uWidth/columns * (col+1);  pVert5->v0  = vEnd - vHeight/rows * (row+1);
#else
                pVert0->u0 = uStart + uWidth/columns * col;      pVert0->v0  = vStart + vHeight/rows * row;
                pVert1->u0 = uStart + uWidth/columns * (col+1);  pVert1->v0  = vStart + vHeight/rows * row;
                pVert2->u0 = uStart + uWidth/columns * col;      pVert2->v0  = vStart + vHeight/rows * (row+1);
                pVert3->u0 = uStart + uWidth/columns * col;      pVert3->v0  = vStart + vHeight/rows * (row+1);
                pVert4->u0 = uStart + uWidth/columns * (col+1);  pVert4->v0  = vStart + vHeight/rows * row;
                pVert5->u0 = uStart + uWidth/columns * (col+1);  pVert5->v0  = vStart + vHeight/rows * (row+1);
#endif
               
                  
/*                
                DEBUGMSG(ZONE_INFO, "Util::CreateTriangleList(): ");
                DEBUGMSG(ZONE_INFO, "vert[0] = (%4.2f, %4.2f, %4.2f) 0x%x (%4.2f, %4.2f)", pVert0->x, pVert0->y, pVert0->z, pVert0->color, pVert0->u0, pVert0->v0);
                DEBUGMSG(ZONE_INFO, "vert[1] = (%4.2f, %4.2f, %4.2f) 0x%x (%4.2f, %4.2f)", pVert1->x, pVert1->y, pVert1->z, pVert1->color, pVert1->u0, pVert1->v0);
                DEBUGMSG(ZONE_INFO, "vert[2] = (%4.2f, %4.2f, %4.2f) 0x%x (%4.2f, %4.2f)", pVert2->x, pVert2->y, pVert2->z, pVert2->color, pVert2->u0, pVert2->v0);
                DEBUGMSG(ZONE_INFO, "vert[3] = (%4.2f, %4.2f, %4.2f) 0x%x (%4.2f, %4.2f)", pVert3->x, pVert3->y, pVert3->z, pVert3->color, pVert3->u0, pVert3->v0);
                DEBUGMSG(ZONE_INFO, "vert[4] = (%4.2f, %4.2f, %4.2f) 0x%x (%4.2f, %4.2f)", pVert4->x, pVert4->y, pVert4->z, pVert4->color, pVert4->u0, pVert4->v0);
                DEBUGMSG(ZONE_INFO, "vert[5] = (%4.2f, %4.2f, %4.2f) 0x%x (%4.2f, %4.2f)", pVert5->x, pVert5->y, pVert5->z, pVert5->color, pVert5->u0, pVert5->v0);
 */
            }
        }
    }
    catch (...)
    {
        RETAILMSG(ZONE_ERROR, "Util::CreateTriangleList(): exception occured.");
        return E_FAIL;
    }
    
Exit:
    return rval;
}



string 
Util::FindValueForKey( IN const string& searchString, IN const string& key, IN const string& separator )
{
    string              result;
    string::size_type   start;
    string::size_type   end;

    string search = key + separator;

    start  = searchString.find( search ) + search.length();
    
    if (string::npos == start)
        return result;
    
    // If the value we found is a quoted string,
    // scan until the closing quote.
    // Otherwise, scan until the next whitespace.
    if ( searchString[start] == '"' )
    {
        end = searchString.find("\"", start+1) + 1;
    }
    else if ( searchString[start] == '\'' )
    {
        end = searchString.find("'", start+1) + 1;
    }
    else
    {
        end = searchString.find(" \r\n\t", start);
    }
        
    result = searchString.substr(start, end-start);
    

    // Strip out any quotes, trailing \n, etc.
//    result.erase(std::remove(result.begin(), result.end(), '"'), result.end());
//    result.erase(std::remove(result.begin(), result.end(), '`'), result.end());
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    
    return result;
}



float
Util::FindFloatValueForKey( IN const string& searchString, IN const string& key, IN const string& separator )
{
    return atof( FindValueForKey( searchString, key, separator ).c_str() );
}



int
Util::FindIntValueForKey( IN const string& searchString, IN const string& key, IN const string& separator )
{
    return atoi( FindValueForKey( searchString, key, separator ).c_str() );
}


string
Util::ResolutionSpecificFilename( IN const string& baseFilename )
{
    // Select background image based on screen size.
    std::string filename = baseFilename;
    // /app/textures/background3.png
    
    int pos = filename.find("textures/", 0);
    int len = strlen("textures/");

    if (Platform::IsWidescreen()) {
        filename = filename.replace(pos, len, "textures/4-inch/");
    } else {
        filename = filename.replace(pos, len, "textures/3.5-inch/");
    }
    
    return filename;
}


} // END namespace Z
