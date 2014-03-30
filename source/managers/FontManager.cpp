/*
 *  FontManager.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 2/26/11.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include "Engine.hpp"
#include "FontManager.hpp"
#include "FileManager.hpp"
#include "Log.hpp"
#include "Handle.hpp"
#include "Settings.hpp"
#include "Platform.hpp"
#include "Types.hpp"
#include "Util.hpp"



namespace Z 
{



FontManager& 
FontManager::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new FontManager();
    }
    
    return static_cast<FontManager&>(*s_pInstance);
}


FontManager::FontManager() 
{
    RETAILMSG(ZONE_VERBOSE, "FontManager()");
    
    s_pResourceManagerName = "FontManager";
}


FontManager::~FontManager()
{
    RETAILMSG(ZONE_VERBOSE, "\t~FontManager()");
    
    // TODO: release m_FontBatch and m_vertexList;
    
}



RESULT
FontManager::Init( IN const string& settingsFilename )
{
    RETAILMSG(ZONE_INFO, "FontManager::Init( %s )", settingsFilename.c_str());

    RESULT rval = S_OK;
    char   path[MAX_PATH];
    
    //
    // Create a Settings object and load the file.
    //
    Settings mySettings;
    if ( FAILED(mySettings.Read( settingsFilename )) )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FontManager::Init( %s ): failed to load settings file", settingsFilename.c_str() );
        return E_UNEXPECTED;
    }
    

    //
    // Create each Font.
    //
    UINT32 numFonts = mySettings.GetInt("/Fonts.NumFonts");

    for (int i = 0; i < numFonts; ++i)
    {
        sprintf(path, "/Fonts/Font%d", i);
        //DEBUGMSG(ZONE_INFO, "Loading [%s]", path);

        Font *pFont = NULL;
        CreateFont( &mySettings, path, &pFont );
        if (!pFont)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: FontManager::Init( %s ): failed to create Font", path);
            // Continue loading other Fonts rather than aborting.
            continue;
        }
        
        //DEBUGMSG(ZONE_Font, "Created Font [%s]", pFont->GetName().c_str());
        HFont hFont;
        CHR(Add(pFont->GetName(), pFont, &hFont));
        
        if (m_hDefaultFont.IsNull())
        {
            m_hDefaultFont = hFont;
        }
    }
    
Exit:
    return rval;
}



RESULT
FontManager::CreateFont( IN const Settings* pSettings, IN const string& settingsPath, INOUT Font** ppFont )
{
    RESULT rval = S_OK;
    string name;
    
    if (!pSettings || "" == settingsPath || !ppFont)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: FontManager::CreateFont(): invalid argument");
        return E_INVALID_ARG;
    }
    
    Font *pFont = new Font();
    CPR(pFont);

    name = pSettings->GetString( settingsPath + ".Name" );
    
    if ("" == name)
    {
        // Auto-generate the name if needed.
        char instanceName[MAX_PATH];
        sprintf(instanceName, "%s.%s", 
            settingsPath.c_str(), 
            pSettings->GetString( settingsPath + ".Property" ).c_str());
            
        name = string(instanceName);
    }
    
    if (FAILED(pFont->Init( name, pSettings, settingsPath )))
    {
        delete pFont;
        pFont = NULL;
    }

    // Caller must AddRef()
    *ppFont = pFont;
    
Exit:    
    return rval;
}



RESULT
FontManager::Shutdown()
{
    RESULT rval = S_OK;
    
/*    
    // Release all FontBatches
    FontBatchMapIterator ppFontBatch;
    for (ppFontBatch = m_FontBatchMap.begin(); ppFontBatch != m_FontBatchMap.end(); ++ppFontBatch)
    {
        FontBatch* pFontBatch = ppFontBatch->second;
        
        DEBUGCHK( pFontBatch );
        
        pFontBatch->numFonts    = 0;
        pFontBatch->numVertices   = 0;
        
        // Free the batched Fonts
        BatchedFonts* pBatchedFonts = &pFontBatch->Fonts;
        BatchedFontsIterator ppBatchedFont;
        for (ppBatchedFont = pBatchedFonts->begin(); ppBatchedFont != pBatchedFonts->end(); ++ppBatchedFont)
        {
            BatchedFont* pBatchedFont = *ppBatchedFont;
            DEBUGCHK(pBatchedFont);
            delete pBatchedFont;
        }
        
        pFontBatch->Fonts.clear();
        SAFE_ARRAY_DELETE(pFontBatch->pVertices);
        
        //SAFE_DELETE(pFontBatch);
        m_FontBatchMap.erase( ppFontBatch );
        
        //        RETAILMSG(ZONE_Font | ZONE_VERBOSE, "FontManager::BeginBatch(): cleared previous FontBatch [%s]", pFontBatch->textureName.c_str());
        RETAILMSG(ZONE_Font | ZONE_VERBOSE, "FontManager::BeginBatch(): cleared previous FontBatch 0x%x", pFontBatch);
    }
    
    DEBUGCHK(m_FontBatchMap.size() == 0);
*/
    
    ResourceManager<Font>::Shutdown();
    
