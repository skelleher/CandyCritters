//
//  HUDViewController.mm
//  Critters
//
//  Created by Sean Kelleher on 4/5/11.
//  Copyright 2011 Sean Kelleher. All rights reserved.
//

#import "HUDViewController.h"


@implementation HUDViewController


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

    [m_pauseButton release];
    [m_leftButton release];
    [m_rightButton release];
    [m_rotateButton release];
    [m_dropButton release];
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
}


- (void)viewDidUnload
{
    Sounds.Release( m_hButtonClickSound );

    [m_pauseButton release];
    m_pauseButton = nil;
    [m_leftButton release];
    m_leftButton = nil;
    [m_rightButton release];
    m_rightButton = nil;
    [m_rotateButton release];
    m_rotateButton = nil;
    [m_dropButton release];
    m_dropButton = nil;
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

- (IBAction)onPauseTouchDown:(id)sender 
{
    Sounds.Play( m_hButtonClickSound );
}


- (IBAction)onPauseTouchUp:(id)sender 
{
    GameObjects.SendMessageFromSystem( MSG_PauseGame );
}


- (IBAction)onLeftTouchDown:(id)sender 
{
}


- (IBAction)onLeftTouchUp:(id)sender 
{
    GameObjects.SendMessageFromSystem( MSG_ColumnLeft );
}


- (IBAction)onRightTouchDown:(id)sender 
{
}


- (IBAction)onRightTouchUp:(id)sender 
{
    GameObjects.SendMessageFromSystem( MSG_ColumnRight );
}


- (IBAction)onRotateTouchDown:(id)sender 
{
}


- (IBAction)onRotateTouchUp:(id)sender 
{
    GameObjects.SendMessageFromSystem( MSG_ColumnRotate );
}


- (IBAction)onDropTouchDown:(id)sender 
{
}


- (IBAction)onDropTouchUp:(id)sender 
{
    GameObjects.SendMessageFromSystem( MSG_ColumnDropToBottom );
}

@end
