//
//  UIViewOverlay.mm
//  Critters
//
//  Created by Sean Kelleher on 4/5/11.
//  Copyright 2011 Sean Kelleher. All rights reserved.
//
//
// Overriden hit-testing for UIViewOverlay.
//
// We test only against our children (assumes the UIViewOverlay itself is transparent).
// If the point is not inside a child we return false.  
// This propagates the touch up the responder chain, allowing subviews stacked below us to respond.
//
// Great for game HUDs where you want to click both on HUD buttons, but also objects below the HUD.
//

#import "UIViewTransparent.h"


@implementation UIViewTransparent

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code
    }
    return self;
}


- (void)dealloc
{
    [super dealloc];
}


//
// Override hit-testing for UIViewOverlay.
//
// We test only against our children (assumes the UIViewOverlay itself is transparent).
// If the point is not inside a child we return false.  
// This propagates the touch up the responder chain, allowing subviews stacked below us to respond.
//
// Great for game HUDs where you want to click both on HUD buttons, but also objects below the HUD.
//
- (BOOL)pointInside:(CGPoint)point withEvent:(UIEvent *)event 
{
    // UIView will be "transparent" for touch events if we return NO
    for (UIView* subview in self.subviews)
    {
        // Only hit-test against UIControls.
        if ( ![subview isKindOfClass:[UIControl class]] )
        {
            continue;
        }
    
        CGPoint childPoint = [subview convertPoint:point fromView:self];
        if ( [subview pointInside:childPoint withEvent:event] )
        {
            return YES;
        }
    }
    
    return NO;
}


@end
