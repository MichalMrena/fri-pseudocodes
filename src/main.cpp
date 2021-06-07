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

    // Create output stream. File if 2nd arg is provided, std::cout otherwise.
    auto ofstOpt = std::optional<std::ofstream>();
    if (argc > 2)
    {
        auto ofst = std::ofstream(argv[2]);
        if (not ofst.is_open())
        {
            std::cerr << "Failed to open output file: " << argv[2] << '\n';
            return 1;
        }
        ofstOpt = std::move(ofst);
    }

    auto colors = fri::CodeColorInfo { .function_   = fri::Color {255, 255, 0  }
                                     , .variable_   = fri::Color {0,   255, 255}
                                     , .keyword_    = fri::Color {0,   0,   255}
                                     , .plain_      = fri::Color {255, 255, 255}
                                     , .customType_ = fri::Color {0,   255, 0  }
                                     , .primType_   = fri::Color {0,   0,   255}
                                     , .string_     = fri::Color {255, 0,   0  } };

    // Analyze code and generate pseudocode.
    auto& ost               = static_cast<std::ostream&>(ofstOpt ? ofstOpt.value() : std::cout);
    auto printer            = fri::ConsoleCodePrinter();
    auto generator          = fri::PseudocodeGenerator(printer, colors);
    auto const abstractCode = fri::extract_code(code);
    auto const& classes     = abstractCode.get_classes();

    std::cout << "---------------------------------------------" << '\n';
    for (auto const& c : classes)
    {
        c->accept(generator);
    }

    // std::cout << "\n";
    // std::cout << "\x1B[90m Text"       << '\n'; // black
    // std::cout << "\x1B[91m Text"       << '\n'; // red
    // std::cout << "\x1B[92m Text"       << '\n'; // green
    // std::cout << "\x1B[93m Text"       << '\n'; // yellow
    // std::cout << "\x1B[94m Text"       << '\n'; // blue
    // std::cout << "\x1B[95m Text"       << '\n'; // purple
    // std::cout << "\x1B[96m Text"       << '\n'; // cyan
    // std::cout << "\x1B[97m Text"       << '\n'; // white
    // std::cout << '\n';
    // std::cout << "\x1B[3;42;30m Text  \x1B[0m"  << '\n';
    // std::cout << "\x1B[3;43;30m Text  \x1B[0m"  << '\n';
    // std::cout << "\x1B[3;44;30m Text  \x1B[0m"  << '\n';
    // std::cout << "\x1B[3;104;30m Text \x1B[0m" << '\n';
    // std::cout << "\x1B[3;100;30m Text \x1B[0m" << '\n';
    // std::cout << "\x1B[3;47;35m Text  \x1B[0m"  << '\n';
    // std::cout << "\x1B[2;47;35m Text  \x1B[0m"  << '\n';
    // std::cout << "\x1B[1;47;35m Text  \x1B[0m"  << '\n';

    // \x1B[0m reset all
}