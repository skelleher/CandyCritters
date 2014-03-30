#include "DebugRenderer.hpp"
#include "Engine.hpp"
#include "Log.hpp"


namespace Z
{


// Static data
DebugRenderer* DebugRenderer::s_pInstance = NULL;
    
DebugRenderer::DebugRenderList  DebugRenderer::s_debugRenderList;
HEffect                         DebugRenderer::s_hDebugEffect;
HFont                           DebugRenderer::s_hDebugFont;
HTexture                        DebugRenderer::s_hDebugQuadTexture;
WORLD_POSITION                  DebugRenderer::s_textCursorPos;
WORLD_POSITION                  DebugRenderer::s_textCursorOrigin;

DebugRenderer::DebugRenderer()  
{
    // EffectMan is always up and running before the renderer(s), so
    // this should generally be safe to do in the ctor.
    EffectMan.Get( "DefaultEffect", &s_hDebugEffect );
    FontMan.Get( "DebugFont", &s_hDebugFont );

    // We use point sprites for drawing debug quads, so it can't be part of a
    // TextureAtlas.  
    // TODO: point sprites confer no advantage; just send down a triangle strip.
    TextureMan.CreateFromFile( "/app/textures/DebugQuad.png", &s_hDebugQuadTexture);

    //
    // If caller doesn't specify a position we render text starting at upper-left corner
    // of the screen, with each ::Text() call on a new line.
    //
    Rectangle screenRect;
    Platform::GetScreenRectCamera( &screenRect );
    float fontHeight = FontMan.GetHeight( s_hDebugFont );
    s_textCursorOrigin = WORLD_POSITION(0, fontHeight, 0 );
    
    s_textCursorPos = s_textCursorOrigin;
}


DebugRenderer::~DebugRenderer() 
{
    EffectMan.Release( s_hDebugEffect );
}


DebugRenderer&
DebugRenderer::Instance()
{
    if (!s_pInstance) 
    {
        s_pInstance = new DebugRenderer();
    }
    
    return *s_pInstance;
}



RESULT
DebugRenderer::Line( const WORLD_POSITION& pos, const vec3& direction, Color color, float fOpacity, float fLength, float fWidth )
{
    DebugRenderItem item;
    
    item.type       = DEBUG_RENDER_ITEM_LINE;
    item.pos        = pos;
    item.direction  = direction;
    item.color      = color;
    item.fOpacity   = fOpacity;
    item.fLength    = fLength;
    item.fWidth     = fWidth;
    
    item.fields = (DebugRenderItemFields) (DRIF_TYPE | DRIF_POSITION | DRIF_DIRECTION | DRIF_COLOR | DRIF_OPACITY | DRIF_LENGTH | DRIF_WIDTH);
    
    s_debugRenderList.push_back( item );
    
    return S_OK;
}



RESULT
DebugRenderer::Text( const WORLD_POSITION& pos, const string& text, Color color, float fOpacity, float fScale )
{
    DebugRenderItem item;
    
    item.type       = DEBUG_RENDER_ITEM_TEXT;
    item.pos        = pos;
    item.text       = text;
    item.color      = color;
    item.fOpacity   = fOpacity;
    item.fScale     = fScale;
    
    item.fields = (DebugRenderItemFields) (DRIF_TYPE | DRIF_POSITION | DRIF_TEXT | DRIF_COLOR | DRIF_OPACITY | DRIF_SCALE);
    
    s_debugRenderList.push_back( item );

    return S_OK;
}



RESULT
DebugRenderer::Text( const string& text, Color color, float fOpacity, float fScale )
{
    DebugRenderItem item;
    
    item.type       = DEBUG_RENDER_ITEM_TEXT;
    item.pos        = s_textCursorPos;
    item.text       = text;
    item.color      = color;
    item.fOpacity   = fOpacity;
    item.fScale     = fScale;
    
    item.fields = (DebugRenderItemFields) (DRIF_TYPE | DRIF_POSITION | DRIF_TEXT | DRIF_COLOR | DRIF_OPACITY | DRIF_SCALE);
    
    s_debugRenderList.push_back( item );

    // Move cursor to next line.
    s_textCursorPos.y += FontMan.GetHeight( s_hDebugFont );


    return S_OK;
}




RESULT
DebugRenderer::Quad( const WORLD_POSITION& pos, Color color, float fOpacity, float fScale )
{
    DebugRenderItem item;
    
    item.type       = DEBUG_RENDER_ITEM_QUAD;
    item.pos        = pos;
    item.color      = color;
    item.fOpacity   = fOpacity;
    item.fScale     = fScale;
    
    item.fields = (DebugRenderItemFields) (DRIF_TYPE | DRIF_POSITION | DRIF_COLOR | DRIF_OPACITY | DRIF_SCALE);
    
    s_debugRenderList.push_back( item );

    return S_OK;
}



RESULT
DebugRenderer::Quad( const AABB& bounds, Color color, float fOpacity )
{
    RESULT rval = S_OK;

    // Draw screen-aligned 2D rectangular outline for the AABB.
    // TODO: have solid mode using a quad.
    
    WORLD_POSITION corner1;
    WORLD_POSITION corner2;
    WORLD_POSITION corner3;
    WORLD_POSITION corner4;

    corner1.x = bounds.GetMin().x;  corner1.y = bounds.GetMin().y;              corner1.z = 0.0f;
    corner2.x = bounds.GetMin().x;  corner2.y = corner1.y + bounds.GetHeight(); corner2.z = 0.0f;
    corner3.x = bounds.GetMax().x;  corner3.y = bounds.GetMax().y;              corner3.z = 0.0f;
    corner4.x = bounds.GetMax().x;  corner4.y = corner3.y - bounds.GetHeight(); corner4.z = 0.0f;

    CHR( Line(corner1, vec3(0,  1, 0),  Color::Green(), fOpacity, bounds.GetHeight(), 5.0f) );
    CHR( Line(corner2, vec3(1,  0, 0),  Color::Green(), fOpacity, bounds.GetWidth(),  5.0f) );
    CHR( Line(corner3, vec3(0, -1, 0),  Color::Green(), fOpacity, bounds.GetHeight(), 5.0f) );
    CHR( Line(corner4, vec3(-1, 0, 0),  Color::Green(), fOpacity, bounds.GetWidth(),  5.0f) );
    
//    CHR( Point( bounds.GetCenter(), Color::Black(), 1.0f ) );
    
Exit:
    return rval;
}



RESULT
DebugRenderer::Draw( )
{
    RESULT                  rval        = S_OK;
    DebugRenderListIterator pRenderItem;

    CHR(Renderer.PushEffect( s_hDebugEffect ));
    CHR(Renderer.SetModelViewMatrix( mat4::Identity() ));

    CHR(Renderer.EnableDepthTest ( true  ));
    CHR(Renderer.EnableLighting  ( false ));
    CHR(Renderer.EnableAlphaBlend( true  ));
    CHR(Renderer.EnableTexturing ( true  ));
    
    
    // TODO: moving draw batching from SpriteMan into a generic RenderBatch.
    // RenderBatch.Begin();
    // ...
    // RenderBatch.End();
    

    //
    // Render each queued DebugRenderItem.
    //
    for (pRenderItem = s_debugRenderList.begin(); pRenderItem != s_debugRenderList.end(); ++pRenderItem)
    {
        DebugRenderItem* pItem = &(*pRenderItem);
        if (!pItem || !(pItem->fields & DRIF_TYPE))
        {
            continue;
        }
                
        switch (pItem->type)
        {
            case DEBUG_RENDER_ITEM_LINE:
                {
                    DEBUGCHK(pItem->fields & (DRIF_COLOR | DRIF_POSITION | DRIF_LENGTH | DRIF_OPACITY));
                
                    Vertex vertices[2];
                    vertices[0].color   = pItem->color;
                    vertices[1].color   = pItem->color;
                    
                    vertices[0].x       = pItem->pos.x;
                    vertices[0].y       = pItem->pos.y;
                    vertices[0].z       = pItem->pos.z;
                    
                    vec3 endPoint       = pItem->pos + (pItem->direction * pItem->fLength);
                    vertices[1].x       = endPoint.x;
                    vertices[1].y       = endPoint.y;
                    vertices[1].z       = endPoint.z;
                        
                    Renderer.DrawLines( vertices, 2, pItem->fWidth );
                }
                break;
                
            case DEBUG_RENDER_ITEM_TEXT:
                {
                    DEBUGCHK(pItem->fields & (DRIF_POSITION | DRIF_TEXT | DRIF_COLOR | DRIF_OPACITY));

                    vec2 pos2D(pItem->pos.x, pItem->pos.y);
                    FontMan.Draw( pos2D, pItem->text, s_hDebugFont, pItem->color, pItem->fScale, pItem->fOpacity );
                }
                break;
                
            case DEBUG_RENDER_ITEM_QUAD:
                {
                    DEBUGCHK(pItem->fields & (DRIF_POSITION | DRIF_COLOR | DRIF_OPACITY));

                    Vertex pointSprite;
                    
                    pointSprite.color   = pItem->color * pItem->fOpacity;
                    pointSprite.x       = pItem->pos.x;
                    pointSprite.y       = pItem->pos.y;
                    pointSprite.z       = pItem->pos.z;
                    pointSprite.u0      = 0;
                    pointSprite.v0      = 0;
                    
                    Renderer.SetTexture(0, s_hDebugQuadTexture);
                    Renderer.DrawPointSprites( &pointSprite, 1, pItem->fScale );
                }
                break;
                
            default:
                RETAILMSG(ZONE_ERROR, "ERROR: DebugRenderer::Draw(): invalid render item type 0x%x", pItem->type);
        }
    }


Exit:
    IGNOREHR(Renderer.PopEffect());
    return rval;
}



RESULT
DebugRenderer::Reset( )
{
    s_debugRenderList.clear();
    s_textCursorPos = s_textCursorOrigin;
    
    return S_OK;
}



} // END namespace Z