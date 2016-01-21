/* ********************************************************************************************************* */
/*! @file QuineFeatureDetection.h
 *
 *  @brief This file contains classes for 'akaze_response_struc' and 'QuineFeatureDetection'.
 *
 *  @details (akaze_response_struc) 
 *             Previously, this was a structure, but then converted to
 *             a full class. Contains public variables for all the returned image descriptions and keypoints.
 *
 *           (QuineFeatureDetection) 
 *            Performs all the image processing and description methods.
 *
 *  @author Brett Spurrier (@reirrupStterB on Twitter)
 *
 *  @date 1/21/14
 *  @copyright 2014 Brett R. Spurrier
 */
/* ********************************************************************************************************* */
#ifndef __Quine__QuineFeatureDetection__
#define __Quine__QuineFeatureDetection__

#include <iostream>
#include <vector>
#include "AKAZE.h"
#include "AKAZEConfig.h"
#include "utils.h"


class akaze_response_struc {
public:
    
    /* ************************************************************************* */
    /*!
     * @brief Public class variables
     */
    std::vector<cv::KeyPoint> kpts;
    int kpts_count;
    cv::Mat filter;
    cv::Mat desc;
    AKAZEOptions options;
    
    
    /* ************************************************************************* */
    /*!
     * @brief Initializes the akaze_response_struc class
     *
     * @return (akaze_response_struc)
     */
    akaze_response_struc();
    
    
    /* ************************************************************************* */
    /*!
     * @brief Desctroys the QuineFeatureDetection class
     *
     * @return (void)
     */
    ~akaze_response_struc();
};


/* ************************************************************************* */
/*!
 * @class Defines the image feature detection methods
 */
class QuineFeatureDetection {
public:
    
    /* ************************************************************************* */
    /*!
     * @brief Initializes the QuineFeatureDetection class
     *
     * @return (QuineFeatureDetection)
     */
    QuineFeatureDetection();
    
    
    /* ************************************************************************* */
    /*!
     * @brief Initializes the QuineFeatureDetection class
     *
     * @return (QuineFeatureDetection)
     */
    ~QuineFeatureDetection();
    
    
    /* ************************************************************************* */
    /*!
     * @brief Initializes the QuineFeatureDetection class
     *
     * @return (QuineFeatureDetection)
     */
    virtual void initialize();
    
    
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
    virtual void compute_signature(const cv::Mat& greyFrame, akaze_response_struc &response, bool is_query);
    
    
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
    virtual void get_gray(const cv::Mat& src, cv::Mat& dst);
    
    
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
    virtual void resize_to_width(const cv::Mat& src, cv::Mat& dst, float width);

};

#endif /* defined(__Quine__QuineFeatureDetection__) */
