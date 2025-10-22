#include <gtest/gtest.h>

#include "../includes.hpp"

using namespace cppress::json;
using namespace cppress::json::getter;
using namespace cppress::json::maker;

TEST(JsonObject, BasicCreationAndStringify) {
    auto obj = std::make_shared<json_object>();
    EXPECT_EQ(obj->stringify(), "{}");
    EXPECT_TRUE(obj->empty());
    EXPECT_EQ(obj->size(), 0);
}

TEST(JsonObject, InsertAndAccess) {
    auto obj = std::make_shared<json_object>();
    obj->insert("name", make_string("John"));
    obj->insert("age", make_number(30));

    EXPECT_EQ(obj->size(), 2);
    EXPECT_TRUE(obj->contains("name"));
    EXPECT_FALSE(obj->contains("missing"));

    auto name = obj->get("name");
    EXPECT_EQ(get_string(name), "John");
    EXPECT_EQ(get_number(obj->get("age")), 30);
}

TEST(JsonObject, STLLikeOperations) {
    auto obj = std::make_shared<json_object>();
    obj->insert("a", make_number(1));
    obj->insert("b", make_number(2));
    obj->insert("c", make_number(3));

    EXPECT_EQ(obj->count("a"), 1);
    EXPECT_EQ(obj->count("missing"), 0);

    auto it = obj->find("b");
    EXPECT_NE(it, obj->end());
    EXPECT_EQ(get_number(it->second), 2);

    obj->erase("b");
    EXPECT_EQ(obj->size(), 2);
    EXPECT_FALSE(obj->contains("b"));
}

TEST(JsonObject, Iterator) {
    auto obj = std::make_shared<json_object>();
    obj->insert("x", make_number(10));
    obj->insert("y", make_number(20));

    int sum = 0;
    for (const auto& [key, value] : *obj) {
        sum += static_cast<int>(get_number(value));
    }
    EXPECT_EQ(sum, 30);
}

TEST(JsonArray, BasicOperations) {
    auto arr = std::make_shared<json_array>();
    EXPECT_TRUE(arr->empty());
    EXPECT_EQ(arr->size(), 0);

    arr->push_back(make_number(1));
    arr->push_back(make_string("test"));
    arr->push_back(make_boolean(true));

    EXPECT_EQ(arr->size(), 3);
    EXPECT_EQ(get_number(arr->at(0)), 1);
    EXPECT_EQ(get_string(arr->at(1)), "test");
    EXPECT_TRUE(get_boolean(arr->at(2)));
}

TEST(JsonArray, FrontBackAccess) {
    auto arr = std::make_shared<json_array>();
    arr->push_back(make_number(100));
    arr->push_back(make_number(200));
    arr->push_back(make_number(300));

    EXPECT_EQ(get_number(arr->front()), 100);
    EXPECT_EQ(get_number(arr->back()), 300);

    arr->pop_back();
    EXPECT_EQ(arr->size(), 2);
    EXPECT_EQ(get_number(arr->back()), 200);
}

TEST(JsonArray, Iterator) {
    auto arr = std::make_shared<json_array>();
    arr->push_back(make_number(1));
    arr->push_back(make_number(2));
    arr->push_back(make_number(3));

    int product = 1;
    for (const auto& elem : *arr) {
        product *= static_cast<int>(get_number(elem));
    }
    EXPECT_EQ(product, 6);
}

TEST(JsonArray, Stringify) {
    auto arr = std::make_shared<json_array>();
    arr->push_back(make_number(1));
    arr->push_back(make_string("hello"));
    arr->push_back(make_boolean(false));

    std::string json = arr->stringify();
    EXPECT_EQ(json, R"([1,"hello",false])");
}

TEST(JsonString, BasicOperations) {
    auto str = make_string("Hello World");
    auto json_str = std::dynamic_pointer_cast<json_string>(str);

    EXPECT_EQ(json_str->value, "Hello World");
    EXPECT_EQ(json_str->size(), 11);
    EXPECT_FALSE(json_str->empty());
    EXPECT_EQ(json_str->stringify(), R"("Hello World")");
}

