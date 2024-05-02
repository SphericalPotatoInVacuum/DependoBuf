#include <string>
#include <variant>

namespace dbuf {
struct Independent;

struct Independent {
  struct First {
    int a;
    bool b;
    bool check() const {
      return true;
    }
  };

  struct Second {
    bool c;
    int d;
    bool check() const {
      return true;
    }
  };

  struct Third {
    std::string e;
    std::string f;
    bool check() const {
      return true;
    }
  };

  std::variant<First, Second, Third> value;
  bool check() const {
    return false || (std::holds_alternative<First>(value) && std::get<First>(value).check()) ||
           (std::holds_alternative<Second>(value) && std::get<Second>(value).check()) ||
           (std::holds_alternative<Third>(value) && std::get<Third>(value).check());
  }
};

} // namespace dbuf