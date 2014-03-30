#pragma once

#include "ResourceManager.hpp"
//#include "Handle.hpp"
//#include "Renderer.hpp"
#include "Object.hpp"
#include "Settings.hpp"

#include <OpenGLES/ES1/gl.h>
#include <string>
using std::string;


namespace Z
{



// Order of shader uniform parameters
const UINT32 ATTRIBUTE_VERTEX_POSITION          = 0;
const UINT32 ATTRIBUTE_VERTEX_DIFFUSE           = 1;
const UINT32 ATTRIBUTE_VERTEX_TEXTURE_COORD0    = 2;
const UINT32 ATTRIBUTE_VERTEX_TEXTURE_COORD1    = 3;


//=============================================================================
//
// A Shader instance.
//
//=============================================================================
class Shader : public Object
{
public:
    Shader();
    virtual ~Shader();
    
    RESULT              Init        ( IN const string& name, IN GLuint shaderProgram );
    RESULT              PreRender   ( ) const;
    RESULT              PostRender  ( ) const;
    const string&       GetName     ( ) const;
    const GLuint        GetProgramID( ) const;
    
protected:
    GLuint              m_shaderProgram;
};

typedef Handle<Shader> HShader;




//=============================================================================
//
// ShaderManager
//
//=============================================================================
class ShaderManager : public ResourceManager<Shader>
{
public:
    static  ShaderManager& Instance();
    
    virtual RESULT      Init                ( IN const string& settingsFilename );
    RESULT              PrewarmShaders      ( );

    GLuint              GetShaderProgramID  ( IN HShader hShader          );
    GLuint              GetShaderProgramID  ( IN const string& shaderName );
    
    RESULT              CreateShader        ( IN const string& name, IN const string& vertexShaderName, IN const string& fragmentShaderName, INOUT HShader* pHShader = NULL );
    
protected:
    ShaderManager();
    ShaderManager( const ShaderManager& rhs );
    ShaderManager& operator=( const ShaderManager& rhs );
    virtual ~ShaderManager();
    

protected:
    RESULT  CreateShader ( IN const string& name, IN const string& vertexshader, IN const string& fragmentshader, INOUT Shader** ppShader );
    RESULT  PrewarmShader( IN Shader* pShader );

    // Load binary and ASCII shader programs
    static RESULT LoadVertexShader     ( IN const string& filename, OUT GLuint* pVertexShader   );
    static RESULT LoadFragmentShader   ( IN const string& filename, OUT GLuint* pFragmentShader );
    static RESULT LoadBinaryShader     ( IN const string& filename, IN  GLenum  eShaderType, OUT GLuint* pShader );
    static RESULT LoadSourceShader     ( IN const string& filename, IN  GLenum  eShaderType, OUT GLuint* pShader );
    static RESULT LinkShaderProgram    ( IN GLuint vertexShader, IN GLuint fragmentShader, OUT GLuint* pShaderProgram );
    
protected:
    HShader m_defaultHShader;
};


#define ShaderMan ((ShaderManager&)ShaderManager::Instance())
    


} // END namespace Z



