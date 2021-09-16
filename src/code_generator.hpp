#ifndef FRI_CODE_GENERATOR_HPP
#define FRI_CODE_GENERATOR_HPP

#include "abstract_code.hpp"

#include <ostream>
#include <fstream>
#include <string_view>
#include <cstdint>

namespace fri
{
    /**
     *  @brief RGB color.
     */
    struct Color
    {
        std::uint8_t r_ {};
        std::uint8_t g_ {};
        std::uint8_t b_ {};
    };

    auto to_string  (Color const& c) -> std::string;
    auto operator== (Color const&, Color const&) -> bool;
    auto operator!= (Color const&, Color const&) -> bool;

    /**
     *  @brief Font style.
     */
    enum class FontStyle
    {
        Normal, Bold, Italic
    };

    /**
     *  @brief Text style.
     */
    struct TextStyle
    {
        Color color_ {};
        FontStyle style_ {FontStyle::Normal};
    };

    /**
     *  @brief Style of different parts of code.
     */
    struct CodeStyleInfo
    {
        TextStyle function_       {Color {}, FontStyle::Normal};
        TextStyle variable_       {Color {}, FontStyle::Normal};
        TextStyle memberVariable_ {Color {}, FontStyle::Normal};
        TextStyle keyword_        {Color {}, FontStyle::Normal};
        TextStyle plain_          {Color {}, FontStyle::Normal};
        TextStyle customType_     {Color {}, FontStyle::Normal};
        TextStyle primType_       {Color {}, FontStyle::Normal};
        TextStyle stringLiteral_  {Color {}, FontStyle::Normal};
        TextStyle valLiteral_     {Color {}, FontStyle::Normal};
        TextStyle numLiteral_     {Color {}, FontStyle::Normal};
    };

    /**
     *  @brief Output settings.
     */
    struct OutputSettings
    {
        unsigned int  fontSize = 9;
        unsigned int  indentSpaces = 2;
        std::string   font {"Consolas"};
        CodeStyleInfo style {};
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
         *  @brief Prints string to the output using given or similar color if possible and given style if possible.
         */
        virtual auto out (std::string_view, TextStyle const&) -> ICodePrinter& = 0;
    };

    /**
     *  @brief Prints code to the console.
     */
    class ConsoleCodePrinter : public ICodePrinter
    {
    public:
        ConsoleCodePrinter  (OutputSettings const&);
        ~ConsoleCodePrinter ();

        auto inc_indent () -> void override;
        auto dec_indent () -> void override;
        auto begin_line () -> void override;
        auto end_line   () -> void override;
        auto blank_line () -> void override;

        auto out (std::string_view) -> ConsoleCodePrinter& override;
        auto out (std::string_view, TextStyle const&) -> ConsoleCodePrinter& override;

    private:
        auto set_color   (Color const&) -> void;
        auto reset_color ()             -> void;

    private:
        std::size_t      indentStep_;
        std::size_t      currentIndent_;
        std::string_view spaces_;
    };

    /**
     *  @brief Prints code to a RTF file.
     */
    class RtfCodePrinter : public ICodePrinter
    {
    public:
        RtfCodePrinter   (std::ofstream&, OutputSettings const&);
        ~RtfCodePrinter  ();

        auto inc_indent  () -> void override;
        auto dec_indent  () -> void override;
        auto begin_line  () -> void override;
        auto end_line    () -> void override;
        auto blank_line  () -> void override;

        auto out (std::string_view) -> RtfCodePrinter& override;
        auto out (std::string_view, TextStyle const&) -> RtfCodePrinter& override;

    private:
        auto begin_color (Color const&) -> void;
        auto end_color   ()             -> void;
        auto begin_style (FontStyle)    -> void;
        auto end_style   (FontStyle)    -> void;
        auto color_code  (Color const&) -> unsigned;
        static auto encode (std::string_view) -> std::string;

    private:
        std::ofstream*     ofst_;
        std::size_t        indentStep_;
        std::size_t        currentIndent_;
        std::string_view   spaces_;
        std::vector<Color> colors_;
    };

    /**
     *  @brief Generates pseudocode.
     */
    class PseudocodeGenerator : public CodeVisitor
    {
    public:
        PseudocodeGenerator (ICodePrinter&, CodeStyleInfo);

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
        auto visit (ConstructorCall const&)      -> void override;
        auto visit (DestructorCall const&)       -> void override;
        auto visit (MemberFunctionCall const&)   -> void override;
        auto visit (ExpressionCall const&)       -> void override;
        auto visit (This const&)                 -> void override;
        auto visit (IfExpression const&)         -> void override;
        auto visit (Lambda const&)               -> void override;

        auto visit (PrimType const&)             -> void override;
        auto visit (CustomType const&)           -> void override;
        auto visit (Indirection const&)          -> void override;

