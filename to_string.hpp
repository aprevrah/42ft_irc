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

// TODO: Call the file utils maybe
inline std::vector<std::string> split_string(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::istringstream       stream(str);
    std::string              token;

    while (std::getline(stream, token, delimiter)) {
        if (!token.empty()) {
            result.push_back(token);
        }
    }

    return result;
}