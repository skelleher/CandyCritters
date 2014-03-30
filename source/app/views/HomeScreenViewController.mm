//
//  HomeScreenViewController.m
//  Critters
//
//  Created by Sean Kelleher on 3/28/11.
//  Copyright 2011 Sean Kelleher. All rights reserved.
//

#import "HomeScreenViewController.h"
#import "Util.hpp"
#import "Macros.hpp"


@interface HomeScreenViewController ()
{
    UIImageView* critter;
}
@end



@implementation HomeScreenViewController


namespace Z
{
    extern bool g_showTutorial;
}



// The designated initializer.  Override if you create the controller programmatically and want to perform customization that is not appropriate for viewDidLoad.
/*
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization.
    }
    return self;
}
*/


// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad 
{
    [super viewDidLoad];
    
    Sounds.GetCopy( "Drip", &m_hNewGameSound     );
    Sounds.GetCopy( "Drip", &m_hButtonTapSound   );

    [playButton     setBackgroundImage:[UIImage imageNamed:@"mainMenuPlay.png"]         forState:UIControlStateNormal];
    [playButton     setBackgroundImage:[UIImage imageNamed:@"mainMenuPlayDown.png"]     forState:UIControlStateHighlighted];
    [aboutButton    setBackgroundImage:[UIImage imageNamed:@"mainMenuCredits.png"]      forState:UIControlStateNormal];
    [aboutButton    setBackgroundImage:[UIImage imageNamed:@"mainMenuCreditsDown.png"]  forState:UIControlStateHighlighted];
    [tutorialButton setBackgroundImage:[UIImage imageNamed:@"mainMenuTutorial.png"]     forState:UIControlStateNormal];
    [tutorialButton setBackgroundImage:[UIImage imageNamed:@"mainMenuTutorialDown.png"] forState:UIControlStateHighlighted];

    // Create an animating Critter head
    
    UIImage* image = [UIImage imageNamed:@"RedCritter.png"];
    critter = [[UIImageView alloc] initWithImage:image];
    float x = (self.view.frame.size.width - critter.frame.size.width*0.75)/2;
    critter.frame = CGRectMake(x, 20, critter.frame.size.width*0.75, critter.frame.size.height*0.75);
    [self.view addSubview:critter];
    
//    critter.layer.transform     = CATransform3DScale(critter.layer.transform, 0.75, 0.75, 1.0);

//    UIBezierPath *path = [UIBezierPath bezierPath];
//    [path moveToPoint:CGPointMake(100.0, 100.0)];
//    [path addLineToPoint:CGPointMake(200.0, 200.0)];
//    [path addLineToPoint:CGPointMake(100.0, 300.0)];
//
//    CAKeyframeAnimation *animatePosition = [CAKeyframeAnimation animationWithKeyPath:@"position"];
//    animatePosition.path = [path CGPath];
//    animatePosition.duration = 1.0;
//    animatePosition.autoreverses = YES;
//    animatePosition.repeatCount = CGFLOAT_MAX;
//    [critter.layer addAnimation:animatePosition forKey:@"position"];

    // Animate the scale.
    CAKeyframeAnimation *pulseAnimation = [CAKeyframeAnimation animationWithKeyPath:@"transform"];

    CATransform3D startingScale   = CATransform3DScale (CATransform3DRotate(critter.layer.transform, RADIANS(0),   0.0, 0.0, 1.0), 1.0, 1.0, 1.0);
    CATransform3D overshootScale  = CATransform3DScale (CATransform3DRotate(critter.layer.transform, RADIANS(10),  0.0, 0.0, 1.0), 1.1, 1.1, 1.0);
    CATransform3D undershootScale = CATransform3DScale (CATransform3DRotate(critter.layer.transform, RADIANS(-10), 0.0, 0.0, 1.0), 0.8, 0.8, 1.0);
//    CATransform3D endingScale     = critter.layer.transform;
    CATransform3D endingScale     = CATransform3DScale (CATransform3DRotate(critter.layer.transform, RADIANS(5),   0.0, 0.0, 1.0), 1.0, 1.0, 1.0);

    NSArray *keyframes          = [NSArray arrayWithObjects:
                                  [NSValue valueWithCATransform3D:startingScale],
                                  [NSValue valueWithCATransform3D:overshootScale],
                                  [NSValue valueWithCATransform3D:undershootScale],
                                  [NSValue valueWithCATransform3D:endingScale],
                                  nil];

    NSArray *times              = [NSArray arrayWithObjects:
                                  [NSNumber numberWithFloat:0.0f],
                                  [NSNumber numberWithFloat:0.5f],
                                  [NSNumber numberWithFloat:0.75f],
                                  [NSNumber numberWithFloat:1.0f],
                                  nil];


    NSArray *timingFunctions    = [NSArray arrayWithObjects:
                                  [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut],
                                  [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut],
                                  [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut],
                                  nil];

    [pulseAnimation setValues:keyframes];
    [pulseAnimation setKeyTimes:times];
    [pulseAnimation setTimingFunctions:timingFunctions];
    
    pulseAnimation.fillMode = kCAFillModeForwards;
    pulseAnimation.removedOnCompletion = NO;
    pulseAnimation.autoreverses = YES;
    pulseAnimation.duration = 10.0;
    pulseAnimation.repeatCount = CGFLOAT_MAX;

    [critter.layer addAnimation:pulseAnimation forKey:@"transform"];
}


