#pragma once

#include "core/ast/ast.h"

#include <fstream>

namespace dbuf::gen {

class ITargetCodeGenerator {
public:
  explicit ITargetCodeGenerator(const std::string &out_filename);

  virtual void Generate(ast::AST *tree) const = 0;

protected:
  std::ofstream output_;
};

class CppCodeGenerator : public ITargetCodeGenerator {
public:
  explicit CppCodeGenerator(const std::string &out_file)
      : ITargetCodeGenerator(out_file) {}

  void Generate(ast::AST *tree) const override;
};
class ListGenerators {
public:
  void Fill(std::vector<std::string> &formats, const std::string &path, const std::string &filename);

  void Process(ast::AST *tree) const;

private:
  std::vector<std::unique_ptr<ITargetCodeGenerator>> targets_;
};
} // namespace dbuf::gen