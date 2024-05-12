#include <string>
#include <unordered_map>
#include <vector>

namespace dbuf::gen {

const std::unordered_map<std::string, std::string> BUILD_IN_TYPES = {
    {"Int", "int"},
    {"Unsigned", "Unsigned"},
    {"Float", "float"},
    {"String", "str"},
    {"Bool", "bool"},
};

std::string camel_to_snake(const std::string &camel_str);

std::string get_python_type(const std::string &type, bool inside_outer_class = false);

std::string
typed_args(const std::vector<std::string> &names, const std::vector<std::string> &types, const std::string &first_arg);

std::string untyped_args(const std::vector<std::string> &names);

std::string py_tuple(const std::vector<std::string> &names);

bool tuple_is_empty(const std::string &tuple_str);

} // namespace dbuf::gen
