//
//  ConfirmViewController.h
//  Critters
//
//  Created by Sean Kelleher on 5/14/11.
//  Copyright 2011 Sean Kelleher. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "Engine.hpp"
#import "CustomUILabel.h"


@interface ConfirmViewController : UIViewController 
{
    Z::HSound m_hButtonClickSound;
    IBOutlet CustomUILabel* m_label;
    IBOutlet UIButton* noButton;
    IBOutlet UIButton* yesButton;
    float m_maxLabelHeight;
    
    NSString* m_text;
}

@property(readwrite, retain) NSString* message;


- (IBAction)onYesButtonTouchUp:(id)sender;
- (IBAction)onNoButtonTouchUp:(id)sender;

@end
