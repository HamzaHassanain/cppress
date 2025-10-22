#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>

#include "element.hpp"
#include "self_closing_element.hpp"

namespace hh_html_builder
{
    /**
     * @brief Parse HTML string into a collection of element objects.
     * @param html Reference to HTML string to parse (may be modified during parsing)
     * @return Vector of shared pointers to parsed element objects
     *
     * Converts a raw HTML string into a structured collection of element objects
     * that can be manipulated programmatically. This function analyzes the HTML
     * markup, identifies tags, attributes, and content, and constructs the
     * appropriate element hierarchy.
     *
     * The parser handles:
     * - Standard HTML elements with opening and closing tags
     * - Self-closing/void elements (img, br, hr, etc.)
     * - Nested element structures and hierarchies
     * - HTML attributes with proper value extraction
     * - Text content between elements
     * - Malformed HTML with reasonable error recovery
     *
     * The function returns a vector of top-level elements, where each element
     * may contain its own hierarchy of child elements. This structure mirrors
     * the original HTML document organization and allows for easy manipulation
     * and reconstruction.
     *
     * Example usage:
     * ```cpp
     * std::string html = "<div class='container'><p>Hello</p><br><p>World</p></div>";
     * auto elements = parse_html_string(html);
     * // Returns vector with one div element containing p, br, and p children
     * ```
     *
     * @note The input HTML string may be modified during parsing for optimization
     * @note The parser automatically detects and creates appropriate element types
     *       (regular elements vs. self-closing elements)
     * @note Returns empty vector if the HTML string is empty or contains no valid elements
     */
    std::vector<std::shared_ptr<element>> parse_html_string(std::string &html);

    /**
     * @brief Parse HTML template string with parameter substitution.
     * @param text HTML template string containing parameter placeholders
     * @param params Map of parameter names to replacement values
     * @return Processed HTML string with parameters substituted
     *
     * Processes an HTML template string by replacing parameter placeholders
     * with actual values from the provided parameter map. This function enables
     * template-based HTML generation where dynamic content can be injected
     * into predefined HTML structures.
     *
     * The parameter substitution mechanism allows for:
     * - Dynamic content insertion into HTML templates
     * - Reusable HTML components with configurable values
     * - Separation of HTML structure from dynamic data
     * - Template-based document generation workflows
     *
     * Parameter placeholders in the HTML template are typically marked with
     * special syntax (such as {{paramName}} or ${paramName}) that the parser
     * recognizes and replaces with corresponding values from the params map.
     *
     * Example usage:
     * ```cpp
     * std::string template_html = "<h1>{{title}}</h1><p>Welcome, {{username}}!</p>";
     * std::map<std::string, std::string> params = {
     *     {"title", "Dashboard"},
     *     {"username", "John Doe"}
     * };
     * std::string result = parse_html_with_params(template_html, params);
     * // Returns: "<h1>Dashboard</h1><p>Welcome, John Doe!</p>"
     * ```
     *
     * @note Unmatched parameter placeholders may be left unchanged or removed
     *       depending on implementation
     * @note Parameter values are inserted as-is, so HTML escaping should be
     *       handled separately if needed for security
     * @note This function returns a processed string rather than element objects
     */
    std::string parse_html_with_params(const std::string &text, const std::map<std::string, std::string> &params);

    /**
     * @brief Internal optimized parsing function for HTML string segments.
     * @param html HTML string to parse
     * @param start Starting position within the HTML string
     * @param end Ending position within the HTML string
     * @return Pair containing parsed elements vector and final parsing position
     *
     * Internal optimization function designed for efficient parsing of HTML
     * string segments. This function is typically used internally by the main
     * parsing functions to process specific portions of HTML documents without
     * needing to reprocess the entire string.
     *
     * Key features:
     * - Segment-based parsing for improved performance on large documents
     * - Position tracking to enable incremental parsing workflows
     * - Optimized for scenarios requiring partial HTML processing
     * - Returns both results and parsing state for continued processing
     *
     * The function returns a pair where:
     * - First element: Vector of parsed element objects from the specified segment
     * - Second element: Final position reached during parsing (useful for continuation)
     *
     * This internal function enables advanced parsing scenarios such as:
     * - Streaming HTML processing for large documents
     * - Incremental parsing with state preservation
     * - Performance optimization for repeated parsing operations
     * - Custom parsing workflows with fine-grained control
     *
     * @note This is an internal function and should typically not be called
     *       directly by end users of the library
     * @note The start and end positions should be valid indices within the HTML string
     * @note Performance characteristics may vary based on HTML complexity and segment size
     */
    std::pair<std::vector<std::shared_ptr<element>>, size_t> parse_html_optimized(const std::string &html, size_t start, size_t end);
}