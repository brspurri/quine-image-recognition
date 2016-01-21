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
 * @file   QuineImageManager.h
 * @author Brett Spurrier (brett@quinevision.com)
 * @date   August, 2014
 * @brief  Class file for image comparisons with currently loaded databases.
 *
 */


#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>


/* ************************************************************************* */
/*!
 *  @class      QuineImageManager
 *
 *  @abstract   Manager class for handing operations on Quine Images.
 *
 *  @discussion This class contains methods for syncing databases from the server
 *              to the client as well as methods for loading databases into memory are
 *              also present. This class also provides a method to add a locally
 *              stored image to a new database.
 *
 *  @updated 2014-08-15
 */
@interface QuineImageManager : NSObject
typedef void (^SyncStartingBlock)(QuineImageManager *);
typedef void (^SyncCompletionBlock)(QuineImageManager *, NSError *);
typedef void (^SyncProgressBlock)(QuineImageManager *, CGFloat);
@property (nonatomic, copy) SyncStartingBlock syncSartingBlock;
@property (nonatomic, copy) SyncCompletionBlock syncCompletetionBlock;
@property (nonatomic, copy) SyncProgressBlock syncProgressBlock;


/* ************************************************************************* */
/*!
 *  @brief Initializes the QuineImageManager
 *
 *  Initializes a QuineImageManager instance using an inputted
 *  application api key.
 *
 *  @param key          User identifier key. See http://quinevision.com/me.
 *  @return             void
 */
-(id)initWithKey:(NSString *)key;


/* ************************************************************************* */
/*!
 *  @brief Connects to the server and syncs the users databases.
 *
 *  Connects to the server to validate the users permissions
 *  against the supplied key. Upon a successful handshake,
 *  the server and syncs the users databases in two steps:
 *
 *  1. Connect to the server and retrieve a list of endpoints (databases) to download.
 *
 *  2. Download and overwrite the current databases with the newly downloaded ones.
 *
 *  @return     void
 */
-(void)syncDatabasesToDevice;

-(void)syncTrackingToDeviceForImage:(NSString *)imageId;

-(void)syncAllImageMetadata;


/* ************************************************************************* */
/*!
 *  @brief Syncs an NSDictionary of Image metadata.
 *
 *  Given an Image ID, the method will sync a json document (gzipped)
 *  containing metadata for a given image ID. If metadata for the image id has
 *  previously been synced via this method or syncAllImageMetadata(), then
 *  this method will query the metadata from the server and update it locally.
 *
 *  Use this method (or syncAllImageMetadata) if you would like metadata
 *  to be available offline.
 *
 *  @param imageId      The ID associcated with the image.
 *  @return             void
 */
-(NSDictionary *)syncMetadataForImageId:(NSString *)imageId;


/* ************************************************************************* */
/*!
 *  @brief Returns an NSDictionary of Image metadata
 *
 *  Given an Image ID, the method will return an NSDictionary
 *  containing data for a given image ID. If metadata for the image id has 
 *  previously been synced via syncMetadataForImageId() or syncAllImageMetadata(), then
 *  this method will query the metadata from the server and store it locally.
 *
 *  Works in concert with syncMetadataForImageId: if no metadata has 
 *  yet been synced.
 *
 *  @param imageId      The ID associcated with the image.
 *  @return             NSDictionary of the metadata information for this image.
 */
-(NSDictionary *)getMetadataForImageId:(NSString *)imageId;


/* ************************************************************************* */
/*!
 *  @brief NOT YET IMPLEMENTED
 *
 *  Input two UIImages and obtain their matching value.
 *  NOTE: This is NOT YET IMPLEMENTED. Stay tuned for a future release.
 *
 *  @param firstImage   First UIImage object.
 *  @param secondImage  Second UIImage object.
 *  @return             float value of the match similarity.
 */
-(CGFloat)compareUIImage:(UIImage *)firstImage
             withUIImage:(UIImage *)secondImage;


/* ************************************************************************* */
/*!
 *  @brief Add a local image to a database.
 * 
 *  Allows a local image to be inserted into a database.
 *  However, this method will NOT sync back to the server.
 *  This method is intended for adding local images to a new (unsynced)
 *  database.
 *
 *  @param image        Image to add to the database.
 *  @param databaseName Name of the database where to add the image.
 *  @param metadata     Additional string information
 *  @return             void
 */
-(void)addImage:(UIImage *)image
     toDatabase:(NSString *)databaseName
   withMetadata:(NSString *)metadata;


/* ************************************************************************* */
/*!
 *  @brief Sets the verbosity for the current class.
 * 
 *  If true, then output will be written to the console.
 *  If false, all output will be suppressed.
 *
 *  @param verbose       Write output to the log console.
 *  @return             void
 */
-(void)setVerbose:(BOOL)verbose;


/* ************************************************************************* */
/*!
 *  @brief Loads a database from disk to memory.
 *
 *  If a database is already loaded the database is not re-loaded.
 *
 *  @param databaseName      Base name of database.
 *  @return                  void
 */
-(void)loadDatabase:(NSString *)databaseName;


/* ************************************************************************* */
/*!
 * @brief Loads all the databases associated with the current key.
 *
 * @return void
 */
-(void)loadAllDatabases;


/* ************************************************************************* */
/*!
 * @brief Lists the names of all the databases stored on the device.
 *
 * @return NSArray*
 */
-(NSArray *)listDatabases;


/* ************************************************************************* */
/*!
 *  @brief Removes a database by name
 *
 *  Deletes a database by name that is located in the applications 
 *  documentation folder.
 *
 * @return                  void
 */
-(void) removeDatabase:(NSString *)databaseName;


+(NSString *)getDatabasePathForDatabase:(NSString *)databaseName;
+(NSString *)getDatabasePathForDatabase:(NSString *)databaseName withExt:(NSString *)ext;
@end
