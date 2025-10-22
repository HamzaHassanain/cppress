#pragma once

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#include "element.hpp"
#include "self_closing_element.hpp"

namespace cppress::html {

// Forward declarations for type checking
/**
 * @brief Checks if an element is a self-closing element.
 * @param elem The element to check.
 * @return true if the element is a self_closing_element, false otherwise.
 */
inline bool is_self_closing(const std::shared_ptr<element>& elem) {
    return std::dynamic_pointer_cast<self_closing_element>(elem) != nullptr;
}

/**
 * @brief Checks if an element is a standard (non-self-closing) element.
 * @param elem The element to check.
 * @return true if the element is a standard element, false otherwise.
 */
inline bool is_standard_element(const std::shared_ptr<element>& elem) {
    return elem && !is_self_closing(elem);
}

}  // namespace cppress::html

namespace cppress::html::maker {

/**
 * @brief Creates a standard HTML element.
 * @param tag The HTML tag name.
 * @return A shared pointer to the created element.
 */
inline std::shared_ptr<cppress::html::element> make_element(const std::string& tag) {
    return std::make_shared<cppress::html::element>(tag);
}

/**
 * @brief Creates an HTML element with text content.
 * @param tag The HTML tag name.
 * @param text_content The text content.
 * @return A shared pointer to the created element.
 */
inline std::shared_ptr<cppress::html::element> make_element(const std::string& tag,
                                                            const std::string& text_content) {
    return std::make_shared<cppress::html::element>(tag, text_content);
}

/**
 * @brief Creates an HTML element with attributes.
 * @param tag The HTML tag name.
 * @param attributes Map of attribute name-value pairs.
 * @return A shared pointer to the created element.
 */
inline std::shared_ptr<cppress::html::element> make_element(
    const std::string& tag, const std::map<std::string, std::string>& attributes) {
    return std::make_shared<cppress::html::element>(tag, attributes);
}

/**
 * @brief Creates an HTML element with text content and attributes.
 * @param tag The HTML tag name.
 * @param text_content The text content.
 * @param attributes Map of attribute name-value pairs.
 * @return A shared pointer to the created element.
 */
inline std::shared_ptr<cppress::html::element> make_element(
    const std::string& tag, const std::string& text_content,
    const std::map<std::string, std::string>& attributes) {
    return std::make_shared<cppress::html::element>(tag, text_content, attributes);
}

/**
 * @brief Creates a self-closing HTML element.
 * @param tag The HTML tag name.
 * @return A shared pointer to the created self-closing element.
 */
inline std::shared_ptr<cppress::html::element> make_self_closing(const std::string& tag) {
    return std::make_shared<cppress::html::self_closing_element>(tag);
}

/**
 * @brief Creates a self-closing HTML element with attributes.
 * @param tag The HTML tag name.
 * @param attributes Map of attribute name-value pairs.
 * @return A shared pointer to the created self-closing element.
 */
inline std::shared_ptr<cppress::html::element> make_self_closing(
    const std::string& tag, const std::map<std::string, std::string>& attributes) {
    return std::make_shared<cppress::html::self_closing_element>(tag, attributes);
}

// Common HTML element factories

/**
 * @brief Creates a div element.
 * @return A shared pointer to a div element.
 */
inline std::shared_ptr<cppress::html::element> make_div() {
    return make_element("div");
}

/**
 * @brief Creates a paragraph element.
 * @param text Optional text content.
 * @return A shared pointer to a p element.
 */
inline std::shared_ptr<cppress::html::element> make_paragraph(const std::string& text = "") {
    return make_element("p", text);
}

/**
 * @brief Creates a heading element.
 * @param level Heading level (1-6).
 * @param text Heading text.
 * @return A shared pointer to an h1-h6 element.
 */
inline std::shared_ptr<cppress::html::element> make_heading(int level, const std::string& text) {
    return make_element("h" + std::to_string(level), text);
}

/**
 * @brief Creates a span element.
 * @param text Optional text content.
 * @return A shared pointer to a span element.
 */
inline std::shared_ptr<cppress::html::element> make_span(const std::string& text = "") {
    return make_element("span", text);
}

/**
 * @brief Creates an anchor (link) element.
 * @param href The URL to link to.
 * @param text The link text.
 * @return A shared pointer to an a element.
 */
inline std::shared_ptr<cppress::html::element> make_link(const std::string& href,
                                                         const std::string& text) {
    return make_element("a", text, {{"href", href}});
}

/**
 * @brief Creates an image element.
 * @param src The image source URL.
 * @param alt The alt text.
 * @return A shared pointer to an img element.
 */
inline std::shared_ptr<cppress::html::element> make_image(const std::string& src,
                                                          const std::string& alt = "") {
    return make_self_closing("img", {{"src", src}, {"alt", alt}});
}

/**
 * @brief Creates a line break element.
 * @return A shared pointer to a br element.
 */
inline std::shared_ptr<cppress::html::element> make_br() {
    return make_self_closing("br");
}

/**
 * @brief Creates a horizontal rule element.
 * @return A shared pointer to an hr element.
 */
inline std::shared_ptr<cppress::html::element> make_hr() {
    return make_self_closing("hr");
}

/**
 * @brief Creates an input element.
 * @param type The input type (text, password, email, etc.).
 * @param name The input name.
 * @return A shared pointer to an input element.
 */
inline std::shared_ptr<cppress::html::element> make_input(const std::string& type,
                                                          const std::string& name = "") {
    std::map<std::string, std::string> attrs = {{"type", type}};
    if (!name.empty()) {
        attrs["name"] = name;
    }
    return make_self_closing("input", attrs);
}

/**
 * @brief Creates a button element.
 * @param text The button text.
 * @param type The button type (button, submit, reset).
 * @return A shared pointer to a button element.
 */
inline std::shared_ptr<cppress::html::element> make_button(const std::string& text,
                                                           const std::string& type = "button") {
    return make_element("button", text, {{"type", type}});
}

}  // namespace cppress::html::maker

