//
//  OpenGLAppDelegate.mm
//  Critters
//
//  Created by Sean Kelleher on 8/31/10.
//  Copyright Sean Kelleher 2010. All rights reserved.
//

#import "OpenGLAppDelegate.hpp"
#import "Platform.hpp"
#import "Engine.hpp"
#import "Game.hpp"
#import "Level.hpp"                         // for struct Level
#import "GameState.hpp"                     // for extern Level* g_pLevel;
#import <AVFoundation/AVAudioSession.h>
#import "LocalyticsSession.h"
#import <targetconditionals.h>
#import "OpenGLViewController.hpp"
#import "HomeScreenViewController.h"
#import "PauseScreenViewController.h"
#import "GameOverScreenViewController.h"
#import "HUDViewController.h"
#import "ConfirmViewController.h"
#import "AboutViewController.h"
#import "TutorialViewController.h"
#import "LevelViewController.h"


using Z::Log;
using Z::Engine;
using Z::Platform;
using Z::ZONE_INFO;
using Z::ZONE_ERROR;


@implementation OpenGLAppDelegate
@synthesize     window          = m_window;



// HACK HACK: just to get analytics working quickly.
namespace Z
{
    extern UINT32           g_totalScore;
    extern UINT32           g_highestLevel;
    extern UINT32           g_highestLevelEver;

    extern struct Level*    g_pLevel;
}


// HACK HACK: to get Views working quickly.
UIView*             homeScreenView;
UIView*             pauseScreenView;
UIView*             gameOverScreenView;
UIView*             hudView;
UIView*             confirmQuitView;
UIView*             aboutView;

// HACK HACK: to get Views working quickly.
UIViewController*   homeScreenViewController;
UIViewController*   pauseScreenViewController;
UIViewController*   gameOverScreenViewController;
UIViewController*   hudViewController;
UIViewController*   aboutViewController;
UIViewController*   tutorialViewController;
UIViewController*   levelViewController;
ConfirmViewController* confirmViewController;


static OpenGLAppDelegate* s_pSharedInstance = nil;



+ (OpenGLAppDelegate*)sharedInstance
{
    return s_pSharedInstance;
}



#pragma mark -
#pragma mark Application lifecycle


- (id)init
{
    if (s_pSharedInstance)
    {
        NSLog(@"ERROR: you have created multiple instances of OpenGLAppDelegate.");
        assert(0);
    }

    self = [super init];
    s_pSharedInstance = self;
    
    return self;
}   



- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions 
{    
    [self startAnalytics];


    // screenRect is in POINTS, *NOT* PIXELS on iOS devices.
    // So iPhone4 == 320x480 points == 640x960 pixels.
    CGRect screenRect  = [[UIScreen mainScreen] bounds];

    float screenScale = 1.0f;
    if (!Z::Platform::IsIPhone3G())
    {
        screenScale = [[UIScreen mainScreen] scale];
    }

    
    // Setting this in .plist isn't working; for now do it here.
    [application setStatusBarHidden:YES];

    

    //
    // Init the game engine here, so we have logging and settings.
    // TODO: we really need logging/settings split out from engine init,
    // because engine init pulls in too much, like creating the renderer.
    //
    Z::Engine::Init();


    //
    // If we add a .nib later, we don't need to allocate the window; it'll be dehydrated on launch.
    //
    m_window = [[UIWindow alloc] initWithFrame: screenRect];


    //
    // TODO: move all this to a deferred function call after didFinishLaunching.
    // The splash screen won't hide until this function returns!
    //


    [self enableAudio];
    

    //
    // Create the OpenGLView.
    // This has a side effect of initializing OpenGL ES and the Renderer.
    // We need to do this, unfortunately, before Z::Engine::LoadResources().
    //
    m_openglView            = [[OpenGLView alloc] initWithFrame: screenRect];
    m_openglViewController  = [[OpenGLViewController alloc] init];
    [m_openglViewController setView:m_openglView];
    m_window.rootViewController = m_openglViewController;

    //
    // Load resources, including sounds and shaders, that
    // may be used by the following views.
    // TODO: alloc VCs only on demand, so we won't need to call LoadResources()
    // here ( it is properly called from Game::Start() ).
    //
    Z::Engine::LoadResources();
    
    
    //
    // Create the game view controllers.
    // TODO: these need to be created on demand by SceneManager.
    //
    homeScreenViewController        = [[HomeScreenViewController      alloc] init];
    pauseScreenViewController       = [[PauseScreenViewController     alloc] init];
    gameOverScreenViewController    = [[GameOverScreenViewController  alloc] init];
    hudViewController               = [[HUDViewController             alloc] init];
    confirmViewController           = [[ConfirmViewController         alloc] init];
    aboutViewController             = [[AboutViewController           alloc] init];
    tutorialViewController          = [[TutorialViewController        alloc] init];
    levelViewController             = [[LevelViewController           alloc] init];

    [m_window addSubview:m_openglView];
    [m_window makeKeyAndVisible];

    Z::Game::Start();

    //
    // Go to the home screen.
    //
    // TODO: replace all of these with SceneManager calls.
    GameObjects.SendMessageFromSystem( MSG_GameScreenTest );

	return YES;
}


