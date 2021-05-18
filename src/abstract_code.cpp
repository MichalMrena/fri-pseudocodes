#include "abstract_code.hpp"

namespace fri
{
    auto CompoundStatement::accept
        (CodeVisitor& visitor) const -> void
    {
        visitor.visit(*this);
        for (auto const& s : statements_)
        {
            s->accept(visitor);
        }
        visitor.visit_post(*this);
    }

    auto ForLoop::accept
        (CodeVisitor& visitor) const -> void
    {
        visitor.visit(*this);
        body_.accept(visitor);
        visitor.visit_post(*this);
    }

    auto WhileLoop::accept
        (CodeVisitor& visitor) const -> void
    {
        visitor.visit(*this);
        body_.accept(visitor);
        visitor.visit_post(*this);
    }

    auto DoWhileLoop::accept
        (CodeVisitor& visitor) const -> void
    {
        visitor.visit(*this);
        body_.accept(visitor);
        visitor.visit_post(*this);
    }

    auto FieldDefinition::accept
        (CodeVisitor& visitor) const -> void
    {
        visitor.visit(*this);
        if (initializer_)
        {
            initializer_->accept(visitor);
        }
        visitor.visit_post(*this);
    }

    auto VariableDefinition::accept
        (CodeVisitor& visitor) const -> void
    {
        visitor.visit(*this);
        if (initializer_)
        {
            initializer_->accept(visitor);
        }
        visitor.visit_post(*this);
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

        for (auto const& m : methods_)
        {
            m.accept(visitor);
        }

        for (auto const& f : fields_)
        {
            f.accept(visitor);
        }

        visitor.visit_post(*this);
    }

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