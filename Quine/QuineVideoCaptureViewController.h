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
 * @file   QuineVideoCaptureViewController.h
 * @author Brett Spurrier (brett@quinevision.com)
 * @date   August, 2014
 * @brief  Class file for managing the camera and camera frame processing.
 *
 */

#import "VideoCaptureViewController.h"



/* ************************************************************************* */
/*!
 *  @class      QuineVideoCaptureViewController
 *
 *  @abstract   View Controller class for Quine Scanner.
 *
 *  @discussion QuineVideoCaptureViewController performs image processing
 *              on each camera frame in real-time as it is received by the 
 *              device. The processing stages image recognition and, or, 
 *              object tracking.
 *
 *  @updated 2014-08-15
 */
@interface QuineVideoCaptureViewController : VideoCaptureViewController { }
typedef void (^QuineScannerControllerResult)(QuineVideoCaptureViewController *, NSDictionary *);
@property (nonatomic, copy) QuineScannerControllerResult result;



/* ************************************************************************* */
/*!
 *  @brief Toggles the camera torch.
 *
 *  This toggle will turn the camera torch on and off.
 *  The method is exposed for all devices,  however, won't
 *  perform any actions for those devices with no camera torch.
 *
 *  @return void
 */
- (void)toggleTorch;



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
- (void)toggleCamera;



/* ************************************************************************* */
/*!
 *  @brief Enable image matching on the scanner.
 *
 *  This flag will enable (YES) or disable (NO) the processing of images 
 *  frames returned from QuineVideoCaptureViewController. This methd is best
 *  used to stop the camera frames from processing after an image has been
 *  successfully matched and the code cycle is performing downstream tasks.
 *
 *  <em>Usage:</em>
 *  <pre>
 *  @textblock
 *  QuineVideoCaptureViewController *vc = [[QuineVideoCaptureViewController alloc] init];
 *  [vc setScanningEnabled:YES];
 *  @/textblock
 *  </pre>
 *
 *  @param enabled (BOOL)
 *        Speficied whether the camera frame processing is enabled (YES) 
 *        or disabled (NO).
 *
 *  @return void
 */
- (void)setScanningEnabled:(BOOL)enabled;



/* ************************************************************************* */
/*!
 * @brief Sets the verbosity for the current class. If true, then
 *        output will be written to the console. If false, all output
 *        will be suppressed.
 *
 * @param verbose (BOOL)
 *        Specifies whether output should be written to the log console.
 *
 * @return (void)
 */
-(void)setOverlayView:(UIView *)overlayView;

@end