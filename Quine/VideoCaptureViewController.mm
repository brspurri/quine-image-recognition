/*
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
 * @file   VideoCaptureViewController.mm
 * @Author Brett Spurrier (brett@quinevision.com)
 * @date   August, 2014
 * @brief  Class file for image comparisons with currently loaded databases.
 *
 */

#import "VideoCaptureViewController.h"


// Number of frames to average for FPS calculation
const int kFrameTimeBufferSize = 5;

// Private interface
@interface VideoCaptureViewController ()
- (BOOL)createCaptureSessionForCamera:(NSInteger)camera grayscale:(BOOL)grayscale;
- (void)destroyCaptureSession;
- (void)processFrame:(cv::Mat&)mat videoRect:(CGRect)rect videoOrientation:(AVCaptureVideoOrientation)orientation;
- (void)updateDebugInfo;
@property (nonatomic, assign) float fps;
@end


@implementation VideoCaptureViewController

@synthesize fps = _fps;
@synthesize camera = _camera;
@synthesize captureGrayscale = _captureGrayscale;
@synthesize qualityPreset = _qualityPreset;
@synthesize captureSession = _captureSession;
@synthesize captureDevice = _captureDevice;
@synthesize videoOutput = _videoOutput;
@synthesize videoPreviewLayer = _videoPreviewLayer;

@dynamic showDebugInfo;
@dynamic torchOn;

- (id)init
{
    self = [super init];
    if (self) {
        _camera = -1;
        _captureGrayscale = YES;
        
        // Create frame time circular buffer for calculating averaged fps
        _frameTimes = (float*)malloc(sizeof(float) * kFrameTimeBufferSize);
    }
    return self;
}

- (void)dealloc
{
    [self destroyCaptureSession];
    _fpsLabel = nil;
    if (_frameTimes) {
        free(_frameTimes);
    }

}
- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self createCaptureSessionForCamera:_camera
                              grayscale:NO];
    [_captureSession startRunning];
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    
    [self destroyCaptureSession];
    _fpsLabel = nil;
}

// MARK: Accessors
- (void)setFps:(float)fps
{
    [self willChangeValueForKey:@"fps"];
    _fps = fps;
    [self didChangeValueForKey:@"fps"];
    
    [self updateDebugInfo];
}

- (BOOL)showDebugInfo
{
    return (_fpsLabel != nil);
}

// Show/hide debug panel with current FPS
- (void)setShowDebugInfo:(BOOL)showDebugInfo
{
    if (!showDebugInfo && _fpsLabel) {
        [_fpsLabel removeFromSuperview];
        _fpsLabel = nil;
    }
    
    if (showDebugInfo && !_fpsLabel) {
        // Create label to show FPS
        CGRect frame = self.view.bounds;
        frame.size.height = 40.0f;
        _fpsLabel = [[UILabel alloc] initWithFrame:frame];
        _fpsLabel.textColor = [UIColor whiteColor];
        _fpsLabel.backgroundColor = [UIColor colorWithWhite:0.0f alpha:0.5f];
        [self.view addSubview:_fpsLabel];
        
        [self updateDebugInfo];
    }
}

// Set torch on or off (if supported)
- (void)setTorchOn:(BOOL)torch
{
    NSError *error = nil;
    if ([_captureDevice hasTorch]) {
        BOOL locked = [_captureDevice lockForConfiguration:&error];
        if (locked) {
            _captureDevice.torchMode = (torch)? AVCaptureTorchModeOn : AVCaptureTorchModeOff;
            [_captureDevice unlockForConfiguration];
        }
    }
}

// Return YES if the torch is on
- (BOOL)torchOn
{
    return (_captureDevice.torchMode == AVCaptureTorchModeOn);
}


