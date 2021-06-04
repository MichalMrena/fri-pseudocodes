#include "code_generator.hpp"

#include <iostream>
#include <algorithm>

namespace fri
{
// ConsoleCodePrinter definitions:

    ConsoleCodePrinter::ConsoleCodePrinter
        () :
        indentStep_    (4),
        currentIndent_ (0),
        spaces_        ("                               ")
    {
    }

    auto ConsoleCodePrinter::inc_indent
        () -> void
    {

        ++currentIndent_;
    }

    auto ConsoleCodePrinter::dec_indent
        () -> void
    {
         if (currentIndent_ > 0)
         {
             --currentIndent_;
         }
    }

    auto ConsoleCodePrinter::begin_line
        () -> void
    {
        auto const sc = std::min(spaces_.size(), currentIndent_ * indentStep_);
        std::cout << spaces_.substr(0, sc);
    }

    auto ConsoleCodePrinter::end_line
        () -> void
    {
        std::cout << '\n';
    }

    auto ConsoleCodePrinter::blank_line
        () -> void
    {
        this->end_line();
    }

    auto ConsoleCodePrinter::set_color
        (Color const&) -> void
    {
        // TODO
    }

    auto ConsoleCodePrinter::out
        (std::string_view s) -> ConsoleCodePrinter&
    {
        std::cout << s;
        return *this;
    }

// PseudocodeGenerator definitions:

    PseudocodeGenerator::PseudocodeGenerator
        (ICodePrinter& out, CodeColorInfo const& colors) :
        out_    (&out),
        colors_ (&colors)
    {
    }

    auto PseudocodeGenerator::visit
        (IntLiteral const&) -> void
    {
    }

    auto PseudocodeGenerator::visit
        (FloatLiteral const&) -> void
    {
    }

    auto PseudocodeGenerator::visit
        (StringLiteral const& s) -> void
    {
        out_->out(s.str_);
    }

    auto PseudocodeGenerator::visit
        (BinaryOperator const&) -> void
    {
    }

    auto PseudocodeGenerator::visit
        (PrimType const& p) -> void
    {
        out_->set_color(colors_->primType_);
        out_->out(p.name_);
    }

    auto PseudocodeGenerator::visit
        (CustomType const& c) -> void
    {
        out_->set_color(colors_->customType_);
        out_->out(c.name_);
    }

    auto PseudocodeGenerator::visit
        (Indirection const& p) -> void
    {
        p.pointee_->accept(*this);
        out_->set_color(colors_->plain_);
        out_->out("*");
    }

    auto PseudocodeGenerator::visit
        (Class const& c) -> void
    {
        // Class header.
        out_->begin_line();
        out_->set_color(colors_->keyword_);
        out_->out("Trieda ").out(c.name_);

        if (not c.bases_.empty())
        {
            out_->set_color(colors_->keyword_);
            out_->out(" rozširuje ");
            out_->set_color(colors_->plain_);

            auto it = std::begin(c.bases_);
            auto const end = std::end(c.bases_);
            while (it != end)
            {
                out_->set_color(colors_->customType_);
                out_->out((*it)->name_);
                ++it;
                if (it != end)
                {
                    out_->set_color(colors_->plain_);
                    out_->out(", ");
                }
            }
        }

        out_->out(" {");
        out_->end_line();
        out_->inc_indent();

        // Visit methods.
        for (auto const& m : c.methods_)
        {
            m.accept(*this);
        }
        if (not c.methods_.empty())
        {
            out_->blank_line();
        }

        // Visit fields.
        for (auto const& f : c.fields_)
        {
            f.accept(*this);
        }

        // Class end.
        out_->dec_indent();
        out_->begin_line();
        out_->out("}");
        out_->end_line();
        out_->end_line();
    }

    auto PseudocodeGenerator::visit
        (Method const& m) -> void
    {
        out_->begin_line();
        out_->out("Operácia ").out(m.name_).out("(");

        auto it = std::begin(m.params_);
        auto const end = std::end(m.params_);
        while (it != end)
        {
            it->accept(*this);
            ++it;
            if (it != end)
            {
                out_->out(", ");
            }
        }

        out_->out("): ");
        m.retType_->accept(*this);

        if (m.body_.has_value())
        {
            m.body_.value().accept(*this);
        }
        out_->end_line();
    }

    auto PseudocodeGenerator::visit
        (ForLoop const&) -> void
    {

    }

    auto PseudocodeGenerator::visit
        (WhileLoop const&) -> void
    {

    }

    auto PseudocodeGenerator::visit
        (DoWhileLoop const&) -> void
    {
    }

    auto PseudocodeGenerator::visit
        (VariableDefinition const& f) -> void
    {
        out_->out(f.name_).out(": ");
        f.type_->accept(*this);
        if (f.initializer_)
        {
            out_->set_color(colors_->plain_);
            out_->out(" <- ");
            f.initializer_->accept(*this);
        }
    }

    auto PseudocodeGenerator::visit
        (FieldDefinition const& f) -> void
    {
        out_->begin_line();
        out_->set_color(colors_->keyword_);
        out_->out("Atribút ");
        f.var_.accept(*this);
        out_->end_line();
    }

    auto PseudocodeGenerator::visit
        (CompoundStatement const& ss) -> void
    {
        out_->set_color(colors_->plain_);
        out_->out(" {");
        out_->end_line();
        out_->inc_indent();

        for (auto const& s : ss.statements_)
        {
            out_->begin_line();
            s->accept(*this);
            out_->end_line();
        }

        out_->dec_indent();
        out_->set_color(colors_->plain_);
        out_->begin_line();
        out_->out("}");
        out_->end_line();
    }
}