        auto visit (Class const&)                -> void override;
        auto visit (Method const&)               -> void override;
        auto visit (VarDefCommon const&)         -> void override;
        auto visit (FieldDefinition const&)      -> void override;
        auto visit (ParamDefinition const&)      -> void override;
        auto visit (VarDefinition const&)        -> void override;
        auto visit (ForLoop const&)              -> void override;
        auto visit (WhileLoop const&)            -> void override;
        auto visit (DoWhileLoop const&)          -> void override;
        auto visit (CompoundStatement const&)    -> void override;
        auto visit (ExpressionStatement const&)  -> void override;
        auto visit (Return const&)               -> void override;
        auto visit (If const&)                   -> void override;
        auto visit (Delete const&)               -> void override;
        auto visit (Throw const&)                -> void override;

    private:
        static auto bin_op_to_string (BinOpcode) -> std::string;
        static auto un_op_to_string  (UnOpcode)  -> std::string;
        static auto is_compound_op   (BinOpcode) -> bool;
        static auto is_postfixx      (UnOpcode)  -> bool;
        static auto is_bothtfix      (UnOpcode)  -> bool;

        auto visit (Constructor const&, CompoundStatement const&) -> void;

        auto visit_decl (Method const&)      -> void;
        auto visit_decl (Constructor const&) -> void;
        auto visit_decl (Destructor const&)  -> void;
        auto visit_def  (Class const&, Method const&)      -> void;
        auto visit_def  (Class const&, Constructor const&) -> void;
        auto visit_def  (Class const&, Destructor const&)  -> void;

        auto visit_member_base (Expression const&) -> void;
        auto visit_class_name  (Class const&) -> void;

        auto op_color (std::string_view) -> Color;

        template<class Range>
        auto output_range (Range&&, std::string_view, TextStyle const&) -> void;

        template<class InputIt>
        auto visit_range (std::string_view, InputIt, InputIt) -> void;

    private:
        ICodePrinter* out_;
        CodeStyleInfo style_;
    };

    /**
     *  @brief Adapter that implements methods as empty.
     */
    class CodeVisitorAdapter : public CodeVisitor
    {
        auto visit (IntLiteral const&)           -> void override {};
        auto visit (FloatLiteral const&)         -> void override {};
        auto visit (StringLiteral const&)        -> void override {};
        auto visit (NullLiteral const&)          -> void override {};
        auto visit (BoolLiteral const&)          -> void override {};
        auto visit (BinaryOperator const&)       -> void override {};
        auto visit (Parenthesis const&)          -> void override {};
        auto visit (VarRef const&)               -> void override {};
        auto visit (MemberVarRef const&)         -> void override {};
        auto visit (UnaryOperator const&)        -> void override {};
        auto visit (New const&)                  -> void override {};
        auto visit (FunctionCall const&)         -> void override {};
        auto visit (ConstructorCall const&)      -> void override {};
        auto visit (DestructorCall const&)       -> void override {};
        auto visit (MemberFunctionCall const&)   -> void override {};
        auto visit (ExpressionCall const&)       -> void override {};
        auto visit (This const&)                 -> void override {};
        auto visit (IfExpression const&)         -> void override {};
        auto visit (Lambda const&)               -> void override {};

        auto visit (PrimType const&)             -> void override {};
        auto visit (CustomType const&)           -> void override {};
        auto visit (Indirection const&)          -> void override {};

        auto visit (Class const&)                -> void override {};
        auto visit (Method const&)               -> void override {};
        auto visit (VarDefCommon const&)         -> void override {};
        auto visit (FieldDefinition const&)      -> void override {};
        auto visit (ParamDefinition const&)      -> void override {};
        auto visit (VarDefinition const&)        -> void override {};
        auto visit (ForLoop const&)              -> void override {};
        auto visit (WhileLoop const&)            -> void override {};
        auto visit (DoWhileLoop const&)          -> void override {};
        auto visit (CompoundStatement const&)    -> void override {};
        auto visit (ExpressionStatement const&)  -> void override {};
        auto visit (Return const&)               -> void override {};
        auto visit (If const&)                   -> void override {};
        auto visit (Delete const&)               -> void override {};
        auto visit (Throw const&)                -> void override {};
    };

    class IsCheckVisitorBase : public CodeVisitorAdapter
    {
    public:
        auto result () const -> bool;

    protected:
        bool result_ {false};
    };

    /**
     *  @brief Checks whether given type is void.
     */
    struct IsVoidVisitor : public IsCheckVisitorBase
    {
        auto visit (PrimType const&) -> void override;
    };

    /**
     *  @brief Check if given expression is this pointer.
     */
    struct IsThisVisitor : public IsCheckVisitorBase
    {
        auto visit (This const&) -> void override;
    };

    /**
     *  @brief Check
     */
    struct IsIndirectionVisitor : public IsCheckVisitorBase
    {
        auto visit (Indirection const&) -> void override;
    };
}

#endif