#ifndef FRI_ABSTRACT_CODE_HPP
#define FRI_ABSTRACT_CODE_HPP

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace fri
{
    class CodeVisitor;

    /**
     *  @brief Common base class for all expressions.
     */
    struct Expression
    {
        virtual ~Expression () = default;
        virtual auto accept (CodeVisitor& visitor) const -> void = 0;
    };

    /**
     *  @brief Common base class for all statements.
     */
    struct Statement
    {
        virtual ~Statement () = default;
        virtual auto accept (CodeVisitor& visitor) const -> void = 0;
    };

    /**
     *  @brief Base class for types.
     */
    struct Type
    {
        virtual ~Type () = default;
        virtual auto accept (CodeVisitor& visitor) const -> void = 0;
    };

    /**
     *  @brief Class template that implements accept for all derived classes.
     */
    template<class VirtualBase, class Derived>
    struct Visitable : public VirtualBase
    {
        auto accept (CodeVisitor& visitor) const -> void override;
    };

    /**
     *  @brief Primitive type or class.
     */
    struct ValueType : public Visitable<Type, ValueType>
    {
        std::string name_;

        ValueType (std::string name);
    };

    /**
     *  @brief Pointer or reference.
     */
    struct Indirection : public Visitable<Type, Indirection>
    {
        std::unique_ptr<Type> pointee_ {};

        Indirection (std::unique_ptr<Type> pointee);
    };

    /**
     *  @brief Integral literal.
     */
    struct IntLiteral : public Visitable<Expression, IntLiteral>
    {
        std::int64_t num_;
    };

    /**
     *  @brief Floating point literal.
     */
    struct FloatLiteral : public Visitable<Expression, FloatLiteral>
    {
        double num_;
    };

    /**
     *  @brief Binary operator.
     */
    struct BinaryOperator : public Visitable<Expression, BinaryOperator>
    {
        char                        op_;
        std::unique_ptr<Expression> lhs_;
        std::unique_ptr<Expression> rhs_;
    };

    /**
     *  @brief Compound statement (block of code).
     */
    struct CompoundStatement : public Visitable<Statement, CompoundStatement>
    {
        std::vector<std::unique_ptr<Statement>> statements_;
    };

    /**
     *  @brief For loop.
     */
    struct ForLoop : public Visitable<Statement, ForLoop>
    {
        CompoundStatement body_;
    };

    /**
     *  @brief While loop.
     */
    struct WhileLoop : public Visitable<Statement, WhileLoop>
    {
        CompoundStatement body_;
    };

    /**
     *  @brief Do while loop.
     */
    struct DoWhileLoop : public Visitable<Statement, DoWhileLoop>
    {
        CompoundStatement body_;
    };

    /**
     *  @brief Variable definition.
     */
    struct VariableDefinition : public Visitable<Statement, VariableDefinition>
    {
        std::string                 type_;
        std::string                 name_;
        std::unique_ptr<Expression> initializer_ {};
    };

    /**
     *  @brief Class field definition.
     */
    struct FieldDefinition : public Visitable<Statement, FieldDefinition>
    {
        VariableDefinition var_;
    };

    /**
     *  @brief Method definition.
     */
    struct Method : public Visitable<Statement, Method>
    {
        std::string                     name_;
        std::string                     retType_;
        std::vector<VariableDefinition> params_;
        CompoundStatement               body_;
    };

    auto is_pure_virtual (Method const&) -> bool;

    /**
     *  @brief Class definition.
     */
    struct Class : public Visitable<Statement, Class>
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

        virtual auto visit (IntLiteral const&)     -> void = 0;
        virtual auto visit (FloatLiteral const&)   -> void = 0;
        virtual auto visit (BinaryOperator const&) -> void = 0;

        virtual auto visit (ValueType const&)   -> void = 0;
        virtual auto visit (Indirection const&) -> void = 0;

        virtual auto visit (Class const&)              -> void = 0;
        virtual auto visit (Method const&)             -> void = 0;
        virtual auto visit (ForLoop const&)            -> void = 0;
        virtual auto visit (WhileLoop const&)          -> void = 0;
        virtual auto visit (DoWhileLoop const&)        -> void = 0;
        virtual auto visit (FieldDefinition const&)    -> void = 0;
        virtual auto visit (VariableDefinition const&) -> void = 0;
        virtual auto visit (CompoundStatement const&)  -> void = 0;
    };

    template<class VirtualBase, class Derived>
    auto Visitable<VirtualBase, Derived>::accept
        (CodeVisitor& visitor) const -> void
    {
        visitor.visit(static_cast<Derived const&>(*this));
    }
}

#endif