#ifndef FRI_CODE_GENERATOR_HPP
#define FRI_CODE_GENERATOR_HPP

#include <libtuor/types.hpp>
#include "abstract_code.hpp"
#include "code_output.hpp"
#include "types.hpp"

#include <cstdint>
#include <fstream>
#include <ostream>
#include <string_view>
#include <unordered_map>

namespace fri
{
    // /**
    //  *  @brief Prints code to a RTF file.
    //  */
    // class RtfCodePrinter : public CommonCodePrinter
    // {
    // public:
    //     RtfCodePrinter   (std::ofstream&, OutputSettings const&);
    //     ~RtfCodePrinter  ();

    //     auto begin_line () -> void override;
    //     auto end_line   () -> void override;
    //     auto blank_line () -> void override;
    //     auto end_region () -> void override;

    //     auto out (std::string_view) -> RtfCodePrinter& override;
    //     auto out (std::string_view, TokenStyle const&) -> RtfCodePrinter& override;


    // private:
    //     using base = CommonCodePrinter;

    // private:
    //     auto begin_color (Color const&) -> void;
    //     auto end_color   ()             -> void;
    //     auto begin_style (FontStyle)    -> void;
    //     auto end_style   (FontStyle)    -> void;
    //     auto color_code  (Color const&) -> unsigned;
    //     static auto encode (std::string_view) -> std::string;

    // private:
    //     std::ofstream*     ofst_;
    //     std::vector<Color> colors_;
    // };

    // /**
    //  *  @brief Decorates code priter with line numbering.
    //  */
    // class NumberedCodePrinter : public ICodePrinter
    // {
    // public:
    //     NumberedCodePrinter(ICodePrinter&, std::size_t, TokenStyle);

    //     auto inc_indent () -> void override;
    //     auto dec_indent () -> void override;
    //     auto begin_line () -> void override;
    //     auto end_line   () -> void override;
    //     auto wrap_line  () -> void override;
    //     auto blank_line () -> void override;
    //     auto end_region () -> void override;

    //     auto out (std::string_view) -> NumberedCodePrinter& override;
    //     auto out (std::string_view, TokenStyle const&) -> NumberedCodePrinter& override;

    //     auto current_indent () const -> IndentState override;

    // private:
    //     auto out_number () -> void;
    //     auto out_spaces () -> void;

    // private:
    //     inline static constexpr auto Spaces
    //         = std::string_view("                                             ");

    // private:
    //     ICodePrinter*     decoree_;
    //     std::size_t const numWidth_;
    //     TokenStyle const   numStyle_;
    //     std::size_t       currentNum_;
    // };

    // TODO use
    // struct IsInline
    // {
    //     bool is_;
    // };
    // instead of:
    enum class IsInline
    {
        Inline, NoInline
    };

    /**
     *  @brief Generates pseudocode.
     */
    class PseudocodeGenerator : public CodeVisitor
    {
    public:
        PseudocodeGenerator (ICodeOutputter& out);

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
        auto visit (TemplatedType const&)        -> void override;
        auto visit (Indirection const&)          -> void override;
        auto visit (Function const&)             -> void override;
        auto visit (Nested const&)               -> void override;

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
        auto visit (Break const&)                -> void override;
        auto visit (Case const&)                 -> void override;
        auto visit (Switch const&)               -> void override;

        auto out_plain    (std::string_view)     -> void;
        auto out_var_name (std::string_view)     -> void;

    private:
        static auto bin_op_to_string     (BinOpcode) -> std::string;
        static auto un_op_to_string      (UnOpcode)  -> std::string;
        static auto is_compound_op       (BinOpcode) -> bool;
        static auto is_call              (UnOpcode)  -> bool;
        static auto is_postfixx          (UnOpcode)  -> bool;
        static auto is_bothtfix          (UnOpcode)  -> bool;
        static auto simplify_type_name   (std::string_view) -> std::string_view;
        static auto simplify_member_name (std::string_view) -> std::string_view;

        auto visit_decl (Class const&, Method const&, IsInline)      -> void;
        auto visit_decl (Class const&, Constructor const&, IsInline) -> void;
        auto visit_decl (Class const&, Destructor const&)            -> void;
        auto visit_def  (Class const&, Method const&)                -> void;
        auto visit_def  (Class const&, Constructor const&)           -> void;
        auto visit_def  (Class const&, Destructor const&)            -> void;

        auto visit_member_base (Expression const&) -> void;
        auto visit_class_name  (Class const&) -> void;

        /**
         *  @brief Outputs declaration of method/constructor either into single
         *  line or multiple line if the decl is long.
         *  @tparam OutputName outputs `int foo` or `int Bar.foo`
         *          or `konštruktor` or `konštruktor Bar`.
         *  @tparam OutputType outputs `: int` or nothing for constructor.
         */
        template<class OutputName, class OutputType>
        auto visit_decl (
            OutputName&&,
            std::vector<ParamDefinition> const&,
            OutputType&&
        ) -> void;

        template<class Range>
        auto output_range (Range&&, std::string_view, TokenType) -> void;

        auto visit_args (std::vector<std::unique_ptr<Expression>> const&) -> void;

        template<class Range, class OutputSep>
        auto visit_range (Range&&, OutputSep&&) -> void;

        template<class Range, class Visitor, class OutputSep>
        auto visit_range (Range&&, Visitor&&, OutputSep&&) -> void;

        /**
         *  Temporarly swaps @c out_ for dummy, executes @c LineOut arg
         *  and then returns length of the current line.
         *  (If @c LineOut uses multiple lines, it returns length
         *  of the last line. )
         */
        template<class LineOut>
        auto try_output_length (LineOut&&) -> int64;

        auto map_func_name (std::string const&) const -> std::string_view;

    private:
        ICodeOutputter* out_;
        std::unordered_map<std::string, std::string> funcNames_;
    };

    /**
     *  @brief Visits variable definition or does nothing.
     */
    struct ForVarDefVisitor : public CodeVisitorAdapter
    {
        PseudocodeGenerator* real_;
        ForVarDefVisitor(PseudocodeGenerator&);
        auto visit (VarDefinition const&) -> void override;
    };

    /**
     *  @brief Visits for loop condition.
     */
    struct ForFromVisitor : public CodeVisitorAdapter
    {
        PseudocodeGenerator* real_;
        ForFromVisitor(PseudocodeGenerator&);
        auto visit (VarDefinition const&) -> void override;
    };

    /**
     *  @brief Visits for loop condition.
     */
    struct ForToVisitor : public CodeVisitorAdapter
    {
        PseudocodeGenerator* real_;
        ForToVisitor(PseudocodeGenerator&);
        auto visit (BinaryOperator const&) -> void override;
    };
}

#endif