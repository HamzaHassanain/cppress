#include <gtest/gtest.h>

#include "../includes.hpp"

using namespace cppress::html;
using namespace cppress::html::maker;
using namespace cppress::html::getter;

TEST(Element, BasicCreationAndToString) {
    auto elem = std::make_shared<element>("div");
    EXPECT_EQ(elem->get_tag(), "div");
    EXPECT_TRUE(elem->empty());
    EXPECT_EQ(elem->size(), 0);
    EXPECT_EQ(elem->to_string(), "<div></div>");
}

TEST(Element, CreationWithTextContent) {
    auto elem = std::make_shared<element>("p", "Hello World");
    EXPECT_EQ(elem->get_text_content(), "Hello World");
    EXPECT_EQ(elem->to_string(), "<p>Hello World</p>");
}

TEST(Element, CreationWithAttributes) {
    auto elem = std::make_shared<element>(
        "div", std::map<std::string, std::string>{{"class", "container"}, {"id", "main"}});
    EXPECT_TRUE(elem->has_attribute("class"));
    EXPECT_TRUE(elem->has_attribute("id"));
    EXPECT_EQ(elem->get_attribute("class"), "container");
    EXPECT_EQ(elem->get_attribute("id"), "main");
}

TEST(Element, CreationWithTextAndAttributes) {
    auto elem = std::make_shared<element>(
        "a", "Click here", std::map<std::string, std::string>{{"href", "https://example.com"}});
    EXPECT_EQ(elem->get_text_content(), "Click here");
    EXPECT_EQ(elem->get_attribute("href"), "https://example.com");
}

TEST(Element, STLLikeChildOperations) {
    auto parent = make_element("div");
    auto child1 = make_element("p", "First");
    auto child2 = make_element("p", "Second");
    auto child3 = make_element("p", "Third");

    parent->push_back(child1);
    parent->push_back(child2);
    parent->add_child(child3);

    EXPECT_EQ(parent->size(), 3);
    EXPECT_FALSE(parent->empty());
    EXPECT_EQ(get_text(parent->front()), "First");
    EXPECT_EQ(get_text(parent->back()), "Third");
}

TEST(Element, ArrayAccessOperators) {
    auto parent = make_element("div");
    parent->push_back(make_element("p", "Zero"));
    parent->push_back(make_element("p", "One"));
    parent->push_back(make_element("p", "Two"));

    EXPECT_EQ(get_text((*parent)[0]), "Zero");
    EXPECT_EQ(get_text(parent->at(1)), "One");
    EXPECT_EQ(get_text((*parent)[2]), "Two");
}

TEST(Element, Iterator) {
    auto parent = make_element("ul");
    parent->push_back(make_element("li", "Item 1"));
    parent->push_back(make_element("li", "Item 2"));
    parent->push_back(make_element("li", "Item 3"));

    int count = 0;
    for (const auto& child : *parent) {
        count++;
        EXPECT_EQ(get_tag(child), "li");
    }
    EXPECT_EQ(count, 3);
}

TEST(Element, ReverseIterator) {
    auto parent = make_element("div");
    parent->push_back(make_element("span", "1"));
    parent->push_back(make_element("span", "2"));
    parent->push_back(make_element("span", "3"));

    std::string result;
    for (auto it = parent->rbegin(); it != parent->rend(); ++it) {
        result += get_text(*it);
    }
    EXPECT_EQ(result, "321");
}

TEST(Element, PopBackAndClear) {
    auto parent = make_element("div");
    parent->push_back(make_element("p", "First"));
    parent->push_back(make_element("p", "Second"));
    parent->push_back(make_element("p", "Third"));

    EXPECT_EQ(parent->size(), 3);

    parent->pop_back();
    EXPECT_EQ(parent->size(), 2);
    EXPECT_EQ(get_text(parent->back()), "Second");

    parent->clear();
    EXPECT_TRUE(parent->empty());
}

