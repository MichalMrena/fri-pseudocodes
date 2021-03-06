#include "clang_statement_visitor.hpp"
#include "clang_utils.hpp"
    #include <iostream>

namespace fri
{
    StatementVisitor::StatementVisitor
        (clang::ASTContext& c) :
        context_      (&c),
        expressioner_ (*this, c)
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
        auto const init = decl->getInit();
        if (init)
        {
            statement_ = std::make_unique<VarDefinition>
                ( extract_type(context_->getPrintingPolicy(), decl->getType(), expressioner_)
                , decl->getName().str(), expressioner_.read_expression(init) );
        }
        else
        {
            statement_ = std::make_unique<VarDefinition>
                ( extract_type(context_->getPrintingPolicy(), decl->getType(), expressioner_)
                , decl->getName().str() );
        }
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
        statement_ = std::make_unique<ExpressionStatement>(expressioner_.read_expression(c));
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

    auto StatementVisitor::VisitSwitchStmt
        (clang::SwitchStmt* const s) -> bool
    {
        auto cond  = expressioner_.read_expression(s->getCond());
        auto cases = std::vector<Case>();
        auto def   = std::optional<CompoundStatement>{};

        auto swCit = s->child_begin();
        ++swCit; // cond
        if (swCit != s->child_end() and clang::isa<clang::CompoundStmt>(*swCit))
        {
            auto const end = (*swCit)->child_end();
            auto cit = (*swCit)->child_begin();
            while (cit != end)
            {
                if (auto const swCase = clang::dyn_cast<clang::CaseStmt>(*cit))
                {
                    auto caseExpr = swCase->child_begin() != swCase->child_end()
                        ? expressioner_.read_expression(*swCase->child_begin())
                        : std::unique_ptr<Expression>();

                    auto const subStmt = swCase->getSubStmt();
                    if (auto const compoundSub = clang::dyn_cast<clang::CompoundStmt>(subStmt))
                    {
                        this->VisitCompoundStmt(compoundSub);
                        cases.emplace_back(std::move(caseExpr), std::move(*this->release_compound()));
                        ++cit;
                    }
                    else if (subStmt)
                    {
                        auto caseBody = std::vector<uptr<Statement>>();
                        caseBody.emplace_back(this->read_statement(subStmt));
                        ++cit;

                        while (cit != end and not (clang::isa<clang::CaseStmt>(*cit) or clang::isa<clang::DefaultStmt>(*cit)))
                        {
                            if (not clang::isa<clang::BreakStmt>(*cit))
                            {
                                auto stmt = this->read_statement(*cit);
                                if (stmt)
                                {
                                    caseBody.emplace_back(std::move(stmt));
                                }
                            }
                            ++cit;
                        }
                        cases.emplace_back(std::move(caseExpr), std::move(caseBody));
                    }
                    else // TODO empty case, subStmt might asctually be next case?
                    {
                        cases.emplace_back(std::move(caseExpr), CompoundStatement(std::vector<uptr<Statement>>()));
                        while (cit != end)
                        {
                            ++cit;
                        }
                    }
                }
                else if (auto const swDefault = clang::dyn_cast<clang::DefaultStmt>(*cit))
                {
                    auto defaultBody = std::vector<uptr<Statement>>();
                    if (swDefault->getSubStmt())
                    {
                        defaultBody.emplace_back(this->read_statement(swDefault->getSubStmt()));
                    }
                    ++cit; // subStmt
                    while (cit != end and not (clang::isa<clang::CaseStmt>(*cit) or clang::isa<clang::DefaultStmt>(*cit)))
                    {
                        if (not clang::isa<clang::BreakStmt>(*cit))
                        {
                            auto stmt = this->read_statement(*cit);
                            if (stmt)
                            {
                                defaultBody.emplace_back(std::move(stmt));
                            }
                        }
                        ++cit;
                    }
                    def = std::optional<CompoundStatement>(std::move(defaultBody));
                }
                else
                {
                    std::cout << "## Unexpected statement in switch." << '\n';
                    (*cit)->dump();
                    ++cit;
                }
            }
        }

        statement_ = def
            ? std::make_unique<Switch>(std::move(cond), std::move(cases), std::move(*def))
            : std::make_unique<Switch>(std::move(cond), std::move(cases));

        // std::cout << "## Cond:" << '\n';
        // s->getCond();
        // for (auto const c : (*(++s->child_begin()))->children())
        // {
        //     std::cout << "## Child:" << '\n';
        //     c->dump();
        // }
        return false;
    }

    auto StatementVisitor::VisitBreakStmt
        (clang::BreakStmt* const) -> bool
    {
        statement_ = std::make_unique<Break>();
        return false;
    }
}