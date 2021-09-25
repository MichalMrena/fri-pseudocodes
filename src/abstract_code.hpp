#ifndef FRI_ABSTRACT_CODE_HPP
#define FRI_ABSTRACT_CODE_HPP

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <optional>
#include <variant>
#include <type_traits>

#include "types.hpp"
#include "utils.hpp"

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
        virtual auto to_string () const -> std::string = 0;
        virtual auto is_const  () const -> bool = 0;
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

    struct IsConst
    {
        bool is_;
        IsConst (bool);
        operator bool () const;
    };

    template<class Derived>
    struct CommonType : public VisitableFamily<Type, Derived>
    {
        bool isConst_;
        CommonType (IsConst);
        auto is_const () const -> bool override;
    };

    struct PrimType : public CommonType<PrimType>
    {
        std::string name_;
        PrimType (IsConst, std::string);
        auto to_string () const -> std::string override;
    };

    struct CustomType : public CommonType<CustomType>
    {
        std::string name_;
        CustomType (IsConst, std::string);
        auto to_string () const -> std::string override;
    };

    struct TemplatedType : public CommonType<TemplatedType>
    {
        using arg_var_t = std::variant<uptr<Type>, uptr<Expression>>;
        uptr<Type>             base_;
        std::vector<arg_var_t> args_;
        TemplatedType (IsConst, uptr<Type>, std::vector<arg_var_t>);
        auto to_string () const -> std::string override;
    };

    struct Indirection : public CommonType<Indirection>
    {
        uptr<Type> pointee_ {};
        Indirection (IsConst, uptr<Type>);
        auto to_string () const -> std::string override;
    };

    struct Function : public CommonType<Function>
    {
        std::vector<uptr<Type>> params_;
        uptr<Type>              ret_;
        Function (std::vector<uptr<Type>>, uptr<Type>);
        auto to_string () const -> std::string override;
    };

    struct Nested : public CommonType<Nested>
    {
        uptr<Type>  nest_;
        std::string name_;
        Nested(IsConst, uptr<Type>, std::string);
        auto to_string () const -> std::string override;
    };

// Other:

    struct VarDefCommon : public Visitable<VarDefCommon>
    {
        uptr<Type>       type_;
        std::string      name_;
        uptr<Expression> initializer_ {};
        VarDefCommon(uptr<Type>, std::string);
        VarDefCommon(uptr<Type>, std::string, uptr<Expression>);
    };

    struct ParamDefinition : public Visitable<ParamDefinition>
    {
        VarDefCommon var_;
        ParamDefinition(uptr<Type>, std::string);
        ParamDefinition(uptr<Type>, std::string, uptr<Expression>);
    };

    struct FieldDefinition : public Visitable<FieldDefinition>
    {
        VarDefCommon var_;
        FieldDefinition(uptr<Type>, std::string);
        FieldDefinition(uptr<Type>, std::string, uptr<Expression>);
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
        BinOpcode        op_;
        uptr<Expression> lhs_;
        uptr<Expression> rhs_;
        BinaryOperator (uptr<Expression>, BinOpcode, uptr<Expression>);
    };

    struct UnaryOperator : public VisitableFamily<Expression, UnaryOperator>
    {
        using arg_variant = std::variant<uptr<Expression>, uptr<Type>>;
        UnOpcode    op_;
        arg_variant arg_;
        UnaryOperator (UnOpcode, uptr<Expression>);
        UnaryOperator (UnOpcode, uptr<Type>);
    };

    struct Parenthesis : public VisitableFamily<Expression, Parenthesis>
    {
        uptr<Expression> expression_;
        Parenthesis (uptr<Expression>);
    };

    struct VarRef : public VisitableFamily<Expression, VarRef>
    {
        std::string name_;
        VarRef (std::string);
    };

    struct MemberVarRef : public VisitableFamily<Expression, MemberVarRef>
    {
        bool             indirectBase_;
        uptr<Expression> base_;
        std::string      name_;
        MemberVarRef (uptr<Expression>, std::string);
    };

    struct New : public VisitableFamily<Expression, New>
    {
        uptr<Type>                    type_;
        std::vector<uptr<Expression>> args_;
        New (uptr<Type>, std::vector<uptr<Expression>>);
    };

    struct FunctionCall : public VisitableFamily<Expression, FunctionCall>
    {
        std::string                   name_;
        std::vector<uptr<Expression>> args_;
        FunctionCall (std::string, std::vector<uptr<Expression>>);
    };

    struct ConstructorCall : public VisitableFamily<Expression, ConstructorCall>
    {
        uptr<Type>                    type_;
        std::vector<uptr<Expression>> args_;
        ConstructorCall (uptr<Type>, std::vector<uptr<Expression>>);
    };

    struct DestructorCall : public VisitableFamily<Expression, DestructorCall>
    {
        uptr<Expression> ex_;
        DestructorCall (uptr<Expression>);
    };

    struct MemberFunctionCall : public VisitableFamily<Expression, MemberFunctionCall>
    {
        bool                          indirectBase_;
        uptr<Expression>              base_;
        std::string                   call_;
        std::vector<uptr<Expression>> args_;
        MemberFunctionCall (uptr<Expression>, std::string, std::vector<uptr<Expression>>);
    };

    struct ExpressionCall : public VisitableFamily<Expression, ExpressionCall>
    {
        uptr<Expression>              ex_;
        std::vector<uptr<Expression>> args_;
        ExpressionCall (uptr<Expression>, std::vector<uptr<Expression>>);
    };

    struct This : public VisitableFamily<Expression, This>
    {
    };

    struct IfExpression : public VisitableFamily<Expression, IfExpression>
    {
        uptr<Expression> cond_;
        uptr<Expression> then_;
        uptr<Expression> else_;
        IfExpression (uptr<Expression>, uptr<Expression>, uptr<Expression>);
    };

