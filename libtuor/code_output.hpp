#ifndef FRI_CODE_OUTPUTTER_HPP
#define FRI_CODE_OUTPUTTER_HPP

#include <libtuor/types.hpp>
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <iosfwd>
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
     *  @brief Describes actual state of indentation in a code printer.
     */
    struct IndentState
    {
        int64 spaceCount_;
        int64 currentLevel_;
    };

    /**
     *  @brief Interface for code outputters.
     */
    class ICodeOutputter
    {
    public:
        virtual ~ICodeOutputter () = default;

        /**
         *  @brief Increase current indentation.
         */
        virtual auto inc_indent () -> void = 0;

        /**
         *  @brief Decrease current indentation.
         */
        virtual auto dec_indent () -> void = 0;

        /**
         *  @brief Adds indentation characters to the output.
         */
        virtual auto begin_line () -> void = 0;

        /**
         *  @brief Terminates current line and jumps to the next line.
         */
        virtual auto end_line () -> void = 0;

        /**
         *  @brief Outputs an empty line.
         */
        virtual auto blank_line () -> void = 0;

        /**
         *  @brief Jumps to the next line with the same indent.
         */
        virtual auto wrap_line () -> void = 0;

        /**
         *  @brief Prints string to the output.
         */
        virtual auto out (std::string_view token) -> ICodeOutputter& = 0;

        /**
         *  @brief Prints string to the output using given style if possible.
         */
        virtual auto out
            (std::string_view token , TokenType type) -> ICodeOutputter& = 0;

        /**
         *  @brief Current state of indentation.
         */
        virtual auto get_current_indent () const -> IndentState = 0;

        /**
         *  @brief Ends the current region e.g. page.
         */
        virtual auto end_region () -> void = 0;
    };

    /**
     *  @brief Code outputter that manages indentation.
     */
    class IndentingCodeOutputter : public ICodeOutputter
    {
    public:
        IndentingCodeOutputter(CodeStyle const& style);

        auto inc_indent () -> void override;

        auto dec_indent () -> void override;

        auto wrap_line () -> void override;

        auto get_current_indent () const -> IndentState override;

    protected:
        auto get_spaces () const -> std::string_view;

        auto get_current_space_count () const -> int64;

    private:
        inline static const auto Spaces = std::string(1024, ' ');

    private:
        int64 spaceCount_;
        int64 currentLevel_;
    };

    /**
     *  @brief Console code outputter.
     */
    class ConsoleCodeOutputter : public IndentingCodeOutputter
    {
    public:
        ConsoleCodeOutputter (CodeStyle style);

        auto begin_line () -> void override;

        auto end_line () -> void override;

        auto blank_line () -> void override;

        auto end_region () -> void override;

        auto out (std::string_view token) -> ConsoleCodeOutputter& override;

        auto out (
            std::string_view token,
            TokenType type
        ) -> ConsoleCodeOutputter& override;

    private:
        auto set_color (Color color) -> void;

        auto reset_color () -> void;

    private:
        CodeStyle style_;
    };

    // /**
    //  *  @brief Rtf code outputter.
    //  */
    // class RtfCodeOutputter : public IndentingCodeOutputter
    // {
    // public:
    //     RtfCodeOutputter(std::ostream& ost, CodeStyle style);
    // };

    // /**
    //  *  @brief LaTeX code outputter.
    //  */
    // class LatexCodeOutputter : public IndentingCodeOutputter
    // {
    // public:
    //     RtfCodeOutputter(std::ostream& ost, CodeStyle style);
    // };

    /**
    *  @brief /dev/null code outputter. Measures how long a line would be.
    */
    class DummyCodeOutputter : public IndentingCodeOutputter
    {
    public:
        DummyCodeOutputter (CodeStyle const& style);

        auto begin_line () -> void override;
        auto end_line   () -> void override;
        auto blank_line () -> void override;
        auto end_region () -> void override;

        auto out (std::string_view token) -> DummyCodeOutputter& override;
        auto out (
            std::string_view token,
            TokenType
        ) -> DummyCodeOutputter& override;

        auto get_column () const -> int64;

    private:
        int64 currentColumn_;
    };

    auto make_code_outputter (OutputType) -> std::unique_ptr<ICodeOutputter>;
}

#endif