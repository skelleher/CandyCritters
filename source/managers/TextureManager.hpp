#pragma once

#include "ResourceManager.hpp"
#include "Handle.hpp"
#include "Object.hpp"
#include "Settings.hpp"

#include <string>
using std::string;

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>


namespace Z
{



// Forward declarations
class TextureAtlas;
class Texture;
typedef Handle<Texture>      HTexture;
typedef Handle<TextureAtlas> HTextureAtlas;

// Texture coordinates relative to TextureAtlas
typedef struct 
{
    float   widthPixelsTextureAtlas;
    float   heightPixelsTextureAtlas;
    float   offsetX;
    float   offsetY;
    float   widthPixels;
    float   heightPixels;
    float   uStart;
    float   vStart;
    float   uEnd;
    float   vEnd;
    
    UINT32  textureID;
//    string name;
//    HTextureAtlas hTextureAtlas;
    bool    isBackedByTextureAtlas;
} TextureInfo;



class TextureManager : public ResourceManager<Texture>
{
public:
    static  TextureManager& Instance();
    
    virtual RESULT      Init    ( IN const string& settingsFilename );


    RESULT              CreateFromFile( IN const  string&    filename, 
                                        INOUT     HTexture*  pHandle );
    
    RESULT              Create ( IN const string&       name,
                                 IN const TextureInfo&  textureInfo,
                                 INOUT    HTexture*     pHandle );


    RESULT              GetInfo ( IN HTexture hTexture, INOUT TextureInfo* pTextureInfo );


    RESULT              CreateGLESTexture ( IN const string& filename, INOUT TextureInfo* pTextureInfo );
    
    RESULT              CreateGLESTexture ( IN const BYTE* pBuffer, UINT32 bufferSize, GLuint pixelFormat, GLuint pixelType, UINT32 width, UINT32 height, INOUT TextureInfo* pTextureInfo );

protected:
    TextureManager();
    TextureManager( const TextureManager& rhs );
    TextureManager& operator=( const TextureManager& rhs );
    virtual ~TextureManager();
    
    
protected:
    RESULT CreateTextureAtlas( IN Settings* pSettings, IN const string& settingsPath, INOUT TextureAtlas** ppTextureAtlas );

protected:
    GLuint  m_boundTextureID;
};

#define TextureMan ((TextureManager&)TextureManager::Instance())
//#define Textures   ((TextureManager&)TextureManager::Instance())




//=============================================================================
//
// A TextureAtlas from which Textures may be created.
//
//=============================================================================
class TextureAtlas : virtual public Object
{
public:
    TextureAtlas();
    virtual ~TextureAtlas();
    
    RESULT      Init( IN const string& name, IN const Settings* pSettings, IN const string& settingsPath );
    RESULT      InitFromTexturePackerJSONFile( IN const string& filename, bool createSprites );

    GLuint      GetTextureID( ) const;
    UINT32      GetWidth( ) const;
    UINT32      GetHeight( ) const;

  
protected:
    GLuint      m_textureID;
    GLuint      m_pixelFormat;
    GLuint      m_pixelType;

    UINT32      m_width;
    UINT32      m_height;
    float       m_fScaleTextureWidth;
    float       m_fScaleTextureHeight;
    UINT32      m_numTextures;

    string      m_imageFilename;
};

typedef Handle<TextureAtlas>    HTextureAtlas;




//=============================================================================
//
// A Texture instance.
//
//=============================================================================
class Texture : virtual public Object
{
public:
    Texture();
    virtual ~Texture();
    
    RESULT              Init    ( IN const string& name, IN TextureAtlas* pTextureAtlas, IN const TextureInfo& textureInfo );
    RESULT              Bind    ( );
    const TextureInfo*  GetInfo ( );

protected:
    TextureAtlas*       m_pTextureAtlas;
    
    // TODO: these are redundant with m_textureInfo, and only used for debug statements.
    UINT32              m_width;
    UINT32              m_height;
    UINT32              m_offsetX;
    UINT32              m_offsetY;
    
    TextureInfo         m_textureInfo;
};

typedef Handle<Texture> HTexture;



} // END namespace Z


