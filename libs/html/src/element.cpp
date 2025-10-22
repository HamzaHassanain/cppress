#include <stdexcept>
#include <iostream>

#include "../includes/document_parser.hpp"
#include "../includes/element.hpp"

namespace hh_html_builder
{
    element::element() : tag("") {}

    element::element(const std::string &tag) : tag(tag) {}

    element::element(const std::string &tag, const std::string &text_content)
        : tag(tag), text_content(text_content) {}

    element::element(const std::string &tag, const std::map<std::string, std::string> &attributes)
        : tag(tag), attributes(attributes) {}

    element::element(const std::string &tag, const std::string &text_content, const std::map<std::string, std::string> &attributes)
        : tag(tag), text_content(text_content), attributes(attributes) {}

    void element::add_child(std::shared_ptr<element> child)
    {
        children.push_back(child);
    }

    void element::set_text_content(const std::string &text_content)
    {
        this->text_content = text_content;
    }

    std::string element::get_tag() const
    {
        return tag;
    }

    std::string element::get_text_content() const
    {
        return text_content;
    }

    std::map<std::string, std::string> element::get_attributes() const
    {
        return attributes;
    }

    std::string element::get_attribute(const std::string &key) const
    {
        auto it = attributes.find(key);
        if (it != attributes.end())
        {
            return it->second;
        }
        return "";
    }

    std::vector<std::shared_ptr<element>> element::get_children() const
    {
        return children;
    }

    std::string element::to_string() const
    {
        std::string result = tag.empty() ? "" : ("<" + tag);

        if (!tag.empty())
            for (const auto &attr : attributes)
            {
                if (attr.second.empty())
                {
                    result += " " + attr.first;
                }
                else
                {
                    result += " " + attr.first + "=\"" + attr.second + "\"";
                }
            }
        result += (tag.empty() ? "" : ">") + text_content;
        for (const auto &child : children)
        {
            result += child->to_string();
        }
        result += tag.empty() ? "" : ("</" + tag + ">\n");
        return result;
    }

    void element::set_params_recursive(const std::map<std::string, std::string> &params)
    {
        set_params(params);
        for (const auto &child : children)
        {
            child->set_params_recursive(params);
        }
    }
    void element::set_params(const std::map<std::string, std::string> &params)
    {
        this->text_content = parse_html_with_params(text_content, params);
        // check atrs
        for (auto &attr : attributes)
        {
            attr.second = parse_html_with_params(attr.second, params);
        }
    }

    element element::copy() const
    {
        element copy = *this;
        copy.children.clear();
        for (const auto &child : children)
        {
            copy.add_child(std::make_shared<element>(child->copy()));
        }
        return copy;
    }

};