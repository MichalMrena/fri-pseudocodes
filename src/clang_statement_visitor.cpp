#include "clang_statement_visitor.hpp"

#include "clang_utils.hpp"

namespace fri
{
    auto StatementVisitor::release_compound
        () -> std::unique_ptr<CompoundStatement>
    {
        return std::unique_ptr<CompoundStatement>(std::move(compound_));
    }

    auto StatementVisitor::release_statement
        () -> std::unique_ptr<Statement>
    {
        return std::unique_ptr<Statement>(std::move(statement_));
    }

    auto StatementVisitor::VisitCompoundStmt
        (clang::CompoundStmt* compound) -> bool
    {
        auto statements = std::vector<std::unique_ptr<Statement>>();

        for (auto const s : compound->body())
        {
            this->TraverseStmt(s);
            auto statement = this->release_statement();
            statements.emplace_back(statement ? std::move(statement) : std::make_unique<ExpressionStatement>(std::make_unique<StringLiteral>("<unknown statement>")));
        }

        compound_ = std::make_unique<CompoundStatement>(std::move(statements));
        return false;
    }

    auto StatementVisitor::VisitVarDecl
        (clang::VarDecl* decl) -> bool
    {
        auto def = std::make_unique<VarDefinition>();

        def->var_.type_ = extract_type(decl->getType());
        def->var_.name_ = decl->getName().str();
        auto const init = decl->getInit();
        if (init)
        {
            def->var_.initializer_ = expressioner_.read_expression(init);
        }

        statement_ = std::move(def);
        return false;
    }

    auto StatementVisitor::VisitReturnStmt
        (clang::ReturnStmt* ret) -> bool
    {
        statement_ = std::make_unique<Return>(expressioner_.read_expression(ret->getRetValue()));
        return false;
    }

    auto StatementVisitor::VisitCompoundAssignOperator
        (clang::CompoundAssignOperator* ca) -> bool
    {
        auto const op = switch_operator(ca->getOpcode());
        auto lhs      = expressioner_.read_expression(ca->getLHS());
        auto rhs      = expressioner_.read_expression(ca->getRHS());
        auto expr     = std::make_unique<BinaryOperator>(std::move(lhs), op, std::move(rhs));
        statement_    = std::make_unique<ExpressionStatement>(std::move(expr));
        return false;
    }

    auto StatementVisitor::VisitIfStmt
        (clang::IfStmt* ifs) -> bool
    {
        auto cond = expressioner_.read_expression(ifs->getCond());
        this->TraverseStmt(ifs->getThen());
        auto then = compound_ ? CompoundStatement(std::move(*this->release_compound()))
                              : CompoundStatement(this->release_statement());
        if (ifs->getElse())
        {
            this->TraverseStmt(ifs->getElse());
            auto els = compound_ ? CompoundStatement(std::move(*this->release_compound()))
                                 : CompoundStatement(this->release_statement());
            statement_ = std::make_unique<If>(std::move(cond), std::move(then), std::move(els));
        }
        else
        {
            statement_ = std::make_unique<If>(std::move(cond), std::move(then));
        }
        return false;
    }

    auto StatementVisitor::VisitWhileStmt
        (clang::WhileStmt* w) -> bool
    {
        auto c = expressioner_.read_expression(w->getCond());
        this->TraverseStmt(w->getBody());
        auto cb = this->release_compound();
        if (cb)
        {
            statement_ = std::make_unique<WhileLoop>(std::move(c), std::move(*cb));
        }
        else
        {
            auto b = this->release_statement();
            statement_ = std::make_unique<WhileLoop>(std::move(c), CompoundStatement(std::move(b)));
        }
        return false;
    }

    auto StatementVisitor::VisitDoStmt
        (clang::DoStmt* d) -> bool
    {
        auto c = expressioner_.read_expression(d->getCond());
        this->TraverseStmt(d->getBody());
        auto cb = this->release_compound();
        if (cb)
        {
            statement_ = std::make_unique<DoWhileLoop>(std::move(c), std::move(*cb));
        }
        else
        {
            auto b = this->release_statement();
            statement_ = std::make_unique<DoWhileLoop>(std::move(c), CompoundStatement(std::move(b)));
        }
        return false;
    }
}