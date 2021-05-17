#include "cxx_source_parser.hpp"
#include "code_generator.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "File paths not provided." << '\n';
        return 1;
    }

    auto ifst = std::ifstream(argv[1]);
    if (not ifst.is_open())
    {
        std::cerr << "Failed to open input file: " << argv[1] << '\n';
        return 1;
    }

    auto ofst = std::ofstream(argv[2]);
    if (not ofst.is_open())
    {
        std::cerr << "Failed to open output file: " << argv[2] << '\n';
        return 1;
    }

    auto ist = std::stringstream();
    ist << ifst.rdbuf();
    auto const code = ist.str();

    auto printer            = fri::PseudocodePrinter(ofst);
    auto const abstractCode = fri::extract_code(code);
    auto const& classes     = abstractCode.get_classes();

    for (auto const& c : classes)
    {
        c.accept(printer);
    }
}