#ifndef FRI_CLANG_STATEMENT_VISITOR_HPP
#define FRI_CLANG_STATEMENT_VISITOR_HPP

#include "clang/AST/RecursiveASTVisitor.h"
#include "abstract_code.hpp"
#include "clang_expression_visitor.hpp"
#include <memory>

namespace fri
{
    class StatementVisitor : public clang::RecursiveASTVisitor<StatementVisitor>
    {
    public:
        StatementVisitor (clang::ASTContext&);

        auto release_statement ()             -> std::unique_ptr<Statement>;
        auto release_compound  ()             -> std::unique_ptr<CompoundStatement>;
        auto read_statement    (clang::Stmt*) -> std::unique_ptr<Statement>;

        auto VisitCompoundStmt           (clang::CompoundStmt*)           -> bool;
        auto VisitVarDecl                (clang::VarDecl*)                -> bool;
        auto VisitReturnStmt             (clang::ReturnStmt*)             -> bool;
        auto VisitCompoundAssignOperator (clang::CompoundAssignOperator*) -> bool;
        auto VisitIfStmt                 (clang::IfStmt*)                 -> bool;
        auto VisitWhileStmt              (clang::WhileStmt*)              -> bool;
        auto VisitDoStmt                 (clang::DoStmt*)                 -> bool;
        auto VisitForStmt                (clang::ForStmt*)                -> bool;
        auto VisitUnaryOperator          (clang::UnaryOperator*)          -> bool;
        auto VisitCXXDeleteExpr          (clang::CXXDeleteExpr*)          -> bool;
        auto VisitCallExpr               (clang::CallExpr*)               -> bool;
        auto VisitBinaryOperator         (clang::BinaryOperator*)         -> bool;
        auto VisitCXXThrowExpr           (clang::CXXThrowExpr*)           -> bool;
        auto VisitSwitchStmt             (clang::SwitchStmt*)             -> bool;
        auto VisitBreakStmt              (clang::BreakStmt*)              -> bool;

    private:
        clang::ASTContext*                 context_;
        std::unique_ptr<Statement>         statement_ {};
        std::unique_ptr<CompoundStatement> compound_  {};
        ExpressionVisitor                  expressioner_;
    };
}

#endif