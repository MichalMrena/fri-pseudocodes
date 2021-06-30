#include "code_generator.hpp"

#include <iostream>
#include <algorithm>
#include <type_traits>
#include <cctype>

namespace fri
{
// Utils:

    namespace
    {
        template<class T>
        struct is_smart_pointer : public std::false_type
        {
        };

        template<class T>
        struct is_smart_pointer<std::unique_ptr<T>> : public std::true_type
        {
        };

        template<class T>
        struct is_smart_pointer<std::shared_ptr<T>> : public std::true_type
        {
        };

        template<class T>
        auto constexpr is_smart_pointer_v = is_smart_pointer<T>::value;
    }

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
                     : Color {255, 0,   255} == c ? "\x1B[95m"
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
        (ICodePrinter& out, CodeColorInfo colors) :
        out_    (&out),
        colors_ (std::move(colors))
    {
    }

    auto PseudocodeGenerator::visit
        (IntLiteral const& i) -> void
    {
        out_->out(std::to_string(i.num_), colors_.plain_);
    }

    auto PseudocodeGenerator::visit
        (FloatLiteral const& f) -> void
    {
        out_->out(std::to_string(f.num_), colors_.plain_);
    }

    auto PseudocodeGenerator::visit
        (StringLiteral const& s) -> void
    {
        out_->out("\"", colors_.string_);
        out_->out(s.str_, colors_.string_);
        out_->out("\"", colors_.string_);
    }

    auto PseudocodeGenerator::visit
        (NullLiteral const&) -> void
    {
        out_->out("NULL", colors_.valLiteral_);
    }

    auto PseudocodeGenerator::visit
        (BoolLiteral const& b) -> void
    {
        out_->out(b.val_ ? "pravda" : "nepravda", colors_.valLiteral_);
    }

    auto PseudocodeGenerator::visit
        (BinaryOperator const& b) -> void
    {
        auto const opstr = bin_op_to_string(b.op_);
        auto const color = this->op_color(opstr);
        if (is_compound_op(b.op_))
        {
            b.lhs_->accept(*this);
            out_->out(" <- ", colors_.plain_);
            b.lhs_->accept(*this);
            out_->out(" ").out(opstr, color).out(" ");
            b.rhs_->accept(*this);
        }
        else
        {
            b.lhs_->accept(*this);
            out_->out(" ").out(opstr, color).out(" ");
            b.rhs_->accept(*this);
        }
    }

    auto PseudocodeGenerator::visit
        (Parenthesis const& p) -> void
    {
        out_->out("(", colors_.plain_);
        p.expression_->accept(*this);
        out_->out(")", colors_.plain_);
    }

    auto PseudocodeGenerator::visit
        (VarRef const& r) -> void
    {
        out_->out(r.name_, colors_.variable_);
    }

    auto PseudocodeGenerator::visit
        (MemberVarRef const& m) -> void
    {
        m.base_->accept(*this);
        out_->out(".", colors_.plain_);
        out_->out(m.name_, colors_.variable_);
    }

    auto PseudocodeGenerator::visit
        (UnaryOperator const& r) -> void
    {
        auto const opstr = un_op_to_string(r.op_);
        auto const color = this->op_color(opstr);
        if (is_postfixx(r.op_))
        {
            if (0 == r.arg_.index())
            {
                std::get<0>(r.arg_)->accept(*this);
            }
            else if (1 == r.arg_.index())
            {
                std::get<1>(r.arg_)->accept(*this);
            }
            out_->out(opstr, color);
        }
        else
        {
            out_->out(opstr, color);
            if (0 == r.arg_.index())
            {
                std::get<0>(r.arg_)->accept(*this);
            }
            else if (1 == r.arg_.index())
            {
                std::get<1>(r.arg_)->accept(*this);
            }
        }
    }

    auto PseudocodeGenerator::visit
        (New const& n) -> void
    {
        out_->out("vytvor ", colors_.keyword_);
        n.type_->accept(*this);
        out_->out("(", colors_.plain_);
        this->visit_range(", ", std::begin(n.args_), std::end(n.args_));
        out_->out(")", colors_.plain_);
    }

    auto PseudocodeGenerator::visit
        (FunctionCall const& c) -> void
    {
        out_->out(c.name_, colors_.function_);
        out_->out("(", colors_.plain_);
        this->visit_range(", ", std::begin(c.args_), std::end(c.args_));
        out_->out(")", colors_.plain_);
    }

    auto PseudocodeGenerator::visit
        (This const&) -> void
    {
        out_->out("self", colors_.keyword_);
    }

    auto PseudocodeGenerator::visit
        (PrimType const& p) -> void
    {
        out_->out(p.name_, colors_.primType_);
    }

