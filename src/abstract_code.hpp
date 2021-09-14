#ifndef FRI_ABSTRACT_CODE_HPP
#define FRI_ABSTRACT_CODE_HPP

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <optional>
#include <variant>

namespace fri
{
    class CodeVisitor;

    struct Expression
    {
        virtual ~Expression () = default;
        virtual auto accept (CodeVisitor&) const -> void = 0;
    };

    struct Statement
    {
        virtual ~Statement () = default;
        virtual auto accept (CodeVisitor&) const -> void = 0;
    };

    struct Type
    {
        virtual ~Type () = default;
        virtual auto accept (CodeVisitor&) const -> void = 0;
    };

    /**
     *  @brief Implements accept using CRTP.
     */
    template<class Derived>
    struct Visitable
    {
        auto accept (CodeVisitor&) const -> void;
    };

    /**
     *  @brief Implements accept using CRTP. Makes derived classe a @p VirtualBase .
     */
    template<class VirtualBase, class Derived>
    struct VisitableFamily : public VirtualBase
    {
        auto accept (CodeVisitor&) const -> void override;
    };

// Types:

    struct PrimType : public VisitableFamily<Type, PrimType>
    {
        std::string name_;
        PrimType (std::string name);
    };

    struct CustomType : public VisitableFamily<Type, CustomType>
    {
        std::string name_;
        CustomType (std::string name);
    };

    struct Indirection : public VisitableFamily<Type, Indirection>
    {
        std::unique_ptr<Type> pointee_ {};
        Indirection (std::unique_ptr<Type> pointee);
    };

// Other:

    struct VarDefCommon : public Visitable<VarDefCommon>
    {
        std::unique_ptr<Type>       type_;
        std::string                 name_;
        std::unique_ptr<Expression> initializer_ {};
        VarDefCommon(std::unique_ptr<Type>, std::string);
        VarDefCommon(std::unique_ptr<Type>, std::string, std::unique_ptr<Expression>);
    };

    struct ParamDefinition : public Visitable<ParamDefinition>
    {
        VarDefCommon var_;
        ParamDefinition(std::unique_ptr<Type>, std::string);
        ParamDefinition(std::unique_ptr<Type>, std::string, std::unique_ptr<Expression>);
    };

    struct FieldDefinition : public Visitable<FieldDefinition>
    {
        VarDefCommon var_;
        FieldDefinition(std::unique_ptr<Type>, std::string);
        FieldDefinition(std::unique_ptr<Type>, std::string, std::unique_ptr<Expression>);
    };

// Expressions:

    struct IntLiteral : public VisitableFamily<Expression, IntLiteral>
    {
        std::int64_t num_;
        IntLiteral (std::int64_t);
    };

    struct FloatLiteral : public VisitableFamily<Expression, FloatLiteral>
    {
        double num_;
        FloatLiteral (double);
    };

    struct StringLiteral : public VisitableFamily<Expression, StringLiteral>
    {
        std::string str_;
        StringLiteral (std::string);
    };

    struct NullLiteral : public VisitableFamily<Expression, NullLiteral>
    {
    };

    struct BoolLiteral : public VisitableFamily<Expression, BoolLiteral>
    {
        bool val_;
        BoolLiteral (bool);
    };

    enum class BinOpcode
    {
        Add, Sub, Mul, Div, Mod, And, Or, LT, LE, GT, GE, EQ, NE,
        AddAssign, SubAssign, MulAssign, DivAssign, ModAssign,
        Assign,
        Unknown
    };

    enum class UnOpcode
    {
        IncPre, IncPost, DecPre, DecPost, LogNot, Deref, Address, ArNot,
        Sizeof,
        Unknown
    };

    struct BinaryOperator : public VisitableFamily<Expression, BinaryOperator>
    {
        BinOpcode                   op_;
        std::unique_ptr<Expression> lhs_;
        std::unique_ptr<Expression> rhs_;
        BinaryOperator (std::unique_ptr<Expression>, BinOpcode, std::unique_ptr<Expression>);
    };

    struct UnaryOperator : public VisitableFamily<Expression, UnaryOperator>
    {
        using arg_variant = std::variant<std::unique_ptr<Expression>, std::unique_ptr<Type>>;
        UnOpcode    op_;
        arg_variant arg_;
        UnaryOperator (UnOpcode, std::unique_ptr<Expression>);
        UnaryOperator (UnOpcode, std::unique_ptr<Type>);
    };

    struct Parenthesis : public VisitableFamily<Expression, Parenthesis>
    {
        std::unique_ptr<Expression> expression_;
        Parenthesis (std::unique_ptr<Expression>);
    };

    struct VarRef : public VisitableFamily<Expression, VarRef>
    {
        std::string name_;
        VarRef (std::string);
    };

