//
//  QuineDatabaseManager.h
//  Quine
//
//  Created by Brett Spurrier  on 5/7/15.
//  Copyright (c) 2015 Spurrier. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface QuineDatabaseManager : NSObject
-(NSArray *)listDatabases;
-(void)setVerbose:(BOOL)verbose;
-(void)loadDatabase:(NSString *)databaseName;
-(NSArray *)listLoadedImagesForDatabase:(NSString *)databaseName;
@end
