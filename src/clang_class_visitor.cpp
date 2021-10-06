#include "clang_class_visitor.hpp"
#include "clang_utils.hpp"

    #include <iostream>

namespace fri
{
    ClassVisitor::ClassVisitor
        ( clang::ASTContext& context
        , std::vector<std::unique_ptr<Class>>& classes
        , std::vector<std::string> const& namespaces ) :
        classes_      (&classes),
        namespaces_   (&namespaces),
        context_      (&context),
        statementer_  (context),
        expressioner_ (statementer_, context)
    {
    }

    auto ClassVisitor::VisitCXXRecordDecl
        (clang::CXXRecordDecl* classDecl) -> bool
    {
        auto const qualName = classDecl->getQualifiedNameAsString();
        // Only classes from our namespaces.
        if (not this->should_visit(qualName))
        {
            return true;
        }

        auto& c = this->get_class(qualName);
        c.name_ = classDecl->getNameAsString();

        // If it is a template, read all parameters.
        if (classDecl->isTemplated())
        {
            auto temDecl = classDecl->getDescribedTemplate();
            if (temDecl)
            {
                for (auto param : *temDecl->getTemplateParameters())
                {
                    c.templateParams_.emplace_back(param->getNameAsString());
                }
            }
        }

        // Read all base classes.
        for (auto const base : classDecl->bases())
        {
            auto const bt = base.getType();
            // if (bt->isRecordType())
            // {
            //     auto const baseDecl = bt->getAs<clang::RecordType>()->getAsCXXRecordDecl();
            //     c.bases_.emplace_back(&this->get_class(baseDecl->getQualifiedNameAsString()));
            // }
            // else if (auto const tt = clang::dyn_cast<clang::TemplateSpecializationType>(bt.getTypePtr()))
            // {
            //     c.bases_.emplace_back(&this->get_class(tt->getTemplateName().getAsTemplateDecl()->getQualifiedNameAsString()));
            // }
            c.bases_.emplace_back(extract_type(context_->getPrintingPolicy(), bt, expressioner_));
        }

        // Read all member variables.
        for (auto const field : classDecl->fields())
        {
            auto const init = field->getInClassInitializer();
            auto type = extract_type(context_->getPrintingPolicy(), field->getType(), expressioner_);
            if (init)
            {
                c.fields_.emplace_back( std::move(type)
                                      , field->getNameAsString()
                                      , expressioner_.read_expression(init) );
            }
            else
            {
                c.fields_.emplace_back( std::move(type)
                                      , field->getNameAsString() );
            }
        }

        // Read all methods.
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
                    ps.emplace_back(extract_type(context_->getPrintingPolicy(), param->getType(), expressioner_), param->getNameAsString());
                }
                return ps;
            }();

            // Constructor.
            if (auto const conDecl = clang::dyn_cast<clang::CXXConstructorDecl>(methodPtr))
            {
                auto con = clang::dyn_cast<clang::CXXConstructorDecl>(conDecl->getDefinition());
                if (not con)
                {
                    continue;
                }
                auto baseInitList = std::vector<BaseInitPair>();
                auto initList = std::vector<MemberInitPair>();
                for (auto const& init : con->inits())
                {
                    auto exprs = expressioner_.read_expressions(init->getInit());
                    if (init->isMemberInitializer())
                    {
                        auto name = init->getMember()->getNameAsString();
                        initList.emplace_back( std::move(name)
                                             , std::move(exprs) );
                    }
                    else if (init->isBaseInitializer())
                    {
                        auto baseType = init->getBaseClass();
                        auto name = this->get_base_name(baseType);
                        baseInitList.emplace_back( std::move(name)
                                                 , std::move(exprs) );
                    }
                }

                c.constructors_.emplace_back( std::move(params)
                                            , std::move(baseInitList)
                                            , std::move(initList)
                                            , std::move(methodBody) );
                continue;
            }

            // Normal method.
            auto retType = extract_type(context_->getPrintingPolicy(), methodPtr->getReturnType(), expressioner_);
            auto name = methodPtr->getNameAsString();
            c.methods_.emplace_back(std::move(name), std::move(retType), std::move(params), std::move(methodBody));
        }

        return true;
    }

    auto ClassVisitor::VisitTypeAliasTemplateDecl
        (clang::TypeAliasTemplateDecl* aliasDeclTemp) -> bool
    {
        auto const aliasDecl    = aliasDeclTemp->getTemplatedDecl();
        auto const aliasName    = aliasDecl->getNameAsString();
        auto const typePtr      = aliasDecl->getUnderlyingType().getTypePtr();
        auto const originalName = [typePtr]()
        {
            if (auto const tst = clang::dyn_cast<clang::TemplateSpecializationType>(typePtr))
            {
                auto const temDecl = tst->getTemplateName().getAsTemplateDecl();
                return temDecl ? temDecl->getNameAsString() : "<some template>";
            }
            return std::string("<unknown type>");
        }();
        auto const c = this->try_get_class(originalName);
        if (c)
        {
            c->alias_ = aliasName;
        }
        return true;
    }

    auto ClassVisitor::get_base_name
        (clang::Type const* t) -> std::unique_ptr<Type>
    {
        if (auto icn = clang::dyn_cast<clang::InjectedClassNameType>(t))
        {
            return this->get_base_name(icn->getInjectedTST());
        }
        else if (auto tst = clang::dyn_cast<clang::TemplateSpecializationType>(t))
        {
            using variant_t = TemplatedType::arg_var_t;
            auto temDecl = tst->getTemplateName().getAsTemplateDecl();
            auto name = temDecl ? temDecl->getNameAsString() : "<some template>";
            auto args = std::vector<variant_t>();
            auto const argc = tst->getNumArgs();
            for (auto i = 0u; i < argc; ++i)
            {
                auto const arg = tst->getArg(i);
                args.emplace_back(extract_type(context_->getPrintingPolicy(), arg.getAsType(), expressioner_));
            }
            return std::make_unique<TemplatedType>( IsConst(false)
                                                  , std::make_unique<CustomType>(IsConst(false), name)
                                                  , std::move(args) );
        }
        else
        {
                // t->dump();
            return std::make_unique<CustomType>(IsConst(false), "base");
        }
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

    auto ClassVisitor::try_get_class
        (std::string_view const name) -> Class*
    {
        auto const it = std::find_if(std::begin(*classes_), std::end(*classes_), [name](auto const& c)
        {
            return c->qualName_.ends_with(name);
        });
        return it != std::end(*classes_) ? it->get() : nullptr;
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