#include "includes/utils.hpp"
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#include <direct.h>  // For _getcwd on Windows
#else
#include <unistd.h>  // For getcwd on POSIX systems
#endif

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace cppress::shared {

// Helper functions for web-related tasks

std::string url_encode(const std::string& value) {
    std::ostringstream escaped;
    for (const char& c : value) {
        if (isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' ||
            c == '~') {
            escaped << c;
        } else {
            // Use unsigned char cast to avoid sign-extension for non-ascii values
            escaped << '%' << std::uppercase << std::setw(2) << std::setfill('0') << std::hex
                    << (static_cast<int>(static_cast<unsigned char>(c)) & 0xFF);
            // Reset formatting flags
            escaped << std::dec;
        }
    }
    return escaped.str();
}

std::string url_decode(const std::string& value) {
    std::string decoded;
    for (size_t i = 0; i < value.length(); ++i) {
        if (value[i] == '%') {
            if (i + 2 < value.length()) {
                std::string hex = value.substr(i + 1, 2);
                char decoded_char = static_cast<char>(std::stoi(hex, nullptr, 16));
                decoded += decoded_char;
                i += 2;
            }
        } else {
            decoded += value[i];
        }
    }
    return decoded;
}

const std::vector<std::string> static_extensions = {
    // Web Documents
    "html", "htm", "xhtml", "xml",
    // Stylesheets
    "css", "scss", "sass", "less",
    // JavaScript
    "js", "mjs", "jsx", "ts", "tsx",
    // Images
    "png", "jpg", "jpeg", "gif", "bmp", "tiff", "tif", "svg", "webp", "ico", "cur", "avif",
    // Fonts
    "woff", "woff2", "ttf", "otf", "eot",
    // Audio
    "mp3", "wav", "ogg", "m4a", "aac", "flac",
    // Video
    "mp4", "webm", "avi", "mov", "wmv", "flv", "mkv",
    // Documents
    "pdf", "doc", "docx", "xls", "xlsx", "ppt", "pptx", "txt", "rtf", "odt", "ods", "odp",
    // Archives
    "zip", "rar", "7z", "tar", "gz", "bz2",
    // Data formats
    "json", "csv", "yaml", "yml", "toml",
    // Web Manifests & Config
    "manifest", "webmanifest", "map", "htaccess",
    // Other common formats
    "swf", "eps", "ai", "psd", "sketch"};

const std::map<std::string, std::string> mime_types = {
    // Web Documents
    {"html", "text/html"},
    {"htm", "text/html"},
    {"xhtml", "application/xhtml+xml"},
    {"xml", "application/xml"},

    // Stylesheets
    {"css", "text/css"},
    {"scss", "text/x-scss"},
    {"sass", "text/x-sass"},
    {"less", "text/x-less"},

    // JavaScript
    {"js", "application/javascript"},
    {"mjs", "application/javascript"},
    {"jsx", "text/jsx"},
    {"ts", "application/typescript"},
    {"tsx", "text/tsx"},

    // Images
    {"png", "image/png"},
    {"jpg", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"gif", "image/gif"},
    {"bmp", "image/bmp"},
    {"tiff", "image/tiff"},
    {"tif", "image/tiff"},
    {"svg", "image/svg+xml"},
    {"webp", "image/webp"},
    {"ico", "image/x-icon"},
    {"cur", "image/x-icon"},
    {"avif", "image/avif"},

    // Fonts
    {"woff", "font/woff"},
    {"woff2", "font/woff2"},
    {"ttf", "font/ttf"},
    {"otf", "font/otf"},
    {"eot", "application/vnd.ms-fontobject"},

    // Audio
    {"mp3", "audio/mpeg"},
    {"wav", "audio/wav"},
    {"ogg", "audio/ogg"},
    {"m4a", "audio/mp4"},
    {"aac", "audio/aac"},
    {"flac", "audio/flac"},

    // Video
    {"mp4", "video/mp4"},
    {"webm", "video/webm"},
    {"avi", "video/x-msvideo"},
    {"mov", "video/quicktime"},
    {"wmv", "video/x-ms-wmv"},
    {"flv", "video/x-flv"},
    {"mkv", "video/x-matroska"},

    // Documents
    {"pdf", "application/pdf"},
    {"doc", "application/msword"},
    {"docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
    {"xls", "application/vnd.ms-excel"},
    {"xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
    {"ppt", "application/vnd.ms-powerpoint"},
    {"pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
    {"txt", "text/plain"},
    {"rtf", "application/rtf"},
    {"odt", "application/vnd.oasis.opendocument.text"},
    {"ods", "application/vnd.oasis.opendocument.spreadsheet"},
    {"odp", "application/vnd.oasis.opendocument.presentation"},

    // Archives
    {"zip", "application/zip"},
    {"rar", "application/vnd.rar"},
    {"7z", "application/x-7z-compressed"},
    {"tar", "application/x-tar"},
    {"gz", "application/gzip"},
    {"bz2", "application/x-bzip2"},

    // Data formats
    {"json", "application/json"},
    {"csv", "text/csv"},
    {"yaml", "application/x-yaml"},
    {"yml", "application/x-yaml"},
    {"toml", "application/toml"},

    // Web Manifests & Config
    {"manifest", "text/cache-manifest"},
    {"webmanifest", "application/manifest+json"},
    {"map", "application/json"},
    {"htaccess", "text/plain"},

    // Other common formats
    {"swf", "application/x-shockwave-flash"},
    {"eps", "application/postscript"},
    {"ai", "application/postscript"},
    {"psd", "image/vnd.adobe.photoshop"},
    {"sketch", "application/x-sketch"}};

std::string get_mime_type_from_extension(const std::string& extension) {
    auto it = mime_types.find(extension);
    if (it != mime_types.end()) {
        return it->second;
    }
    return "application/octet-stream";
}

std::string get_file_extension_from_mime(const std::string& mime_type) {
    for (const auto& pair : mime_types) {
        if (pair.second == mime_type) {
            return pair.first;
        }
    }
    return "";
}

std::string get_file_extension_from_uri(const std::string& uri) {
    size_t dot_pos = uri.find_last_of('.');
    if (dot_pos != std::string::npos) {
        return uri.substr(dot_pos + 1);
    }
    return "";
}

std::string sanitize_path(const std::string& path) {
    std::string sanitized = path;
    size_t pos;
    while ((pos = sanitized.find("..")) != std::string::npos) {
        sanitized.erase(pos, 2);
    }
    return sanitized;
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    return (first == std::string::npos || last == std::string::npos)
               ? ""
               : str.substr(first, last - first + 1);
}

bool unknown_method(const std::string& method) {
    static const std::vector<std::string> known_methods = {
        methods::GET,   methods::POST, methods::PUT,    methods::DELETE_,
        methods::PATCH, methods::HEAD, methods::OPTIONS};
    return std::find(known_methods.begin(), known_methods.end(), method) == known_methods.end();
}

std::string to_lowercase(const std::string& str) {
    std::string lower_case_str = str;
    std::transform(lower_case_str.begin(), lower_case_str.end(), lower_case_str.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return lower_case_str;
}

std::string to_uppercase(const std::string& str) {
    std::string upper_case_str = str;
    std::transform(upper_case_str.begin(), upper_case_str.end(), upper_case_str.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return upper_case_str;
}

std::string get_current_working_directory() {
    const int PATH_MAX = 4096;
    char buffer[PATH_MAX];
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    if (_getcwd(buffer, sizeof(buffer)) != nullptr) {
        return std::string(buffer);
    } else {
        throw std::runtime_error("Failed to get current working directory");
    }
#else
    if (getcwd(buffer, sizeof(buffer)) != nullptr) {
        return std::string(buffer);
    } else {
        throw std::runtime_error("Failed to get current working directory");
    }
#endif
}
}  // namespace cppress::shared
