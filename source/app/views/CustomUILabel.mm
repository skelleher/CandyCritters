//
//  CustomUILabel.m
//  Critters
//
//  Created by Sean Kelleher on 11/28/12.
//
//

#import "CustomUILabel.h"

@implementation CustomUILabel

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        self.shadowColor       = [UIColor.darkGrayColor colorWithAlphaComponent:0.90f];
        self.shadowOffset      = CGSizeMake(2, 2);
        self.outlineThickness  = 1;
        self.outlineColor      = UIColor.blackColor;
    }
    return self;
}



- (void)drawTextInRect:(CGRect)rect
{
    // Save shadow offset and text color
    CGSize shadowOffset = self.shadowOffset;
    UIColor *textColor  = self.textColor;
    CGContextRef c      = UIGraphicsGetCurrentContext();

    CGRect rectIncludingOutline = rect;
    rectIncludingOutline.size.width  += self.outlineThickness * 2;
    rectIncludingOutline.size.height += self.outlineThickness * 2;
    rectIncludingOutline.origin.x    += self.outlineThickness;
    rectIncludingOutline.origin.y    += self.outlineThickness;

    // Draw text normally (with optional dropshadow)
    CGContextSetTextDrawingMode(c, kCGTextFill);
    [super drawTextInRect:rectIncludingOutline];

    // Draw outline
    CGContextSetTextDrawingMode(c, kCGTextStroke);
    CGContextSetLineJoin(c, kCGLineJoinRound);
    CGContextSetLineWidth(c, self.outlineThickness);
    self.textColor      = self.outlineColor;
    self.shadowOffset   = CGSizeMake(0, 0);
    [super drawTextInRect:rectIncludingOutline];

    // Restore settings
    self.shadowOffset   = shadowOffset;
    self.textColor      = textColor;
}


@end
