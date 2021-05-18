#include "abstract_code.hpp"

namespace fri
{
    auto CompoundStatement::accept
        (CodeVisitor& visitor) const -> void
    {
        visitor.visit(*this);
    }

    auto ForLoop::accept
        (CodeVisitor& visitor) const -> void
    {
        visitor.visit(*this);
    }

    auto WhileLoop::accept
        (CodeVisitor& visitor) const -> void
    {
        visitor.visit(*this);
    }

    auto DoWhileLoop::accept
        (CodeVisitor& visitor) const -> void
    {
        visitor.visit(*this);
    }

    auto VariableDefinition::accept
        (CodeVisitor& visitor) const -> void
    {
        visitor.visit(*this);
    }

    auto FieldDefinition::accept
        (CodeVisitor& visitor) const -> void
    {
        visitor.visit(*this);
    }

    auto Method::accept
        (CodeVisitor& visitor) const -> void
    {
        visitor.visit(*this);
    }

    auto Class::accept
        (CodeVisitor& visitor) const -> void
    {
        visitor.visit(*this);
    }

// TranslationUnit definitions:

    TranslationUnit::TranslationUnit
        (std::vector<Class> classes) :
        classes_ (std::move(classes))
    {
    }

    auto TranslationUnit::get_classes
        () const -> std::vector<Class> const&
    {
        return classes_;
    }
}