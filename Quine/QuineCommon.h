//
//  QuineCommon.h
//  Quine
//
//  Created by Brett Spurrier on 2/1/14.
//  Copyright (c) 2014 Spurrier. All rights reserved.
//

#include <stdlib.h>

#ifndef Quine_QuineCommon_h
#define Quine_QuineCommon_h

#include <string>
const std::string whiteSpaces( " \f\n\r\t\v" );


void trimRight( std::string& str,
               const std::string& trimChars = whiteSpaces )
{
    std::string::size_type pos = str.find_last_not_of( trimChars );
    str.erase( pos + 1 );
}


void trimLeft( std::string& str,
              const std::string& trimChars = whiteSpaces )
{
    std::string::size_type pos = str.find_first_not_of( trimChars );
    str.erase( 0, pos );
}


void trim( std::string& str, const std::string& trimChars = whiteSpaces )
{
    for (size_t i = 0; i < str.length(); ++i)
        if (!isalnum(str[i]))
            str.erase(i, 1);
    
    trimRight( str, trimChars );
    trimLeft( str, trimChars );
}


std::string random_string(int len)
{
    srand(time(0));
    std::string str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    int pos;
    while(str.size() != len) {
        pos = ((rand() % (str.size() - 1)));
        str.erase (pos, 1);
    }
    return str;
}

#endif
