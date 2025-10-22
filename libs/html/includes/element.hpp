#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace cppress::html {

/**
 * @brief Core HTML element representation with hierarchical structure support and STL-like
 * interface.
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
 * - Dynamic attribute management with key-value pairs (map-like interface)
 * - Hierarchical child element support using smart pointers (vector-like interface)
 * - STL-like container operations (size(), empty(), at(), begin(), end(), etc.)
 * - Iterator support for traversing child elements
 * - Text content handling for leaf nodes
 * - Recursive parameter setting for template-like functionality
 * - Deep copying capabilities for element duplication
 * - String serialization for HTML output generation
 *
 * @note Elements use shared_ptr for child management to ensure proper memory
 *       management and allow safe sharing of elements across different contexts.
 * @note The class is designed to be extended for specialized element types
 *       such as self-closing elements or custom components.
 * @note The STL-like interface provides consistency with other cppress libraries (e.g., JSON)
 */
class element {
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
    // STL-like type aliases for children container
    using value_type = std::shared_ptr<element>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = std::vector<value_type>::iterator;
    using const_iterator = std::vector<value_type>::const_iterator;
    using reverse_iterator = std::vector<value_type>::reverse_iterator;
    using const_reverse_iterator = std::vector<value_type>::const_reverse_iterator;

    // STL-like type aliases for attributes container
    using attribute_key_type = std::string;
    using attribute_value_type = std::string;
    using attribute_type = std::pair<const attribute_key_type, attribute_value_type>;
    using attributes_iterator = std::map<attribute_key_type, attribute_value_type>::iterator;
    using attributes_const_iterator =
        std::map<attribute_key_type, attribute_value_type>::const_iterator;

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
    element(const std::string& tag);

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
    element(const std::string& tag, const std::string& text_content);

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
    element(const std::string& tag, const std::map<std::string, std::string>& attributes);

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
    element(const std::string& tag, const std::string& text_content,
            const std::map<std::string, std::string>& attributes);

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

    /**
     * @brief Virtual default destructor
     */
    virtual ~element() = default;

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
    virtual void set_text_content(const std::string& text_content);

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
    virtual void set_params_recursive(const std::map<std::string, std::string>& params);

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
    virtual void set_params(const std::map<std::string, std::string>& params);

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

    // STL-like methods for children management

    /**
     * @brief Returns the number of child elements.
     * @return The number of children.
     */
    size_type size() const noexcept;

    /**
     * @brief Checks if the element has no children.
     * @return true if there are no children, false otherwise.
     */
    bool empty() const noexcept;

    /**
     * @brief Accesses a child element by index with bounds checking.
     * @param index The index of the child to access.
     * @return A reference to the child element.
     * @throws std::out_of_range if index is out of bounds.
     */
    reference at(size_type index);

    /**
     * @brief Accesses a child element by index with bounds checking (const version).
     * @param index The index of the child to access.
     * @return A const reference to the child element.
     * @throws std::out_of_range if index is out of bounds.
     */
    const_reference at(size_type index) const;

    /**
     * @brief Adds a child element to the end (alias for add_child).
     * @param child Shared pointer to the child element to add.
     */
    void push_back(std::shared_ptr<element> child);

    /**
     * @brief Removes the last child element.
     * @throws std::out_of_range if there are no children.
     */
    void pop_back();

    /**
     * @brief Accesses the first child element.
     * @return A reference to the first child.
     * @throws std::out_of_range if there are no children.
     */
    reference front();

    /**
     * @brief Accesses the first child element (const version).
     * @return A const reference to the first child.
     * @throws std::out_of_range if there are no children.
     */
    const_reference front() const;

    /**
     * @brief Accesses the last child element.
     * @return A reference to the last child.
     * @throws std::out_of_range if there are no children.
     */
    reference back();

    /**
     * @brief Accesses the last child element (const version).
     * @return A const reference to the last child.
     * @throws std::out_of_range if there are no children.
     */
    const_reference back() const;

    /**
     * @brief Removes all child elements.
     */
    void clear() noexcept;

    /**
     * @brief Reserves capacity for child elements.
     * @param capacity Minimum capacity to reserve.
     */
    void reserve(size_type capacity);

    /**
     * @brief Returns an iterator to the beginning of children.
     * @return Iterator to the first child.
     */
    iterator begin() noexcept;