TEST(Element, AttributeManagement) {
    auto elem = make_element("div");

    elem->set_attribute("class", "container");
    elem->set_attribute("id", "main");
    elem->set_attribute("data-value", "123");

    EXPECT_EQ(elem->attributes_size(), 3);
    EXPECT_FALSE(elem->attributes_empty());
    EXPECT_TRUE(elem->has_attribute("class"));
    EXPECT_FALSE(elem->has_attribute("missing"));

    elem->erase_attribute("data-value");
    EXPECT_EQ(elem->attributes_size(), 2);
    EXPECT_FALSE(elem->has_attribute("data-value"));
}

TEST(Element, AttributeIteration) {
    auto elem = make_element("input");
    elem->set_attribute("type", "text");
    elem->set_attribute("name", "username");
    elem->set_attribute("placeholder", "Enter name");

    int count = 0;
    for (auto it = elem->attributes_begin(); it != elem->attributes_end(); ++it) {
        count++;
        EXPECT_FALSE(it->first.empty());
        EXPECT_FALSE(it->second.empty());
    }
    EXPECT_EQ(count, 3);
}

TEST(Element, CopyElement) {
    auto original = make_element("div");
    original->set_attribute("class", "original");
    original->push_back(make_element("p", "Content"));

    auto copy = original->copy();
    EXPECT_EQ(copy.get_tag(), original->get_tag());
    EXPECT_EQ(copy.get_attribute("class"), "original");
    EXPECT_EQ(copy.size(), 1);
}

TEST(Element, NestedStructureToString) {
    auto html = make_element("html");
    auto head = make_element("head");
    auto body = make_element("body");

    head->push_back(make_element("title", "Test Page"));
    body->push_back(make_element("h1", "Welcome"));
    body->push_back(make_element("p", "This is a test."));

    html->push_back(head);
    html->push_back(body);

    std::string result = html->to_string();

    EXPECT_NE(result.find("<html>"), std::string::npos);
    EXPECT_NE(result.find("<head>"), std::string::npos);
    EXPECT_NE(result.find("<title>Test Page</title>"), std::string::npos);
    EXPECT_NE(result.find("<h1>Welcome</h1>"), std::string::npos);

    auto html_back = parse(result)[0];
    EXPECT_EQ(html_back->to_string(), result);
}

TEST(SelfClosingElement, CannotHaveChildren) {
    auto br = std::make_shared<self_closing_element>("br");
    auto child = make_element("div");

    br->add_child(child);
    EXPECT_EQ(br->size(), 0);
    EXPECT_TRUE(br->get_children().empty());
}

TEST(SelfClosingElement, CannotHaveTextContent) {
    auto hr = std::make_shared<self_closing_element>("hr");
    hr->set_text_content("This should be ignored");

    EXPECT_TRUE(hr->get_text_content().empty());
}

TEST(Document, BasicCreation) {
    document doc;
    EXPECT_EQ(doc.get_doctype(), "html");
    EXPECT_TRUE(doc.empty());
}

TEST(Document, CustomDoctype) {
    document doc("html PUBLIC \"-//W3C//DTD HTML 4.01//EN\"");
    EXPECT_NE(doc.get_doctype(), "html");

    std::string expected_doc = "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\"><html></html>";

    EXPECT_EQ(doc.to_string(), expected_doc);
}

TEST(Document, AddingChildren) {
    document doc;
    auto body = make_element("body");
    body->push_back(make_element("h1", "Title"));

    doc.add_child(body);
    EXPECT_EQ(doc.size(), 1);
    EXPECT_FALSE(doc.empty());
}

TEST(Document, ToStringWithDoctype) {
    document doc;
    doc.add_child(make_element("body"));

    std::string result = doc.to_string();
    EXPECT_NE(result.find("<!DOCTYPE html>"), std::string::npos);
    EXPECT_NE(result.find("<html>"), std::string::npos);
    EXPECT_NE(result.find("</html>"), std::string::npos);
}

