//
//  QuineCompare.m
//  Quine
//
//  Created by Brett Spurrier on 4/18/14.
//  Copyright (c) 2015 Spurrier. All rights reserved.
//

// Objective-C imports
#import "QuineCompare.h"
#import <Accelerate/Accelerate.h>

// C++ includes
#include "QuineDatabaseOperations.h"
#include "QuineFeatureDetection.h"

#pragma mark -
#pragma mark Pre-processor
#define USE_FILTER 1
#define MATCH_WINDOW_LENGTH 5


#pragma mark -
#pragma mark Interface (Private)
/* ************************************************************************* */
/*!
 *  Interface (Private)
 */
@interface QuineCompare () {
    CGFloat _dratio;
    CGFloat _acceptRatio;
    NSInteger _windowPosition;
    NSMutableArray *_matchWindow;
}
@end


#pragma mark -
#pragma mark Implementation
/* ************************************************************************* */
/*!
 *  Implementation
 */
@implementation QuineCompare


#pragma mark -
#pragma mark Initialization
/* ************************************************************************* */
/*!
 *  Initializtion
 */
- (id)init {
    self = [super init];
    if (self) {
        _dratio = 0.96;
        _acceptRatio = 0.10;
        
        _windowPosition = 0;
        _matchWindow = [[NSMutableArray alloc] initWithObjects:@"", @"", @"", @"", @"", nil];
    }
    return self;
}


#pragma mark -
#pragma mark Conversion methods
/* ************************************************************************* */
/*!
 *  @brief Conversion methods
 */
-(cv::Mat)createMatFromUIImage:(UIImage *)image {
    CGColorSpaceRef colorSpace = CGImageGetColorSpace(image.CGImage);
    CGFloat cols = image.size.width;
    CGFloat rows = image.size.height;
    
    cv::Mat mat(rows, cols, CV_8UC4); // 8 bits per component, 4 channels (color channels + alpha)
    
    CGContextRef contextRef = CGBitmapContextCreate(mat.data,                   // Pointer to  data
                                                    cols,                       // Width of bitmap
                                                    rows,                       // Height of bitmap
                                                    8,                          // Bits per component
                                                    mat.step[0],                // Bytes per row
                                                    colorSpace,                 // Colorspace
                                                    kCGImageAlphaNoneSkipLast |
                                                    kCGBitmapByteOrderDefault); // Bitmap info flags
    
    CGContextDrawImage(contextRef, CGRectMake(0, 0, cols, rows), image.CGImage);
    CGContextRelease(contextRef);
    
    return mat;
}


#pragma mark -
#pragma mark Comparison methods
/* ************************************************************************* */
/*!
 *  @brief Comparison methods
 */
-(NSDictionary *)compareUIImageToLoadedDatabase:(UIImage *)query {
    cv::Mat img = [self createMatFromUIImage:query];
    return [self compareMatToLoadedDatabase:img];
}


/* ************************************************************************* */
/*!
 *  @brief Comparisons a image to a collection of loaded databases.
 *
 *  @details This method is the main interface between the camera feed
 *           and the matched image result.
 */