    auto PseudocodeGenerator::visit
        (CustomType const& c) -> void
    {
        out_->out(c.name_, colors_.customType_);
    }

    auto PseudocodeGenerator::visit
        (Indirection const& p) -> void
    {
        p.pointee_->accept(*this);
        out_->out("*", colors_.plain_);
    }

    auto PseudocodeGenerator::visit
        (Class const& c) -> void
    {
        // Class header.
        out_->begin_line();
        out_->out(is_interface(c) ? "Rozhranie " : "Trieda ", colors_.keyword_);
        out_->out(c.name_.empty() ? c.qualName_ : c.name_, colors_.customType_);

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
            out_->out(" implementuje ", colors_.keyword_);
            while (itImpl != basesMid)
            {
                auto const& name = (*itImpl)->name_.empty() ? (*itImpl)->qualName_ : (*itImpl)->name_;
                out_->out(name, colors_.customType_);
                ++itImpl;
                if ((itImpl != basesMid))
                {
                    out_->out(", ", colors_.plain_);
                }
            }
        }

        // Print base classes.
        auto itExt = basesMid;
        if (itExt != basesEnd)
        {
            out_->out(" rozširuje ", colors_.keyword_);
            while (itExt != basesEnd)
            {
                auto const& name = (*itImpl)->name_.empty() ? (*itImpl)->qualName_ : (*itImpl)->name_;
                out_->out(name, colors_.customType_);
                ++itExt;
                if (itExt != basesEnd)
                {
                    out_->out(", ", colors_.plain_);
                }
            }
        }

        out_->out(" {", colors_.plain_);
        out_->end_line();
        out_->inc_indent();

        // Visit methods.
        auto const end = std::end(c.methods_);
        auto it = std::begin(c.methods_);
        while (it != end)
        {
            it->accept(*this);
            ++it;

            if (it != end)
            {
                out_->blank_line();
            }
            else if (not c.fields_.empty())
            {
                out_->blank_line();
            }
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
        out_->out("operácia ", colors_.keyword_);
        out_->out(m.name_, colors_.function_);
        out_->out("(", colors_.plain_);

        this->visit_range(", ", std::begin(m.params_), std::end(m.params_));

        out_->out("): ", colors_.plain_);
        m.retType_->accept(*this);

        if (m.body_.has_value())
        {
            m.body_.value().accept(*this);
        }
        out_->end_line();
    }

    auto PseudocodeGenerator::visit
        (ForLoop const& f) -> void
    {
        out_->out("Pre ", colors_.keyword_);
        if (f.var_)
        {
            f.var_->accept(*this);
        }
            else
            {
                out_->out("null");
            }
        out_->out(" pokiaľ ", colors_.keyword_);
        if (f.cond_)
        {
            f.cond_->accept(*this);
        }
        out_->out(" pričom ", colors_.keyword_);
        if (f.inc_)
        {
            f.inc_->accept(*this);
        }
        f.body_.accept(*this);
    }

    auto PseudocodeGenerator::visit
        (WhileLoop const& w) -> void
    {
        out_->out("Pokiaľ ", colors_.keyword_);
        w.loop_.condition_->accept(*this);
        out_->out(" rob", colors_.keyword_);
        w.loop_.body_.accept(*this);
    }

    auto PseudocodeGenerator::visit
        (DoWhileLoop const& d) -> void
    {
        out_->out("Rob", colors_.keyword_);
        d.loop_.body_.accept(*this);
        out_->out(" pokiaľ ", colors_.keyword_);
        d.loop_.condition_->accept(*this);
    }

    auto PseudocodeGenerator::visit
        (VarDefCommon const& f) -> void
    {
        out_->out(f.name_, colors_.variable_);
        out_->out(": ", colors_.plain_);
        f.type_->accept(*this);
        if (f.initializer_)
        {
            out_->out(" <- ", colors_.plain_);
            f.initializer_->accept(*this);
        }
    }

    auto PseudocodeGenerator::visit
        (FieldDefinition const& f) -> void
    {
        out_->begin_line();
        out_->out("vlastnosť ", colors_.keyword_);
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
        out_->out("premenná ", colors_.keyword_);
        v.var_.accept(*this);
    }

    auto PseudocodeGenerator::visit
        (CompoundStatement const& ss) -> void
    {
        out_->out(" {", colors_.plain_);
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
        out_->out("}", colors_.plain_);
    }

    auto PseudocodeGenerator::visit
        (ExpressionStatement const& e) -> void
    {
        e.expression_->accept(*this);
    }

    auto PseudocodeGenerator::visit
        (Return const& r) -> void
    {
        out_->out("Vráť ", colors_.keyword_);
        r.expression_->accept(*this);
    }

