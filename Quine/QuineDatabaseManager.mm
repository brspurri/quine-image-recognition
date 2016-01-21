//
//  QuineDatabaseManager.m
//  Quine
//
//  Created by Brett Spurrier  on 5/7/15.
//  Copyright (c) 2015 Spurrier. All rights reserved.
//

#import "QuineDatabaseManager.h"
#import "QuineImageManager.h"

#include "QuineDatabaseOperations.h"

@interface QuineDatabaseManager () {
    BOOL _verbose;
}
-(void)loadDatabase:(NSString *)databaseName forceReload:(BOOL)force;

@end

@implementation QuineDatabaseManager

/* ************************************************************************* */
/*!
 *  Initialization
 *
 *  @return self
 */
-(id)init {
    self = [super init];
    if(self) {
    }
    return self;
}

-(void)setVerbose:(BOOL)verbose {
    _verbose = verbose;
}


/* ************************************************************************* */
/*!
 * @brief Lists the names of all the databases stored on the device.
 *
 * @return (NSArray *)
 */
-(NSArray *)listDatabases {
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    NSArray *filenames = [[NSFileManager defaultManager] subpathsOfDirectoryAtPath:documentsDirectory error:nil];
    
    NSMutableArray *databaseNames = [[NSMutableArray alloc] initWithCapacity:[filenames count]];
    for (NSString *filename in filenames) {
        [databaseNames addObject:[filename stringByDeletingPathExtension]];
    }
    return databaseNames;
}


/* ************************************************************************* */
/*!
 * @brief Loads a database into memory
 *
 * @return (void)
 */
-(void)loadDatabase:(NSString *)databaseName forceReload:(BOOL)force {
    
    // Get the full path of the database
    NSString *databasePath = [QuineImageManager getDatabasePathForDatabase:databaseName withExt:@"bin"];
    
    //Load the database into memory. Appends it to the list of databases if it isn't loaded already
    QuineDatabaseOperations database_op = QuineDatabaseOperations();
    database_op.load_database([databasePath cStringUsingEncoding: NSASCIIStringEncoding], force);
    database_op.~QuineDatabaseOperations();
    
    if(_verbose) {
        NSLog(@"[Load COMPLETE]: %@\n", databaseName);
    }
}

-(void)loadDatabase:(NSString *)databaseName {
    [self loadDatabase:databaseName forceReload:NO];
}


-(NSArray *)listLoadedImagesForDatabase:(NSString *)databaseName {
    
    // Get the full path of the database
    NSString *databasePath = [QuineImageManager getDatabasePathForDatabase:databaseName withExt:@"bin"];
    
    std::vector<std::string> images;
    QuineDatabaseOperations database_op = QuineDatabaseOperations();
    database_op.list_images([databasePath cStringUsingEncoding: NSASCIIStringEncoding], images);
    database_op.~QuineDatabaseOperations();
    
    NSMutableArray *imageArray = [[NSMutableArray alloc] initWithCapacity:images.size()];
    std::for_each(images.begin(), images.end(), ^(std::string str) {
        id nsstr = [NSString stringWithUTF8String:str.c_str()];
        [imageArray addObject:nsstr];
    });
    if(_verbose) {
        for(NSString *i in imageArray) {
            NSLog(@"[Image LIST]: %@\n", i);
        }
    }
    
    return imageArray;
}

@end
