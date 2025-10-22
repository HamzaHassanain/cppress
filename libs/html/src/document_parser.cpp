

#include <iostream>
#include <algorithm>
#include <map>
#include <stdexcept>
#include <stack>
#include <sstream>
#include <set>
#include <cctype>

#include <thread>
#include <chrono>

#include "../includes/document_parser.hpp"
#include "../includes/element.hpp"
#include "../includes/doctype_element.hpp"
#include "../includes/self_closing_element.hpp"
namespace hh_html_builder
{
    /**
     * @brief Check if HTML string contains a DOCTYPE declaration.
     * @param html HTML string to check
     * @return true if DOCTYPE is found, false otherwise
     */
    bool has_doctype(const std::string &html)
    {
        return html.find("<!doctype") != std::string::npos;
    }

    /**
     * @brief Extract and remove DOCTYPE declaration from HTML string.
     * @param html HTML string (modified by removing the DOCTYPE)
     * @return Extracted DOCTYPE string, or empty string if not found
     *
     * Finds the DOCTYPE declaration, extracts it, and removes it from the
     * original HTML string to prepare for further parsing.
     */
    std::string extract_doctype(std::string &html)
    {
        size_t start = html.find("<!doctype");
        if (start == std::string::npos)
            return "";

        size_t end = html.find('>', start);
        if (end == std::string::npos)
            return "";
        // remove it form the html itself
        auto doctype = html.substr(start, end - start + 1);
        html.erase(start, end - start + 1);
        return doctype;
    }

    /**
     * @brief Convert all HTML tag names to lowercase for consistent parsing.
     * @param html HTML string to modify (tags are converted to lowercase)
     *
     * Normalizes HTML by converting tag names to lowercase while preserving
     * attribute values, classes, and other content. This ensures consistent
     * parsing regardless of the original case used in the HTML.
     */
    void transform_tags_to_lower_case(std::string &html)
    {
        size_t pos = 0;
        while ((pos = html.find('<', pos)) != std::string::npos)
        {
            size_t end_pos = html.find('>', pos);
            if (end_pos == std::string::npos)
                break;

            // do not decapitalize the attribute names, classes, and so on,
            size_t __end_pos = html.find(' ', pos + 1);
            if (__end_pos == std::string::npos)
                __end_pos = end_pos;
            __end_pos = std::min(__end_pos, end_pos);

            std::string tag = html.substr(pos + 1, __end_pos - pos - 1);
            std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
            html.replace(pos + 1, __end_pos - pos - 1, tag);
            pos = end_pos + 1;
        }
    }

    /**
     * @brief Remove leading and trailing whitespace from a string.
     * @param str String to trim
     * @return Trimmed string with whitespace removed from both ends
     */
    std::string trim(std::string &str)
    {
        size_t start = str.find_first_not_of(" \t\n\r");
        if (start == std::string::npos)
            return "";
        size_t end = str.find_last_not_of(" \t\n\r");
        return str.substr(start, end - start + 1);
    }

    /**
     * @brief Remove all line break characters from HTML string.
     * @param html HTML string to modify (line breaks are removed)
     *
     * Removes all '\n' characters to create a single-line HTML string,
     * which simplifies parsing by eliminating line-based formatting.
     */
    void remove_all_line_breaks(std::string &html)
    {
        html.erase(std::remove(html.begin(), html.end(), '\n'), html.end());
    }
    /**
     * @brief Parse HTML attribute string into key-value pairs.
     * @param attr_string String containing HTML attributes to parse
     * @return Map of attribute names to their values
     *
     *  Parser that handles various attribute formats:
     * - Simple attributes: class="value" id="test"
     * - Boolean attributes: disabled checked
     * - Quoted values with spaces and special characters
     * - Proper handling of equals signs and quote escaping
     */
    std::map<std::string, std::string> parse_attributes(std::string &attr_string)
    {
        std::map<std::string, std::string> attributes;

        if (attr_string.empty())
            return attributes;
        attr_string = trim(attr_string);
        std::string current = "";
        bool did_open_an_attribute = false;

        std::string current_key = "";
        for (int i = 0; i < static_cast<int>(attr_string.size()); i++)
        {
            char c = attr_string[i];
            if (c == '=' && !did_open_an_attribute)
            {
                current_key = trim(current);
                current = "";
            }
            else
            {
                current += c;
            }
            if (c == '"')
            {
                if (did_open_an_attribute)
                {
                    did_open_an_attribute = false;
                    current.erase(current.begin());
                    current.pop_back();
                    attributes[current_key] = current;
                    current = "";
                    current_key = "";
                }
                else
                {
                    did_open_an_attribute = true;
                }
            }
            else if (!did_open_an_attribute && (c == ' ' || c == '\t' || c == '\n'))
            {
                current_key = trim(current);
                current = "";
                if (!current_key.empty())
                    attributes[current_key] = "";
                current_key = "";
            }
        }

        if (current.size())
        {
            current_key = trim(current);
            if (!current_key.empty())
            {
                attributes[current_key] = "";
            }
        }

        attributes.erase("");
        attributes.erase("/");
        attributes.erase(" ");

        return attributes;
    }

