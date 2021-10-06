#ifndef FRI_CLANG_CLASS_VISITOR_HPP
#define FRI_CLANG_CLASS_VISITOR_HPP

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang_statement_visitor.hpp"
#include <memory>
#include <vector>

namespace fri
{
    class ClassVisitor : public clang::RecursiveASTVisitor<ClassVisitor>
    {
    public:
        explicit ClassVisitor ( clang::ASTContext& context
                              , std::vector<std::unique_ptr<Class>>& classes
                              , std::vector<std::string> const& namespaces );

        auto VisitCXXRecordDecl (clang::CXXRecordDecl*) -> bool;
        auto VisitTypeAliasTemplateDecl (clang::TypeAliasTemplateDecl*) -> bool;

    private:
        auto get_base_name (clang::Type const*) -> std::unique_ptr<Type>;
        auto get_class     (std::string const&) -> Class&;
        auto try_get_class (std::string_view)   -> Class*;
        auto should_visit  (std::string_view qalName) const -> bool;

    private:
        std::vector<std::unique_ptr<Class>>* classes_;
        std::vector<std::string> const*      namespaces_;
        clang::ASTContext*                   context_;
        StatementVisitor                     statementer_;
        ExpressionVisitor                    expressioner_;
    };
}

#endif