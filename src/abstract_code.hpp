#ifndef FRI_ABSTRACT_CODE_HPP
#define FRI_ABSTRACT_CODE_HPP

#include <string>
#include <vector>
#include <memory>
#include <variant>

namespace fri
{
    class CodeVisitor;

    /**
     *  @brief Base class for expressions.
     */
    struct Expression
    {
        virtual ~Expression() = default;
        virtual auto accept (CodeVisitor& visitor) const -> void = 0;
    };

    /**
     *  @brief Base class for statements.
     */
    struct Statement
    {
        virtual ~Statement() = default;
        virtual auto accept (CodeVisitor& visitor) const -> void = 0;
    };

    /**
     *  @brief Compound statement (block of code).
     */
    struct CompoundStatement : public Statement
    {
        std::vector<std::unique_ptr<Statement>> statements_;

        auto accept (CodeVisitor& visitor) const -> void override;
    };

    /**
     *  @brief Base class for loops.
     */
    struct Loop : public Statement
    {
        CompoundStatement body_;
    };

    /**
     *  @brief For loop.
     */
    struct ForLoop : public Loop
    {
        auto accept (CodeVisitor& visitor) const -> void override;
    };

    /**
     *  @brief While loop.
     */
    struct WhileLoop : public Loop
    {
        auto accept (CodeVisitor& visitor) const -> void override;
    };

    /**
     *  @brief Do while loop.
     */
    struct DoWhileLoop : public Loop
    {
        auto accept (CodeVisitor& visitor) const -> void override;
    };

    /**
     *  @brief Base class for variable definitions.
     */
    struct VariableDefinitionBase : public Statement
    {
        std::string                 type_;
        std::string                 name_;
        std::unique_ptr<Expression> initializer_ {};
    };

    /**
     *  @brief Class field definition.
     */
    struct FieldDefinition : public VariableDefinitionBase
    {
        auto accept (CodeVisitor& visitor) const -> void override;
    };

    /**
     *  @brief Variable definition.
     */
    struct VariableDefinition : public VariableDefinitionBase
    {
        auto accept (CodeVisitor& visitor) const -> void override;
    };

    /**
     *  @brief Method definition.
     */
    struct Method
    {
        std::string                     name_;
        std::string                     retType_;
        std::vector<VariableDefinition> params_;
        CompoundStatement               body_;

        auto accept (CodeVisitor& visitor) const -> void;
    };

    /**
     *  @brief Class definition.
     */
    struct Class
    {
        std::string                  name_;
        std::vector<Method>          methods_;
        std::vector<FieldDefinition> fields_;

        auto accept (CodeVisitor& visitor) const -> void;
    };

    /**
     *  @brief Code from a translation unit. Just classes for now.
     */
    class TranslationUnit
    {
    public:
        TranslationUnit (std::vector<Class> classes);
        auto get_classes () const -> std::vector<Class> const&;

    private:
        std::vector<Class> classes_;
    };

    /**
     *  @brief Visitor interface for code classes.
     */
    class CodeVisitor
    {
    public:
        virtual ~CodeVisitor() = default;

        virtual auto visit (Class const& c)              -> void = 0;
        virtual auto visit (Method const& c)             -> void = 0;
        virtual auto visit (ForLoop const& c)            -> void = 0;
        virtual auto visit (WhileLoop const& c)          -> void = 0;
        virtual auto visit (DoWhileLoop const& c)        -> void = 0;
        virtual auto visit (FieldDefinition const& c)    -> void = 0;
        virtual auto visit (VariableDefinition const& c) -> void = 0;
        virtual auto visit (CompoundStatement const& c)  -> void = 0;

        virtual auto visit_post (Class const&)              -> void { };
        virtual auto visit_post (Method const&)             -> void { };
        virtual auto visit_post (ForLoop const&)            -> void { };
        virtual auto visit_post (WhileLoop const&)          -> void { };
        virtual auto visit_post (DoWhileLoop const&)        -> void { };
        virtual auto visit_post (FieldDefinition const&)    -> void { };
        virtual auto visit_post (VariableDefinition const&) -> void { };
        virtual auto visit_post (CompoundStatement const&)  -> void { };
    };
}

#endif