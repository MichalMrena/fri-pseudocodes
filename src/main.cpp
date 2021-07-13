#include "clang_source_parser.hpp"
#include "code_generator.hpp"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <optional>
#include <utility>

enum class OutputMode
{
    Console, File
};

int main(int argc, char **argv)
{
    if (argc < 1)
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
    auto const outputMode = argc > 1 ? OutputMode::File : OutputMode::Console;
    auto ofstOpt = outputMode == OutputMode::Console ? std::nullopt :
    ([=]()
    {
        auto ofst = std::ofstream(argv[2]);
        return ofst.is_open() ? std::optional<std::ofstream>(std::move(ofst))
                              : std::nullopt;
    }());

    // Check if the output file is writiable.
    if (outputMode == OutputMode::File and not ofstOpt.has_value())
    {
        std::cerr << "Failed to open output file: " << argv[2] << '\n';
        return 1;
    }

    // Read the code from the input file.
    auto ist = std::stringstream();
    ist << ifst.rdbuf();
    auto const code = ist.str();

    // auto colors = fri::CodeColorInfo { .function_   = fri::Color {255, 255, 0  }
    //                                  , .variable_   = fri::Color {0,   255, 255}
    //                                  , .keyword_    = fri::Color {0,   0,   255}
    //                                  , .plain_      = fri::Color {255, 255, 255}
    //                                  , .customType_ = fri::Color {0,   255, 0  }
    //                                  , .primType_   = fri::Color {0,   0,   255}
    //                                  , .string_     = fri::Color {255, 0,   0  }
    //                                  , .valLiteral_ = fri::Color {255, 0,   255} };

    auto colors = fri::CodeColorInfo { .function_   = fri::Color {191, 144, 0  }
                                     , .variable_   = fri::Color {0,   112, 192}
                                     , .keyword_    = fri::Color {0,   32,  96 }
                                     , .plain_      = fri::Color {0,   0,   0  }
                                     , .customType_ = fri::Color {0,   176, 80 }
                                     , .primType_   = fri::Color {0,   32,  96 }
                                     , .string_     = fri::Color {197, 90,  17 }
                                     , .valLiteral_ = fri::Color {112, 48,  160} };

    // Analyze the code and generate pseudocode.
    using printer_var = std::variant<fri::ConsoleCodePrinter, fri::RtfCodePrinter>;
    auto printerVar   = outputMode == OutputMode::File ? printer_var(std::in_place_type_t<fri::RtfCodePrinter>(), ofstOpt.value())
                                                       : printer_var(std::in_place_type_t<fri::ConsoleCodePrinter>());

    auto& p                 = outputMode == OutputMode::File ? static_cast<fri::ICodePrinter&>(std::get<1>(printerVar)) : static_cast<fri::ICodePrinter&>(std::get<0>(printerVar));
    auto generator          = fri::PseudocodeGenerator(p, colors);
    auto const abstractCode = fri::extract_code(code);
    auto const& classes     = abstractCode.get_classes();

    std::cout << "---------------------------------------------" << '\n';
    for (auto const& c : classes)
    {
        c->accept(generator);
    }

    std::cout << "á" << "(" << std::string("á").size() << "): ";
    for (auto const c : std::string("á"))
    {
        std::cout << static_cast<unsigned>(static_cast<unsigned char>(c)) << ' ';
    }
    std::cout << "\n";
}