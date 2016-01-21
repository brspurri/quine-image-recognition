/* ********************************************************************************************************* */
/*! @file QuineDatabaseOperations.h
 *
 *  @brief This file contains classe for 'QuineDatabaseOperations'.
 *
 *  @details (QuineDatabaseOperations)
 *             Previously, this was a structure, but then converted to
 *             a full class. Contains public variables for all the returned image descriptions and keypoints.
 *
 *           (QuineFeatureDetection)
 *            Performs all the image processing and description methods.
 *
 *  @author Brett Spurrier (@reirrupStterB on Twitter)
 *
 *  @date 1/26/14
 *  @copyright 2014 Brett R. Spurrier
 */
/* ********************************************************************************************************* */

#ifndef __Quine__QuineDatabaseOperations__
#define __Quine__QuineDatabaseOperations__

#include <iostream>
#include "QuineConstants.h"

//TODO: Remove below mst likely
/*
struct feature_struc {
    std::string image_id;
    std::string hash;
    int x;
    int y;
    int scale;
    int angle;
    int laplacian;
    
    cv::Mat desc_mat;
    std::vector<unsigned char> desc_bytes;
    std::bitset<FEATURE_DESC_SIZE_BITS> desc_bits;
};


struct database_struc {
    
    std::vector<feature_struc> features;
    cv::Mat featuresMat;
    std::string database_path;
    std::string index_path;

};
*/


class QuineDatabaseOperations {
public:
    
    /* ************************************************************************* */
    /*!
     * Constructor method that provides an QuineDatabaseOperations class instance.
     * @returns The QuineDatabaseOperations instance.
     */
    QuineDatabaseOperations();
    
    
    /* ************************************************************************* */
    /*!
     * Destructor method that destroys the current QuineDatabaseOperations class instance.
     * @returns void
     */
    ~QuineDatabaseOperations();
    

    /* ************************************************************************* */
    /**
     * @brief Adds an image to the specified database
     *
     * @param img (const cv::Mat)
     *        Input source image of OpenCV type cv::Mat
     *
     * @param hash (const std::string)
     *        Hash value for the image. Used so that the same images are
     *        not duplicated in the database
     *
     * @param img_id (const std::string)
     *        String GUID used as an identifier for the image
     *
     * @param meta (const std::string)
     *        Meta data used as auxilary information that the end-user may
     *        wish to store with the image in the database
     *
     * @param path (const std::string)
     *        Full path to the database
     *
     * @return (void)
     */
    virtual void add_image(const cv::Mat& img,
                           const std::string& hash,
                           const std::string& img_id,
                           const std::string& meta,
                           const std::string& path);
    
    
    /* ************************************************************************* */
    /**
     * @brief Loads a database from disk to memory. This method may be called
     *        multiple times to load mutiple databases
     *
     * @param path (const std::string)
     *        Full path to the database
     *
     * @return (void)
     */
    virtual void load_database(const std::string& path, bool force);
    
    
    /* ************************************************************************* */
    /**
     * @brief Lists the image tags for a loaded database.
     *
     * @param path (const std::string)
     *        Full path to the database
     *
     * @return (void)
     */
    virtual void list_images(const std::string& path, std::vector<std::string> &images);
    
    
    /* ************************************************************************* */
    /**
     * @brief Loads a database from disk to memory. This method may be called
     *        multiple times to load mutiple databases
     *
     * @return (std::string)
     *        A vector containing the loaded database paths
     */
    virtual std::vector<std::string> list_loaded_databases();
    
    virtual void get_database(std::string &db,
                              cv::Mat &source,
                              cv::Mat &filter,
                              cv::vector<std::string> &metadata,
                              cv::vector<std::string> &hashtable,
                              bool force);
    
};


#endif /* defined(__Quine__QuineDatabaseOperations__) */
