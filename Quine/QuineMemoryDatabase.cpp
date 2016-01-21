//
//  QuineMemoryDatabase.cpp
//  Quine
//
//  Created by Brett Spurrier on 4/9/14.
//  Copyright (c) 2014 Spurrier. All rights reserved.
//

#include "QuineMemoryDatabase.h"


#include <zlib.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <string>
#include <streambuf>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>


#pragma mark -
#pragma mark GZip compression routines
/* ************************************************************************* */
/*!
 * @brief Decompresses a Quine database file to a std::string, 
 *        and then loads the database source matrix and images indices.
 *
 * @param compressed_file_path (std::string)
 *        Full path to the gzipped binary database.
 *
 * @return (void)
 */
std::vector<char> gzip_uncompress(const std::string &compressed_file_path)
{
    char outbuffer[1024*16];
    gzFile infile = (gzFile)gzopen(compressed_file_path.c_str(), "rb");
    std::vector<char> outfile;
    gzrewind(infile);
    while(!gzeof(infile))
    {
        int len = gzread(infile, outbuffer, sizeof(outbuffer));
        outfile.insert(outfile.end(), outbuffer, outbuffer+len);
    }
    gzclose(infile);
    return outfile;
}


/*
bool gzip_compress(std::string &uncompressed_file_path, std::string  &compressed_file_path)
{
	FILE *infile = fopen(uncompressed_file_path, "rb");
	gzFile outfile = gzopen(compressed_file_path, "wb");
	if (!infile || !outfile)
		return -1;
	char inbuffer[128];
	int num_read = 0;
	unsigned long total_read = 0, total_wrote = 0;
	while ((num_read = fread(inbuffer, 1, sizeof(inbuffer), infile)) > 0)
	{
		total_read += num_read;
		gzwrite(outfile, inbuffer, num_read);
	}
	fclose(infile);
	gzclose(outfile);
	return true;
}
*/


template<class ReturnType>
std::vector<ReturnType> split(const std::string&, const std::string&, const bool = true);

template<>
std::vector<std::string> split(const std::string& s, const std::string& delim, const bool keep_empty) {
    
    std::vector<std::string> result;
    if (delim.empty()) {
        result.push_back(s);
        return result;
    }
    std::string::const_iterator substart = s.begin(), subend;
    while (true) {
        subend = std::search(substart, s.end(), delim.begin(), delim.end());
        std::string temp(substart, subend);
        if (keep_empty || !temp.empty()) {
            result.push_back(temp);
        }
        if (subend == s.end()) {
            break;
        }
        substart = subend + delim.size();
    }
    return result;
    
}

std::vector<cv::KeyPoint> concat_keypoint(const std::string& x,
                                          const std::string& y,
                                          const std::string& class_id,
                                          const std::string& s,
                                          const std::string& delim,
                                          const bool keep_empty = true) {
    // code for converting string to int
    
    std::vector<cv::KeyPoint> result;
    if (delim.empty()) {
        
        cv::KeyPoint kpt = cv::KeyPoint();
        kpt.size = atof(s.c_str());
        kpt.pt = cv::Point2f(atof(x.c_str()), atof(y.c_str()));
        kpt.class_id = atoi(class_id.c_str());
        
        result.push_back(kpt);
        return result;
    }
    
    std::string::const_iterator x_substart = x.begin(), x_subend;
    std::string::const_iterator y_substart = y.begin(), y_subend;
    std::string::const_iterator class_substart = class_id.begin(), class_subend;
    std::string::const_iterator size_substart = s.begin(), size_subend;
    
    while (true) {
        x_subend = std::search(x_substart, x.end(), delim.begin(), delim.end());
        y_subend = std::search(y_substart, y.end(), delim.begin(), delim.end());
        class_subend = std::search(class_substart, class_id.end(), delim.begin(), delim.end());
        size_subend = std::search(size_substart, s.end(), delim.begin(), delim.end());
        
        std::string x_temp(x_substart, x_subend);
        std::string y_temp(y_substart, y_subend);
        std::string class_temp(class_substart, class_subend);
        std::string size_temp(size_substart, size_subend);
        
        
        if (keep_empty || !x_temp.empty()) {
            cv::KeyPoint kpt = cv::KeyPoint();
            
            kpt.pt = cv::Point2f(atof(x_temp.c_str()), atof(y_temp.c_str()));
            kpt.class_id = atoi(class_temp.c_str());
            kpt.size = atof(size_temp.c_str());
            
            result.push_back(kpt);
        }
        if (x_subend == x.end()) {
            break;
        }
        x_substart = x_subend + delim.size();
        y_substart = y_subend + delim.size();
        class_substart = class_subend + delim.size();
        size_substart = size_subend + delim.size();
    }
    return result;
    
}



/*
std::vector<std::string> split(const std::string& s, const std::string& delim, const bool keep_empty = true) {
    
    
}
*/

void get_file_extension(std::string &filename, std::string &ext) {

    std::string::size_type idx;
    
    idx = filename.rfind('.');
    
    if(idx != std::string::npos)
    {
        ext = filename.substr(idx+1);
    }
    else
    {
        ext = "";
    }
    
}


#pragma mark -
#pragma mark QuineDatabaseOperations | Database Read/Write
/* ************************************************************************* */
/*!
 * @brief Reads the desciptors for a set of images in a .yaml.comp 
 *        gzipped compressed file. Also reads the index information 
 *        for the list of images as a comma separated string.
 *
 * @param database_path (std::string)
 *        Full path to the binary database. This value will
 *        be used for both the .bin and .idx files.
 *
 * @param source (cv::Mat)
 *        OpenCV matrix containing the descriptor information for the dataset
 *        Initially this is unset.
 *
 * @param meta_json (std::string)
 *        JSON string containing index and meta information for the images
 *        contained in the array. Initially, this is unset
 *
 * @return (void)
 */
