/*
 *  ShaderManager.cpp
 *
 *  Created by Sean Kelleher on 9/26/10.
 *  Copyright 2010 Sean Kelleher. All rights reserved.
 *
 */

#include "ShaderManager.hpp"
#include "FileManager.hpp"
#include "Log.hpp"
#include "Handle.hpp"
#include "Settings.hpp"
#include "Platform.hpp"
#include "Types.hpp"
#include "Util.hpp"
#include "PerfTimer.hpp"
#include "Engine.hpp"   // HACK: so we can pre-warm shaders by calling into the Renderer.

#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES2/gl.h>



namespace Z
{



ShaderManager& 
ShaderManager::Instance()
{
    if (!s_pInstance)
    {
        s_pInstance = new ShaderManager();
    }
    
    return static_cast<ShaderManager&>(*s_pInstance);
}


ShaderManager::ShaderManager()
{
    RETAILMSG(ZONE_VERBOSE, "ShaderManager()");
    
    s_pResourceManagerName = "ShaderManager";
}


ShaderManager::~ShaderManager()
{
    RETAILMSG(ZONE_VERBOSE, "\t~ShaderManager()");
}



RESULT
ShaderManager::Init( IN const string& settingsFilename )
{
    RETAILMSG(ZONE_INFO, "ShaderManager::Init( %s )", settingsFilename.c_str());

    RESULT rval = S_OK;
    
    //
    // If the device doesn't support shaders, don't do anything
    //
    if (NULL == glGetString(GL_SHADING_LANGUAGE_VERSION))
    {
        RETAILMSG(ZONE_INFO, "ShaderManger::Init(): device does not support shaders.");
        // Clear the error!
        glGetError();
        return rval;
    }
    
    
    //
    // Create a Settings object and load the file.
    //
    Settings mySettings;
    if ( FAILED(mySettings.Read( settingsFilename )) )
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ShaderManager::Init( %s ): failed to load settings file", settingsFilename.c_str() );
        return E_UNEXPECTED;
    }

    

    //
    // Create each Shader.
    //
    UINT32 numShaders = mySettings.GetInt("/Shaders.NumShaders");
    char path[MAX_PATH];
    
    for (int i = 0; i < numShaders; ++i)
    {
        sprintf(path, "/Shaders/Shader%d", i);
        DEBUGMSG(ZONE_INFO, "Loading [%s]", path);
        string shaderSettings(path);

        string  name    = mySettings.GetString(shaderSettings + ".Name");
        string  vs      = mySettings.GetString(shaderSettings + ".VertexShader");
        string  fs      = mySettings.GetString(shaderSettings + ".FragmentShader");
        
        HShader hShader;
        Shader* pShader = NULL;
        RESULT  result  = CreateShader( name, vs, fs, &pShader );
        
        if (FAILED(result))
        {
            RETAILMSG(ZONE_ERROR, "ERROR: ShaderManager::Init( %s ): failed to create Shader", name.c_str());
            
            if (name == "Default")
            {
                DEBUGCHK(0);
            }
            else
            {
                // Continue loading other Shaders rather than aborting.
                continue;
            }
        }
        
        DEBUGMSG(ZONE_SHADER, "Created Shader [%s]", pShader->GetName().c_str());
        CHR(Add(pShader->GetName(), pShader, &hShader));

        
        // Save the first shader as our default.
        if (m_defaultHShader.IsNull())
        {
            m_defaultHShader = hShader;
        }
    }

Exit:    
    return rval;
}



GLuint
ShaderManager::GetShaderProgramID( IN HShader hShader    )
{
    GLuint rval = 0xFFFFFFFF;
    
    Shader* pShader = GetObjectPointer( hShader );
    if (pShader)
    {
        rval = pShader->GetProgramID();
    }
    
Exit:
    return rval;
}



