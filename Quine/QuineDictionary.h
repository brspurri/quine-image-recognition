/*!
 * QUINE VISION, INC. ("COMPANY") CONFIDENTIAL
 * Unpublished Copyright (c) 2014 QUINE VISION, INC., All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of COMPANY. The intellectual and technical
 * concepts contained herein are proprietary to COMPANY and
 * may be covered by U.S. and Foreign Patents, patents in
 * process, and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this
 * material is strictly forbidden unless prior written permission
 * is obtained from COMPANY.  Access to the source code contained
 * herein is hereby forbidden to anyone except current COMPANY
 * employees, managers or contractors who have executed
 * Confidentiality and Non-disclosure agreements explicitly
 * covering such access.
 *
 * The copyright notice above does not evidence any actual or
 * intended publication or disclosure  of  this source code, which
 * includes information that is confidential and/or proprietary,
 * and is a trade secret, of  COMPANY.
 *
 * ANY REPRODUCTION, MODIFICATION, DISTRIBUTION, PUBLIC  PERFORMANCE,
 * OR PUBLIC DISPLAY OF OR THROUGH USE  OF THIS  SOURCE CODE  WITHOUT
 * THE EXPRESS WRITTEN CONSENT OF COMPANY IS STRICTLY PROHIBITED,
 * AND IN VIOLATION OF APPLICABLE LAWS AND INTERNATIONAL TREATIES.
 * THE RECEIPT OR POSSESSION OF  THIS SOURCE CODE AND/OR RELATED
 * INFORMATION DOES NOT CONVEY OR IMPLY ANY RIGHTS TO REPRODUCE,
 * DISCLOSE OR DISTRIBUTE ITS CONTENTS, OR TO MANUFACTURE, USE,
 * OR SELL ANYTHING THAT IT  MAY DESCRIBE, IN WHOLE OR IN PART.
 *
 * @file   QuineDictionary.h
 * @author Brett Spurrier (brett@quinevision.com)
 * @date   August, 2014
 * @brief  C/C++ dictionary class.
 *
 */

//Adopted from https://github.com/metulburr/cplusplus/blob/master/dictionary.cpp
#ifndef Quine_QuineDictionary_h
#define Quine_QuineDictionary_h
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iomanip>


/* ************************************************************************* */
/*!
 *  @class      Dict
 *
 *  @abstract   C/C++ dictionary class
 *
 *  @discussion This is a more managable implementation of std::map<T,T>().
 *
 *  @updated 2014-08-15
 */
template <class KEY, class VALUE>
class Dict {
public:
    std::map<KEY, VALUE> dictionary;
    
    void increment(KEY k) {
        
        if ( dictionary.find(k) == dictionary.end() ) {
            dictionary[k] = 1;
        } else {
            dictionary[k] = dictionary[k] + 1;
        }
    
    }
    
    void update(KEY k, VALUE v){
        dictionary[k] = v;
    }
    
    std::vector<KEY> keys(){
        std::vector<KEY> v;
        for(typename std::map<KEY, VALUE>::const_iterator it = dictionary.begin();
            it != dictionary.end(); ++it){
            v.push_back(it->first);
        }
        return v;
    }
    
    std::vector<VALUE> values(){
        std::vector<VALUE> v;
        for(class std::map<KEY, VALUE>::const_iterator it = dictionary.begin();
            it != dictionary.end(); ++it){
            v.push_back(it->second);
        }
        return v;
    }
    VALUE pop(KEY k){
        VALUE val = dictionary[k];
        dictionary.erase(k);
        return val;
    }
    
    VALUE get(KEY k){
        //return value of k given
        VALUE val = dictionary[k];
        return val;
    }
    
    void clear(){
        dictionary.clear();
    }
    
    std::string display(){
        std::string disp;
        for(auto key:keys()){
            disp += key;
            disp += " -> ";
            disp += this->get(key);
            disp += '\n';
        }
        return disp;
    }
    
    std::vector<KEY> sorted(){
        //return vector of sorted keys
        std::vector<VALUE> v;
        for(class std::map<KEY, VALUE>::const_iterator it = dictionary.begin();
            it != dictionary.end(); ++it){
            v.push_back(it->second);
        }
        std::sort(v.begin(), v.end());
        std::vector<KEY> k;
        for (int i=0; i < v.size(); i++){
            for (auto key:keys()){
                if (v[i]==get(key)){
                    k.push_back(key);
                }
            }
        }
        return k;
    }
};


#endif