    struct MemberVarRef : public VisitableFamily<Expression, MemberVarRef>
    {
        bool                        indirectBase_;
        std::unique_ptr<Expression> base_;
        std::string                 name_;
        MemberVarRef (std::unique_ptr<Expression>, std::string);
    };

    struct New : public VisitableFamily<Expression, New>
    {
        std::unique_ptr<Type>                    type_;
        std::vector<std::unique_ptr<Expression>> args_;
        New (std::unique_ptr<Type>, std::vector<std::unique_ptr<Expression>>);
    };

    struct FunctionCall : public VisitableFamily<Expression, FunctionCall>
    {
        std::string                              name_;
        std::vector<std::unique_ptr<Expression>> args_;
        FunctionCall (std::string, std::vector<std::unique_ptr<Expression>>);
    };

    struct ConstructorCall : public VisitableFamily<Expression, ConstructorCall>
    {
        std::unique_ptr<Type>                    type_;
        std::vector<std::unique_ptr<Expression>> args_;
        ConstructorCall (std::unique_ptr<Type>, std::vector<std::unique_ptr<Expression>>);
    };

    struct DestructorCall : public VisitableFamily<Expression, DestructorCall>
    {
        std::unique_ptr<Expression> ex_;
        DestructorCall (std::unique_ptr<Expression>);
    };

    struct MemberFunctionCall : public VisitableFamily<Expression, MemberFunctionCall>
    {
        bool                                     indirectBase_;
        std::unique_ptr<Expression>              base_;
        std::string                              call_;
        std::vector<std::unique_ptr<Expression>> args_;
        MemberFunctionCall (std::unique_ptr<Expression>, std::string, std::vector<std::unique_ptr<Expression>>);
    };

    struct ExpressionCall : public VisitableFamily<Expression, ExpressionCall>
    {
        std::unique_ptr<Expression>              ex_;
        std::vector<std::unique_ptr<Expression>> args_;
        ExpressionCall (std::unique_ptr<Expression>, std::vector<std::unique_ptr<Expression>>);
    };

    struct This : public VisitableFamily<Expression, This>
    {
    };

    struct IfExpression : public VisitableFamily<Expression, IfExpression>
    {
        std::unique_ptr<Expression> cond_;
        std::unique_ptr<Expression> then_;
        std::unique_ptr<Expression> else_;
        IfExpression (std::unique_ptr<Expression>, std::unique_ptr<Expression>, std::unique_ptr<Expression>);
    };

// Statements:

    struct Delete : public VisitableFamily<Statement, Delete>
    {
        std::unique_ptr<Expression> ex_;
        Delete (std::unique_ptr<Expression>);
    };

    struct VarDefinition : public VisitableFamily<Statement, VarDefinition>
    {
        VarDefCommon var_;
        VarDefinition(std::unique_ptr<Type>, std::string);
        VarDefinition(std::unique_ptr<Type>, std::string, std::unique_ptr<Expression>);
    };

    struct CompoundStatement : public VisitableFamily<Statement, CompoundStatement>
    {
        std::vector<std::unique_ptr<Statement>> statements_;
        CompoundStatement (std::unique_ptr<Statement>);
        CompoundStatement (std::vector<std::unique_ptr<Statement>>);
    };

    struct Return : public VisitableFamily<Statement, Return>
    {
        std::unique_ptr<Expression> expression_;
        Return (std::unique_ptr<Expression>);
    };

    struct If : public VisitableFamily<Statement, If>
    {
        std::unique_ptr<Expression> condition_;
        CompoundStatement then_;
        std::optional<CompoundStatement> else_ {};
        If (std::unique_ptr<Expression>, CompoundStatement);
        If (std::unique_ptr<Expression>, CompoundStatement, CompoundStatement);
    };

    struct ExpressionStatement : public VisitableFamily<Statement, ExpressionStatement>
    {
        std::unique_ptr<Expression> expression_;
        ExpressionStatement (std::unique_ptr<Expression>);
    };

    struct ForLoop : public VisitableFamily<Statement, ForLoop>
    {
        std::unique_ptr<Statement>  var_;
        std::unique_ptr<Expression> cond_;
        std::unique_ptr<Expression> inc_;
        CompoundStatement           body_;
        ForLoop (std::unique_ptr<Statement>, std::unique_ptr<Expression>, std::unique_ptr<Expression>, CompoundStatement);
    };

    struct CondLoop
    {
        std::unique_ptr<Expression> condition_;
        CompoundStatement body_;
    };

    struct WhileLoop : public VisitableFamily<Statement, WhileLoop>
    {
        CondLoop loop_;
        WhileLoop (std::unique_ptr<Expression>, CompoundStatement);
    };

    struct DoWhileLoop : public VisitableFamily<Statement, DoWhileLoop>
    {
        CondLoop loop_;
        DoWhileLoop (std::unique_ptr<Expression>, CompoundStatement);
    };

    struct Throw : public VisitableFamily<Statement, Throw>
    {
    };

// Lambda:

