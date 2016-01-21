/***********************************************************************************************************/
/*! @file QuineFeatureDetection.cpp
 *
 *  @brief Accompanies QuineFeatureDetection.h.
 *
 *  @author Brett Spurrier (@reirrupStterB on Twitter)
 *
 *  @date 1/21/14
 *  @copyright 2014 Brett R. Spurrier
 */
/***********************************************************************************************************/

#include "QuineFeatureDetection.h"
#include <ctime>
#include <thread>
#include <algorithm>
#include <assert.h>
#include <string>

#include "QuineConstants.h"
#include "AKAZE.h"
#include "AKAZEConfig.h"


#pragma mark -
#pragma mark akaze_response_struc
/* ************************************************************************* */
/*!
 * @brief Initializes the akaze_response_struc class
 *
 * @return (akaze_response_struc)
 */
akaze_response_struc::akaze_response_struc() {
}


/* ************************************************************************* */
/*!
 * @brief Desctroys the QuineFeatureDetection class
 *
 * @return (void)
 */
akaze_response_struc::~akaze_response_struc() {
}


#pragma mark -
#pragma mark QuineFeatureDetection
/* ************************************************************************* */
/*!
 * @brief Initializes the QuineFeatureDetection class
 *
 * @return (QuineFeatureDetection)
 */
QuineFeatureDetection::QuineFeatureDetection()
{
    initialize();
}


/* ************************************************************************* */
/*!
 * @brief Desctroys the QuineFeatureDetection class
 *
 * @return (void)
 */
QuineFeatureDetection::~QuineFeatureDetection() {
}


/* ************************************************************************* */
/*!
 * @brief Initializes the class. Currently unused.
 *
 * @return (bool)
 */
void QuineFeatureDetection::initialize() {
}


#pragma mark -
#pragma mark QuineFeatureDetection | Image functions
/* ************************************************************************* */
/*!
 * @brief Converts the image to grayscale (single channel).
 *
 * @param src (cv::Mat)
 *        Input source image of OpenCV type cv::Mat.
 *
 * @param dst (cv::Mat)
 *        Output destination image of OpenCV type cv::Mat, single (gray) channel.
 *
 * @return (void)
 */
void QuineFeatureDetection::get_gray(const cv::Mat& src, cv::Mat& dst)
{
    const int numChannes = src.channels();
    
    
    if (numChannes == 4) {
        //src.convertTo(dst, CV_BGRA2GRAY);
        
        // Print the pre color convert
        /*
        printf(" Pre color convertion\n");
        printf(" %d %d %d %d ", (int)src.at<unsigned char>(0, 0), (int)src.at<unsigned char>(0, 1), (int)src.at<unsigned char>(0, 2), (int)src.at<unsigned char>(0, 3));
        printf(" %d %d %d %d ", (int)src.at<unsigned char>(0, 4), (int)src.at<unsigned char>(0, 5), (int)src.at<unsigned char>(0, 6), (int)src.at<unsigned char>(0, 7));
        printf(" %d %d %d %d ", (int)src.at<unsigned char>(0, 8), (int)src.at<unsigned char>(0, 9), (int)src.at<unsigned char>(0, 10), (int)src.at<unsigned char>(0, 11));
        printf(" %d %d %d %d ", (int)src.at<unsigned char>(0, 12), (int)src.at<unsigned char>(0, 13), (int)src.at<unsigned char>(0, 14), (int)src.at<unsigned char>(0, 15));
        printf(" %d %d %d %d ", (int)src.at<unsigned char>(0, 16), (int)src.at<unsigned char>(0, 17), (int)src.at<unsigned char>(0, 18), (int)src.at<unsigned char>(0, 19));
        printf(" %d %d %d %d\n\n", (int)src.at<unsigned char>(0, 20), (int)src.at<unsigned char>(0, 21), (int)src.at<unsigned char>(0, 22), (int)src.at<unsigned char>(0, 23));
        */
        // Convert
        cv::cvtColor(src, dst, CV_BGRA2GRAY);
        
        // Print the post color convert
        /*
        printf(" Post color convertion\n");
        printf(" %d %d %d %d ", (int)dst.at<unsigned char>(0, 0), (int)dst.at<unsigned char>(0, 1), (int)dst.at<unsigned char>(0, 2), (int)dst.at<unsigned char>(0, 3));
        printf(" %d %d %d %d ", (int)dst.at<unsigned char>(0, 4), (int)dst.at<unsigned char>(0, 5), (int)dst.at<unsigned char>(0, 6), (int)dst.at<unsigned char>(0, 7));
        printf(" %d %d %d %d ", (int)dst.at<unsigned char>(0, 8), (int)dst.at<unsigned char>(0, 9), (int)dst.at<unsigned char>(0, 10), (int)dst.at<unsigned char>(0, 11));
        printf(" %d %d %d %d ", (int)dst.at<unsigned char>(0, 12), (int)dst.at<unsigned char>(0, 13), (int)dst.at<unsigned char>(0, 14), (int)dst.at<unsigned char>(0, 15));
        printf(" %d %d %d %d ", (int)dst.at<unsigned char>(0, 16), (int)dst.at<unsigned char>(0, 17), (int)dst.at<unsigned char>(0, 18), (int)dst.at<unsigned char>(0, 19));
        printf(" %d %d %d %d\n", (int)dst.at<unsigned char>(0, 20), (int)dst.at<unsigned char>(0, 21), (int)dst.at<unsigned char>(0, 22), (int)dst.at<unsigned char>(0, 23));
        */
        
        // One day, I'll update this
        //cv::neon_cvtColorBGRA2GRAY(src, dst);
    }
    else if (numChannes == 3) {
        cv::cvtColor(src, dst, CV_BGR2GRAY);
    }
    else if (numChannes == 1) {
        dst = src;
    }
}


