#pragma once

#include "Types.hpp"
#include "Errors.hpp"
#include "Log.hpp"
#include "Object.hpp"
    
namespace Z
{



class RenderContext : virtual public Object
{
public:
    typedef enum 
    {
        RENDER_CONTEXT_OPENGLES1 = 0,
        RENDER_CONTEXT_OPENGLES2,
    } RENDER_CONTEXT_TYPE;


public:
    RenderContext( RENDER_CONTEXT_TYPE type );    
    virtual ~RenderContext();


    RESULT  SetRenderBuffer( void* layer );
    RESULT  Bind();
    RESULT  Unbind();
    RESULT  Present();

protected:
    RenderContext(const RenderContext& rhs);
    RenderContext& operator=(const RenderContext& rhs);
    
protected:
    RENDER_CONTEXT_TYPE  m_type;
    void                *m_EAGLContext;
};



} // END namespace Z
