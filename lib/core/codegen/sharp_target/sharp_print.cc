#include "core/codegen/sharp_target/sharp_print.h"

namespace dbuf::gen {

SharpPrinter::SharpPrinter(std::shared_ptr<std::ofstream> output) {
    out_ = std::move(output);
}

} // namespace dbuf::gen