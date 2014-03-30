//
//  OpenGLView.mm
//  Critters
//
//  Created by Sean Kelleher on 8/31/10.
//  Copyright 2010 Sean Kelleher. All rights reserved.
//

#import "OpenGLView.hpp"

#import "Macros.hpp"
#import "Settings.hpp"
#import "Engine.hpp"

using Z::Log;
using Z::ZONE_INFO;
using Z::ZONE_WARN;
using Z::ZONE_ERROR;


@implementation OpenGLView

@synthesize renderContext = m_pRenderContext;


+ (Class) layerClass
{
    // Tell callers we're an OpenGL ES layer.
    return [CAEAGLLayer class];
}


- (id)initWithFrame:(CGRect)frame 
{
    float screenScaleFactor = 1.0f;

    if ((self = [super initWithFrame:frame])) 
    {
        DEBUGMSG(ZONE_INFO, "OpenGLView:initWithFrame: (%2.2fpts x %2.2fpts) @ (%2.2f, %2.2f)", 
                 frame.size.width, frame.size.height,
                 frame.origin.x, frame.origin.y);

        m_frameRect     = frame;
        m_lastTimestamp = 0.0f;

        
        //
        // Handle retina display
        //
        if([[UIScreen mainScreen] respondsToSelector: NSSelectorFromString(@"scale")])
        {
            if([self respondsToSelector: NSSelectorFromString(@"contentScaleFactor")])
            {
                // Set our glLayer contentScaleFactor
                screenScaleFactor  = [[UIScreen mainScreen] scale];

#ifdef DEBUG
                CAEAGLLayer *eaglLayer  = (CAEAGLLayer *)self.layer;
                DEBUGMSG(ZONE_INFO, "Main screen scale factor: %f", screenScaleFactor);
                DEBUGMSG(ZONE_INFO, "OpenGLView  scale factor: %f", self.contentScaleFactor);
                DEBUGMSG(ZONE_INFO, "eaglLayer   scale factor: %f", eaglLayer.contentsScale);
#endif                

                float customScaleFactor = Z::GlobalSettings.GetFloat("/Settings.fScreenScaleFactor", 1.0f);
                
                if (customScaleFactor != 1.0f)
                {
                    RETAILMSG(ZONE_WARN, "WARNING: /Settings.fScreenScaleFactor = %2.2f. Render buffer will be scaled by the OS.", customScaleFactor);
                    self.contentScaleFactor = screenScaleFactor * customScaleFactor;
                }
                else
                {
                    self.contentScaleFactor = screenScaleFactor;
                }
            }
        }

        //
        // Enable multi-touch
        //
        [self setMultipleTouchEnabled:YES];
        [self setUserInteractionEnabled:YES];

        //
        // Initialize EAGL for this View
        //
        bool bUseOpenGLES1 = Z::GlobalSettings.GetBool("/Settings.bUseOpenGLES1");
        RETAILMSG(ZONE_INFO, "Settings.bUseOpenGLES1 = %d", bUseOpenGLES1);
  
        if (bUseOpenGLES1)
        {
            m_pRenderContext = new Z::RenderContext( Z::RenderContext::RENDER_CONTEXT_OPENGLES1 );
        }
        else
        {
            m_pRenderContext = new Z::RenderContext( Z::RenderContext::RENDER_CONTEXT_OPENGLES2 );
        }

        if (!m_pRenderContext || FAILED(m_pRenderContext->Bind()))
        {
            RETAILMSG(ZONE_ERROR, "ERROR: OpenGLView:initWithFrame: failed to create and bind EAGL context.");
            [self release];
            return nil;
        }

        Z::Engine::SetRenderContext( m_pRenderContext );

        //
        // Init the renderer; also creates a RenderTarget, which needs to happen
        // before SetRenderBuffer() below.
        // Save a pointer to the IRenderer, so we can 
        // call m_pRenderer->Rotate() on device rotation.
        //
        m_pRenderer = &Z::Engine::GetRenderer();
        assert(m_pRenderer);
        m_pRenderer->Init( frame.size.width * screenScaleFactor, frame.size.height * screenScaleFactor );

        //
        // Assign backing storage to the renderbuffer.
        //
        CAEAGLLayer* eaglLayer = (CAEAGLLayer*) super.layer;
        eaglLayer.opaque = YES;
        m_pRenderContext->SetRenderBuffer( eaglLayer );

        //
        // Register for orientation changes
        //
        [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didRotate:) name:UIDeviceOrientationDidChangeNotification object:nil];
    }

Exit:    
    return self;
}



- (void) drawView: (CADisplayLink*) displayLink
{
    Z::Engine::Update();
    Z::Engine::Render();

//    m_pRenderContext->Present();
    
Exit:
    return;
}



- (void) didRotate:(NSNotification*)notification
{
    UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];

//    DEBUGMSG(ZONE_INFO, "OpenGLView:didRotate: %d", orientation );

//    CGRect frame = [self frame];
///    m_pRenderer->Resize( CGRectGetWidth(frame), CGRectGetHeight(frame) );

    m_pRenderer->Rotate( (Z::IRenderer::Orientation)orientation );
}



- (void)dealloc 
{
    DEBUGMSG(ZONE_INFO, "OpenGLView:dealloc: releasing EGL context");
    
    m_pRenderContext->Unbind();
    SAFE_RELEASE(m_pRenderContext);
    
    SAFE_RELEASE(m_pRenderer);
    
    [super dealloc];
}

@end
