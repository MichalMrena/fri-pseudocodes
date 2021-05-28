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
     *  @brief Visits definitions of classes in clang AST.
     */
    class ClassVisitor : public clang::RecursiveASTVisitor<ClassVisitor>
    {
    public:
        explicit ClassVisitor (clang::ASTContext& context, std::vector<std::unique_ptr<Class>>& classes);

        auto VisitCXXRecordDecl (clang::CXXRecordDecl* classDecl) -> bool;

    private:
        auto get_class (std::string const&) -> Class&;

    private:
        std::vector<std::unique_ptr<Class>>* classes_;
        clang::ASTContext*                   context_;
    };

    /**
     *  @brief Handles translation unit from clang.
     */
    class FindClassConsumer : public clang::ASTConsumer
    {
    public:
        explicit FindClassConsumer (clang::ASTContext& context, std::vector<std::unique_ptr<Class>>& classes);
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
        explicit FindClassAction (std::vector<std::unique_ptr<Class>>& classes);
        virtual auto CreateASTConsumer (clang::CompilerInstance& compiler, llvm::StringRef) -> std::unique_ptr<clang::ASTConsumer>;

    private:
        std::vector<std::unique_ptr<Class>>* classes_;
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
        else
        {
            return std::make_unique<ValueType>("Not implemented.");
        }
    }

// ClassVisitor definitions:

    ClassVisitor::ClassVisitor
        (clang::ASTContext& context, std::vector<std::unique_ptr<Class>>& classes) :
        classes_ (&classes),
        context_ (&context)
    {
    }

    auto ClassVisitor::VisitCXXRecordDecl
        (clang::CXXRecordDecl* classDecl) -> bool
    {
        auto& c = this->get_class(classDecl->getQualifiedNameAsString());
        c.name_ = classDecl->getNameAsString();

        for (auto const base : classDecl->bases())
        {
            auto const bt = base.getType();
            if (bt->isRecordType())
            {
                std::cout << "    base is record type" << '\n'; // TODO
            }
        }

        for (auto const field : classDecl->fields())
        {
            c.fields_.emplace_back();
            auto& f = c.fields_.back();
            f.var_.name_ = field->getNameAsString();
            f.var_.type_ = field->getType().getAsString(); // TODO
        }

        for (auto const method : classDecl->methods())
        {
            c.methods_.emplace_back();
            auto& m    = c.methods_.back();
            m.name_    = method->getNameAsString();
            m.retType_ = method->getReturnType().getAsString();
            for (auto i = 0u; i < method->getNumParams(); ++i)
            {
                auto const param = method->getParamDecl(i); // TODO
                m.params_.emplace_back();
                auto& p = m.params_.back();
                p.name_ = param->getNameAsString();
                p.type_ = param->getType().getAsString();
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

// FindClassConsumer definitions:

    FindClassConsumer::FindClassConsumer
        (clang::ASTContext& context, std::vector<std::unique_ptr<Class>>& classes) :
        visitor_ (context, classes)
    {
    }

    auto FindClassConsumer::HandleTranslationUnit
        (clang::ASTContext& context) -> void
    {
        visitor_.TraverseDecl(context.getTranslationUnitDecl());
    }

// FindClassAction definitions:

    FindClassAction::FindClassAction
        (std::vector<std::unique_ptr<Class>>& classes) :
        classes_ (&classes)
    {
    }

    auto FindClassAction::CreateASTConsumer
        (clang::CompilerInstance& compiler, llvm::StringRef) -> std::unique_ptr<clang::ASTConsumer>
    {
        return std::make_unique<FindClassConsumer>(compiler.getASTContext(), *classes_);
    }

// extract_code definition:

    auto extract_code
        (std::string const& code) -> TranslationUnit
    {
        auto cs = std::vector<std::unique_ptr<Class>>();
        clang::tooling::runToolOnCode(std::make_unique<FindClassAction>(cs), code);
        return TranslationUnit(std::move(cs));
    }
}