TEST(JsonString, STLLikeMethods) {
    auto str = std::make_shared<json_string>("test");

    str->append(" string");
    EXPECT_EQ(str->value, "test string");

    EXPECT_EQ(str->substr(0, 4), "test");
    EXPECT_NE(str->find("string"), std::string::npos);

    str->clear();
    EXPECT_TRUE(str->empty());
}

TEST(JsonNumber, BasicOperations) {
    auto num = std::make_shared<json_number>(42.5);

    EXPECT_EQ(num->value, 42.5);
    EXPECT_EQ(num->to_int(), 42);
    EXPECT_EQ(num->to_double(), 42.5);
    EXPECT_FALSE(num->is_integer());
}

TEST(JsonNumber, IntegerOperations) {
    auto num = std::make_shared<json_number>(100);

    EXPECT_TRUE(num->is_integer());
    EXPECT_EQ(num->stringify(), "100");

    ++(*num);
    EXPECT_EQ(num->value, 101);

    *num += 10;
    EXPECT_EQ(num->value, 111);
}

TEST(JsonNumber, ArithmeticOperators) {
    auto num = std::make_shared<json_number>(10.0);

    *num += 5;
    EXPECT_EQ(num->value, 15.0);

    *num *= 2;
    EXPECT_EQ(num->value, 30.0);

    *num -= 10;
    EXPECT_EQ(num->value, 20.0);

    *num /= 4;
    EXPECT_EQ(num->value, 5.0);
}

TEST(JsonBoolean, BasicOperations) {
    auto bool_true = std::make_shared<json_boolean>(true);
    auto bool_false = std::make_shared<json_boolean>(false);

    EXPECT_TRUE(bool_true->value);
    EXPECT_FALSE(bool_false->value);
    EXPECT_EQ(bool_true->stringify(), "true");
    EXPECT_EQ(bool_false->stringify(), "false");
}

TEST(JsonBoolean, LogicalOperators) {
    auto b = std::make_shared<json_boolean>(true);

    *b &= false;
    EXPECT_FALSE(b->value);

    *b |= true;
    EXPECT_TRUE(b->value);
}

TEST(JsonParse, SimpleObject) {
    std::string json = R"({"name": "Alice", "age": 25})";
    auto data = parse(json);

    EXPECT_EQ(data.size(), 2);
    EXPECT_EQ(get_string(data["name"]), "Alice");
    EXPECT_EQ(get_number(data["age"]), 25);
}

TEST(JsonParse, NestedObject) {
    std::string json = R"({
        "user": {
            "name": "Bob",
            "age": 30
        },
        "active": true
    })";
    auto data = parse(json);

    EXPECT_TRUE(get_boolean(data["active"]));

    auto user = std::dynamic_pointer_cast<json_object>(data["user"]);
    EXPECT_EQ(get_string(user->get("name")), "Bob");
    EXPECT_EQ(get_number(user->get("age")), 30);
}

TEST(JsonParse, ArrayParsing) {
    std::string json = R"({"numbers": [1, 2, 3, 4, 5]})";
    auto data = parse(json);

    auto arr = get_array(data["numbers"]);
    EXPECT_EQ(arr.size(), 5);
    EXPECT_EQ(get_number(arr[0]), 1);
    EXPECT_EQ(get_number(arr[4]), 5);
}

TEST(JsonValue, ParsePrimitives) {
    auto str_val = json_value(R"("hello")");
    EXPECT_EQ(get_string(str_val), "hello");

    auto num_val = json_value("42");
    EXPECT_EQ(get_number(num_val), 42);

    auto bool_val = json_value("true");
    EXPECT_TRUE(get_boolean(bool_val));

    auto null_val = json_value("null");
    EXPECT_EQ(null_val, nullptr);
}

