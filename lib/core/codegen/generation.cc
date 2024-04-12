#include "core/codegen/generation.h"

#include <filesystem>
#include <set>
#include <sstream>

namespace dbuf::gen {
ITargetCodeGenerator::ITargetCodeGenerator(const std::string &out_file) {
  output_ = std::make_shared<std::ofstream>(std::ofstream(out_file));
  if (!output_->is_open()) {
    throw std::string("Cannot open file in the given path");
  }
}

void ListGenerators::Fill(std::vector<std::string> &formats, const std::string &path, const std::string &filename) {
  if (!std::filesystem::is_directory(path)) {
    throw "Incorrect path: {}" + path;
  }

  std::set<std::string> added_formats;
  targets_.reserve(formats.size());

  for (std::string &format : formats) {
    if ((format == "cpp") || (format == "c++")) {
      if (!added_formats.contains("cpp")) {
        std::stringstream full_path;
        full_path << path << "/" << filename << ".h";
        targets_.emplace_back(std::make_shared<CppCodeGenerator>(CppCodeGenerator(full_path.str())));
        added_formats.insert("cpp");
      } else {
        throw std::string("You can add only one c++ file");
      }
    } else {
      throw "Unsupported foramt: {}" + format;
    }
  }
}

void ListGenerators::Process(ast::AST *tree) {
  for (const auto &target : targets_) {
    target->Generate(tree);
  }
}
} // namespace dbuf::gen