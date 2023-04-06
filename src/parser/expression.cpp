#include <memory>
#include <string>
#include <vector>

class Expression {
public:
  virtual ~Expression() {}
  virtual std::shared_ptr<Primary> Evaluate() = 0;
};

class BinaryExpression : public Expression {
public:
  BinaryExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
      : left_(left), right_(right) {}

protected:
  std::shared_ptr<Expression> left_;
  std::shared_ptr<Expression> right_;
};

class PlusExpression : public BinaryExpression {
public:
  PlusExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
      : BinaryExpression(left, right) {}

  std::shared_ptr<Primary> Evaluate() override {
    std::shared_ptr<Primary> left  = left_->Evaluate();
    std::shared_ptr<Primary> right = right_->Evaluate();

    return left->operator+(right);
  }
};

class MinusExpression : public BinaryExpression {
public:
  MinusExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
      : BinaryExpression(left, right) {}
  std::shared_ptr<Primary> Evaluate() override {
    std::shared_ptr<Primary> left  = left_->Evaluate();
    std::shared_ptr<Primary> right = right_->Evaluate();

    return left->operator-(right);
  }
};

class StarExpression : public BinaryExpression {
public:
  StarExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
      : BinaryExpression(left, right) {}
  std::shared_ptr<Primary> Evaluate() override {
    std::shared_ptr<Primary> left  = left_->Evaluate();
    std::shared_ptr<Primary> right = right_->Evaluate();

    return left->operator*(right);
  }
};

class SlashExpression : public BinaryExpression {
public:
  SlashExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
      : BinaryExpression(left, right) {}
  std::shared_ptr<Primary> Evaluate() override {
    std::shared_ptr<Primary> left  = left_->Evaluate();
    std::shared_ptr<Primary> right = right_->Evaluate();

    return left->operator/(right);
  }
};

class BangEqualExpression : public BinaryExpression {
public:
  BangEqualExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
      : BinaryExpression(left, right) {}
  std::shared_ptr<Primary> Evaluate() override {
    std::shared_ptr<Primary> left  = left_->Evaluate();
    std::shared_ptr<Primary> right = right_->Evaluate();

    return left->operator!=(right);
  }
};

class GreaterEqualExpression : public BinaryExpression {
public:
  GreaterEqualExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
      : BinaryExpression(left, right) {}
  std::shared_ptr<Primary> Evaluate() override {
    std::shared_ptr<Primary> left  = left_->Evaluate();
    std::shared_ptr<Primary> right = right_->Evaluate();

    return left->operator>=(right);
  }
};

class LessEqualExpression : public BinaryExpression {
public:
  LessEqualExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
      : BinaryExpression(left, right) {}
  std::shared_ptr<Primary> Evaluate() override {
    std::shared_ptr<Primary> left  = left_->Evaluate();
    std::shared_ptr<Primary> right = right_->Evaluate();

    return left->operator<=(right);
  }
};

class AndExpression : public BinaryExpression {
public:
  AndExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
      : BinaryExpression(left, right) {}
  std::shared_ptr<Primary> Evaluate() override {
    std::shared_ptr<Primary> left  = left_->Evaluate();
    std::shared_ptr<Primary> right = right_->Evaluate();

    return left->operator&&(right);
  }
};

class OrExpression : public BinaryExpression {
public:
  OrExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
      : BinaryExpression(left, right) {}
  std::shared_ptr<Primary> Evaluate() override {
    std::shared_ptr<Primary> left  = left_->Evaluate();
    std::shared_ptr<Primary> right = right_->Evaluate();

    return left->operator||(right);
  }
};

class LessExpression : public BinaryExpression {
public:
  LessExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
      : BinaryExpression(left, right) {}
  std::shared_ptr<Primary> Evaluate() override {
    std::shared_ptr<Primary> left  = left_->Evaluate();
    std::shared_ptr<Primary> right = right_->Evaluate();

    return left->operator<(right);
  }
};

class EqualExpression : public BinaryExpression {
public:
  EqualExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
      : BinaryExpression(left, right) {}
  std::shared_ptr<Primary> Evaluate() override {
    std::shared_ptr<Primary> left  = left_->Evaluate();
    std::shared_ptr<Primary> right = right_->Evaluate();

    return left->operator==(right);
  }
};

