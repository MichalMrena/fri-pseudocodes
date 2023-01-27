#include <libtuor/code_output.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/container/flat_map.hpp>

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

    // auto to_string (Color c) -> std::string
    // {
    //     auto res = std::string("Color(");
    //     res += std::to_string(c.r_);
    //     res += ", ";
    //     res += std::to_string(c.g_);
    //     res += ", ";
    //     res += std::to_string(c.b_);
    //     res += ")";
    //     return res;
    // }

    // auto operator== (Color l, Color r) -> bool
    // {
    //     return l.r_ == r.r_
    //         && l.g_ == r.g_
    //         && l.b_ == r.b_;
    // }

    // auto operator!= (Color l, Color r) -> bool
    // {
    //     return not (l == r);
    // }

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
}