//
//  OpenGLView.hpp
//  Critters
//
//  Created by Sean Kelleher on 8/31/10.
//  Copyright 2010 Sean Kelleher. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

#import "IRenderer.hpp"
#import "RenderContext.hpp"
#import "PerfTimer.hpp"


@interface OpenGLView : UIView 
{
    CGRect              m_frameRect;
    float               m_lastTimestamp;

    Z::IRenderer       *m_pRenderer;
    Z::RenderContext   *m_pRenderContext;
    Z::PerfTimer        m_perfTimer;
}

@property (nonatomic, readonly) Z::RenderContext* renderContext;

- (void) drawView:  (CADisplayLink*)  displayLink;
- (void) didRotate: (NSNotification*) notification;

@end