TEST(JsonMaker, FactoryFunctions) {
    auto obj = make_object();
    obj->insert("str", make_string("value"));
    obj->insert("num", make_number(123));
    obj->insert("bool", make_boolean(true));
    obj->insert("null", make_null());

    EXPECT_EQ(obj->size(), 4);
    EXPECT_EQ(get_string(obj->get("str")), "value");
    EXPECT_EQ(obj->get("null"), nullptr);
}

TEST(JsonGetter, SafeAccessors) {
    auto obj = make_object();
    obj->insert("value", make_number(42));

    auto val = try_get_number(obj->get("value"));
    EXPECT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), 42);

    auto missing = try_get_number(obj->get("missing"));
    EXPECT_FALSE(missing.has_value());
}

TEST(JsonTypeChecking, TypeValidation) {
    auto str = make_string("test");
    auto num = make_number(10);
    auto bool_val = make_boolean(true);
    auto arr = make_array();
    auto obj = make_object();

    EXPECT_TRUE(is_string(str));
    EXPECT_TRUE(is_number(num));
    EXPECT_TRUE(is_boolean(bool_val));
    EXPECT_TRUE(is_array(arr));
    EXPECT_TRUE(is_object(obj));

    EXPECT_FALSE(is_number(str));
    EXPECT_FALSE(is_string(num));
}

TEST(JsonComplexStructure, BuildAndStringify) {
    auto root = make_object();

    root->insert("title", make_string("Dashboard"));
    root->insert("count", make_number(42));

    auto tags = make_array();
    tags->push_back(make_string("json"));
    tags->push_back(make_string("cpp"));
    tags->push_back(make_string("library"));
    root->insert("tags", tags);

    auto metadata = make_object();
    metadata->insert("version", make_string("1.0"));
    metadata->insert("active", make_boolean(true));
    root->insert("metadata", metadata);

    std::string json = root->stringify();
    EXPECT_FALSE(json.empty());
    EXPECT_NE(json.find("Dashboard"), std::string::npos);
    EXPECT_NE(json.find("tags"), std::string::npos);
}

TEST(JsonGeneration, SimpleObjectToJson) {
    auto person = make_object();
    person->insert("name", make_string("Alice"));
    person->insert("age", make_number(25));
    person->insert("employed", make_boolean(true));

    std::string json = person->stringify();
    auto parsed = parse(json);

    EXPECT_EQ(get_string(parsed["name"]), "Alice");
    EXPECT_EQ(get_number(parsed["age"]), 25);
    EXPECT_TRUE(get_boolean(parsed["employed"]));
}

TEST(JsonGeneration, ArrayToJson) {
    auto colors = make_array();
    colors->push_back(make_string("red"));
    colors->push_back(make_string("green"));
    colors->push_back(make_string("blue"));

    std::string json = colors->stringify();
    EXPECT_EQ(json, R"(["red","green","blue"])");

    auto parsed = json_value(json);
    auto arr = get_array(parsed);
    EXPECT_EQ(arr.size(), 3);
    EXPECT_EQ(get_string(arr[1]), "green");
}

TEST(JsonGeneration, NestedObjectToJson) {
    auto company = make_object();
    company->insert("name", make_string("TechCorp"));

    auto address = make_object();
    address->insert("street", make_string("123 Main St"));
    address->insert("city", make_string("Boston"));
    address->insert("zip", make_number(02101));
    company->insert("address", address);

    auto employees = make_array();
    for (int i = 1; i <= 3; ++i) {
        auto emp = make_object();
        emp->insert("id", make_number(i));
        emp->insert("name", make_string("Employee" + std::to_string(i)));
        employees->push_back(emp);
    }
    company->insert("employees", employees);

    std::string json = company->stringify();
    auto parsed = parse(json);

    EXPECT_EQ(get_string(parsed["name"]), "TechCorp");

    auto parsed_addr = std::dynamic_pointer_cast<json_object>(parsed["address"]);
    EXPECT_EQ(get_string(parsed_addr->get("city")), "Boston");

    auto parsed_emps = get_array(parsed["employees"]);
    EXPECT_EQ(parsed_emps.size(), 3);

    auto first_emp = std::dynamic_pointer_cast<json_object>(parsed_emps[0]);
    EXPECT_EQ(get_number(first_emp->get("id")), 1);
}