void QuineMemory::load_database_from_file(const std::string &database_path,
                                          cv::Mat &source,
                                          cv::Mat &filter,
                                          cv::vector<std::string> &meta_json)
{

    std::string file_data;
    std::string yaml_path = database_path;
    std::string meta_string;
    if(database_exists(yaml_path)) {
        
        std::string file_ext;
        get_file_extension(yaml_path, file_ext);
        
        // Decomrpess the file data
        std::vector<char> decompressed_buffer;
        if (file_ext == "bin") {
            decompressed_buffer = gzip_uncompress(database_path);
        }
        else {
            
            std::ifstream testFile(database_path, std::ios::binary);
            std::vector<char> fileContents((std::istreambuf_iterator<char>(testFile)),
                                           std::istreambuf_iterator<char>());
            
            decompressed_buffer = fileContents;
        }
        
        std::string decompressed( decompressed_buffer.begin(), decompressed_buffer.end() );
        if (decompressed_buffer.size() == 0) {
            printf( "Error decompressing file." );
            return;
        }
        else {
            
            //Read the descriptors and data from the database file
            cv::FileStorage storage(decompressed, cv::FileStorage::READ + cv::FileStorage::MEMORY);
            if(storage.isOpened()) {
                
                std::string meta_json_joined;
                
                // Read the data
                storage["data"] >> source;
                storage["idx"] >> meta_json_joined;
                storage["class"] >> filter;
                storage.release();
             
                // TEMPORARY DEBUGGING CODE
                cv::Mat image1_row1 = source.row(0);
                for (int n=0; n<64; n++) {
                    printf("%f ", image1_row1.at<float>(0, n));
                }
                printf("\n-\n");
                cv::Mat image1_row2 = source.row(1);
                for (int n=0; n<64; n++) {
                    printf("%f ", image1_row2.at<float>(0, n));
                }
                printf("\n\n");
                
                cv::Mat image2_row1 = source.row(50);
                for (int n=0; n<64; n++) {
                    printf("%f ", image2_row1.at<float>(0, n));
                }
                printf("\n-\n");
                cv::Mat image2_row2 = source.row(51);
                for (int n=0; n<64; n++) {
                    printf("%f ", image2_row2.at<float>(0, n));
                }
                printf("\n\n");
                
                cv::Mat image3_row1 = source.row(100);
                for (int n=0; n<64; n++) {
                    printf("%f ", image3_row1.at<float>(0, n));
                }
                printf("\n-\n");
                cv::Mat image3_row2 = source.row(101);
                for (int n=0; n<64; n++) {
                    printf("%f ", image3_row2.at<float>(0, n));
                }
                printf("\n\n");
                
                // END TEMPORARY DEBUGGING CODE
                
                std::istringstream iss(meta_json_joined);
                copy(std::istream_iterator<std::string>(iss),
                     std::istream_iterator<std::string>(),
                     std::back_inserter<cv::vector<std::string> >(meta_json));
                
            }
            else {
                std::cout << "[Error]: Could not open database." << std:: endl;
            }
        }
    }
}


/* ************************************************************************* */
/*!
 * @brief Saves images to a local database at the specified path into memory.
 *
 * @param database_path (const std::string)
 *        A string containing the path of the database which will contain the image.
 *
 * @param source_descriptors (const cv::Mat)
 *        A cv::Mat containing the concatenated descriptors for each image
 *        to be added to the database.
 *
 * @param meta_json (const std::string)
 *        A string containing the JSON-encoded information for each image
 *        to be added to the database.

 *
 * @return (void)
 */
bool QuineMemory::save_database_to_file(const std::string &database_path,
                                        const cv::Mat &source,
                                        const cv::Mat &filter,
                                        const cv::vector<std::string> &meta_json,
                                        const cv::vector<std::string> &hashtable,
                                        bool should_compress=true)
{

    // Initialize variables
    std::ostringstream oss;
    std::string compressed_extention = ".comp";
    cv::Mat c;
    
    //Store the descriptors and data in the database file
    std::string yaml_path = database_path;
    cv::FileStorage storage(yaml_path, cv::FileStorage::WRITE);
    
    cv::Mat filter_converted;
    //filter.convertTo(filter_converted, CV_8U);
    
    // Add the data
    storage << "data" << source << "idx" << meta_json << "hash" << hashtable << "class" << filter;
    
    // Release the cv::FileStorage variable
    storage.release();
    
    if(should_compress) {
        
        // Set the compressed file path name
        std::string compressed_path = database_path;
        //compressed_path = substr_replace(compressed_path, database_extention, compressed_extention);
        
        // Compress the file to binary
        throw 130; // NOT YET IMPLEMENTED
    }
    
    return database_exists(yaml_path);
}


std::string QuineMemory::substr_replace(std::string &s,
                                        std::string toReplace,
                                        std::string replaceWith) {
    
    return(s.replace(s.find(toReplace), toReplace.length(), replaceWith));
}

#pragma mark -
#pragma mark QuineDatabaseOperations | Database Management
/* ************************************************************************* */
/*!
 * @brief Determines if the database path already exists
 *
 * @param database_path (std::string)
 *        Full path to the binary database. This value will
 *        be used for both the .bin and .idx files.
 *
 * @return (bool)
 */
bool QuineMemory::database_exists(const std::string& name) {
    
    bool exists = false;
    std::ifstream Infield(name);
    exists = Infield.good();
    return exists;
}