class GreaterExpression : public BinaryExpression {
public:
  GreaterExpression(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
      : BinaryExpression(left, right) {}
  std::shared_ptr<Primary> Evaluate() override {
    std::shared_ptr<Primary> left  = left_->Evaluate();
    std::shared_ptr<Primary> right = right_->Evaluate();

    return left->operator>(right);
  }
};

class TypeExpression : public Expression {
public:
  TypeExpression(TypeExpression type_expression) : type_(t) {}

private:
  TypeExpression type_expression_;
};

class UnaryExpression : public Expression {
public:
  UnaryExpression(std::shared_ptr<Expression> expression) : expression(expression) {}

protected:
  std::shared_ptr<Primary> expression;
};

class MinusExpression : public UnaryExpression {
public:
  MinusExpression(std::shared_ptr<Expression> expression) : UnaryExpression(expression) {}
  std::shared_ptr<Primary> Evaluate() { return (expression->Evaluate())->operator+(); }
};

class BangExpression : public UnaryExpression {
public:
  BangExpression(std::unique_ptr<Expression> expression) : expression_(std::move(expression)) {}
  std::shared_ptr<Primary> Evaluate() override { return (expression->Evaluate())->operator!(); }

private:
  std::unique_ptr<Expression> expression_;
};

class Primary {
public:
  virtual ~Primary() {}

  // Boolean operators
  virtual std::shared_ptr<Primary> operator!() const = 0; // Logical NOT operator
  virtual std::shared_ptr<Primary>
  operator&&(const std::shared_ptr<Primary> other) const = 0; // Logical AND operator
  virtual std::shared_ptr<Primary>
  operator||(const std::shared_ptr<Primary> other) const = 0; // Logical OR operator

  // Comparison operators
  virtual std::shared_ptr<Primary> operator==(const std::shared_ptr<Primary> other) const = 0;
  virtual std::shared_ptr<Primary> operator!=(const std::shared_ptr<Primary> other) const = 0;
  virtual std::shared_ptr<Primary> operator<(const std::shared_ptr<Primary> other) const  = 0;
  virtual std::shared_ptr<Primary> operator<=(const std::shared_ptr<Primary> other) const = 0;
  virtual std::shared_ptr<Primary> operator>(const std::shared_ptr<Primary> other) const  = 0;
  virtual std::shared_ptr<Primary> operator>=(const std::shared_ptr<Primary> other) const = 0;

  // Arithmetic operators
  virtual std::shared_ptr<Primary>
  operator+(const std::shared_ptr<Primary> other) const = 0; // Addition operator
  virtual std::shared_ptr<Primary>
  operator-(const std::shared_ptr<Primary> other) const = 0; // Subtraction operator
  virtual std::shared_ptr<Primary>
  operator*(const std::shared_ptr<Primary> other) const = 0; // Multiplication operator
  virtual std::shared_ptr<Primary>
  operator/(const std::shared_ptr<Primary> other) const = 0; // Division operator

  // Unary arithmetic operators
  virtual std::shared_ptr<Primary> operator+() const = 0; // Unary plus operator
  virtual std::shared_ptr<Primary> operator-() const = 0; // Unary minus operator

  virtual std::shared_ptr<Primary> Evaluate() const = 0;
};

// Value class definition
class Value : public Primary {
public:
  virtual operator bool() const        = 0;
  virtual operator float() const       = 0;
  virtual operator std::string() const = 0;
  virtual operator int() const         = 0;
  virtual ~Value() {}
};

// Literal value class definitions
class BoolLiteral : public Value {
public:
  BoolLiteral(bool value) : value_(value) {}

  operator bool() const override { return value_; }

  // Boolean operators
  std::shared_ptr<Primary> operator!() const override {
    return std::make_shared<Primary>(!value_);
  } // Logical NOT operator
  std::shared_ptr<Primary> operator&&(const std::shared_ptr<Primary> other) const override {
    bool other_value = *std::static_pointer_cast<bool>(other);
    return std::make_shared<Primary>(value_ && other_value);
  } // Logical AND operator
  std::shared_ptr<Primary> operator||(const std::shared_ptr<Primary> other) const override {
    bool other_value = *std::static_pointer_cast<bool>(other);
    return std::make_shared<Primary>(value_ || other_value);
  } // Logical OR operator

  // Comparison operators
  std::shared_ptr<Primary> operator==(const std::shared_ptr<Primary> other) const override {
    bool other_value = *std::static_pointer_cast<bool>(other);
    return std::make_shared<Primary>(value_ == other_value);
  }
  std::shared_ptr<Primary> operator!=(const std::shared_ptr<Primary> other) const override {
    bool other_value = *std::static_pointer_cast<bool>(other);
    return std::make_shared<Primary>(value_ != other_value);
  }
  std::shared_ptr<Primary> operator<(const std::shared_ptr<Primary> other) const override {
    bool other_value = *std::static_pointer_cast<bool>(other);
    return std::make_shared<Primary>(value_ < other_value);
  }
  std::shared_ptr<Primary> operator<=(const std::shared_ptr<Primary> other) const override {
    bool other_value = *std::static_pointer_cast<bool>(other);
    return std::make_shared<Primary>(value_ <= other_value);
  }
  std::shared_ptr<Primary> operator>(const std::shared_ptr<Primary> other) const override {
    bool other_value = *std::static_pointer_cast<bool>(other);
    return std::make_shared<Primary>(value_ > other_value);
  }
  std::shared_ptr<Primary> operator>=(const std::shared_ptr<Primary> other) const override {
    bool other_value = *std::static_pointer_cast<bool>(other);
    return std::make_shared<Primary>(value_ >= other_value);
  }

private:
  bool value_;
};

class FloatLiteral : public Value {
public:
  FloatLiteral(float value) : value_(value) {}