// Statements:

    struct Delete : public VisitableFamily<Statement, Delete>
    {
        uptr<Expression> ex_;
        Delete (uptr<Expression>);
    };

    struct VarDefinition : public VisitableFamily<Statement, VarDefinition>
    {
        VarDefCommon var_;
        VarDefinition(uptr<Type>, std::string);
        VarDefinition(uptr<Type>, std::string, uptr<Expression>);
    };

    struct CompoundStatement : public VisitableFamily<Statement, CompoundStatement>
    {
        std::vector<uptr<Statement>> statements_;
        CompoundStatement (uptr<Statement>);
        CompoundStatement (std::vector<uptr<Statement>>);
    };

    struct Return : public VisitableFamily<Statement, Return>
    {
        uptr<Expression> expression_;
        Return (uptr<Expression>);
    };

    struct If : public VisitableFamily<Statement, If>
    {
        uptr<Expression> condition_;
        CompoundStatement then_;
        std::optional<CompoundStatement> else_ {};
        If (uptr<Expression>, CompoundStatement);
        If (uptr<Expression>, CompoundStatement, CompoundStatement);
    };

    struct ExpressionStatement : public VisitableFamily<Statement, ExpressionStatement>
    {
        uptr<Expression> expression_;
        ExpressionStatement (uptr<Expression>);
    };

    struct ForLoop : public VisitableFamily<Statement, ForLoop>
    {
        uptr<Statement>  var_;
        uptr<Expression> cond_;
        uptr<Expression> inc_;
        CompoundStatement           body_;
        ForLoop (uptr<Statement>, uptr<Expression>, uptr<Expression>, CompoundStatement);
    };

    struct CondLoop
    {
        uptr<Expression> condition_;
        CompoundStatement body_;
    };

    struct WhileLoop : public VisitableFamily<Statement, WhileLoop>
    {
        CondLoop loop_;
        WhileLoop (uptr<Expression>, CompoundStatement);
    };

    struct DoWhileLoop : public VisitableFamily<Statement, DoWhileLoop>
    {
        CondLoop loop_;
        DoWhileLoop (uptr<Expression>, CompoundStatement);
    };

    struct Break : public VisitableFamily<Statement, Break>
    {
    };

    struct Case : public VisitableFamily<Statement, Case>
    {
        uptr<Expression>  expr_;
        CompoundStatement body_;
        Case (uptr<Expression>, CompoundStatement);
    };

    struct Switch : public VisitableFamily<Statement, Switch>
    {
        uptr<Expression>                 cond_;
        std::vector<Case>                cases_;
        std::optional<CompoundStatement> default_;
        Switch ( uptr<Expression>
               , std::vector<Case> );
        Switch ( uptr<Expression>
               , std::vector<Case>
               , CompoundStatement );
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

    struct BaseInitPair
    {
        uptr<Type>                    base_;
        std::vector<uptr<Expression>> init_;
        BaseInitPair( uptr<Type>
                    , std::vector<uptr<Expression>> );
    };

    struct MemberInitPair
    {
        std::string                   name_;
        std::vector<uptr<Expression>> init_;
        MemberInitPair(std::string, std::vector<uptr<Expression>>);
    };

    struct Constructor : public Visitable<Constructor>
    {
        std::vector<ParamDefinition>     params_;
        std::vector<BaseInitPair>        baseInitList_;
        std::vector<MemberInitPair>      initList_;
        std::optional<CompoundStatement> body_ {};
        Constructor( std::vector<ParamDefinition>
                   , std::vector<BaseInitPair>
                   , std::vector<MemberInitPair>
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
        uptr<Type>            retType_;
        std::vector<ParamDefinition>     params_;
        std::optional<CompoundStatement> body_ {};
        Method( std::string
              , uptr<Type>
              , std::vector<ParamDefinition>
              , std::optional<CompoundStatement> );
    };

    struct Class : public Visitable<Class>
    {
        std::string                        qualName_;
        std::string                        name_;
        std::vector<std::string>           templateParams_;
        std::vector<Constructor>           constructors_;
        std::optional<Destructor>          destructor_;
        std::vector<Method>                methods_;
        std::vector<FieldDefinition>       fields_;
        std::vector<uptr<Type>> bases_;

        Class (std::string qualName);
        auto name () const -> std::string;
    };

    auto is_interface (Class const&) -> bool;
    auto is_interface (Type const&) -> bool;

    /**
     *  @brief Code from a translation unit. Just classes for now.
     */
    class TranslationUnit
    {
    public:
        TranslationUnit (std::vector<uptr<Class>> classes);
        auto get_classes () const -> std::vector<uptr<Class>> const&;

    private:
        std::vector<uptr<Class>> classes_;
    };

    /**
     *  @brief Visitor interface for code classes.
     */
    class CodeVisitor
    {
    public:
        virtual ~CodeVisitor () = default;

        virtual auto visit (IntLiteral const&)          -> void = 0;
        virtual auto visit (FloatLiteral const&)        -> void = 0;
        virtual auto visit (StringLiteral const&)       -> void = 0;
        virtual auto visit (NullLiteral const&)         -> void = 0;
        virtual auto visit (BoolLiteral const&)         -> void = 0;
        virtual auto visit (BinaryOperator const&)      -> void = 0;
        virtual auto visit (Parenthesis const&)         -> void = 0;
        virtual auto visit (VarRef const&)              -> void = 0;
        virtual auto visit (MemberVarRef const&)        -> void = 0;
        virtual auto visit (UnaryOperator const&)       -> void = 0;
        virtual auto visit (New const&)                 -> void = 0;
        virtual auto visit (FunctionCall const&)        -> void = 0;
        virtual auto visit (ConstructorCall const&)     -> void = 0;
        virtual auto visit (DestructorCall const&)      -> void = 0;
        virtual auto visit (MemberFunctionCall const&)  -> void = 0;
        virtual auto visit (ExpressionCall const&)      -> void = 0;
        virtual auto visit (This const&)                -> void = 0;
        virtual auto visit (IfExpression const&)        -> void = 0;
        virtual auto visit (Lambda const&)              -> void = 0;

        virtual auto visit (PrimType const&)            -> void = 0;
        virtual auto visit (CustomType const&)          -> void = 0;
        virtual auto visit (TemplatedType const&)       -> void = 0;
        virtual auto visit (Indirection const&)         -> void = 0;
        virtual auto visit (Function const&)            -> void = 0;
        virtual auto visit (Nested const&)              -> void = 0;

        virtual auto visit (Class const&)               -> void = 0;
        virtual auto visit (Method const&)              -> void = 0;
        virtual auto visit (VarDefCommon const&)        -> void = 0;
        virtual auto visit (FieldDefinition const&)     -> void = 0;
        virtual auto visit (ParamDefinition const&)     -> void = 0;
        virtual auto visit (VarDefinition const&)       -> void = 0;
        virtual auto visit (ForLoop const&)             -> void = 0;
        virtual auto visit (WhileLoop const&)           -> void = 0;
        virtual auto visit (DoWhileLoop const&)         -> void = 0;
        virtual auto visit (CompoundStatement const&)   -> void = 0;
        virtual auto visit (ExpressionStatement const&) -> void = 0;
        virtual auto visit (Return const&)              -> void = 0;
        virtual auto visit (If const&)                  -> void = 0;
        virtual auto visit (Delete const&)              -> void = 0;
        virtual auto visit (Throw const&)               -> void = 0;
        virtual auto visit (Break const&)               -> void = 0;
        virtual auto visit (Case const&)                -> void = 0;
        virtual auto visit (Switch const&)              -> void = 0;
    };

    /**
     *  @brief Adapter that implements methods as no-op.
     */
    class CodeVisitorAdapter : public CodeVisitor
    {
        auto visit (IntLiteral const&)          -> void override {};
        auto visit (FloatLiteral const&)        -> void override {};
        auto visit (StringLiteral const&)       -> void override {};
        auto visit (NullLiteral const&)         -> void override {};
        auto visit (BoolLiteral const&)         -> void override {};
        auto visit (BinaryOperator const&)      -> void override {};
        auto visit (Parenthesis const&)         -> void override {};
        auto visit (VarRef const&)              -> void override {};
        auto visit (MemberVarRef const&)        -> void override {};
        auto visit (UnaryOperator const&)       -> void override {};
        auto visit (New const&)                 -> void override {};
        auto visit (FunctionCall const&)        -> void override {};
        auto visit (ConstructorCall const&)     -> void override {};
        auto visit (DestructorCall const&)      -> void override {};
        auto visit (MemberFunctionCall const&)  -> void override {};
        auto visit (ExpressionCall const&)      -> void override {};
        auto visit (This const&)                -> void override {};
        auto visit (IfExpression const&)        -> void override {};
        auto visit (Lambda const&)              -> void override {};

        auto visit (PrimType const&)            -> void override {};
        auto visit (CustomType const&)          -> void override {};
        auto visit (TemplatedType const&)       -> void override {};
        auto visit (Indirection const&)         -> void override {};
        auto visit (Function const&)            -> void override {};
        auto visit (Nested const&)              -> void override {};

        auto visit (Class const&)               -> void override {};
        auto visit (Method const&)              -> void override {};
        auto visit (VarDefCommon const&)        -> void override {};
        auto visit (FieldDefinition const&)     -> void override {};
        auto visit (ParamDefinition const&)     -> void override {};
        auto visit (VarDefinition const&)       -> void override {};
        auto visit (ForLoop const&)             -> void override {};
        auto visit (WhileLoop const&)           -> void override {};
        auto visit (DoWhileLoop const&)         -> void override {};
        auto visit (CompoundStatement const&)   -> void override {};
        auto visit (ExpressionStatement const&) -> void override {};
        auto visit (Return const&)              -> void override {};
        auto visit (If const&)                  -> void override {};
        auto visit (Delete const&)              -> void override {};
        auto visit (Throw const&)               -> void override {};
        auto visit (Break const&)               -> void override {};
        auto visit (Case const&)                -> void override {};
        auto visit (Switch const&)              -> void override {};
    };

    /**
     *  @brief Checks if visitable type is T.
     */
    template<class T>
    class IsaVisitor : public CodeVisitorAdapter
    {
    public:
        auto visit  (T const&) -> void override;
        auto result () const   -> bool;

    private:
        bool result_ {false};
    };

    /**
     *  @brief Checks if U is T.
     */
    template<class T, class U>
    auto isa (U const& u) -> bool
    {
        auto v = IsaVisitor<T>();
        if constexpr (std::is_pointer_v<U> or is_smart_pointer_v<U>)
        {
            u->accept(v);
        }
        else
        {
            u.accept(v);
        }
        return v.result();
    }

    template<class VirtualBase, class Derived>
    auto VisitableFamily<VirtualBase, Derived>::accept
        (CodeVisitor& visitor) const -> void
    {
        visitor.visit(static_cast<Derived const&>(*this));
    }

    template<class Derived>
    auto Visitable<Derived>::accept
        (CodeVisitor& visitor) const -> void
    {
        visitor.visit(static_cast<Derived const&>(*this));
    }

    template<class T>
    auto IsaVisitor<T>::visit
        (T const&) -> void
    {
        result_ = true;
    }

    template<class T>
    auto IsaVisitor<T>::result
        () const -> bool
    {
        return result_;
    }
}

#endif