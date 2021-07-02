#ifndef FRI_CLANG_EXPRESSION_VISITOR_HPP
#define FRI_CLANG_EXPRESSION_VISITOR_HPP

#include "clang/AST/RecursiveASTVisitor.h"
#include "abstract_code.hpp"
#include <memory>

namespace fri
{
    class StatementVisitor;

    class ExpressionVisitor : public clang::RecursiveASTVisitor<ExpressionVisitor>
    {
    public:
        ExpressionVisitor (StatementVisitor&);

        auto read_expression (clang::Stmt*) -> std::unique_ptr<Expression>;

        auto VisitIntegerLiteral              (clang::IntegerLiteral*)              -> bool;
        auto VisitFloatingLiteral             (clang::FloatingLiteral*)             -> bool;
        auto VisitCXXBoolLiteralExpr          (clang::CXXBoolLiteralExpr*)          -> bool;
        auto VisitCXXNullPtrLiteralExpr       (clang::CXXNullPtrLiteralExpr*)       -> bool;
        auto VisitParenExpr                   (clang::ParenExpr*)                   -> bool;
        auto VisitBinaryOperator              (clang::BinaryOperator*)              -> bool;
        auto VisitDeclRefExpr                 (clang::DeclRefExpr*)                 -> bool;
        auto VisitCXXDependentScopeMemberExpr (clang::CXXDependentScopeMemberExpr*) -> bool;
        auto VisitMemberExpr                  (clang::MemberExpr*)                  -> bool;
        auto VisitCXXNewExpr                  (clang::CXXNewExpr*)                  -> bool;
        auto VisitUnaryOperator               (clang::UnaryOperator*)               -> bool;
        auto VisitCompoundAssignOperator      (clang::CompoundAssignOperator*)      -> bool;
        auto VisitUnaryExprOrTypeTraitExpr    (clang::UnaryExprOrTypeTraitExpr*)    -> bool;
        auto VisitCXXThisExpr                 (clang::CXXThisExpr*)                 -> bool;
        auto VisitCallExpr                    (clang::CallExpr*)                    -> bool;
        auto VisitConditionalOperator         (clang::ConditionalOperator*)         -> bool;
        auto VisitCXXUnresolvedConstructExpr  (clang::CXXUnresolvedConstructExpr*)  -> bool;
        auto VisitLambdaExpr                  (clang::LambdaExpr*)                  -> bool;

    private:
        std::unique_ptr<Expression> expression_;
        StatementVisitor*           statementer_;
    };
}

#endif