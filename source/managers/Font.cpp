/*
 *  Font.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 2/26/11.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include "Font.hpp"
#include "Util.hpp"
#include "Engine.hpp"
#include "FileManager.hpp"

// TEST
#include "DebugRenderer.hpp"

#include <string>
using std::string;

// We allocate an array of FontChar structures, 
// and sort them by the ASCII value of the glyph.
static const int  MAX_PRINTABLE_GLYPHS = 96;
static const char FIRST_PRINTABLE_GLYPH = ' ';



namespace Z 
{



// ============================================================================
//
//  Font Implementation
//
// ============================================================================

// Don't bother with 4 verts per glyph (GL_TRIANGLE_STRIP).
// We batch them up and submit one big array, and 
// that's much easier without strips.
#define VERTS_PER_GLYPH 6


#pragma mark Font Implementation

Font::Font() :
    m_numFontChars(0),
    m_pFontChars(NULL),
    m_lineHeight(0),
    m_textureWidth(0),
    m_textureHeight(0)
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "Font( %4d )", m_ID);
}


Font::~Font()
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "\t~Font( %4d )", m_ID);
    
    SAFE_ARRAY_DELETE(m_pFontChars);
    m_numFontChars = 0;
}


/*
Font*
Font::Clone() const
{
    Font* pFontClone = new Font(*this);
    
    // Give it a new name with a random suffix
    char instanceName[MAX_NAME];
    sprintf(instanceName, "%s_%X", m_name.c_str(), (unsigned int)Platform::Random());
    pFontClone->m_name = string(instanceName);
    
    // Reset state

    return pFontClone;
}
*/


RESULT
Font::Init( IN const string& name, IN const Settings* pSettings, IN const string& settingsPath )
{
    RESULT      rval        = S_OK;
    string      fontfile;
    string      texture;
    TextureInfo info;

    RETAILMSG(ZONE_OBJECT, "Font[%4d]::Init( %s )", m_ID, name.c_str());
    
    m_name = name;
    
    if (!pSettings)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Font::Init(): NULL settings");
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    //
    // Get the Texture.
    //
    texture = pSettings->GetString( settingsPath + ".Texture" );
    CHR(TextureMan.Get( texture, &m_hTexture ));
    CHR(TextureMan.GetInfo( m_hTexture, &info ));
    m_textureWidth  = info.widthPixels;
    m_textureHeight = info.heightPixels;

    
    //
    // Parse the .FNT description.
    //
    fontfile = pSettings->GetString( settingsPath + ".FNTFile" );
    CHR(InitFromFNTFile( fontfile ));


    RETAILMSG(ZONE_ANIMATION, "Font[%4d]: \"%-32s\" texture: \"%s\" numchars: %d", 
        m_ID, m_name.c_str(), texture.c_str(), m_numFontChars);
    
Exit:
    return rval;
}



