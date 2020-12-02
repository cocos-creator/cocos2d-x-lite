/****************************************************************************
 Copyright (c) 2010-2013 cocos2d-x.org
 Copyright (c) 2013-2016 Chukong Technologies Inc.
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#include "AppDelegate.h"
#import "ViewController.h"
#include "platform/ios/View.h"
#include "cocos/bindings/event/EventDispatcher.h"

#include "SDKWrapper.h"
#include "Game.h"

cc::Device::Orientation _lastOrientation;

@implementation AppDelegate

Game* game = nullptr;
@synthesize window;

#pragma mark -
#pragma mark Application lifecycle

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
//     [[SDKWrapper getInstance] application:application didFinishLaunchingWithOptions:launchOptions];
    // Add the view controller's view to the window and display.
    CGRect bounds = [[UIScreen mainScreen] bounds];
    self.window = [[UIWindow alloc] initWithFrame: bounds];

    // Should create view controller first, cc::Application will use it.
    _viewController = [[ViewController alloc]init];
    _viewController.view = [[View alloc] initWithFrame:bounds];
    _viewController.view.contentScaleFactor = UIScreen.mainScreen.scale;
    [self.window setRootViewController:_viewController];

    // cocos2d application instance
    game = new Game(bounds.size.width, bounds.size.height);
    game->init();

    [[NSNotificationCenter defaultCenter] addObserver:self
        selector:@selector(statusBarOrientationChanged:)name:UIApplicationDidChangeStatusBarOrientationNotification object:nil];

    [self.window makeKeyAndVisible];

    return YES;
}


- (void) statusBarOrientationChanged:(NSNotification *)note
{
    // https://developer.apple.com/documentation/uikit/uideviceorientation
    // NOTE: do not use API [UIDevice currentDevice].orientation to get the device orientation
    // when the device rotates to LandscapeLeft, device.orientation returns UIDeviceOrientationLandscapeRight
    // when the device rotates to LandscapeRight, device.orientation returns UIDeviceOrientationLandscapeLeft
    cc::Device::Orientation orientation;
    switch([[UIApplication sharedApplication] statusBarOrientation])
    {
        case UIInterfaceOrientationLandscapeRight:
            orientation = cc::Device::Orientation::LANDSCAPE_RIGHT;
            break;
        case UIInterfaceOrientationLandscapeLeft:
            orientation = cc::Device::Orientation::LANDSCAPE_LEFT;
            break;
        case UIInterfaceOrientationPortraitUpsideDown:
            orientation = cc::Device::Orientation::PORTRAIT_UPSIDE_DOWN;
            break;
        case UIInterfaceOrientationPortrait:
            orientation = cc::Device::Orientation::PORTRAIT;
            break;
        default:
            break;
    }
    if(_lastOrientation != orientation){
        cc::EventDispatcher::dispatchOrientationChangeEvent((int) orientation);
        _lastOrientation = orientation;
    }
}

- (void)applicationWillResignActive:(UIApplication *)application {
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
//     [[SDKWrapper getInstance] applicationWillResignActive:application];
    game->onPause();
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    /*
     Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
     */
//     [[SDKWrapper getInstance] applicationDidBecomeActive:application];
    game->onResume();
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    /*
     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
     If your application supports background execution, called instead of applicationWillTerminate: when the user quits.
     */
//     [[SDKWrapper getInstance] applicationDidEnterBackground:application];
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    /*
     Called as part of  transition from the background to the inactive state: here you can undo many of the changes made on entering the background.
     */
//     [[SDKWrapper getInstance] applicationWillEnterForeground:application];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
//     [[SDKWrapper getInstance] applicationWillTerminate:application];
    delete game;
    game = nullptr;
}

#pragma mark -
#pragma mark Memory management

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
    cc::EventDispatcher::dispatchMemoryWarningEvent();
}

@end
