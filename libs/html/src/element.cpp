#include "../includes/element.hpp"

#include <iostream>
#include <stdexcept>

#include "../includes/document_parser.hpp"

namespace cppress::html {
element::element() : tag("") {}

element::element(const std::string& tag) : tag(tag) {}

element::element(const std::string& tag, const std::string& text_content)
    : tag(tag), text_content(text_content) {}

element::element(const std::string& tag, const std::map<std::string, std::string>& attributes)
    : tag(tag), attributes(attributes) {}

element::element(const std::string& tag, const std::string& text_content,
                 const std::map<std::string, std::string>& attributes)
    : tag(tag), text_content(text_content), attributes(attributes) {}

void element::add_child(std::shared_ptr<element> child) {
    children.push_back(child);
}

void element::set_text_content(const std::string& text_content) {
    this->text_content = text_content;
}

std::string element::get_tag() const {
    return tag;
}

std::string element::get_text_content() const {
    return text_content;
}

std::map<std::string, std::string> element::get_attributes() const {
    return attributes;
}

std::string element::get_attribute(const std::string& key) const {
    auto it = attributes.find(key);
    if (it != attributes.end()) {
        return it->second;
    }
    return "";
}

std::vector<std::shared_ptr<element>> element::get_children() const {
    return children;
}

std::string element::to_string() const {
    std::string result = tag.empty() ? "" : ("<" + tag);

    if (!tag.empty())
        for (const auto& attr : attributes) {
            if (attr.second.empty()) {
                result += " " + attr.first;
            } else {
                result += " " + attr.first + "=\"" + attr.second + "\"";
            }
        }
    result += (tag.empty() ? "" : ">") + text_content;
    for (const auto& child : children) {
        result += child->to_string();
    }
    result += tag.empty() ? "" : ("</" + tag + ">");
    return result;
}

void element::set_params_recursive(const std::map<std::string, std::string>& params) {
    set_params(params);
    for (const auto& child : children) {
        child->set_params_recursive(params);
    }
}
void element::set_params(const std::map<std::string, std::string>& params) {
    this->text_content = substitute_params(text_content, params);
    // check atrs
    for (auto& attr : attributes) {
        attr.second = substitute_params(attr.second, params);
    }
}

element element::copy() const {
    element copy = *this;
    copy.children.clear();
    for (const auto& child : children) {
        copy.add_child(std::make_shared<element>(child->copy()));
    }
    return copy;
}

// STL-like methods for children management

element::size_type element::size() const noexcept {
    return children.size();
}

bool element::empty() const noexcept {
    return children.empty();
}

element::reference element::at(size_type index) {
    if (index >= children.size()) {
        throw std::out_of_range("Child index out of bounds");
    }
    return children[index];
}

element::const_reference element::at(size_type index) const {
    if (index >= children.size()) {
        throw std::out_of_range("Child index out of bounds");
    }
    return children[index];
}

void element::push_back(std::shared_ptr<element> child) {
    children.push_back(child);
}

void element::pop_back() {
    if (children.empty()) {
        throw std::out_of_range("Cannot pop from element with no children");
    }
    children.pop_back();
}

element::reference element::front() {
    if (children.empty()) {
        throw std::out_of_range("Element has no children");
    }
    return children.front();
}

element::const_reference element::front() const {
    if (children.empty()) {
        throw std::out_of_range("Element has no children");
    }
    return children.front();
}

element::reference element::back() {
    if (children.empty()) {
        throw std::out_of_range("Element has no children");
    }
    return children.back();
}

element::const_reference element::back() const {
    if (children.empty()) {
        throw std::out_of_range("Element has no children");
    }
    return children.back();
}

void element::clear() noexcept {
    children.clear();
}

void element::reserve(size_type capacity) {
    children.reserve(capacity);
}

element::iterator element::begin() noexcept {
    return children.begin();
}

element::const_iterator element::begin() const noexcept {
    return children.begin();
}

element::const_iterator element::cbegin() const noexcept {
    return children.cbegin();
}

element::iterator element::end() noexcept {
    return children.end();
}

element::const_iterator element::end() const noexcept {
    return children.end();
}

element::const_iterator element::cend() const noexcept {
    return children.cend();
}

element::reverse_iterator element::rbegin() noexcept {
    return children.rbegin();
}

element::const_reverse_iterator element::rbegin() const noexcept {
    return children.rbegin();
}

element::const_reverse_iterator element::crbegin() const noexcept {
    return children.crbegin();
}

element::reverse_iterator element::rend() noexcept {
    return children.rend();
}

element::const_reverse_iterator element::rend() const noexcept {
    return children.rend();
}

element::const_reverse_iterator element::crend() const noexcept {
    return children.crend();
}

// STL-like methods for attributes management

void element::set_attribute(const std::string& key, const std::string& value) {
    attributes[key] = value;
}

element::size_type element::erase_attribute(const std::string& key) {
    return attributes.erase(key);
}

bool element::has_attribute(const std::string& key) const {
    return attributes.find(key) != attributes.end();
}

element::size_type element::attributes_size() const noexcept {
    return attributes.size();
}

bool element::attributes_empty() const noexcept {
    return attributes.empty();
}

element::attributes_iterator element::attributes_begin() noexcept {
    return attributes.begin();
}

element::attributes_const_iterator element::attributes_begin() const noexcept {
    return attributes.begin();
}

element::attributes_const_iterator element::attributes_cbegin() const noexcept {
    return attributes.cbegin();
}

element::attributes_iterator element::attributes_end() noexcept {
    return attributes.end();
}

element::attributes_const_iterator element::attributes_end() const noexcept {
    return attributes.end();
}

element::attributes_const_iterator element::attributes_cend() const noexcept {
    return attributes.cend();
}

// Operator overloads

element::reference element::operator[](size_type index) {
    return children[index];
}

element::const_reference element::operator[](size_type index) const {
    return children[index];
}

};  // namespace cppress