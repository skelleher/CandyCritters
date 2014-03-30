//
//  GameOverScreenViewController.mm
//  Critters
//
//  Created by Sean Kelleher on 4/5/11.
//  Copyright 2011 Sean Kelleher. All rights reserved.
//

#import "GameOverScreenViewController.h"


namespace Z
{
    extern UINT32 g_totalScore;
    extern UINT32 g_highScore;
    extern UINT32 g_gameNumBlocksCleared;
    extern UINT32 g_gameNumLinesCleared;
    extern UINT32 g_gameNumChains;
    
    extern UINT32 g_highestLevel;
    extern bool   g_newHighScore;
}



@implementation GameOverScreenViewController

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
    [tryAgainButton release];
    [mainMenuButton release];
    [scoreValueLabel release];
    [highScoreValueLabel release];
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
    
    // Load custom font and apply to our Labels
    UIFont* fontLabel = [UIFont fontWithName:@"BanzaiBros" size:24];
    UIFont* fontValue = [UIFont fontWithName:@"BanzaiBros" size:42];
    
    NSInteger   outlineThickness = 2.0;
    UIColor*    outlineColor     = UIColor.blackColor;
    UIColor*    textColor        = UIColor.whiteColor;
    UIColor*    shadowColor      = [UIColor.darkGrayColor colorWithAlphaComponent:0.90f];
//    CGSize      shadowOffset     = CGSizeMake(3, 4);
    CGSize      shadowOffset     = CGSizeMake(1, 1);
    
    scoreLabel.font                      = fontLabel;
    scoreLabel.shadowColor               = shadowColor;
    scoreLabel.shadowOffset              = shadowOffset;
    scoreLabel.outlineThickness          = outlineThickness;
    scoreLabel.outlineColor              = outlineColor;
    scoreLabel.textColor                 = textColor;

    highScoreLabel.font                  = fontLabel;
    highScoreLabel.shadowColor           = shadowColor;
    highScoreLabel.shadowOffset          = shadowOffset;
    highScoreLabel.outlineThickness      = outlineThickness;
    highScoreLabel.outlineColor          = outlineColor;
    highScoreLabel.textColor             = textColor;

    scoreValueLabel.font                 = fontValue;
    scoreValueLabel.shadowColor          = shadowColor;
    scoreValueLabel.shadowOffset         = shadowOffset;
    scoreValueLabel.outlineThickness     = outlineThickness;
    scoreValueLabel.outlineColor         = outlineColor;
    scoreValueLabel.textColor            = textColor;

    highScoreValueLabel.font             = fontValue;
    highScoreValueLabel.shadowColor      = shadowColor;
    highScoreValueLabel.shadowOffset     = shadowOffset;
    highScoreValueLabel.outlineThickness = outlineThickness;
    highScoreValueLabel.outlineColor     = outlineColor;
    highScoreValueLabel.textColor        = textColor;
    
    newHighScoreLabel.font               = fontLabel;
    newHighScoreLabel.shadowColor        = shadowColor;
    newHighScoreLabel.shadowOffset       = shadowOffset;
    newHighScoreLabel.outlineThickness   = outlineThickness;
    newHighScoreLabel.outlineColor       = outlineColor;
    newHighScoreLabel.textColor          = UIColor.cyanColor;
}


- (void)viewDidUnload
{
    Sounds.Release( m_hButtonClickSound );

    [tryAgainButton release];
    tryAgainButton = nil;
    [mainMenuButton release];
    mainMenuButton = nil;
    [scoreValueLabel release];
    scoreValueLabel = nil;
    [highScoreValueLabel release];
    highScoreValueLabel = nil;
    [newHighScoreLabel release];
    newHighScoreLabel = nil;
    
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}


- (void)viewWillAppear:(BOOL)animated
{
    highScoreValueLabel.text = [NSString stringWithFormat:@"%lu",  Z::g_highScore];
    scoreValueLabel.text     = [NSString stringWithFormat:@"%lu",  Z::g_totalScore];


    CGRect labelFrame = highScoreValueLabel.frame;
    labelFrame.size.width  += highScoreValueLabel.outlineThickness * 3;
    labelFrame.size.height += highScoreValueLabel.outlineThickness * 3;
    labelFrame.origin.x    -= highScoreValueLabel.outlineThickness * 3;
    highScoreValueLabel.frame = labelFrame;


    labelFrame = scoreValueLabel.frame;
    labelFrame.size.width  += scoreValueLabel.outlineThickness * 3;
    labelFrame.size.height += scoreValueLabel.outlineThickness * 3;
    labelFrame.origin.x    -= highScoreValueLabel.outlineThickness * 3;
    scoreValueLabel.frame = labelFrame;

    if (Z::g_newHighScore)
    {
//        newHighScoreLabel.hidden = false;
        GameObjects.SendMessageFromSystem( MSG_Fireworks );
    }

}


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}



#pragma mark - Touch Handlers

- (IBAction)tryAgainButtonTouchUp:(id)sender 
{
    Sounds.Play( m_hButtonClickSound );
    GameObjects.SendMessageFromSystem( MSG_NewGame );
}


- (IBAction)mainMenuButtonTouchUp:(id)sender 
{
    Sounds.Play( m_hButtonClickSound );
    GameObjects.SendMessageFromSystem( MSG_GameScreenHome );
}


@end
