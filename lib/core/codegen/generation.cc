#include "core/codegen/generation.h"

#include <filesystem>
#include <set>

namespace dbuf::gen {
ITargetCodeGenerator::ITargetCodeGenerator(const std::string &out_filename) {
  output_.open(out_filename, std::ios::out | std::ios::trunc);
  if (!output_.good()) {
    throw "Cannot create file in the given path";
  }
}

void ListGenerators::Fill(std::vector<std::string> &formats, const std::string &path, const std::string &filename) {
  if (!std::filesystem::is_directory(path)) {
    throw "Incorrect path: " + path;
  }

  std::set<std::string> added_formats;
  targets_.reserve(formats.size());

  for (std::string &format : formats) {
    if ((format == "cpp") || (format == "c++")) {
      if (!added_formats.contains("cpp")) {
        std::string full_path = path + "/";
        full_path += filename + ".h";
        targets_.emplace_back(std::make_unique<CppCodeGenerator>(std::move(CppCodeGenerator(full_path))));
        added_formats.insert("cpp");
      } else {
        throw "You can add only one c++ file";
      }
    } else {
      throw "Unsupported foramt: " + format;
    }
  }
}

void ListGenerators::Process(ast::AST *tree) const {
  for (const auto &target : targets_) {
    target->Generate(tree);
  }
}
} // namespace dbuf::gen