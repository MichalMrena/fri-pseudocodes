#include "cxx_source_parser.hpp"

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include <vector>
#include <iostream>

using namespace clang;

namespace fri
{
    /**
     *  @brief Visits definitions of classes in clang AST.
     */
    class ClassVisitor : public RecursiveASTVisitor<ClassVisitor>
    {
    public:
        explicit ClassVisitor (ASTContext& context, std::vector<Class>& classes);

        auto VisitCXXRecordDecl (CXXRecordDecl* classDecl) -> bool;

    private:
        std::vector<Class>* classes_;
        ASTContext*         context_;
    };

    /**
     *  @brief Handles translation unit from clang.
     */
    class FindClassConsumer : public clang::ASTConsumer
    {
    public:
        explicit FindClassConsumer (ASTContext& context, std::vector<Class>& classes);
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
        explicit FindClassAction (std::vector<Class>& classes);
        virtual auto CreateASTConsumer (clang::CompilerInstance& compiler, llvm::StringRef) -> std::unique_ptr<clang::ASTConsumer>;

    private:
        std::vector<Class>* classes_;
    };

// ClassVisitor definitions:

    ClassVisitor::ClassVisitor
        (ASTContext& context, std::vector<Class>& classes) :
        classes_ (&classes),
        context_ (&context)
    {
    }

    auto ClassVisitor::VisitCXXRecordDecl
        (CXXRecordDecl* classDecl) -> bool
    {
        classes_->emplace_back();
        auto& c = classes_->back();

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

// FindClassConsumer definitions:

    FindClassConsumer::FindClassConsumer
        (ASTContext& context, std::vector<Class>& classes) :
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
        (std::vector<Class>& classes) :
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
        auto cs = std::vector<Class>();
        clang::tooling::runToolOnCode(std::make_unique<FindClassAction>(cs), code);
        return TranslationUnit(std::move(cs));
    }
}