TEST(JsonGeneration, DeepNestingToJson) {
    auto level1 = make_object();
    level1->insert("depth", make_number(1));

    auto level2 = make_object();
    level2->insert("depth", make_number(2));

    auto level3 = make_object();
    level3->insert("depth", make_number(3));

    auto level4 = make_object();
    level4->insert("depth", make_number(4));
    level4->insert("value", make_string("deep"));

    level3->insert("nested", level4);
    level2->insert("nested", level3);
    level1->insert("nested", level2);

    std::string json = level1->stringify();
    auto parsed = parse(json);

    auto l2 = std::dynamic_pointer_cast<json_object>(parsed["nested"]);
    auto l3 = std::dynamic_pointer_cast<json_object>(l2->get("nested"));
    auto l4 = std::dynamic_pointer_cast<json_object>(l3->get("nested"));

    EXPECT_EQ(get_number(l4->get("depth")), 4);
    EXPECT_EQ(get_string(l4->get("value")), "deep");
}

TEST(JsonGeneration, MixedTypesArrayToJson) {
    auto mixed = make_array();
    mixed->push_back(make_number(42));
    mixed->push_back(make_string("text"));
    mixed->push_back(make_boolean(true));
    mixed->push_back(make_null());

    auto nested_obj = make_object();
    nested_obj->insert("key", make_string("value"));
    mixed->push_back(nested_obj);

    auto nested_arr = make_array();
    nested_arr->push_back(make_number(1));
    nested_arr->push_back(make_number(2));
    mixed->push_back(nested_arr);
    std::string json = mixed->stringify();

    EXPECT_EQ(json, R"([42,"text",true,null,{"key":"value"},[1,2]])");

    auto parsed = json_value(json);
    auto parsed_arr = std::dynamic_pointer_cast<json_array>(parsed);

    EXPECT_NE(parsed_arr, nullptr);
    EXPECT_EQ(parsed_arr->size(), 6);
    EXPECT_EQ(get_number(parsed_arr->at(0)), 42);
    EXPECT_EQ(get_string(parsed_arr->at(1)), "text");
    EXPECT_TRUE(get_boolean(parsed_arr->at(2)));
    EXPECT_EQ(parsed_arr->at(3), nullptr);
    EXPECT_TRUE(is_object(parsed_arr->at(4)));
    EXPECT_TRUE(is_array(parsed_arr->at(5)));
}

TEST(JsonGeneration, CompleteUserProfile) {
    auto profile = make_object();
    profile->insert("userId", make_number(12345));
    profile->insert("username", make_string("john_doe"));
    profile->insert("email", make_string("john@example.com"));
    profile->insert("verified", make_boolean(true));

    auto settings = make_object();
    settings->insert("theme", make_string("dark"));
    settings->insert("notifications", make_boolean(true));
    settings->insert("language", make_string("en"));
    profile->insert("settings", settings);

    auto tags = make_array();
    tags->push_back(make_string("developer"));
    tags->push_back(make_string("blogger"));
    profile->insert("tags", tags);

    auto posts = make_array();
    for (int i = 1; i <= 2; ++i) {
        auto post = make_object();
        post->insert("id", make_number(i));
        post->insert("title", make_string("Post " + std::to_string(i)));
        post->insert("likes", make_number(i * 10));
        posts->push_back(post);
    }
    profile->insert("posts", posts);

    std::string json = profile->stringify();
    auto parsed = parse(json);

    EXPECT_EQ(get_number(parsed["userId"]), 12345);
    EXPECT_EQ(get_string(parsed["username"]), "john_doe");
    EXPECT_TRUE(get_boolean(parsed["verified"]));

    auto parsed_settings = std::dynamic_pointer_cast<json_object>(parsed["settings"]);
    EXPECT_EQ(get_string(parsed_settings->get("theme")), "dark");

    auto parsed_tags = get_array(parsed["tags"]);
    EXPECT_EQ(parsed_tags.size(), 2);

    auto parsed_posts = get_array(parsed["posts"]);
    EXPECT_EQ(parsed_posts.size(), 2);
    auto post1 = std::dynamic_pointer_cast<json_object>(parsed_posts[0]);
    EXPECT_EQ(get_number(post1->get("likes")), 10);
}

