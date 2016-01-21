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

#import "QuineImageManager.h"
#import <Accelerate/Accelerate.h>
#import <CommonCrypto/CommonDigest.h>
#include <unordered_map>
#include <algorithm>
#import "QuineVideoCaptureViewController.h"
#include "QuineDatabaseOperations.h"
#include "QuineFeatureDetection.h"
#include "QuineDictionary.h"
#include "QuineConstants.h"
#include "QuineFeatureStruct.h"
#import "QuineCompare.h"


/* ************************************************************************* */
/*!
 *  API Endpoint (Constant)
 *  May parameterize this for production.
 */
NSString *const kApiEndpoint = @"https://quine-api.azurewebsites.net";


/* ************************************************************************* */
/*!
 *  Interface (Private)
 */
@interface QuineImageManager () {
    BOOL _verbose;
    BOOL _reachable;
    NSString *_key;
}

// Objectove-C methods
- (NSString *)imageHash:(UIImage *)image;
- (BOOL)isApiReachable;

// C-methods
void createMatFromUIImage(const UIImage* image, cv::Mat& m, bool alphaExist);
@end


/* ************************************************************************* */
/*!
 *  Implementation
 */
@implementation QuineImageManager


/* ************************************************************************* */
/*!
 *  Initialization
 *
 *  @return self
 */
-(id)initWithKey:(NSString *)key {
    self = [super init];
    if(self) {
        _key = key;
        _reachable = [self isApiReachable];
    }
    return self;
}


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
-(void)setVerbose:(BOOL)verbose {
    _verbose = verbose;
}


/* ************************************************************************* */
/*!
 * @brief Connects to the server to validate the users permissions 
 *        against the supplied key. Upon a successful handshake, this method
 *        syncs the users databases to the client in two steps.
 *
 *        (1) Connect to the server and retrieve a list
 *        of endpoints (databases) to download.
 *
 *        (2) Download and overwrite the current databases
 *        with the newly downloaded ones.
 *
 * @return (void)
 */
