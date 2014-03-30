#include "RenderContext.hpp"

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>     // for GL_RENDERBUFFER
#import <OpenGLES/ES1/glext.h>  // for GL_RENDERBUFFER_OES
#import <QuartzCore/QuartzCore.h>

    
namespace Z
{



RenderContext::RenderContext( RENDER_CONTEXT_TYPE type )  :
    m_EAGLContext(NULL)
{ 
    m_type = type;
    
    switch (type)
    {
        case RENDER_CONTEXT_OPENGLES1:
            m_EAGLContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
            break;
        case RENDER_CONTEXT_OPENGLES2:
            m_EAGLContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
            break;
        default:
            RETAILMSG(ZONE_ERROR, "RenderContext(): invalid type %d", type);
            m_EAGLContext = nil;
            DEBUGCHK(0);
    };
    
    if (!m_EAGLContext)
    {
        RETAILMSG(ZONE_ERROR, "ERROR: RenderContext(): failed to allocate m_EAGLContext");
        DEBUGCHK(0);
    }
    else 
    {
        RETAILMSG(ZONE_INFO, "Created RenderContext");
    }

    
};


    
RenderContext::~RenderContext()
{
    Unbind();
    [(EAGLContext*)m_EAGLContext release];
}



RESULT
RenderContext::SetRenderBuffer( void* layer )
{
    RESULT rval = S_OK;
    CPR(m_EAGLContext);
    CPR(layer)

    switch (m_type)
    {
        case (RENDER_CONTEXT_OPENGLES1):
            CBR([(EAGLContext*)m_EAGLContext renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)layer]);
            break;
        case (RENDER_CONTEXT_OPENGLES2):
            CBR([(EAGLContext*)m_EAGLContext renderbufferStorage:GL_RENDERBUFFER     fromDrawable:(CAEAGLLayer*)layer]);
            break;
        default:
            DEBUGCHK(0);
    };
    
Exit:
    if (FAILED(rval))
        RETAILMSG(ZONE_ERROR, "ERROR: RenderContext::SetRenderBuffer( 0x%x ) failed.", (UINT32)layer);
        
    return rval;
}
    


RESULT
RenderContext::Bind()
{
    RESULT rval = S_OK;
    CPR(m_EAGLContext);

    if ( ! [EAGLContext setCurrentContext:(EAGLContext*)m_EAGLContext])
    {
        rval = E_FAIL;
    }
    
Exit:
    return rval;
}



RESULT
RenderContext::Unbind()
{
    RESULT rval = S_OK;
    CPR(m_EAGLContext);

    if ( ! [EAGLContext setCurrentContext:nil])
    {
        rval = E_FAIL;
    }
    
Exit:
    return rval;
}



RESULT
RenderContext::Present()
{
    DEBUGMSG(ZONE_RENDER, "--- PRESENT FRAME ---");

    RESULT rval = S_OK;
    CPR(m_EAGLContext);

    if ( ! [(EAGLContext*)m_EAGLContext presentRenderbuffer:GL_RENDERBUFFER_OES])
    {
        rval = E_FAIL;
    }
    
Exit:
    return rval;
}


} // END namespace Z
