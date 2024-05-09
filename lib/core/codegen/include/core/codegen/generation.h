#pragma once

#include "core/ast/ast.h"

#include <fstream>

namespace dbuf::gen {

class ITargetCodeGenerator {
public:
  virtual void Generate(const ast::AST *tree) = 0;

  virtual ~ITargetCodeGenerator() = default;

protected:
  explicit ITargetCodeGenerator(const std::string &out_file);

  std::shared_ptr<std::ofstream> output_;
};

class ListGenerators {
public:
  void Fill(std::vector<std::string> &formats, const std::string &path, const std::string &filename);

  void Process(ast::AST *tree);

private:
  std::vector<std::shared_ptr<ITargetCodeGenerator>> targets_;
};
} // namespace dbuf::gen