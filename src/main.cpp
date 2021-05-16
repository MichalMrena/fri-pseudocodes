#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"

#include "cxx_source_parser.hpp"
#include "code_generator.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace clang;

class FindNamedClassVisitor : public RecursiveASTVisitor<FindNamedClassVisitor>
{
public:
    explicit FindNamedClassVisitor(ASTContext* Context) :
        Context(Context)
    {
    }

    auto VisitCXXRecordDecl(CXXRecordDecl* Declaration) -> bool
    {
        std::cout << "Found class: " << Declaration->getQualifiedNameAsString() << '\n';

        std::cout << "Methods:" << '\n';
        for (auto m : Declaration->methods())
        {
            std::cout << "    " << m->getQualifiedNameAsString() << '\n';
            std::cout << "        " << m->getBody() << '\n';
        }

        std::cout << "Fields:" << '\n';
        for (auto f : Declaration->fields())
        {
            std::cout << "    " << f->getNameAsString() << '\n';
        }

        return true;
    }

    // auto VisitCXXMethodDecl(CXXMethodDecl* method) -> bool
    // {
    //     std::cout << "Found method: " << method->getQualifiedNameAsString() << '\n';
    //     return true;
    // }

    // auto VisitCompoundStmt(CompoundStmt* stmt) -> bool
    // {
    //     for (auto s : stmt->body())
    //     {

    //     }
    //     return true;
    // }

private:
    ASTContext* Context;
};

// class StatementVisitor : public RecursiveASTVisitor<StatementVisitor>
// {
//     auto VisitVarDecl(VarDecl* decl) -> bool
//     {
//         decl->getInit();
//         return true;
//     }
// };

class FindNamedClassConsumer : public clang::ASTConsumer
{
public:
    explicit FindNamedClassConsumer(ASTContext* Context)
        : Visitor(Context)
    {
    }

    virtual auto HandleTranslationUnit(clang::ASTContext& Context) -> void
    {
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }

private:
    FindNamedClassVisitor Visitor;
};

class FindNamedClassAction : public clang::ASTFrontendAction
{
public:
    virtual auto CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef InFile) -> std::unique_ptr<clang::ASTConsumer>
    {
        return std::make_unique<FindNamedClassConsumer>(&Compiler.getASTContext());
    }
};

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "File paths not provided." << '\n';
        return 1;
    }

    auto ifst = std::ifstream(argv[1]);
    if (not ifst.is_open())
    {
        std::cerr << "Failed to open input file: " << argv[1] << '\n';
        return 1;
    }

    auto ofst = std::ofstream(argv[2]);
    if (not ofst.is_open())
    {
        std::cerr << "Failed to open output file: " << argv[2] << '\n';
        return 1;
    }

    auto ist = std::stringstream();
    ist << ifst.rdbuf();
    auto const code = ist.str();

    // auto parser  = fri::CxxSourceParser();
    // auto printer = fri::PseudocodePrinter(ofst);
    // auto const abstractCode = parser.parse_code(code);

    // for (auto const& c : abstractCode.get_classes())
    // {
    //     c.accept(printer);
    // }

    clang::tooling::runToolOnCode(std::make_unique<FindNamedClassAction>(), code);
}