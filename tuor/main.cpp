#include <libtuor/code_generator.hpp>
#include <libtuor/code_output.hpp>
#include <libtuor/utils.hpp>
#include <array>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <map>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace po = boost::program_options;
namespace pt = boost::property_tree;

using fri::int32;

auto fail (
    po::options_description const& options,
    std::string_view msg = ""
) -> void
{
    if (not empty(msg))
    {
        std::cerr << msg << "\n---\n";
    }
    std::cerr << options;
    std::exit(1);
}

auto make_options_description () -> po::options_description
{
    auto desc = po::options_description("You can use the following options");
    desc.add_options()
    (
        "help,h",
        "Show this message"
    )
    (
        "input,i",
        po::value<std::string>()->required(),
        "Input file"
    )
    (
        "output-type,t",
        po::value<std::string>()->required(),
        "Output type {console|rtf|latex}"
    )
    (
        "output,o",
        po::value<std::string>()->default_value("."),
        "Output directory"
    )
    (
        "style,s",
        po::value<std::string>(),
        "Style settings"
    );
    return desc;
}

auto parse_options (
    po::options_description const& desc,
    int argc,
    char** argv
) -> po::variables_map
{
    auto vm = po::variables_map();
    try
    {
        po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
        po::notify(vm);
    }
    catch (std::exception const& e)
    {
        fail(desc, e.what());
    }
    return vm;
}

auto parse_style_file (std::string const& path) -> std::optional<fri::CodeStyle>
{
    try
    {
        auto style = fri::CodeStyle();
        auto pt = pt::ptree();
        pt::ini_parser::read_ini(path, pt);
        auto const indentNodeOpt = pt.get_child_optional("Indent");
        auto const colorsNodeOpt = pt.get_child_optional("Colors");
        auto const styleNodeOpt  = pt.get_child_optional("Style");

        if (indentNodeOpt)
        {
            auto const indentValudeNodeOpt
                = (*indentNodeOpt).get_child_optional("spaceCount");
            if (indentValudeNodeOpt)
            {
                auto const indentValueOpt
                    = (*indentValudeNodeOpt).get_value_optional<int32>();
                if (indentValueOpt)
                {
                    style.set_indent_size(*indentValueOpt);
                }
            }
        }

        auto tokenStyles = std::map<fri::TokenType, fri::TokenStyle>();

        if (colorsNodeOpt)
        {
            for (auto const& [key, tree] : *colorsNodeOpt)
            {
                auto const tokenTypeOpt = fri::str_to_token_type(
                    fri::to_lowercase(key)
                );
                if (not tokenTypeOpt)
                {
                    std::cerr << "Uknown token type: " << key << "."
                              << "Using default style." << "\n";
                    continue;
                }
                auto const colorStr = tree.get_value<std::string>();
                auto const rgbStrs = fri::to_words(colorStr);
                if (ssize(rgbStrs) != 3)
                {
                    std::cerr << "Invalid color string: " << colorStr << "."
                              << "Using default style." << "\n";
                    continue;
                }
                auto const rgbInts = std::array<std::optional<int32>, 3>{
                    fri::parse<int32>(rgbStrs[0]),
                    fri::parse<int32>(rgbStrs[1]),
                    fri::parse<int32>(rgbStrs[2])
                };

                if (not (rgbInts[0] && rgbInts[1] && rgbInts[2]))
                {
                    std::cerr << "Invalid color string: " << colorStr << "."
                              << "Using default style." << "\n";
                    continue;
                }

                auto const colorOpt = fri::make_color(
                    *rgbInts[0],
                    *rgbInts[1],
                    *rgbInts[2]
                );

                if (not colorOpt)
                {
                    std::cerr << "Invalid color values: " << colorStr << "."
                              << "Using default style." << "\n";
                    continue;
                }

                tokenStyles[*tokenTypeOpt].color_ = *colorOpt;
            }
        }

        if (styleNodeOpt)
        {
            for (auto const& [key, tree] : *styleNodeOpt)
            {
                auto const tokenTypeOpt = fri::str_to_token_type(
                    fri::to_lowercase(key)
                );
                if (not tokenTypeOpt)
                {
                    std::cerr << "Uknown token type: " << key << "."
                              << "Using default style." << "\n";
                    continue;
                }

                auto const fontStyleStr = tree.get_value<std::string>();
                auto const fontStyleOpt = fri::str_to_font_style(
                    fri::to_lowercase(fontStyleStr)
                );

                if (not fontStyleOpt)
                {
                    std::cerr << "Uknown font style: " << fontStyleStr << "."
                              << "Using default style." << "\n";
                    continue;
                }

                tokenStyles[*tokenTypeOpt].style_ = *fontStyleOpt;
            }

        }

        for (auto const& [tokenType, tokenStyle] : tokenStyles)
        {
            style.set(tokenType, tokenStyle);
        }

        return style;
    }
    catch (pt::ini_parser_error const&)
    {
        return std::nullopt;
    }
}

auto main (int argc, char** argv) -> int
{
    auto const desc = make_options_description();
    auto const options = parse_options(desc, argc, argv);

    // Help
    if (options.count("help") > 0)
    {
        std::cout << desc << "\n";
        return 0;
    }

    // Required options
    if (options.count("input") == 0 || options.count("output-type") == 0)
    {
        fail(desc);
    }

    // Output type
    auto const outputTypeOpt = fri::str_to_output_type(
        options.at("output-type").as<std::string>()
    );
    if (not outputTypeOpt)
    {
        fail(desc);
    }

    // Input file
    auto const inputFile = options.at("input").as<std::string>();
    if (not std::filesystem::is_regular_file(inputFile))
    {
        fail(desc, "Input file is not a regular file.");
    }

    // Output directory
    auto const outputDirIt = options.find("output");
    if (outputDirIt != end(options))
    {
        auto const outputDir = outputDirIt->second.as<std::string>();
        if (not std::filesystem::is_directory(outputDir))
        {
            fail(desc, "Output destination is not a directory.");
        }
    }
    else if (outputTypeOpt != fri::OutputType::Console)
    {
        fail(desc, "No output directory provided for non-console format.");
    }

    // Style file
    auto const styleFileIt = options.find("style");
    auto style = fri::CodeStyle();
    if (styleFileIt != end(options))
    {
        auto const styleFile = styleFileIt->second.as<std::string>();
        if (not std::filesystem::is_regular_file(styleFile))
        {
            fail(desc, "Style file is not a regular file.");
        }

        auto const styleOpt = parse_style_file(styleFile);
        if (styleOpt)
        {
            style = *styleOpt;
        }
    }

    auto outputter = fri::make_code_outputter(*outputTypeOpt);
    // auto generator = fri::PseudocodeGenerator(outputter);

    // finally do the parsing
}