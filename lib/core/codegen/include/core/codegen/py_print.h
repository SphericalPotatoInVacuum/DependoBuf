#include <iostream>

namespace dbuf::gen {

class PyPrinter {
public:
  explicit PyPrinter(std::ofstream &out): out_(out) {}

  void print_class(std::string &name);

  void print_field(std::string &type, std::string &name);

private:
  std::ofstream &out_;
};

}