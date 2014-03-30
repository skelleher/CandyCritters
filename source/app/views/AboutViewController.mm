//
//  AboutViewController.mm
//  Critters
//
//  Created by Sean Kelleher on 8/22/11.
//  Copyright 2011 Sean Kelleher. All rights reserved.
//

#import "AboutViewController.h"


@implementation AboutViewController


- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self)
    {
    }
    return self;
}


- (void)dealloc
{
    Sounds.Release( m_hButtonClickSound );

    [m_backButton           release];
    [m_programmerLabel      release];
    [m_programmerValueLabel release];
    [m_artistLabel          release];
    [m_artistValueLabel     release];
    [m_thanksValueLabel     release];
    [m_buildInfoLabel       release];

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
/*
    // Load custom font and apply to our Labels
    UIFont* fontHeading             = [UIFont fontWithName:@"BanzaiBros" size:17];
    UIFont* font                    = [UIFont fontWithName:@"BanzaiBros" size:20];
    
    m_programmerLabel.font          = fontHeading;   
    m_programmerValueLabel.font     = font;
    
    m_artistLabel.font              = fontHeading;
    m_artistValueLabel.font         = font;
    
    m_thanksLabel.font              = fontHeading;
    m_thanksValueLabel.font         = fontHeading;

    m_programmerLabel.outlineThickness      = 1;
    //m_programmerLabel.outlineColor          = UIColor.blackColor;

    m_programmerValueLabel.outlineThickness = 1;
    //m_programmerValueLabel.outlineColor     = UIColor.blackColor;

    m_artistLabel.outlineThickness          = 1;
    //m_artistLabel.outlineColor              = UIColor.blackColor;

    m_artistValueLabel.outlineThickness     = 1;
    //m_artistValueLabel.outlineColor         = UIColor.blackColor;

    m_thanksLabel.outlineThickness          = 1;
    //m_thanksLabel.outlineColor              = UIColor.blackColor;

    m_thanksValueLabel.outlineThickness     = 1;
    //m_thanksValueLabel.outlineColor         = UIColor.blackColor;
*/
    
    [m_backButton setBackgroundImage:[UIImage imageNamed:@"tutorialOK.png"] forState:UIControlStateNormal];
    [m_backButton setBackgroundImage:[UIImage imageNamed:@"tutorialOKDown.png"] forState:UIControlStateHighlighted];
    
    // Display build number
    m_buildInfoLabel.text           = [NSString stringWithCString:Z::Platform::GetBuildInfo() encoding:NSUTF8StringEncoding];
}


- (void)viewDidUnload
{
    Sounds.Release( m_hButtonClickSound );

    [m_backButton           release];
    [m_programmerLabel      release];
    [m_programmerValueLabel release];
    [m_artistLabel          release];
    [m_artistValueLabel     release];
    [m_thanksLabel          release];
    [m_thanksValueLabel     release];
    [m_buildInfoLabel       release];

    [super viewDidUnload];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}


- (void)viewWillAppear:(BOOL)animated
{
}



#pragma mark - Touch Handlers

- (IBAction)onBackButtonTouchDown:(id)sender 
{
}


- (IBAction)onBackButtonTouchUp:(id)sender 
{
    Sounds.Play( m_hButtonClickSound );
    GameObjects.SendMessageFromSystem( MSG_GameScreenHome );
}

@end
