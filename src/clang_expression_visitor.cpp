#include "clang_expression_visitor.hpp"

namespace fri
{
    auto ExpressionVisitor::release_expression
        () -> std::unique_ptr<Expression>
    {
        return expression_ ? std::unique_ptr<Expression>(std::move(expression_))
                           : std::make_unique<StringLiteral>("<unknown expression>");
    }

    auto ExpressionVisitor::VisitIntegerLiteral
        (clang::IntegerLiteral* i) -> bool
    {
        expression_ = std::make_unique<IntLiteral>(i->getValue().getSExtValue());
        return false;
    }

    auto ExpressionVisitor::VisitParenExpr
        (clang::ParenExpr* p) -> bool
    {
        this->TraverseStmt(p->getSubExpr());
        expression_ = std::make_unique<Parenthesis>(this->release_expression());
        return false;
    }

    auto ExpressionVisitor::VisitBinaryOperator
        (clang::BinaryOperator* p) -> bool
    {
        // TODO
        return false;
    }
}