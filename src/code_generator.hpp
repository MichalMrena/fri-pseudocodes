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

    auto operator== (Color const&, Color const&) -> bool;
    auto operator!= (Color const&, Color const&) -> bool;

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
        Color string_;
        Color valLiteral_;
    };

    /**
     *  @brief Interface for code printers.
     */
    class ICodePrinter
    {
    public:
        virtual ~ICodePrinter() = default;

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
         *  @brief Terminates current line and jumps to the begining of the next line.
         */
        virtual auto end_line () -> void = 0;

        /**
         *  @brief Prints an empty line to the output.
         */
        virtual auto blank_line () -> void = 0;

        /**
         *  @brief Prints string to the output.
         */
        virtual auto out (std::string_view) -> ICodePrinter& = 0;

        /**
         *  @brief Prints string to the output using given or similar color if possible.
         */
        virtual auto out (std::string_view, Color const&) -> ICodePrinter& = 0;
    };

    /**
     *  @brief Prints code to the console.
     */
    class ConsoleCodePrinter : public ICodePrinter
    {
    public:
        ConsoleCodePrinter  ();
        ~ConsoleCodePrinter ();

        auto inc_indent () -> void override;
        auto dec_indent () -> void override;
        auto begin_line () -> void override;
        auto end_line   () -> void override;
        auto blank_line () -> void override;

        auto out        (std::string_view)               -> ConsoleCodePrinter& override;
        auto out        (std::string_view, Color const&) -> ConsoleCodePrinter& override;

    private:
        auto set_color   (Color const&) -> void;
        auto reset_color ()             -> void;

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
        PseudocodeGenerator (ICodePrinter&, CodeColorInfo);

        auto visit (IntLiteral const&)           -> void override;
        auto visit (FloatLiteral const&)         -> void override;
        auto visit (StringLiteral const&)        -> void override;
        auto visit (NullLiteral const&)          -> void override;
        auto visit (BoolLiteral const&)          -> void override;
        auto visit (BinaryOperator const&)       -> void override;
        auto visit (Parenthesis const&)          -> void override;
        auto visit (VarRef const&)               -> void override;
        auto visit (MemberVarRef const&)         -> void override;
        auto visit (UnaryOperator const&)        -> void override;
        auto visit (New const&)                  -> void override;
        auto visit (FunctionCall const&)         -> void override;
        auto visit (DestructorCall const&)       -> void override;
        auto visit (MemberFunctionCall const&)   -> void override;
        auto visit (This const&)                 -> void override;

        auto visit (PrimType const&)             -> void override;
        auto visit (CustomType const&)           -> void override;
        auto visit (Indirection const&)          -> void override;

        auto visit (Class const& c)              -> void override;
        auto visit (Method const& c)             -> void override;
        auto visit (VarDefCommon const& c)       -> void override;
        auto visit (FieldDefinition const& c)    -> void override;
        auto visit (ParamDefinition const& c)    -> void override;
        auto visit (VarDefinition const& c)      -> void override;
        auto visit (ForLoop const& c)            -> void override;
        auto visit (WhileLoop const& c)          -> void override;
        auto visit (DoWhileLoop const& c)        -> void override;
        auto visit (CompoundStatement const& c)  -> void override;
        auto visit (ExpressionStatement const&)  -> void override;
        auto visit (Return const&)               -> void override;
        auto visit (Assignment const&)           -> void override;
        auto visit (If const&)                   -> void override;
        auto visit (Delete const&)               -> void override;
        auto visit (Throw const&)                -> void override;

    private:
        static auto bin_op_to_string (BinOpcode)        -> std::string;
        static auto un_op_to_string  (UnOpcode)         -> std::string;
        static auto is_compound_op   (BinOpcode)        -> bool;
        static auto is_postfixx      (UnOpcode)         -> bool;

        auto op_color (std::string_view) -> Color;

        template<class InputIt>
        auto visit_range (std::string_view, InputIt, InputIt) -> void;

    private:
        ICodePrinter* out_;
        CodeColorInfo colors_;
    };
}

#endif