TEST(Document, STLLikeOperations) {
    document doc;
    doc.push_back(make_element("head"));
    doc.push_back(make_element("body"));

    EXPECT_EQ(doc.size(), 2);
    EXPECT_EQ(get_tag(doc.at(0)), "head");
    EXPECT_EQ(get_tag(doc[1]), "body");

    doc.clear();
    EXPECT_TRUE(doc.empty());
}

TEST(Document, Iterator) {
    document doc;
    doc.push_back(make_element("head"));
    doc.push_back(make_element("body"));

    int count = 0;
    for (const auto& elem : doc) {
        count++;
        EXPECT_TRUE(is_standard_element(elem));
    }
    EXPECT_EQ(count, 2);
}

TEST(Maker, ElementFactories) {
    auto div = make_div();
    EXPECT_EQ(div->get_tag(), "div");

    auto para = make_paragraph("Test text");
    EXPECT_EQ(para->get_text_content(), "Test text");

    auto h1 = make_heading(1, "Title");
    EXPECT_EQ(h1->get_tag(), "h1");
    EXPECT_EQ(h1->get_text_content(), "Title");

    auto span = make_span("Inline");
    EXPECT_EQ(span->get_tag(), "span");
}

TEST(Maker, LinkAndImage) {
    auto link = make_link("https://example.com", "Click");
    EXPECT_EQ(link->get_tag(), "a");
    EXPECT_EQ(link->get_attribute("href"), "https://example.com");
    EXPECT_EQ(link->get_text_content(), "Click");

    auto img = make_image("photo.jpg", "Photo");
    EXPECT_TRUE(is_self_closing(img));
    EXPECT_EQ(img->get_attribute("src"), "photo.jpg");
    EXPECT_EQ(img->get_attribute("alt"), "Photo");
}

TEST(Maker, SelfClosingFactories) {
    auto br = make_br();
    EXPECT_TRUE(is_self_closing(br));
    EXPECT_EQ(br->get_tag(), "br");

    auto hr = make_hr();
    EXPECT_TRUE(is_self_closing(hr));
    EXPECT_EQ(hr->get_tag(), "hr");

    auto input = make_input("text", "username");
    EXPECT_TRUE(is_self_closing(input));
    EXPECT_EQ(input->get_attribute("type"), "text");
    EXPECT_EQ(input->get_attribute("name"), "username");
}

TEST(Maker, ButtonFactory) {
    auto btn = make_button("Submit", "submit");
    EXPECT_EQ(btn->get_tag(), "button");
    EXPECT_EQ(btn->get_text_content(), "Submit");
    EXPECT_EQ(btn->get_attribute("type"), "submit");
}

TEST(Getter, BasicGetters) {
    auto elem = make_element("div", "Content");
    elem->set_attribute("id", "test");

    EXPECT_EQ(get_tag(elem), "div");
    EXPECT_EQ(get_text(elem), "Content");
    EXPECT_EQ(get_attribute(elem, "id"), "test");
}

TEST(Getter, SafeGetters) {
    auto elem = make_element("div");

    auto text = try_get_text(elem);
    EXPECT_TRUE(text.has_value());
    EXPECT_TRUE(text.value().empty());

    auto missing = try_get_attribute(elem, "missing");
    EXPECT_FALSE(missing.has_value());
}

TEST(Getter, GetChildren) {
    auto parent = make_element("div");
    parent->push_back(make_element("p"));
    parent->push_back(make_element("span"));

    auto children = get_children(parent);
    EXPECT_EQ(children.size(), 2);
}

TEST(Getter, TypeCasting) {
    auto img = make_image("test.jpg");

    auto self_closing = try_as_self_closing(img);
    EXPECT_NE(self_closing, nullptr);

    auto regular = make_div();
    auto not_self_closing = try_as_self_closing(regular);
    EXPECT_EQ(not_self_closing, nullptr);
}

