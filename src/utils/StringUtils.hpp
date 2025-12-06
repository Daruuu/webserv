#pragma once

#include <string>

namespace string_utils {
    // Generic template for converting to string
    template <typename T>
    std::string toString(const T& value);
    
    // Specialized declarations (optional, for documentation)
    //std::string intToString(int value);
    //std::string longToString(long value);
    
    // Convert string to int
    //int stringToInt(const std::string& str, int defaultValue = 0);
    
    // Convert string to long
    //long stringToLong(const std::string& str, long defaultValue = 0);
}

// Include the template implementation
#include "StringUtils.tpp"