// Switch camera 'on-the-fly'
//
// camera: 0 for back camera, 1 for front camera
//
- (void)setCamera:(int)camera
{
    if (camera != _camera)
    {
        _camera = camera;
        
        if (_captureSession) {
            NSArray* devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
            
            [_captureSession beginConfiguration];
            
            [_captureSession removeInput:[[_captureSession inputs] lastObject]];
            
            if (_camera >= 0 && _camera < [devices count]) {
                _captureDevice = [devices objectAtIndex:camera];
            }
            else {
                _captureDevice = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
            }
            
            // Create device input
            NSError *error = nil;
            AVCaptureDeviceInput *input = [[AVCaptureDeviceInput alloc] initWithDevice:_captureDevice error:&error];
            [_captureSession addInput:input];
            
            [_captureSession commitConfiguration];
        }
    }
}

// MARK: AVCaptureVideoDataOutputSampleBufferDelegate delegate methods

// AVCaptureVideoDataOutputSampleBufferDelegate delegate method called when a video frame is available
//
// This method is called on the video capture GCD queue. A cv::Mat is created from the frame data and
// passed on for processing with OpenCV.
- (void)captureOutput:(AVCaptureOutput *)captureOutput
didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection *)connection
{
    @autoreleasepool {
        
        
        CVPixelBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
        OSType format = CVPixelBufferGetPixelFormatType(pixelBuffer);
        CGRect videoRect = CGRectMake(0.0f, 0.0f, CVPixelBufferGetWidth(pixelBuffer), CVPixelBufferGetHeight(pixelBuffer));
        AVCaptureVideoOrientation videoOrientation = [[[_videoOutput connections] objectAtIndex:0] videoOrientation];
        
        if (format == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange) {
            // For grayscale mode, the luminance channel of the YUV data is used
            CVPixelBufferLockBaseAddress(pixelBuffer, 0);
            void *baseaddress = CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 0);
            
            cv::Mat mat(videoRect.size.height, videoRect.size.width, CV_8UC1, baseaddress, 0);
            
            [self processFrame:mat videoRect:videoRect videoOrientation:videoOrientation];
            
            CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
        }
        else if (format == kCVPixelFormatType_32BGRA) {
            // For color mode a 4-channel cv::Mat is created from the BGRA data
            CVPixelBufferLockBaseAddress(pixelBuffer, 0);
            void *baseaddress = CVPixelBufferGetBaseAddress(pixelBuffer);
            
            cv::Mat mat(videoRect.size.height, videoRect.size.width, CV_8UC4, baseaddress, 0);
            
            [self processFrame:mat videoRect:videoRect videoOrientation:videoOrientation];
            
            CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
        }
        else {
            NSLog(@"Unsupported video format");
        }
        
        // Update FPS calculation
        CMTime presentationTime = CMSampleBufferGetOutputPresentationTimeStamp(sampleBuffer);
        
        if (_lastFrameTimestamp == 0) {
            _lastFrameTimestamp = presentationTime.value;
            _framesToAverage = 1;
        }
        else {
            float frameTime = (float)(presentationTime.value - _lastFrameTimestamp) / presentationTime.timescale;
            _lastFrameTimestamp = presentationTime.value;
            
            _frameTimes[_frameTimesIndex++] = frameTime;
            
            if (_frameTimesIndex >= kFrameTimeBufferSize) {
                _frameTimesIndex = 0;
            }
            
            float totalFrameTime = 0.0f;
            for (int i = 0; i < _framesToAverage; i++) {
                totalFrameTime += _frameTimes[i];
            }
            
            float averageFrameTime = totalFrameTime / _framesToAverage;
            float fps = 1.0f / averageFrameTime;
            
            if (fabsf(fps - _captureQueueFps) > 0.1f) {
                _captureQueueFps = fps;
                dispatch_async(dispatch_get_main_queue(), ^{
                    [self setFps:fps];
                });
            }
            
            _framesToAverage++;
            if (_framesToAverage > kFrameTimeBufferSize) {
                _framesToAverage = kFrameTimeBufferSize;
            }
        }
    
    }
}


// MARK: Methods to override