TEST(Getter, NullElementHandling) {
    std::shared_ptr<element> null_elem = nullptr;

    EXPECT_THROW(get_tag(null_elem), std::runtime_error);
    EXPECT_THROW(get_text(null_elem), std::runtime_error);
    EXPECT_THROW(get_attribute(null_elem, "any"), std::runtime_error);

    EXPECT_FALSE(try_get_tag(null_elem).has_value());
    EXPECT_FALSE(try_get_text(null_elem).has_value());
}

TEST(TypeChecking, ElementTypes) {
    auto regular = make_div();
    auto self_close = make_br();

    EXPECT_TRUE(is_standard_element(regular));
    EXPECT_FALSE(is_self_closing(regular));

    EXPECT_TRUE(is_self_closing(self_close));
    EXPECT_FALSE(is_standard_element(self_close));
}

TEST(ComplexStructure, BlogPost) {
    auto article = make_element("article");
    article->set_attribute("class", "blog-post");

    auto header = make_element("header");
    header->push_back(make_heading(1, "My Blog Post"));
    header->push_back(make_element("time", "2025-10-22"));

    auto content = make_element("div");
    content->set_attribute("class", "content");
    content->push_back(make_paragraph("First paragraph."));
    content->push_back(make_paragraph("Second paragraph."));
    content->push_back(make_image("image.jpg", "Article image"));

    article->push_back(header);
    article->push_back(content);

    std::string html = article->to_string();
    EXPECT_NE(html.find("blog-post"), std::string::npos);
    EXPECT_NE(html.find("<h1>My Blog Post</h1>"), std::string::npos);
    EXPECT_NE(html.find("First paragraph"), std::string::npos);

    auto article_back = parse(html)[0];

    EXPECT_EQ(article_back->to_string(), html);
}

TEST(ComplexStructure, NavigationMenu) {
    auto nav = make_element("nav");
    auto ul = make_element("ul");

    for (int i = 1; i <= 5; ++i) {
        auto li = make_element("li");
        auto link = make_link("#section" + std::to_string(i), "Link " + std::to_string(i));
        li->push_back(link);
        ul->push_back(li);
    }

    nav->push_back(ul);

    EXPECT_EQ(ul->size(), 5);
    std::string html = nav->to_string();
    EXPECT_NE(html.find("<nav>"), std::string::npos);
    EXPECT_NE(html.find("<ul>"), std::string::npos);
    EXPECT_NE(html.find("Link 3"), std::string::npos);

    auto nav_back = parse(html)[0];
    EXPECT_EQ(nav_back->to_string(), html);
}

TEST(ComplexStructure, FormWithInputs) {
    auto form = make_element("form");
    form->set_attribute("action", "/submit");
    form->set_attribute("method", "post");

    auto name_input = make_input("text", "username");
    name_input->set_attribute("placeholder", "Username");

    auto email_input = make_input("email", "email");
    email_input->set_attribute("required", "required");

    auto submit = make_button("Submit", "submit");

    form->push_back(name_input);
    form->push_back(email_input);
    form->push_back(submit);

    std::string html = form->to_string();
    EXPECT_NE(html.find("action=\"/submit\""), std::string::npos);
    EXPECT_NE(html.find("type=\"email\""), std::string::npos);
}

TEST(ComplexStructure, NestedTable) {
    auto table = make_element("table");
    auto tbody = make_element("tbody");

    for (int i = 0; i < 3; ++i) {
        auto tr = make_element("tr");
        for (int j = 0; j < 4; ++j) {
            auto td = make_element("td", "Cell " + std::to_string(i) + "," + std::to_string(j));
            tr->push_back(td);
        }
        tbody->push_back(tr);
    }

    table->push_back(tbody);

    EXPECT_EQ(tbody->size(), 3);
    EXPECT_EQ(tbody->at(0)->size(), 4);

    std::string html = table->to_string();
    EXPECT_NE(html.find("Cell 2,3"), std::string::npos);
}

TEST(ErrorHandling, OutOfBoundsAccess) {
    auto parent = make_element("div");
    parent->push_back(make_element("p"));

    EXPECT_THROW(parent->at(5), std::out_of_range);
    EXPECT_THROW(parent->pop_back(); parent->pop_back(), std::out_of_range);
}

