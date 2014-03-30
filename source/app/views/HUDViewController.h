//
//  HUDViewController.h
//  Critters
//
//  Created by Sean Kelleher on 4/5/11.
//  Copyright 2011 Sean Kelleher. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "Engine.hpp"


@interface HUDViewController : UIViewController 
{
    IBOutlet UIButton *m_pauseButton;
    IBOutlet UIButton *m_leftButton;
    IBOutlet UIButton *m_rightButton;
    IBOutlet UIButton *m_rotateButton;
    IBOutlet UIButton *m_dropButton;
    Z::HSound          m_hButtonClickSound;
}


- (IBAction)onPauseTouchUp:(id)sender;
- (IBAction)onPauseTouchDown:(id)sender;
- (IBAction)onLeftTouchUp:(id)sender;
- (IBAction)onLeftTouchDown:(id)sender;
- (IBAction)onRightTouchUp:(id)sender;
- (IBAction)onRightTouchDown:(id)sender;
- (IBAction)onRotateTouchUp:(id)sender;
- (IBAction)onRotateTouchDown:(id)sender;
- (IBAction)onDropTouchUp:(id)sender;
- (IBAction)onDropTouchDown:(id)sender;


@end