namespace cppress::html::getter {

/**
 * @brief Gets the tag name of an element.
 * @param elem The element.
 * @return The tag name.
 * @throws std::runtime_error if the element is null.
 */
inline std::string get_tag(const std::shared_ptr<cppress::html::element>& elem) {
    if (!elem) {
        throw std::runtime_error("Cannot get tag from null element");
    }
    return elem->get_tag();
}

/**
 * @brief Safely gets the tag name of an element.
 * @param elem The element.
 * @return An optional containing the tag name if successful, or empty if null.
 */
inline std::optional<std::string> try_get_tag(const std::shared_ptr<cppress::html::element>& elem) {
    if (!elem) {
        return std::nullopt;
    }
    return elem->get_tag();
}

/**
 * @brief Gets the text content of an element.
 * @param elem The element.
 * @return The text content.
 * @throws std::runtime_error if the element is null.
 */
inline std::string get_text(const std::shared_ptr<cppress::html::element>& elem) {
    if (!elem) {
        throw std::runtime_error("Cannot get text from null element");
    }
    return elem->get_text_content();
}

/**
 * @brief Safely gets the text content of an element.
 * @param elem The element.
 * @return An optional containing the text if successful, or empty if null.
 */
inline std::optional<std::string> try_get_text(
    const std::shared_ptr<cppress::html::element>& elem) {
    if (!elem) {
        return std::nullopt;
    }
    return elem->get_text_content();
}

/**
 * @brief Gets an attribute value from an element.
 * @param elem The element.
 * @param key The attribute name.
 * @return The attribute value, or empty string if not found.
 * @throws std::runtime_error if the element is null.
 */
inline std::string get_attribute(const std::shared_ptr<cppress::html::element>& elem,
                                 const std::string& key) {
    if (!elem) {
        throw std::runtime_error("Cannot get attribute from null element");
    }
    return elem->get_attribute(key);
}

/**
 * @brief Safely gets an attribute value from an element.
 * @param elem The element.
 * @param key The attribute name.
 * @return An optional containing the attribute value if successful.
 */
inline std::optional<std::string> try_get_attribute(
    const std::shared_ptr<cppress::html::element>& elem, const std::string& key) {
    if (!elem) {
        return std::nullopt;
    }
    auto value = elem->get_attribute(key);
    if (value.empty()) {
        return std::nullopt;
    }
    return value;
}

/**
 * @brief Gets all children of an element.
 * @param elem The element.
 * @return Vector of child elements.
 * @throws std::runtime_error if the element is null.
 */
inline std::vector<std::shared_ptr<cppress::html::element>> get_children(
    const std::shared_ptr<cppress::html::element>& elem) {
    if (!elem) {
        throw std::runtime_error("Cannot get children from null element");
    }
    return elem->get_children();
}

/**
 * @brief Safely gets all children of an element.
 * @param elem The element.
 * @return An optional containing the children if successful, or empty if null.
 */
inline std::optional<std::vector<std::shared_ptr<cppress::html::element>>> try_get_children(
    const std::shared_ptr<cppress::html::element>& elem) {
    if (!elem) {
        return std::nullopt;
    }
    return elem->get_children();
}

/**
 * @brief Casts an element to self_closing_element.
 * @param elem The element.
 * @return A shared pointer to self_closing_element.
 * @throws std::runtime_error if the element is not a self-closing element.
 */
inline std::shared_ptr<cppress::html::self_closing_element> as_self_closing(
    const std::shared_ptr<cppress::html::element>& elem) {
    if (auto self_closing = std::dynamic_pointer_cast<cppress::html::self_closing_element>(elem)) {
        return self_closing;
    }
    throw std::runtime_error("Element is not a self-closing element");
}

/**
 * @brief Safely casts an element to self_closing_element.
 * @param elem The element.
 * @return A shared pointer to self_closing_element, or nullptr if not self-closing.
 */
inline std::shared_ptr<cppress::html::self_closing_element> try_as_self_closing(
    const std::shared_ptr<cppress::html::element>& elem) {
    return std::dynamic_pointer_cast<cppress::html::self_closing_element>(elem);
}

}  // namespace cppress::html::getter
