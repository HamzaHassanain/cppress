/**
 * @file includes.hpp
 * @brief Main header file for the cppress HTML library.
 *
 * This library provides a complete HTML document builder framework with
 * an STL-like interface. It supports programmatic HTML generation with
 * familiar C++ Standard Library patterns, consistent with the cppress JSON library.
 *
 * @section features Features
 * - Programmatic HTML document construction
 * - STL-like container interfaces (similar to std::vector, std::map)
 * - Type-safe element creation and manipulation
 * - Iterator support for element hierarchies
 * - Template-based HTML generation with parameter substitution
 * - Comprehensive type checking utilities
 * - Doxygen-documented API
 *
 * @section types HTML Types
 * - element: Standard HTML elements with children and attributes
 * - self_closing_element: Self-closing/void elements (img, br, hr, etc.)
 * - doctype_element: DOCTYPE declarations
 * - document: Complete HTML documents with DOCTYPE
 *
 * @section namespaces Namespaces
 * - cppress: Main namespace containing all HTML types
 * - cppress::maker: Factory functions for creating HTML elements
 * - cppress::getter: Type-safe accessor functions for extracting element data
 *
 * @section example Basic Usage Example
 * @code
 * #include "includes.hpp"
 * using namespace cppress::html;
 *
 * // Create a document
 * document doc("html");
 *
 * // Create elements with factory functions
 * auto body = maker::make_element("body");
 * auto heading = maker::make_heading(1, "Welcome");
 * auto para = maker::make_paragraph("This is a paragraph.");
 *
 * // Build hierarchy
 * body->add_child(heading);
 * body->push_back(para);  // STL-like method
 * doc.add_child(body);
 *
 * // STL-like iteration
 * for (const auto& child : *body) {
 *     std::cout << child->get_tag() << std::endl;
 * }
 *
 * // Generate HTML
 * std::string html = doc.to_string();
 * // Output: <!DOCTYPE html>
 * //         <html><body><h1>Welcome</h1><p>This is a paragraph.</p></body></html>
 *
 * // Common element factories
 * auto div = maker::make_div();
 * auto link = maker::make_link("https://example.com", "Click here");
 * auto img = maker::make_image("image.jpg", "Description");
 * auto input = maker::make_input("text", "username");
 *
 * // STL-like operations
 * div->reserve(10);  // Reserve capacity for children
 * div->push_back(link);
 * div->push_back(img);
 * auto first_child = div->front();
 * auto child_count = div->size();
 * bool has_children = !div->empty();
 *
 * // Attribute management
 * div->set_attribute("class", "container");
 * div->set_attribute("id", "main");
 * if (div->has_attribute("class")) {
 *     auto cls = div->get_attribute("class");
 * }
 * @endcode
 *
 * @author Hamza Moahmmed Hassanain
 * @version 1.0
 */

#pragma once

#include "includes/doctype_element.hpp"
#include "includes/document.hpp"
#include "includes/document_parser.hpp"
#include "includes/element.hpp"
#include "includes/helpers.hpp"
#include "includes/self_closing_element.hpp"
