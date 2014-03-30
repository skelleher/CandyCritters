//
//  TutorialViewController.h
//  Critters
//
//  Created by Sean Kelleher on 8/29/11.
//  Copyright 2011 Sean Kelleher. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "Engine.hpp"


@interface TutorialViewController : UIViewController 
{
    IBOutlet UIButton *m_doneButton;
    Z::HSound          m_hButtonClickSound;
}


- (IBAction)onDoneTouchUp:(id)sender;
- (IBAction)onDoneTouchDown:(id)sender;


@end
