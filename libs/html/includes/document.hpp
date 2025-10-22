#pragma once
#include <string>
#include "element.hpp"
namespace hh_html_builder
{

    class document
    {
        std::shared_ptr<element> root;
        std::string doctype;

    public:
        document(const std::string &doctype = "html")
        {
            this->doctype = doctype;
            root = std::make_shared<element>("html");
        }
        std::string to_string() const
        {
            std::string result = "<!DOCTYPE " + doctype + ">\n";
            result += root->to_string();
            return result;
        }
        void add_child(std::shared_ptr<element> elem)
        {
            if (elem)
            {
                root->add_child(elem);
            }
        }
        ~document() = default;
    };

}