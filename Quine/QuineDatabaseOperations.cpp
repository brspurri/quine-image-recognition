//
//  QuineDatabaseOperations.cpp
//  Quine
//
//  Created by Brett Spurrier on 1/26/14.
//  Copyright (c) 2014 Spurrier. All rights reserved.
//

#include "QuineDatabaseOperations.h"
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "QuineMemoryDatabase.h"
#include "QuineFeatureDetection.h"
#include "QuineFeatureStruct.h"
#include "QuineCommon.h"


#pragma mark -
#pragma mark QuineDatabaseOperations
/* ************************************************************************* */
/**
 * @brief Constructor. Fired on class creation
 *
 * @return (QuineDatabaseOperations)
 */
QuineDatabaseOperations::QuineDatabaseOperations()
{
}


/* ************************************************************************* */
/**
 * @brief Desctuctor
 *
 * @return (void)
 */
QuineDatabaseOperations::~QuineDatabaseOperations()
{
}


#pragma mark -
#pragma mark QuineDatabaseOperations | Add Image
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

void QuineDatabaseOperations::add_image(const cv::Mat& img,
                                        const std::string& hash,
                                        const std::string& img_id,
                                        const std::string& meta,
                                        const std::string& path)
{
    // Initial declarations
    cv::Mat resized_img, gray_img;
    
    // Database reference which to add the image to
    cv::Mat source;
    cv::Mat filter;
    
    // JSON string reference which to add the image metadata to
    cv::vector<std::string> metadata;
    cv::vector<std::string> hashtable;
    
    // Get a reference to the current in-memory database
    //  source -> Current database associated with the path
    //  filter -> Current database class filter information. 1 x source.rows() in size.
    //  metadata -> Current database image information associated with the path
    //  hashtable -> NOT YET IMPLEMENTED
    QuineMemory::database()->get_database(path, source, filter, metadata, hashtable, false);
    
    // Initialize the feature detection
    QuineFeatureDetection image = QuineFeatureDetection();
    
    // Resize and gray the input image
    image.resize_to_width(img, resized_img, RESIZED_IMAGE_WIDTH);
    image.get_gray(resized_img, gray_img);
    
    // Calculate the query descriptor and add it to the specified database
    akaze_response_struc result_img;
    image.compute_signature(gray_img, result_img, false);
    
    // Add or append the descriptors to the in memory database
    if(source.empty()) {
        source = result_img.desc;
        filter = result_img.filter;
    }
    else {
        source.push_back(result_img.desc);
        filter.push_back(result_img.filter);
    }
    
    //Update the metadata information and add it to the specified database
    metadata.push_back(meta);
    //hashtable.push_back(hash);
    
    // Push the appended database to memory (and disk)
    QuineMemory::database()->update_database(path, source, filter, metadata, hashtable, true);

    // Cleanup
    image.~QuineFeatureDetection();
    
}


#pragma mark -
#pragma mark QuineDatabaseOperations | Database operations
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
void QuineDatabaseOperations::load_database(const std::string& path, bool force)
{
    QuineMemory::database()->load_database(path);
}


/* ************************************************************************* */
/**
 * @brief Lists the image tags for a loaded database.
 *
 * @param path (const std::string)
 *        Full path to the database
 *
 * @return (void)
 */
void QuineDatabaseOperations::list_images(const std::string& path, std::vector<std::string> &images)
{
    QuineMemory::database()->load_images(path, images);
}


/* ************************************************************************* */
/**
 * @brief Loads a database from disk to memory. This method may be called
 *        multiple times to load mutiple databases
 *
 * @return (std::string)
 *        A vector containing the loaded database paths
 */
std::vector<std::string> QuineDatabaseOperations::list_loaded_databases()
{
    return QuineMemory::database()->list_loaded_databases();
    
}

void QuineDatabaseOperations::get_database(std::string &db,
                                           cv::Mat &source,
                                           cv::Mat &filter,
                                           cv::vector<std::string> &metadata,
                                           cv::vector<std::string> &hashtable,
                                           bool force) {
    
    QuineMemory::database()->get_database(db, source, filter, metadata, hashtable, force);
    
}

