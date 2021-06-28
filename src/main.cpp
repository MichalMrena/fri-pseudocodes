#include "clang_source_parser.hpp"
#include "code_generator.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <optional>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Input file path not provided." << '\n';
        return 1;
    }

    // Check input and read code from it.
    auto ifst = std::ifstream(argv[1]);
    if (not ifst.is_open())
    {
        std::cerr << "Failed to open input file: " << argv[1] << '\n';
        return 1;
    }
    auto ist = std::stringstream();
    ist << ifst.rdbuf();
    auto const code = ist.str();

    auto colors = fri::CodeColorInfo { .function_   = fri::Color {255, 255, 0  }
                                     , .variable_   = fri::Color {0,   255, 255}
                                     , .keyword_    = fri::Color {0,   0,   255}
                                     , .plain_      = fri::Color {255, 255, 255}
                                     , .customType_ = fri::Color {0,   255, 0  }
                                     , .primType_   = fri::Color {0,   0,   255}
                                     , .string_     = fri::Color {255, 0,   0  }
                                     , .valLiteral_ = fri::Color {255, 0,   255} };

    // Analyze code and generate pseudocode.
    // auto& ost               = static_cast<std::ostream&>(ofstOpt ? ofstOpt.value() : std::cout);
    auto printer            = fri::ConsoleCodePrinter();
    auto generator          = fri::PseudocodeGenerator(printer, colors);
    auto const abstractCode = fri::extract_code(code);
    auto const& classes     = abstractCode.get_classes();

    std::cout << "---------------------------------------------" << '\n';
    for (auto const& c : classes)
    {
        c->accept(generator);
    }
}