//
//  OpenGLViewController.h
//  Critters
//
//  Created by Sean Kelleher on 9/6/10.
//  Copyright 2010 Sean Kelleher. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface OpenGLViewController : UIViewController 
{
}

@property (nonatomic, retain) CADisplayLink* displayLink;


// TODO: move these to the app delegate?

- (void) touchesBegan:      (NSSet*)touches withEvent:(UIEvent*)event;
- (void) touchesMoved:      (NSSet*)touches withEvent:(UIEvent*)event;
- (void) touchesEnded:      (NSSet*)touches withEvent:(UIEvent*)event;
- (void) touchesCancelled:  (NSSet*)touches withEvent:(UIEvent*)event;


- (void) motionBegan:    (UIEventSubtype)motion withEvent:(UIEvent *)event;
- (void) motionEnded:    (UIEventSubtype)motion withEvent:(UIEvent *)event;
- (void) motionCancelled:(UIEventSubtype)motion withEvent:(UIEvent *)event;


@end
