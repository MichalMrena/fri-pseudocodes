#include "clang_class_visitor.hpp"
#include "clang_utils.hpp"

    #include <iostream>

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

            // context_->getPrintingPolicy();
            // std::cout << classDecl->getQualifiedNameAsString() << " : ";
        for (auto const base : classDecl->bases())
        {
            auto const bt = base.getType();
            if (bt->isRecordType())
            {
                auto const baseDecl = bt->getAs<clang::RecordType>()->getAsCXXRecordDecl();
                c.bases_.emplace_back(&this->get_class(baseDecl->getQualifiedNameAsString()));
                    // std::cout << baseDecl->getQualifiedNameAsString() << " ";
            }
            else if (auto const tt = clang::dyn_cast<clang::TemplateSpecializationType>(bt.getTypePtr()))
            {
                c.bases_.emplace_back(&this->get_class(tt->getTemplateName().getAsTemplateDecl()->getQualifiedNameAsString()));
                    // std::cout << tt->getTemplateName().getAsTemplateDecl()->getQualifiedNameAsString() << " ";
            }
            else
            {
                    // std::cout << "<other> ";
            }

        }
            // std::cout << '\n';

        for (auto const field : classDecl->fields())
        {
            c.fields_.emplace_back( extract_type(context_->getPrintingPolicy(), field->getType())
                                  , field->getNameAsString() );
        }

        for (auto const methodPtr : classDecl->methods())
        {
            if (methodPtr->isImplicit())
            {
                continue;
            }

            // Optional body of Constructor, Destructor or Method.
            auto methodBody = [this, methodPtr]()
            {
                if (methodPtr->isPure())
                {
                    return std::optional<CompoundStatement> {};
                }

                auto const bodyPtr = methodPtr->getBody();
                if (not bodyPtr)
                {
                    return std::optional<CompoundStatement> {};
                }

                statementer_.TraverseStmt(bodyPtr);
                auto const compound = statementer_.release_compound();
                if (compound)
                {
                    return std::optional<CompoundStatement>(std::move(*compound));
                }

                return std::optional<CompoundStatement> {};
            }();

            // Destructor.
            if (clang::isa<clang::CXXDestructorDecl>(methodPtr))
            {
                c.destructor_ = Destructor(std::move(methodBody));
                continue;
            }

            // Parameters.
            auto params = [this, methodPtr]()
            {
                auto ps = std::vector<ParamDefinition>();
                for (auto i = 0u; i < methodPtr->getNumParams(); ++i)
                {
                    auto const param = methodPtr->getParamDecl(i);
                    // param->getInit(); // TODO
                    ps.emplace_back(extract_type(context_->getPrintingPolicy(), param->getType()), param->getNameAsString());
                }
                return ps;
            }();

            // Constructor.
            if (clang::isa<clang::CXXConstructorDecl>(methodPtr))
            {
                c.constructors_.emplace_back(std::move(params), std::move(methodBody));
                continue;
            }

            // Normal method.
            auto retType = extract_type(context_->getPrintingPolicy(), methodPtr->getReturnType());
            auto name = methodPtr->getNameAsString();
            c.methods_.emplace_back(std::move(name), std::move(retType), std::move(params), std::move(methodBody));
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