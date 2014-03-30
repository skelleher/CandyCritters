//
//  LevelViewController
//  Critters
//
//  Created by Sean Kelleher on 11/19/12.
//  Copyright 2011 Sean Kelleher. All rights reserved.
//

#import "LevelViewController.h"
#import "GameState.hpp"


const int NUMBER_OF_DIFFICULTY_INCREMENTS = 3;

static NSMutableArray *s_buttons;


@implementation LevelViewController

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

    // Load custom font and apply to our Labels
    UIFont* font = [UIFont fontWithName:@"BanzaiBros" size:52];

//    m_difficultyValue.font              = font;
//    m_difficultyValue.shadowColor       = [UIColor.darkGrayColor colorWithAlphaComponent:0.90f];
//    m_difficultyValue.shadowOffset      = CGSizeMake(5, 7);
//    m_difficultyValue.outlineThickness  = 3;
//    m_difficultyValue.outlineColor      = UIColor.blackColor;

//    [self setDifficulty:g_difficulty ];

    Sounds.GetCopy( "Drip", &m_hButtonClickSound );
    
    
    //
    // Draw the level select buttons.
    // HACK: hard-code the positions rather than dynamic fitting.
    //
    float xStart  = 18;
    float yStart  = 122;
    float xOffset = 74;
    float yOffset = 74;

    CGRect frame  = CGRectMake(xStart, yStart, 64, 64);

    //UserSettings.GetInt( "/UserPrefs.HighestLevel", g_highestLevelEver );
    
    s_buttons = [[NSMutableArray alloc] init];
    for (int i = 1; i < Z::g_numLevels+1; ++i)
    {
        // Create a button.
        UIButton* button = [UIButton buttonWithType:UIButtonTypeCustom]; //autorelease
        [s_buttons addObject:button];


        // Configure the button.
        // TODO: on iOS 6.0 and greater we can use NSAttributedString.
//        [button setTitle:     [NSString stringWithFormat:@"%d", i] forState:UIControlStateNormal];
//        NSAttributedString* title = [[NSAttributedString alloc] initWithAttributedString:@""];
//        [button setTitle:title forState:UIControlStateNormal];
//        [button setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];

        CustomUILabel* title = [[CustomUILabel alloc] init];
        title.text = [NSString stringWithFormat:@"%d", i];
        title.font = font;
        title.backgroundColor = UIColor.clearColor;
        title.textColor = UIColor.whiteColor;
        title.outlineColor = UIColor.blackColor;
        title.outlineThickness = 2.0;
        [title sizeToFit];
        CGRect labelFrame = title.frame;
        labelFrame.size.width  += title.outlineThickness;
        labelFrame.size.height += title.outlineThickness;
        labelFrame.origin.x += (frame.size.width  - labelFrame.size.width)/2;
        labelFrame.origin.y += (frame.size.height - labelFrame.size.height)/2 + 2;
        title.frame = labelFrame;
        [button addSubview:title];

        [button setBackgroundImage:[UIImage imageNamed:@"levelSelect.png"]      forState:UIControlStateNormal];
        [button setBackgroundImage:[UIImage imageNamed:@"levelSelectDown.png"]  forState:UIControlStateHighlighted];
        button.frame = frame;
        //button.titleLabel.font = font;
        button.contentVerticalAlignment   = UIControlContentVerticalAlignmentCenter;
        button.contentHorizontalAlignment = UIControlContentHorizontalAlignmentCenter;

        // Disable the button if player hasn't reached that level yet.
        if (i > Z::g_highestLevelEver)
        {
            // TODO: overlay a locked icon.
            // TODO: change background image to be "greyed out"
            button.enabled = NO;
            title.textColor = UIColor.lightGrayColor;
            [button setBackgroundImage:[UIImage imageNamed:@"levelSelectDisabled.png"] forState:UIControlStateNormal];
        }

        // Bind it to an event handler
        [button addTarget:self
                   action:@selector(onLevelButtonTouchUp:)
         forControlEvents:UIControlEventTouchUpInside];
 
 
        [self.view addSubview: button];
        
        // Wrap buttons to next row as needed.
        frame.origin.x += xOffset;
        if (frame.origin.x + frame.size.width >= self.view.frame.size.width)
        {
            frame.origin.x = xStart;
            frame.origin.y += yOffset;
        }
    }
}


