#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

namespace hh_html_builder
{

    /**
     * @brief Core HTML element representation with hierarchical structure support.
     *
     * This class provides a comprehensive representation of HTML elements, supporting
     * all standard HTML features including tags, text content, attributes, and nested
     * child elements. It serves as the fundamental building block for HTML document
     * construction and manipulation within the HTML builder framework.
     *
     * The class supports both simple text-based elements and complex nested structures,
     * making it suitable for building complete HTML documents programmatically.
     * It provides methods for attribute management, content manipulation, and
     * hierarchical element relationships.
     *
     * Key features:
     * - Dynamic attribute management with key-value pairs
     * - Hierarchical child element support using smart pointers
     * - Text content handling for leaf nodes
     * - Recursive parameter setting for template-like functionality
     * - Deep copying capabilities for element duplication
     * - String serialization for HTML output generation
     *
     * @note Elements use shared_ptr for child management to ensure proper memory
     *       management and allow safe sharing of elements across different contexts.
     * @note The class is designed to be extended for specialized element types
     *       such as self-closing elements or custom components.
     */
    class element
    {
    protected:
        /// HTML tag name (e.g., "div", "p", "span", "h1")
        std::string tag;

        /// Text content contained within the element
        std::string text_content;

        /// HTML attributes as key-value pairs (e.g., {"class", "container"}, {"id", "main"})
        std::map<std::string, std::string> attributes;

        /// Child elements forming the hierarchical structure
        std::vector<std::shared_ptr<element>> children;

    public:
        /**
         * @brief Default constructor creating an empty element.
         *
         * Creates an element with no tag, content, attributes, or children.
         * This constructor is useful when the element details will be set
         * later through setter methods or when creating placeholder elements.
         */
        element();

        /**
         * @brief Construct element with specified tag name.
         * @param tag HTML tag name for the element
         *
         * Creates an element with the specified tag name but no content,
         * attributes, or children. This is the most common constructor for
         * creating basic HTML elements that will be populated later.
         *
         * Example: element("div") creates a <div></div> element
         */
        element(const std::string &tag);

        /**
         * @brief Construct element with tag name and text content.
         * @param tag HTML tag name for the element
         * @param text_content Text content to be placed inside the element
         *
         * Creates an element with both tag name and text content. This constructor
         * is ideal for simple text-containing elements like paragraphs, headings,
         * or labels that don't require additional attributes.
         *
         * Example: element("p", "Hello World") creates <p>Hello World</p>
         */
        element(const std::string &tag, const std::string &text_content);

        /**
         * @brief Construct element with tag name and attributes.
         * @param tag HTML tag name for the element
         * @param attributes Map of attribute name-value pairs
         *
         * Creates an element with specified tag and attributes but no text content.
         * This constructor is useful for elements that need styling or behavior
         * attributes but will have their content added later or through child elements.
         *
         * Example: element("div", {{"class", "container"}, {"id", "main"}})
         * creates <div class="container" id="main"></div>
         */
        element(const std::string &tag, const std::map<std::string, std::string> &attributes);

        /**
         * @brief Construct element with tag name, text content, and attributes.
         * @param tag HTML tag name for the element
         * @param text_content Text content to be placed inside the element
         * @param attributes Map of attribute name-value pairs
         *
         * Creates a fully specified element with tag, content, and attributes.
         * This constructor provides complete element initialization in a single
         * call, making it convenient for creating complex elements with all
         * properties defined upfront.
         *
         * Example: element("a", "Click here", {{"href", "https://example.com"}, {"target", "_blank"}})
         * creates <a href="https://example.com" target="_blank">Click here</a>
         */
        element(const std::string &tag, const std::string &text_content, const std::map<std::string, std::string> &attributes);

        /**
         * @brief Add a child element to this element's hierarchy.
         * @param child Shared pointer to the child element to add
         *
         * Appends the specified element as a child of this element, creating
         * a parent-child relationship in the HTML structure. The child element
         * is managed through a shared pointer, ensuring proper memory management
         * and allowing the same element to be referenced from multiple contexts.
         *
         * This method enables building complex nested HTML structures by
         * assembling elements hierarchically. Child elements will be rendered
         * inside the parent element when converting to HTML string.
         *
         * @note Virtual method allows specialized element types to override
         *       child addition behavior if needed.
         */
        virtual void add_child(std::shared_ptr<element> child);

        /**
         * @brief Set or update the text content of this element.
         * @param text_content New text content for the element
         *
         * Updates the element's text content, replacing any existing content.
         * This method is used to set the inner text that appears between the
         * opening and closing tags of the element. Setting text content is
         * typically done for leaf elements that don't contain child elements.
         *
         * @note Setting text content on an element that has child elements
         *       may result in both text and child elements being rendered,
         *       depending on the implementation of to_string().
         */
        virtual void set_text_content(const std::string &text_content);