    /**
     * @brief Get the set of HTML tag names that are self-closing.
     * @return Set containing all standard HTML self-closing tag names
     *
     * Returns a predefined set of HTML5 void elements that don't require
     * closing tags and cannot contain child content.
     */
    std::set<std::string> get_self_closing_tags()
    {
        return {
            "area", "base", "br", "col", "embed", "hr", "img", "input",
            "link", "meta", "param", "source", "track", "wbr"};
    }

    /**
     * @brief Check if a given tag name is a self-closing HTML element.
     * @param tag Tag name to check
     * @return true if the tag is self-closing, false otherwise
     */
    bool is_self_closing_tag(std::string &tag)
    {
        static std::set<std::string> self_closing_tags = get_self_closing_tags();
        std::string lower_tag = tag;
        std::transform(lower_tag.begin(), lower_tag.end(), lower_tag.begin(), ::tolower);
        return self_closing_tags.find(lower_tag) != self_closing_tags.end();
    }
    /**
     * @brief Check if a tag string represents a closing tag.
     * @param tag Tag string to check
     * @return true if it's a closing tag (starts with /), false otherwise
     */
    bool is_closing_tag(std::string &tag)
    {
        return tag.size() > 1 && tag[0] == '/' && tag[1] != '/';
    }
    /**
     * @brief Separate tag name from its attributes in a tag content string.
     * @param tag_content Full tag content (e.g., "div class='container' id='main'")
     * @return Pair containing tag name and attributes string
     */
    std::pair<std::string, std::string> extract_tag_and_attributes(std::string &tag_content)
    {
        size_t space_pos = tag_content.find(' ');
        if (space_pos == std::string::npos)
        {
            return {tag_content, ""};
        }

        std::string tag_name = tag_content.substr(0, space_pos);
        std::string attributes = tag_content.substr(space_pos + 1);
        return {tag_name, attributes};
    }
    /**
     * @brief Remove all HTML comments from the HTML string.
     * @param html HTML string to modify (comments are removed)
     *
     * Finds and removes all HTML comments (<!-- comment -->) from the string.
     * Throws runtime_error if a comment is malformed (missing closing tag).
     */
    void remove_all_comments(std::string &html)
    {
        size_t pos = 0;
        while ((pos = html.find("<!--", pos)) != std::string::npos)
        {
            size_t end_pos = html.find("-->", pos + 4);
            if (end_pos == std::string::npos)
                throw std::runtime_error("Malformed comment: no closing tag found");
            html.erase(pos, end_pos - pos + 3);
        }
    }
    /**
     * @brief Read the content of an HTML tag starting at a given position.
     * @param html HTML string to read from
     * @param pos Reference to current position (updated to closing '>')
     * @return Tag content between < and >
     *
     * Extracts tag content and updates the position to the closing '>'.
     * Throws runtime_error if no closing '>' is found.
     */
    std::string read_tag(const std::string &html, size_t &pos)
    {
        size_t start = pos + 1;
        size_t end = html.find('>', start);
        if (end == std::string::npos)
        {
            throw std::runtime_error("Malformed HTML: no closing '>' found");
        }

        std::string tag = html.substr(start, end - start);
        pos = end; // Update position to the closing '>'
        return tag;
    }
    /**
     * @brief Extract just the tag name from a tag string that may include attributes.
     * @param tag_with_atrs Tag string potentially containing attributes
     * @return Clean tag name without attributes
     *
     * Separates the tag name from any attributes by finding the first whitespace.
     */
    std::string fix_tag(std::string &tag_with_atrs)
    {
        std::string tag_name;
        for (size_t i = 0; i < tag_with_atrs.size(); ++i)
        {
            if (tag_with_atrs[i] == ' ' || tag_with_atrs[i] == '\t' || tag_with_atrs[i] == '\n')
            {
                tag_name = tag_with_atrs.substr(0, i);
                return tag_name;
            }
        }

        return tag_with_atrs;
    }