// Override this method to process the video frame with OpenCV
//
// Note that this method is called on the video capture GCD queue. Use dispatch_sync or dispatch_async to update UI
// from the main queue.
//
// mat: The frame as an OpenCV::Mat object. The matrix will have 1 channel for grayscale frames and 4 channels for
//      BGRA frames. (Use -[VideoCaptureViewController setGrayscale:])
// rect: A CGRect describing the video frame dimensions
// orientation: Will generally by AVCaptureVideoOrientationLandscapeRight for the back camera and
//              AVCaptureVideoOrientationLandscapeRight for the front camera
//
- (void)processFrame:(cv::Mat &)mat videoRect:(CGRect)rect videoOrientation:(AVCaptureVideoOrientation)orientation
{
    
}

// MARK: Geometry methods

// Create an affine transform for converting CGPoints and CGRects from the video frame coordinate space to the
// preview layer coordinate space. Usage:
//
// CGPoint viewPoint = CGPointApplyAffineTransform(videoPoint, transform);
// CGRect viewRect = CGRectApplyAffineTransform(videoRect, transform);
//
// Use CGAffineTransformInvert to create an inverse transform for converting from the view cooridinate space to
// the video frame coordinate space.
//
// videoFrame: a rect describing the dimensions of the video frame
// video orientation: the video orientation
//
// Returns an affine transform
//
- (CGAffineTransform)affineTransformForVideoFrame:(CGRect)videoFrame
                                      orientation:(AVCaptureVideoOrientation)videoOrientation
{
    CGSize viewSize = self.view.bounds.size;
    NSString * const videoGravity = _videoPreviewLayer.videoGravity;
    CGFloat widthScale = 1.0f;
    CGFloat heightScale = 1.0f;
    
    // Move origin to center so rotation and scale are applied correctly
    CGAffineTransform t = CGAffineTransformMakeTranslation(-videoFrame.size.width / 2.0f, -videoFrame.size.height / 2.0f);
    
    switch (videoOrientation) {
        case AVCaptureVideoOrientationPortrait:
            widthScale = viewSize.width / videoFrame.size.width;
            heightScale = viewSize.height / videoFrame.size.height;
            break;
            
        case AVCaptureVideoOrientationPortraitUpsideDown:
            t = CGAffineTransformConcat(t, CGAffineTransformMakeRotation(M_PI));
            widthScale = viewSize.width / videoFrame.size.width;
            heightScale = viewSize.height / videoFrame.size.height;
            break;
            
        case AVCaptureVideoOrientationLandscapeRight:
            t = CGAffineTransformConcat(t, CGAffineTransformMakeRotation(M_PI_2));
            widthScale = viewSize.width / videoFrame.size.height;
            heightScale = viewSize.height / videoFrame.size.width;
            break;
            
        case AVCaptureVideoOrientationLandscapeLeft:
            t = CGAffineTransformConcat(t, CGAffineTransformMakeRotation(-M_PI_2));
            widthScale = viewSize.width / videoFrame.size.height;
            heightScale = viewSize.height / videoFrame.size.width;
            break;
    }
    
    // Adjust scaling to match video gravity mode of video preview
    if (videoGravity == AVLayerVideoGravityResizeAspect) {
        heightScale = MIN(heightScale, widthScale);
        widthScale = heightScale;
    }
    else if (videoGravity == AVLayerVideoGravityResizeAspectFill) {
        heightScale = MAX(heightScale, widthScale);
        widthScale = heightScale;
    }
    
    // Apply the scaling
    t = CGAffineTransformConcat(t, CGAffineTransformMakeScale(widthScale, heightScale));
    
    // Move origin back from center
    t = CGAffineTransformConcat(t, CGAffineTransformMakeTranslation(viewSize.width / 2.0f, viewSize.height / 2.0f));
    
    return t;
}