TEST(ErrorHandling, EmptyContainerAccess) {
    auto empty = make_element("div");

    EXPECT_THROW(empty->front(), std::out_of_range);
    EXPECT_THROW(empty->back(), std::out_of_range);
    EXPECT_THROW(empty->pop_back(), std::out_of_range);
}

TEST(ErrorHandling, GetterWithNull) {
    std::shared_ptr<element> null_ptr = nullptr;

    EXPECT_THROW(as_self_closing(null_ptr), std::runtime_error);
    EXPECT_THROW(get_children(null_ptr), std::runtime_error);
}

TEST(Performance, LargeHierarchy) {
    auto root = make_element("div");
    root->reserve(1000);

    for (int i = 0; i < 1000; ++i) {
        auto child = make_element("div");
        child->set_attribute("data-id", std::to_string(i));
        child->set_text_content("Content " + std::to_string(i));
        root->push_back(child);
    }

    EXPECT_EQ(root->size(), 1000);
    EXPECT_EQ(get_text(root->at(500)), "Content 500");
    EXPECT_EQ(root->at(999)->get_attribute("data-id"), "999");
}

TEST(Performance, DeepNesting) {
    auto root = make_element("div");
    auto current = root;

    for (int i = 0; i < 100; ++i) {
        auto child = make_element("div");
        child->set_attribute("level", std::to_string(i));
        current->push_back(child);
        current = child;
    }

    current->set_text_content("Deepest level");

    std::string html = root->to_string();
    EXPECT_NE(html.find("Deepest level"), std::string::npos);
    EXPECT_NE(html.find("level=\"50\""), std::string::npos);
}

TEST(Performance, ManyAttributes) {
    auto elem = make_element("div");

    for (int i = 0; i < 100; ++i) {
        elem->set_attribute("data-attr-" + std::to_string(i), "value-" + std::to_string(i));
    }

    EXPECT_EQ(elem->attributes_size(), 100);
    EXPECT_TRUE(elem->has_attribute("data-attr-50"));
    EXPECT_EQ(elem->get_attribute("data-attr-99"), "value-99");
}

TEST(Performance, StringGeneration) {
    auto root = make_element("html");
    auto body = make_element("body");

    for (int i = 0; i < 100; ++i) {
        auto section = make_element("section");
        section->push_back(make_heading(2, "Section " + std::to_string(i)));
        section->push_back(make_paragraph("Lorem ipsum dolor sit amet."));
        body->push_back(section);
    }

    root->push_back(body);

    std::string html = root->to_string();
    EXPECT_GT(html.length(), 1000);
    EXPECT_NE(html.find("Section 50"), std::string::npos);
}

TEST(Performance, IterationSpeed) {
    auto parent = make_element("ul");

    for (int i = 0; i < 1000; ++i) {
        parent->push_back(make_element("li", std::to_string(i)));
    }

    int sum = 0;
    for (const auto& child : *parent) {
        sum += std::stoi(get_text(child));
    }

    EXPECT_EQ(sum, 499500);
}

TEST(Performance, CopyLargeStructure) {
    auto original = make_element("div");

    for (int i = 0; i < 50; ++i) {
        auto child = make_element("p", "Content " + std::to_string(i));
        child->set_attribute("id", "p-" + std::to_string(i));
        original->push_back(child);
    }

    auto copy = original->copy();

    EXPECT_EQ(copy.size(), original->size());
    EXPECT_EQ(copy.at(25)->get_text_content(), "Content 25");
}

TEST(EdgeCases, EmptyElements) {
    auto empty_div = make_element("div");
    EXPECT_EQ(empty_div->to_string(), "<div></div>");

    auto empty_p = make_element("p", "");
    EXPECT_EQ(empty_p->to_string(), "<p></p>");
}

TEST(EdgeCases, SpecialCharactersInText) {
    auto para = make_element("p", "Text with <special> & \"characters\"");
    std::string html = para->to_string();
    EXPECT_NE(html.find("special"), std::string::npos);
}