GLuint
ShaderManager::GetShaderProgramID( IN const string& shaderName )
{
    GLuint rval = 0xFFFFFFFF;
    
    HShader hShader;
    Get(shaderName, &hShader);
    
    Shader* pShader = GetObjectPointer( hShader );
    if (pShader)
    {
        rval = pShader->GetProgramID();
    }
    
Exit:
    return rval;
}



RESULT
ShaderManager::CreateShader( IN const string& name, IN const string& vertexShaderName, IN const string& fragmentShaderName, INOUT HShader* pHShader )
{
    RESULT  rval            = S_OK;
    Shader* pShader         = NULL;
    HShader hShader;

    if ("" == name || "" == vertexShaderName || "" == fragmentShaderName)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ShaderManager::CreateShader(): empty name");
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    CHR(CreateShader( name, vertexShaderName, fragmentShaderName, &pShader));
    
    if (pShader)
    {
        DEBUGMSG(ZONE_SHADER, "Loaded Shader [%s]", pShader->GetName().c_str());
        CHR(Add(pShader->GetName(), pShader, &hShader));
    }
    
    if (pHShader)
    {
        *pHShader = hShader;
    }

Exit:
    return rval;
}



RESULT
ShaderManager::CreateShader( IN const string& name, IN const string& vertexShaderName, IN const string& fragmentShaderName, INOUT Shader** ppShader )
{
    RESULT  rval            = S_OK;
    GLuint  vertexShader    = 0;
    GLuint  fragmentShader  = 0;
    GLuint  shaderProgram   = 0;
    Shader* pShader         = NULL;
    HShader hShader;
    
    if (!ppShader)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ShaderManager::CreateShader(): NULL pointer");
        rval = E_NULL_POINTER;
        goto Exit;
    }
    
    if ("" == name || "" == vertexShaderName || "" == fragmentShaderName)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ShaderManager::CreateShader(): empty name");
        rval = E_INVALID_ARG;
        goto Exit;
    }

    // Load, compile, and link the shaders into a program.
    CHR(LoadVertexShader  (vertexShaderName,   &vertexShader  ));
    CHR(LoadFragmentShader(fragmentShaderName, &fragmentShader));
    CHR(LinkShaderProgram (vertexShader, fragmentShader, &shaderProgram))

    // Create a Shader object.
    pShader = new Shader();
    CPREx(pShader, E_OUTOFMEMORY);
    CHR(pShader->Init( name, shaderProgram ));

    // Caller must AddRef()
    *ppShader = pShader;

    RETAILMSG(ZONE_SHADER, "Shader[%4d]: program: %d \"%-32s\" %s %s",
              pShader->GetID(), shaderProgram, name.c_str(), vertexShaderName.c_str(), fragmentShaderName.c_str());
    
Exit:
    return rval;
}



RESULT
ShaderManager::PrewarmShaders( )
{
    RESULT rval = S_OK;
    
    ResourceListIterator ppShader;
    
    for (ppShader = m_resourceList.begin(); ppShader != m_resourceList.end(); ++ppShader)
    {
        Shader* pShader = *ppShader;
        
        if (pShader)
        {
            IGNOREHR(PrewarmShader( pShader ));
        }
    }

Exit:
    return rval;
}



RESULT
ShaderManager::PrewarmShader( IN Shader* pShader )
{
    RESULT rval = S_OK;
    
    CPR(pShader);
    
    static PerfTimer timer;
    timer.Start();
    
    static Rectangle rect = { 0,0,32,32 };
    static Vertex    pVertices[4];
    static bool      initialized = false;
    
    if (!initialized)
    {
        CHR(Util::CreateTriangleStrip( &rect, pVertices ));
        initialized = true;
    }
    
    
    //
    // Render a single small quad.
    // This forces the Effect to compile its shaders and create scratch surfaces,
    // ensuring it is instantly ready the first time it's used for real.
    // Otherwise, the user may see a delay (especially bad when showing/hiding a Scene).
    //
    CHR(Renderer.BeginFrame());
    CHR(pShader->PreRender());
    CHR(Renderer.DrawTriangleStrip( pVertices, 4 ));
    CHR(pShader->PostRender());
    CHR(Renderer.EndFrame());

    
    timer.Stop();
    
    RETAILMSG(ZONE_SHADER, "ShaderManager::PrewarmShader( \"%s\" ): %4.2f ms",
              pShader->GetName().c_str(),
              timer.ElapsedMilliseconds());
    
Exit:
    return rval;
}





