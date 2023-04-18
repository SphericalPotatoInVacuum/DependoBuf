#include "core/parser/expression.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace dbuf::parser {

void FieldInitialization::AddField(
    std::string field_identifier,
    std::unique_ptr<Expression> expression) {
  fields_.emplace_back(std::move(field_identifier), std::move(expression));
}

} // namespace dbuf::parser
