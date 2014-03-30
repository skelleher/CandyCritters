#pragma once


namespace Z
{



typedef struct 
{
    union
    {
        struct {
            float   x;
            float   y;
            float   z;
        };
        float position[3];
    };
    
    union {
        struct {
            BYTE   r;
            BYTE   g;
            BYTE   b;
            BYTE   a;
        };
        UINT32 color;
    };
    
    union {
        struct {  
            float   u0;
            float   v0;
        };
        float texCoord[2];
    };
} Vertex;


// TODO: custom vertex formats for lighting, multi-texturing, point sprites, etc.




} // END namespace Z
