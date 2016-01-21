//
//  QuineConstants.h
//  Quine
//
//  Created by Brett Spurrier on 1/30/14.
//  Copyright (c) 2014 Spurrier. All rights reserved.
//

#ifndef Quine_QuineConstants_h
#define Quine_QuineConstants_h

#define RESIZED_IMAGE_WIDTH 200

#define IMAGE_MAX_DESC_INT   200
#define IMAGE_MAX_KPTS       40
#define BIT_SIZE             IMAGE_MAX_DESC_INT*256

//Datbase row lengths
#define IMAGE_ID_SIZE_BYTES         32
#define IMAGE_HASH_BYTES            32
#define FEATURE_ID_SIZE_BYTES       0
#define FEATURE_X_SIZE_BYTES        sizeof(int)
#define FEATURE_Y_SIZE_BYTES        sizeof(int)
#define FEATURE_SCALE_SIZE_BYTES    sizeof(int)
#define FEATURE_ANGLE_SIZE_BYTES    sizeof(int)
#define FEATURE_LAP_SIZE_BYTES      sizeof(int)
#define FEATURE_DESC_SIZE_BYTES     32
#define FEATURE_DESC_SIZE_BITS      FEATURE_DESC_SIZE_BYTES * 8
#define FEATURE_ROW_SIZE            IMAGE_ID_SIZE_BYTES + IMAGE_HASH_BYTES + FEATURE_ID_SIZE_BYTES + \
                                    FEATURE_X_SIZE_BYTES + FEATURE_Y_SIZE_BYTES + FEATURE_SCALE_SIZE_BYTES + \
                                    FEATURE_ANGLE_SIZE_BYTES + FEATURE_LAP_SIZE_BYTES + FEATURE_DESC_SIZE_BYTES


#define IMAGE_DESC_ALL_BYTES IMAGE_DESC_BYTES * IMAGE_MAX_KPTS
#define ROW_SIZE_BYTES       IMAGE_ID_SIZE_BYTES + IMAGE_HASH_BYTES + IMAGE_DESC_ALL_BYTES

//ORB Constants
const int ORB_MAX_KPTS = IMAGE_MAX_DESC_INT;
const float ORB_SCALE_FACTOR = 1.5;
const int ORB_PYRAMID_LEVELS = 3;
const float ORB_EDGE_THRESHOLD = 31.0;
const int ORB_FIRST_PYRAMID_LEVEL = 0;
const int ORB_WTA_K = 2;
const int ORB_PATCH_SIZE = 31;
const int GRID_SIZE = 1;


//Matching
#define FEATURE_MATCH_THRESHOLD 50
#define QUERY_FEATURES_MAX_TO_MATCH 10

//Databa
#endif
