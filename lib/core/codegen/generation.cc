#include "core/codegen/generation.h"

#include <set>

namespace dbuf::gen {
ITargetCodeGenerator::ITargetCodeGenerator(const std::string &out_filename) {
  output_.open(out_filename, std::ios::out | std::ios::trunc);
  if (!output_.is_open()) {
    throw "Cannot open file " + out_filename;
  }
}

void ListGenerators::Fill(std::vector<std::string> &files) {
  std::set<std::string> added_formats;
  targets_.reserve(files.size());
  for (std::string &filename : files) {
    if ((filename.ends_with(".cpp")) && (!added_formats.contains(".cpp"))) {
      targets_.emplace_back(std::make_unique<CppCodeGenerator>(std::move(CppCodeGenerator(filename))));
      added_formats.insert(".cpp");
    } else {
      throw "Not supported language format in file " + filename;
    }
  }
}

void ListGenerators::Process(ast::AST *tree) const {
  for (const auto &target : targets_) {
    target->Generate(tree);
  }
}
} // namespace dbuf::gen