    auto PseudocodeGenerator::visit
        (If const& i) -> void
    {
        out_->out("Ak ", colors_.keyword_);
        i.condition_->accept(*this);
        out_->out(" potom", colors_.keyword_);
        i.then_.accept(*this);
        if (i.else_)
        {
            out_->out(" inak", colors_.keyword_);
            i.else_.value().accept(*this);
        }
    }

    auto PseudocodeGenerator::visit
        (Delete const& d) -> void
    {
        out_->out("zruš ", colors_.keyword_);
        d.ex_->accept(*this);
    }

    auto PseudocodeGenerator::visit
        (DestructorCall const& d) -> void
    {
        out_->out("deštrutktor ", colors_.keyword_);
        d.ex_->accept(*this);
    }

    auto PseudocodeGenerator::visit
        (MemberFunctionCall const& m) -> void
    {
        m.base_->accept(*this);
        out_->out(".", colors_.plain_);
        out_->out(m.call_, colors_.function_);
        out_->out("(", colors_.plain_);
        this->visit_range(", ", std::begin(m.args_), std::end(m.args_));
        out_->out(")", colors_.plain_);
    }

    auto PseudocodeGenerator::visit
        (ExpressionCall const& e) -> void
    {
        e.ex_->accept(*this);
        out_->out("(", colors_.plain_);
        this->visit_range(", ", std::begin(e.args_), std::end(e.args_));
        out_->out(")", colors_.plain_);
    }

    auto PseudocodeGenerator::visit
        (Throw const&) -> void
    {
        out_->out("CHYBA", colors_.string_);
    }

    auto PseudocodeGenerator::bin_op_to_string
        (BinOpcode op) -> std::string
    {
        switch (op)
        {
            case BinOpcode::Add: return "+";
            case BinOpcode::Sub: return "-";
            case BinOpcode::Mul: return "*";
            case BinOpcode::Div: return "/";
            case BinOpcode::Mod: return "mod";
            case BinOpcode::And: return "a";
            case BinOpcode::Or:  return "alebo";
            case BinOpcode::LT:  return "<";
            case BinOpcode::LE:  return "<=";
            case BinOpcode::GT:  return ">";
            case BinOpcode::GE:  return ">=";
            case BinOpcode::EQ:  return "==";
            case BinOpcode::NE:  return "≠";

            case BinOpcode::Assign: return "<-";

            case BinOpcode::AddAssign: return "+";
            case BinOpcode::SubAssign: return "-";
            case BinOpcode::MulAssign: return "*";
            case BinOpcode::DivAssign: return "/";
            case BinOpcode::ModAssign: return "mod";

            case BinOpcode::Unknown: return "<unknown operator>";
            default:                 return "<unknown operator>";
        }
    }

    auto PseudocodeGenerator::un_op_to_string
        (UnOpcode const op) -> std::string
    {
        switch (op)
        {
            case UnOpcode::IncPre:  return "++";
            case UnOpcode::IncPost: return "++";
            case UnOpcode::DecPre:  return "--";
            case UnOpcode::DecPost: return "--";
            case UnOpcode::LogNot:  return "nie ";
            case UnOpcode::Deref:   return "sprístupni ";
            case UnOpcode::Address: return "adresa ";
            case UnOpcode::ArNot:   return "-";
            case UnOpcode::Sizeof:  return "veľkosť ";
            default:                return "<unknown operator>";
        }
    }

    auto PseudocodeGenerator::is_compound_op
        (BinOpcode const op) -> bool
    {
        switch (op)
        {
            case BinOpcode::AddAssign:
            case BinOpcode::SubAssign:
            case BinOpcode::MulAssign:
            case BinOpcode::DivAssign:
            case BinOpcode::ModAssign: return true;
            default:                   return false;
        }
    }

    auto PseudocodeGenerator::is_postfixx
        (UnOpcode const op) -> bool
    {
        switch (op)
        {
            case UnOpcode::IncPost:
            case UnOpcode::DecPost: return true;
            default:                return false;
        }
    }

    auto PseudocodeGenerator::op_color
        (std::string_view const s) -> Color
    {
        return not s.empty() and std::isalnum(s.front()) ? colors_.keyword_ : colors_.plain_;
    }

    template<class InputIt>
    auto PseudocodeGenerator::visit_range (std::string_view sep, InputIt first, InputIt last) -> void
    {
        using pointee_t = std::remove_cv_t<std::remove_reference_t<decltype(*first)>>;

        while (first != last)
        {
            auto const& elem = *first;

            if constexpr (std::is_pointer_v<pointee_t> or is_smart_pointer_v<pointee_t>)
            {
                elem->accept(*this);
            }
            else
            {
                elem.accept(*this);
            }

            ++first;
            if (first != last)
            {
                out_->out(sep, colors_.plain_);
            }
        }
    }
}