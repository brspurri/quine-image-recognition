//
//  QuineMemoryDatabase.h
//  Quine
//
//  Created by Brett Spurrier on 4/9/14.
//  Copyright (c) 2014 Spurrier. All rights reserved.
//

#ifndef __Quine__QuineMemoryDatabase__
#define __Quine__QuineMemoryDatabase__

#include "QuineDictionary.h"


class QuineMemory
{
private:
    
    Dict<std::string, cv::Mat> m_sources;
    Dict<std::string, cv::Mat> m_filter;
    Dict<std::string, cv::vector<std::string> > m_indicies;
    Dict<std::string, cv::vector<std::string> > m_hashtable;
    
    static bool instance_flag;
    static QuineMemory *s_instance;
    QuineMemory() { }
    
    virtual std::string substr_replace(std::string &s,
                                       std::string toReplace,
                                       std::string replaceWith);
    
    
public:
    static QuineMemory* database();
    void method();
    ~QuineMemory()
    {
        instance_flag = false;
    }
    
    std::vector<std::string> get_database_paths() {
        
        //Make sure that the same number of keys exist for both m_sources and m_indicies.
        //  By having the same number of keys, the assumption is that the keys themselves are the same.
        //  Might not be the best logic, but hey, it works so go with it.
        assert(m_sources.keys().size() == m_indicies.keys().size());
        
        //Send back the keys for m_sources (and assumed m_indicies)
        return m_sources.keys();
    }
    
    std::vector<std::string> list_loaded_databases() {
        return get_database_paths();
    }
    
    
    
    void load_database(const std::string &db, bool force) {
        
        //Initial declarations
        cv::Mat source;
        cv::Mat filter;
        cv::vector<std::string> metadata;
        
        // Check if the database has been read from the disk
        std::vector<std::string> keys = get_database_paths();
        if(std::find(keys.begin(), keys.end(), db) == keys.end() || force) {
            
            //Not found, so load the database from the disk
            load_database_from_file(db, source, filter, metadata);
            source.convertTo(source, CV_32FC1);
            
            //Check to see if a database was loaded. If not, this is the first use of the database
            if(!source.empty()) {
                m_sources.update(db, source);
                m_filter.update(db, filter);
                m_indicies.update(db, metadata);
            }
        }
    }
    
    
    void load_database(const std::string &db) {
        load_database(db, false);
    }
    
    
    void load_images(const std::string &db, std::vector<std::string> &images) {
        images = m_indicies.get(db);
    }
    
    
    void unload_database(const std::string &db) {
        m_sources.pop(db);
        m_indicies.pop(db);
    }
    
    
    void get_database(const std::string &db,
                      cv::Mat &source,
                      cv::Mat &filter,
                      cv::vector<std::string> &metadata,
                      cv::vector<std::string> &hashtable,
                      bool force)
    {

        // Check if the database has been read from the disk
        std::vector<std::string> keys = get_database_paths();
        if(std::find(keys.begin(), keys.end(), db) == keys.end() || force) {
            
            //Not found, so load the database from the disk
            load_database_from_file(db, source, filter, metadata);
            
            // Add the information to the singleton sources,
            //   so long as the database info is correct (i.e., not empty).
            if(!source.empty()) {
                m_sources.update(db, source);
                m_filter.update(db, filter);
                m_indicies.update(db, metadata);
            }
            
        }
        
        else {
            // Database is currently loaded, so grab the dictionary
            source = m_sources.dictionary[db];
            filter = m_filter.dictionary[db];
            metadata = m_indicies.dictionary[db];
        }
    }
    
    
    cv::vector<std::string> get_indices(const std::string &db)
    {
        return m_indicies.dictionary[db];
    }
    
    void update_database(const std::string &db,
                         cv::Mat &source,
                         cv::Mat &filter,
                         cv::vector<std::string> &meta,
                         cv::vector<std::string> &hashtable,
                         bool save) {
        
        // Update the memory copies of the database
        m_filter.update(db, filter);
        m_sources.update(db, source);
        m_indicies.update(db, meta);
        
        // If specified, save the database to disk
        if(save) {
            if(save_database_to_file(db, source, filter, meta, hashtable, false)) {
                std::cout << "[Quine: Success]: Image was assed to database" << std::endl;
            }
            else {
                std::cout << "[Quine: Error]: Image failed to add to database" << std::endl;
            }
        }
    }
    
    
    /* ************************************************************************* */
    /*!
     * @brief Reads the desciptors for a set of images in a .bin file.
     *        Reads the index information for each image in a .idx (json format) file.
     *
     * @param database_path (std::string)
     *        Full path to the binary database. This value will
     *        be used for both the .bin and .idx files.
     *
     * @param idx_json (std::string)
     *        JSON string containing index and meta information for the images
     *        contained in the array. Initially, this is unset
     *
     * @param source_descriptors (cv::Mat)
     *        OpenCV matrix containing the descriptor information for the dataset
     *        Initially this is unset.
     *
     * @return (void)
     */
    virtual void load_database_from_file(const std::string &database_path,
                                         cv::Mat &source,
                                         cv::Mat &filter,
                                         cv::vector<std::string> &meta_json);
    
    
    virtual bool save_database_to_file(const std::string &database_path,
                                       const cv::Mat &source,
                                       const cv::Mat &filter,
                                       const cv::vector<std::string> &meta_json,
                                       const cv::vector<std::string> &hashtable,
                                       bool should_compress);
    
    
    /* ************************************************************************* */
    /*!
     * Detemines whether the database already exists.
     *
     * @returns boolean
     */
    virtual bool database_exists(const std::string& name);
    
    
};

bool QuineMemory::instance_flag = false;
QuineMemory* QuineMemory::s_instance = NULL;
QuineMemory* QuineMemory::database()
{
    if(!instance_flag)
    {
        s_instance = new QuineMemory();
        instance_flag = true;
        return s_instance;
    }
    else
    {
        return s_instance;
    }
}

void QuineMemory::method()
{
    std::cout << "Method of the singleton class" << std::endl;
}


#endif /* defined(__Quine__QuineMemoryDatabase__) */
