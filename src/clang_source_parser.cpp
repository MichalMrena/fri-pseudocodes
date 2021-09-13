#include "clang_source_parser.hpp"

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include "clang_expression_visitor.hpp"
#include "clang_statement_visitor.hpp"
#include "clang_class_visitor.hpp"

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <cassert>
#include <algorithm>

namespace fri
{
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
        auto args = std::vector<std::string> {"-O0", "-I/usr/local/lib/clang/14.0.0/include"};
        auto ns   = std::vector<std::string> {"mm", "adt", "amt"};
        clang::tooling::runToolOnCodeWithArgs(std::make_unique<FindClassAction>(cs, ns), code, args);
        return TranslationUnit(std::move(cs));
    }
}