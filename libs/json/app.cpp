#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "includes.hpp"
using namespace std;

using namespace cppress::json;
using namespace cppress::json::getter;
using namespace cppress::json::maker;

int main() {
    // Example 1: Build a JSON object manually and stringify it
    cout << "=== Example 1: Manual construction ===" << endl;
    auto obj = make_shared<json_object>();
    obj->insert("name", maker::make_string("Alice"));
    obj->insert("age", maker::make_number(30));
    obj->insert("active", maker::make_boolean(true));

    cout << "Object: " << obj->stringify() << endl << endl;

    // Example 2: Build an array containing primitives and nested objects
    cout << "=== Example 2: Arrays and nesting ===" << endl;
    auto arr = make_shared<json_array>();
    arr->insert(maker::make_string("first"));
    arr->insert(maker::make_number(3.1415));

    auto nested = make_shared<json_object>();
    nested->insert("k", maker::make_string("v"));
    arr->insert(nested);

    obj->insert("items", arr);
    cout << "Nested object: " << obj->stringify() << endl << endl;

    // Example 3: Parse a JSON string and read values using helpers
    cout << "=== Example 3: Parsing JSON text ===" << endl;
    string json_text;
    string file = "jsfile.json";

    ifstream ifs(file);
    if (ifs) {
        json_text.assign((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());
    }

    try {
        auto parsed = parse(json_text);
        for (const auto& [key, item] : parsed) {
            // Access top-level properties from the returned map

            cout << item->stringify() << endl;
        }

        if (auto admin_ptr = parsed["admin"]) {
            cout << "admin: " << (getter::get_boolean(admin_ptr) ? "true" : "false") << endl;
        }

        if (auto scores_ptr = parsed["scores"]) {
            auto scores = getter::get_array(scores_ptr);
            cout << "scores: [";
            for (size_t i = 0; i < scores.size(); ++i) {
                // Each element is a json_object pointer that should be a number
                cout << getter::get_number(scores[i]);
                if (i + 1 < scores.size())
                    cout << ", ";
            }
            cout << "]" << endl;
        }

        if (auto meta_ptr = parsed["meta"]) {
            auto meta_obj = meta_ptr;
            if (meta_obj) {
                if (auto region_ptr = meta_obj->get("region")) {
                    cout << "region: " << getter::get_string(region_ptr) << endl;
                }
            }
        }

        // Show full parsed structure by reassembling into a json_object and stringifying
        auto root = make_shared<json_object>();
        for (const auto& p : parsed) {
            root->insert(p.first, p.second);
        }
        cout << "full parsed: " << root->stringify() << endl;
    } catch (const std::exception& e) {
        cerr << "Parse error: " << e.what() << endl;
    }

    return 0;
}