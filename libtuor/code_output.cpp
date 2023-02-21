#include <algorithm>
#include <array>
#include <bits/ranges_algo.h>
#include <bits/ranges_base.h>
#include <boost/container/static_vector.hpp>
#include <boost/container/flat_map.hpp>
#include <cassert>
#include <cmath>
#include <libtuor/code_output.hpp>
#include <iostream>
#include <ostream>
#include <string_view>
#include <tuple>

namespace fri
{
    namespace bc = boost::container;

    template<class Key, class Value, std::size_t Size>
    using StaticLookupTableType = bc::flat_map<
        Key,
        Value,
        std::less<Key>,
        bc::static_vector<std::pair<Key, Value>, Size>
    >;

    template<class Enum>
    auto str_to_enum (
        std::string_view s,
        std::initializer_list<std::pair<std::string_view, Enum>> mapping
    ) -> std::optional<Enum>
    {
        static auto const table = StaticLookupTableType<
            std::string_view,
            Enum,
            enum_instance_count<Enum>()
        >(mapping);
        auto const it = table.find(s);
        return it != end(table)
            ? std::optional<Enum>(it->second)
            : std::nullopt;
    }

    auto str_to_output_type (std::string_view s) -> std::optional<OutputType>
    {
        return str_to_enum<OutputType>(s,
        {
            {"console", OutputType::Console},
            {"rtf",     OutputType::Rtf},
            {"latex",   OutputType::LaTeX}
        });
    }

    auto str_to_font_style (std::string_view s) -> std::optional<FontStyle>
    {
        return str_to_enum<FontStyle>(s,
        {
            {"normal",  fri::FontStyle::Normal},
            {"bold",    fri::FontStyle::Bold},
            {"italic",  fri::FontStyle::Italic}
        });
    }

    auto str_to_token_type (std::string_view s) -> std::optional<fri::TokenType>
    {
        return str_to_enum<TokenType>(s,
        {
            {"function",        fri::TokenType::Function},
            {"variable",        fri::TokenType::Variable},
            {"membervariable",  fri::TokenType::MemberVariable},
            {"keyword",         fri::TokenType::Keyword},
            {"controlkeyword",  fri::TokenType::ControlKeyword},
            {"plainsymbol",     fri::TokenType::PlainSymbol},
            {"userdefinedtype", fri::TokenType::UserDefinedType},
            {"builtintype",     fri::TokenType::BuiltinType},
            {"stringliteral",   fri::TokenType::StringLiteral},
            {"valueliteral",    fri::TokenType::ValueLiteral},
            {"numericliteral",  fri::TokenType::NumericLiteral},
            {"linenumber",      fri::TokenType::LineNumber}
        });
    }

    auto to_string (Color c) -> std::string
    {
        auto res = std::string("Color(");
        res += std::to_string(c.r_);
        res += ", ";
        res += std::to_string(c.g_);
        res += ", ";
        res += std::to_string(c.b_);
        res += ")";
        return res;
    }

    auto operator== (Color l, Color r) -> bool
    {
        return l.r_ == r.r_
            && l.g_ == r.g_
            && l.b_ == r.b_;
    }

    auto operator!= (Color l, Color r) -> bool
    {
        return not (l == r);
    }

    auto make_color (int32 r, int32 g, int32 b) -> std::optional<Color>
    {
        auto const validRanges = r >= 0 && r < 256
                              && g >= 0 && g < 256
                              && b >= 0 && b < 256;
        return validRanges
            ? std::optional<Color>(Color{r, g, b})
            : std::nullopt;
    }

    auto CodeStyle::get (TokenType const type) const -> TokenStyle const&
    {
        return style_[token_type_uindex(type)];
    }

    auto CodeStyle::set (TokenType const type, TokenStyle const style) -> void
    {
        style_[token_type_uindex(type)] = style;
    }

    auto CodeStyle::get_indent_size () const -> int32
    {
        return indentSpaceCount_;
    }

    auto CodeStyle::set_indent_size (int32 const spaceCount) -> void
    {
        indentSpaceCount_ = spaceCount;
    }

    IndentingCodeOutputter::IndentingCodeOutputter
        (CodeStyle const& style) :
        spaceCount_(style.get_indent_size()),
        currentLevel_(0)
    {
    }

    auto IndentingCodeOutputter::inc_indent () -> void
    {
        ++currentLevel_;
    }

    auto IndentingCodeOutputter::dec_indent () -> void
    {
        assert(currentLevel_ > 0);
        --currentLevel_;
    }