// ============================================================================
//
//  Private classes; other systems only see Handles to these
//
// ============================================================================

#pragma mark Static Methods

RESULT 
ShaderManager::LoadBinaryShader( IN const string& filename, IN GLenum eShaderType, OUT GLuint* pShader )
{
    RESULT  rval            = E_FAIL;
    GLint   nBinaryFormats  = -1;
    GLint*  pBinaryFormats  = NULL;
    HFile   hShader;
    UINT32  size            = 0;
    UINT32  numBytesRead    = 0;
    BYTE*   pBytes          = NULL;
    
    
    if ("" == filename)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ShaderManager::LoadBinaryShader(): empty name");
        rval = E_INVALID_ARG;
        goto Exit;
    }
    
    CPREx(pShader, E_NULL_POINTER);
    
//    RETAILMSG(ZONE_INFO, "Loading shader [%s]", filename.c_str());
    
    // Which binary formats does the OpenGL ES driver support?
    VERIFYGL(glGetIntegerv(GL_NUM_SHADER_BINARY_FORMATS, &nBinaryFormats));
    if (nBinaryFormats <= 0)
    {
        CHR(E_FAIL);
    }
    
    pBinaryFormats = new GLint[nBinaryFormats];
    CPREx(pBinaryFormats, E_OUTOFMEMORY);
    pBinaryFormats[0] = 0xDEADBEEF;
    
    glGetIntegerv(GL_SHADER_BINARY_FORMATS, pBinaryFormats);
    if (0xDEADBEEF == pBinaryFormats[0])
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ShaderManager::LoadBinaryShader(): GL_SHADER_BINARY_FORMATS not supported on this platform; driver bug!");
        rval = E_UNEXPECTED;
        goto Exit;
    }

    
    // Try to load a binary shader of each format until we find one
    rval = E_FAIL;
    for (int i = 0; i < nBinaryFormats; i++)
    {
        RETAILMSG(ZONE_INFO, "Loading shader [%s]", filename.c_str());
        
        // Open file and read the bytes.
        CHR(FileMan.OpenFile( filename, &hShader ));
        CHR(FileMan.GetFileSize( hShader, &size ));
        pBytes = new BYTE[size];
        CPREx(pBytes, E_OUTOFMEMORY);
        CHR(FileMan.ReadFile( hShader, pBytes, size, &numBytesRead ));
        DEBUGCHK(size == numBytesRead);
        
        VERIFYGL(*pShader = glCreateShader(eShaderType));
        
        glShaderBinary(1, pShader, pBinaryFormats[i], pBytes, size);
        
        GLint error = glGetError();
        if (error != GL_NO_ERROR)
        {
            RETAILMSG(ZONE_WARN, "glShaderBinary(): error = 0x%x", error);
            glDeleteShader(*pShader);
            *pShader = 0;
            
            continue;
        }
        
        rval = S_OK; // We found one
        break;
    }
    
Exit:
    SAFE_ARRAY_DELETE(pBytes);
    FileMan.Release( hShader );
    return rval;
}