-(NSDictionary *)compareMatToLoadedDatabase:(cv::Mat&)query {
    
    
    ////////////////////////////////////////////////////////////
    // Initialize the C++ feature detection and database class
    
    QuineFeatureDetection feature = QuineFeatureDetection();
    QuineDatabaseOperations database_op = QuineDatabaseOperations();
    
    
    ////////////////////////////////////////////////////////////
    // Resize and graysale the image
    
    cv::Mat resized_img, gray_img;
    feature.resize_to_width(query, resized_img, RESIZED_IMAGE_WIDTH);
    feature.get_gray(resized_img, gray_img);
    
    
    ////////////////////////////////////////////////////////////
    // Describe the query image
    
    akaze_response_struc result_img;
    feature.compute_signature(gray_img, result_img, true);
    
    
    ////////////////////////////////////////////////////////////
    // List the currently loaded databases, and initalize
    //   the results dictionary to match the number of dictionaries.
    
    std::vector<std::string> loaded_databases = database_op.list_loaded_databases();
    std::string matched_image_meta = "";
    NSString* databaseName = @"";
    NSMutableDictionary *resultsDictionary = [[NSMutableDictionary alloc] initWithCapacity:loaded_databases.size()];
    
    
    ////////////////////////////////////////////////////////////
    // Perform a match if more than zero keypoints are returned.
    //   Otherwise proceed to return an empty dictionary.
    
    if(result_img.kpts.size() > 0) {
    
        
        //////////////////////////////////////////////////
        // Loop through all the loaded databases and
        //   compare the query image
        
        for(auto db:loaded_databases){
            
            
            //////////////////////////////////////////////
            // Declare the std::set to hold the results
            
            std::set<int> results;
            
            
            //////////////////////////////////////////////
            // Get an in-memory reference to the database
            //   Note: hashtable is unused at this time.
            
            cv::Mat source, filter;
            std::vector<std::string> metadata, hashtable;
            database_op.get_database(db, source, filter, metadata, hashtable, false);
            
            
            //////////////////////////////////////////////
            // Compare the images

            matched_image_meta = compare_mat_souces(result_img.desc,
                                                    result_img.kpts_count,
                                                    source,
                                                    result_img.filter,
                                                    filter,
                                                    metadata,
                                                    results,
                                                    _dratio,
                                                    _acceptRatio);
            
            
            //////////////////////////////////////////////
            // Add the match result to the sliding window
            // Can proly be improved and optimized.
            
            NSString *match = [NSString stringWithCString:matched_image_meta.c_str() encoding:[NSString defaultCStringEncoding]];
            [_matchWindow replaceObjectAtIndex:_windowPosition withObject:match];
            
            _windowPosition++;
            if(_windowPosition >= MATCH_WINDOW_LENGTH) {
                _windowPosition = 0;
            }
            
            NSCountedSet *set = [[NSCountedSet alloc] initWithArray:_matchWindow];
            NSArray *sortedValues = [[set allObjects] sortedArrayUsingComparator:^(id obj1, id obj2) {
                NSUInteger n = [set countForObject:obj1];
                NSUInteger m = [set countForObject:obj2];
                return (n <= m)? (n < m)? NSOrderedAscending : NSOrderedSame : NSOrderedDescending;
            }];

            // Minimum of 2(?) of the MATCH_WINDOW_LENGTH frames need to match
            //   in order to be added to the result dictionary
            if([set countForObject:[sortedValues objectAtIndex:0]] < 2) {
                match = @"";
            }
            
            
            //////////////////////////////////////////////
            // Update the results dictionary
            
            databaseName = [self getDatabaseNameFromPathWithCString:db.c_str()];
            [resultsDictionary setObject:[NSString stringWithCString:matched_image_meta.c_str()
                                                            encoding:[NSString defaultCStringEncoding]]
                                  forKey:databaseName];
        }
    }
    
    
    //////////////////////////////////////////////////
    // Cleanup
    
    feature.~QuineFeatureDetection();
    database_op.~QuineDatabaseOperations();
    
    
    //////////////////////////////////////////////////
    // Return
    
    return resultsDictionary;
    
}


#pragma mark -
#pragma mark Helper methods
/* ************************************************************************* */
/*!
 *  @brief Helper methods
 */
-(NSString *)getDatabaseNameFromPathWithCString:(const char *)path {
    return [[[[[NSString stringWithCString:path
                                  encoding:[NSString defaultCStringEncoding]] lastPathComponent]
              stringByDeletingPathExtension] lastPathComponent]
            stringByDeletingPathExtension];
}



/* ************************************************************************* */
/*!
 *  @brief Template methods
 */


#pragma mark -
#pragma mark Class accessors
/* ************************************************************************* */
/*!
 * @brief Sets the sensitivity of each feature match.
 *
 * @param sensitivity (CGFloat).
 *          Floating value in the range [0:1]. 
 *          Default (and recommended) is 0.96 (emperically determined).
 */
-(void)setSensitivity:(CGFloat)sensitivity {
    _dratio = sensitivity;
}


