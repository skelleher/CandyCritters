//
//  GameOverViewController.h
//  Critters
//
//  Created by Sean Kelleher on 4/5/11.
//  Copyright 2011 Sean Kelleher. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "Engine.hpp"
#import "CustomUILabel.h"


@interface GameOverScreenViewController : UIViewController 
{
    
    IBOutlet UIButton       *tryAgainButton;
    IBOutlet UIButton       *mainMenuButton;
    IBOutlet CustomUILabel  *scoreLabel;
    IBOutlet CustomUILabel  *highScoreLabel;
    IBOutlet CustomUILabel  *scoreValueLabel;
    IBOutlet CustomUILabel  *highScoreValueLabel;
    IBOutlet CustomUILabel  *newHighScoreLabel;
    Z::HSound               m_hButtonClickSound;
}

- (IBAction)tryAgainButtonTouchUp:(id)sender;
- (IBAction)mainMenuButtonTouchUp:(id)sender;

@end
