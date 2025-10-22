#include <stdexcept>
#include "../includes/self_closing_element.hpp"
namespace hh_html_builder
{
    self_closing_element::self_closing_element(const std::string &tag)
        : element(tag) {}

    self_closing_element::self_closing_element(const std::string &tag, const std::map<std::string, std::string> &attributes)
        : element(tag, attributes) {}

    std::string self_closing_element::to_string() const
    {
        std::string result = "<" + get_tag();
        for (const auto &attr : get_attributes())
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
        result += " />";
        return result;
    }

    std::vector<std::shared_ptr<element>> self_closing_element::get_children() const
    {
        // throw std::runtime_error("Self-closing elements cannot have children");
        return {}; // Return an empty vector since self-closing elements have no children
    }

    std::string self_closing_element::get_text_content() const
    {
        return "";
    }

    void self_closing_element::set_text_content(const std::string &text_content)
    {
        (void)text_content;
        // throw std::runtime_error("Self-closing elements cannot have text content");
    }
    void self_closing_element::add_child(std::shared_ptr<element> child)
    {
        (void)child;
        // throw std::runtime_error("Self-closing elements cannot have children");
    }
}