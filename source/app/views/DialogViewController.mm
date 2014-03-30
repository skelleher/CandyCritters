//
//  DialogViewController.mm
//  Critters
//
//  Created by Sean Kelleher on 5/27/13.
//  Copyright 2013 Sean Kelleher. All rights reserved.
//

#import "DialogViewController.h"
#import "SceneManager.hpp"
#import "StoryboardManager.hpp"
#import "EffectManager.hpp"


@interface DialogViewController ()
@property (nonatomic, assign) Z::HScene      hDialogScene;
@property (nonatomic, assign) Z::HStoryboard hShowDialogStoryboard;
@property (nonatomic, assign) Z::HStoryboard hHideDialogStoryboard;
@property (nonatomic, assign) Z::HEffect     hDialogEffect;
//@property (nonatomic, assign) Z::Callback    didAppearCallback;
@end


@implementation DialogViewController

@synthesize cancelButton  = cancelButton;
@synthesize confirmButton = confirmButton;
@synthesize hShowDialogStoryboard;
@synthesize hHideDialogStoryboard;
@synthesize hDialogEffect;
//@synthesize didAppearCallback;


//void didAppear( void* pContext )
//{
//    Z::Engine::Pause();
//}



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
//        Z::Callback callback( didAppear, NULL );
//        self.didAppearCallback = callback;
    }
    return self;
}

- (void)dealloc
{
    [super dealloc];
    
    self.hDialogScene.Release();
    self.hDialogEffect.Release();
    self.hShowDialogStoryboard.Release();
    self.hHideDialogStoryboard.Release();
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

    UIFont*  buttonFont       = [UIFont fontWithName:@"BanzaiBros" size:46];
    UIFont*  textFont         = [UIFont fontWithName:@"BanzaiBros" size:32];

    float    outlineThickness = 2.5;
    UIColor* outlineColor     = UIColor.blackColor;
    UIColor* textColor        = [UIColor colorWithRed:1.0 green:0.875 blue:0.0 alpha:1.0];
    UIColor* shadowColor      = [UIColor.darkGrayColor colorWithAlphaComponent:0.90f];
    CGSize   shadowOffset     = CGSizeMake(0, 0);

    m_label.font              = textFont;
    m_label.lineBreakMode     = UILineBreakModeWordWrap;
    m_label.numberOfLines     = 0;
    m_label.textAlignment     = UITextAlignmentCenter;
    m_label.textColor         = [UIColor yellowColor];

    m_label.shadowColor       = shadowColor;
    m_label.shadowOffset      = shadowOffset;
    m_label.outlineThickness  = outlineThickness;
    m_label.outlineColor      = outlineColor;
    m_label.textColor         = textColor;


    CustomUILabel* cancelLabel = [[CustomUILabel alloc] init];
    cancelLabel.text = @"";
    cancelLabel.font = buttonFont;
    cancelLabel.backgroundColor = UIColor.clearColor;
    cancelLabel.textColor = UIColor.whiteColor;
    cancelLabel.outlineColor = UIColor.blackColor;
    cancelLabel.outlineThickness = 3.0;
    [cancelLabel sizeToFit];
    CGRect labelFrame = cancelLabel.frame;
    labelFrame.size.width  += cancelLabel.outlineThickness * 3;
    labelFrame.size.height += cancelLabel.outlineThickness * 3;
    labelFrame.origin.x += (cancelButton.frame.size.width  - labelFrame.size.width)/2  + 2;
    labelFrame.origin.y += (cancelButton.frame.size.height - labelFrame.size.height)/2;
    cancelLabel.frame = labelFrame;
    [cancelButton addSubview:cancelLabel];


    CustomUILabel* confirmLabel = [[CustomUILabel alloc] init];
    confirmLabel.text = @"OK";
    confirmLabel.font = buttonFont;
    confirmLabel.backgroundColor = UIColor.clearColor;
    confirmLabel.textColor = UIColor.whiteColor;
    confirmLabel.outlineColor = UIColor.blackColor;
    confirmLabel.outlineThickness = 3.0;
    [confirmLabel sizeToFit];
    labelFrame = confirmLabel.frame;
    labelFrame.size.width  += confirmLabel.outlineThickness * 3;
    labelFrame.size.height += confirmLabel.outlineThickness * 3;
    labelFrame.origin.x += (confirmButton.frame.size.width  - labelFrame.size.width)/2  + 2;
    labelFrame.origin.y += (confirmButton.frame.size.height - labelFrame.size.height)/2;
    confirmLabel.frame = labelFrame;
    [confirmButton addSubview:confirmLabel];
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

//    CGSize labelSize = [m_label.text sizeWithFont:m_label.font
//                                constrainedToSize:m_label.frame.size
//                                    lineBreakMode:m_label.lineBreakMode];
//
//    CGFloat y = (m_maxLabelHeight - labelSize.height) / 2;
//
//    m_label.frame = CGRectMake(
//        m_label.frame.origin.x, y,
//        m_label.frame.size.width, labelSize.height);

    if (!self.cancelButtonLabel || [self.cancelButtonLabel isEqualToString:@""])
    {
        self.cancelButton.hidden = YES;
    }
    else
    {
        self.cancelButton.hidden = NO;
    }
}