Exit:
    return rval;
}



RESULT
FontManager::Draw( IN const vec2& position, IN const string& characters, IN HFont hFont, Color color, float scale, float opacity, float rotX, float rotY, float rotZ, bool worldSpace )
{
    RESULT rval = S_OK;

    if (hFont.IsNull())
    {
        hFont = m_hDefaultFont;
    }

    Font* pFont = GetObjectPointer( hFont );
    if (!pFont)
    {
        RETAILMSG(ZONE_ERROR, "FontManager::Draw( 0x%x ): bad handle", (UINT32)hFont);
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    CHR(pFont->Draw( position, characters, color, scale, opacity, rotX, rotY, rotZ, worldSpace ));

Exit:
    return rval;
}



RESULT
FontManager::Draw( IN const vec2& position, IN const char* pCharacters, IN HFont hFont, Color color, float scale, float opacity, float rotX, float rotY, float rotZ, bool worldSpace )
{
    RESULT rval = S_OK;

    if (hFont.IsNull())
    {
        hFont = m_hDefaultFont;
    }

    Font* pFont = GetObjectPointer( hFont );
    if (!pFont)
    {
        RETAILMSG(ZONE_ERROR, "FontManager::Draw( 0x%x ): bad handle", (UINT32)hFont);
        rval = E_BAD_HANDLE;
        goto Exit;
    }

    CHR(pFont->Draw( position, pCharacters, color, scale, opacity, rotX, rotY, rotZ, worldSpace ));

Exit:
    return rval;
}



float
FontManager::GetHeight( IN HFont hFont )
{
    Font* pFont = GetObjectPointer( hFont );
    if (!pFont)
    {
        RETAILMSG(ZONE_ERROR, "FontManager::GetHeight( 0x%x ): bad handle", (UINT32)hFont);
        return 0.0f;
    }

    return pFont->GetHeight();
}


float
FontManager::GetWidth( IN HFont hFont, IN const string& text )
{
    Font* pFont = GetObjectPointer( hFont );
    if (!pFont)
    {
        RETAILMSG(ZONE_ERROR, "FontManager::GetWidth( 0x%x ): bad handle", (UINT32)hFont);
        return 0.0f;
    }
    
    return pFont->GetWidth( text );
}


float
FontManager::GetHeight( IN const string& name )
{
    HFont hFont;
    IGNOREHR(Get( name, &hFont ));

    Font* pFont = GetObjectPointer( hFont );
    if (!pFont)
    {
        RETAILMSG(ZONE_ERROR, "FontManager::GetHeight( %s ): not found", name.c_str());
        return 0.0f;
    }

    return pFont->GetHeight();
}



float
FontManager::GetWidth( IN const string& name, IN const string& text )
{
    HFont hFont;
    IGNOREHR(Get( name, &hFont ));

    Font* pFont = GetObjectPointer( hFont );
    if (!pFont)
    {
        RETAILMSG(ZONE_ERROR, "FontManager::GetWidth( %s ): not found", name.c_str());
        return 0.0f;
    }
    
    return pFont->GetWidth( text );
}



} // END namespace Z