/* ************************************************************************* */
/*!
 * @brief Resizes the cv::Mat image to a new width while
 *        maintaining a proper aspect ratio.
 *
 * @param src (cv::Mat)
 *        Input source image of OpenCV type cv::Mat.
 *
 * @param dst (cv::Mat)
 *        Output destination image of OpenCV type cv::Mat with new width.
 *
 * @param width (float)
 *        The width of the outputted cv::Mat image (dst).
 *
 * @return (void)
 */
void QuineFeatureDetection::resize_to_width(const cv::Mat& src, cv::Mat& dst, float width) {
    
    float oldWidth = src.cols;
    float scaleFactor = width / oldWidth;
    
    float newHeight = src.rows * scaleFactor;
    float newWidth = oldWidth * scaleFactor;
    
    cv::resize(src, dst, cv::Size(newWidth, newHeight), 0, 0, CV_INTER_LINEAR);
}


#pragma mark -
#pragma mark QuineFeatureDetection | Image Description
/* ************************************************************************* */
/*!
 * @brief Sorts the cv::Keypoint responses according from highest to lowest.
 *        For use with std::sort.
 *
 * @param i (cv::KeyPoint)
 *
 * @param j (cv::KeyPoint)
 *        The second cv::Keypoint
 *
 * @return (bool)
 */
bool sort_responses(cv::KeyPoint i, cv::KeyPoint j) { return i.response > j.response; }


/* ************************************************************************* */
/*!
 * @brief Computes the image descriptor for an input image.
 *
 * @param frame (const cv::Mat)
 *              The input image. This image must be single channel
 *              and grayscale. For performance, the image should also
 *              be reduced in size, yet still have the correct aspect ratio.
 *
 * @return (akaze_response_struc)
 */
void QuineFeatureDetection::compute_signature(const cv::Mat& frame, akaze_response_struc &response, bool is_query) {
    
    // Time declarations
    double t1 = 0.0;
    double t2 = 0.0;
    double takaze = 0.0;
    
    // Initial image and descriptor declarations
    cv::Mat img_32;
    std::vector<cv::KeyPoint> kpts_akaze;
    cv::Mat desc_akaze;
    
    // Set the AKAZE options
    AKAZEOptions options;
    options.img_width = frame.cols;
    options.img_height = frame.rows;
    
    // Declare the AKAZE evolution steps
    libAKAZE::AKAZE evolution1(options);
    
    // Feature detection process
    t1 = cv::getTickCount();
    frame.convertTo(img_32,CV_32F,1.0/255.0,0);
    
    // Build the scale space and detect the features
    evolution1.Create_Nonlinear_Scale_Space(img_32);
    evolution1.Feature_Detection(kpts_akaze);
    std::sort(kpts_akaze.begin(), kpts_akaze.end(), sort_responses);
    
    // Feature description process
    evolution1.Compute_Descriptors(kpts_akaze, desc_akaze);
    t2 = cv::getTickCount();
    takaze = 1000.0*(t2-t1)/cv::getTickFrequency();
    
    if(!is_query) {
        // Handle the data
        cv::Mat mat(0, AKAZEOptions::AKAZE_FEATURECOUNT, CV_32FC1);
    
        // Either expand or contract the data to make it a constant size.
        //  This is necessary to make the indexing process much less complex.
        if(desc_akaze.rows >= AKAZEOptions::AKAZE_KEYPOINTCOUNT) {
            cv::Rect roi(0, 0, desc_akaze.cols, AKAZEOptions::AKAZE_KEYPOINTCOUNT);
            mat = desc_akaze(roi);
        }
        else if (desc_akaze.rows < AKAZEOptions::AKAZE_KEYPOINTCOUNT) {
            mat = cv::Mat(AKAZEOptions::AKAZE_KEYPOINTCOUNT, desc_akaze.cols, CV_32FC1, cv::Scalar(0));
            if(desc_akaze.rows > 0) {
                desc_akaze.copyTo(mat(cv::Rect(0, 0, desc_akaze.cols, desc_akaze.rows)));
            }
        }
    
        // Build the filter cv::Mat
        kpts_akaze.resize(AKAZEOptions::AKAZE_KEYPOINTCOUNT);
        
        // Copy descriptor data to response object
        mat.copyTo(response.desc);
        mat.release();
    }
    else {
        desc_akaze.copyTo(response.desc);
    }
    
    std::vector<uint8_t> feature_class;
    for(auto kpt:kpts_akaze) {
        feature_class.push_back(kpt.class_id);
    }
    cv::Mat filter((int)feature_class.size(), 1, CV_8UC1, &feature_class[0]);
    
    // Copy keypoint information to response object
    response.kpts_count = desc_akaze.rows;
    response.kpts = kpts_akaze;
    filter.copyTo(response.filter);
    
    // Set the response options
    response.options = options;
    
    // Uncomment to show the description proccess time
    //std::cout << "A-KAZE Features (" << desc_akaze.rows << ") Extraction Time (ms): " << takaze << std::endl;
    
    // Uncomment to show the panel of computation times
    //evolution1.Show_Computation_Times();

    // Cleanup
    filter.release();
    desc_akaze.release();
    //evolution1.~AKAZE(); //Not sure if this works...
    
}