RESULT 
ShaderManager::LoadSourceShader( IN const string& filename, IN GLenum eShaderType, OUT GLuint* pShader )
{
    RESULT      rval                = S_OK;
    GLint       shaderCompiled      = 0;
    GLboolean   bHasShaderCompiler  = false;
    BYTE*       pShaderSource       = NULL;
    HFile       hShader;
    UINT32      size                = 0;
    UINT32      numBytesRead        = 0;
    

    if ("" == filename)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ShaderManager::LoadSourceShader(): empty filename");
        rval = E_INVALID_ARG;
        goto Exit;
    }

    CPREx(pShader, E_NULL_POINTER);
   
    RETAILMSG(ZONE_SHADER, "Loading shader [%s]", filename.c_str());
    
    // Does this platform support shader compilation?
    VERIFYGL(glGetBooleanv(GL_SHADER_COMPILER, &bHasShaderCompiler));
    if (!bHasShaderCompiler)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: ShaderManager::LoadSourceShader(): this platform doesn't support shader compilation.");
        rval = E_UNEXPECTED;
        goto Exit;
    }
    
 

    //
    // Open file and read the source.
    //
    CHR(FileMan.OpenFile( filename, &hShader ));
    CHR(FileMan.GetFileSize( hShader, &size ));
    pShaderSource = new BYTE[size + 1];    // BUG BUG: need to NULL-terminate the source, or compilation randomly chokes
    CPREx(pShaderSource, E_OUTOFMEMORY);
    CHR(FileMan.ReadFile( hShader, pShaderSource, size, &numBytesRead ));
    DEBUGCHK(size == numBytesRead);
    pShaderSource[size] = '\0';             // BUG BUG: need to NULL-terminate the source, or compilation randomly chokes
    

    //RETAILMSG(ZONE_SHADER, "Shader = \n[\n%s]", pShaderSource);
    
    
    //
    // We have the source code; compile it.
    //
    *pShader = glCreateShader(eShaderType);
    
    // Compile the shader.
    VERIFYGL(glShaderSource(*pShader, 1, (const GLchar**)&pShaderSource, NULL));
    VERIFYGL(glCompileShader(*pShader));
    
    // Test if compilation succeeded.
    glGetShaderiv(*pShader, GL_COMPILE_STATUS, &shaderCompiled);
    if (!shaderCompiled)
    {
        int i32InfoLogLength = 0;
        glGetShaderiv(*pShader, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
        
        if (i32InfoLogLength > 0)
        {
            char* pszInfoLog = new char[i32InfoLogLength];
            
            if (pszInfoLog)
            {
                glGetShaderInfoLog(*pShader, i32InfoLogLength, NULL, pszInfoLog);
                RETAILMSG(ZONE_ERROR, "CompileShader \"%s\" Error: %s\n", filename.c_str(), pszInfoLog);
                delete [] pszInfoLog;
                
                DEBUGCHK(0);
            }
        }
        
        glDeleteShader(*pShader);
        *pShader = 0;
        CHR(E_FAIL);
    }
    
    
Exit:
    SAFE_ARRAY_DELETE(pShaderSource);
////    FileMan.CloseHandle(hShader);
    FileMan.CloseFile(hShader);
    return rval;
}


RESULT 
ShaderManager::LoadVertexShader( IN const string& filename, OUT GLuint* pVertexShader )
{
    RESULT rval = S_OK;
    
    if (FAILED( LoadBinaryShader( filename, GL_VERTEX_SHADER, pVertexShader ) ))
    {
        char shaderName[MAX_PATH];
        sprintf(shaderName, "%s.glsl", filename.c_str());
        rval = LoadSourceShader( shaderName, GL_VERTEX_SHADER, pVertexShader );
    }
    
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: failed to load or compile shader [%s]", filename.c_str());
    }
    
    return rval;
}



RESULT 
ShaderManager::LoadFragmentShader( IN const string& filename, OUT GLuint* pFragmentShader )
{
    RESULT rval = S_OK;
    
    if (FAILED( LoadBinaryShader( filename, GL_FRAGMENT_SHADER, pFragmentShader ) ))
    {
        char shaderName[MAX_PATH];
        sprintf(shaderName, "%s.glsl", filename.c_str());
        rval = LoadSourceShader( shaderName, GL_FRAGMENT_SHADER, pFragmentShader );
    }
    
    if (FAILED(rval))
    {
        RETAILMSG(ZONE_ERROR, "ERROR: failed to load or compile shader [%s]", filename.c_str());
    }
    
    return rval;
}