-(void)syncDatabasesToDevice {
    
    
    ////////////////////////////////////////////////////////
    // STEP ONE
    
    // Send the sync starting block call
    if (self.syncSartingBlock) {
        self.syncSartingBlock(self);
    }
    
    
    /////////////////////////////////////////////////////////
    // Is the API reachable?
    
    if(!_reachable) {
        NSDictionary *userInfo = @{
                                   NSLocalizedDescriptionKey: NSLocalizedString(@"API was unreachable unsuccessful.", nil),
                                   NSLocalizedFailureReasonErrorKey: NSLocalizedString(@"API was unreachable unsuccessful.", nil),
                                   NSLocalizedRecoverySuggestionErrorKey: NSLocalizedString(@"Are you connected to the internet?", nil)
                                   };
        NSError *reachibilityError = [NSError errorWithDomain:@"com.quinevision" code:100 userInfo:userInfo];
        if (self.syncCompletetionBlock) {
            self.syncCompletetionBlock(self, reachibilityError);
        }
        
        if(_verbose) {
            NSLog(@"[Syncing ERROR]: Not reachable. No internet connection? Falling back to previously synced versions of your image databases.");
            return;
        }
    }
    
    
    /////////////////////////////////////////////////////////
    // Call the syncing endpoint
    //   Should this be quinevision.com?

    NSString *syncEndpoint = [NSString stringWithFormat:@"%@/api/database/connect", kApiEndpoint];
    if(_verbose) {
        NSLog(@"[Syncing STARTING]: %@", syncEndpoint);
    }
    
    NSMutableURLRequest *request = [[NSMutableURLRequest alloc] initWithURL:[NSURL URLWithString:syncEndpoint]];
    [request setHTTPMethod:@"POST"];
    
    NSString *auth = [NSString stringWithFormat:@"bearer %@", _key];
    [request setValue:auth forHTTPHeaderField:@"Authorization"];
    
    [NSURLConnection sendAsynchronousRequest:request
                                       queue:[NSOperationQueue mainQueue]
                           completionHandler:^(NSURLResponse *response, NSData *data, NSError *error) {
                               
                               NSError *jsonError = nil;
                               NSDictionary *JSON = [NSJSONSerialization
                                                     JSONObjectWithData: data
                                                     options: NSJSONReadingMutableContainers
                                                     error: &jsonError];
                               
                               
                               /////////////////////////////////////////////////////////
                               // STEP TWO
                               
                               NSOperationQueue *queue = [NSOperationQueue mainQueue];
                               [queue setMaxConcurrentOperationCount:1];
                               
                               for(id key in JSON) {
                                   if(![[[key lastPathComponent] pathExtension] isEqualToString:@"bin"]) {
                                       continue;
                                   }
                                   
                                   // Requires that the dictionary returned is of the format:
                                   // {
                                   //    md5: ""
                                   //    url: ""
                                   // }
                                   NSURLRequest *req = [NSURLRequest requestWithURL:[NSURL URLWithString:[[JSON objectForKey:key] objectForKey:@"url"]]];
                                   if(_verbose) {
                                       NSLog(@"[Syncing STARTED]: %@", key);
                                   }
                                   [NSURLConnection sendAsynchronousRequest:req
                                                                      queue:queue
                                                          completionHandler:^(NSURLResponse *response, NSData *data, NSError *error) {

                                                              NSString *downloadedDatabaseName = [[[response URL] path] lastPathComponent];
                                                              
                                                              if(!error) {
                                                                  
                                                                  NSError *writeError = nil;
                                                                  //Save the GZipped compressed file
                                                                  NSString *compressedFilenamePath = [QuineImageManager getDatabasePathForDatabase:downloadedDatabaseName withExt:@""];
                                                                  [data writeToFile:compressedFilenamePath options:NSDataWritingAtomic error:&writeError];
                                                                  
                                                                  if(writeError) {
                                                                      if(_verbose) {
                                                                          NSLog(@"[Syncing ERROR]: %@\n", [writeError description]);
                                                                      }
                                                                  }
                                                                  else {
                                                                      if(_verbose) {
                                                                          NSLog(@"[Syncing COMPLETE]\n");
                                                                      }
                                                                  }
                                                              }
                                                              else {
                                                                  if(_verbose) {
                                                                      NSLog(@"[Syncing ERROR]: %@\n", [error description]);
                                                                  }
                                                              }
                                                              
                                                              // Call the completion block
                                                              if (self.syncCompletetionBlock) {
                                                                  self.syncCompletetionBlock(self, error);
                                                              }
                                                              
                                                          }];
                                }

                           }];
}


#pragma mark -
#pragma mark Sync - Tracking methods
/* ************************************************************************* */
/*!
 * @brief Syncs the users tracking data for
 *        an array of image ids.
 *
 * @return (void)
 */

-(void)syncTrackingToDeviceForImages:(NSArray *)imageIds {
    
    
    NSOperationQueue *queue = [NSOperationQueue mainQueue];
    [queue setMaxConcurrentOperationCount:1];
    
    NSString *syncEndpoint = [NSString stringWithFormat:@"%@/api/database/tracking/connect", kApiEndpoint];
    if(_verbose) {
        NSLog(@"[Sync Tracking STARTING]: %@", syncEndpoint);
    }
    
    NSError *error = nil;
    NSData *json;
    if ([NSJSONSerialization isValidJSONObject:imageIds]) {
        json = [NSJSONSerialization dataWithJSONObject:imageIds options:NSJSONWritingPrettyPrinted error:&error];
        
        if (json != nil && error == nil) {
            NSString *jsonString = [[NSString alloc] initWithData:json encoding:NSUTF8StringEncoding];
            
            NSMutableURLRequest *request = [[NSMutableURLRequest alloc] initWithURL:[NSURL URLWithString:syncEndpoint]];
            [request setHTTPMethod:@"POST"];
            
            [request setValue:[NSString stringWithFormat:@"%lu", (unsigned long)[jsonString length]] forHTTPHeaderField:@"Content-length"];
            [request setHTTPBody:[jsonString dataUsingEncoding:NSUTF8StringEncoding]];

            [NSURLConnection sendAsynchronousRequest:request
                                               queue:[NSOperationQueue mainQueue]
                                   completionHandler:^(NSURLResponse *response, NSData *data, NSError *error) {
                                       
                                       NSError *jsonError = nil;
                                       NSDictionary *json = [NSJSONSerialization
                                                             JSONObjectWithData: data
                                                             options: NSJSONReadingMutableContainers
                                                             error: &jsonError];

                                       NSLog(@"Tracking:\n%@\n", json);
                                       
                                   }];
            
            
        }
    }
    

    
    
}

