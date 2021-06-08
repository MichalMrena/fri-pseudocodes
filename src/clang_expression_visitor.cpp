#include "clang_expression_visitor.hpp"

#include "clang_utils.hpp"

#include <iostream>

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

    auto ExpressionVisitor::VisitFloatingLiteral
        (clang::FloatingLiteral* f) -> bool
    {
        expression_ = std::make_unique<FloatLiteral>(f->getValue().convertToDouble());
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
        (clang::BinaryOperator* b) -> bool
    {
        this->TraverseStmt(b->getLHS());
        auto lhs = this->release_expression();
        this->TraverseStmt(b->getRHS());
        auto rhs = this->release_expression();
        auto const op = switch_operator(b->getOpcode());
        expression_ = std::make_unique<BinaryOperator>(std::move(lhs), op, std::move(rhs));
        return false;
    }

    auto ExpressionVisitor::VisitDeclRefExpr
        (clang::DeclRefExpr* r) -> bool
    {
        expression_ = std::make_unique<VarRef>(r->getNameInfo().getAsString());
        return false;
    }
}