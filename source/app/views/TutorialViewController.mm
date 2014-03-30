//
//  TutorialViewController.mm
//  Critters
//
//  Created by Sean Kelleher on 8/29/11.
//  Copyright 2011 Sean Kelleher. All rights reserved.
//

#import "TutorialViewController.h"


@implementation TutorialViewController


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
    Sounds.Release( m_hButtonClickSound );

    [m_doneButton release];
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

    [m_doneButton setBackgroundImage:[UIImage imageNamed:@"tutorialOK.png"] forState:UIControlStateNormal];
    [m_doneButton setBackgroundImage:[UIImage imageNamed:@"tutorialOKDown.png"] forState:UIControlStateHighlighted];
}


- (void)viewDidUnload
{
    Sounds.Release( m_hButtonClickSound );

    [m_doneButton release];
    m_doneButton = nil;

    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
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

- (IBAction)onDoneTouchDown:(id)sender 
{
}


- (IBAction)onDoneTouchUp:(id)sender 
{
    GameObjects.SendMessageFromSystem( MSG_GameScreenLevel );
}

@end
