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

  void print_inner_class(const std::string &name);

  void print_def_check(
      std::vector<std::string> &names,
      std::vector<std::string> &types,
      std::unordered_map<std::string, std::vector<std::string>> &fields_deps,
      const std::string &struct_name,
      int level = 2);

  void print_def_init(
    std::vector<std::string> &names,
    std::vector<std::string> &types,
    std::vector<std::vector<std::string>> &deps,
    int level = 1);

  void print_field(const std::string &name, const std::string &type, int level);

private:
  void print_line(std::vector<std::string> tokens = {}, int level = 0);

  static std::string get_python_type(const std::string &type);

  static std::string camel_to_snake(const std::string &camel_str);

  std::shared_ptr<std::ofstream> out_ = nullptr;

  static const std::unordered_map<std::string, std::string> kBuildInTypes;

  static const std::string kReadme;
  static const std::string kImports;
  static const std::string kCreateUnsigned;

  static std::vector<std::string> kTab;
};

} // namespace dbuf::gen