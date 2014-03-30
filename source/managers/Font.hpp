#pragma once

#include "Types.hpp"
#include "Handle.hpp"
#include "Object.hpp"
#include "Settings.hpp"
#include "TextureManager.hpp"
#include "EffectManager.hpp"

#include <string>
using std::string;


namespace Z
{



//=============================================================================
//
// A Font instance.
//
//=============================================================================
class Font : virtual public Object
{
public:
    Font();
    virtual ~Font();
    
    RESULT          Init                ( IN const string& name, IN const Settings* pSettings, IN const string& settingsPath );
    RESULT          Draw                ( IN const vec2& position, IN const string& characters,  Color color = Color::White(), float scale = 1.0f, float opacity = 1.0f, float rotX = 0, float rotY = 0, float rotZ = 0, bool worldSpace = false /* TODO: custom layout function, e.g. cos, sin, circle */ );
    RESULT          Draw                ( IN const vec2& position, IN const char*   pCharacters, Color color = Color::White(), float scale = 1.0f, float opacity = 1.0f, float rotX = 0, float rotY = 0, float rotZ = 0, bool worldSpace = false /* TODO: custom layout function, e.g. cos, sin, circle */ );

    inline float    GetHeight           ( )                             { return m_lineHeight; }
    float           GetWidth            ( IN const string& text );

protected:
    RESULT          InitFromFNTFile     ( IN const string& filename );
    

protected:
    struct FontChar
    {
        char        c;
        float       width;
        float       height;
        vec2        bearing;
        vec2        advance;
        float       u0;
        float       v0;
    };

protected:
    HTexture    m_hTexture;
//    HEffect     m_hEffect;
    UINT32      m_numFontChars;
    FontChar*   m_pFontChars;
    float       m_lineHeight;
    float       m_textureHeight;
    float       m_textureWidth;
};

typedef Handle<Font> HFont;



} // END namespace Z
