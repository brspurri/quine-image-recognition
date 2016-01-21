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
 * @file   QuineCompare.h
 * @Author Brett Spurrier (brett@quinevision.com)
 * @date   August, 2014
 * @brief  Class file for image comparisons with currently loaded databases.
 *
 */


#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>


/* ************************************************************************* */
/*!
 *  @class      QuineCompare
 *
 *  @abstract   Manager class for handing database searches on Quine Images.
 *
 *  @discussion This class contains methods for comparing query images
 *              with loaded databases, either previously synced from the server
 *              or generated on the local device.
 *
 *  @updated 2015-05-12
 */
@interface QuineCompare : NSObject


/* ************************************************************************* */
/*!
 *  @brief [NOT UPDATED. PLEASE IGNORE FOR NOW.]
 *  Compares a UIImage with the currently loaded databases.
 * 
 *  An input image is supplied and compared to a collection of loaded databases,
 *  which are loaded by:
 *
 *  @code 
 *    QuineImageManager *imageManager = [[QuineImageManager alloc] init];
 *    [imageManager loadDatabases];
 *  @endcode
 *
 *  or
 *
 *  @code
 *    QuineImageManager *imageManager = [[QuineImageManager alloc] init];
 *    [imageManager loadDatabase:@"MyDatabaseName"];
 *  @endcode
 * 
 *  @c compareUIImageToLoadedDatabase: resizes the input @c UIImage to an appropriate
 *  size to analyze. The image is deconstructed into a unique representative 
 *  signature which is compared to image signatures in the loaded databases.
 *  The return value is a key-value dictionary where the key is the database name
 *  and the corresponding value is the matched image id.
 *
 *  @a Usage:
 *  @code
 *     QuineCompare *imageCompareManager = [[QuineCompare alloc] init];
 *     NSDictionary *results = [imageCompareManager compareUIImageToLoadedDatabase:[UIImage imageNamed:@"queryImage.jpg"]];
 *  @endcode
 *
 *  @param query (@c UIImage*) Input image that will be compared against
 *                 the loaded databases.
 *
 *  @returns (@c NSDictionary*) of the matched @c ImageId for each loaded dictionary.
 *
 */
-(NSDictionary *)compareUIImageToLoadedDatabase:(UIImage *)query;


/* ************************************************************************* */
/*!
 *  @protected
 *  @brief Compares a cv::Mat with the currently loaded databases.
 *
 *  @param query (cv::Mat&) Reference to an input image loaded
 *               into a cv::Mat structure.
 *
 *  @returns (NSDictionary *) of the matched Image Id for each loaded dictionary.
 *
 */
-(NSDictionary *)compareMatToLoadedDatabase:(cv::Mat&)query;


/* ************************************************************************* */
/*!
 * @brief Sets the sensitivity of each feature match.
 *
 * @param sensitivity (CGFloat).
 *          Floating value in the range [0:1].
 *          Default (and recommended) is 0.96 (emperically determined).
 */
-(void)setSensitivity:(CGFloat)sensitivity;
@end