- (NSString*)stringFromBuffer:(const char*)buffer length:(unsigned int)theLength
{
    char ascii[1024]  = { 0 };
    for (int src = theLength-1, dst = 0; src >= 0; --src, ++dst)
    {
        ascii[dst] = buffer[src] + ' ';
    }

    NSString* string = [NSString stringWithCString:ascii encoding:NSUTF8StringEncoding];
    
    return string;
}



- (void)testBetaExpired
{
#ifndef SHIPBUILD
    NSDateFormatter *dateFormatter  = [[[NSDateFormatter alloc] init] autorelease];
    NSLocale        *localeUS       = [[[NSLocale alloc] initWithLocaleIdentifier:@"en_US"] autorelease];
    
    [dateFormatter setDateFormat:@"MMM d yyyy"];
    [dateFormatter setLocale:localeUS];

    NSDate          *compileDate    = [dateFormatter dateFromString:[NSString stringWithUTF8String:__DATE__]];
    NSDate          *currentDate    = [NSDate date];

    NSTimeInterval  elapsedSeconds  = [currentDate timeIntervalSinceDate:compileDate];
    
    NSLog(@"%f seconds elapsed since compile.", elapsedSeconds);

    // Expire after 30 days
    if ( elapsedSeconds > (NSTimeInterval)(60 * 60 * 24 * 30) ) 
    {
        NSLog(@"------------------------------------");
        NSLog(@"----      Beta has expired      ----");
        NSLog(@"------------------------------------");

        UIAlertView *expirationAlert = [[UIAlertView alloc] initWithTitle:@"Beta Expired" 
                                                                  message:@"This application has expired; please delete it and install the latest version." 
                                                                 delegate:self 
                                                        cancelButtonTitle:@"Exit" 
                                                        otherButtonTitles:nil];
        [expirationAlert show];
        [expirationAlert release];
    }   
#endif
}


#pragma mark UIAlertViewDelegate
- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex 
{
    Z::Engine::Shutdown();
    
    exit(0);
    
    // Forcibly crash the app if the Beta expired or it was pirated.
    [self messageDoesNotExist];
}


- (void)applicationWillResignActive:(UIApplication *)application 
{
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
    
    RETAILMSG(ZONE_INFO, "------------------------------------");
    RETAILMSG(ZONE_INFO, "--- applicationWillResignActive  ---");
    RETAILMSG(ZONE_INFO, "------------------------------------");

    GameObjects.SendMessageFromSystem( MSG_PauseGame );
    
    // Throttle frame rate to 15fps.
    [m_openglViewController.displayLink setFrameInterval:60/15];

    // Flush user preferences.
    Z::Game::SaveUserPreferences();
}



