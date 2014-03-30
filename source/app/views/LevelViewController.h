//
//  LevelViewController
//  Critters
//
//  Created by Sean Kelleher on 11/19/12.
//  Copyright 2011 Sean Kelleher. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "Engine.hpp"
#import "CustomUILabel.h"


@interface LevelViewController : UIViewController
{
//    IBOutlet UIButton       *m_okButton;
//    IBOutlet UISlider       *m_difficultySlider;
//    IBOutlet CustomUILabel  *m_difficultyValue;
    
    Z::HSound           m_hButtonClickSound;
}

//- (IBAction)onContinueButtonTouchUp:(id)sender;
//- (IBAction)onContinueButtonTouchDown:(id)sender;
//- (IBAction)onLevelSliderValueChanged:(id)sender;
//- (IBAction)onLevelSliderDoneChanging:(id)sender;
//- (void)setDifficulty:(int)difficulty;

@end
