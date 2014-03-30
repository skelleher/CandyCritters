//
//  HomeScreenViewController.h
//  Critters
//
//  Created by Sean Kelleher on 3/28/11.
//  Copyright 2011 Sean Kelleher. All rights reserved.
//
//  NOTE: this must be a .h file to support Interface Builder.
//  Maybe that's been fixed in Xcode 4.
//  The implementation can be .mm, and in fact needs to be in 
//  order to include the Z Engine header files.
//
//

#import <UIKit/UIKit.h>
#import <MessageUI/MFMailComposeViewController.h>
#import "Engine.hpp"


@interface HomeScreenViewController : UIViewController <MFMailComposeViewControllerDelegate>
{
    Z::HSound               m_hNewGameSound;
    Z::HSound               m_hButtonTapSound;

    IBOutlet UIButton       *playButton;
    IBOutlet UIButton       *aboutButton;
    IBOutlet UIButton       *tutorialButton;

    IBOutlet UIButton       *mailLogButton;
}


- (IBAction) onPlayButtonTapped;
- (IBAction) onAboutButtonTapped;
- (IBAction) onScoresButtonTapped;
- (IBAction) onAwardsButtonTapped;
- (IBAction) onTutorialButtonTapped;
- (IBAction) onMailButtonTapped;

@end