TEST(JsonParsing, ComplexNestedStructure) {
    std::string json = R"({
        "data": {
            "items": [
                {"id": 1, "values": [10, 20, 30]},
                {"id": 2, "values": [40, 50, 60]}
            ],
            "metadata": {
                "count": 2,
                "type": "collection"
            }
        }
    })";

    auto parsed = parse(json);
    auto data = std::dynamic_pointer_cast<json_object>(parsed["data"]);

    auto items = get_array(data->get("items"));
    EXPECT_EQ(items.size(), 2);

    auto item1 = std::dynamic_pointer_cast<json_object>(items[0]);
    auto values1 = get_array(item1->get("values"));
    EXPECT_EQ(get_number(values1[1]), 20);

    auto metadata = std::dynamic_pointer_cast<json_object>(data->get("metadata"));
    EXPECT_EQ(get_string(metadata->get("type")), "collection");
}

TEST(JsonParsing, EmptyContainers) {
    std::string json = R"({
        "emptyObject": {},
        "emptyArray": [],
        "name": "test"
    })";

    auto parsed = parse(json);
    EXPECT_EQ(parsed.size(), 3);

    auto empty_obj = std::dynamic_pointer_cast<json_object>(parsed["emptyObject"]);
    EXPECT_TRUE(empty_obj->empty());

    auto empty_arr = get_array(parsed["emptyArray"]);
    EXPECT_TRUE(empty_arr.empty());

    EXPECT_EQ(get_string(parsed["name"]), "test");
}

TEST(JsonParsing, SpecialCharactersInStrings) {
    std::string json = R"({
        "quote": "He said \"Hello\"",
        "newline": "Line1\nLine2",
        "tab": "Col1\tCol2",
        "backslash": "Path\\to\\file"
    })";

    auto parsed = parse(json);

    EXPECT_NE(get_string(parsed["quote"]).find("Hello"), std::string::npos);
    EXPECT_NE(get_string(parsed["newline"]).find("Line2"), std::string::npos);
    EXPECT_NE(get_string(parsed["tab"]).find("Col2"), std::string::npos);
    EXPECT_NE(get_string(parsed["backslash"]).find("file"), std::string::npos);
}

TEST(JsonParsing, NumericEdgeCases) {
    std::string json = R"({
        "integer": 42,
        "negative": -100,
        "decimal": 3.14159,
        "scientific": 1.5e10,
        "zero": 0
    })";

    auto parsed = parse(json);

    EXPECT_EQ(get_number(parsed["integer"]), 42);
    EXPECT_EQ(get_number(parsed["negative"]), -100);
    EXPECT_NEAR(get_number(parsed["decimal"]), 3.14159, 0.00001);
    EXPECT_EQ(get_number(parsed["scientific"]), 1.5e10);
    EXPECT_EQ(get_number(parsed["zero"]), 0);
}

TEST(JsonParsing, BooleanAndNull) {
    std::string json = R"({
        "isTrue": true,
        "isFalse": false,
        "nullValue": null
    })";

    auto parsed = parse(json);

    EXPECT_TRUE(get_boolean(parsed["isTrue"]));
    EXPECT_FALSE(get_boolean(parsed["isFalse"]));
    EXPECT_EQ(parsed["nullValue"], nullptr);
}

