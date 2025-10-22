#pragma once

#include "element.hpp"

namespace hh_html_builder
{
    /**
     * @brief Specialized element class for HTML self-closing (void) elements.
     *
     * This class extends the base element class to handle HTML elements that
     * don't require closing tags and cannot contain child elements or text content.
     * Self-closing elements, also known as void elements, are a specific subset
     * of HTML elements that represent standalone content or functionality.
     *
     * Common self-closing elements include:
     * - `<img>` for images
     * - `<br>` for line breaks
     * - `<hr>` for horizontal rules
     * - `<input>` for form inputs
     * - `<meta>` for metadata
     * - `<link>` for external resources
     * - `<area>` for image map areas
     * - `<base>` for document base URL
     * - `<col>` for table columns
     * - `<embed>` for embedded content
     * - `<source>` for media resources
     * - `<track>` for media tracks
     * - `<wbr>` for word break opportunities
     *
     * The class enforces the constraints of self-closing elements by:
     * - Preventing addition of child elements
     * - Preventing setting of text content
     * - Rendering with self-closing syntax (e.g., `<img />` or `<br>`)
     *
     * @note Self-closing elements are rendered without closing tags in HTML5
     * @note Attempts to add children or text content to self-closing elements
     *       should be handled gracefully (typically ignored or throwing exceptions)
     * @note This class provides specialized behavior for elements that are
     *       inherently self-contained and cannot have content
     */
    class self_closing_element : public element
    {
    public:
        /**
         * @brief Construct a self-closing element with specified tag name.
         * @param tag HTML tag name for the self-closing element
         *
         * Creates a self-closing element with the specified tag name but no
         * attributes. This constructor is suitable for simple self-closing
         * elements that don't require additional attributes.
         *
         * Examples:
         * - self_closing_element("br") creates a line break element
         * - self_closing_element("hr") creates a horizontal rule element
         */
        self_closing_element(const std::string &tag);

        /**
         * @brief Construct a self-closing element with tag name and attributes.
         * @param tag HTML tag name for the self-closing element
         * @param attributes Map of attribute name-value pairs
         *
         * Creates a self-closing element with the specified tag name and attributes.
         * This constructor is ideal for self-closing elements that require
         * attributes for proper functionality, such as images with src attributes
         * or input elements with type and name attributes.
         *
         * Examples:
         * - self_closing_element("img", {{"src", "image.jpg"}, {"alt", "Description"}})
         *   creates an image element
         * - self_closing_element("input", {{"type", "text"}, {"name", "username"}})
         *   creates a text input element
         */
        self_closing_element(const std::string &tag, const std::map<std::string, std::string> &attributes);

        /**
         * @brief Override to prevent adding child elements to self-closing elements.
         * @param child Shared pointer to the child element (will be rejected)
         *
         * Overrides the base class add_child method to enforce the constraint
         * that self-closing elements cannot have child elements. The implementation
         * should either silently ignore the child addition attempt or throw an
         * appropriate exception to indicate the invalid operation.
         *
         * This method maintains the semantic correctness of self-closing elements
         * by preventing invalid HTML structure that would result from adding
         * children to elements that cannot contain them.
         *
         * @note The exact behavior (ignore vs. exception) depends on the
         *       implementation strategy chosen for error handling
         */
        virtual void add_child(std::shared_ptr<element> child) override;

        /**
         * @brief Convert the self-closing element to its string representation.
         * @return String containing the properly formatted self-closing element
         *
         * Overrides the base element's to_string() method to produce the correct
         * self-closing syntax. The output format follows HTML5 standards for
         * void elements, typically rendering as `<tag attributes>` without a
         * closing tag, or `<tag attributes />` in XHTML-style formatting.
         *
         * The method ensures that self-closing elements are rendered correctly
         * according to HTML specifications, without closing tags that would
         * be invalid for these element types.
         *
         * Examples:
         * - `<br>` for line breaks
         * - `<img src="image.jpg" alt="Description">` for images
         * - `<input type="text" name="username">` for form inputs
         */
        virtual std::string to_string() const override;

        /**
         * @brief Override to return empty children collection.
         * @return Empty vector since self-closing elements cannot have children
         *
         * Overrides the base class method to always return an empty vector,
         * reflecting the fact that self-closing elements cannot and do not
         * contain child elements. This maintains consistency with the
         * self-closing element constraints.
         *
         * @note This method will always return an empty vector regardless
         *       of any previous attempts to add children
         */
        virtual std::vector<std::shared_ptr<element>> get_children() const override;

        /**
         * @brief Override to return empty text content.
         * @return Empty string since self-closing elements cannot have text content
         *
         * Overrides the base class method to always return an empty string,
         * reflecting the fact that self-closing elements cannot contain text
         * content. This maintains consistency with the self-closing element
         * constraints and HTML specifications.
         *
         * @note This method will always return an empty string regardless
         *       of any previous attempts to set text content
         */
        virtual std::string get_text_content() const override;

        /**
         * @brief Override to prevent setting text content on self-closing elements.
         * @param text_content Text content string (will be rejected)
         *
         * Overrides the base class set_text_content method to enforce the
         * constraint that self-closing elements cannot have text content.
         * The implementation should either silently ignore the content setting
         * attempt or throw an appropriate exception to indicate the invalid operation.
         *
         * This method maintains the semantic correctness of self-closing elements
         * by preventing invalid HTML structure that would result from adding
         * text content to elements that cannot contain it.
         *
         * @note The exact behavior (ignore vs. exception) depends on the
         *       implementation strategy chosen for error handling
         */
        virtual void set_text_content(const std::string &text_content) override;
    };
}