#ifndef FRI_CODE_OUTPUTTER_HPP
#define FRI_CODE_OUTPUTTER_HPP

#include <libtuor/types.hpp>
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace fri
{
    template<class Enum>
    auto constexpr enum_instance_count () -> std::size_t
    {
        return static_cast<std::size_t>(
            static_cast<std::underlying_type_t<Enum>>(Enum::COUNT)
        );
    }

    /**
     *  @brief Font style.
     */
    enum class FontStyle
    {
        Normal,
        Bold,
        Italic,
        COUNT
    };

    auto str_to_font_style (std::string_view s) -> std::optional<FontStyle>;

    /**
     *  @brief Identifies type of a token.
     */
    enum class TokenType
    {
        Function,
        Variable,
        MemberVariable,
        Keyword,
        ControlKeyword,
        PlainSymbol,
        UserDefinedType,
        BuiltinType,
        StringLiteral,
        ValueLiteral,
        NumericLiteral,
        LineNumber,
        COUNT
    };

    auto constexpr token_type_uindex (TokenType const type) -> std::size_t
    {
        return static_cast<std::size_t>(
            static_cast<std::underlying_type_t<TokenType>>(type)
        );
    }

    auto str_to_token_type
        (std::string_view s) -> std::optional<fri::TokenType>;

    /**
     *  @brief Output format type.
     */
    enum class OutputType
    {
        Console,
        Rtf,
        LaTeX,
        COUNT
    };

    auto str_to_output_type (std::string_view s) -> std::optional<OutputType>;

    /**
     *  @brief RGB color.
     */
    struct Color
    {
        int32 r_{};
        int32 g_{};
        int32 b_{};
    };

    auto to_string  (Color c) -> std::string;
    auto operator== (Color l, Color r) -> bool;
    auto operator!= (Color l, Color r) -> bool;
    auto make_color (int32 r, int32 g, int32 b) -> std::optional<Color>;

    /**
     *  @brief Token style.
     */
    struct TokenStyle
    {
        Color color_{};
        FontStyle style_{FontStyle::Normal};
    };

    /**
     *  @brief Style of different parts of code.
     */
    class CodeStyle
    {
    public:
        auto get (TokenType type) const -> TokenStyle const&;
        auto set (TokenType type, TokenStyle style) -> void;
        auto get_indent_size () const -> int32;
        auto set_indent_size (int32 spaceCount) -> void;

    private:
        std::array<TokenStyle, enum_instance_count<TokenType>()> style_;
        int32 indentSpaceCount_;
    };

    /**
     *  @brief Interface for code outputters.
     */
    class ICodeOutputter
    {
    public:
        virtual ~ICodeOutputter () = default;
    };

    /**
     *  @brief Console code outputter.
     */
    class ConsoleCodeOutputter : public ICodeOutputter
    {
    public:
    };

    /**
     *  @brief Rtf code outputter.
     */
    class RtfCodeOutputter : public ICodeOutputter
    {
    public:
    };

    /**
     *  @brief LaTeX code outputter.
     */
    class LatexCodeOutputter : public ICodeOutputter
    {
    public:
    };

    auto make_code_outputter (OutputType) -> std::unique_ptr<ICodeOutputter>;
}

#endif