TEST(JsonParsing, WhitespaceHandling) {
    std::string json = R"(  {   "key1"  :  "value1"  ,  "key2"  :  42  }  )";

    auto parsed = parse(json);

    EXPECT_EQ(get_string(parsed["key1"]), "value1");
    EXPECT_EQ(get_number(parsed["key2"]), 42);
}

TEST(JsonParsing, ArrayOfObjects) {
    std::string json = R"({
        "users": [
            {"name": "Alice", "age": 25},
            {"name": "Bob", "age": 30},
            {"name": "Charlie", "age": 35}
        ]
    })";

    auto parsed = parse(json);
    auto users = get_array(parsed["users"]);

    EXPECT_EQ(users.size(), 3);

    auto bob = std::dynamic_pointer_cast<json_object>(users[1]);
    EXPECT_EQ(get_string(bob->get("name")), "Bob");
    EXPECT_EQ(get_number(bob->get("age")), 30);
}

TEST(JsonParsing, DeeplyNestedArrays) {
    std::string json = R"({
        "matrix": [
            [1, 2, 3],
            [4, 5, 6],
            [7, 8, 9]
        ]
    })";

    auto parsed = parse(json);
    auto matrix = get_array(parsed["matrix"]);

    EXPECT_EQ(matrix.size(), 3);

    auto row2 = get_array(matrix[1]);
    EXPECT_EQ(get_number(row2[0]), 4);
    EXPECT_EQ(get_number(row2[1]), 5);
    EXPECT_EQ(get_number(row2[2]), 6);
}

TEST(JsonErrorHandling, InvalidJsonThrowsException) {
    std::string invalid_json = R"({invalid json})";

    EXPECT_THROW({ parse(invalid_json); }, std::runtime_error);
}

TEST(JsonErrorHandling, MissingClosingBrace) {
    std::string invalid_json = R"({"key": "value")";

    EXPECT_THROW({ parse(invalid_json); }, std::runtime_error);
}

TEST(JsonErrorHandling, InvalidNumberFormat) {
    std::string invalid_json = R"({"number": 12.34.56})";

    EXPECT_THROW({ parse(invalid_json); }, std::runtime_error);
}

TEST(JsonErrorHandling, UnterminatedString) {
    std::string invalid_json = R"({"key": "unterminated})";

    EXPECT_THROW({ parse(invalid_json); }, std::runtime_error);
}

TEST(JsonErrorHandling, InvalidBooleanValue) {
    std::string invalid_json = R"({"flag": tru})";

    EXPECT_THROW({ parse(invalid_json); }, std::runtime_error);
}

TEST(JsonErrorHandling, TrailingComma) {
    std::string json_with_trailing = R"({"key": "value",})";

    EXPECT_THROW({ parse(json_with_trailing); }, std::runtime_error);
}

TEST(JsonErrorHandling, MissingColon) {
    std::string invalid_json = R"({"key" "value"})";

    EXPECT_THROW({ parse(invalid_json); }, std::runtime_error);
}

TEST(JsonErrorHandling, TypeMismatchAccess) {
    auto obj = make_object();
    obj->insert("number", make_number(42));

    EXPECT_THROW({ get_string(obj->get("number")); }, std::runtime_error);

    EXPECT_THROW({ get_boolean(obj->get("number")); }, std::runtime_error);
}

TEST(JsonErrorHandling, OutOfBoundsArrayAccess) {
    auto arr = make_array();
    arr->push_back(make_number(1));

    EXPECT_THROW({ arr->at(5); }, std::out_of_range);
}

TEST(JsonErrorHandling, AccessNonExistentKey) {
    auto obj = make_object();
    obj->insert("key", make_string("value"));

    auto result = obj->get("nonexistent");
    EXPECT_EQ(result, nullptr);

    EXPECT_THROW({ obj->at("nonexistent"); }, std::out_of_range);
}