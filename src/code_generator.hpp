#ifndef FRI_CODE_GENERATOR_HPP
#define FRI_CODE_GENERATOR_HPP

#include "abstract_code.hpp"

#include <ostream>
#include <string_view>
#include <cstdint>

namespace fri
{
    /**
     *  @brief RGB color.
     */
    struct Color
    {
        std::uint8_t r_;
        std::uint8_t g_;
        std::uint8_t b_;
    };

    /**
     *  @brief Colors of different parts of code.
     */
    struct CodeColorInfo
    {
        Color function_;
        Color variable_;
        Color keyword_;
        Color plain_;
        Color customType_;
        Color primType_;
    };

    /**
     *  @brief Interface for code printers.
     */
    class ICodePrinter
    {
    public:
        virtual ~ICodePrinter() = default;

        virtual auto inc_indent () -> void = 0;
        virtual auto dec_indent () -> void = 0;
        virtual auto begin_line () -> void = 0;
        virtual auto end_line   () -> void = 0;
        virtual auto blank_line () -> void = 0;

        virtual auto out        (std::string_view) -> ICodePrinter& = 0;
        virtual auto set_color  (Color const&)     -> void = 0;
    };

    /**
     *  @brief Base class for code printers.
     */
    class ConsoleCodePrinter : public ICodePrinter
    {
    public:
        ConsoleCodePrinter ();

        auto inc_indent () -> void override;
        auto dec_indent () -> void override;
        auto begin_line () -> void override;
        auto end_line   () -> void override;
        auto blank_line () -> void override;

        auto out        (std::string_view) -> ConsoleCodePrinter& override;
        auto set_color  (Color const&)     -> void override;

    private:
        std::size_t      indentStep_;
        std::size_t      currentIndent_;
        std::string_view spaces_;
    };

    /**
     *  @brief Generates pseudocode.
     */
    class PseudocodeGenerator : public CodeVisitor
    {
    public:
        PseudocodeGenerator (ICodePrinter&, CodeColorInfo const&);

        auto visit (IntLiteral const&)     -> void override;
        auto visit (FloatLiteral const&)   -> void override;
        auto visit (StringLiteral const&)  -> void override;
        auto visit (BinaryOperator const&) -> void override;

        auto visit (PrimType const&)    -> void override;
        auto visit (CustomType const&)  -> void override;
        auto visit (Indirection const&) -> void override;

        auto visit (Class const& c)              -> void override;
        auto visit (Method const& c)             -> void override;
        auto visit (ForLoop const& c)            -> void override;
        auto visit (WhileLoop const& c)          -> void override;
        auto visit (DoWhileLoop const& c)        -> void override;
        auto visit (FieldDefinition const& c)    -> void override;
        auto visit (VariableDefinition const& c) -> void override;
        auto visit (CompoundStatement const& c)  -> void override;

    private:
        ICodePrinter*        out_;
        CodeColorInfo const* colors_;
    };
}

#endif