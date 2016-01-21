/*!
 * QUINE VISION, INC. ("COMPANY") CONFIDENTIAL
 * Unpublished Copyright (c) 2014 QUINE VISION, INC., All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of COMPANY. The intellectual and technical
 * concepts contained herein are proprietary to COMPANY and
 * may be covered by U.S. and Foreign Patents, patents in
 * process, and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this
 * material is strictly forbidden unless prior written permission
 * is obtained from COMPANY.  Access to the source code contained
 * herein is hereby forbidden to anyone except current COMPANY
 * employees, managers or contractors who have executed
 * Confidentiality and Non-disclosure agreements explicitly
 * covering such access.
 *
 * The copyright notice above does not evidence any actual or
 * intended publication or disclosure  of  this source code, which
 * includes information that is confidential and/or proprietary,
 * and is a trade secret, of  COMPANY.
 *
 * ANY REPRODUCTION, MODIFICATION, DISTRIBUTION, PUBLIC  PERFORMANCE,
 * OR PUBLIC DISPLAY OF OR THROUGH USE  OF THIS  SOURCE CODE  WITHOUT
 * THE EXPRESS WRITTEN CONSENT OF COMPANY IS STRICTLY PROHIBITED,
 * AND IN VIOLATION OF APPLICABLE LAWS AND INTERNATIONAL TREATIES.
 * THE RECEIPT OR POSSESSION OF  THIS SOURCE CODE AND/OR RELATED
 * INFORMATION DOES NOT CONVEY OR IMPLY ANY RIGHTS TO REPRODUCE,
 * DISCLOSE OR DISTRIBUTE ITS CONTENTS, OR TO MANUFACTURE, USE,
 * OR SELL ANYTHING THAT IT  MAY DESCRIBE, IN WHOLE OR IN PART.
 *
 * @file   QuineVideoCaptureViewController.mm
 * @author Brett Spurrier (brett@quinevision.com)
 * @date   August, 2014
 * @brief  Class file for managing the camera and camera frame processing.
 *
 */


// Imports
#import "QuineVideoCaptureViewController.h"
#import "QuineImageManager.h"
#import "QuineCompare.h"

// Includes
#include "QuineFeatureDetection.h"
#include "QuineDatabaseOperations.h"
#include "QuineImageCompare.h"
#include "QuineDictionary.h"
#include "QuineConstants.h"


/* ************************************************************************* */
/*!
 *  Interface (Private)
 */
@interface QuineVideoCaptureViewController () {
    QuineImageManager *_imageManager;
    QuineCompare *_imageCompare;
    BOOL _isProcessing;
    BOOL _isEnabled;
}
- (void)displayFeatures:(const std::vector<cv::KeyPoint> &)faces
           forVideoRect:(CGRect)rect
       videoOrientation:(AVCaptureVideoOrientation)videoOrientation;
@end


/* ************************************************************************* */
/*!
 * Implementation
 */
@implementation QuineVideoCaptureViewController


/* ************************************************************************* */
/*!
 *  Initialization
 *
 *  @return self
 */
- (id)init
{
    self = [super init];
    if (self) {
        self.captureGrayscale = YES;
        self.qualityPreset = AVCaptureSessionPresetMedium;
        
        if(!_imageManager) {
            _imageManager = [[QuineImageManager alloc] init];
        }
        
        if(!_imageCompare) {
            _imageCompare = [[QuineCompare alloc] init];
        }
        
        _isEnabled = YES;
        
    }
    return self;
}

#pragma mark - View lifecycle

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    NSLog(@"Loading scanner view\n");
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(setScanningStartNotification:)
                                                 name:@"note_quine_scanner_start"
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(setScanningStopNotification:)
                                                 name:@"note_quine_scanner_stop"
                                               object:nil];
    
}

-(void)dealloc {
    
    NSLog(@"Deallocating scanner\n");
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:@"note_quine_scanner_start"
                                                  object:nil];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:@"note_quine_scanner_stop"
                                                  object:nil];
    
}

- (void)viewDidUnload
{
    [super viewDidUnload];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}



/* ************************************************************************* */
/*!
 *  @brief Toggles the camera torch.
 *
 *  This toggle will turn the camera torch on and off.
 *  The method is exposed for all devices,  however, won't 
 *  perform any actions for those devices with only rear 
 *  facing cameras.
 *
 *  @return void
 */
- (void)toggleTorch
{
    self.torchOn = !self.torchOn;
}


/* ************************************************************************* */
/*!
 *  @brief Toggles the camera back to font.
 *
 *  This toggle will move switch the camera from front to back, and then
 *  again from back to front. The method is exposed for all devices,
 *  however, won't perform any actions for those devices with only
 *  rear facing cameras.
 *
 *  @return void
 */
- (void)toggleCamera
{
    if (self.camera == 1) {
        self.camera = 0;
    }
    else
    {
        self.camera = 1;
    }
}