- (IBAction)onConfirmButtonTouchUp:(id)sender 
{
    Z::Engine::Resume();

    Sounds.Play( m_hButtonClickSound );

//    GameObjects.SendMessageFromSystem( MSG_GameOver       );
//    GameObjects.SendMessageFromSystem( MSG_GameScreenHome );

    self.onConfirm.Invoke();
}


- (IBAction)onCancelButtonTouchUp:(id)sender
{
    Z::Engine::Resume();

    Sounds.Play( m_hButtonClickSound );

//    GameObjects.SendMessageFromSystem( MSG_GameScreenPrevious );

    self.onCancel.Invoke();
}



+ (DialogViewController*) createDialogWithMessage:(NSString*)message confirmLabel:(NSString*)confirmLabel cancelLabel:(NSString*)cancelLabel
{
    DialogViewController* dialogVC = [[DialogViewController alloc] initWithNibName:nil bundle:nil];

    dialogVC.message = message;
    dialogVC.confirmButtonLabel = confirmLabel ? confirmLabel : @"hoya";
    dialogVC.cancelButtonLabel  = cancelLabel  ? cancelLabel  : @"hermano";
    
    [dialogVC.cancelButton  addTarget:self action:@selector(onCancelButtonTouchUp:)  forControlEvents:UIControlEventTouchUpInside];
    [dialogVC.confirmButton addTarget:self action:@selector(onConfirmButtonTouchUp:) forControlEvents:UIControlEventTouchUpInside];
    
    Z::HScene hScene;
    SceneMan.CreateScene( "Dialog", dialogVC, &hScene );
    dialogVC.hDialogScene = hScene;

    return dialogVC;
}



- (void)sethDialogEffect:(Z::HEffect)hEffect
{
    hDialogEffect.Release();
    hDialogEffect = hEffect;
}



- (void)sethShowDialogStoryboard:(Z::HStoryboard)hStoryboard
{
    hShowDialogStoryboard.Release();
    hShowDialogStoryboard = hStoryboard;
}



- (void)sethHideDialogStoryboard:(Z::HStoryboard)hStoryboard
{
    hHideDialogStoryboard.Release();
    hHideDialogStoryboard = hStoryboard;
}



- (void)show
{
    [self showWithStoryboard:nil andEffect:nil];
    
    Z::Engine::Pause();
}



- (void)hide
{
    [self hideWithStoryboard:nil andEffect:nil];

    Z::Engine::Resume();
}



- (void)showWithStoryboard:(NSString*)storyboard andEffect:(NSString*)effect
{
    if (!storyboard)
    {
        storyboard = @"TwistIn";
    }

    if (!effect)
    {
        effect = @"MorphEffect";
    }
    
    Z::HStoryboard hStoryboard;
    Storyboards.GetCopy([storyboard UTF8String], &hStoryboard);
    self.hShowDialogStoryboard = hStoryboard;

    Z::HEffect hEffect;
    Effects.GetCopy([effect UTF8String], &hEffect);
    self.hDialogEffect = hEffect;
    
//    Storyboards.CallbackOnFinished( self.hShowDialogStoryboard, self.didAppearCallback );
    SceneMan.Show( self.hDialogScene, self.hShowDialogStoryboard, self.hDialogEffect );
}



- (void)hideWithStoryboard:(NSString*)storyboard andEffect:(NSString*)effect
{
    if (!storyboard)
    {
        storyboard = @"TwistOut";
    }

    if (!effect)
    {
        effect = @"MorphEffect";
    }
    
    Z::HStoryboard hStoryboard;
    Storyboards.GetCopy([storyboard UTF8String], &hStoryboard);
    self.hHideDialogStoryboard = hStoryboard;

    Z::HEffect hEffect;
    Effects.GetCopy([effect UTF8String], &hEffect);
    self.hDialogEffect = hEffect;

    SceneMan.Hide( self.hDialogScene, self.hHideDialogStoryboard, self.hDialogEffect );
}




@end
