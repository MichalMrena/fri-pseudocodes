#include "clang_class_visitor.hpp"

#include "clang_utils.hpp"

namespace fri
{
    ClassVisitor::ClassVisitor
        ( clang::ASTContext& context
        , std::vector<std::unique_ptr<Class>>& classes
        , std::vector<std::string> const& namespaces ) :
        classes_     (&classes),
        namespaces_  (&namespaces),
        context_     (&context),
        statementer_ (context)
    {
    }

    auto ClassVisitor::VisitCXXRecordDecl
        (clang::CXXRecordDecl* classDecl) -> bool
    {
        auto const qualName = classDecl->getQualifiedNameAsString();
        if (not this->should_visit(qualName))
        {
            return true;
        }

        auto& c = this->get_class(qualName);
        c.name_ = classDecl->getNameAsString();

        for (auto const base : classDecl->bases())
        {
            auto const bt = base.getType();
            if (bt->isRecordType())
            {
                auto const baseDecl = bt->getAs<clang::RecordType>()->getAsCXXRecordDecl();
                c.bases_.emplace_back(&this->get_class(baseDecl->getQualifiedNameAsString()));
            }
        }

        for (auto const field : classDecl->fields())
        {
            auto& f = c.fields_.emplace_back();
            f.var_.name_ = field->getNameAsString();
            f.var_.type_ = extract_type(field->getType());
        }

        for (auto const method : classDecl->methods())
        {
            if (method->isImplicit() or method == classDecl->getDestructor())
            {
                continue;
            }

            auto& m    = c.methods_.emplace_back();
            m.name_    = method->getNameAsString();
            m.retType_ = extract_type(method->getReturnType());

            for (auto i = 0u; i < method->getNumParams(); ++i)
            {
                auto const param = method->getParamDecl(i);
                auto& p = m.params_.emplace_back();
                p.var_.name_ = param->getNameAsString();
                p.var_.type_ = extract_type(param->getType());
            }

            if (method->isPure())
            {
                m.body_ = std::nullopt;
            }
            else
            {
                auto const body = method->getBody();
                if (body)
                {
                    statementer_.TraverseStmt(body);
                    auto const compound = statementer_.release_compound();
                    if (compound)
                    {
                        m.body_ = std::move(*compound);
                    }
                }
            }
        }

        return true;
    }

    auto ClassVisitor::get_class
        (std::string const& name) -> Class&
    {
        auto const it = std::find_if(std::begin(*classes_), std::end(*classes_), [&name](auto const& c)
        {
            return c->qualName_ == name;
        });

        if (std::end(*classes_) != it)
        {
            return **it;
        }

        return *classes_->emplace_back(std::make_unique<Class>(name));
    }

    auto ClassVisitor::should_visit
        (std::string_view qualName) const -> bool
    {
        return std::any_of(std::begin(*namespaces_), std::end(*namespaces_), [qualName](auto const& n)
        {
            return qualName.starts_with(n);
        });
    }
}