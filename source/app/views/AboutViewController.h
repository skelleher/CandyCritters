//
//  AboutViewController.h
//  Critters
//
//  Created by Sean Kelleher on 8/22/11.
//  Copyright 2011 Sean Kelleher. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "Engine.hpp"
#import "CustomUILabel.h"

@interface AboutViewController : UIViewController 
{
    IBOutlet UIButton*      m_backButton;
    IBOutlet CustomUILabel* m_programmerLabel;
    IBOutlet CustomUILabel* m_programmerValueLabel;
    IBOutlet CustomUILabel* m_artistLabel;
    IBOutlet CustomUILabel* m_artistValueLabel;
    IBOutlet CustomUILabel* m_thanksLabel;
    IBOutlet CustomUILabel* m_thanksValueLabel;
    IBOutlet CustomUILabel* m_buildInfoLabel;
    Z::HSound               m_hButtonClickSound;
}


- (IBAction)onBackButtonTouchUp:(id)sender;
- (IBAction)onBackButtonTouchDown:(id)sender;


@end
