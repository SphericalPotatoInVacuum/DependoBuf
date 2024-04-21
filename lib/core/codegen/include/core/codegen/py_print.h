#include <fstream>
#include <iostream>
#include <unordered_map>

namespace dbuf::gen {

class PyPrinter {
public:
  PyPrinter() = default;

  explicit PyPrinter(std::shared_ptr<std::ofstream> output);

  void init_file();

  void print_class(const std::string &name);

  void print_field(const std::string &name, const std::string &type);

private:
  static std::string get_python_type(const std::string &type);

  std::shared_ptr<std::ofstream> out_ = nullptr;

  static const std::string kReadme;
  static const std::string kImport;

  static const std::unordered_map<std::string, std::string> kBuildInTypes;
};

} // namespace dbuf::gen