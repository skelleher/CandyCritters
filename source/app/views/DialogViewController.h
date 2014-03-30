//
//  DialogViewController.h
//  Critters
//
//  Created by Sean Kelleher on 5/27/13.
//  Copyright 2013 Sean Kelleher. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "Engine.hpp"
#import "CustomUILabel.h"
#import "Callback.hpp"


@interface DialogViewController : UIViewController
{
    Z::HSound m_hButtonClickSound;
    IBOutlet CustomUILabel* m_label;
    IBOutlet UIButton* cancelButton;
    IBOutlet UIButton* confirmButton;
    float m_maxLabelHeight;
    NSString* m_text;
}

@property(readwrite, retain) NSString* message;
@property(readwrite, retain) NSString* confirmButtonLabel;
@property(readwrite, retain) NSString* cancelButtonLabel;
@property(readwrite, retain) UIButton* cancelButton;
@property(readwrite, retain) UIButton* confirmButton;
@property(nonatomic, assign) Z::Callback onConfirm;
@property(nonatomic, assign) Z::Callback onCancel;


+ (DialogViewController*) createDialogWithMessage:(NSString*)message confirmLabel:(NSString*)confirmLabel cancelLabel:(NSString*)cancelLabel;

- (void)show;
- (void)hide;
- (void)showWithStoryboard:(NSString*)storyboard andEffect:(NSString*)effect;
- (void)hideWithStoryboard:(NSString*)storyboard andEffect:(NSString*)effect;

- (IBAction)onConfirmButtonTouchUp:(id)sender;
- (IBAction)onCancelButtonTouchUp:(id)sender;

@end