#pragma mark -
#pragma mark Comparison functions
/* ************************************************************************* */
/*!
 * @brief Compares a set of query descriptors to a set of source descriptors.
 *          In an ideal world, this method will be completed stupidly fast.
 *          It's getting there.
 *
 * @param query (const cv::Mat) <CV_32F>
 *          Input matrix contining the query descriptors.
 *          This should only represent a single image.
 *
 * @param sources (const cv::Mat) <CV_32F>
 *          Input matrix contining the source descriptors for a set of images.
 *          Most likely, this set will be a full database of descriptors.
 *          Benchmarking has shown good performance on an iPhone 5 for 1000 images.
 *
 * @param query_filter (const cv::Mat) <CV_8U>
 *          Class Id data for each feature in source. The size will be query.rows() x 1 (i.e., vertical).
 *          This data will be matrix multiplied with source_filter in order to create
 *          a complete filter matrix.
 *
 * @param source_filter (const cv::Mat) <CV_8U>
 *          Class Id data for each feature in source. The size will be 1 x sources.rows() (i.e., horizontal).
 *          This data will be matrix multiplied with query_filter in order to create
 *          a complete filter matrix.
 *
 *          The resulting filter matrix will contain information for the intersection of feature class when 
 *          normalized against an individual class id.
 *
 * @param metadata (const v::vector<std::string>)
 *          Input matrix contining the source descriptors for a set of images.
 *          Most likely, this set will be a full database of descriptors.
 *          Benchmarking has shown good performance on an iPhone 5 for 1000 images.
 *
 * @param results_idxs (std::set) <int>
 *          A counted set of indicies matching the image.
 *          The highest count within the set should correspond to the index
 *          of the matched image.
 *
 * @param dratio (const float)
 *          Float value for the threshold percentage of a matched feature.
 *
 * @param accept_ratio (const float)
 *          Float value for the threshold matched feature percentage for an accepted image match.
 *
 * @return (void)
 */