-(void)syncTrackingToDeviceForImage:(NSString *)imageId {
    return [self syncTrackingToDeviceForImages:[NSArray arrayWithObject:imageId]];
}



/* ************************************************************************* */
/*!
 * @brief Loads all the databases associated with the current key.
 *
 * @return (void)
 */
-(void)loadAllDatabases {
    for (NSString *db in [self listDatabases]) {
        [self loadDatabase:db];
    }
}



/* ************************************************************************* */
/*!
 *  @brief Compares one UIImage to another UIImage
 *
 *  @param firstImage First input image
 *  @param secondImage Second input image
 */
-(CGFloat)compareUIImage:(UIImage *)firstImage
             withUIImage:(UIImage *)secondImage {
    
    NSException *up = [NSException exceptionWithName:@"NotImplmented"
                                             reason:@"Not yet implemented"
                                           userInfo:nil];
    @throw up;
}


/* ************************************************************************* */
/*!
 *  @brief Add a local image to a database.
 *
 *  Allows a local image to be inserted into a database.
 *  However, this method will \b NOT sync back to the server.
 *  This method is intended for adding local images to a new (unsynced)
 *  database.
 *
 *  @param image        Image to add to the database.
 *  @param databaseName Name of the database where to add the image.
 *  @param metadata     Additional string information
 *  @return void
 */
-(void)addImage:(UIImage *)image
     toDatabase:(NSString *)databaseName
      withMetadata:(NSString *)metadata {
    
    //Compute the hash of the image
    NSString *hash = [self imageHash:image];
    
    //Compute the image id of the image
    NSString *imageId = [self generateImageId];
    
    // Convert the image from a UIImage to cv::Mat
    cv::Mat mat;
    createMatFromUIImage(image, mat, true);
    
    //Add the image to the database
    NSString *databasePath = [QuineImageManager getDatabasePathForDatabase:databaseName withExt:@"bin"];
    QuineDatabaseOperations database_op = QuineDatabaseOperations();
    
    // Add the image to the database
    database_op.add_image(mat,
                          [hash cStringUsingEncoding: NSASCIIStringEncoding],
                          [imageId cStringUsingEncoding: NSASCIIStringEncoding],
                          [metadata cStringUsingEncoding: NSASCIIStringEncoding],
                          [databasePath cStringUsingEncoding: NSASCIIStringEncoding]);
    
    //Cleanup
    mat.release();
    database_op.~QuineDatabaseOperations();
    
}


#pragma mark -
#pragma Private internal methods
/* ************************************************************************* */
/**
 *  @brief Convert UIImage to cv::Mat
 *
 *  Converts an input UIImage to a cv::Mat structure.
 * 
 *  @b NOTE: This method is repeated verbatim in QuineCompare.
 *  However, we don't want the method to be exposed in the framework,
 *  so the code is repeated. If anyone has a better way, I'm all ears.
 *
 *  @param image UIImage input
 *  @return cv::Mat
 */

