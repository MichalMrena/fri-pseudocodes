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

    auto string_to_style(std::string_view s)
    {
        return s == "normal" ? fri::FontStyle::Normal :
               s == "bold"   ? fri::FontStyle::Bold   :
               s == "italic" ? fri::FontStyle::Italic :
                               fri::FontStyle::Normal;
    }

    auto console_dummy_settings()
    {
        auto ret = fri::OutputSettings {};
        ret.style = fri::CodeStyleInfo
            { .function_       = fri::TextStyle {fri::Color {255, 255, 0  }, fri::FontStyle::Normal}
            , .variable_       = fri::TextStyle {fri::Color {0,   255, 255}, fri::FontStyle::Normal}
            , .memberVariable_ = fri::TextStyle {fri::Color {0,   255, 255}, fri::FontStyle::Normal}
            , .keyword_        = fri::TextStyle {fri::Color {0,   0,   255}, fri::FontStyle::Normal}
            , .plain_          = fri::TextStyle {fri::Color {255, 255, 255}, fri::FontStyle::Normal}
            , .customType_     = fri::TextStyle {fri::Color {0,   255, 0  }, fri::FontStyle::Normal}
            , .primType_       = fri::TextStyle {fri::Color {0,   0,   255}, fri::FontStyle::Normal}
            , .stringLiteral_  = fri::TextStyle {fri::Color {255, 0,   0  }, fri::FontStyle::Normal}
            , .valLiteral_     = fri::TextStyle {fri::Color {255, 0,   255}, fri::FontStyle::Normal}
            , .numLiteral_     = fri::TextStyle {fri::Color {255, 0,   0  }, fri::FontStyle::Normal} };
        return ret;
    }

    auto try_load_setting(OutputMode const outputMode)
    {
        auto ifst = std::ifstream("settings.txt");
        if (not ifst.is_open())
        {
            std::cerr << "Settings error: " << std::strerror(errno) << '\n';
            return outputMode == OutputMode::Console
                       ? console_dummy_settings()
                       : fri::OutputSettings {};
        }

        auto const print_ignore = [](auto const& settingName)
        {
            std::cout << "Ignoring setting line: " << settingName << '\n';
        };

        auto settings = fri::OutputSettings();
        auto styleMap = std::unordered_map<std::string, fri::TextStyle>();
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
            else if (settingName == "font")
            {
                if (words.size() < 2)
                {
                    print_ignore(settingName);
                    continue;
                }

                auto const fEnd = std::end(words);
                auto fIt = std::begin(words) + 2;
                auto fontName = std::string(std::move(words[1]));
                while (fIt != fEnd)
                {
                    fontName += " ";
                    fontName += std::move(*fIt);
                    ++fIt;
                }
                settings.font = std::move(fontName);
            }
            else if (settingName == "style")
            {
                if (not std::getline(ifst, line))
                {
                    print_ignore("style");
                    break;
                }

                auto styleWords = fri::to_words(std::move(line));
                while (styleWords[0] != "end")
                {
                    auto& styleTarget = styleWords[0];
                    if (styleWords.size() < 5)
                    {
                        print_ignore(styleTarget);
                        continue;
                    }
                    auto const s = string_to_style(styleWords[1]);
                    auto const r = fri::parse<std::uint8_t>(styleWords[2]);
                    auto const g = fri::parse<std::uint8_t>(styleWords[3]);
                    auto const b = fri::parse<std::uint8_t>(styleWords[4]);
                    if (r and g and b)
                    {
                        styleMap.emplace(styleTarget, fri::TextStyle {fri::Color {r, g, b}, s});
                    }
                    else
                    {
                        print_ignore(styleTarget);
                    }
                    if (not std::getline(ifst, line))
                    {
                        break;
                    }
                    styleWords = fri::to_words(std::move(line));
                }

                auto const style_or_default = [](auto const& cMap, auto const& name)
                {
                    auto const it = cMap.find(name);
                    return it == std::end(cMap)
                        ? fri::TextStyle {fri::Color {0, 0, 0}, fri::FontStyle::Normal}
                        : it->second;
                };

                // Temporary exception for console.
                if (outputMode == OutputMode::Console)
                {
                    return console_dummy_settings();
                }
                else
                {
                    settings.style =
                        fri::CodeStyleInfo
                        { .function_       = style_or_default(styleMap, "function")
                        , .variable_       = style_or_default(styleMap, "variable")
                        , .memberVariable_ = style_or_default(styleMap, "memberVariable")
                        , .keyword_        = style_or_default(styleMap, "keyword")
                        , .plain_          = style_or_default(styleMap, "plain")
                        , .customType_     = style_or_default(styleMap, "customType")
                        , .primType_       = style_or_default(styleMap, "primType")
                        , .stringLiteral_  = style_or_default(styleMap, "stringLiteral")
                        , .valLiteral_     = style_or_default(styleMap, "valLiteral")
                        , .numLiteral_     = style_or_default(styleMap, "numLiteral") };
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

        // TODO inicializuj [predka] iba ak je to naozaj predok

    // Possibly read settings or use defaults.
    auto settings = try_load_setting(outputMode);

    // Read the code from the input file.
    auto ist = std::stringstream();
    ist << ifst.rdbuf();
    auto const code = ist.str();

    // Analyze the code and generate pseudocode.
    auto printerVar         = printer(outputMode, ofstOpt, settings);
    auto generator          = fri::PseudocodeGenerator(printer_ref(printerVar), settings.style);
    auto const abstractCode = fri::extract_code(code);

    std::cout << "---------------------------------------------" << '\n';
    for (auto const& c : abstractCode.get_classes())
    {
        c->accept(generator);
    }
}