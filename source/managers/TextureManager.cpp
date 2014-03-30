/*
 *  TextureManager.cpp
 *  Critters
 *
 *  Created by Sean Kelleher on 9/12/10.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include "TextureManager.hpp"
#include "FileManager.hpp"
#include "Log.hpp"
#include "Handle.hpp"
#include "Settings.hpp"
#include "Platform.hpp"
#include "Util.hpp"
#include "Types.hpp"
#include "Image.hpp"
#include "SpriteManager.hpp"
#include "json.h"

#include <OpenGLES/ES1/gl.h>



namespace Z
{


    
TextureManager& 
TextureManager::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new TextureManager();
    }
    
    return static_cast<TextureManager&>(*s_pInstance);
}


TextureManager::TextureManager() :
    m_boundTextureID(0)
{
    RETAILMSG(ZONE_VERBOSE, "TextureManager()");
    
    s_pResourceManagerName = "TextureManager";
}


TextureManager::~TextureManager()
{
    RETAILMSG(ZONE_VERBOSE, "\t~TextureManager()");
}



RESULT
TextureManager::Init( IN const string& settingsFilename )
{
    RETAILMSG(ZONE_INFO, "TextureManager::Init( %s )", settingsFilename.c_str());

    RESULT rval = S_OK;
    char   path[MAX_PATH];
    
    //
    // Create a Settings object and load the file.
    //
    Settings mySettings;
    if ( FAILED(mySettings.Read( settingsFilename )) )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: TextureManager::Init( %s ): failed to load settings file", settingsFilename.c_str() );
        return E_UNEXPECTED;
    }
    

    //
    // Create each TextureAtlas.
    //
    UINT32 numAtlasses = mySettings.GetInt("/Textures.NumAtlasses");

    for (int i = 0; i < numAtlasses; ++i)
    {
        sprintf(path, "/Textures/Atlas%d", i);
        DEBUGMSG(ZONE_INFO, "Loading [%s]", path);

        TextureAtlas *pTextureAtlas = NULL;
        CreateTextureAtlas( &mySettings, path, &pTextureAtlas );
        if (!pTextureAtlas)
        {
            RETAILMSG(ZONE_ERROR, "ERROR: TextureManager::Init( %s ): failed to create TextureAtlas", path);
            // Continue loading other texture atlasses rather than aborting.
            continue;
        }
    }
    
    return rval;
}



RESULT
TextureManager::CreateFromFile( IN const string &filename, INOUT HTexture* pHandle )
{
    RESULT          rval = S_OK;
    Texture*        pTexture = NULL;
    HTexture        hTexture;
    TextureInfo     textureInfo;
    char            textureName[MAX_PATH];

    // If the file has already been loaded, return a handle to it.
    if (SUCCEEDED(Get( filename, &hTexture )))
    {
        RETAILMSG(ZONE_ERROR, "TextureManager::CreateFromFile( \"%s\" ): texture already loaded.", filename.c_str());
        if (pHandle)
        {
            *pHandle = hTexture;
        }
        
        rval = S_OK;
        goto Exit;
    }
    

    // Create a GLES texture.
    memset(&textureInfo, 0, sizeof(TextureInfo));
    CHR(CreateGLESTexture(filename, &textureInfo));

    // Create a Texture object.
    sprintf(textureName, "TEXTURE:%s", filename.c_str());
    pTexture = new Texture();
    CPR(pTexture);
    CHR(pTexture->Init(textureName, NULL, textureInfo));

    CHR(Add(filename, pTexture, &hTexture));
    
    if (pHandle)
    {
        *pHandle = hTexture;
    }

Exit:
    return rval;
}



// Create an HTexture directly from an OpenGL texture, which was created elsewhere.
// This is a convenience method for things like render-to-texture, where the texture is then used by an HSprite.
// Game developers generally should not call this method directly.
RESULT
TextureManager::Create( IN const string& name, IN const TextureInfo& textureInfo, INOUT HTexture* pHandle )
{
    RESULT      rval       = S_OK;
    Texture*    pTexture   = NULL;
    HTexture    hTexture;
    char        textureName[MAX_NAME];

    pTexture = new Texture();
    CPREx(pTexture, E_OUTOFMEMORY);
    
    if ("" == name) 
    {
        sprintf(textureName, "texture_%X", (unsigned int)Platform::Random());
    } 
    else
    {
        sprintf(textureName, "%s", name.c_str());
    }
    
    CHR(pTexture->Init( textureName,
                        NULL,
                        textureInfo ));
                       
    
    DEBUGMSG(ZONE_TEXTURE, "Created Texture [%s]", name.c_str());
    CHR(TextureMan.Add(name, pTexture, &hTexture));
    
    if (pHandle)
    {
        *pHandle = hTexture;
    }
    
Exit:
    return rval;
}



RESULT
TextureManager::CreateTextureAtlas( IN Settings* pSettings, IN const string& settingsPath, INOUT TextureAtlas** ppTextureAtlas )
{
    RESULT rval = S_OK;
    string filename;
    string name;
    
    if (!pSettings || "" == settingsPath || !ppTextureAtlas)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: TextureManager::CreateTextureAtlas(): invalid argument");
        return E_INVALID_ARG;
    }
    
    *ppTextureAtlas = NULL;
    
    // Construct it; will create all its child Texture objects
    TextureAtlas *pTextureAtlas = new TextureAtlas();
    CPR(pTextureAtlas);

    filename = pSettings->GetString( settingsPath + ".Filename" );
    if ( string::npos != filename.find( ".json" ))
    {
        bool createSprites = pSettings->GetBool( settingsPath + ".CreateSprites", false );
        CHR(pTextureAtlas->InitFromTexturePackerJSONFile ( filename, createSprites ));
    }
    else
    {
        name = pSettings->GetString( settingsPath + ".Name" );
        CHR(pTextureAtlas->Init( name, pSettings, settingsPath ));
    }

    // Caller must AddRef()
    *ppTextureAtlas = pTextureAtlas;
    
Exit:
    if (FAILED(rval))
    {
        SAFE_DELETE(pTextureAtlas);
        *ppTextureAtlas = NULL;
    }
    
    return rval;
}



RESULT 
TextureManager::CreateGLESTexture ( IN const string& filename, INOUT TextureInfo* pTextureInfo )
{
    RESULT rval         = S_OK;
    HFile  hFile;
    UINT32 fileSize;
    UINT32 numBytesRead;
    BYTE*  pBuffer      = NULL;
    BYTE*  pBufferRGBA  = NULL;
    UINT32 bufferRGBASize = 0;
    ImageProperties props;
    GLuint pixelFormat = GL_RGBA;
    GLuint pixelType   = GL_UNSIGNED_BYTE;


    if ("" == filename)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: TextureManager::CreateGLESTexture( \"%s\", 0x%x ): empty filename",
                  filename.c_str(), pTextureInfo);
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    if (!pTextureInfo)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: TextureManager::CreateGLESTexture( \"%s\", 0x%x ): NULL pointer",
                  filename.c_str(), pTextureInfo);
        rval = E_NULL_POINTER;
        goto Exit;
    }
    

    if ( string::npos == filename.find(".png"))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: TextureManager::CreateGLESTexture( \"%s\" ): we only support .PNG files for now.", filename.c_str());
    }


    //
    // Get image properties.
    //
    CHR(Image::GetImageProperties( filename, &props ));
    switch( props.bytesPerPixel )
    {
        case 2:
            pixelFormat = GL_RGB565;
            pixelType   = GL_UNSIGNED_SHORT;
            break;
        case 3:
            pixelFormat = GL_RGB;
            pixelType   = GL_UNSIGNED_BYTE;
            break;
        case 4:
            // TODO: GL_BGRA?
            pixelFormat = GL_RGBA;
            pixelType   = GL_UNSIGNED_BYTE;
            break;
        default:
            RETAILMSG(ZONE_ERROR, "ERROR: TextureManager::CreateGLESTexture( \"%s\" ): unsupported pixelFormat (not RGBA or RGB565)", filename.c_str());
            DEBUGCHK(0);
    };
    
    
    //
    // Load the source file.
    //
    CHR( FileMan.OpenFile( filename, &hFile ) );
    CHR( FileMan.GetFileSize( hFile, &fileSize ) );
    pBuffer = new BYTE[fileSize];
    CHR( FileMan.ReadFile( hFile, pBuffer, fileSize, &numBytesRead ) );
    DEBUGCHK( fileSize == numBytesRead );

    //
    // Convert from .PNG to RGBA buffer.
    //
    CHR( Image::ConvertPNGToRGBA(pBuffer, fileSize, &pBufferRGBA, &bufferRGBASize ) );
    DEBUGCHK(bufferRGBASize);

    //
    // Create the texture.
    //
    CHR(CreateGLESTexture( pBufferRGBA, bufferRGBASize, pixelFormat, pixelType, props.width, props.height, pTextureInfo ));

Exit:
    if ( !hFile.IsNull() )
    {
        FileMan.CloseFile( hFile );
    }
    SAFE_ARRAY_DELETE(pBuffer)
    SAFE_ARRAY_DELETE(pBufferRGBA)
    return rval;
}



RESULT
TextureManager::CreateGLESTexture ( IN const BYTE* pBuffer, UINT32 bufferSize, GLuint pixelFormat, GLuint pixelType, UINT32 width, UINT32 height, INOUT TextureInfo* pTextureInfo )
{
    RESULT rval = S_OK;
    UINT32 textureWidth         = width;
    UINT32 textureHeight        = height;
    float  fScaleTextureWidth   = 1.0f;
    float  fScaleTextureHeight  = 1.0f;
    GLuint textureID            = 0xFFFFFFFF;
    
    if ( !pBuffer || !bufferSize || !pixelFormat || !pixelType || !width || !height || !pTextureInfo )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: TextureManager::CreateGLESTexture( 0x%x, %d, 0x%x, 0x%x, %d, %d, 0x%x ): invalid argument",
                  pBuffer, bufferSize, pixelFormat, pixelType, width, height, pTextureInfo);
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    
    //
    // Round texture dimensions up to nearest power-of-two, if needed.
    // OpenGL ES 1.1 devices do not support NPOT textures.
    //
    if ( (!Util::IsPowerOfTwo(textureWidth) || !Util::IsPowerOfTwo(textureHeight)) && Platform::IsOpenGLES1() )
    {
        RETAILMSG(ZONE_WARN, "WARNING: CreateGLESTexture: non-power-of-two dimensions %d x %d are not supported on this device!",
                  textureWidth, textureHeight);
        
        UINT32 orgWidth       = textureWidth;
        UINT32 orgHeight      = textureHeight;

        textureWidth          = Util::RoundUpToPowerOfTwo( orgWidth  );
        textureHeight         = Util::RoundUpToPowerOfTwo( orgHeight );
        fScaleTextureWidth    = (float)orgWidth /(float)textureWidth;
        fScaleTextureHeight   = (float)orgHeight/(float)textureHeight;
        
        RETAILMSG(ZONE_WARN, "Scaling texture to %d x %d", textureWidth, textureHeight);
    }

    
    //
    // Create the texture
    //
    VERIFYGL(glActiveTexture(GL_TEXTURE0));
    VERIFYGL(glGenTextures(1, &textureID));
    VERIFYGL(glBindTexture(GL_TEXTURE_2D, textureID));
    VERIFYGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    VERIFYGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    VERIFYGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
//    VERIFYGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    
    // TODO: mipmapping fails if the textures aren't power-of-two?
    // Do we need to check for this?
    // TODO: generate mipmaps offline and set explicitly.
    //VERIFYGL(glGenerateMipmap(GL_TEXTURE_2D));  // NOTE: For OpenGL ES 1.x, enable BEFORE uploading pixels.  For OpenGL ES 2.0, enable AFTER.
    //VERIFYGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
//    VERIFYGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    VERIFYGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));    // TODO: fix texture atlases to put 1-pixel border around textures, so we can turn LINEAR back on.
    
    
    //
    // Create the texture (with dimensions optionally scaled up to power-of-two), but do not provide a buffer.
    // Below we'll use glTexSubImage2D(), which allows uploading an NPOT pixel buffer into a POT texture.
    //
    switch( pixelFormat )
    {
        case GL_RGB:
        case GL_RGBA:
        case GL_BGRA:
        case GL_RGB565:
        VERIFYGL(glTexImage2D( 
                                GL_TEXTURE_2D, 
                                0, 
                                pixelFormat,
                                textureWidth, 
                                textureHeight, 
                                0,
                                pixelFormat,
                                pixelType,
                                NULL
                                ));

                break;
        case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
            VERIFYGL(glCompressedTexImage2D( 
                                GL_TEXTURE_2D, 
                                0, 
                                pixelFormat, 
                                textureWidth, 
                                textureHeight, 
                                0, 
                                bufferSize, 
                                NULL));
            break;
        default:
            DEBUGCHK(0);
    };


    //
    // Provide the pixel data.
    //
    switch( pixelFormat )
    {
        case GL_RGB:
        case GL_RGBA:
        case GL_BGRA:
        case GL_RGB565:
              VERIFYGL(glTexSubImage2D( 
                                GL_TEXTURE_2D, 
                                0, 
                                0,
                                0,
                                width, 
                                height, 
                                pixelFormat,
                                pixelType,
                                pBuffer
                                ));
                break;
                
        case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
            VERIFYGL(glCompressedTexSubImage2D( 
                                GL_TEXTURE_2D, 
                                0, 
                                0,
                                0,
                                pixelFormat,
                                width, 
                                height, 
                                bufferSize,
                                pBuffer
                                ));
            break;
            
        default:
            DEBUGCHK(0);
    }


    //
    // Pass TextureInfo back to caller.
    //
    pTextureInfo->textureID                 = textureID;
    pTextureInfo->widthPixelsTextureAtlas   = textureWidth;
    pTextureInfo->heightPixelsTextureAtlas  = textureHeight;
    pTextureInfo->offsetX                   = 0;
    pTextureInfo->offsetY                   = 0;
    pTextureInfo->widthPixels               = width;
    pTextureInfo->heightPixels              = height;
    pTextureInfo->uStart                    = 0.0f;
    pTextureInfo->vStart                    = 0.0f;
    pTextureInfo->uEnd                      = fScaleTextureWidth;
    pTextureInfo->vEnd                      = fScaleTextureHeight;
        
Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "TextureAtlas::CreateGLESTexture(): failed!");
        glDeleteTextures(1, &textureID);
        DEBUGCHK(0);
    }
    
    return rval;
}




RESULT
TextureManager::GetInfo( IN const HTexture hTexture, INOUT TextureInfo* pTextureInfo )
{
    RESULT      rval        = S_OK;
    Texture*    pTexture    = NULL;
    
    CPR(pTextureInfo);
    
    pTexture = GetObjectPointer( hTexture );
    if (pTexture)
    {
        memcpy(pTextureInfo, pTexture->GetInfo(), sizeof(TextureInfo));
        CPREx(pTextureInfo, E_UNEXPECTED);
    }
    
Exit:
    return rval;
}




// ============================================================================
//
//  Private classes; other systems only see Handles to these
//
// ============================================================================




// ============================================================================
//
//  TextureAtlas Implementation
//
// ============================================================================

#pragma mark TextureAtlas Implementation


TextureAtlas::TextureAtlas() :
    m_textureID(0),
    m_width(0),
    m_height(0),
    m_fScaleTextureWidth(1.0),
    m_fScaleTextureHeight(1.0),
//    m_name(""),
    m_imageFilename("")
{
    DEBUGMSG(ZONE_OBJECT | ZONE_VERBOSE, "TextureAtlas( %4d )", m_ID);
}


TextureAtlas::~TextureAtlas()
{
    DEBUGMSG(ZONE_OBJECT | ZONE_VERBOSE, "\t~TextureAtlas( %4d )", m_ID);
}
    

RESULT
TextureAtlas::Init( IN const string& name, IN const Settings* pSettings, IN const string& settingsPath )
{
    RESULT rval = S_OK;
    string path, pixelType, pixelFormat;
    

    if (!pSettings)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: TextureAtlas::Init(): NULL settings");
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    
    // Set the members
    m_name          = name;
    m_imageFilename = pSettings->GetString( settingsPath + ".Filename"      );
    m_width         = pSettings->GetInt   ( settingsPath + ".Width"         );
    m_height        = pSettings->GetInt   ( settingsPath + ".Height"        );
    m_numTextures   = pSettings->GetInt   ( settingsPath + ".NumTextures"   );

    DEBUGMSG(ZONE_TEXTURE, "TextureAtlas[%4d]::Init( %s )", m_ID, m_imageFilename.c_str());

    CBR(m_name.length() > 0);
    CBR(m_width         > 0);
    CBR(m_height        > 0);
    CBR(m_numTextures   > 0);

    
    // TODO: lookup table that maps strings to ints, then switch statements for pixelFormat/pixelType.
    
    pixelFormat = pSettings->GetString( settingsPath + ".PixelFormat" );
    if (pixelFormat == "GL_RGBA")
    {
        m_pixelFormat = GL_RGBA;
    } 
    else if (pixelFormat == "GL_RGB")
    {
        m_pixelFormat = GL_RGB;
    }
    else if (pixelFormat == "GL_BGRA")
    {
        // TODO: assert that device supports GL_BGRA!
        // All iOS devices should
        m_pixelFormat = GL_BGRA;
    } 
    else if (pixelFormat == "GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG")
    {
        m_pixelFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
    }
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: TextureAtlas::Init(): invalid PixelFormat \"%s\"", pixelFormat.c_str());
        rval = E_INVALID_ARG;
        goto Exit;
    }


    pixelType   = pSettings->GetString( settingsPath + ".PixelType"   );
    if (pixelType == "GL_UNSIGNED_BYTE")
    {
        m_pixelType = GL_UNSIGNED_BYTE;
    } 
    else if (pixelType == "GL_UNSIGNED_SHORT_5_6_5")
    {
        m_pixelType = GL_UNSIGNED_SHORT_5_6_5;
    }
    else if (pixelType == "GL_UNSIGNED_SHORT_5_5_5_1")
    {
        m_pixelType = GL_UNSIGNED_SHORT_5_5_5_1;
    }
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: TextureAtlas::Init(): invalid PixelType \"%s\"", pixelType.c_str());
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    
    //
    // Create the OpenGL ES texture
    //
    {
    TextureInfo textureInfo;
    CHR( TextureMan.CreateGLESTexture( m_imageFilename, &textureInfo ) );
    m_textureID = textureInfo.textureID;
    m_width     = textureInfo.widthPixelsTextureAtlas;
    m_height    = textureInfo.heightPixelsTextureAtlas;
    }
    
    // Create the child Textures
    {
        TextureInfo textureInfo;
        Texture* pTexture;
        char buf[MAX_PATH];
        string textureSettings;

        for (int i = 0; i < m_numTextures; ++i)
        {
            pTexture = new Texture();
            if (!pTexture)
            {
                RETAILMSG(ZONE_ERROR, "TextureAtlas::Init( %s ): out of memory allocating Texture %d",
                          settingsPath.c_str(), i);
                rval = E_OUTOFMEMORY;
                goto Exit;
            }
            
            sprintf(buf, "%s/Texture%d", settingsPath.c_str(), i);
            textureSettings = buf;
            
            string name             = pSettings->GetString( textureSettings + ".Name"    );

            textureInfo.widthPixelsTextureAtlas  = m_width;
            textureInfo.heightPixelsTextureAtlas = m_height;

            textureInfo.offsetX      = pSettings->GetInt     ( textureSettings + ".OffsetX" );
            textureInfo.offsetY      = pSettings->GetInt     ( textureSettings + ".OffsetY" );
            textureInfo.widthPixels  = pSettings->GetInt     ( textureSettings + ".Width"   );
            textureInfo.heightPixels = pSettings->GetInt     ( textureSettings + ".Height"  );

            textureInfo.uStart       = (float)textureInfo.offsetX                               / (float)m_width;
            textureInfo.vStart       = (float)textureInfo.offsetY                               / (float)m_height;
            textureInfo.uEnd         = (float)(textureInfo.offsetX + textureInfo.widthPixels)   / (float)m_width;
            textureInfo.vEnd         = (float)(textureInfo.offsetY + textureInfo.heightPixels)  / (float)m_height;

            textureInfo.textureID = m_textureID;

            RESULT result = pTexture->Init(name, this, textureInfo);
            if (FAILED(result))
            {
                RETAILMSG(ZONE_ERROR, "ERROR: TextureAtlas::Init(): failed to create texture \"%s\"", textureSettings.c_str());
                SAFE_DELETE(pTexture);
                continue;
            }
            
            //DEBUGMSG(ZONE_TEXTURE, "Created Texture [%s]", name.c_str());
            CHR(TextureMan.Add(name, pTexture));
        }
    }

Exit:
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: TextureAtlas::Init( \"%s\" ) failed.", m_name.c_str());
    }

    return rval;
}



RESULT
TextureAtlas::InitFromTexturePackerJSONFile( const string& filename, bool createSprites )
{
    RESULT          rval = S_OK;
    HFile           hJSONFile;
    size_t          fileSize     = 0;
    size_t          bytesRead    = 0;
    char*           buffer       = NULL;
    string*         pDocument    = NULL;
    string          pixelFormat;
    
    Json::Value     root;
    Json::Reader    reader;
    Json::Value     textures;

    Json::Value::Members            textureNames;
    Json::Value::Members::iterator  pTextureName;


    if (!filename.length())
    {
        RETAILMSG(ZONE_ERROR, "ERROR: TextureAtlas::InitFromTexturePackerJSONFile(): must provide a TexturePacker .json filename");
        rval = E_INVALID_ARG;
        goto Exit;
    }

    DEBUGMSG(ZONE_TEXTURE, "TextureAtlas[%4d]::InitFromTexturePackerJSONFile( %s )", m_ID, filename.c_str());


    //
    // Read .JSON file into a string.
    //
    CHR(FileMan.OpenFile( filename, &hJSONFile ));
    CHR(FileMan.GetFileSize( hJSONFile, &fileSize ));
    DEBUGCHK( fileSize );
    buffer = (char*)new char[fileSize];
    CHR(FileMan.ReadFile( hJSONFile, (BYTE*)buffer, fileSize, &bytesRead ));
    DEBUGCHK( fileSize == bytesRead );
    pDocument = new string( buffer, bytesRead );

    //
    // Parse the .JSON document.
    //
    if ( !reader.parse( *pDocument, root ) )
    {
        RETAILMSG(ZONE_INFO, "ERROR: TextureAtlas[%4d]::InitFromTexturePackerJSONFile(): %s", m_ID, reader.getFormattedErrorMessages().c_str());
        rval = E_INVALID_DATA;
        goto Exit;
    }

    //
    // Make sure it's a TexturePacker file.
    //
    CBR( 0 == root["meta"]["app"].asString().compare( "http://www.texturepacker.com" ) );
    

    //
    // Read TextureAtlas members.
    //
    m_name          = filename;
    m_width         = root["meta"]["size"]["w"].asInt();
    m_height        = root["meta"]["size"]["h"].asInt();
    m_imageFilename = "/app/textures/" + root["meta"]["image"].asString();

    textures        = root["frames"];
    textureNames    = textures.getMemberNames();
    m_numTextures   = textureNames.size();

    CBR(m_name.length()          > 0);
    CBR(m_imageFilename.length() > 0);
    CBR(m_width                  > 0);
    CBR(m_height                 > 0);
    CBR(m_numTextures            > 0);

    pixelFormat = root["meta"]["format"].asString();
    if (pixelFormat == "RGBA8888")
    {
        m_pixelFormat = GL_RGBA;
        m_pixelType   = GL_UNSIGNED_BYTE;
    } 
    else 
    {
        RETAILMSG(ZONE_ERROR, "ERROR: TextureAtlas::Init(): invalid PixelFormat \"%s\"", pixelFormat.c_str());
        rval = E_INVALID_ARG;
        goto Exit;
    }


    //
    // Create the OpenGL ES texture
    //
    {
    TextureInfo textureInfo;
    CHR( TextureMan.CreateGLESTexture( m_imageFilename, &textureInfo ) );
    m_textureID = textureInfo.textureID;
    m_width     = textureInfo.widthPixelsTextureAtlas;
    m_height    = textureInfo.heightPixelsTextureAtlas;
    }
    


    //
    // Create each child Texture.
    //
    RETAILMSG(ZONE_INFO, "TextureAtlas %s, imageFile %s, %d Textures", m_name.c_str(), m_imageFilename.c_str(), textureNames.size());
    for (pTextureName = textureNames.begin(); pTextureName != textureNames.end(); ++pTextureName)
    {
        string      textureName = *pTextureName;
        TextureInfo textureInfo;
        Rectangle   frameRect;
        Rectangle   spriteSourceSizeRect;
        Rectangle   sourceSizeRect;
        
        // Read the Texture description.
        Json::Value texture                 = textures[textureName];
        Json::Value frame                   = texture["frame"];
        Json::Value spriteSourceSize        = texture["spriteSourceSize"];
        Json::Value sourceSize              = texture["sourceSize"];
        
        frameRect.x                         = frame["x"].asFloat();
        frameRect.y                         = frame["y"].asFloat();
        frameRect.width                     = frame["w"].asFloat();
        frameRect.height                    = frame["h"].asFloat();
        
        spriteSourceSizeRect.x              = spriteSourceSize["x"].asFloat();
        spriteSourceSizeRect.y              = spriteSourceSize["y"].asFloat();
        spriteSourceSizeRect.width          = spriteSourceSize["w"].asFloat();
        spriteSourceSizeRect.height         = spriteSourceSize["h"].asFloat();

        sourceSizeRect.width                = sourceSize["w"].asFloat();
        sourceSizeRect.height               = sourceSize["h"].asFloat();


        // Strip ".png" or ".pvr" from the texture name.
        size_t pos = textureName.find(".png");
        if (pos != string::npos)
            textureName.replace(pos, 4, "");

        pos = textureName.find(".pvr");
        if (pos != string::npos)
            textureName.replace(pos, 4, "");
        
        
        Texture* pTexture = new Texture();
        CPREx(pTexture, E_OUTOFMEMORY);


        // Texture Atlas
        textureInfo.textureID                = m_textureID;
        textureInfo.widthPixelsTextureAtlas  = m_width;
        textureInfo.heightPixelsTextureAtlas = m_height;

        // Texture
        textureInfo.offsetX                  = frameRect.x;
        textureInfo.offsetY                  = frameRect.y;
        textureInfo.widthPixels              = frameRect.width;
        textureInfo.heightPixels             = frameRect.height;

        // Texture coordinates relative to Atlas
        textureInfo.uStart                   = (float)textureInfo.offsetX                               / (float)m_width;
        textureInfo.vStart                   = (float)textureInfo.offsetY                               / (float)m_height;
        textureInfo.uEnd                     = (float)(textureInfo.offsetX + textureInfo.widthPixels)   / (float)m_width;
        textureInfo.vEnd                     = (float)(textureInfo.offsetY + textureInfo.heightPixels)  / (float)m_height;


        RESULT result = pTexture->Init(textureName, this, textureInfo);
        if (FAILED(result))
        {
            RETAILMSG(ZONE_ERROR, "ERROR: TextureAtlas::Init(): failed to create texture \"%s\"", textureName.c_str());
            SAFE_DELETE(pTexture);
            continue;
        }
        
        
        //
        // Add to TextureManager
        //
        HTexture hTexture;
        CHR(TextureMan.Add(textureName, pTexture, &hTexture));
        
        
        //
        // Optionally create a Sprite for each Texture.
        // Only makes sense for single-frame Sprites.
        // If the Sprite has multiple frames of animation, 
        // for now, we still define them in sprites.xml.
        //
        if (createSprites)
        {
            Rectangle spriteRect;
            spriteRect.x        = 0;
            spriteRect.y        = 0;
            spriteRect.width    = sourceSizeRect.width;
            spriteRect.height   = sourceSizeRect.height;
            
            IGNOREHR(SpriteMan.CreateFromTexture( textureName, hTexture, spriteRect ));
        }
    }
 
 
    
Exit:
    IGNOREHR(FileMan.CloseFile( hJSONFile ));

    SAFE_ARRAY_DELETE(buffer);
    SAFE_DELETE(pDocument);
    
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: TextureAtlas::InitFromTexturePackerJSONFile( \"%s\" ) failed.", m_name.c_str());
    }

    return rval;
}



GLuint
TextureAtlas::GetTextureID() const
{
    return m_textureID;
}



UINT32
TextureAtlas::GetWidth() const
{
    return m_width;
}



UINT32
TextureAtlas::GetHeight() const
{
    return m_height;
}





// ============================================================================
//
//  Texture Implementation
//
// ============================================================================


#pragma mark Texture Implementation

Texture::Texture() :
    m_pTextureAtlas(NULL),
    m_width(0),
    m_height(0)
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "Texture( %4d )", m_ID);
    
    memset(&m_textureInfo, 0, sizeof(TextureInfo));
}



Texture::~Texture()
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "\t~Texture( %4d )", m_ID);
    if (m_pTextureAtlas)
    {
        SAFE_RELEASE(m_pTextureAtlas);
    }
    else
    {
        IGNOREGL(glDeleteTextures(1, (const GLuint*)&m_textureInfo.textureID));
    }
}




RESULT
Texture::Init( const string& name, TextureAtlas* pTextureAtlas, IN const TextureInfo& textureInfo )
{
    RESULT rval = S_OK;
    
    CBR(textureInfo.widthPixels  > 0);
    CBR(textureInfo.heightPixels > 0);
    
    // TODO: we have a refcounting problem here!!
    // Or shall we not let TextureAtlas hold a ref to its children?
    m_pTextureAtlas = pTextureAtlas;
    
    m_name          = name;
    m_width         = (UINT32)textureInfo.widthPixels;
    m_height        = (UINT32)textureInfo.heightPixels;
    m_offsetX       = textureInfo.offsetX;
    m_offsetY       = textureInfo.offsetY;
    m_textureInfo   = textureInfo;

    m_textureInfo.isBackedByTextureAtlas = m_pTextureAtlas ? true : false;

    
    RETAILMSG(ZONE_TEXTURE, "Texture[%4d]: \"%-32s\" %4.0f, %4.0f, %4.0f, %4.0f, uStart = %2.4f, uEnd = %2.4f, vStart = %2.4f, vEnd = %2.4f",
              m_ID, m_name.c_str(), 
              m_textureInfo.offsetX, m_textureInfo.offsetY, m_textureInfo.widthPixels, m_textureInfo.heightPixels, 
              m_textureInfo.uStart, m_textureInfo.uEnd, m_textureInfo.vStart, m_textureInfo.vEnd);
    
Exit:    
    return rval;
}



RESULT
Texture::Bind()
{
    RESULT rval = S_OK;

    VERIFYGL(glBindTexture(GL_TEXTURE_2D, m_textureInfo.textureID));
    
Exit:
    return rval;
}


const TextureInfo*
Texture::GetInfo()
{
    return &m_textureInfo;
}



} // END namespace Z


    