void createMatFromUIImage(const UIImage* image, cv::Mat& m, bool alphaExist) {
    
    CGColorSpaceRef colorSpace = CGImageGetColorSpace(image.CGImage);
    CGFloat cols = image.size.width, rows = image.size.height;
    CGContextRef contextRef;
    CGBitmapInfo bitmapInfo = kCGImageAlphaPremultipliedLast;
    if (CGColorSpaceGetModel(colorSpace) == 0)
    {
        m.create(rows, cols, CV_8UC1); // 8 bits per component, 1 channel
        bitmapInfo = kCGImageAlphaNone;
        if (!alphaExist)
            bitmapInfo = kCGImageAlphaNone;
        contextRef = CGBitmapContextCreate(m.data, m.cols, m.rows, 8,
                                           m.step[0], colorSpace,
                                           bitmapInfo);
    }
    else
    {
        m.create(rows, cols, CV_8UC4); // 8 bits per component, 4 channels
        if (!alphaExist)
            bitmapInfo = kCGImageAlphaNoneSkipLast |
            kCGBitmapByteOrderDefault;
        contextRef = CGBitmapContextCreate(m.data, m.cols, m.rows, 8,
                                           m.step[0], colorSpace,
                                           bitmapInfo);
    }
    CGContextDrawImage(contextRef, CGRectMake(0, 0, cols, rows),
                       image.CGImage);
    CGContextRelease(contextRef);

}


-(void)removeDatabase:(NSString *)databaseName {
    
    NSString *filePath = [QuineImageManager getDatabasePathForDatabase:databaseName withExt:@"yaml"];
    NSError *error;
    BOOL success =[[NSFileManager defaultManager] removeItemAtPath:filePath error:&error];
    if (success) {
        NSLog(@"Database removed\n");
    }
    else {
        NSLog(@"Could not delete file -:%@ ",[error localizedDescription]);
    }
    
}

+(NSString *)getDatabasePathForDatabase:(NSString *)databaseName {
    return [self getDatabasePathForDatabase:databaseName withExt:@"bin"];
}

+(NSString *)getDatabasePathForDatabase:(NSString *)databaseName withExt:(NSString *)ext {
    
    NSArray *dirPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *docs = [dirPaths objectAtIndex:0];
    NSString *databaseFilename;
    if ([ext isEqualToString:@""]) {
        databaseFilename = [docs stringByAppendingPathComponent:databaseName];
    }
    else {
        databaseFilename = [docs stringByAppendingPathComponent:[NSString stringWithFormat:@"%@.%@", databaseName, ext]];
    }
    
    return databaseFilename;
}



/* ************************************************************************* */
/*!
 * @brief Generates a hash value for a UIImage.
 *        This is used so that duplicate (identical) images are not
 *        added to the same database
 *
 * @param image (UIImage *)
 *        The image to be hased
 *
 * @return (NSSring *)
 */
-(NSString *)imageHash:(UIImage *)image {
    unsigned char result[16];
    NSData *imageData = [NSData dataWithData:UIImagePNGRepresentation(image)];
    CC_MD5([imageData bytes], [imageData length], result);
    NSString *hash = [NSString stringWithFormat:
                           @"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
                           result[0], result[1], result[2], result[3],
                           result[4], result[5], result[6], result[7],
                           result[8], result[9], result[10], result[11],
                           result[12], result[13], result[14], result[15]
                           ];
    
    //NSLog(@"Hash: %@\n", hash);
    return hash;
}


/* ************************************************************************* */
/*!
 * @brief Generates a unique GUID to be used as the image ID
 *
 * @return (NSSring *)
 */
-(NSString *)generateImageId {
    
    CFUUIDRef guid = CFUUIDCreate(NULL);
    CFStringRef string = CFUUIDCreateString(NULL, guid);
    CFRelease(guid);
    return (__bridge NSString *)string;
    
}


/* ************************************************************************* */
/*!
 * @brief Check for a current internet connection
 *
 * @return (BOOL)
 */
- (BOOL)isApiReachable {
    
    NSURL *url = [NSURL URLWithString:[NSString stringWithFormat:@"%@/api/reachable/connected", kApiEndpoint]];
    NSData *data = [NSData dataWithContentsOfURL:url];
    NSString *str = [[NSString alloc] initWithData:data encoding:NSASCIIStringEncoding];
    if (str && [str isEqualToString:@"\"Reachable OK\""]) {
        return YES;
    }
    else {
        return NO;
    }
}


-(void)dealloc {
    
    /*
    for (auto i:_databases.keys()) {
        std::cout << "Deallocating database: " << i << std::endl;
        _databases.pop(i);
    }
     */
}

@end