RESULT
Font::InitFromFNTFile( IN const string& filename )
{
    RESULT  rval = S_OK;
    HFile   hFile;
    UINT32  bytesRead;
    UINT32  fontIndex = 0;
    string  line(1024, ' ');
    string  temp;
    string  recordType;
    
    CHR(FileMan.OpenFile( filename, &hFile ));

    while (SUCCEEDED(FileMan.ReadLine( hFile, (BYTE*)line.data(), 1024, &bytesRead )))
    {
        string::size_type end = line.find(" ", 0);
        if (string::npos == end)
            continue;
        
        recordType = line.substr(0, end);

        if ("char" == recordType)
        {
            char    c       = Util::FindValueForKey   ( line, "letter" )[1];
            UINT32  width   = Util::FindIntValueForKey( line, "width"  );
            UINT32  height  = Util::FindIntValueForKey( line, "height"  );

            int     x       = Util::FindIntValueForKey( line, "x" );
            int     y       = Util::FindIntValueForKey( line, "y" );

            vec2    bearing;
            bearing.x       = Util::FindIntValueForKey( line, "xoffset" );
            bearing.y       = Util::FindIntValueForKey( line, "yoffset" );
            
            vec2    advance;
            advance.x       = Util::FindIntValueForKey( line, "xadvance" );

            // FontChars live in an array sorted by the ASCII value of the character.
            // The first printable ASCII character is '!' at index 0.
            fontIndex = c - FIRST_PRINTABLE_GLYPH;
            if (!c || fontIndex >= MAX_PRINTABLE_GLYPHS)
            {
                RETAILMSG(ZONE_WARN, "Font::InitFromFNTFile( \"%s\" ): out-of-range ASCII value %d", m_name.c_str(), c);
                DEBUGCHK(0);
                continue;
            }
            

            m_pFontChars[ fontIndex ].c       = c;
            m_pFontChars[ fontIndex ].width   = width;
            m_pFontChars[ fontIndex ].height  = height;
            m_pFontChars[ fontIndex ].bearing = bearing;
            m_pFontChars[ fontIndex ].advance = advance;
            m_pFontChars[ fontIndex ].u0      = (float)x/(float)m_textureWidth;
            m_pFontChars[ fontIndex ].v0      = (float)y/(float)m_textureHeight;
            
            
            RETAILMSG(ZONE_FONT | ZONE_VERBOSE, "'%c' %dx%d bearing: (%2.2f,%2.2f) adv: %2.2f uv: (%2.5f, %2.5f)",
                c, width, height, bearing.x, bearing.y, advance.x, (float)x/(float)m_textureWidth, (float)y/(float)m_textureHeight);

        }
        else if ("info" == recordType)
        {
            temp = Util::FindValueForKey( line, "face" );
            RETAILMSG(ZONE_FONT, "font face = %s", temp.c_str());
        }
        else if ("common" == recordType)
        {
            m_lineHeight    = Util::FindFloatValueForKey( line, "lineHeight" );
            m_textureWidth  = Util::FindFloatValueForKey( line, "scaleW"     );
            m_textureHeight = Util::FindFloatValueForKey( line, "scaleH"     );
            RETAILMSG(ZONE_FONT, "m_lineHeight:    %4.2f", m_lineHeight      );
            RETAILMSG(ZONE_FONT, "m_textureWidth:  %4.0f", m_textureWidth    );
            RETAILMSG(ZONE_FONT, "m_textureHeight: %4.0f", m_textureHeight   );
        }
        else if ("page" == recordType)
        {
            temp = Util::FindValueForKey( line, "file" );
            RETAILMSG(ZONE_FONT, "file=%s", temp.c_str());
        }
        else if ("chars" == recordType)
        {
            m_numFontChars = Util::FindIntValueForKey( line, "count" );
            RETAILMSG(ZONE_FONT, "m_numFontChars: %d", m_numFontChars);
            
            DEBUGCHK(NULL == m_pFontChars);
            //m_pFontChars = new FontChar[ m_numFontChars ];
            m_pFontChars = new FontChar[ MAX_PRINTABLE_GLYPHS ];
            memset(m_pFontChars, 0, sizeof(FontChar)*m_numFontChars);
        }
        
    }

Exit:
    IGNOREHR(FileMan.CloseFile( hFile ));
    return rval;
}



float
Font::GetWidth( IN const string& text )
{
    float       totalWidth  = 0.0f;
    FontChar*   pFontChar   = NULL;
    
    const char* pCharacters = text.c_str();
    
    while( *pCharacters )
    {
        // Convert from ASCII to table index.
        UINT8 i = *pCharacters - FIRST_PRINTABLE_GLYPH;
        
        if (i >= m_numFontChars)
        {
            // TODO: use a box or something for characters not supported by this font.
            i = 0;
        }
        
        pFontChar = &m_pFontChars[i];
        DEBUGCHK(pFontChar);
        
        float width = pFontChar->width;
        
        // The ' ' (space) character in a .FNT has zero-width,
        // so use the advance metric instead.
        if (0 == width)
        {
            width = pFontChar->advance.x;
        }
    
        totalWidth += width;
        pCharacters++;
    }
        
    return totalWidth;
}



RESULT
Font::Draw( IN const vec2& position, IN const string& characters, Color color, float scale, float opacity, float rotX, float rotY, float rotZ, bool worldSpace )
{
    RESULT      rval = S_OK;
    const char* str;

    str = characters.c_str();
    CHR(Draw( position, str, color, scale, opacity, worldSpace ));

Exit:
    return rval;
}


//
//
// TODO: RenderBatch
//
//


