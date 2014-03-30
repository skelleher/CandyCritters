#pragma once
#include "Types.hpp"
#include "Color.hpp"
#include "EffectManager.hpp"
#include "FontManager.hpp"

#include <string>
#include <vector>
using std::string;
using std::vector;


namespace Z
{

//
// This class provides a toolkit of debug rendering primitives:
//  lines, boxes, spheres, billboards, etc.
//
// Items drawn with DebugRender paint on top of everything else in the scene.
//
class DebugRenderer
{
public:
    static  DebugRenderer& Instance();


    RESULT  Line    ( const WORLD_POSITION& pos, const vec3&   direction,   Color color = Color::Green(), float fOpacity = 1.0f, float fLength = 16.0f, float fWidth = 10.0f );

    RESULT  Quad    ( const WORLD_POSITION& pos,                            Color color = Color::Green(), float fOpacity = 1.0f, float fScale  = 64.0f );
    RESULT  Quad    ( const AABB& bounds,                                   Color color = Color::Green(), float fOpacity = 1.0f );

    RESULT  Text    ( const WORLD_POSITION& pos, const string& text,        Color color = Color::Green(), float fOpacity = 1.0f, float fScale  = 1.0f  );
    RESULT  Text    ( const string& text,Color color = Color::Green(), float fOpacity = 1.0f, float fScale  = 1.0f );
    

    // Call this to render the queued items, after rendering the primary scene but possibly before rendering the UI.
    RESULT  Draw    ( );
    RESULT  Reset   ( );

protected:
    DebugRenderer();
    DebugRenderer( const DebugRenderer& rhs );
    DebugRenderer& operator=( const DebugRenderer& rhs );
    virtual ~DebugRenderer();
    
protected:
    //
    // DebugRender calls are packaged into
    // DebugRenderItems and queued for later drawing.
    //
    enum DebugRenderItemType
    {
        DEBUG_RENDER_ITEM_LINE  = 1,
        DEBUG_RENDER_ITEM_TEXT,
        DEBUG_RENDER_ITEM_QUAD,
    };
    
    // Mask for which fields have been set.
    enum DebugRenderItemFields
    {
        DRIF_TYPE       = BIT0,
        DRIF_POSITION   = BIT1,
        DRIF_DIRECTION  = BIT2,
        DRIF_TEXT       = BIT3,
        DRIF_COLOR      = BIT4,
        DRIF_OPACITY    = BIT5,
        DRIF_SCALE      = BIT6,
        DRIF_LENGTH     = BIT7,
        DRIF_HEIGHT     = BIT8,
        DRIF_WIDTH      = BIT9,
    };
    
    struct DebugRenderItem
    {
        // Fields
        DebugRenderItemFields   fields;
        DebugRenderItemType     type;
        WORLD_POSITION          pos;
        vec3                    direction;
        string                  text;
        Color                   color;
        float                   fOpacity;
        float                   fScale;
        float                   fLength;
        float                   fHeight;
        float                   fWidth;
    };
    
    
    typedef std::vector<DebugRenderItem>    DebugRenderList;
    typedef DebugRenderList::iterator       DebugRenderListIterator;
    
    static DebugRenderer*       s_pInstance;
    static DebugRenderList      s_debugRenderList;  // TODO: separate lists for lines, point sprites, etc?  Batch them together.
    static HEffect              s_hDebugEffect;
    static HFont                s_hDebugFont;
    static HTexture             s_hDebugQuadTexture;
    static WORLD_POSITION       s_textCursorOrigin;
    static WORLD_POSITION       s_textCursorPos;
};


#define DebugRender ((DebugRenderer&)DebugRenderer::Instance())




} // END namespace Z