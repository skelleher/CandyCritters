#pragma once

#include "ResourceManager.hpp"
#include "Handle.hpp"
#include "IRenderer.hpp"
#include "Object.hpp"
#include "Settings.hpp"
#include "TextureManager.hpp"
#include "IDrawable.hpp"
#include "Font.hpp"

#include <string>
using std::string;

    

namespace Z
{


class FontManager : public ResourceManager<Font>
{
public:
    static  FontManager& Instance();

    virtual RESULT  Init        ( IN const string& settingsFilename );
    virtual RESULT  Shutdown    ( );

    RESULT          Draw        ( IN const vec2& position, IN const string& characters,  IN HFont hFont = HFont::NullHandle(), Color color = Color::White(), float scale = 1.0f, float opacity = 1.0f, float rotX = 0, float rotY = 0, float rotZ = 0, bool worldSpace = false );
    RESULT          Draw        ( IN const vec2& position, IN const char*   pCharacters, IN HFont hFont = HFont::NullHandle(), Color color = Color::White(), float scale = 1.0f, float opacity = 1.0f, float rotX = 0, float rotY = 0, float rotZ = 0, bool worldSpace = false );
    
    float           GetHeight   ( IN HFont hFont );
    float           GetWidth    ( IN HFont hFont, IN const string& text );
    float           GetHeight   ( IN const string& name );
    float           GetWidth    ( IN const string& name, IN const string& text );
    
protected:
    FontManager();
    FontManager( const FontManager& rhs );
    FontManager& operator=( const FontManager& rhs );
    virtual ~FontManager();
 
    RESULT  CreateFont( IN const Settings* pSettings, IN const string& settingsPath, INOUT Font** ppFont );
  
    
protected:
    HFont       m_hDefaultFont;
    
          
/*
protected:
    // We create one FontBatch for each Material / texture atlas combo
    // between BeginBatch() and EndBatch().
    // Then we batch up all Fonts for each Material / texture atlas into a vertex array
    // and submit them at once.

    struct BatchedFont
    {
        Font*     pFont;
        float       opacity;
        float       scale;
        vec3        rotation;
        vec3        position;
    };
    
    typedef     vector<BatchedFont*>      BatchedFonts;
    typedef     BatchedFonts::iterator    BatchedFontsIterator;

    // A batch of Fonts, which share the same Material and texture atlas.
    struct FontBatch
    {
//        const string&   textureName;
        HMaterial       hMaterial;
        HTexture        hTexture;
        UINT32          textureID;
        UINT32          numFonts;
        UINT32          numVertices;
        Vertex*         pVertices;
        BatchedFonts  Fonts;
    };
    

    // Key for our map of FontBatches.
    // They are sorted by Material and by texture atlas.
    class FontBatchKey
    {
    public:
        FontBatchKey() {}
        FontBatchKey( long unsigned int x) {}
        FontBatchKey(const int A, const int B) :
            hMaterial(), textureID(B) {}
        
        HMaterial   hMaterial;
        UINT32      textureID;
        
        // std::map requires that Keys be sortable using the less-than operator.
        bool operator<(const FontBatchKey &rhs) const { if (hMaterial == rhs.hMaterial) { return textureID < rhs.textureID; } else { return hMaterial < rhs.hMaterial; } }
    };
    
//    typedef     map<UINT32, FontBatch*>   FontBatchMap;
    typedef     map<FontBatchKey, FontBatch*>   FontBatchMap;
    typedef     FontBatchMap::iterator            FontBatchMapIterator;

    FontBatchMap  m_FontBatchMap;
    bool            m_inFontBatch;
*/    
};

#define FontMan ((FontManager&)FontManager::Instance())



} // END namespace Z



