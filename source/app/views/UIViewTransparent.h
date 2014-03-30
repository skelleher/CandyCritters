//
//  UIViewTransparent.h
//  Critters
//
//  Created by Sean Kelleher on 4/5/11.
//  Copyright 2011 Sean Kelleher. All rights reserved.
//
//
// Overriden hit-testing for UIViewTransparent.
//
// We test only against our children.
// If the point is not inside a child we return false.  
// This propagates the touch up the responder chain, allowing subviews stacked below us to respond.
//
// Great for game HUDs where you want to click both on HUD buttons, but also objects below the HUD.
//

#import <UIKit/UIKit.h>


@interface UIViewTransparent : UIView {
    
}

@end
