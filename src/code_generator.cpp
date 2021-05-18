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

    auto CodePrinter::blank_line
        () -> void
    {
        this->end_line();
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
        // Class header.
        this->begin_line();
        this->out() << "Trieda " << c.name_;
        this->end_line();
        this->inc_indent();

        // Visit methods.
        for (auto const& m : c.methods_)
        {
            m.accept(*this);
        }
        if (not c.methods_.empty())
        {
            this->blank_line();
        }

        // Visit fields.
        for (auto const& f : c.fields_)
        {
            f.accept(*this);
        }

        // Class end.
        this->dec_indent();
        this->begin_line();
        this->out() << "adeirt";
        this->end_line();
    }

    auto PseudocodePrinter::visit
        (Method const& m) -> void
    {
        this->begin_line();
        this->out() << "Operácia " << m.name_ << "(";

        auto it = std::begin(m.params_);
        auto const end = std::end(m.params_);
        while (it != end)
        {
            it->accept(*this);
            ++it;
            if (it != end)
            {
                this->out() << ", ";
            }
        }

        this->out() << "): " << m.retType_;
        this->end_line();
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
        (VariableDefinition const& f) -> void
    {
        this->out() << f.name_ << ": " << f.type_;
        // TODO initializer if present
    }

    auto PseudocodePrinter::visit
        (FieldDefinition const& f) -> void
    {
        this->begin_line();
        this->out() << "Atribút ";
        f.var_.accept(*this);
        this->end_line();
    }

    auto PseudocodePrinter::visit
        (CompoundStatement const&) -> void
    {

    }
}