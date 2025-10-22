#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>
namespace hh_json
{
    class JsonObject
    {
        std::unordered_map<std::string, std::shared_ptr<JsonObject>> data;

    public:
        JsonObject();
        JsonObject(const std::unordered_map<std::string, std::shared_ptr<JsonObject>> &initial_data);
        virtual ~JsonObject();

        virtual bool set_json_data(const std::string &jsonString);
        virtual void insert(const std::string &key, std::shared_ptr<JsonObject> value);
        virtual void erase(const std::string &key);
        virtual std::shared_ptr<JsonObject> get(const std::string &key) const;
        virtual std::string stringify() const;
        virtual void clear();

        const std::unordered_map<std::string, std::shared_ptr<JsonObject>> &get_data() const;

        std::shared_ptr<JsonObject> &operator[](const std::string &key);

        bool has_key(const std::string &key) const;
    };
}