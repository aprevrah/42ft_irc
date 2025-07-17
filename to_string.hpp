#pragma once
#include <sstream>
#include <string>

// Helper function for C++98 compatibility (std::to_string not available)
inline std::string to_string(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

inline std::string to_string(unsigned int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}