    /**
     * @brief Returns a const iterator to the beginning of children.
     * @return Const iterator to the first child.
     */
    const_iterator begin() const noexcept;

    /**
     * @brief Returns a const iterator to the beginning of children.
     * @return Const iterator to the first child.
     */
    const_iterator cbegin() const noexcept;

    /**
     * @brief Returns an iterator to the end of children.
     * @return Iterator to one past the last child.
     */
    iterator end() noexcept;

    /**
     * @brief Returns a const iterator to the end of children.
     * @return Const iterator to one past the last child.
     */
    const_iterator end() const noexcept;

    /**
     * @brief Returns a const iterator to the end of children.
     * @return Const iterator to one past the last child.
     */
    const_iterator cend() const noexcept;

    /**
     * @brief Returns a reverse iterator to the beginning.
     * @return Reverse iterator to the first child of reversed container.
     */
    reverse_iterator rbegin() noexcept;

    /**
     * @brief Returns a const reverse iterator to the beginning.
     * @return Const reverse iterator to the first child of reversed container.
     */
    const_reverse_iterator rbegin() const noexcept;

    /**
     * @brief Returns a const reverse iterator to the beginning.
     * @return Const reverse iterator to the first child of reversed container.
     */
    const_reverse_iterator crbegin() const noexcept;

    /**
     * @brief Returns a reverse iterator to the end.
     * @return Reverse iterator to one past the last child of reversed container.
     */
    reverse_iterator rend() noexcept;

    /**
     * @brief Returns a const reverse iterator to the end.
     * @return Const reverse iterator to one past the last child of reversed container.
     */
    const_reverse_iterator rend() const noexcept;

    /**
     * @brief Returns a const reverse iterator to the end.
     * @return Const reverse iterator to one past the last child of reversed container.
     */
    const_reverse_iterator crend() const noexcept;

    // STL-like methods for attributes management

    /**
     * @brief Sets or updates an attribute value.
     * @param key The attribute name.
     * @param value The attribute value.
     */
    void set_attribute(const std::string& key, const std::string& value);

    /**
     * @brief Removes an attribute by key.
     * @param key The attribute name to remove.
     * @return The number of attributes removed (0 or 1).
     */
    size_type erase_attribute(const std::string& key);

    /**
     * @brief Checks if an attribute exists.
     * @param key The attribute name to check.
     * @return true if the attribute exists, false otherwise.
     */
    bool has_attribute(const std::string& key) const;

    /**
     * @brief Returns the number of attributes.
     * @return The number of attributes.
     */
    size_type attributes_size() const noexcept;

    /**
     * @brief Checks if the element has no attributes.
     * @return true if there are no attributes, false otherwise.
     */
    bool attributes_empty() const noexcept;

    /**
     * @brief Returns an iterator to the beginning of attributes.
     * @return Iterator to the first attribute.
     */
    attributes_iterator attributes_begin() noexcept;

    /**
     * @brief Returns a const iterator to the beginning of attributes.
     * @return Const iterator to the first attribute.
     */
    attributes_const_iterator attributes_begin() const noexcept;

    /**
     * @brief Returns a const iterator to the beginning of attributes.
     * @return Const iterator to the first attribute.
     */
    attributes_const_iterator attributes_cbegin() const noexcept;

    /**
     * @brief Returns an iterator to the end of attributes.
     * @return Iterator to one past the last attribute.
     */
    attributes_iterator attributes_end() noexcept;

    /**
     * @brief Returns a const iterator to the end of attributes.
     * @return Const iterator to one past the last attribute.
     */
    attributes_const_iterator attributes_end() const noexcept;

    /**
     * @brief Returns a const iterator to the end of attributes.
     * @return Const iterator to one past the last attribute.
     */
    attributes_const_iterator attributes_cend() const noexcept;

    // Operator overloads

    /**
     * @brief Accesses a child element by index without bounds checking.
     * @param index The index of the child.
     * @return A reference to the child element.
     */
    reference operator[](size_type index);

    /**
     * @brief Accesses a child element by index without bounds checking (const version).
     * @param index The index of the child.
     * @return A const reference to the child element.
     */
    const_reference operator[](size_type index) const;

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
    std::string get_attribute(const std::string& key) const;
};

}  // namespace cppress::html