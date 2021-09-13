#include "clang_source_parser.hpp"
#include "code_generator.hpp"
#include "utils.hpp"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <optional>
#include <utility>
#include <cassert>
#include <unordered_map>
#include <cstring>

namespace
{
    enum class OutputMode
    {
        Console, File
    };

    auto try_load_setting(OutputMode const outputMode)
    {
        auto ifst = std::ifstream("/home/michal/Projects/fri-pseudocodes/input/settings.txt");
        if (not ifst.is_open())
        {
            std::cout << "Error: " << std::strerror(errno) << '\n';
            return fri::OutputSettings {};
        }

        auto const print_ignore = [](auto const& settingName)
        {
            std::cout << "Ignoring setting line: " << settingName << '\n';
        };

        auto settings = fri::OutputSettings();
        auto colorMap = std::unordered_map<std::string, fri::Color>();
        auto line = std::string();
        while (std::getline(ifst, line))
        {
            auto words = fri::to_words(std::move(line));
            auto const& settingName = words[0];
            if (settingName == "fontSize")
            {
                if (words.size() < 2)
                {
                    print_ignore(settingName);
                    continue;
                }
                auto const val = fri::parse<unsigned int>(words[1]);
                if (not val)
                {
                    print_ignore(settingName);
                    continue;
                }
                settings.fontSize = val;
            }
            else if (settingName == "indent")
            {
                if (words.size() < 2)
                {
                    print_ignore(settingName);
                    continue;
                }
                auto const val = fri::parse<unsigned int>(words[1]);
                if (not val)
                {
                    print_ignore(settingName);
                    continue;
                }
                settings.indentSpaces = val;
            }
            else if (settingName == "colors")
            {
                if (not std::getline(ifst, line))
                {
                    print_ignore("colors");
                    break;
                }

                auto colorWords = fri::to_words(std::move(line));
                while (colorWords[0] != "end")
                {
                    auto& colorTarget = colorWords[0];
                    if (colorWords.size() < 4)
                    {
                        print_ignore(colorTarget);
                        continue;
                    }
                    auto const r = fri::parse<std::uint8_t>(colorWords[1]);
                    auto const g = fri::parse<std::uint8_t>(colorWords[2]);
                    auto const b = fri::parse<std::uint8_t>(colorWords[3]);
                    if (r and g and b)
                    {
                        colorMap.emplace(colorTarget, fri::Color {r, g, b});
                    }
                    else
                    {
                        print_ignore(colorTarget);
                    }
                    if (not std::getline(ifst, line))
                    {
                        break;
                    }
                    colorWords = fri::to_words(std::move(line));
                }

                auto const color_or_default = [](auto const& cMap, auto const& name)
                {
                    auto const it = cMap.find(name);
                    return it == std::end(cMap) ? fri::Color {0, 0, 0} : it->second;
                };

                // Temporary exception for console.
                if (outputMode == OutputMode::Console)
                {
                    settings.colors =
                        fri::CodeColorInfo
                            { .function_   = fri::Color {255, 255, 0  }
                            , .variable_   = fri::Color {0,   255, 255}
                            , .keyword_    = fri::Color {0,   0,   255}
                            , .plain_      = fri::Color {255, 255, 255}
                            , .customType_ = fri::Color {0,   255, 0  }
                            , .primType_   = fri::Color {0,   0,   255}
                            , .string_     = fri::Color {255, 0,   0  }
                            , .valLiteral_ = fri::Color {255, 0,   255} };
                }
                else
                {
                    settings.colors = fri::CodeColorInfo
                        { .function_   = color_or_default(colorMap, "function")
                        , .variable_   = color_or_default(colorMap, "variable")
                        , .keyword_    = color_or_default(colorMap, "keyword")
                        , .plain_      = color_or_default(colorMap, "plain")
                        , .customType_ = color_or_default(colorMap, "customType")
                        , .primType_   = color_or_default(colorMap, "primType")
                        , .string_     = color_or_default(colorMap, "string")
                        , .valLiteral_ = color_or_default(colorMap, "valLiteral") };
                }

            }
            else
            {
                print_ignore(settingName);
            }
        }

        return settings;
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

    auto printer( OutputMode const m
                , std::optional<std::ofstream>& osfOpt
                , fri::OutputSettings const& settings ) -> printer_variant_t
    {
        switch (m)
        {
        case OutputMode::File:
            assert(osfOpt.has_value());
            return printer_variant_t(std::in_place_type_t<fri::RtfCodePrinter>(), osfOpt.value(), settings);

        default:
            return printer_variant_t(std::in_place_type_t<fri::ConsoleCodePrinter>(), settings);
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

    // Possibly read settings or use defaults.
    auto settings = try_load_setting(outputMode);

    // Read the code from the input file.
    auto ist = std::stringstream();
    ist << ifst.rdbuf();
    auto const code = ist.str();

    // Analyze the code and generate pseudocode.
    auto printerVar         = printer(outputMode, ofstOpt, settings);
    auto generator          = fri::PseudocodeGenerator(printer_ref(printerVar), settings.colors);
    auto const abstractCode = fri::extract_code(code);

    std::cout << "---------------------------------------------" << '\n';
    for (auto const& c : abstractCode.get_classes())
    {
        c->accept(generator);
    }
}