- (void)viewWillAppear:(BOOL)animated
{
    // User may have leveled up since we were last display; unlock the corresponding level select buttons.
    
    for (int i = 0; i < s_buttons.count; ++i)
    {
        UIButton* button = s_buttons[i];
        CustomUILabel* label = button.subviews[1];
        int level = atoi( [label.text UTF8String] );
    
        if ( level <= Z::g_highestLevelEver )
        {
            button.enabled = YES;
            CustomUILabel* title = button.subviews[1];
            title.textColor = UIColor.whiteColor;
            [button setBackgroundImage:[UIImage imageNamed:@"levelSelect.png"] forState:UIControlStateNormal];
            // TODO: set color, remove lock overlay, etc.
        }
    }
}


- (void)viewDidUnload
{
    Sounds.Release( m_hButtonClickSound );

    for (int i = 0; i < s_buttons.count; ++i)
    {
        UIButton* button = s_buttons[i];
        [button release];
    }
    
    
    [super viewDidUnload];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}


#pragma mark - Touch Handlers


- (IBAction)onLevelButtonTouchUp:(id)sender
{
    UIButton* button = (UIButton*)sender;
    UILabel*  label  = button.subviews[1];

    int level = atoi( [label.text UTF8String] );
    Z::Log::Print(Z::ZONE_INFO, "Player selected level %d\n", level);
    
    Z::g_level = level-1;
    Z::g_pLevel = &Z::g_levels[ Z::g_level ];

    Sounds.Play( m_hButtonClickSound );
    GameObjects.SendMessageFromSystem( MSG_NewGame );
}


//- (IBAction)onContinueButtonTouchDown:(id)sender
//{
//}
//
//
//- (IBAction)onContinueButtonTouchUp:(id)sender
//{
//    Sounds.Play( m_hButtonClickSound );
//    GameObjects.SendMessageFromSystem( MSG_NewGame );
//}
//
//
//- (IBAction)onLevelSliderValueChanged:(id)sender
//{
//    UISlider* slider = (UISlider*)sender;
//    g_difficulty = (UINT32)(slider.value * (float)NUMBER_OF_DIFFICULTY_INCREMENTS+1);
//
//    [self setDifficulty:g_difficulty ];
//}
//
//
//- (IBAction)onLevelSliderDoneChanging:(id)sender
//{
//    Sounds.Play( m_hButtonClickSound );
//    Game::SaveUserPreferences();
//    
//    RETAILMSG(ZONE_INFO, "SET DIFFICULTY: %d", g_difficulty);
//}


//- (void)setDifficulty:(int)difficulty
//{
//    difficulty = MAX(1, difficulty);
//    difficulty = MIN(difficulty, NUMBER_OF_DIFFICULTY_INCREMENTS+1);
//
//    [m_difficultySlider setValue:(float)((difficulty-1)/(float)NUMBER_OF_DIFFICULTY_INCREMENTS) animated:NO];
//
//    switch (difficulty)
//    {
//        case 1:
//            [m_difficultyValue setText:@"Easy"];
//            g_difficultyMultiplier =  0.75f;
//            break;
//        case 2:
//            [m_difficultyValue setText:@"Medium"];
//            g_difficultyMultiplier = 1.0f;
//            break;
//        case 3:
//            [m_difficultyValue setText:@"Hard"];
//            g_difficultyMultiplier = 1.25f;
//            break;
//        case 4:
//            [m_difficultyValue setText:@"No Picnic!"];
//            g_difficultyMultiplier = 1.5f;
//            break;
//        default:
//            [m_difficultyValue setText:@"Medium"];
//            g_difficultyMultiplier = 1.0f;
//            break;
//    }
//}

@end
