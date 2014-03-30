//
//  main.m
//  Critters
//
//  Created by Sean Kelleher on 8/31/10.
//  Copyright Sean Kelleher 2010. All rights reserved.
//

#import <UIKit/UIKit.h>

int main(int argc, char *argv[]) 
{
    
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];


    //
    // Because we have no .xib specified in our .plist, explicitly pass the 
    // delegate name as the fourth parameter.
    //
    int retVal = UIApplicationMain(argc, argv, nil, @"OpenGLAppDelegate");
    
    
    [pool release];
    return retVal;
}
