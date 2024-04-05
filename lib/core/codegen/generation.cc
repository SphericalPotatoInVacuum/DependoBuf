#include "core/codegen/generation.h"

namespace dbuf::gen {
ITargetCodeGenerator::ITargetCodeGenerator(const std::string &out_filename) {
  target.open(out_filename, std::ios::out | std::ios::trunc);
  if (!target.is_open()) {
    throw "Cannot open file " + out_filename;
  }
}

void ListGenerators::Fill(std::vector<const std::string> &files) {
  targets.reserve(files.size());
  for (const std::string &filename : files) {
    if (filename.ends_with(".cpp")) {
      targets.emplace_back(std::make_unique<CppCodeGenerator>(std::move(CppCodeGenerator(filename))));
    } else {
      throw "Not supported language format in file " + filename;
    }
  }
}

void ListGenerators::Process(ast::AST *tree) const {
  for (auto &target : targets) {
    target->Generate(tree);
  }
}
} // namespace dbuf::gen