- (void)viewWillAppear:(BOOL)animated
{
    // TODO: start fade in storyboard
}



- (void)viewDidAppear:(BOOL)animated
{

}



/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations.
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/

- (void)didReceiveMemoryWarning 
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc. that aren't in use.
}

- (void)viewDidUnload 
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}


- (void)dealloc 
{
    Sounds.Release( m_hNewGameSound );

    [super dealloc];
}


#pragma mark -
#pragma mark Button Handlers


- (IBAction) onPlayButtonTapped
{
    Sounds.Play( m_hNewGameSound );
    
#ifdef SHIPBUILD
    Z::g_showTutorial = Z::Settings::User().GetBool("/UserPrefs.bShowTutorial", true);
    if (Z::g_showTutorial)
    {
        GameObjects.SendMessageFromSystem( MSG_GameScreenTutorial );

        // Prevent tutorial from running next time.
        Z::g_showTutorial = false;
        Z::Settings::User().Write();
    }
    else
#endif
    {
        GameObjects.SendMessageFromSystem( MSG_GameScreenLevel );
    }
}



- (IBAction) onAboutButtonTapped
{
    Sounds.Play( m_hButtonTapSound );
    GameObjects.SendMessageFromSystem( MSG_GameScreenCredits );
}

- (IBAction) onScoresButtonTapped 
{
    Sounds.Play( m_hButtonTapSound );
#ifdef USE_OPENFEINT
    Z::OpenFeint.ShowDashboardHighScores();
#endif
}

- (IBAction) onAwardsButtonTapped
{
    Sounds.Play( m_hButtonTapSound );
#ifdef USE_OPENFEINT
    Z::OpenFeint.ShowDashboardAchievements();
#endif
}


- (IBAction) onTutorialButtonTapped
{
    Sounds.Play( m_hNewGameSound );
    GameObjects.SendMessageFromSystem( MSG_GameScreenTutorial );
}


- (IBAction)onMailButtonTapped
{
    if ([MFMailComposeViewController canSendMail]) 
    {
        MFMailComposeViewController *picker = [[MFMailComposeViewController alloc] init];
        picker.mailComposeDelegate = self;
        
        [picker setSubject:@"_COLUMNS_ Log File"];
        
        // Set up the recipients.
        NSArray *toRecipients = [NSArray arrayWithObjects:@"sean@seankelleher.org", nil];
        [picker setToRecipients:toRecipients];
        
        // Attach the log file.
        const string filename = Z::Log::GetFilename();
        NSData *myData = [NSData dataWithContentsOfFile:[NSString stringWithCString:filename.c_str() encoding:NSUTF8StringEncoding]];
        [picker addAttachmentData:myData mimeType:@"text/plain" fileName:@"logfile.txt"];

        // Present the mail composition interface.
        [self presentModalViewController:picker animated:YES];

        [picker release]; // Can safely release the controller now.
        
    }
    else
    {
        // TODO: show an alert.
        DEBUGCHK(0);
    }
}


// The mail compose view controller delegate method
- (void)mailComposeController:(MFMailComposeViewController *)controller
          didFinishWithResult:(MFMailComposeResult)result
                        error:(NSError *)error
{
    [controller dismissModalViewControllerAnimated:NO];
}




@end
