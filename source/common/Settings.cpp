#include "Settings.hpp"
#include "FileManager.hpp" // for ease, taking a dependency on FileManager for ::Read( "/app/relative/path/file" )

#include <string>
using std::string;


namespace Z
{



//
// Static Data
// 
Settings*   Settings::s_pGlobalSettingsInstance = NULL;
Settings*   Settings::s_pUserSettingsInstance   = NULL;


//
// Static Methods
//
Settings& 
Settings::Global()
{
    // TODO: make this thread-safe with an auto-lock?
    
    if (!s_pGlobalSettingsInstance)
    {
        s_pGlobalSettingsInstance = new Settings();
    }
    
    return *s_pGlobalSettingsInstance;
}




Settings& 
Settings::User()
{
    // TODO: make this thread-safe with an auto-lock?
    
    if (!s_pUserSettingsInstance)
    {
        s_pUserSettingsInstance = new Settings();
    }
    
    return *s_pUserSettingsInstance;
}




//
// Instance Methods
//

Settings::Settings () :
    m_filename(""),
    m_pXMLDocument(NULL),
    m_hXMLDocument(NULL),
    m_hRoot(NULL)
{
}


Settings::~Settings()
{
    if (m_pXMLDocument)
        m_pXMLDocument->Clear();
        
    SAFE_DELETE( m_pXMLDocument  );
}


RESULT
Settings::Read( const string& filename )
{
    RESULT rval = S_OK;
    
    m_filename = filename;
    
    HFile  hFile;
    string absolutePath;
    CHR(FileMan.OpenFile( m_filename, &hFile ));
    CHR(FileMan.GetAbsolutePath( hFile, &absolutePath ));
    
    m_pXMLDocument = new TiXmlDocument( absolutePath.c_str() );
	if ( !m_pXMLDocument->LoadFile() ) 
    {
        DEBUGMSG( ZONE_ERROR, "ERROR: Settings::Read(): bad file [%s]", m_filename.c_str());
        DEBUGMSG( ZONE_ERROR, "%s at %d,%d", 
            m_pXMLDocument->ErrorDesc(), 
            m_pXMLDocument->ErrorRow(),
            m_pXMLDocument->ErrorCol() );
        return E_FILE_NOT_FOUND;
    }
    m_hXMLDocument = TiXmlHandle( m_pXMLDocument );

    TiXmlElement* pRoot;
	pRoot = m_hXMLDocument.FirstChildElement().Element();
	if (!pRoot)
    {
        DEBUGMSG( ZONE_ERROR, "ERROR: Settings::Read(): [%s] has no root", m_filename.c_str());
        return E_BAD_FILE_FORMAT;
    }

    m_hRoot = TiXmlHandle( pRoot );

    DEBUGMSG( ZONE_INFO, "\n");
    DEBUGMSG( ZONE_INFO, "Settings loaded from [%s]", m_filename.c_str());
    DEBUGMSG( ZONE_INFO, "---------------------");
    
    // TODO: validate that XML file contains our game settings
    
Exit:
    FileMan.CloseFile( hFile );
    
    if (!SUCCEEDED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Settings failed to load from [%s]", m_filename.c_str());
    }
    
    return rval;
}


RESULT
Settings::Write( const string& filename )
{
    RESULT rval = S_OK;

    m_filename = filename;

    HFile  hFile;
    string absolutePath;
    CHR(FileMan.OpenFile( m_filename, &hFile, FileManager::WRITE ));
    CHR(FileMan.GetAbsolutePath( hFile, &absolutePath ));


    if ( !m_pXMLDocument->SaveFile( absolutePath.c_str() ) )
    {
        DEBUGMSG( ZONE_ERROR, "ERROR: Settings::Write(): bad file [%s]", m_filename.c_str());
        DEBUGMSG( ZONE_ERROR, "%s at %d,%d", 
                 m_pXMLDocument->ErrorDesc(), 
                 m_pXMLDocument->ErrorRow(),
                 m_pXMLDocument->ErrorCol() );
                 
        return E_FAIL;
    }
    
    DEBUGMSG( ZONE_INFO, "\n");
    DEBUGMSG( ZONE_INFO, "Settings written to [%s]", m_filename.c_str());
    DEBUGMSG( ZONE_INFO, "---------------------");
    

Exit:
    FileMan.CloseFile( hFile );

    if (!SUCCEEDED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: Settings failed to write to [%s]", m_filename.c_str());
    }
    
    return rval;
}


RESULT
Settings::Write( )
{
    RESULT rval = S_OK;
    
    rval = Write( m_filename );
    
Exit:
    return rval;
}



RESULT
Settings::SetFilename( const string& filename )
{
    m_filename = filename;
    
    return S_OK;
}


const string&  
Settings::GetFilename() const
{
    return m_filename;
}



string
Settings::GetString( const string& name, const string& def ) const
{
    const char* rval = def.c_str();
    string path;
    string attribute;
    int    indexOfSeparator;
    
    DEBUGCHK(name.c_str());

    indexOfSeparator    = name.find_last_of('.');
    path                = name.substr( 0,                    indexOfSeparator );
    attribute           = name.substr( indexOfSeparator+1,   name.length()    );

    TiXmlElement* pElement = FindElement( path );
    if (pElement)
    {
        const char* pTemp = pElement->Attribute( attribute.c_str() );
        if ( !pTemp )
        {
            rval = def.c_str();
        }
        else
        {
            rval = pTemp;
        }
    }

    DEBUGMSG( ZONE_SETTINGS, "[%s] = [%s]", name.c_str(), rval);

    return rval;
}


bool
Settings::GetBool( const string& name, const bool def ) const
{
    bool   rval = def;
    int    temp;
    string path;
    string attribute;
    int    indexOfSeparator;

    indexOfSeparator    = name.find_last_of('.');
    path                = name.substr( 0,                    indexOfSeparator );
    attribute           = name.substr( indexOfSeparator+1,   name.length()    );
    
    TiXmlElement* pElement = FindElement( path );
    if (pElement)
    {
        if ( TIXML_NO_ATTRIBUTE == pElement->QueryIntAttribute( attribute.c_str(), &temp ) )
        {
            rval = def;
        }
        else
        {
            rval = (bool)temp;
        }
    }

    DEBUGMSG( ZONE_SETTINGS, "[%s] = %s", name.c_str(), rval ? "true" : "false");
    
    return rval;
}


float
Settings::GetFloat( const string& name, float def ) const
{
    float  rval = def;
    double temp;
    string path;
    string attribute;
    int    indexOfSeparator;

    indexOfSeparator    = name.find_last_of('.');
    path                = name.substr( 0,                    indexOfSeparator );
    attribute           = name.substr( indexOfSeparator+1,   name.length()    );
    
    TiXmlElement* pElement = FindElement( path );
    if (pElement)
    {
        if ( TIXML_NO_ATTRIBUTE == pElement->QueryDoubleAttribute( attribute.c_str(), &temp ) )
        {
            rval = def;
        }
        else
        {
            rval = (float)temp;
        }
    }

    DEBUGMSG( ZONE_SETTINGS, "[%s] = %f", name.c_str(), (float)rval);

    return (float)rval;
}


int
Settings::GetInt( const string& name, int def ) const
{
    int    rval = def;
    string path;
    string attribute;
    int    indexOfSeparator;

    indexOfSeparator    = name.find_last_of('.');
    path                = name.substr( 0,                    indexOfSeparator );
    attribute           = name.substr( indexOfSeparator+1,   name.length()    );
    
    TiXmlElement* pElement = FindElement( path );
    if (pElement)
    {
        if ( TIXML_NO_ATTRIBUTE == pElement->QueryIntAttribute( attribute.c_str(), &rval ) )
        {
            rval = def;
        }
    }

    DEBUGMSG( ZONE_SETTINGS, "[%s] = %d", name.c_str(), rval);
    
    return rval;
}


vec2
Settings::GetVec2( const string& name, const vec2& def ) const
{
    vec2   rval = def;
    string path;
    string attribute;
    int    indexOfSeparator;

    indexOfSeparator    = name.find_last_of('.');
    path                = name.substr( 0,                    indexOfSeparator );
    attribute           = name.substr( indexOfSeparator+1,   name.length()    );
    
    TiXmlElement* pElement = FindElement( path );
    if (pElement)
    {
        const char* pString = pElement->Attribute( attribute.c_str() );
        if ( !pString )
        {
            rval = def;
        }
        else
        {
            float x, y;
            sscanf( pString, "(%f, %f)", &x, &y );
            rval = vec2( x, y );
        }
    }

    DEBUGMSG( ZONE_SETTINGS, "[%s] = (%f, %f)", name.c_str(), rval.x, rval.y);
 
    return rval;
}



vec3
Settings::GetVec3( const string& name, const vec3& def ) const
{
    vec3   rval = def;
    string path;
    string attribute;
    int    indexOfSeparator;

    indexOfSeparator    = name.find_last_of('.');
    path                = name.substr( 0,                    indexOfSeparator );
    attribute           = name.substr( indexOfSeparator+1,   name.length()    );
    
    TiXmlElement* pElement = FindElement( path );
    if (pElement)
    {
        const char* pString = pElement->Attribute( attribute.c_str() );
        if ( !pString )
        {
            rval = def;
        }
        else
        {
            float x, y, z;
            sscanf( pString, "(%f, %f, %f)", &x, &y, &z );
            rval = vec3( x, y, z );
        }
    }

    DEBUGMSG( ZONE_SETTINGS, "[%s] = (%f, %f, %f)", name.c_str(), rval.x, rval.y, rval.z);
 
    return rval;
}



vec4
Settings::GetVec4( const string& name, const vec4& def ) const
{
    vec4   rval = def;
    string path;
    string attribute;
    int    indexOfSeparator;

    indexOfSeparator    = name.find_last_of('.');
    path                = name.substr( 0,                    indexOfSeparator );
    attribute           = name.substr( indexOfSeparator+1,   name.length()    );
    
    TiXmlElement* pElement = FindElement( path );
    if (pElement)
    {
        const char* pString = pElement->Attribute( attribute.c_str() );
        if ( !pString )
        {
            rval = def;
        }
        else
        {
            float x, y, z, w;
            sscanf( pString, "(%f, %f, %f, %f)", &x, &y, &z, &w );
            rval = vec4( x, y, z, w );
        }
    }

    DEBUGMSG( ZONE_SETTINGS, "[%s] = (%f, %f, %f, %f)", name.c_str(), rval.x, rval.y, rval.z, rval.w);
 
    return rval;
}
 


Color
Settings::GetColor( const string& name, const Color& def ) const
{
    Color  rval = def;
    string path;
    string attribute;
    int    indexOfSeparator;

    indexOfSeparator    = name.find_last_of('.');
    path                = name.substr( 0,                    indexOfSeparator );
    attribute           = name.substr( indexOfSeparator+1,   name.length()    );
    
    TiXmlElement* pElement = FindElement( path );
    if (pElement)
    {
        const char* pString = pElement->Attribute( attribute.c_str() );
        if ( !pString )
        {
            rval = def;
        }
        else
        {
            float x, y, z, w;
            sscanf( pString, "(%f, %f, %f, %f)", &x, &y, &z, &w );
            rval = Color( x, y, z, w );
        }
    }

    DEBUGMSG( ZONE_SETTINGS, "[%s] = (%fr, %fg, %fb, %fa)", name.c_str(), rval.floats.r, rval.floats.g, rval.floats.b, rval.floats.a);
 
    return rval;
}



RESULT
Settings::SetString( const string& name, string value )
{
    RESULT rval = S_OK;
    string path;
    string attribute;
    int    indexOfSeparator;
    
    DEBUGCHK(name.c_str());

    indexOfSeparator    = name.find_last_of('.');
    path                = name.substr( 0,                    indexOfSeparator );
    attribute           = name.substr( indexOfSeparator+1,   name.length()    );

    TiXmlElement* pElement = FindElement( path );
    if (pElement)
    {
        pElement->SetAttribute( attribute.c_str(), value.c_str() );
    }
    else 
    {
        // TODO: create the Element.
        DEBUGCHK(0);
        rval = E_NOT_FOUND;
    }


    DEBUGMSG( ZONE_SETTINGS, "[%s] = [%s]", name.c_str(), value.c_str());

    return rval;
}



RESULT
Settings::SetBool( const string& name, bool value )
{
    RESULT rval = S_OK;
    string path;
    string attribute;
    int    indexOfSeparator;
    
    DEBUGCHK(name.c_str());

    indexOfSeparator    = name.find_last_of('.');
    path                = name.substr( 0,                    indexOfSeparator );
    attribute           = name.substr( indexOfSeparator+1,   name.length()    );

    TiXmlElement* pElement = FindElement( path );
    if (pElement)
    {
        char temp[32];
        sprintf(temp, "%d", value);
        pElement->SetAttribute( attribute.c_str(), temp );
    }
    else 
    {
        // TODO: create the Element.
        DEBUGCHK(0);
        rval = E_NOT_FOUND;
    }


    DEBUGMSG( ZONE_SETTINGS, "[%s] = [%d]", name.c_str(), value);

    return rval;
}



RESULT
Settings::SetFloat( const string& name, float value )
{
    RESULT rval = S_OK;
    string path;
    string attribute;
    int    indexOfSeparator;
    
    DEBUGCHK(name.c_str());

    indexOfSeparator    = name.find_last_of('.');
    path                = name.substr( 0,                    indexOfSeparator );
    attribute           = name.substr( indexOfSeparator+1,   name.length()    );

    TiXmlElement* pElement = FindElement( path );
    if (pElement)
    {
        char temp[32];
        sprintf(temp, "%f", value);
        pElement->SetAttribute( attribute.c_str(), temp );
    }
    else 
    {
        // TODO: create the Element.
        DumpAttributes(m_pXMLDocument->RootElement(), 4);
        DEBUGCHK(0);
        rval = E_NOT_FOUND;
    }


    DEBUGMSG( ZONE_SETTINGS, "[%s] = [%f]", name.c_str(), value);

    return rval;
}



RESULT
Settings::SetInt( const string& name, int value )
{
    RESULT rval = S_OK;
    string path;
    string attribute;
    int    indexOfSeparator;
    
    DEBUGCHK(name.c_str());

    indexOfSeparator    = name.find_last_of('.');
    path                = name.substr( 0,                    indexOfSeparator );
    attribute           = name.substr( indexOfSeparator+1,   name.length()    );

    TiXmlElement* pElement = FindElement( path );
    if (pElement)
    {
        char temp[32];
        sprintf(temp, "%d", value);
        pElement->SetAttribute( attribute.c_str(), temp );
    }
    else 
    {
        // TODO: create the Element.
        DEBUGCHK(0);
        rval = E_NOT_FOUND;
    }


    DEBUGMSG( ZONE_SETTINGS, "[%s] = [%d]", name.c_str(), value);

    return rval;
}



RESULT
Settings::SetVec2( const string& name, vec2 value )
{
    RESULT rval = S_OK;
    string path;
    string attribute;
    int    indexOfSeparator;
    
    DEBUGCHK(name.c_str());

    indexOfSeparator    = name.find_last_of('.');
    path                = name.substr( 0,                    indexOfSeparator );
    attribute           = name.substr( indexOfSeparator+1,   name.length()    );

    TiXmlElement* pElement = FindElement( path );
    if (pElement)
    {
        char temp[32];
        sprintf(temp, "(%2.2f, %2.2f)", value.x, value.y);
        pElement->SetAttribute( attribute.c_str(), temp );
    }
    else 
    {
        // TODO: create the Element.
        DEBUGCHK(0);
        rval = E_NOT_FOUND;
    }


    DEBUGMSG( ZONE_SETTINGS, "[%s] = [(%2.2f, %2.2f)]", name.c_str(), value.x, value.y);

    return rval;
}



RESULT
Settings::SetVec3( const string& name, vec3 value )
{
    RESULT rval = S_OK;
    string path;
    string attribute;
    int    indexOfSeparator;
    
    DEBUGCHK(name.c_str());

    indexOfSeparator    = name.find_last_of('.');
    path                = name.substr( 0,                    indexOfSeparator );
    attribute           = name.substr( indexOfSeparator+1,   name.length()    );

    TiXmlElement* pElement = FindElement( path );
    if (pElement)
    {
        char temp[32];
        sprintf(temp, "(%2.2f, %2.2f, %2.2f)", value.x, value.y, value.z);
        pElement->SetAttribute( attribute.c_str(), temp );
    }
    else 
    {
        // TODO: create the Element.
        DEBUGCHK(0);
        rval = E_NOT_FOUND;
    }


    DEBUGMSG( ZONE_SETTINGS, "[%s] = [(%2.2f, %2.2f, %2.2f)]", name.c_str(), value.x, value.y, value.z);

    return rval;
}




RESULT
Settings::SetVec4( const string& name, vec4 value )
{
    RESULT rval = S_OK;
    string path;
    string attribute;
    int    indexOfSeparator;
    
    DEBUGCHK(name.c_str());

    indexOfSeparator    = name.find_last_of('.');
    path                = name.substr( 0,                    indexOfSeparator );
    attribute           = name.substr( indexOfSeparator+1,   name.length()    );

    TiXmlElement* pElement = FindElement( path );
    if (pElement)
    {
        char temp[32];
        sprintf(temp, "(%2.2f, %2.2f, %2.2f, %2.2f)", value.x, value.y, value.z, value.w);
        pElement->SetAttribute( attribute.c_str(), temp );
    }
    else 
    {
        // TODO: create the Element.
        DEBUGCHK(0);
        rval = E_NOT_FOUND;
    }


    DEBUGMSG( ZONE_SETTINGS, "[%s] = [(%2.2f, %2.2f, %2.2f, %2.2f)]", name.c_str(), value.x, value.y, value.z, value.w);

    return rval;
}



RESULT
Settings::SetColor( const string& name, Color value )
{
    RESULT rval = S_OK;
    string path;
    string attribute;
    int    indexOfSeparator;
    
    DEBUGCHK(name.c_str());

    indexOfSeparator    = name.find_last_of('.');
    path                = name.substr( 0,                    indexOfSeparator );
    attribute           = name.substr( indexOfSeparator+1,   name.length()    );

    TiXmlElement* pElement = FindElement( path );
    if (pElement)
    {
        char temp[32];
        sprintf(temp, "(%2.2f, %2.2f, %2.2f, %2.2f)", value.floats.r, value.floats.g, value.floats.b, value.floats.a);
        pElement->SetAttribute( attribute.c_str(), temp );
    }
    else 
    {
        // TODO: create the Element.
        DEBUGCHK(0);
        rval = E_NOT_FOUND;
    }


    DEBUGMSG( ZONE_SETTINGS, "[%s] = [(%2.2f, %2.2f, %2.2f, %2.2f)]", name.c_str(), value.floats.r, value.floats.g, value.floats.b, value.floats.a);

    return rval;
}






// print all attributes of pElement.
void
Settings::DumpAttributes(TiXmlElement* pElement, unsigned int indent) const
{
	if ( !pElement ) return;

	TiXmlAttribute* pAttrib=pElement->FirstAttribute();
	int i = 0;
	int ival;
	double dval;
//	const char* pIndent=getIndent(indent);
	DEBUGMSG( ZONE_SETTINGS, "\n");
	while (pAttrib)
	{
		DEBUGMSG( ZONE_SETTINGS,  "%s: value=[%s]", pAttrib->Name(), pAttrib->Value());

		if (pAttrib->QueryIntValue(&ival)==TIXML_SUCCESS)    DEBUGMSG( ZONE_SETTINGS,  " int=%d",  ival);
		if (pAttrib->QueryDoubleValue(&dval)==TIXML_SUCCESS) DEBUGMSG( ZONE_SETTINGS,  " d=%1.1f", dval);
		DEBUGMSG( ZONE_SETTINGS,  "\n" );
		i++;
		pAttrib = pAttrib->Next();
	}

	return;
}


TiXmlElement*   
Settings::FindElement( const string& path ) const
{
//    TiXmlElement* pParentElement = NULL;
    TiXmlElement* pCurrentElement   = NULL;
    TiXmlElement* pTargetElement = NULL;
    char          temppath[MAX_PATH];
    char*         pToken;

    pCurrentElement = m_pXMLDocument->FirstChildElement();
    if (!pCurrentElement)
        return NULL;

    // Convert to c-string for strtok; easier/cheaper than using C++
    // Need to make a local, writable copy for strtok.
    strncpy( temppath, path.c_str(), MAX_DOM_PATH );

    if (!strcmp( temppath, "/" ))
        return pCurrentElement;

    pToken = strtok( temppath, "/" );
    while (pToken != NULL && pCurrentElement)
    {
        // If current token doesn't match current element, return error
        //if ( stricmp( pToken, pCurrentElement->Value() ) )
        if ( strcasecmp( pToken, pCurrentElement->Value() ) )
        {
            return NULL;
        }
        else
        {
            pTargetElement = pCurrentElement;
        }

        // Get next element by name
        pToken = strtok( NULL, "/" );
        if (pToken)
        {
            pCurrentElement = pCurrentElement->FirstChildElement( pToken );
            if (!pCurrentElement)
                return NULL;
        }
    }

    return pTargetElement;    
}



TiXmlAttribute*  
Settings::FindAttribute( const string& path ) const
{
    return NULL;
}


} // END namespace Z


   