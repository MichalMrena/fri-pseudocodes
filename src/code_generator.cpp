#include "code_generator.hpp"

#include <iostream>

namespace fri
{
// CodePrinter definitions:

    CodePrinter::CodePrinter
        (std::ostream& ost) :
        ost_ (&ost)
    {
    }

    auto CodePrinter::out
        () -> std::ostream&
    {
        return *ost_;
    }

// PseudocodePrinter definitions:

    PseudocodePrinter::PseudocodePrinter
        (std::ostream& ost) :
        CodePrinter(ost)
    {
    }

    auto PseudocodePrinter::visit
        (Class const& c) -> void
    {
        std::cout << "Found class: " << c.name_ << '\n';
    }

    auto PseudocodePrinter::visit
        (Method const&) -> void
    {

    }

    auto PseudocodePrinter::visit
        (ForLoop const&) -> void
    {

    }

    auto PseudocodePrinter::visit
        (WhileLoop const&) -> void
    {

    }

    auto PseudocodePrinter::visit
        (DoWhileLoop const&) -> void
    {

    }

    auto PseudocodePrinter::visit
        (CompoundStatement const&) -> void
    {

    }
}