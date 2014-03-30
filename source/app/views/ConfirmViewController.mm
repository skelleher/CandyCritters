//
//  ConfirmViewController.mm
//  Critters
//
//  Created by Sean Kelleher on 5/14/11.
//  Copyright 2011 Sean Kelleher. All rights reserved.
//

#import "ConfirmViewController.h"


@implementation ConfirmViewController

//NSString* m_text;


- (void)setMessage:(NSString *)text
{
    [m_text release];
    m_text = text;
    [m_text retain];
    
    m_label.text = m_text;
}


- (NSString*)message
{
    return m_text;
}


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

    m_maxLabelHeight = m_label.frame.size.height;

    UIFont* font                = [UIFont fontWithName:@"BanzaiBros" size:46];
    UIFont* font64              = [UIFont fontWithName:@"BanzaiBros" size:64];

    NSInteger   outlineThickness = 3;
    UIColor*    outlineColor     = UIColor.blackColor;
    UIColor*    textColor        = [UIColor colorWithRed:0.99 green:0.96 blue:0.0 alpha:1.0];
    UIColor*    shadowColor      = [UIColor.darkGrayColor colorWithAlphaComponent:0.90f];
    CGSize      shadowOffset     = CGSizeMake(3, 4);

    m_label.font             = font64;
    m_label.lineBreakMode    = UILineBreakModeWordWrap;
    m_label.numberOfLines    = 0;
    m_label.textAlignment    = UITextAlignmentCenter;
    m_label.textColor        = [UIColor yellowColor];

    m_label.shadowColor      = shadowColor;
    m_label.shadowOffset     = shadowOffset;
    m_label.outlineThickness = outlineThickness;
    m_label.outlineColor     = outlineColor;
    m_label.textColor        = textColor;


    CustomUILabel* noLabel = [[CustomUILabel alloc] init];
    noLabel.text = @"NO";
    noLabel.font = font;
    noLabel.backgroundColor = UIColor.clearColor;
    noLabel.textColor = UIColor.whiteColor;
    noLabel.outlineColor = UIColor.blackColor;
    noLabel.outlineThickness = 3.0;
    [noLabel sizeToFit];
    CGRect labelFrame = noLabel.frame;
    labelFrame.size.width  += noLabel.outlineThickness * 3;
    labelFrame.size.height += noLabel.outlineThickness * 3;
    labelFrame.origin.x += (noButton.frame.size.width  - labelFrame.size.width)/2  + 2;
    labelFrame.origin.y += (noButton.frame.size.height - labelFrame.size.height)/2;
    noLabel.frame = labelFrame;
    [noButton addSubview:noLabel];


    CustomUILabel* yesLabel = [[CustomUILabel alloc] init];
    yesLabel.text = @"YES";
    yesLabel.font = font;
    yesLabel.backgroundColor = UIColor.clearColor;
    yesLabel.textColor = UIColor.whiteColor;
    yesLabel.outlineColor = UIColor.blackColor;
    yesLabel.outlineThickness = 3.0;
    [yesLabel sizeToFit];
    labelFrame = yesLabel.frame;
    labelFrame.size.width  += yesLabel.outlineThickness * 3;
    labelFrame.size.height += yesLabel.outlineThickness * 3;
    labelFrame.origin.x += (noButton.frame.size.width  - labelFrame.size.width)/2  + 2;
    labelFrame.origin.y += (noButton.frame.size.height - labelFrame.size.height)/2;
    yesLabel.frame = labelFrame;
    [yesButton addSubview:yesLabel];
}


- (void)viewDidUnload
{
    [super viewDidUnload];

    Sounds.Release( m_hButtonClickSound );
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}


- (void)viewWillAppear:(BOOL)animated
{
    m_label.text = m_text;

    CGSize labelSize = [m_label.text sizeWithFont:m_label.font
                                constrainedToSize:m_label.frame.size
                                    lineBreakMode:m_label.lineBreakMode];

    CGFloat y = (m_maxLabelHeight - labelSize.height) / 2;

    m_label.frame = CGRectMake(
        m_label.frame.origin.x, y,
        m_label.frame.size.width, labelSize.height);
}


- (IBAction)onYesButtonTouchUp:(id)sender 
{
    Sounds.Play( m_hButtonClickSound );

//    GameObjects.SendMessageFromSystem( MSG_GameOver       );
    GameObjects.SendMessageFromSystem( MSG_GameScreenHome );
}


- (IBAction)onNoButtonTouchUp:(id)sender 
{
    Sounds.Play( m_hButtonClickSound );

    GameObjects.SendMessageFromSystem( MSG_GameScreenPrevious );
}

@end
