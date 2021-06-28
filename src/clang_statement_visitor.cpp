#include "clang_statement_visitor.hpp"
#include "clang_utils.hpp"
#include <iostream>

namespace fri
{
    StatementVisitor::StatementVisitor
        (clang::ASTContext& c) :
        context_ (&c)
    {
    }

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

    auto StatementVisitor::read_statement
        (clang::Stmt* s) -> std::unique_ptr<Statement>
    {
        this->TraverseStmt(s);
        return this->release_statement();
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
        statement_ = std::make_unique<ExpressionStatement>(expressioner_.read_expression(ca));
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

    auto StatementVisitor::VisitForStmt
        (clang::ForStmt* f) -> bool
    {
        auto const varp  = f->getInit();
        auto const condp = f->getCond();
        auto const incp  = f->getInc();

        auto var   = not varp  ? std::unique_ptr<Statement>()  : this->read_statement(varp);
        auto cond  = not condp ? std::unique_ptr<Expression>() : expressioner_.read_expression(condp);
        auto inc   = not incp  ? std::unique_ptr<Expression>() : expressioner_.read_expression(incp);
        auto body  = [this, f]()
        {
            this->TraverseStmt(f->getBody());
            auto cb = this->release_compound();
            return cb ? std::move(cb) : [this]()
            {
                auto b = this->release_statement();
                return std::make_unique<CompoundStatement>(std::move(b));
            }();
        }();

        statement_ = std::make_unique<ForLoop>(std::move(var), std::move(cond), std::move(inc), std::move(*body));
        return false;
    }

    auto StatementVisitor::VisitUnaryOperator
        (clang::UnaryOperator* uo) -> bool
    {
        statement_ = std::make_unique<ExpressionStatement>(expressioner_.read_expression(uo));
        return false;
    }

    auto StatementVisitor::VisitCXXDeleteExpr
        (clang::CXXDeleteExpr* d) -> bool
    {
        statement_ = std::make_unique<Delete>(expressioner_.read_expression(d->getArgument()));
        return false;
    }

    auto StatementVisitor::VisitCallExpr
        (clang::CallExpr* c) -> bool
    {
        auto fc = c->child_begin();
        if (fc != c->child_end())
        {
            // TODO c->getCallReturnType(*context_).getTypePtr()->isVoidType()
            // by malo vracať void (e.g. free()) ale je tam <dependent typename>
            // v ideálnom prípade by sa pre void vracal ProcedureCall a inak
            // ExpressionStatement(FunctionCall), ale zatiaľ je teda všetko procedúra...

            auto name = std::string("");
            if (auto const unr = clang::dyn_cast<clang::UnresolvedLookupExpr>(*fc))
            {
                name = unr->getName().getAsString();
            }
            else if (auto const m = clang::dyn_cast<clang::MemberExpr>(*fc))
            {
                name = m->getMemberNameInfo().getAsString();
            }
            else
            {
                name = "<unknown call type>";
            }

            auto args = std::vector<std::unique_ptr<Expression>>();
            for (auto const arg : c->arguments())
            {
                args.emplace_back(expressioner_.read_expression(arg));
            }

            statement_ = std::make_unique<ProcedureCall>(std::move(name), std::move(args));
        }
        return false;
    }

    auto StatementVisitor::VisitBinaryOperator
        (clang::BinaryOperator* const op) -> bool
    {
        statement_ = std::make_unique<ExpressionStatement>(expressioner_.read_expression(op));
        return false;
    }

    auto StatementVisitor::VisitCXXThrowExpr
        (clang::CXXThrowExpr* const) -> bool
    {
        statement_ = std::make_unique<Throw>();
        return false;
    }
}