- (void)applicationDidBecomeActive:(UIApplication *)application 
{
    RETAILMSG(ZONE_INFO, "------------------------------------");
    RETAILMSG(ZONE_INFO, "---  applicationDidBecomeActive  ---");
    RETAILMSG(ZONE_INFO, "------------------------------------");
    
    // Restore frame rate.
    int frameRate = Z::GlobalSettings.GetInt("/Settings.FrameRateHZ", 60);
    [m_openglViewController.displayLink setFrameInterval:60/frameRate];
    [m_openglViewController.displayLink setPaused:NO];
}



- (void)applicationDidEnterBackground:(UIApplication *)application 
{
    RETAILMSG(ZONE_INFO, "-------------------------------------");
    RETAILMSG(ZONE_INFO, "--- applicationDidEnterBackground ---");
    RETAILMSG(ZONE_INFO, "-------------------------------------");
    
    GameTime.Pause();

    // Stop rendering.  Do this first since background render
    // can crash the app.
    [m_openglViewController.displayLink setPaused:YES];

    // HACK HACK: stop rendering.  pausing displayLink not working?!
    [m_openglViewController viewWillDisappear:NO];

    [self endAnalytics];
}


- (void)applicationWillEnterForeground:(UIApplication *)application 
{
    RETAILMSG(ZONE_INFO, "--------------------------------------");
    RETAILMSG(ZONE_INFO, "--- applicationWillEnterForeground ---");
    RETAILMSG(ZONE_INFO, "--------------------------------------");
    
    GameTime.Resume();
    [self enableAudio];
    [self startAnalytics];
   
    // HACK HACK: resume rendering
    [m_openglViewController viewWillAppear:NO];
    [m_openglViewController.displayLink setPaused:NO];
}



- (void)applicationWillTerminate:(UIApplication *)application 
{
    RETAILMSG(ZONE_INFO, "--------------------------------------");
    RETAILMSG(ZONE_INFO, "---    applicationWillTerminate    ---");
    RETAILMSG(ZONE_INFO, "--------------------------------------");

    // Flush user preferences.
    Z::Game::SaveUserPreferences();

    GameTime.Pause();

    Z::Game::Stop();
    Z::Engine::Shutdown();
    
    [homeScreenViewController       release];
    [pauseScreenViewController      release];
    [gameOverScreenViewController   release];
    [hudViewController              release];
    [confirmViewController          release];
    [aboutViewController            release];
    [tutorialViewController         release];
    [levelViewController            release];

    [m_openglViewController         release];
    [m_openglView                   release];
    [pauseScreenView                release];
    [homeScreenView                 release];
    [hudView                        release];
    [m_window                       release];
    
    
    [self endAnalytics];

    RETAILMSG(ZONE_INFO, "applicationWillTerminate: closing Log");
    Z::Log::Close();
}


