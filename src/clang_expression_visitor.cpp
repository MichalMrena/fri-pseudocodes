#include "clang_expression_visitor.hpp"

#include "clang_utils.hpp"

#include <iostream>

namespace fri
{
    auto ExpressionVisitor::read_expression
        (clang::Stmt* s) -> std::unique_ptr<Expression>
    {
        this->TraverseStmt(s);
        return expression_ ? std::move(expression_)
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
        expression_ = std::make_unique<Parenthesis>(this->read_expression(p->getSubExpr()));
        return false;
    }

    auto ExpressionVisitor::VisitBinaryOperator
        (clang::BinaryOperator* b) -> bool
    {
        auto lhs = this->read_expression(b->getLHS());
        auto rhs = this->read_expression(b->getRHS());
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

    auto ExpressionVisitor::VisitCXXDependentScopeMemberExpr
        (clang::CXXDependentScopeMemberExpr* r) -> bool
    {
        expression_ = std::make_unique<VarRef>(r->getMemberNameInfo().getAsString());
        return false;
    }
}