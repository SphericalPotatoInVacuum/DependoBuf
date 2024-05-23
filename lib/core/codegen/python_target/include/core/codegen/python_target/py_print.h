#include <fstream>
#include <memory>
#include <vector>

namespace dbuf::gen {

class PyPrinter {
public:
  PyPrinter() = default;

  explicit PyPrinter(std::shared_ptr<std::ofstream> output);

  void init_file();

  void print_def_is_consistent();

  void print_line(const std::vector<std::string> &tokens, int level = 0);

  void print_line(std::vector<std::string> &&tokens = {}, int level = 0);

  void print_outer_class(const std::string &name);

  void print_inner_class(const std::string &name, int level = 1);

  void print_inner_class_field(const std::string &name, const std::string &type, int level = 2);

  void print_def_check_message(
      const std::vector<std::string> &dep_names,
      const std::vector<std::string> &dep_types,
      const std::vector<std::string> &field_names,
      const std::vector<std::string> &field_deps,
      int level = 2);

  void print_def_check_enum(
      const std::vector<std::string> &dep_names,
      const std::vector<std::string> &dep_types,
      const std::vector<std::string> &field_names,
      const std::vector<std::string> &field_deps,
      const std::string &struct_name,
      const std::string &constructor_name,
      int level = 2);

  void print_type(const std::string &struct_name, const std::vector<std::string> &inner_types, int level = 1);

  void print_def_possible_types(
      const std::vector<std::string> &names,
      const std::vector<std::string> &types,
      const std::vector<std::string> &expected_params_matrix,
      const std::vector<std::vector<std::string>> &possible_types_matrix,
      int level = 1);

  void print_def_init(
      const std::vector<std::string> &names,
      const std::vector<std::string> &types,
      const std::vector<std::string> &deps,
      int level = 1);

  void print_method_construct(
      const std::string &message_name,
      const std::vector<std::string> &names,
      const std::vector<std::string> &types,
      int level = 1);

  void print_enum_constructor(
      const std::string &subclass,
      const std::vector<std::string> &names,
      const std::vector<std::string> &types,
      int level = 1);

private:
  void print_def_with_typed_args(
      const std::string &name,
      const std::string &first_arg,
      const std::vector<std::string> &arg_names,
      const std::vector<std::string> &arg_types,
      int level,
      const std::string &res_type);

  void print_instance_method(
      const std::string &name,
      const std::vector<std::string> &arg_names,
      const std::vector<std::string> &arg_types,
      int level,
      const std::string &res_type);

  void print_instance_method(
      const std::string &name,
      const std::vector<std::string> &arg_names,
      const std::vector<std::string> &arg_types,
      int level);

  void print_class_method(
      const std::string &name,
      const std::vector<std::string> &arg_names,
      const std::vector<std::string> &arg_types,
      int level,
      const std::string &res_type);

  void print_class_method(
      const std::string &name,
      const std::vector<std::string> &arg_names,
      const std::vector<std::string> &arg_types,
      int level);

  void print_constructor(
      const std::string &def_name,
      const std::string &struct_name,
      const std::vector<std::string> &names,
      const std::vector<std::string> &types,
      int level);

  void print_var_deps(const std::string &dep_name, const std::string &deps, int level);

  void print_consistency_check(const std::string &expected, const std::vector<std::string> &types, int level);

  void print_dbuf_error();

  std::shared_ptr<std::ofstream> out_ = nullptr;
  std::vector<std::string> tabs_;

  static const std::string kReadme;
  static const std::string kImports;
  static const std::string kCreateUnsigned;
};

} // namespace dbuf::gen