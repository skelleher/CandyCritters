#pragma once


#include <string>
#include "tinyxml.h"
#include "Macros.hpp"
#include "Errors.hpp"
#include "Color.hpp"
#include "Vector.hpp"


using std::string;


namespace Z
{


// How deep within an XML document we can parse by name
// e.g. "/root/child/foo/bar/baz.attribute"
#define MAX_DOM_PATH    1024


class Settings
{
public:
    // Return "global settings" singleton
    // TODO: this is hacky.  The app should own instance creation.
    static Settings& Global();
    
    // Return "user settings" singleton.
    // TODO: this is hacky.  The app should own instance creation.
    static Settings& User();
    
    
public:
    Settings();
    virtual ~Settings();

    RESULT          Read        ( const string& filename );
    RESULT          Write       ( const string& filename );
    RESULT          Write       ();
    RESULT          SetFilename ( const string& filename );
    const string&   GetFilename () const;

    string          GetString   ( const string& name, const string&      def = ""                               ) const;
    bool            GetBool     ( const string& name,       bool         def = false                            ) const;
    float           GetFloat    ( const string& name,       float        def = 0.0f                             ) const;
    int             GetInt      ( const string& name,       int          def = 0                                ) const;
   
    vec2            GetVec2     ( const string& name, const vec2&        def = vec2(0.0f, 0.0f)                 ) const;
    vec3            GetVec3     ( const string& name, const vec3&        def = vec3(0.0f, 0.0f, 0.0f)           ) const;
    vec4            GetVec4     ( const string& name, const vec4&        def = vec4(0.0f, 0.0f, 0.0f, 0.0f)     ) const;
 
    Color           GetColor    ( const string& name, const Color&       def = Color(0.0f, 0.0f, 0.0f, 0.0f)    ) const;

    RESULT          SetString   ( const string& name, string      value );
    RESULT          SetBool     ( const string& name, bool        value );
    RESULT          SetFloat    ( const string& name, float       value );
    RESULT          SetInt      ( const string& name, int         value );
    RESULT          SetVec2     ( const string& name, vec2        value );
    RESULT          SetVec3     ( const string& name, vec3        value );
    RESULT          SetVec4     ( const string& name, vec4        value );
    RESULT          SetColor    ( const string& name, Color       value );

protected:
    void            DumpAttributes   ( TiXmlElement* pElement, unsigned int indent ) const;
    TiXmlElement*   FindElement      ( const string& path ) const;
    TiXmlAttribute* FindAttribute    ( const string& path ) const;

    
protected:
    static Settings* s_pGlobalSettingsInstance;
    static Settings* s_pUserSettingsInstance;
    
protected:
    string          m_filename;

    TiXmlDocument*  m_pXMLDocument;
    TiXmlHandle     m_hXMLDocument;
    TiXmlHandle     m_hRoot;
};


#define GlobalSettings Settings::Global()
#define UserSettings   Settings::User()

} // END namespace Z

