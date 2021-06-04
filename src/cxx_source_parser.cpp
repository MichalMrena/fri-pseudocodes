#include "cxx_source_parser.hpp"

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include <vector>
#include <unordered_map>
#include <iostream>

namespace fri
{
    /**
     *  @brief Visits statements in clang AST.
     */
    class StatementVisitor : public clang::RecursiveASTVisitor<StatementVisitor>
    {
    public:
        auto release_result () -> std::unique_ptr<Statement>;

        auto VisitCompoundStmt (clang::CompoundStmt*) -> bool;
        auto VisitVarDecl      (clang::VarDecl*)      -> bool;

    private:
        std::unique_ptr<Statement> result_ {};
    };

    /**
     *  @brief Visits definitions of classes in clang AST.
     */
    class ClassVisitor : public clang::RecursiveASTVisitor<ClassVisitor>
    {
    public:
        explicit ClassVisitor ( clang::ASTContext& context
                              , std::vector<std::unique_ptr<Class>>& classes
                              , std::vector<std::string> const& namespaces );

        auto VisitCXXRecordDecl (clang::CXXRecordDecl* classDecl) -> bool;

    private:
        auto get_class    (std::string const&) -> Class&;
        auto should_visit (std::string_view qalName) const -> bool;

    private:
        std::vector<std::unique_ptr<Class>>* classes_;
        std::vector<std::string> const*      namespaces_;
        clang::ASTContext*                   context_;
        StatementVisitor                     statementer_;
    };

    /**
     *  @brief Handles translation unit from clang.
     */
    class FindClassConsumer : public clang::ASTConsumer
    {
    public:
        explicit FindClassConsumer ( clang::ASTContext& context
                                   , std::vector<std::unique_ptr<Class>>& classes
                                   , std::vector<std::string> const& namespaces );
        virtual auto HandleTranslationUnit (clang::ASTContext& context) -> void;

    private:
        ClassVisitor visitor_;
    };

    /**
     *  @brief Action that introduces our visitors.
     */
    class FindClassAction : public clang::ASTFrontendAction
    {
    public:
        explicit FindClassAction (std::vector<std::unique_ptr<Class>>& classes, std::vector<std::string> const& namespaces);
        virtual auto CreateASTConsumer (clang::CompilerInstance& compiler, llvm::StringRef) -> std::unique_ptr<clang::ASTConsumer>;

    private:
        std::vector<std::unique_ptr<Class>>* classes_;
        std::vector<std::string> const*      namespaces_;
    };

// Utility functions:

    auto extract_type (clang::QualType qt) -> std::unique_ptr<Type>
    {
        auto const t = qt.getTypePtr();
        if (t->isPointerType())
        {
            auto const ptr = t->getAs<clang::PointerType>();
            return std::make_unique<Indirection>(extract_type(ptr->getPointeeType()));
        }
        else if (t->isReferenceType())
        {
            auto const ref = t->getAs<clang::ReferenceType>();
            return std::make_unique<Indirection>(extract_type(ref->getPointeeType()));
        }
        else if (t->isBuiltinType())
        {
            return std::make_unique<PrimType>(qt.getAsString());
        }
        else if (t->isRecordType())
        {
            auto const r = t->getAs<clang::RecordType>();
            return std::make_unique<CustomType>(r->getAsRecordDecl()->getName().str());
        }
        else if (t->isTemplateTypeParmType())
        {
            return std::make_unique<CustomType>(qt.getAsString());
        }
        else
        {
            return std::make_unique<PrimType>("<unknown type>");
        }
    }

// StatementVisitor definitions:

    auto StatementVisitor::release_result
        () -> std::unique_ptr<Statement>
    {
        return std::unique_ptr<Statement>(std::move(result_));
    }

    auto StatementVisitor::VisitCompoundStmt
        (clang::CompoundStmt* compound) -> bool
    {
        auto result = std::make_unique<CompoundStatement>();

        for (auto const s : compound->body())
        {
            this->TraverseStmt(s);
            result->statements_.emplace_back(this->release_result());
        }

        result_ = std::move(result);
        return false;
    }

    auto StatementVisitor::VisitVarDecl
        (clang::VarDecl* decl) -> bool
    {
        auto def = std::make_unique<VariableDefinition>();

        def->type_ = extract_type(decl->getType());
        def->name_ = decl->getName().str();
        auto const init = decl->getInit();
        if (init)
        {
            def->initializer_ = std::make_unique<StringLiteral>("<unknown expression>"); // TODO
        }

        result_ = std::move(def);
        return false;
    }

// ClassVisitor definitions:

    ClassVisitor::ClassVisitor
        ( clang::ASTContext& context
        , std::vector<std::unique_ptr<Class>>& classes
        , std::vector<std::string> const& namespaces ) :
        classes_    (&classes),
        namespaces_ (&namespaces),
        context_    (&context)
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
            auto& m    = c.methods_.emplace_back();
            m.name_    = method->getNameAsString();
            m.retType_ = extract_type(method->getReturnType());

            for (auto i = 0u; i < method->getNumParams(); ++i)
            {
                auto const param = method->getParamDecl(i);
                auto& p = m.params_.emplace_back();
                p.name_ = param->getNameAsString();
                p.type_ = extract_type(param->getType());
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
                    m.body_ = CompoundStatement();
                    // if is compound
                    method->hasBody();
                }
                else
                {
                    // TODO not good
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

// FindClassConsumer definitions:

    FindClassConsumer::FindClassConsumer
        ( clang::ASTContext& context
        , std::vector<std::unique_ptr<Class>>& classes
        , std::vector<std::string> const& namespaces ) :
        visitor_ (context, classes, namespaces)
    {
    }

    auto FindClassConsumer::HandleTranslationUnit
        (clang::ASTContext& context) -> void
    {
        visitor_.TraverseDecl(context.getTranslationUnitDecl());
    }

// FindClassAction definitions:

    FindClassAction::FindClassAction
        (std::vector<std::unique_ptr<Class>>& classes, std::vector<std::string> const& namespaces) :
        classes_    (&classes),
        namespaces_ (&namespaces)
    {
    }

    auto FindClassAction::CreateASTConsumer
        (clang::CompilerInstance& compiler, llvm::StringRef) -> std::unique_ptr<clang::ASTConsumer>
    {
        return std::make_unique<FindClassConsumer>(compiler.getASTContext(), *classes_, *namespaces_);
    }

// extract_code definition:

    auto extract_code
        (std::string const& code) -> TranslationUnit
    {
        auto cs   = std::vector<std::unique_ptr<Class>>();
        auto args = std::vector<std::string> {};
        auto ns   = std::vector<std::string> {"mm", "adt", "amt"};
        clang::tooling::runToolOnCodeWithArgs(std::make_unique<FindClassAction>(cs, ns), code, args);
        return TranslationUnit(std::move(cs));
    }
}