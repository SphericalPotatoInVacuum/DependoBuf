#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace dbuf::gen {

class PyPrinter {
public:
  PyPrinter() = default;

  explicit PyPrinter(std::shared_ptr<std::ofstream> output);

  void init_file();

  void print_outer_class(const std::string &name);

  void print_inner_class(const std::string &name, int level = 1);

  void print_inner_class_field(const std::string &name, const std::string &type, int level = 2);

  void print_def_check(
      std::vector<std::string> &names,
      std::vector<std::string> &types,
      std::unordered_map<std::string, std::vector<std::string>> &fields_deps,
      const std::string &struct_name,
      int level = 2);

  void print_type(const std::string &struct_name, std::vector<std::string> &inner_types, int level = 1);

  void print_dep_deps(std::string &dep_name, std::vector<std::string> &deps, int level = 1);

  void print_def_possible_types(
      std::vector<std::string> &names,
      std::vector<std::string> &types,
      int level = 1);

  void print_def_init(
      std::vector<std::string> &names,
      std::vector<std::string> &types,
      int level = 1);
  
  void print_method_construct(const std::string &message_name,
      std::vector<std::string> &names,
      std::vector<std::string> &types,
      int level = 1);
  
  void print_enum_constructor(const std::string &subclass,
      std::vector<std::string> &names,
      std::vector<std::string> &types,
      int level = 1);

private:
  void print_line(std::vector<std::string> tokens = {}, int level = 0);

  void print_def_with_typed_args(
      std::string name,
      std::string first_arg,
      std::vector<std::string> &arg_names,
      std::vector<std::string> &arg_types,
      int level,
      std::string res_type);
  
  void print_instance_method(
      std::string name,
      std::vector<std::string> &arg_names,
      std::vector<std::string> &arg_types,
      int level,
      std::string res_type = "None");
    
  void print_class_method(
      std::string name,
      std::vector<std::string> &arg_names,
      std::vector<std::string> &arg_types,
      int level,
      std::string res_type = "None");
  
  void print_constructor(std::string def_name,
      const std::string &struct_name,
      std::vector<std::string> &names,
      std::vector<std::string> &types,
      int level);

  static std::string get_python_type(const std::string &type, bool inside_outer_class = false);

  static std::string camel_to_snake(const std::string &camel_str);

  static std::string typed_args(std::vector<std::string> &names,
      std::vector<std::string> &types,
      std::string first_arg);

  static std::string untyped_args(std::vector<std::string> &names);

  std::shared_ptr<std::ofstream> out_ = nullptr;
  std::vector<std::string> tabs_;

  static const std::unordered_map<std::string, std::string> kBuildInTypes;

  static const std::string kReadme;
  static const std::string kImports;
  static const std::string kCreateUnsigned;
};

} // namespace dbuf::gen