#include "../includes/document.hpp"

#include <stdexcept>

namespace cppress::html {

document::document(const std::string& doctype) : doctype(doctype) {
    root = std::make_shared<element>("html");
}

std::string document::to_string() const {
    std::string result = "<!DOCTYPE " + doctype + ">";
    result += root->to_string();
    return result;
}

void document::add_child(std::shared_ptr<element> elem) {
    if (elem) {
        root->add_child(elem);
    }
}

void document::push_back(std::shared_ptr<element> elem) {
    add_child(elem);
}

std::shared_ptr<element> document::get_root() const {
    return root;
}

void document::set_doctype(const std::string& doctype) {
    this->doctype = doctype;
}

std::string document::get_doctype() const {
    return doctype;
}

document::size_type document::size() const noexcept {
    return root->size();
}

bool document::empty() const noexcept {
    return root->empty();
}

void document::clear() noexcept {
    root->clear();
}

document::iterator document::begin() noexcept {
    return root->begin();
}

document::const_iterator document::begin() const noexcept {
    return root->begin();
}

document::const_iterator document::cbegin() const noexcept {
    return root->cbegin();
}

document::iterator document::end() noexcept {
    return root->end();
}

document::const_iterator document::end() const noexcept {
    return root->end();
}

document::const_iterator document::cend() const noexcept {
    return root->cend();
}

document::value_type& document::at(size_type index) {
    return root->at(index);
}

const document::value_type& document::at(size_type index) const {
    return root->at(index);
}

document::value_type& document::operator[](size_type index) {
    return (*root)[index];
}

const document::value_type& document::operator[](size_type index) const {
    return (*root)[index];
}

}  // namespace cppress::html
