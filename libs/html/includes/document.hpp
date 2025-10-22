#pragma once
#include <memory>
#include <string>

#include "element.hpp"

namespace cppress::html {

/**
 * @brief HTML document representation with STL-like interface.
 *
 * This class represents a complete HTML document with a DOCTYPE declaration
 * and root HTML element. It provides an STL-like interface for managing the
 * document structure, consistent with other cppress libraries.
 *
 * Key features:
 * - Automatic DOCTYPE declaration management
 * - Root element access and manipulation
 * - STL-like container operations for root children
 * - String serialization for complete HTML output
 *
 * @note The document automatically creates an <html> root element
 * @note All child elements are added to the root element
 */
class document {
private:
    std::shared_ptr<element> root;
    std::string doctype;

public:
    // STL-like type aliases
    using value_type = std::shared_ptr<element>;
    using size_type = std::size_t;
    using iterator = std::vector<value_type>::iterator;
    using const_iterator = std::vector<value_type>::const_iterator;

    /**
     * @brief Constructs an HTML document with specified DOCTYPE.
     * @param doctype The DOCTYPE declaration (default: "html" for HTML5).
     *
     * Creates a new HTML document with the specified DOCTYPE and an empty
     * root <html> element. The default DOCTYPE "html" produces the HTML5
     * declaration: <!DOCTYPE html>
     */
    document(const std::string& doctype = "html");

    /**
     * @brief Default destructor.
     */
    ~document() = default;

    /**
     * @brief Converts the complete document to an HTML string.
     * @return String containing the DOCTYPE and complete HTML markup.
     *
     * Generates a complete HTML document string including the DOCTYPE
     * declaration followed by the root element and all its children.
     */
    std::string to_string() const;

    /**
     * @brief Adds a child element to the document's root element.
     * @param elem Shared pointer to the element to add.
     *
     * Adds the specified element as a child of the document's root <html>
     * element. This is typically used to add <head> and <body> elements,
     * or any other top-level content.
     */
    void add_child(std::shared_ptr<element> elem);

    /**
     * @brief Alias for add_child() - adds an element to the document root.
     * @param elem Shared pointer to the element to add.
     */
    void push_back(std::shared_ptr<element> elem);

    /**
     * @brief Gets the root HTML element of the document.
     * @return Shared pointer to the root element.
     *
     * Provides access to the document's root <html> element for direct
     * manipulation or inspection. This allows advanced operations on the
     * document structure beyond simple child addition.
     */
    std::shared_ptr<element> get_root() const;

    /**
     * @brief Sets the DOCTYPE declaration.
     * @param doctype The new DOCTYPE string.
     *
     * Updates the document's DOCTYPE declaration. Common values include:
     * - "html" for HTML5
     * - Legacy DOCTYPE declarations for older HTML versions
     */
    void set_doctype(const std::string& doctype);

    /**
     * @brief Gets the DOCTYPE declaration.
     * @return The DOCTYPE string.
     */
    std::string get_doctype() const;

    /**
     * @brief Returns the number of direct children in the root element.
     * @return The number of root children.
     */
    size_type size() const noexcept;

    /**
     * @brief Checks if the document root has no children.
     * @return true if the root has no children, false otherwise.
     */
    bool empty() const noexcept;

    /**
     * @brief Removes all children from the root element.
     */
    void clear() noexcept;

    /**
     * @brief Returns an iterator to the beginning of root children.
     * @return Iterator to the first child of the root element.
     */
    iterator begin() noexcept;

    /**
     * @brief Returns a const iterator to the beginning of root children.
     * @return Const iterator to the first child of the root element.
     */
    const_iterator begin() const noexcept;

    /**
     * @brief Returns a const iterator to the beginning of root children.
     * @return Const iterator to the first child of the root element.
     */
    const_iterator cbegin() const noexcept;

    /**
     * @brief Returns an iterator to the end of root children.
     * @return Iterator to one past the last child of the root element.
     */
    iterator end() noexcept;

    /**
     * @brief Returns a const iterator to the end of root children.
     * @return Const iterator to one past the last child of the root element.
     */
    const_iterator end() const noexcept;

    /**
     * @brief Returns a const iterator to the end of root children.
     * @return Const iterator to one past the last child of the root element.
     */
    const_iterator cend() const noexcept;

    /**
     * @brief Accesses a root child by index with bounds checking.
     * @param index The index of the child to access.
     * @return A reference to the child element.
     * @throws std::out_of_range if index is out of bounds.
     */
    value_type& at(size_type index);

    /**
     * @brief Accesses a root child by index with bounds checking (const version).
     * @param index The index of the child to access.
     * @return A const reference to the child element.
     * @throws std::out_of_range if index is out of bounds.
     */
    const value_type& at(size_type index) const;

    /**
     * @brief Accesses a root child by index without bounds checking.
     * @param index The index of the child.
     * @return A reference to the child element.
     */
    value_type& operator[](size_type index);

    /**
     * @brief Accesses a root child by index without bounds checking (const version).
     * @param index The index of the child.
     * @return A const reference to the child element.
     */
    const value_type& operator[](size_type index) const;
};

}  // namespace cppress