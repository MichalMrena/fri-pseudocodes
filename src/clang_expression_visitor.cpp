#include "clang_expression_visitor.hpp"
#include "clang_statement_visitor.hpp"
#include "clang_utils.hpp"
    #include <iostream>

namespace fri
{
    ExpressionVisitor::ExpressionVisitor
        ( StatementVisitor&  s
        , clang::ASTContext& c ) :
        statementer_ (&s),
        context_     (&c)
    {
    }

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
        auto base   = r->isImplicitAccess() ? std::make_unique<This>() : this->read_expression(r->getBase());
        expression_ = std::make_unique<MemberVarRef>( std::move(base)
                                                    , r->getMemberNameInfo().getAsString() );
        return false;
    }

    auto ExpressionVisitor::VisitMemberExpr
        (clang::MemberExpr* const m) -> bool
    {
        auto base = m->isImplicitAccess() ? std::make_unique<This>() : this->read_expression(m->getBase());
        expression_ = std::make_unique<MemberVarRef>(std::move(base), m->getMemberNameInfo().getAsString());
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
            expression_ = std::make_unique<New>(extract_type(context_->getPrintingPolicy(), pt), std::move(argsVec));
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
            expression_ = std::make_unique<UnaryOperator>(UnOpcode::Sizeof, extract_type(context_->getPrintingPolicy(), uo->getArgumentType()));
        }
        else
        {
            expression_ = std::make_unique<UnaryOperator>(UnOpcode::Unknown, this->read_expression(uo->getArgumentExpr()));
        }
        return false;
    }

    auto ExpressionVisitor::VisitCXXNullPtrLiteralExpr
        (clang::CXXNullPtrLiteralExpr* const) -> bool
    {
        expression_ = std::make_unique<NullLiteral>();
        return false;
    }

    auto ExpressionVisitor::VisitCXXThisExpr
        (clang::CXXThisExpr* const) -> bool
    {
        expression_ = std::make_unique<This>();
        return false;
    }

    auto ExpressionVisitor::VisitCallExpr
        (clang::CallExpr* const c) -> bool
    {
        // Je tu trochu technický problém lebo void(...) funkcie nie sú po správnosti Expression.
        // Budeme sa ale tváriť, že namiesto void vracajú unit...

        // TODO to private static method
        auto const make_args = [this](auto&& as)
        {
            auto args = std::vector<std::unique_ptr<Expression>>();
            for (auto const arg : as)
            {
                args.emplace_back(this->read_expression(arg));
            }
            return args;
        };

        if (auto const o = clang::dyn_cast<clang::CXXOperatorCallExpr>(c))
        {
            if (o->getOperator() == clang::OverloadedOperatorKind::OO_Equal) // TODO is infix
            {
                auto lhs    = o->getNumArgs() > 0 ? this->read_expression(o->getArg(0)) : std::make_unique<VarRef>("<not good>");
                auto rhs    = o->getNumArgs() > 1 ? this->read_expression(o->getArg(1)) : std::make_unique<VarRef>("<not good>");
                expression_ = std::make_unique<BinaryOperator>(std::move(lhs), BinOpcode::Assign, std::move(rhs));
            }
            return false;
        }

        auto fc = c->child_begin();
        if (fc != c->child_end())
        {
            if (auto const d = clang::dyn_cast<clang::CXXPseudoDestructorExpr>(*fc))
            {
                expression_ = std::make_unique<DestructorCall>(this->read_expression(d->getBase()));
            }
            else if (auto const um = clang::dyn_cast<clang::UnresolvedMemberExpr>(*fc))
            {
                auto base   = um->isImplicitAccess() ? std::make_unique<This>() : this->read_expression(um->getBase());
                auto name   = um->getMemberNameInfo().getAsString();
                auto args   = make_args(c->arguments());
                expression_ = std::make_unique<MemberFunctionCall>(std::move(base), std::move(name), std::move(args));
            }
            else if (auto const m = clang::dyn_cast<clang::MemberExpr>(*fc))
            {
                auto base   = m->isImplicitAccess() ? std::make_unique<This>() : this->read_expression(m->getBase());
                auto name   = m->getMemberNameInfo().getAsString();
                auto args   = make_args(c->arguments());
                expression_ = std::make_unique<MemberFunctionCall>(std::move(base), std::move(name), std::move(args));
            }
            else if (auto const unr = clang::dyn_cast<clang::UnresolvedLookupExpr>(*fc))
            {
                auto name   = unr->getName().getAsString();
                expression_ = std::make_unique<FunctionCall>(std::move(name), make_args(c->arguments()));
            }
            else if (auto const dm = clang::dyn_cast<clang::CXXDependentScopeMemberExpr>(*fc))
            {
                auto base   = dm->isImplicitAccess() ? std::make_unique<This>() : this->read_expression(dm->getBase());
                auto name   = dm->getMemberNameInfo().getAsString();
                auto args   = make_args(c->arguments());
                expression_ = std::make_unique<MemberFunctionCall>(std::move(base), std::move(name), std::move(args));
            }
            else if (auto const v = clang::dyn_cast<clang::DeclRefExpr>(*fc)) // TODO zovšeobecniť na expression?
            {
                expression_ = std::make_unique<ExpressionCall>(this->read_expression(*fc), make_args(c->arguments()));
            }
            else
            {
                expression_ = std::make_unique<FunctionCall>("<unknown call type>", make_args(c->arguments()));
            }
        }

        return false;
    }

    auto ExpressionVisitor::VisitConditionalOperator
        (clang::ConditionalOperator* const c) -> bool
    {
        expression_ = std::make_unique<IfExpression>( this->read_expression(c->getCond())
                                                    , this->read_expression(c->getTrueExpr())
                                                    , this->read_expression(c->getFalseExpr()) );
        return false;
    }

    auto ExpressionVisitor::VisitCXXUnresolvedConstructExpr
        (clang::CXXUnresolvedConstructExpr* const c) -> bool
    {
        auto const make_args = [this](auto&& as)
        {
            auto args = std::vector<std::unique_ptr<Expression>>();
            for (auto const arg : as)
            {
                args.emplace_back(this->read_expression(arg));
            }
            return args;
        };
 
        expression_ = std::make_unique<ConstructorCall>( extract_type(context_->getPrintingPolicy(), c->getType())
                                                       , make_args(c->arguments()) );
        return false;
    }

    auto ExpressionVisitor::VisitLambdaExpr
        (clang::LambdaExpr* l) -> bool
    {
        statementer_->TraverseStmt(l->getBody());
        auto body = statementer_->release_compound();
        if (body)
        {
            auto params = std::vector<ParamDefinition>();
            for (auto const p : l->getCallOperator()->parameters())
            {
                params.emplace_back();
                auto& param = params.back();
                param.var_.name_ = p->getNameAsString();
                param.var_.type_ = extract_type(context_->getPrintingPolicy(), p->getType());
            }
            expression_ = std::make_unique<Lambda>( std::move(params)
                                                  , std::move(*body) );
        }
        return false;
    }
}