    /**
     * @brief Main recursive HTML parsing wrapper function.
     * @param html HTML string to parse
     * @return Vector of parsed element objects
     * @note Optimized to run in O(n) time complexity using single-pass parsing.
     *
     * Entry point for the recursive HTML parser. Delegates to the optimized
     * parsing algorithm that processes the entire HTML string in linear time.
     */
    std::vector<std::shared_ptr<element>> solve_recursive(std::string &html)
    {
        return parse_html_optimized(html, 0, html.length()).first;
    }

    /**
     * @brief Optimized O(n) HTML parser using single-pass algorithm.
     * @param html The HTML string to parse
     * @param start Starting position in the HTML string
     * @param end Ending position in the HTML string
     * @return A pair containing the parsed elements and the position after parsing
     *
     * Recursive parser that processes HTML in linear time using a single-pass
     * algorithm. This is the core parsing engine that handles:
     *
     * - Text content extraction between tags
     * - Opening and closing tag matching
     * - Self-closing element detection
     * - Attribute parsing and validation
     * - Nested element hierarchy construction
     * - Error detection for malformed HTML
     *
     * The algorithm works by:
     * 1. Scanning for the next '<' character
     * 2. Extracting and analyzing tag content
     * 3. Determining if it's opening, closing, or self-closing
     * 4. For opening tags, recursively parsing children until matching closing tag
     * 5. Building element hierarchy with proper parent-child relationships
     *
     * Returns both the parsed elements and the final parsing position to enable
     * efficient continuation of parsing at higher recursion levels.
     */
    std::pair<std::vector<std::shared_ptr<element>>, size_t> parse_html_optimized(const std::string &html, size_t start, size_t end)
    {
        std::vector<std::shared_ptr<element>> result;
        size_t pos = start;

        while (pos < end)
        {
            // Find next tag opening
            size_t tag_start = html.find('<', pos);

            // If no more tags, handle remaining text
            if (tag_start == std::string::npos || tag_start >= end)
            {
                if (pos < end)
                {
                    std::string text_content = html.substr(pos, end - pos);
                    if (!text_content.empty() && text_content.find_first_not_of(" \t\n\r") != std::string::npos)
                    {
                        auto text_element = std::make_shared<element>("", text_content);
                        result.push_back(text_element);
                    }
                }
                break;
            }

            // Handle text content before the tag
            if (tag_start > pos)
            {
                std::string text_content = html.substr(pos, tag_start - pos);
                if (!text_content.empty() && text_content.find_first_not_of(" \t\n\r") != std::string::npos)
                {
                    auto text_element = std::make_shared<element>("", text_content);
                    result.push_back(text_element);
                }
            }

            // Find tag closing
            size_t tag_end = html.find('>', tag_start);
            if (tag_end == std::string::npos || tag_end >= end)
            {
                throw std::runtime_error("Malformed HTML: no closing '>' found");
            }

            // Extract tag content
            std::string tag_content = html.substr(tag_start + 1, tag_end - tag_start - 1);

            // Skip empty tags
            if (tag_content.empty())
            {
                pos = tag_end + 1;
                continue;
            }

            // Check if it's a closing tag
            if (tag_content[0] == '/')
            {
                // This is a closing tag, return to parent level
                return {result, tag_start};
            }

            // Parse tag name and attributes
            auto [tag_name, attributes] = extract_tag_and_attributes(tag_content);
            tag_name = trim(tag_name);
            attributes = trim(attributes);
            auto parsed_attributes = parse_attributes(attributes);

            // Handle self-closing tags
            if (is_self_closing_tag(tag_name))
            {
                auto elm = std::make_shared<self_closing_element>(tag_name, parsed_attributes);
                result.push_back(elm);
                pos = tag_end + 1;
                continue;
            }

            // Handle regular opening tags
            auto opening_element = std::make_shared<element>(tag_name, parsed_attributes);

            // Recursively parse children
            auto [children, closing_pos] = parse_html_optimized(html, tag_end + 1, end);

            // Add children to the element
            for (const auto &child : children)
            {
                opening_element->add_child(child);
            }

            result.push_back(opening_element);

            // Find the actual closing tag position
            if (closing_pos < end)
            {
                size_t closing_tag_end = html.find('>', closing_pos);
                if (closing_tag_end == std::string::npos)
                {
                    throw std::runtime_error("Malformed HTML: no closing '>' found for closing tag");
                }

                // Verify this is the correct closing tag
                std::string closing_tag_content = html.substr(closing_pos + 1, closing_tag_end - closing_pos - 1);
                if (closing_tag_content.length() > 1 && closing_tag_content[0] == '/')
                {
                    std::string closing_tag_name = closing_tag_content.substr(1);
                    closing_tag_name = trim(closing_tag_name);

                    if (closing_tag_name != tag_name)
                    {
                        throw std::runtime_error("Unmatched closing tag: expected </" + tag_name + "> but found </" + closing_tag_name + ">");
                    }
                }

                pos = closing_tag_end + 1;
            }
            else
            {
                // No closing tag found, treat as self-closing or end of input
                pos = end;
            }
        }

        return {result, end};
    }

