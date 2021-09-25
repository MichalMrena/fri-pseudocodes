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
        TextStyle controlKeyword_ {Color {}, FontStyle::Normal};
        TextStyle plain_          {Color {}, FontStyle::Normal};
        TextStyle customType_     {Color {}, FontStyle::Normal};
        TextStyle primType_       {Color {}, FontStyle::Normal};
        TextStyle stringLiteral_  {Color {}, FontStyle::Normal};
        TextStyle valLiteral_     {Color {}, FontStyle::Normal};
        TextStyle numLiteral_     {Color {}, FontStyle::Normal};
        TextStyle lineNumber_     {Color {}, FontStyle::Normal};
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
     *  @brief Describes actual state of indentation in a code printer.
     */
    struct IndentState
    {
        std::size_t step;
        std::size_t current;
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
         *  @brief Jumps to the next line with the same indent.
         */
        virtual auto wrap_line () -> void = 0;

        /**
         *  @brief Prints string to the output.
         */
        virtual auto out (std::string_view) -> ICodePrinter& = 0;

        /**
         *  @brief Prints string to the output using given style if possible.
         */
        virtual auto out (std::string_view, TextStyle const&) -> ICodePrinter& = 0;

        /**
         *  @brief Current state of indentation.
         */
        virtual auto current_indent () const -> IndentState = 0;

        /**
         *  @brief Ends the current region e.g. page.
         */
        virtual auto end_region () -> void = 0;
    };

    /**
     *  @brief Implements indentation that is common for all printers.
     */
    class CommonCodePrinter : public ICodePrinter
    {
    public:
        CommonCodePrinter (OutputSettings const&);
        CommonCodePrinter (IndentState);

        auto inc_indent     () -> void override;
        auto dec_indent     () -> void override;
        auto wrap_line      () -> void override;
        auto current_indent () const -> IndentState override;

    protected:
        auto get_indent () const -> std::string_view;

    private:
        inline static constexpr auto Spaces
            = std::string_view("                                             ");

    private:
        std::size_t indentStep_;
        std::size_t indentCurrent_;
    };

    /**
     *  @brief Prints code to the console.
     */
    class ConsoleCodePrinter : public CommonCodePrinter
    {
    public:
        ConsoleCodePrinter  (OutputSettings const&);

        auto begin_line () -> void override;
        auto end_line   () -> void override;
        auto blank_line () -> void override;
        auto end_region () -> void override;

        auto out (std::string_view) -> ConsoleCodePrinter& override;
        auto out (std::string_view, TextStyle const&) -> ConsoleCodePrinter& override;

    private:
        using base = CommonCodePrinter;

    private:
        auto set_color   (Color const&) -> void;
        auto reset_color ()             -> void;
    };

    /**
     *  @brief Prints code to a RTF file.
     */
    class RtfCodePrinter : public CommonCodePrinter
    {
    public:
        RtfCodePrinter   (std::ofstream&, OutputSettings const&);
        ~RtfCodePrinter  ();

        auto begin_line () -> void override;
        auto end_line   () -> void override;
        auto blank_line () -> void override;
        auto end_region () -> void override;

        auto out (std::string_view) -> RtfCodePrinter& override;
        auto out (std::string_view, TextStyle const&) -> RtfCodePrinter& override;


    private:
        using base = CommonCodePrinter;

    private:
        auto begin_color (Color const&) -> void;
        auto end_color   ()             -> void;
        auto begin_style (FontStyle)    -> void;
        auto end_style   (FontStyle)    -> void;
        auto color_code  (Color const&) -> unsigned;
        static auto encode (std::string_view) -> std::string;

    private:
        std::ofstream*     ofst_;
        std::vector<Color> colors_;
    };

    /**
     *  @brief /dev/null code printer. Used to mease how long a line would be.
     */
    class DummyCodePrinter : public CommonCodePrinter
    {
    public:
        DummyCodePrinter (IndentState);

        auto begin_line () -> void override;
        auto end_line   () -> void override;
        auto blank_line () -> void override;
        auto end_region () -> void override;

        auto out (std::string_view) -> DummyCodePrinter& override;
        auto out (std::string_view, TextStyle const&) -> DummyCodePrinter& override;

        auto get_column () const -> std::size_t;

    private:
        using base = CommonCodePrinter;

    private:
        std::size_t currentColumn_ {0};
    };

    /**
     *  @brief Decorates code priter with line numbering.
     */
    class NumberedCodePrinter : public ICodePrinter
    {
    public:
        NumberedCodePrinter(ICodePrinter&, std::size_t, TextStyle);

        auto inc_indent () -> void override;
        auto dec_indent () -> void override;
        auto begin_line () -> void override;
        auto end_line   () -> void override;
        auto wrap_line  () -> void override;
        auto blank_line () -> void override;
        auto end_region () -> void override;

        auto out (std::string_view) -> NumberedCodePrinter& override;
        auto out (std::string_view, TextStyle const&) -> NumberedCodePrinter& override;

        auto current_indent () const -> IndentState override;

    private:
        auto out_number () -> void;
        auto out_spaces () -> void;

    private:
        inline static constexpr auto Spaces
            = std::string_view("                                             ");

    private:
        ICodePrinter*     decoree_;
        std::size_t const numWidth_;
        TextStyle const   numStyle_;
        std::size_t       currentNum_;
    };

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
        static auto bin_op_to_string   (BinOpcode) -> std::string;
        static auto un_op_to_string    (UnOpcode)  -> std::string;
        static auto is_compound_op     (BinOpcode) -> bool;
        static auto is_call            (UnOpcode)  -> bool;
        static auto is_postfixx        (UnOpcode)  -> bool;
        static auto is_bothtfix        (UnOpcode)  -> bool;
        static auto simplify_type_name (std::string_view) -> std::string_view;

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
        auto visit_decl (OutputName&&, std::vector<ParamDefinition> const&, OutputType&&) -> void;

        template<class Range>
        auto output_range (Range&&, std::string_view, TextStyle const&) -> void;

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
        auto try_output_length (LineOut&&) -> std::size_t;

    private:
        ICodePrinter* out_;
        CodeStyleInfo style_;
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