    auto IndentingCodeOutputter::wrap_line () -> void
    {
        this->end_line();
        this->begin_line();
    }

    auto IndentingCodeOutputter::get_current_indent () const -> IndentState
    {
        return IndentState
        {
            spaceCount_,
            currentLevel_
        };
    }

    auto IndentingCodeOutputter::get_spaces () const -> std::string_view
    {
        auto const count = this->get_current_space_count();
        assert(count < 1024);
        return std::string_view(Spaces.data(), Spaces.data() + count);
    }

    auto IndentingCodeOutputter::get_current_space_count () const -> int64
    {
        return spaceCount_ * currentLevel_;
    }

    ConsoleCodeOutputter::ConsoleCodeOutputter
        (CodeStyle style) :
        IndentingCodeOutputter(style),
        style_(std::move(style))
    {
    }

    auto ConsoleCodeOutputter::begin_line () -> void
    {
        std::cout << this->get_spaces();
    }

    auto ConsoleCodeOutputter::end_line () -> void
    {
        std::cout << "\n";
    }

    auto ConsoleCodeOutputter::blank_line () -> void
    {
        this->end_line();
    }

    auto ConsoleCodeOutputter::out
        (std::string_view token) -> ConsoleCodeOutputter&
    {
        return this->out(token, TokenType::PlainSymbol);
    }

    auto ConsoleCodeOutputter::out
        (std::string_view token, TokenType type) -> ConsoleCodeOutputter&
    {
        this->set_color(style_.get(type).color_);
        std::cout << token;
        this->reset_color();
        return *this;
    }

    auto ConsoleCodeOutputter::end_region () -> void
    {
        this->blank_line();
    }

    auto ConsoleCodeOutputter::set_color (Color color) -> void
    {
        auto constexpr ConsoleColors = std::array
        {
            std::make_tuple(Color {255, 0,   0}  , "\x1B[91m"),
            std::make_tuple(Color {0,   255, 0}  , "\x1B[92m"),
            std::make_tuple(Color {255, 255, 0}  , "\x1B[93m"),
            std::make_tuple(Color {0,   0,   255}, "\x1B[94m"),
            std::make_tuple(Color {255, 0,   255}, "\x1B[95m"),
            std::make_tuple(Color {0,   255, 255}, "\x1B[96m"),
            std::make_tuple(Color {255, 255, 255}, "\x1B[97m")
        };

        auto const calculate_distance = [](Color const& l, Color const& r)
        {
            auto const dr = l.r_ - r.r_;
            auto const dg = l.g_ - r.g_;
            auto const db = l.b_ - r.b_;
            return std::sqrt(dr*dr + dg*dg + db*db);
        };

        auto distances = std::array<double, size(ConsoleColors)>();
        auto distancesOut = begin(distances);
        auto consoleColorsIn = begin(ConsoleColors);
        for (auto i = 0; i < ssize(distances); ++i)
        {
            *distancesOut = calculate_distance(
                color,
                std::get<0>(*consoleColorsIn)
            );
            ++distancesOut;
            ++consoleColorsIn;
        }

        auto const closest = std::ranges::min_element(distances);
        auto const closestIndex = std::ranges::distance(
            begin(distances),
            closest
        );

        auto const closestColorPair = ConsoleColors[as_uindex(closestIndex)];
        std::cout << std::get<1>(closestColorPair);
    }

    auto ConsoleCodeOutputter::reset_color () -> void
    {
        std::cout << "\x1B[0m";
    }

    DummyCodeOutputter::DummyCodeOutputter
        (CodeStyle const& style) :
        IndentingCodeOutputter(style),
        currentColumn_(0)
    {
    }

    auto DummyCodeOutputter::begin_line () -> void
    {
        currentColumn_ += this->get_current_space_count();
    }

    auto DummyCodeOutputter::end_line () -> void
    {
        currentColumn_ = 0;
    }

    auto DummyCodeOutputter::blank_line () -> void
    {
        // TODO nestaci metoda z predka?
        this->end_line();
    }

    auto DummyCodeOutputter::end_region () -> void
    {
        this->blank_line();
    }

    auto DummyCodeOutputter::out (std::string_view s) -> DummyCodeOutputter&
    {
        currentColumn_ += s.size();
        return *this;
    }

    auto DummyCodeOutputter::out (
        std::string_view s,
        TokenType
    ) -> DummyCodeOutputter&
    {
        this->out(s);
        return *this;
    }

    auto DummyCodeOutputter::get_column () const -> int64
    {
        return currentColumn_;
    }
}