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
        auto result = std::make_unique<CompoundStatement>();

        for (auto const s : compound->body())
        {
            this->TraverseStmt(s);
            auto statement = this->release_statement();
            result->statements_.emplace_back(statement ? std::move(statement) : std::make_unique<ExpressionStatement>(std::make_unique<StringLiteral>("<unknown statement>")));
        }

        compound_ = std::move(result);
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
            expressioner_.TraverseStmt(init);
            def->var_.initializer_ = expressioner_.release_expression();
        }

        statement_ = std::move(def);
        return false;
    }

    auto StatementVisitor::VisitReturnStmt
        (clang::ReturnStmt* ret) -> bool
    {
        expressioner_.TraverseStmt(ret->getRetValue());
        statement_ = std::make_unique<Return>(expressioner_.release_expression());
        return false;
    }
}