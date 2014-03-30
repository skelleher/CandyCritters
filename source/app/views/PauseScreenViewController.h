//
//  PauseScreenViewController.h
//  Critters
//
//  Created by Sean Kelleher on 4/5/11.
//  Copyright 2011 Sean Kelleher. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "Engine.hpp"

// TEST
#import <MessageUI/MFMailComposeViewController.h>


@interface PauseScreenViewController : UIViewController <MFMailComposeViewControllerDelegate>
{
    
    IBOutlet UIButton *resumeButton;
    IBOutlet UIButton *quitButton;
//    IBOutlet UIButton *mailLogButton;
    IBOutlet UISlider *soundSlider;
    
    Z::HSound           m_hButtonClickSound;
    Z::HSound           m_hFXSound;
}

- (IBAction)onResumeButtonTouchDown:(id)sender;
- (IBAction)onResumeButtonTouchUp:(id)sender;
- (IBAction)onQuitButtonTouchDown:(id)sender;
- (IBAction)onQuitButtonTouchUp:(id)sender;
//- (IBAction)onMailButtonTouchUp:(id)sender;

- (IBAction)onSoundVolumeChange:(id)sender;
- (IBAction)onSoundVolumeDone:(id)sender;

@end