-(void)setScanningStartNotification:(NSNotification *)notification {
    [self setScanningEnabled:YES];
}

-(void)setScanningStopNotification:(NSNotification *)notification {
    [self setScanningEnabled:NO];
}

-(void)setScanningEnabled:(BOOL)enabled {
    _isEnabled = enabled;
}

// MARK: VideoCaptureViewController overrides

- (void)processFrame:(cv::Mat &)mat
           videoRect:(CGRect)rect
    videoOrientation:(AVCaptureVideoOrientation)videOrientation
{
    
    //////////////////////////////////////////////////////
    // Process the current frame with no
    //   overlapping of frames
    
    if(!_isProcessing && _isEnabled) {
        _isProcessing = YES;
        
        
        //////////////////////////////////////////////////////
        // Transpose the frame
        
        cv::transpose(mat, mat);
        CGFloat temp = rect.size.width;
        rect.size.width = rect.size.height;
        rect.size.height = temp;
        
        if (videOrientation == AVCaptureVideoOrientationLandscapeRight) {
            cv::flip(mat, mat, 1);
        }
        else {
            // Front camera output needs to be mirrored to match
            //   preview layer so no flip is required here.
        }
        videOrientation = AVCaptureVideoOrientationPortrait;
        
        
        //////////////////////////////////////////////////////
        // Compare the features with the loaded database
        
        // TODO: There is a nice size memory leak in compareMatToLoadedDatabase. Need to find that.
        NSDictionary *resultsDictionary = [_imageCompare compareMatToLoadedDatabase:mat];
        NSMutableDictionary *resultsDictionaryCopy = [resultsDictionary mutableCopy];
  
        
        //////////////////////////////////////////////////////
        // Send the result off to the result block, but only
        //   if there is a result
        
        for (id key in resultsDictionary) {
            if([[resultsDictionaryCopy objectForKey:key] isEqualToString:@""]) {
                [resultsDictionaryCopy removeObjectForKey:key];
            }
        }
        
        if(self.result && [[resultsDictionaryCopy allKeys] count] > 0) {
            dispatch_async(dispatch_get_main_queue(), ^{
                self.result(self, resultsDictionaryCopy);
            });
        }
        
        
        //////////////////////////////////////////////////////
        // Reset the processing flag
        
        _isProcessing = NO;
    }
}


// Update face markers given vector of face rectangles
- (void)displayFeatures:(const std::vector<cv::KeyPoint> &)features
        forVideoRect:(CGRect)rect
    videoOrientation:(AVCaptureVideoOrientation)videoOrientation
{
    NSArray *sublayers = [NSArray arrayWithArray:[self.view.layer sublayers]];
    int sublayersCount = [sublayers count];
    int currentSublayer = 0;
    
	[CATransaction begin];
	[CATransaction setValue:(id)kCFBooleanTrue forKey:kCATransactionDisableActions];
	
	// hide all the face layers
	for (CAShapeLayer *layer in sublayers) {
        NSString *layerName = [layer name];
		if ([layerName isEqualToString:@"FaceLayer"])
			[layer setHidden:YES];
	}
    
    // Create transform to convert from vide frame coordinate space to view coordinate space
    CGAffineTransform t = [self affineTransformForVideoFrame:rect orientation:videoOrientation];
    
    for (int i = 0; i < features.size(); i++) {
        
        CGRect featureRect;
        featureRect.origin.x = features[i].pt.x - (features[i].size / 2);
        featureRect.origin.y = features[i].pt.y - (features[i].size / 2);
        featureRect.size.width = features[i].size / 2.0;
        featureRect.size.height = features[i].size / 2.0;
        
        featureRect = CGRectApplyAffineTransform(featureRect, t);
        
        CAShapeLayer *featureLayer = nil;

        while (!featureLayer && (currentSublayer < sublayersCount)) {
			CAShapeLayer *currentLayer = [sublayers objectAtIndex:currentSublayer++];
			if ([[currentLayer name] isEqualToString:@"FaceLayer"]) {
				featureLayer = currentLayer;
				[currentLayer setHidden:NO];
			}
		}
        
        if (!featureLayer) {
			featureLayer = [[CAShapeLayer alloc] init];
            featureLayer.name = @"FaceLayer";
            
            featureLayer.path = [UIBezierPath bezierPathWithRoundedRect:CGRectMake(0,
                                                                                   0,
                                                                                   2.0*features[i].size,
                                                                                   2.0*features[i].size)
                                                     cornerRadius:features[i].size].CGPath;
            
            featureLayer.position = CGPointMake(CGRectGetMidX(self.view.frame)-(features[i].size / 2.0),
                                                CGRectGetMidY(self.view.frame)-(features[i].size / 2.0));
            featureLayer.borderColor = [[UIColor redColor] CGColor];
            featureLayer.borderWidth = 1.0f;
			[self.view.layer addSublayer:featureLayer];
		}
        
        featureLayer.frame = featureRect;
    }
    
    [CATransaction commit];
}

@end