    struct Lambda : public VisitableFamily<Expression, Lambda>
    {
        std::vector<ParamDefinition> params_;
        CompoundStatement            body_;
        Lambda (std::vector<ParamDefinition>, CompoundStatement);
    };

// Other:

    struct Constructor : public Visitable<Constructor>
    {
        std::vector<ParamDefinition>     params_;
        std::optional<CompoundStatement> body_ {};
        Constructor( std::vector<ParamDefinition>
                   , std::optional<CompoundStatement> );
    };

    struct Destructor : public Visitable<Constructor>
    {
        std::optional<CompoundStatement> body_;
        Destructor(std::optional<CompoundStatement>);
    };

    struct Method : public Visitable<Method>
    {
        std::string                      name_;
        std::unique_ptr<Type>            retType_;
        std::vector<ParamDefinition>     params_;
        std::optional<CompoundStatement> body_ {};
        Method( std::string
              , std::unique_ptr<Type>
              , std::vector<ParamDefinition>
              , std::optional<CompoundStatement> );
    };

    auto is_pure_virtual (Method const&) -> bool;

    struct Class : public Visitable<Class>
    {
        std::string                  qualName_;
        std::string                  name_;
        std::vector<Constructor>     constructors_;
        std::optional<Destructor>    destructor_;
        std::vector<Method>          methods_;
        std::vector<FieldDefinition> fields_;
        std::vector<Class*>          bases_;

        Class (std::string qualName);
    };

    auto is_interface (Class const&) -> bool;

    /**
     *  @brief Code from a translation unit. Just classes for now.
     */
    class TranslationUnit
    {
    public:
        TranslationUnit (std::vector<std::unique_ptr<Class>> classes);
        auto get_classes () const -> std::vector<std::unique_ptr<Class>> const&;

    private:
        std::vector<std::unique_ptr<Class>> classes_;
    };

    /**
     *  @brief Visitor interface for code classes.
     */
    class CodeVisitor
    {
    public:
        virtual ~CodeVisitor () = default;

        virtual auto visit (IntLiteral const&)           -> void = 0;
        virtual auto visit (FloatLiteral const&)         -> void = 0;
        virtual auto visit (StringLiteral const&)        -> void = 0;
        virtual auto visit (NullLiteral const&)          -> void = 0;
        virtual auto visit (BoolLiteral const&)          -> void = 0;
        virtual auto visit (BinaryOperator const&)       -> void = 0;
        virtual auto visit (Parenthesis const&)          -> void = 0;
        virtual auto visit (VarRef const&)               -> void = 0;
        virtual auto visit (MemberVarRef const&)         -> void = 0;
        virtual auto visit (UnaryOperator const&)        -> void = 0;
        virtual auto visit (New const&)                  -> void = 0;
        virtual auto visit (FunctionCall const&)         -> void = 0;
        virtual auto visit (ConstructorCall const&)      -> void = 0;
        virtual auto visit (DestructorCall const&)       -> void = 0;
        virtual auto visit (MemberFunctionCall const&)   -> void = 0;
        virtual auto visit (ExpressionCall const&)       -> void = 0;
        virtual auto visit (This const&)                 -> void = 0;
        virtual auto visit (IfExpression const&)         -> void = 0;
        virtual auto visit (Lambda const&)               -> void = 0;

        virtual auto visit (PrimType const&)             -> void = 0;
        virtual auto visit (CustomType const&)           -> void = 0;
        virtual auto visit (Indirection const&)          -> void = 0;

        virtual auto visit (Class const&)                -> void = 0;
        virtual auto visit (Method const&)               -> void = 0;
        virtual auto visit (VarDefCommon const&)         -> void = 0;
        virtual auto visit (FieldDefinition const&)      -> void = 0;
        virtual auto visit (ParamDefinition const&)      -> void = 0;
        virtual auto visit (VarDefinition const&)        -> void = 0;
        virtual auto visit (ForLoop const&)              -> void = 0;
        virtual auto visit (WhileLoop const&)            -> void = 0;
        virtual auto visit (DoWhileLoop const&)          -> void = 0;
        virtual auto visit (CompoundStatement const&)    -> void = 0;
        virtual auto visit (ExpressionStatement const&)  -> void = 0;
        virtual auto visit (Return const&)               -> void = 0;
        virtual auto visit (If const&)                   -> void = 0;
        virtual auto visit (Delete const&)               -> void = 0;
        virtual auto visit (Throw const&)                -> void = 0;
    };

    template<class Derived>
    auto Visitable<Derived>::accept
        (CodeVisitor& visitor) const -> void
    {
        visitor.visit(static_cast<Derived const&>(*this));
    }

    template<class VirtualBase, class Derived>
    auto VisitableFamily<VirtualBase, Derived>::accept
        (CodeVisitor& visitor) const -> void
    {
        visitor.visit(static_cast<Derived const&>(*this));
    }
}

#endif