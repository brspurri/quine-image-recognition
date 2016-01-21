//
//  QuineFeatureStruct.h
//  Quine
//
//  Created by Brett Spurrier on 2/1/14.
//  Copyright (c) 2014 Spurrier. All rights reserved.
//

#include <bitset>

#ifndef Quine_QuineFeatureStruct_h
#define Quine_QuineFeatureStruct_h

#define DESCRIPTOR_SIZE 256


/*!
 * Structure for a detected feature.
 */
typedef struct feature {
    
    
    /*!
     * ID of the feature
     */
    char feature_id[32];
    
    
    /*!
     * ID of the image the feature belongs to
     */
    char image_id[32];
    
    
    /*!
     * X position of the feature
     */
    int x;
    
    
    /*!
     * Y position of the feature
     */
    int y;
    
    
    /*!
     * Size position of the feature
     */
    int scale;
    
    
    /*!
     * Angle of the location of the feature from the center of the image.
     * Integer value from 0-360.
     */
    int angle;
    
    
    /*!
     * Lapplacian of the feature. Not set for all detectors.
     */
    bool laplacian;
    
    
    /*!
     * Bitset containg the descriptor information for the feature.
     */
    std::bitset<DESCRIPTOR_SIZE> descriptor;
    
   
    
} feature_t;


/*!
 * Structure for a database.
 */
typedef struct database {
    
    
    /*!
     * ID of the feature
     */
    char feature_id[32];
    
    
    /*!
     * ID of the image the feature belongs to
     */
    char image_id[32];
    
    
    /*!
     * X position of the feature
     */
    int x;
    
    
    /*!
     * Y position of the feature
     */
    int y;
    
    
    /*!
     * Size position of the feature
     */
    int scale;
    
    
    /*!
     * Angle of the location of the feature from the center of the image.
     * Integer value from 0-360.
     */
    int angle;
    
    
    /*!
     * Lapplacian of the feature. Not set for all detectors.
     */
    bool laplacian;
    
    
    /*!
     * Bitset containg the descriptor information for the feature.
     */
    std::bitset<DESCRIPTOR_SIZE> descriptor;
    
    
    
} database_t;

#endif