- (void)startAnalytics
{
    if (!Platform::IsDebuggerAttached())
    {
        if ([[LocalyticsSession sharedLocalyticsSession] isSessionOpen])
            return;
    
        // Start analytics
        [[LocalyticsSession sharedLocalyticsSession] startSession:@""];
        
        
        NSCalendar*         calendar = [[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar];
        NSCalendarUnit      unitFlags = NSYearCalendarUnit | NSMonthCalendarUnit| NSWeekCalendarUnit | NSDayCalendarUnit | NSHourCalendarUnit | NSMinuteCalendarUnit | NSSecondCalendarUnit;
        NSDateComponents*   dateComponents = [calendar components:unitFlags fromDate:[NSDate date]];
        NSInteger           day  = [dateComponents weekday];
        NSInteger           hour = [dateComponents hour];
        [calendar release];
        
        
        NSDictionary *dictionary =
        [NSDictionary dictionaryWithObjectsAndKeys:
         [NSString stringWithCString:Platform::GetOSName()     encoding:NSUTF8StringEncoding],
         @"OS_Name",
         [NSString stringWithCString:Platform::GetOSVersion()  encoding:NSUTF8StringEncoding],
         @"OS_Version",
         [NSString stringWithCString:Platform::GetDeviceType() encoding:NSUTF8StringEncoding],
         @"Device",
         [NSNumber numberWithInteger:day],
         @"WeekDay",
         [NSNumber numberWithInteger:hour],
         @"HourOfDay",
         nil];
        
        [[LocalyticsSession sharedLocalyticsSession] tagEvent:@"Application Start" attributes:dictionary];
    }
}



- (void)endAnalytics
{
    //
    // TODO: Localytics recommends NOT logging values "from a continuous set."  
    // Their reports only look at the top 5 events and group everything else into "other".
    // So somewhat more "useful" reporting if you log "0-5 minutes" "6-10 minutes" and so on.
    //
    NSDictionary *dictionary =
    [NSDictionary dictionaryWithObjectsAndKeys:
     [NSNumber numberWithInt:Z::g_difficulty],
     @"difficulty",
     [NSNumber numberWithInt:Z::g_totalScore],
     @"score",
     [NSNumber numberWithUnsignedInt: (unsigned int)(GameTime.GetTimeDouble()/60.0)],
     @"minutes",
     [NSNumber numberWithInt:Z::g_pLevel->level],
     @"level",
     [NSNumber numberWithInt:Z::g_highestLevel],
     @"highestLevel",
     [NSNumber numberWithInt:Z::g_highestLevelEver],
     @"highestLevelEver",
     nil];
    
    NSLog(@"%@", dictionary);
    
    [[LocalyticsSession sharedLocalyticsSession] tagEvent:@"Application Exit" attributes:dictionary];
    
    
    // Flush analytics
    if (!Platform::IsDebuggerAttached())
    {
        RETAILMSG(ZONE_INFO, "Submitting analytics to server.");
        [[LocalyticsSession sharedLocalyticsSession] close];
        [[LocalyticsSession sharedLocalyticsSession] upload];
    }
}


//
// On iOS we need to change default settings to support a) background music
// from iPod or Pandora and b) resuming audio after a system interruption.
//
- (void)enableAudio
{
    RETAILMSG(ZONE_INFO, "Enabling audio.");
    
    NSError *setCategoryErr = nil;
    NSError *activationErr  = nil;
    bool rval;
    
    rval = [[AVAudioSession sharedInstance] setCategory: AVAudioSessionCategoryAmbient error: &setCategoryErr];
    if (!rval || setCategoryErr)
        RETAILMSG(ZONE_ERROR, "ERROR: failed to set AVAudioSession category to Ambient, error = %s", [[setCategoryErr domain] UTF8String]);

    rval = [[AVAudioSession sharedInstance] setActive: YES error: &activationErr];    
    if (!rval || activationErr)
        RETAILMSG(ZONE_ERROR, "ERROR: failed to activate AVAudioSession, error = %s", [[activationErr domain] UTF8String]);
}




#pragma mark -
#pragma mark Memory management

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application 
{
    /*
     Free up as much memory as possible by purging cached data objects that can be recreated (or reloaded from disk) later.
     */
    
    RETAILMSG(ZONE_ERROR, "ERROR: applicationDidReceiveMemoryWarning !!!!!!!!!!!!!!!!!");
}




- (void)dealloc 
{
    [homeScreenViewController       release];
    [pauseScreenViewController      release];
    [gameOverScreenViewController   release];
    [hudViewController              release];
    [confirmViewController          release];
    [aboutViewController            release];
    [tutorialViewController         release];
    [levelViewController            release];


    [m_openglView                   release];
    [m_openglViewController         release];
    [m_window                       release];
    
    [super dealloc];
}


@end
