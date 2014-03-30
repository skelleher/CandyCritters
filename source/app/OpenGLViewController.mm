//
//  OpenGLViewController.mm
//  Critters
//
//  Created by Sean Kelleher on 9/6/10.
//  Copyright 2010 Sean Kelleher. All rights reserved.
//

#import "OpenGLViewController.hpp"
#import "OpenGLView.hpp"
#import "Log.hpp"
#import "Engine.hpp"

// TEST:
//#import "DebugViewController.hpp"


using Z::Log;
using Z::Engine;
using Z::ZONE_INFO;
using Z::ZONE_WARN;
using Z::ZONE_ERROR;


@implementation OpenGLViewController

@synthesize displayLink;


- (id) init
{
    if ((self = [super initWithNibName:nil bundle:nil]))
    {
    }
    
    return self;
}



// The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
- (id) initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil 
{
    return [self init];
}



/*
// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView 
{
}
*/



// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad 
{
    [super viewDidLoad];
}


- (void)viewWillAppear:(BOOL)animated
{
    //
    // Call drawView: on every screen refresh.
    //
    self.displayLink = [CADisplayLink displayLinkWithTarget:self.view selector:@selector(drawView:)];
    [self.displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];

    //
    // Screen refreshes at 60fps by default, but we can redraw at 30fps to save CPU/battery.
    //
    int frameRate = Z::GlobalSettings.GetInt("/Settings.FrameRateHZ", 60);
    RETAILMSG(ZONE_INFO, "/Settings.FrameRateHZ = %d", frameRate);
    [displayLink setFrameInterval:60/frameRate];
}



- (void)viewDidAppear:(BOOL)animated 
{
    // This is necessary to receive motionBegan: messages.
    // It is not needed for touch screen messages.
    [self becomeFirstResponder];
}


- (void)viewWillDisappear:(BOOL)animated
{
    // Remove drawView: from the run loop (i.e. disable OpenGL rendering).
    // Otherwise we may continue to render when hidden or entering the background, which is an error.

    [self.displayLink removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}



- (BOOL)canBecomeFirstResponder 
{
    return YES;
}



// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation 
{
    // Return YES for supported orientations
    switch (interfaceOrientation)
    {
        case UIInterfaceOrientationPortrait:
            return YES;
            break;
        case UIInterfaceOrientationLandscapeLeft:
        case UIInterfaceOrientationLandscapeRight:
            return NO;
            break;
        default:
            return NO;
    };
}



- (void)didReceiveMemoryWarning 
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}


- (void)viewDidUnload 
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}


- (void)dealloc 
{
    [super dealloc];
}


#pragma mark Touch Input
//
// Delegate touch events in this view to ZEngine, where they will be converted to messages
// and dispatched to listening game objects or UI elements.
//
- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
    DEBUGMSG(Z::ZONE_TOUCH | Z::ZONE_VERBOSE, "OpenGLViewController::touchesBegan()");

    Z::TouchEvent touchEvent;

    // Delegate each touch to ZEngine for handling
    for (UITouch* touch in touches)
    {
        CGPoint point           = [touch locationInView:[self view]];
        
        touchEvent.id           = (Z::UINT32)touch;
        touchEvent.type         = Z::TOUCH_EVENT_BEGIN;
        touchEvent.timestamp    = static_cast<Z::UINT64>([touch timestamp] * 1000);
        touchEvent.point.x      = point.x;
        touchEvent.point.y      = point.y;
        
        TouchScreen.BeginTouch( &touchEvent );
    }
}


- (void) touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event
{
    //DEBUGMSG(Z::ZONE_TOUCH | Z:ZONE_VERBOSE, "OpenGLViewController::touchesMoved()");

    Z::TouchEvent touchEvent;

    // Delegate each touch to ZEngine for handling
    for (UITouch* touch in touches)
    {
        CGPoint point           = [touch locationInView:[self view]];
        
        touchEvent.id           = (Z::UINT32)touch;
        touchEvent.type         = Z::TOUCH_EVENT_UPDATE;
        touchEvent.timestamp    = static_cast<Z::UINT64>([touch timestamp] * 1000);
        touchEvent.point.x      = point.x;
        touchEvent.point.y      = point.y;
        
        TouchScreen.UpdateTouch( &touchEvent );
    }
}



- (void) touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
    DEBUGMSG(Z::ZONE_TOUCH | Z::ZONE_VERBOSE, "OpenGLViewController::touchesEnded()");

    Z::TouchEvent touchEvent;

    // Delegate each touch to ZEngine for handling
    for (UITouch* touch in touches)
    {
        CGPoint point           = [touch locationInView:[self view]];
        
        touchEvent.id           = (Z::UINT32)touch;
        touchEvent.type         = Z::TOUCH_EVENT_END;
        touchEvent.timestamp    = static_cast<Z::UINT64>([touch timestamp] * 1000);
        touchEvent.point.x      = point.x;
        touchEvent.point.y      = point.y;
        
        TouchScreen.EndTouch( &touchEvent );
    }
}



- (void) touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event
{
    //DEBUGMSG(ZONE_TOUCH, "OpenGLViewController::touchesCancelled()");

    Z::TouchEvent touchEvent;

    // Delegate each touch to ZEngine for handling
    for (UITouch* touch in touches)
    {
        CGPoint point           = [touch locationInView:[self view]];
        
        touchEvent.id           = (Z::UINT32)touch;
        touchEvent.type         = Z::TOUCH_EVENT_CANCEL;
        touchEvent.timestamp    = static_cast<Z::UINT64>([touch timestamp] * 1000);
        touchEvent.point.x      = point.x;
        touchEvent.point.y      = point.y;
        
        TouchScreen.EndTouch( &touchEvent );
    }
}



#pragma mark Motion Handlers
- (void)motionBegan:(UIEventSubtype)motion withEvent:(UIEvent *)event
{
    RETAILMSG(ZONE_INFO, "motionBegan:");
}

 

- (void)motionEnded:(UIEventSubtype)motion withEvent:(UIEvent *)event
{
    RETAILMSG(ZONE_INFO, "motionEnded:");

    static Z::UINT32    s_eventId = 0;
    
    Z::AccelerometerEvent accelerometerEvent;
    
    accelerometerEvent.id        = s_eventId++;
    accelerometerEvent.type      = Z::ACCELEROMETER_EVENT_SHAKE;
    accelerometerEvent.timestamp = static_cast<Z::UINT64> ([event timestamp]);
    
    Accelerometer.DeliverEvent( &accelerometerEvent );
    
    //
    // TEST TEST TEST:
    //
    //  Show the log in a modal view.
    //  Good for tracking down rare bugs seen in the field.
    //
/*    
    DebugViewController* debugViewController    = [[DebugViewController alloc] initWithNibName:@"DebugView.xib" bundle:nil];
    UIView*              debugView              = [debugViewController view];
    
    [self.view addSubview:debugView];
//    [self.parentViewController.view addSubview:debugView];
//    [self.parentViewController.view sendSubviewToBack:self.view];
*/
}

 

- (void)motionCancelled:(UIEventSubtype)motion withEvent:(UIEvent *)event
{
    RETAILMSG(ZONE_INFO, "motionCancelled:");
}




@end
