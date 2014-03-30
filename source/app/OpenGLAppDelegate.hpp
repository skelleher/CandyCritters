//
//  OpenGLAppDelegate.h
//  Critters
//
//  Created by Sean Kelleher on 8/31/10.
//  Copyright Sean Kelleher 2010. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "OpenGLViewController.hpp"
#import "OpenGLView.hpp"
#import "Settings.hpp"
#import "PerfTimer.hpp"


@interface OpenGLAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow*               m_window;
    OpenGLView*             m_openglView;
    OpenGLViewController*   m_openglViewController;
    
    // TODO: GameScreens pointer
}

@property (nonatomic, retain) IBOutlet  UIWindow *window;

+ (OpenGLAppDelegate*)sharedInstance;

- (void)testBetaExpired;
- (void)startAnalytics;
- (void)endAnalytics;
- (void)enableAudio;


// Test for cracked app (obfuscated names)
// TODO: move into a separate class.
- (NSString*)stringFromBuffer:(const char*)buffer length:(unsigned int)theLength;
- (void)disableAudio;
- (BOOL)isAudioEnabled;
- (void)getWidth;

@end