    /**
     * @brief Main entry point for parsing HTML strings into element objects.
     * @param html HTML string to parse (modified during processing)
     * @return Vector of parsed element objects including DOCTYPE if present
     *
     * High-level HTML parsing function that performs complete document processing:
     *
     * Preprocessing steps:
     * 1. Remove HTML comments to avoid parsing interference
     * 2. Normalize tag names to lowercase for consistent handling
     * 3. Remove line breaks for simplified parsing
     * 4. Extract and handle DOCTYPE declaration separately
     *
     * Main parsing:
     * 5. Parse the remaining HTML content into element hierarchy
     * 6. Combine DOCTYPE (if present) with parsed elements
     *
     * This function handles complete HTML documents and returns a vector
     * where the first element may be a DOCTYPE declaration followed by
     * the document's element structure.
     */
    std::vector<std::shared_ptr<element>> parse_html_string(std::string &html)
    {

        remove_all_comments(html);
        transform_tags_to_lower_case(html);
        remove_all_line_breaks(html);
        std::string doctype;
        if (has_doctype(html))
        {
            doctype = extract_doctype(html);
        }
        std::vector<std::shared_ptr<element>> result;

        if (!doctype.empty())
        {
            doctype.pop_back();
            doctype.erase(doctype.begin(), doctype.begin() + 9); // Remove "<!doctype "
            std::shared_ptr<element> doctype_element_ptr = std::make_shared<doctype_element>(doctype);
            result.insert(result.begin(), doctype_element_ptr);
        }

        auto solved = solve_recursive(html);

        result.insert(result.end(), solved.begin(), solved.end());

        return result;
    }

    /**
     * @brief Template-based HTML generation with parameter substitution.
     * @param text HTML template string containing parameter placeholders
     * @param params Map of parameter names to replacement values
     * @return Processed HTML string with parameters substituted
     *
     * Simple but effective templating system that replaces placeholders in the
     * format {{param_name}} with actual values from the parameter map.
     *
     * Process:
     * 1. Scan the template for {{parameter_name}} patterns
     * 2. Replace each pattern with the corresponding value from params map
     * 3. Continue until all placeholders are processed
     *
     * Use cases:
     * - Dynamic content injection into HTML templates
     * - User-specific data insertion (names, preferences, etc.)
     * - Configuration-driven HTML generation
     * - Reusable HTML components with variable content
     *
     * Example: "Hello {{name}}!" with params{"name": "World"} → "Hello World!"
     */
    std::string parse_html_with_params(const std::string &text, const std::map<std::string, std::string> &params)
    {
        std::string result = text;

        // Simple parameter substitution: {{param_name}}

        for (const auto &param : params)
        {
            std::string placeholder = "{{" + param.first + "}}";
            size_t pos = 0;
            while ((pos = result.find(placeholder, pos)) != std::string::npos)
            {
                result.replace(pos, placeholder.length(), param.second);
                pos += param.second.length();
            }
        }
        return result;
    }
}