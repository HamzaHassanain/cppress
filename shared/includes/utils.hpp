#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace cppress::shared {

namespace methods {
constexpr const char* GET = "GET";
constexpr const char* POST = "POST";
constexpr const char* PUT = "PUT";
constexpr const char* DELETE_ = "DELETE";
constexpr const char* HEAD = "HEAD";
constexpr const char* OPTIONS = "OPTIONS";
constexpr const char* PATCH = "PATCH";
};  // namespace methods

/**
 * @brief List of known static file extensions that should be treated as static resources.
 *
 * This list is used by is_uri_static() to determine whether a request URI refers
 * to a static asset (CSS, JS, images, fonts, etc.) and therefore should be
 * served from the static file directories instead of being routed to handlers.
 */
extern const std::vector<std::string> static_extensions;

/**
 * @brief Mapping from file extension to MIME type.
 *
 * This map is used to convert file extensions to the appropriate Content-Type
 * header when serving static files. If an extension is not found the default
 * MIME type "application/octet-stream" should be used.
 */
extern const std::map<std::string, std::string> mime_types;

/**
 * @brief URL-encode a string according to RFC 3986.
 * @param value Input string to encode
 * @return Encoded string where reserved characters are percent-encoded
 */
std::string url_encode(const std::string& value);

/**
 * @brief Decode a percent-encoded URL string.
 * @param value Percent-encoded input string
 * @return Decoded string
 */
std::string url_decode(const std::string& value);

/**
 * @brief Get MIME type for a given file extension.
 * @param extension File extension without the leading dot (e.g., "js", "html")
 * @return MIME type string or "application/octet-stream" if unknown
 */
std::string get_mime_type_from_extension(const std::string& extension);

/**
 * @brief Find a file extension associated with a MIME type.
 * @param mime_type MIME type string (e.g., "application/json")
 * @return Extension string without dot or empty string if not found
 */
std::string get_file_extension_from_mime(const std::string& mime_type);

/**
 * @brief Extract file extension from a URI or filename.
 * @param uri URI or path string
 * @return Extension without the dot, or empty string if none found
 */
std::string get_file_extension_from_uri(const std::string& uri);

/**
 * @brief Sanitize a requested path to mitigate directory traversal.
 * @param path Raw path from the request URI
 * @return Sanitized path with dangerous sequences removed
 *
 * This function performs simple sanitization such as removing ".." components.
 * It does not resolve symbolic links or perform filesystem canonicalization;
 * callers that need full security should canonicalize on the server side before
 * opening files.
 */
std::string sanitize_path(const std::string& path);

/**
 * @brief Trim leading and trailing whitespace from a string.
 * @param str Input string
 * @return Trimmed string
 */
std::string trim(const std::string& str);

/**
 * @brief Check if an HTTP method is unknown.
 * @param method HTTP method string (e.g., "GET", "POST")
 * @return true if the method is unknown, false otherwise
 */
bool unknown_method(const std::string& method);

/**
 * @brief Convert a string to lowercase.
 * @param str Input string
 * @return Lowercase version of the input string
 */
std::string to_lowercase(const std::string& str);

/**
 * @brief Convert a string to uppercase.
 * @param str Input string
 * @return Uppercase version of the input string
 */
std::string to_uppercase(const std::string& str);

}  // namespace cppress::shared