RESULT 
ShaderManager::LinkShaderProgram( IN GLuint vertexShader, IN GLuint fragmentShader, OUT GLuint* pShaderProgram )
{
    RESULT  rval = S_OK;
    GLint   linkStatus      = 0;
    GLuint  shaderProgram   = 0;
    
    CPREx(pShaderProgram, E_NULL_POINTER);
    
    // Allocate a shader program handle.
    shaderProgram = glCreateProgram();
    CBR(shaderProgram != 0);
    
    // Attach the vertex and fragment shaders.
    VERIFYGL(glAttachShader(shaderProgram, vertexShader));
    VERIFYGL(glAttachShader(shaderProgram, fragmentShader));
    
    // Bind vertex attributes that all shaders require.
    // Subclasses may bind additional attributes.
    VERIFYGL(glBindAttribLocation(shaderProgram, ATTRIBUTE_VERTEX_POSITION,       "aPosition"));
    VERIFYGL(glBindAttribLocation(shaderProgram, ATTRIBUTE_VERTEX_DIFFUSE,        "aDiffuse"));
    VERIFYGL(glBindAttribLocation(shaderProgram, ATTRIBUTE_VERTEX_TEXTURE_COORD0, "aTextureCoordinate0"));
    
    // Link the program.
    VERIFYGL(glLinkProgram(shaderProgram));
    VERIFYGL(glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus));
    if (!linkStatus)
    {
        int i32InfoLogLength = 0, i32CharsWritten = 0;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &i32InfoLogLength);

        if (i32InfoLogLength > 0)
        {
            char* pszInfoLog = new char[i32InfoLogLength];

            if (pszInfoLog)
            {
                glGetProgramInfoLog(shaderProgram, i32InfoLogLength, &i32CharsWritten, pszInfoLog);
                RETAILMSG(ZONE_ERROR, "LinkShaderProgram Error: %s\n", pszInfoLog);
                delete [] pszInfoLog;
            }
        }
    }
    
    *pShaderProgram = shaderProgram;
    
Exit:
    if (FAILED(rval))
    {
        glDeleteProgram(shaderProgram);
        if (pShaderProgram)
            *pShaderProgram = 0;
    }
    
    return rval;
}





// ============================================================================
//
//  Private classes; other systems only see Handles to these
//
// ============================================================================




// ============================================================================
//
//  Shader Implementation
//
// ============================================================================


#pragma mark Shader Implementation

Shader::Shader() :
    m_shaderProgram(0xFFFFFFFF)
{
    m_name = "Shader";
    
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "Shader( %4d )", m_ID);
}



Shader::~Shader()
{
    RETAILMSG(ZONE_OBJECT | ZONE_VERBOSE, "\t~Shader( %4d )", m_ID);
}



RESULT
Shader::Init( const string& name, GLuint shaderProgram )
{
    RESULT rval = S_OK;
    
    RETAILMSG(ZONE_OBJECT, "Shader[%4d]::Init( %s, %d )", 
              m_ID, name.c_str(), shaderProgram);
    
    m_name          = name;
    m_shaderProgram = shaderProgram;
    
    return rval;
}



RESULT
Shader::PreRender() const
{
    RESULT rval = S_OK;
    
    VERIFYGL(glUseProgram(m_shaderProgram));
       
Exit:
    return rval;
}



RESULT
Shader::PostRender() const
{
    return S_OK;
}


const string&
Shader::GetName() const
{
    return m_name;
}


const GLuint
Shader::GetProgramID( ) const
{
    return m_shaderProgram;
}



} // END namespace Z


