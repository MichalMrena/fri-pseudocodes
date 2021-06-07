#ifndef FRI_ABSTRACT_CODE_HPP
#define FRI_ABSTRACT_CODE_HPP

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <optional>

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
     *  @brief Implements accept using CRTP. And makes derived classe a @p VirtualBase .
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

// Expressions:

    struct IntLiteral : public VisitableFamily<Expression, IntLiteral>
    {
        std::int64_t num_;
        IntLiteral (std::int64_t);
    };

    struct FloatLiteral : public VisitableFamily<Expression, FloatLiteral>
    {
        double num_;
    };

    struct StringLiteral : public VisitableFamily<Expression, StringLiteral>
    {
        std::string str_;
        StringLiteral (std::string);
    };

    struct BinaryOperator : public VisitableFamily<Expression, BinaryOperator>
    {
        char                        op_;
        std::unique_ptr<Expression> lhs_;
        std::unique_ptr<Expression> rhs_;
    };

    struct Parenthesis : public VisitableFamily<Expression, Parenthesis>
    {
        std::unique_ptr<Expression> expression_;
        Parenthesis (std::unique_ptr<Expression>);
    };

// Other:

    struct VarDefCommon : public Visitable<VarDefCommon>
    {
        std::unique_ptr<Type>       type_;
        std::string                 name_;
        std::unique_ptr<Expression> initializer_ {};
    };

    struct ParamDefinition : public Visitable<ParamDefinition>
    {
        VarDefCommon var_;
    };

    struct FieldDefinition : public Visitable<FieldDefinition>
    {
        VarDefCommon var_;
    };

// Statements:

    struct VarDefinition : public VisitableFamily<Statement, VarDefinition>
    {
        VarDefCommon var_;
    };

    struct CompoundStatement : public VisitableFamily<Statement, CompoundStatement>
    {
        std::vector<std::unique_ptr<Statement>> statements_;
    };

    struct Return : public VisitableFamily<Statement, Return>
    {
        std::unique_ptr<Expression> expression_;
        Return (std::unique_ptr<Expression>);
    };

    struct ExpressionStatement : public VisitableFamily<Statement, ExpressionStatement>
    {
        std::unique_ptr<Expression> expression_;
        ExpressionStatement (std::unique_ptr<Expression>);
    };

    struct ForLoop : public VisitableFamily<Statement, ForLoop>
    {
        CompoundStatement body_;
    };

    struct WhileLoop : public VisitableFamily<Statement, WhileLoop>
    {
        CompoundStatement body_;
    };

    struct DoWhileLoop : public VisitableFamily<Statement, DoWhileLoop>
    {
        CompoundStatement body_;
    };

// Other:

    struct Method : public Visitable<Method>
    {
        std::string                      name_;
        std::unique_ptr<Type>            retType_;
        std::vector<ParamDefinition>     params_;
        std::optional<CompoundStatement> body_ {};
    };

    auto is_pure_virtual (Method const&) -> bool;

    struct Class : public Visitable<Class>
    {
        std::string                  qualName_;
        std::string                  name_;
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

        virtual auto visit (IntLiteral const&)          -> void = 0;
        virtual auto visit (FloatLiteral const&)        -> void = 0;
        virtual auto visit (StringLiteral const&)       -> void = 0;
        virtual auto visit (BinaryOperator const&)      -> void = 0;
        virtual auto visit (Parenthesis const&)         -> void = 0;

        virtual auto visit (PrimType const&)            -> void = 0;
        virtual auto visit (CustomType const&)          -> void = 0;
        virtual auto visit (Indirection const&)         -> void = 0;

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