        /**
         * @brief Recursively set parameters on this element and all descendants.
         * @param params Map of parameter name-value pairs to apply
         *
         * Applies the specified parameters to this element and recursively
         * propagates them to all child elements in the hierarchy. This method
         * is useful for template-like functionality where certain values need
         * to be applied throughout an entire element tree.
         *
         * Common use cases include:
         * - Setting global styling parameters across a component
         * - Applying configuration values to template elements
         * - Bulk updating attributes across nested structures
         *
         * The recursive nature ensures that even deeply nested elements
         * receive the parameter updates, making it powerful for comprehensive
         * element tree modifications.
         */
        virtual void set_params_recursive(const std::map<std::string, std::string> &params);

        /**
         * @brief Set parameters on this element only (non-recursive).
         * @param params Map of parameter name-value pairs to apply
         *
         * Applies the specified parameters only to this element, without
         * affecting any child elements. This method provides fine-grained
         * control for updating individual elements without cascading changes
         * to the entire hierarchy.
         *
         * The exact behavior of parameter application depends on the
         * implementation, but typically involves updating attributes,
         * text content placeholders, or other element properties based
         * on the provided parameter mappings.
         */
        virtual void set_params(const std::map<std::string, std::string> &params);

        /**
         * @brief Create a deep copy of this element.
         * @return New element instance that is a complete copy of this element
         *
         * Creates an independent copy of this element, including all attributes,
         * text content, and recursively copying all child elements. The returned
         * element is completely separate from the original and can be modified
         * without affecting the source element.
         *
         * This method is essential for template functionality, element replication,
         * and scenarios where multiple similar elements need to be created from
         * a base template without sharing state.
         *
         * @note The copy includes the entire element hierarchy, so copying
         *       elements with many children may be memory-intensive.
         */
        virtual element copy() const;

        /**
         * @brief Get the text content of this element.
         * @return String containing the element's text content
         *
         * Returns the current text content stored within this element.
         * This is the text that appears between the opening and closing
         * tags when the element is rendered to HTML. For elements without
         * text content, this returns an empty string.
         */
        virtual std::string get_text_content() const;

        /**
         * @brief Get all child elements of this element.
         * @return Vector of shared pointers to child elements
         *
         * Returns a vector containing shared pointers to all child elements
         * that have been added to this element. The order of elements in
         * the vector reflects the order they were added and the order they
         * will appear in the rendered HTML output.
         *
         * @note Returns a copy of the children vector, so modifications
         *       to the returned vector do not affect the element's children.
         */
        virtual std::vector<std::shared_ptr<element>> get_children() const;

        /**
         * @brief Convert this element and its hierarchy to HTML string representation.
         * @return String containing the complete HTML markup for this element
         *
         * Generates a complete HTML string representation of this element,
         * including its tag, attributes, text content, and all child elements.
         * This method recursively processes the entire element hierarchy to
         * produce valid HTML markup.
         *
         * The output includes:
         * - Opening tag with all attributes properly formatted
         * - Text content (if any)
         * - Recursively rendered child elements
         * - Closing tag
         *
         * This is the primary method for converting the programmatic element
         * structure into actual HTML that can be written to files or sent
         * to web browsers.
         */
        virtual std::string to_string() const;

        /**
         * @brief Get the HTML tag name of this element.
         * @return String containing the tag name
         *
         * Returns the HTML tag name for this element (e.g., "div", "p", "span").
         * This is a read-only accessor for the tag property, useful for
         * inspecting element types or implementing element-specific logic.
         */
        std::string get_tag() const;

        /**
         * @brief Get all attributes of this element.
         * @return Map containing all attribute name-value pairs
         *
         * Returns a copy of the complete attribute map for this element.
         * The map contains all HTML attributes that will be included in
         * the element's opening tag when rendered to HTML.
         *
         * @note Returns a copy of the attributes map, so modifications
         *       to the returned map do not affect the element's attributes.
         */
        std::map<std::string, std::string> get_attributes() const;

        /**
         * @brief Get the value of a specific attribute.
         * @param key Name of the attribute to retrieve
         * @return String containing the attribute value, or empty string if not found
         *
         * Retrieves the value of a specific attribute by name. If the attribute
         * does not exist on this element, returns an empty string. This method
         * provides convenient access to individual attribute values without
         * needing to work with the entire attributes map.
         *
         * Example: For an element with class="container", calling
         * get_attribute("class") returns "container".
         */
        std::string get_attribute(const std::string &key) const;
    };

}