TEST(EdgeCases, SpecialCharactersInAttributes) {
    auto elem = make_element("div");
    elem->set_attribute("data-value", "value with \"quotes\" & ampersand");
    elem->set_attribute("class", "class-name-123");

    std::string html = elem->to_string();
    EXPECT_NE(html.find("data-value"), std::string::npos);
}

TEST(EdgeCases, VeryLongTextContent) {
    std::string long_text(10000, 'a');
    auto para = make_element("p", long_text);

    EXPECT_EQ(para->get_text_content().length(), 10000);

    std::string html = para->to_string();
    EXPECT_GT(html.length(), 10000);
}

TEST(EdgeCases, ManyEmptyChildren) {
    auto parent = make_element("div");

    for (int i = 0; i < 100; ++i) {
        parent->push_back(make_element("div"));
    }

    EXPECT_EQ(parent->size(), 100);

    for (const auto& child : *parent) {
        EXPECT_TRUE(child->empty());
    }
}

TEST(EdgeCases, AlternatingElementTypes) {
    auto parent = make_element("div");

    for (int i = 0; i < 50; ++i) {
        if (i % 2 == 0) {
            parent->push_back(make_element("p", "Text"));
        } else {
            parent->push_back(make_br());
        }
    }

    EXPECT_EQ(parent->size(), 50);

    std::string html = parent->to_string();

    auto parent_back = parse(html)[0];
    EXPECT_EQ(parent_back->size(), 50);

    int paragraph_count = 0;
    int br_count = 0;
    for (const auto& child : *parent_back) {
        if (is_self_closing(child)) {
            br_count++;
        } else {
            paragraph_count++;
        }
    }

    EXPECT_EQ(paragraph_count, 25);
    EXPECT_EQ(br_count, 25);
}

TEST(EdgeCases, EmptyAttributeValue) {
    auto elem = make_element("div");
    elem->set_attribute("data-empty", "");

    EXPECT_TRUE(elem->has_attribute("data-empty"));
    EXPECT_EQ(elem->get_attribute("data-empty"), "");
}

TEST(EdgeCases, GetNonExistentAttribute) {
    auto elem = make_element("div");
    EXPECT_EQ(elem->get_attribute("missing"), "");
}

TEST(Integration, CompleteHTMLDocument) {
    document doc{};

    auto head = make_element("head");
    head->push_back(make_element("title", "Complete Page"));
    auto meta = make_self_closing("meta", std::map<std::string, std::string>{{"charset", "UTF-8"}});
    head->push_back(meta);

    auto body = make_element("body");
    body->set_attribute("class", "main-body");

    auto header = make_element("header");
    header->push_back(make_heading(1, "Welcome"));

    auto main = make_element("main");
    auto article = make_element("article");
    article->push_back(make_heading(2, "Article Title"));
    article->push_back(make_paragraph("Article content goes here."));
    main->push_back(article);

    auto footer = make_element("footer");
    footer->push_back(make_paragraph("Copyright 2025"));

    body->push_back(header);
    body->push_back(main);
    body->push_back(footer);

    doc.push_back(head);
    doc.push_back(body);

    std::string html = doc.to_string();
    auto temp = html;

    EXPECT_NE(html.find("<!DOCTYPE html>"), std::string::npos);
    EXPECT_NE(html.find("<head>"), std::string::npos);
    EXPECT_NE(html.find("<title>Complete Page</title>"), std::string::npos);
    EXPECT_NE(html.find("<body"), std::string::npos);
    EXPECT_NE(html.find("main-body"), std::string::npos);
    EXPECT_NE(html.find("Article Title"), std::string::npos);
    EXPECT_NE(html.find("Copyright 2025"), std::string::npos);

    auto doc_back = parse(html);

    EXPECT_EQ(doc_back[0]->to_string() + doc_back[1]->to_string(), temp);
}

TEST(SubstituteParams, EmptyText) {
    std::string text = "";
    std::map<std::string, std::string> params = {{"key", "value"}};

    std::string result = substitute_params(text, params);
    EXPECT_EQ(result, "");
}

