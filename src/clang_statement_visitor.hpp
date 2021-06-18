#ifndef FRI_CLANG_STATEMENT_VISITOR_HPP
#define FRI_CLANG_STATEMENT_VISITOR_HPP

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include "abstract_code.hpp"
#include "clang_expression_visitor.hpp"

#include <memory>

namespace fri
{
    class StatementVisitor : public clang::RecursiveASTVisitor<StatementVisitor>
    {
    public:
        auto release_statement () -> std::unique_ptr<Statement>;
        auto release_compound  () -> std::unique_ptr<CompoundStatement>;

        auto VisitCompoundStmt           (clang::CompoundStmt*)           -> bool;
        auto VisitVarDecl                (clang::VarDecl*)                -> bool;
        auto VisitReturnStmt             (clang::ReturnStmt*)             -> bool;
        auto VisitCompoundAssignOperator (clang::CompoundAssignOperator*) -> bool;

    private:
        std::unique_ptr<Statement>         statement_ {};
        std::unique_ptr<CompoundStatement> compound_  {};
        ExpressionVisitor                  expressioner_;
    };
}

#endif