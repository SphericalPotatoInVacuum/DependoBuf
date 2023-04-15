#include "expression.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace dbuf::parser {

  void FieldInitialization::AddField(const std::string &field_identifier, Expression& expression){
    fields_.push_back(std::make_pair(field_identifier, expression));
  }

} // namespace dbuf::parser