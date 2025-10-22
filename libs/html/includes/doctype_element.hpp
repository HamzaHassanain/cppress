
#pragma once

#include "element.hpp"

namespace hh_html_builder
{
    /**
     * @brief Specialized element class for HTML DOCTYPE declarations.
     *
     * This class extends the base element class to provide specific functionality
     * for HTML DOCTYPE declarations. DOCTYPE elements are special HTML constructs
     * that inform the browser about the HTML version and document type being used.
     * Unlike regular HTML elements, DOCTYPE declarations have a unique syntax and
     * do not follow the standard opening/closing tag pattern.
     *
     * The class automatically handles the proper formatting of DOCTYPE declarations,
     * ensuring they are rendered with the correct `<!DOCTYPE ...>` syntax rather
     * than standard element tags. This specialization is essential for generating
     * valid HTML documents that begin with proper document type declarations.
     *
     * Common DOCTYPE values:
     * - "html" for HTML5 documents
     * - "html PUBLIC..." for older HTML versions
     * - Custom DOCTYPE declarations for specialized document types
     *
     * @note DOCTYPE elements should typically be the first element in an HTML document
     * @note This class overrides the to_string() method to produce DOCTYPE-specific output
     * @note DOCTYPE elements don't support child elements or standard HTML attributes
     */
    class doctype_element : public element
    {
    public:
        /**
         * @brief Construct a DOCTYPE element with specified document type.
         * @param doctype The document type declaration string
         *
         * Creates a DOCTYPE element with the specified document type string.
         * The most common usage is with "html" for HTML5 documents, which
         * produces the standard `<!DOCTYPE html>` declaration.
         *
         * The constructor internally uses the base element constructor with
         * "!DOCTYPE" as the tag name and the provided doctype as text content,
         * but the actual rendering is handled by the overridden to_string() method.
         *
         * Examples:
         * - doctype_element("html") creates `<!DOCTYPE html>`
         * - doctype_element("html PUBLIC \"-//W3C//DTD HTML 4.01//EN\"")
         *   creates a legacy HTML 4.01 DOCTYPE
         */
        doctype_element(const std::string &doctype) : element("!DOCTYPE", doctype) {}

        /**
         * @brief Convert the DOCTYPE element to its string representation.
         * @return String containing the properly formatted DOCTYPE declaration
         *
         * Overrides the base element's to_string() method to produce the correct
         * DOCTYPE syntax. Instead of generating standard HTML tags, this method
         * formats the output as `<!DOCTYPE content>` where content is the
         * document type string provided during construction.
         *
         * This specialized rendering ensures that DOCTYPE elements are properly
         * formatted according to HTML standards, which require the specific
         * `<!DOCTYPE ...>` syntax rather than regular element tags.
         *
         * @note This method ignores any attributes or child elements since
         *       DOCTYPE declarations don't support these features
         */
        std::string to_string() const override
        {
            return "<!DOCTYPE " + get_text_content() + ">";
        }
    };
}