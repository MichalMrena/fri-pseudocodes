#include "cxx_source_parser.hpp"
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

    // Analyze code and generate pseudocode.
    auto& ost               = static_cast<std::ostream&>(ofstOpt ? ofstOpt.value() : std::cout);
    auto printer            = fri::PseudocodePrinter(ost);
    auto const abstractCode = fri::extract_code(code);
    auto const& classes     = abstractCode.get_classes();

    for (auto const& c : classes)
    {
        c.accept(printer);
    }
}