TEST(SubstituteParams, EmptyParametersMap) {
    std::string text = "Hello {{name}}!";
    std::map<std::string, std::string> params;

    std::string result = substitute_params(text, params);
    EXPECT_EQ(result, "Hello {{name}}!");
}

TEST(SubstituteParams, HTMLTemplateSubstitution) {
    std::string html_template = "<h1>{{title}}</h1><p>{{content}}</p>";
    std::map<std::string, std::string> params = {{"title", "Welcome"},
                                                 {"content", "This is the main content."}};

    std::string result = substitute_params(html_template, params);
    EXPECT_EQ(result, "<h1>Welcome</h1><p>This is the main content.</p>");
}

TEST(SubstituteParams, ParameterWithSpecialCharacters) {
    std::string text = "Message: {{message}}";
    std::map<std::string, std::string> params = {{"message", "Hello <world> & \"friends\"!"}};

    std::string result = substitute_params(text, params);
    EXPECT_EQ(result, "Message: Hello <world> & \"friends\"!");
}

TEST(SubstituteParams, ConsecutivePlaceholders) {
    std::string text = "{{first}}{{second}}{{third}}";
    std::map<std::string, std::string> params = {{"first", "A"}, {"second", "B"}, {"third", "C"}};

    std::string result = substitute_params(text, params);
    EXPECT_EQ(result, "ABC");
}

TEST(SubstituteParams, PlaceholderAtStartAndEnd) {
    std::string text = "{{start}} middle content {{end}}";
    std::map<std::string, std::string> params = {{"start", "BEGIN"}, {"end", "FINISH"}};

    std::string result = substitute_params(text, params);
    EXPECT_EQ(result, "BEGIN middle content FINISH");
}

TEST(SubstituteParams, LongParameterValue) {
    std::string long_value(1000, 'x');
    std::string text = "Content: {{data}}";
    std::map<std::string, std::string> params = {{"data", long_value}};

    std::string result = substitute_params(text, params);
    EXPECT_EQ(result, "Content: " + long_value);
    EXPECT_EQ(result.length(), 1009);
}

TEST(SubstituteParams, ComplexHTMLTemplate) {
    std::string template_html = R"(
        <article class="{{class}}">
            <h2>{{title}}</h2>
            <p>Author: {{author}}</p>
            <div class="content">{{body}}</div>
            <footer>Posted on {{date}}</footer>
        </article>
    )";

    std::map<std::string, std::string> params = {{"class", "blog-post"},
                                                 {"title", "My Article"},
                                                 {"author", "John Doe"},
                                                 {"body", "This is the article content."},
                                                 {"date", "2025-10-22"}};

    std::string result = substitute_params(template_html, params);

    EXPECT_NE(result.find("blog-post"), std::string::npos);
    EXPECT_NE(result.find("My Article"), std::string::npos);
    EXPECT_NE(result.find("John Doe"), std::string::npos);
    EXPECT_NE(result.find("This is the article content."), std::string::npos);
    EXPECT_NE(result.find("2025-10-22"), std::string::npos);
}

TEST(SubstituteParams, NestedBracesNotMatched) {
    std::string text = "Value: {{{param}}}";
    std::map<std::string, std::string> params = {{"param", "test"}};

    std::string result = substitute_params(text, params);
    EXPECT_EQ(result, "Value: {test}");
}

TEST(SubstituteParams, ManyParameters) {
    std::string text = "";
    std::map<std::string, std::string> params;

    for (int i = 0; i < 50; ++i) {
        std::string key = "param" + std::to_string(i);
        text += "{{" + key + "}} ";
        params[key] = std::to_string(i);
    }

    std::string result = substitute_params(text, params);

    for (int i = 0; i < 50; ++i) {
        EXPECT_NE(result.find(std::to_string(i)), std::string::npos);
    }
    EXPECT_EQ(result.find("{{"), std::string::npos);
}
