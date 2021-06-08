#include "code_generator.hpp"

#include <iostream>
#include <algorithm>

namespace fri
{
// Color definitions:

    auto operator== (Color const& l, Color const& r) -> bool
    {
        return l.r_ == r.r_
           and l.g_ == r.g_
           and l.b_ == r.b_;
    }

    auto operator!= (Color const& l, Color const& r) -> bool
    {
        return not (l == r);
    }

// ConsoleCodePrinter definitions:

    ConsoleCodePrinter::ConsoleCodePrinter
        () :
        indentStep_    (4),
        currentIndent_ (0),
        spaces_        ("                               ")
    {
    }

    ConsoleCodePrinter::~ConsoleCodePrinter
        ()
    {
        this->reset_color();
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

    auto ConsoleCodePrinter::out
        (std::string_view s) -> ConsoleCodePrinter&
    {
        std::cout << s;
        return *this;
    }

    auto ConsoleCodePrinter::out
        (std::string_view s, Color const& c) -> ConsoleCodePrinter&
    {
        this->set_color(c);
        std::cout << s;
        this->reset_color();
        return *this;
    }

    auto ConsoleCodePrinter::set_color
        (Color const& c) -> void
    {
        std::cout << ( Color {255, 0,   0}   == c ? "\x1B[91m"
                     : Color {0,   255, 0}   == c ? "\x1B[92m"
                     : Color {255, 255, 0}   == c ? "\x1B[93m"
                     : Color {0,   0,   255} == c ? "\x1B[94m"
                     : Color {0,   255, 255} == c ? "\x1B[96m"
                     : Color {255, 255, 255} == c ? "\x1B[97m"
                     :                              "\x1B[94m" );
    }

    auto ConsoleCodePrinter::reset_color
        () -> void
    {
        std::cout << "\x1B[0m";
    }

// PseudocodeGenerator definitions:

    PseudocodeGenerator::PseudocodeGenerator
        (ICodePrinter& out, CodeColorInfo const& colors) :
        out_    (&out),
        colors_ (&colors)
    {
    }

    auto PseudocodeGenerator::visit
        (IntLiteral const& i) -> void
    {
        out_->out(std::to_string(i.num_), colors_->plain_);
    }

    auto PseudocodeGenerator::visit
        (FloatLiteral const& f) -> void
    {
        out_->out(std::to_string(f.num_), colors_->plain_);
    }

    auto PseudocodeGenerator::visit
        (StringLiteral const& s) -> void
    {
        out_->out("\"", colors_->string_);
        out_->out(s.str_, colors_->string_);
        out_->out("\"", colors_->string_);
    }

    auto PseudocodeGenerator::visit
        (BinaryOperator const& b) -> void
    {
        b.lhs_->accept(*this);
        out_->out(" ").out(this->op_to_string(b.op_), colors_->plain_).out(" ");
        b.rhs_->accept(*this);
    }

    auto PseudocodeGenerator::visit
        (Parenthesis const& p) -> void
    {
        out_->out("(", colors_->plain_);
        p.expression_->accept(*this);
        out_->out(")", colors_->plain_);
    }

    auto PseudocodeGenerator::visit
        (VarRef const& r) -> void
    {
        out_->out(r.name_, colors_->variable_);
    }

    auto PseudocodeGenerator::visit
        (PrimType const& p) -> void
    {
        out_->out(p.name_, colors_->primType_);
    }

    auto PseudocodeGenerator::visit
        (CustomType const& c) -> void
    {
        out_->out(c.name_, colors_->customType_);
    }

    auto PseudocodeGenerator::visit
        (Indirection const& p) -> void
    {
        p.pointee_->accept(*this);
        out_->out("*", colors_->plain_);
    }

    auto PseudocodeGenerator::visit
        (Class const& c) -> void
    {
        // Class header.
        out_->begin_line();
        out_->out(is_interface(c) ? "Rozhranie " : "Trieda ", colors_->keyword_);
        out_->out(c.name_, colors_->customType_);

        // Split base classes to interfaces and classes.
        auto bases          = c.bases_;
        auto const basesEnd = std::end(bases);
        auto const basesMid = std::stable_partition(std::begin(bases), std::end(bases), [](auto const b)
        {
            return is_interface(*b);
        });

        // Print implemented interfaces.
        auto itImpl = std::begin(bases);
        if (itImpl != basesMid)
        {
            out_->out(" implementuje ", colors_->keyword_);
            while (itImpl != basesMid)
            {
                out_->out((*itImpl)->name_, colors_->customType_);
                ++itImpl;
                if ((itImpl != basesMid))
                {
                    out_->out(", ", colors_->plain_);
                }
            }
        }

        // Print base classes.
        auto itExt = basesMid;
        if (itExt != basesEnd)
        {
            out_->out(" rozširuje ", colors_->keyword_);
            while (itExt != basesEnd)
            {
                out_->out((*itExt)->name_, colors_->customType_);
                ++itExt;
                if (itExt != basesEnd)
                {
                    out_->out(", ", colors_->plain_);
                }
            }
        }

        out_->out(" {", colors_->plain_);
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
        out_->out("operácia ", colors_->keyword_);
        out_->out(m.name_, colors_->function_);
        out_->out("(", colors_->plain_);

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

        out_->out("): ", colors_->plain_);
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
        (VarDefCommon const& f) -> void
    {
        out_->out(f.name_, colors_->variable_);
        out_->out(": ", colors_->plain_);
        f.type_->accept(*this);
        if (f.initializer_)
        {
            out_->out(" <- ", colors_->plain_);
            f.initializer_->accept(*this);
        }
    }

    auto PseudocodeGenerator::visit
        (FieldDefinition const& f) -> void
    {
        out_->begin_line();
        out_->out("vlastnosť ", colors_->keyword_);
        f.var_.accept(*this);
        out_->end_line();
    }

    auto PseudocodeGenerator::visit
        (ParamDefinition const& p) -> void
    {
        p.var_.accept(*this);
    }

    auto PseudocodeGenerator::visit
        (VarDefinition const& v) -> void
    {
        out_->out("premenná ", colors_->keyword_);
        v.var_.accept(*this);
    }

    auto PseudocodeGenerator::visit
        (CompoundStatement const& ss) -> void
    {
        out_->out(" {", colors_->plain_);
        out_->end_line();
        out_->inc_indent();

        for (auto const& s : ss.statements_)
        {
            out_->begin_line();
            s->accept(*this);
            out_->end_line();
        }

        out_->dec_indent();
        out_->begin_line();
        out_->out("}", colors_->plain_);
        out_->end_line();
    }

    auto PseudocodeGenerator::visit
        (ExpressionStatement const& e) -> void
    {
        e.expression_->accept(*this);
    }

    auto PseudocodeGenerator::visit
        (Return const& r) -> void
    {
        out_->out("Vráť ", colors_->keyword_);
        r.expression_->accept(*this);
    }

    auto PseudocodeGenerator::op_to_string
        (BinOpcode op) const -> std::string
    {
        switch (op)
        {
            case BinOpcode::Add:     return "+";
            case BinOpcode::Sub:     return "-";
            case BinOpcode::Mul:     return "*";
            case BinOpcode::Div:     return "/";
            case BinOpcode::Mod:     return "mod";
            case BinOpcode::And:     return "a";
            case BinOpcode::Or:      return "alebo";
            case BinOpcode::LT:      return "<";
            case BinOpcode::LE:      return "<=";
            case BinOpcode::GT:      return ">";
            case BinOpcode::GE:      return ">=";
            case BinOpcode::EQ:      return "==";
            case BinOpcode::NE:      return "≠";
            case BinOpcode::Unknown: return "<unknown operator>";
            default:                 return "<unknown operator>";
        }
    }
}