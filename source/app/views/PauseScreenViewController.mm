//
//  PauseScreenViewController.mm
//  Critters
//
//  Created by Sean Kelleher on 4/5/11.
//  Copyright 2011 Sean Kelleher. All rights reserved.
//

#import "PauseScreenViewController.h"
#import "Engine.hpp"

// TEST TEST:
#import <MessageUI/MFMailComposeViewController.h>


@implementation PauseScreenViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)dealloc
{
    NSLog(@"%s", __PRETTY_FUNCTION__);

    [resumeButton release];
    [quitButton release];
//    [mailLogButton release];
    [soundSlider release];
    [super dealloc];
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

- (void)viewDidLoad
{
    [super viewDidLoad];

    Sounds.GetCopy( "Drip", &m_hButtonClickSound );
    Sounds.GetCopy( "Drip", &m_hFXSound          );
    
    // TODO: subclass UISlider and implement - (CGRect)thumbRectForBounds:(CGRect)bounds trackRect:(CGRect)rect value:(float)value;
    // So that thumb slider hit box matches the thumb slider image.
    [soundSlider setThumbImage:[UIImage imageNamed:@"volumeSlider.png"] forState:UIControlStateNormal];
    [soundSlider setThumbImage:[UIImage imageNamed:@"volumeSliderDown.png"] forState:UIControlStateHighlighted];

    [quitButton setBackgroundImage:[UIImage imageNamed:@"pausedQuit.png"] forState:UIControlStateNormal];
    [quitButton setBackgroundImage:[UIImage imageNamed:@"pausedQuitDown.png"] forState:UIControlStateHighlighted];

    [resumeButton setBackgroundImage:[UIImage imageNamed:@"pausedPlay.png"] forState:UIControlStateNormal];
    [resumeButton setBackgroundImage:[UIImage imageNamed:@"pausedPlayDown.png"] forState:UIControlStateHighlighted];

//#ifdef SHIPBUILD
//    [mailLogButton setHidden:YES];
//#endif    
}

- (void)viewDidUnload
{
    Sounds.Release( m_hButtonClickSound );
    Sounds.Release( m_hFXSound          );

    [resumeButton release];
    resumeButton = nil;
    [quitButton release];
    quitButton = nil;
//    [mailLogButton release];
//    mailLogButton = nil;
    [soundSlider release];
    soundSlider = nil;
    [super viewDidUnload];
}


- (void)viewWillAppear:(BOOL)animated
{
    float fxVolume      = Sounds.GetFXVolume();

    soundSlider.value   = fxVolume;
}


- (void)viewWillDisappear:(BOOL)animated
{
    // Flush user preferences.
    Z::Game::SaveUserPreferences();
}


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}


- (IBAction)onResumeButtonTouchUp:(id)sender 
{
    GameObjects.SendMessageFromSystem( MSG_UnpauseGame );
}


- (IBAction)onSoundVolumeChange:(id)sender 
{
    UISlider* slider = (UISlider*)sender;
    
    float volume = [slider value];

    Sounds.SetFXVolume( volume );
}


- (IBAction)onSoundVolumeDone:(id)sender 
{
    Sounds.Play( m_hFXSound );
}



- (IBAction)onResumeButtonTouchDown:(id)sender 
{
    // TODO: Scale button up, play a sound.
    Sounds.Play( m_hButtonClickSound );
}

- (IBAction)onQuitButtonTouchDown:(id)sender 
{
    // TODO: Scale button up, play a sound.
    Sounds.Play( m_hButtonClickSound );
}

- (IBAction)onQuitButtonTouchUp:(id)sender 
{
    GameObjects.SendMessageFromSystem( MSG_GameScreenConfirmQuit );
}


/**
- (IBAction)onMailButtonTouchUp:(id)sender 
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
**/


@end