RESULT
Font::Draw( IN const vec2& position, IN const char* pCharacters, Color color, float scale, float opacity, float rotX, float rotY, float rotZ, bool worldSpace )
{
    RESULT      rval = S_OK;
    UINT32      length;
    Vertex*     pVertices;
    Vertex*     pQuad;
    FontChar*   pFontChar;
    vec2        cursorPos;
    mat4        modelview;
//    mat4        camera = GameCamera.GetWorldViewMatrix();
//    mat4        camera = GameCamera.GetViewMatrix();

    CPR(pCharacters);

    length = strlen(pCharacters);
    if (0 == length)
        return S_OK;


    
    // Skip drawing text w/ zero opacity.
    if (Util::CompareFloats(opacity, 0.0f))
        return rval;

    //
    // Transform the text based on scale, rotation, position.
    //
    modelview  = mat4::Scale( scale );
    modelview *= mat4::RotateX( rotX );
    modelview *= mat4::RotateY( rotY );
    modelview *= mat4::RotateZ( rotZ );
    modelview *= mat4::Translate( position.x, position.y, 1.0 );

    // Is the font being drawn in world space (3D) ?
    // Otherwise, draw in screen space.
    if (worldSpace)
    {
        modelview *= GameCamera.GetViewMatrix();
    }


    // Allocate a vertex buffer
    // TODO: VBOs faster?
    pVertices = new Vertex[ length * VERTS_PER_GLYPH ];
    CPREx(pVertices, E_OUTOFMEMORY);
    memset(pVertices, 0, sizeof(Vertex)*length);
    pQuad = pVertices;

    cursorPos = vec2(0,0);

    while( *pCharacters )
    {
        // Convert from ASCII to table index.
        UINT8 i = *pCharacters - FIRST_PRINTABLE_GLYPH;
        
        if (i >= m_numFontChars)
        {
            // TODO: print a box or something for characters not supported by this font.
            i = 0;
        }

        pFontChar = &m_pFontChars[i];
        DEBUGCHK(pFontChar);
    
        // Convert from .FNT coordinates (assumes Y grows toward bottom of screen)
        // to OpenGL ES coordinates (Y grows towards top of screen).
        float xoffset = pFontChar->bearing.x;
        float yoffset = pFontChar->bearing.y;
        float width   = pFontChar->width;
        float height  = pFontChar->height;
        
        // The ' ' (space) character in a .FNT has zero-width,
        // so use the advance metric instead.
        if (0 == width)
        {
            width = pFontChar->advance.x;
        }

        
        Point2D glyphOrigin;
        glyphOrigin.x = cursorPos.x;
        glyphOrigin.y = cursorPos.y + m_lineHeight;
        
        float left, top, right, bottom;
        left    = glyphOrigin.x + xoffset;
        top     = glyphOrigin.y - yoffset;
        right   = left + width;
        bottom  = top  - height;

        Rectangle rect = { left, bottom, width, height };

        float uStart = pFontChar->u0;
        float uEnd   = uStart + (pFontChar->width / m_textureWidth);
        float vStart = pFontChar->v0;
        float vEnd   = vStart + (pFontChar->height / m_textureHeight );

        CHR(Util::CreateTriangleList( &rect, 1, 1, pQuad, uStart, uEnd, vStart, vEnd ));
 
        for (int i = 0; i < 6; ++i)
        {
            pQuad[i].color  = color;
            pQuad[i].a      = (BYTE)(255 * opacity);
        }
        
        
/*
        // TEST
        vec3 min( pQuad[0].x, pQuad[0].y, pQuad[0].z );
        vec3 max( pQuad[5].x, pQuad[5].y, pQuad[5].z );
        min = modelview * min;
        max = modelview * max;
        AABB bounds( min, max );
        DebugRender.Quad( bounds, Color::Green(), 0.2f );
*/        

        cursorPos += pFontChar->advance;
        pQuad     += VERTS_PER_GLYPH;
        pCharacters++;
    }
    

    
//    CHR(Renderer.PushEffect( m_hEffect ));
    CHR(Renderer.SetModelViewMatrix( modelview ));
    CHR(Renderer.SetTexture( 0, m_hTexture ));
    CHR(Renderer.DrawTriangleList( pVertices, length * VERTS_PER_GLYPH ));

/*
    // TEST: 
    DebugRender.Draw();
*/

Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Font::Draw(): 0x%x", rval);
        DEBUGCHK(0);
    }

//    CHR(Renderer.PopEffect());
    SAFE_ARRAY_DELETE(pVertices);

    return rval;
}



} // END namespace Z