// MARK: Private methods
// Sets up the video capture session for the specified camera, quality and grayscale mode
//
//
// camera: -1 for default, 0 for back camera, 1 for front camera
// qualityPreset: [AVCaptureSession sessionPreset] value
// grayscale: YES to capture grayscale frames, NO to capture RGBA frames
//
- (BOOL)createCaptureSessionForCamera:(NSInteger)camera
                            grayscale:(BOOL)grayscale
{
    _lastFrameTimestamp = 0;
    _frameTimesIndex = 0;
    _captureQueueFps = 0.0f;
    _fps = 0.0f;
	
    // Set up AV capture
    NSArray* devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    
    if ([devices count] == 0) {
        NSLog(@"No video capture devices found");
        return NO;
    }
    
    if (camera == -1) {
        _camera = -1;
        _captureDevice =  [self backCamera];
    }
    else if (camera >= 0 && camera < [devices count]) {
        _camera = camera;
        _captureDevice = [self frontCamera];
    }
    else {
        _camera = -1;
        _captureDevice = [self backCamera];
        NSLog(@"Camera number out of range. Using default camera");
    }
    
    // Create the capture session
    _captureSession = [[AVCaptureSession alloc] init];
    _captureSession.sessionPreset = AVCaptureSessionPresetMedium;
    
    // Create device input
    NSError *error = nil;
    AVCaptureDeviceInput *input = [[AVCaptureDeviceInput alloc] initWithDevice:_captureDevice error:&error];
    
    // Create and configure device output
    _videoOutput = [[AVCaptureVideoDataOutput alloc] init];
    
    dispatch_queue_t queue = dispatch_queue_create("cameraQueue", NULL);
    [_videoOutput setSampleBufferDelegate:self queue:queue];
    
    _videoOutput.alwaysDiscardsLateVideoFrames = YES;
    _videoOutput.minFrameDuration = CMTimeMake(1, 30);
    
    
    // For grayscale mode, the luminance channel from the YUV fromat is used
    // For color mode, BGRA format is used
    OSType format = kCVPixelFormatType_32BGRA;
    
    // Check YUV format is available before selecting it (iPhone 3 does not support it)
    if (grayscale && [_videoOutput.availableVideoCVPixelFormatTypes containsObject:
                      [NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8BiPlanarFullRange]]) {
        format = kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;
    }
    
    _videoOutput.videoSettings = [NSDictionary dictionaryWithObject:[NSNumber numberWithUnsignedInt:format]
                                                             forKey:(id)kCVPixelBufferPixelFormatTypeKey];
    
    // Connect up inputs and outputs
    if ([_captureSession canAddInput:input]) {
        [_captureSession addInput:input];
    }
    
    if ([_captureSession canAddOutput:_videoOutput]) {
        [_captureSession addOutput:_videoOutput];
    }
    
    // Create the preview layer
    _videoPreviewLayer = [[AVCaptureVideoPreviewLayer alloc] initWithSession:_captureSession];
    [_videoPreviewLayer setFrame:self.view.bounds];
    _videoPreviewLayer.videoGravity = AVLayerVideoGravityResizeAspectFill;
    [self.view.layer insertSublayer:_videoPreviewLayer atIndex:0];
    
    return YES;
}


- (AVCaptureDevice *)frontCamera {
    NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    for (AVCaptureDevice *device in devices) {
        if ([device position] == AVCaptureDevicePositionFront) {
            return device;
        }
    }
    return [self backCamera];
}

- (AVCaptureDevice *)backCamera {
    NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    for (AVCaptureDevice *device in devices) {
        if ([device position] == AVCaptureDevicePositionBack) {
            return device;
        }
    }
    return nil;
}


// Tear down the video capture session
- (void)destroyCaptureSession
{
    [_captureSession stopRunning];
    
    [_videoPreviewLayer removeFromSuperlayer];
    
    _videoPreviewLayer = nil;
    _videoOutput = nil;
    _captureDevice = nil;
    _captureSession = nil;
}

- (void)updateDebugInfo {
    if (_fpsLabel) {
        _fpsLabel.text = [NSString stringWithFormat:@"FPS: %0.1f", _fps];
    }
}
@end