#include "clang_expression_visitor.hpp"
#include "clang_utils.hpp"
#include <iostream>

namespace fri
{
    auto ExpressionVisitor::read_expression
        (clang::Stmt* const s) -> std::unique_ptr<Expression>
    {
        this->TraverseStmt(s);
        return expression_ ? std::move(expression_)
                           : std::make_unique<StringLiteral>("<unknown expression>");
    }

    auto ExpressionVisitor::VisitIntegerLiteral
        (clang::IntegerLiteral* const i) -> bool
    {
        expression_ = std::make_unique<IntLiteral>(i->getValue().getSExtValue());
        return false;
    }

    auto ExpressionVisitor::VisitFloatingLiteral
        (clang::FloatingLiteral* const f) -> bool
    {
        expression_ = std::make_unique<FloatLiteral>(f->getValue().convertToDouble());
        return false;
    }

    auto ExpressionVisitor::VisitCXXBoolLiteralExpr
        (clang::CXXBoolLiteralExpr* const b) -> bool
    {
        expression_ = std::make_unique<BoolLiteral>(b->getValue());
        return false;
    }

    auto ExpressionVisitor::VisitParenExpr
        (clang::ParenExpr* const p) -> bool
    {
        expression_ = std::make_unique<Parenthesis>(this->read_expression(p->getSubExpr()));
        return false;
    }

    auto ExpressionVisitor::VisitBinaryOperator
        (clang::BinaryOperator* const b) -> bool
    {
        auto lhs = this->read_expression(b->getLHS());
        auto rhs = this->read_expression(b->getRHS());
        auto const op = switch_bin_operator(b->getOpcode());
        expression_ = std::make_unique<BinaryOperator>(std::move(lhs), op, std::move(rhs));
        return false;
    }

    auto ExpressionVisitor::VisitDeclRefExpr
        (clang::DeclRefExpr* const r) -> bool
    {
        expression_ = std::make_unique<VarRef>(r->getNameInfo().getAsString());
        return false;
    }

    auto ExpressionVisitor::VisitCXXDependentScopeMemberExpr
        (clang::CXXDependentScopeMemberExpr* const r) -> bool
    {
        expression_ = std::make_unique<VarRef>(r->getMemberNameInfo().getAsString());
        return false;
    }

    auto ExpressionVisitor::VisitMemberExpr
        (clang::MemberExpr* const m) -> bool
    {
        expression_ = std::make_unique<VarRef>(m->getMemberNameInfo().getAsString());
        return false;
    }

    auto ExpressionVisitor::VisitCXXNewExpr
        (clang::CXXNewExpr* const n) -> bool
    {
        auto const tp = n->getType().getTypePtr();
        if (tp->isPointerType())
        {
            auto argsVec    = std::vector<std::unique_ptr<Expression>>();
            auto const pt   = tp->getAs<clang::PointerType>()->getPointeeType();

            for (auto const arg : n->children())
            {
                if (auto const list = clang::dyn_cast<clang::ParenListExpr>(arg))
                {
                    for (auto const c : list->children())
                    {
                        argsVec.emplace_back(this->read_expression(c));
                    }
                }
            }
            expression_ = std::make_unique<New>(extract_type(pt), std::move(argsVec));
        }
        // TODO placement new args

        return false;
    }

    auto ExpressionVisitor::VisitUnaryOperator
        (clang::UnaryOperator* const uo) -> bool
    {
        auto const op = switch_un_operator(uo->getOpcode());
        auto exp      = this->read_expression(uo->getSubExpr());
        expression_   = std::make_unique<UnaryOperator>(op, std::move(exp));
        return false;
    }

    auto ExpressionVisitor::VisitCompoundAssignOperator
        (clang::CompoundAssignOperator* const ca) -> bool
    {
        auto const op = switch_bin_operator(ca->getOpcode());
        auto lhs      = this->read_expression(ca->getLHS());
        auto rhs      = this->read_expression(ca->getRHS());
        expression_   = std::make_unique<BinaryOperator>(std::move(lhs), op, std::move(rhs));
        return false;
    }

    auto ExpressionVisitor::VisitUnaryExprOrTypeTraitExpr
        (clang::UnaryExprOrTypeTraitExpr* const uo) -> bool
    {
        if (uo->isArgumentType())
        {
            // TODO Zatiaľ neviem ako zistiť, že je to naozaj sizeof..., ale iné asi nepoužívame
            expression_ = std::make_unique<BuiltinUnaryOperator>(BuiltinUnOpcode::Sizeof, extract_type(uo->getArgumentType()));
        }
        else
        {
            expression_ = std::make_unique<BuiltinUnaryOperator>(BuiltinUnOpcode::Unknown, this->read_expression(uo->getArgumentExpr()));
        }
        return false;
    }

    auto ExpressionVisitor::VisitCXXNullPtrLiteralExpr
        (clang::CXXNullPtrLiteralExpr* const) -> bool
    {
        expression_ = std::make_unique<NullLiteral>();
        return false;
    }
}