#include "clang_source_parser.hpp"
#include "code_generator.hpp"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <optional>
#include <utility>
#include <cassert>

namespace
{
    enum class OutputMode
    {
        Console, File
    };

    auto color_profile(OutputMode const m)
    {
        switch (m)
        {
        case OutputMode::File:
            return fri::CodeColorInfo { .function_   = fri::Color {191, 144, 0  }
                                      , .variable_   = fri::Color {0,   112, 192}
                                      , .keyword_    = fri::Color {0,   32,  96 }
                                      , .plain_      = fri::Color {0,   0,   0  }
                                      , .customType_ = fri::Color {0,   176, 80 }
                                      , .primType_   = fri::Color {0,   32,  96 }
                                      , .string_     = fri::Color {197, 90,  17 }
                                      , .valLiteral_ = fri::Color {112, 48,  160} };

        case OutputMode::Console:
            return fri::CodeColorInfo { .function_   = fri::Color {255, 255, 0  }
                                      , .variable_   = fri::Color {0,   255, 255}
                                      , .keyword_    = fri::Color {0,   0,   255}
                                      , .plain_      = fri::Color {255, 255, 255}
                                      , .customType_ = fri::Color {0,   255, 0  }
                                      , .primType_   = fri::Color {0,   0,   255}
                                      , .string_     = fri::Color {255, 0,   0  }
                                      , .valLiteral_ = fri::Color {255, 0,   255} };

        default:
            return fri::CodeColorInfo {};
        }
    }

    auto output_file(OutputMode const m, char** argv)
    {
        switch (m)
        {
        case OutputMode::File:
            return std::optional<std::ofstream>(std::in_place_t(), argv[2]);

        default:
            return std::optional<std::ofstream>(std::nullopt);
        }
    }

    using printer_variant_t = std::variant<fri::ConsoleCodePrinter, fri::RtfCodePrinter>;

    auto printer(OutputMode const m, std::optional<std::ofstream>& osfOpt) -> printer_variant_t
    {
        switch (m)
        {
        case OutputMode::File:
            assert(osfOpt.has_value());
            return printer_variant_t(std::in_place_type_t<fri::RtfCodePrinter>(), osfOpt.value());

        default:
            return printer_variant_t(std::in_place_type_t<fri::ConsoleCodePrinter>());
        }
    }

    auto printer_ref(printer_variant_t& v) -> fri::ICodePrinter&
    {
        return std::holds_alternative<fri::RtfCodePrinter>(v)
                   ? static_cast<fri::ICodePrinter&>(std::get<fri::RtfCodePrinter>(v))
                   : static_cast<fri::ICodePrinter&>(std::get<fri::ConsoleCodePrinter>(v));
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Input file path not provided." << '\n';
        return 1;
    }

    // Check if the input file is readable.
    auto ifst = std::ifstream(argv[1]);
    if (not ifst.is_open())
    {
        std::cerr << "Failed to open input file: " << argv[1] << '\n';
        return 1;
    }

    // If the output path is provided, try to initialize the output stream.
    auto const outputMode = argc > 2 ? OutputMode::File : OutputMode::Console;
    auto ofstOpt = output_file(outputMode, argv);

    // Check if the output file is set and writable.
    if (ofstOpt.has_value() and not ofstOpt.value().is_open())
    {
        std::cerr << "Failed to open output file: " << argv[2] << '\n';
        return 1;
    }

    // Read the code from the input file.
    auto ist = std::stringstream();
    ist << ifst.rdbuf();
    auto const code = ist.str();

    // Analyze the code and generate pseudocode.
    auto printerVar         = printer(outputMode, ofstOpt);
    auto generator          = fri::PseudocodeGenerator(printer_ref(printerVar), color_profile(outputMode));
    auto const abstractCode = fri::extract_code(code);

    std::cout << "---------------------------------------------" << '\n';
    for (auto const& c : abstractCode.get_classes())
    {
        c->accept(generator);
    }
}