  operator float() const override { return value_; }

  // Unary arithmetic operators
  std::shared_ptr<Primary> operator+() const override {
    return std::make_shared<Primary>(+value_);
  } // Unary plus operator
  std::shared_ptr<Primary> operator-() const override {
    return std::make_shared<Primary>(-value_);
  } // Unary minus operator

  // Arithmetic operators
  std::shared_ptr<Primary> operator+(const std::shared_ptr<Primary> other) const override {
    return std::make_shared<Primary>(value_ + *std::static_pointer_cast<float>(other));
  } // Addition operator
  std::shared_ptr<Primary> operator-(const std::shared_ptr<Primary> other) const override {
    return std::make_shared<Primary>(value_ - *std::static_pointer_cast<float>(other));
  } // Subtraction operator
  std::shared_ptr<Primary> operator*(const std::shared_ptr<Primary> other) const override {
    return std::make_shared<Primary>(value_ * *std::static_pointer_cast<float>(other));
  } // Multiplication operator
  std::shared_ptr<Primary> operator/(const std::shared_ptr<Primary> other) const override {
    return std::make_shared<Primary>(value_ / *std::static_pointer_cast<float>(other));
  } // Division operator

private:
  float value_;
};

class IntLiteral : public Value {
public:
  IntLiteral(int value) : value_(value) {}

  operator int() const override { return value_; }

  // Unary arithmetic operators
  std::shared_ptr<Primary> operator+() const override {
    return std::make_shared<Primary>(+value_);
  } // Unary plus operator
  std::shared_ptr<Primary> operator-() const override {
    return std::make_shared<Primary>(-value_);
  } // Unary minus operator

  // Arithmetic operators
  std::shared_ptr<Primary> operator+(const std::shared_ptr<Primary> other) const override {
    return std::make_shared<Primary>(value_ + *std::static_pointer_cast<int>(other));
  } // Addition operator
  std::shared_ptr<Primary> operator-(const std::shared_ptr<Primary> other) const override {
    return std::make_shared<Primary>(value_ - *std::static_pointer_cast<int>(other));
  } // Subtraction operator
  std::shared_ptr<Primary> operator*(const std::shared_ptr<Primary> other) const override {
    return std::make_shared<Primary>(value_ * *std::static_pointer_cast<int>(other));
  } // Multiplication operator
  std::shared_ptr<Primary> operator/(const std::shared_ptr<Primary> other) const override {
    return std::make_shared<Primary>(value_ / *std::static_pointer_cast<int>(other));
  } // Division operator

private:
  int value_;
};

class StringLiteral : public Value {
public:
  StringLiteral(std::string value) : value_(value) {}

  operator std::string() const override { return value_; }

private:
  std::string value_;
};

// Field initialization class definition
class FieldInitialization {
public:
  FieldInitialization() {}
  void AddField(std::string field_identifier, std::unique_ptr<Expression> expression) {
    fields_.push_back(make_pair(field_identifier, std::move(expression)));
  }

private:
  std::vector<std::pair<std::string, std::unique_ptr<Expression>>> fields_;
};

// Constructed value class definition
class ConstructedValue : public Value {
public:
  ConstructedValue(
      std::string constructor_identifier, std::unique_ptr<FieldInitialization> field_initialization)
      : constructor_identifier_(constructor_identifier),
        field_initialization_(std::move(field_initialization)) {}

private:
  std::string constructor_identifier_;
  std::unique_ptr<FieldInitialization> field_initialization_;
};

// Field access class definition
class FieldAccess : public Primary {
public:
  FieldAccess(std::string &var_identifier, std::string &field_identifier)
      : field_access_(nullptr), var_identifier_(var_identifier),
        field_identifier_(field_identifier) {}

  FieldAccess(std::unique_ptr<FieldAccess> field_access, std::string &field_identifier)
      : field_access_(std::move(field_access)), var_identifier_(""),
        field_identifier_(field_identifier) {}

private:
  std::unique_ptr<FieldAccess> field_access_;
  std::string var_identifier_;
  std::string field_identifier_;
};
