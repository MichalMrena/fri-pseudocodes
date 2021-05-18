#include "code_generator.hpp"

#include <iostream>

namespace fri
{
// CodePrinter definitions:

    CodePrinter::CodePrinter
        (std::ostream& ost) :
        indentStep_    (4),
        currentIndent_ (0),
        ost_           (&ost),
        spaces_        ("                            ")
    {
    }

    auto CodePrinter::out
        () -> std::ostream&
    {
        return *ost_;
    }

    auto CodePrinter::inc_indent
        () -> void
    {

        ++currentIndent_;
    }

    auto CodePrinter::dec_indent
        () -> void
    {
         if (currentIndent_ > 0)
         {
             --currentIndent_;
         }
    }

    auto CodePrinter::begin_line
        () -> void
    {
        *ost_ << spaces_.substr(0, currentIndent_ * indentStep_);
    }

    auto CodePrinter::end_line
        () -> void
    {
        *ost_ << '\n';
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
        this->begin_line();
        this->out() << "Trieda " << c.name_;
        this->end_line();
        this->inc_indent();
    }

    auto PseudocodePrinter::visit_post
        (Class const&) -> void
    {
        this->dec_indent();
        this->begin_line();
        this->out() << "adeirt";
        this->end_line();
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
        (FieldDefinition const& f) -> void
    {
        this->begin_line();
        this->out() << "Atribút " << f.name_ << " : " << f.type_;
        this->end_line();
    }

    auto PseudocodePrinter::visit
        (CompoundStatement const&) -> void
    {

    }
}