std::string compare_mat_souces(const cv::Mat &query,
                               const int query_count,
                               const cv::Mat &source,
                               const cv::Mat &query_filter,
                               const cv::Mat &source_filter,
                               const cv::vector<std::string> &metadata,
                               std::set<int> &results_idxs,
                               const float dratio,
                               const float accept_ratio) {
    
    
    //////////////////////////////////////////////////////////
    // Declare the time variables
    
    double t1 = 0.0;
    double t2 = 0.0;
    
    
    //////////////////////////////////////////////////////////
    // Start the clock
    
    t1 = cv::getTickCount();
    
    
    //////////////////////////////////////////////////////////
    // Length of data matrix -> as (float *) vectors
    
    //size_t a_length = result_query.desc.cols * result_query.desc.rows;
    
    
    //////////////////////////////////////////////////////////
    // Transpose Matrix B (and the Filter-Matrix B
    
    float *matrixB_o = (float *)source.data;
    float *matrixB = new float[source.rows * source.cols]();
    vDSP_mtrans(matrixB_o, 1, matrixB, 1, source.cols, source.rows);
    
    
    //////////////////////////////////////////////////////////
    // A • B
    // Perform the comparison by multiplication (the magic)
    
    size_t len = source.rows * query.rows;
    float *matrixA = (float *)query.data;
    float *matrixAB = new float[len]();
    vDSP_mmul(matrixA, 1, matrixB, 1, matrixAB, 1, query.rows, source.rows, query.cols);
    
    
    //////////////////////////////////////////////////////////
    // Filter according to keypoint class
    
    // Build the filter stack
    size_t filter_len = source_filter.rows * source_filter.cols;
    float *matrix_query_filter = new float[filter_len]();
    float *matrix_source_filter = new float[filter_len]();
    
    uint8_t *uc_source_filter = (uint8_t *)source_filter.data;
    uint8_t *uc_query_filter = (uint8_t *)query_filter.data;
    
    std::transform(uc_source_filter, uc_source_filter + filter_len, matrix_source_filter, static_cast_func<float>());
    std::transform(uc_query_filter, uc_query_filter + filter_len, matrix_query_filter, static_cast_func<float>());
    
    float *matrix_filter = new float[len]();
    
    // Laplacian-filter generation step.
    //  1. Add +1 to both query and source. This allows for 0*0 to not be interpreted as a non-match.
    //  2. Matrix multiplication between query and source
    //  3. Values of +4 (2*2) and +1 (1*1) are matching values. All others (only +2 is possible) are mismatches.
    float offset = 1.0f;
    float *matrix_query_filter_offset = new float[len]();
    float *matrix_source_filter_offset = new float[len]();
    vDSP_vsadd(matrix_query_filter, 1, &offset, matrix_query_filter_offset, 1, len);
    vDSP_vsadd(matrix_source_filter, 1, &offset, matrix_source_filter_offset, 1, len);
    vDSP_mmul(matrix_query_filter_offset, 1, matrix_source_filter_offset, 1, matrix_filter, 1, query_filter.rows, source_filter.rows, query_filter.cols);
    
   
    //////////////////////////////////////////////////////////
    // Finish the clock
    
    t2 = cv::getTickCount();
    
    
    //////////////////////////////////////////////////////////
    // Count the matrix for matched features
    //   (can proly be improved)
    
    std::vector<std::string> results_set;
    int frequency = 0;
    for (size_t j=0; j<len; j++) {
        
        if(matrixAB[j] > dratio && (matrix_filter[j] == 1 || matrix_filter[j] == 4)) {
            size_t source_feature_idx = (size_t)(j % source.rows);
            if(source_feature_idx == 0) {
                source_feature_idx = source.rows;
            }
            float image_flt = ((float)source_feature_idx / AKAZEOptions::AKAZE_KEYPOINTCOUNT);
            int image_idx = floor(image_flt);
            
            results_set.push_back(metadata[image_idx]);
        }
    }
    
    
    //////////////////////////////////////////////////////////
    // Count the matrix for matched features
    
    bool matched_image = false;
    std::string matched_meta = most_frequent_element(results_set, frequency);
    
    float threshold = (float)frequency / (float)AKAZEOptions::AKAZE_KEYPOINTCOUNT;
    
    // Debugging print statement. Uncomment for more information.
    // printf("Frequency: %d:%f - %s\n", frequency, threshold, matched_meta.c_str());
    
    if(query_count > 5 && threshold > accept_ratio) {
        float accept = 0.0;
        if (float(frequency) / query_count > 1) {
            accept = 1.0f;
        }
        else {
            accept = float(frequency) / (float)AKAZEOptions::AKAZE_KEYPOINTCOUNT;
        }
        printf("Matched Image: %s at: %.1f%%\n", matched_meta.c_str(), accept * 100);
        matched_image = true;
    }
    
    
    //////////////////////////////////////////////////////////
    // Cleanup
    
    delete [] matrix_source_filter_offset;
    delete [] matrix_query_filter_offset;
    delete [] matrix_source_filter;
    delete [] matrix_query_filter;
    delete [] matrixAB;
    delete [] matrixB;
    
    
    //////////////////////////////////////////////////////////
    // Return the match information
    
    if (matched_image) {
        return matched_meta;
    }
    else {
        return "";
    }
}

void flatten_filter_array(float *flattened_matrix, float *decision_filter, size_t n) {
    for (size_t i=0; i<n; ++i) {
        if (decision_filter[i] == 1.0f || decision_filter[i] == 4.0f) {
            flattened_matrix[i] = 1.0f;
        }
        else {
            flattened_matrix[i] = 0.0f;
        }
    }
}


#pragma mark -
#pragma mark Template functions
/* ************************************************************************* */
/*!
 * @brief Generic C++ template to count maximum occurance of an element
 *        in a container (i.e., std::vector, std::set, etc.
 *
 * @param v (container element)
 *          Input container element to enumerate through
 *
 * @param frequence (&int)
 *          An address pointer to the frequency occurance.
 *
 * @return (int)
 */

template<typename T>
typename T::value_type most_frequent_element(T const& v, int &frequency)
{    // Precondition: v is not empty
    std::map<typename T::value_type, int> frequencyMap;
    int maxFrequency = 0;
    typename T::value_type mostFrequentElement{};
    for (auto&& x : v)
    {
        int f = ++frequencyMap[x];
        if (f > maxFrequency)
        {
            maxFrequency = f;
            mostFrequentElement = x;
        }
    }
    frequency = maxFrequency;
    return mostFrequentElement;
}

template <typename T> // T models Any
struct static_cast_func
{
    template <typename T1> // T1 models type statically convertible to T
    T operator()(const T1& x) const { return static_cast<T>(x); }
};


@end
