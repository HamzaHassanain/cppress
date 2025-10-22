#include <bits/stdc++.h>
#include "html-builder.hpp"
using namespace std;
using namespace hh_html_builder;
int main()
{

    string path = "tmp.html";
    string content;
    ifstream file(path);
    try
    {

        if (file)
        {
            stringstream buffer;
            buffer << file.rdbuf();
            content = buffer.str();
        }

        /*
         title,subtitle,heroTitle,heroDescription,email,github,linkedin
        */

        std::map<std::string, std::string> params = {
            {"title", "My Website"},
            {"subtitle", "Welcome to my website"},
            {"heroTitle", "Welcome to My Website"},
            {"aboutText", "Welcome to My Website"},
            {"aboutExtraText", "This is some extra information about me."},
            {"heroDescription", "This is a sample description for the hero section."},
            {"contactEmail", "contact@example.com"},
            {"github", "https://github.com/username"},
            {"linkedin", "https://linkedin.com/in/username"},
            {"email", "contact@example.com"}

        };
        auto elements = parse_html_string(content);
        // write answer to x.html

        ofstream output("x.html");
        if (output)
        {
            for (auto &el : elements)
            {
                el->set_params_recursive(params);
                output << el->to_string() << endl;
            }
        }

        // element root("body");
        // element main("main");
        // element header("header");
        // element footer("footer");

        // main.add_child(make_shared<element>("h1", "{{heroTitle}}"));
        // main.add_child(make_shared<element>("p", "{{heroDescription}}"));
        // main.add_child(make_shared<element>("img", "{{heroImage}}"));

        // footer.add_child(make_shared<element>("p", "{{footerText}}"));
        // footer.add_child(make_shared<element>("a", "{{footerLink}}"));

        // root.add_child(make_shared<element>(header));
        // root.add_child(make_shared<element>(main));
        // root.add_child(make_shared<element>(footer));

        // std::map<std::string, std::string> params = {
        //     {"heroTitle", "Welcome to My Website"},
        //     {"heroDescription", "This is a sample description for the hero section."},
        //     {"heroImage", "https://example.com/image.jpg"},
        //     {"footerText", "© 2023 My Website"},
        //     {"footerLink", "https://example.com"}};

        // // parsm2

        // std::map<std::string, std::string> params2 = {
        //     {"heroTitle", "Welcome"},
        //     {"heroDescription", "This is an."},
        //     {"heroImage", "https://.com/image.jpg"},
        //     {"footerText", "©  My Website"},
        //     {"footerLink", "https://.com"}};

        // element root2 = root.copy();

        // root.set_text_params_recursive(params);
        // root2.set_text_params_recursive(params2);

        // std::cout << root.to_string() << endl;
        // std::cout << root2.to_string() << endl;
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}