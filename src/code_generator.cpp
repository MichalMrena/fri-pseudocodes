#include "code_generator.hpp"

#include <iostream>
#include <algorithm>
#include <type_traits>
#include <cctype>
#include <cassert>

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
        (OutputSettings const& settings) :
        indentStep_    (settings.indentSpaces),
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
        (std::string_view s, TextStyle const& st) -> ConsoleCodePrinter&
    {
        this->set_color(st.color_);
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

    template<class Op>
    auto for_each_color (CodeStyleInfo const& st, Op&& op)
    {
        op(st.function_.color_);
        op(st.variable_.color_);
        op(st.memberVariable_.color_);
        op(st.keyword_.color_);
        op(st.plain_.color_);
        op(st.customType_.color_);
        op(st.primType_.color_);
        op(st.stringLiteral_.color_);
        op(st.valLiteral_.color_);
        op(st.numLiteral_.color_);
    }

// RtfCodePrinter definitions:

    RtfCodePrinter::RtfCodePrinter
        (std::ofstream& ofst, OutputSettings const& settings) :
        ofst_          (&ofst),
        indentStep_    (settings.indentSpaces),
        currentIndent_ (0),
        spaces_        ("                               ")
    {
        *ofst_ << R"({\rtf1\ansi\deff0\f0\fs)"   << (2 * settings.fontSize) << '\n'
               << R"({\fonttbl)"                 << '\n'
               << R"({\f0\fmodern )"             << settings.font << ";}"   << '\n'
               << R"(})"                         << '\n'
               << R"({\colortbl)"                << '\n'
               << R"(;)"                         << '\n';

        for_each_color(settings.style, [this](auto const& c)
        {
            colors_.emplace_back(c);
        });

        for (auto [r, g, b] : colors_)
        {
            auto const ri = static_cast<unsigned>(r);
            auto const gi = static_cast<unsigned>(g);
            auto const bi = static_cast<unsigned>(b);
            *ofst_ << R"(\red)"   << ri
                   << R"(\green)" << gi
                   << R"(\blue)"  << bi << ';' << '\n';
        }
        *ofst_ << R"(})" << '\n';
    }

    RtfCodePrinter::~RtfCodePrinter
        ()
    {
        ofst_->flush();
        *ofst_ << "}";
    }

    auto RtfCodePrinter::inc_indent
        () -> void
    {
        ++currentIndent_;
    }

    auto RtfCodePrinter::dec_indent
        () -> void
    {
         if (currentIndent_ > 0)
         {
             --currentIndent_;
         }
    }

    auto RtfCodePrinter::begin_line
        () -> void
    {
        auto const sc = std::min(spaces_.size(), currentIndent_ * indentStep_);
        *ofst_ << spaces_.substr(0, sc);
    }

    auto RtfCodePrinter::end_line
        () -> void
    {
        *ofst_ << R"(\line)" << '\n';
    }

    auto RtfCodePrinter::blank_line
        () -> void
    {
        this->end_line();
    }

    auto RtfCodePrinter::out
        (std::string_view s) -> RtfCodePrinter&
    {
        *ofst_ << encode(s);
        return *this;
    }

    auto RtfCodePrinter::out
        (std::string_view s, TextStyle const& st) -> RtfCodePrinter&
    {
        this->begin_color(st.color_);
        this->begin_style(st.style_);
        *ofst_ << encode(s);
        this->end_style(st.style_);
        this->end_color();
        return *this;
    }

    auto RtfCodePrinter::begin_color
        (Color const& c) -> void
    {
        *ofst_ << R"({\cf)"
               << this->color_code(c)
               << ' ';
    }

    auto RtfCodePrinter::end_color
        () -> void
    {
        *ofst_ << '}';
    }

    auto RtfCodePrinter::begin_style
        (FontStyle const s) -> void
    {
        switch (s)
        {
            case FontStyle::Bold:
                *ofst_ << R"(\b )";
                break;

            case FontStyle::Italic:
                *ofst_ << R"(\i )";
                break;

            default:
                break;
        }
    }

    auto RtfCodePrinter::end_style
        (FontStyle const s) -> void
    {
        switch (s)
        {
            case FontStyle::Bold:
                *ofst_ << R"(\b0)";
                break;

            case FontStyle::Italic:
                *ofst_ << R"(\i0)";
                break;

            default:
                break;
        }
    }

    auto RtfCodePrinter::color_code
        (Color const& c) -> unsigned
    {
        auto it = std::find(std::begin(colors_), std::end(colors_), c);
        return it == std::end(colors_)
            ? 0u
            : static_cast<unsigned>(std::distance(std::begin(colors_), it)) + 1;
    }

    auto RtfCodePrinter::encode
        (std::string_view s) -> std::string
    {
        auto cs = std::string();
        cs.reserve(s.size());

        auto const end = std::end(s);
        auto it = std::begin(s);
        while (it != end)
        {
            auto const c  = *it;
            auto const cu = static_cast<unsigned char>(c);
            if (c == '\\' or c == '{' or c == '}')
            {
                (cs += '\\') += c;
            }
            else if (cu & 0x80)
            {
                auto const as_u = [](auto const x){ return static_cast<unsigned char>(x); };
                auto const len = (cu & 0xE0u) == 0xC0u ? 2 :
                                 (cu & 0xF0u) == 0xE0u ? 3 : 4;
                auto const u = 2 == len ? (cu & 0x1Fu) << 6  | (as_u(*(it + 1)) & 0x3Fu) :
                               3 == len ? (cu & 0x1Fu) << 12 | (as_u(*(it + 1)) & 0x3Fu) << 6  | (as_u(*(it + 2)) & 0x3Fu) :
                                          (cu & 0x1Fu) << 18 | (as_u(*(it + 1)) & 0x3Fu) << 12 | (as_u(*(it + 2)) & 0x3Fu) << 6 | (as_u((*(it + 3))) & 0x3Fu) ;
                ((cs += R"(\u)") += std::to_string(u)) += '?';
                std::advance(it, len - 1);
            }
            else
            {
                cs += c;
            }
            ++it;
        }
        return cs;
    }

// PseudocodeGenerator definitions:

    PseudocodeGenerator::PseudocodeGenerator
        (ICodePrinter& out, CodeStyleInfo style) :
        out_   (&out),
        style_ (std::move(style))
    {
    }

    auto PseudocodeGenerator::visit
        (IntLiteral const& i) -> void
    {
        out_->out(std::to_string(i.num_), style_.numLiteral_);
    }

    auto PseudocodeGenerator::visit
        (FloatLiteral const& f) -> void
    {
        out_->out(std::to_string(f.num_), style_.numLiteral_);
    }

    auto PseudocodeGenerator::visit
        (StringLiteral const& s) -> void
    {
        out_->out("\"", style_.stringLiteral_);
        out_->out(s.str_, style_.stringLiteral_);
        out_->out("\"", style_.stringLiteral_);
    }

    auto PseudocodeGenerator::visit
        (NullLiteral const&) -> void
    {
        out_->out("NULL", style_.valLiteral_);
    }

    auto PseudocodeGenerator::visit
        (BoolLiteral const& b) -> void
    {
        out_->out(b.val_ ? "pravda" : "nepravda", style_.valLiteral_);
    }

    auto PseudocodeGenerator::visit
        (BinaryOperator const& b) -> void
    {
        auto const opstr = bin_op_to_string(b.op_);
        // auto const color = this->op_color(opstr); // TODO
        if (is_compound_op(b.op_))
        {
            b.lhs_->accept(*this);
            out_->out(" <- "); // TODO visit assign
            b.lhs_->accept(*this);
            out_->out(" ").out(opstr).out(" ");
            b.rhs_->accept(*this);
        }
        else
        {
            b.lhs_->accept(*this);
            out_->out(" ").out(opstr).out(" ");
            b.rhs_->accept(*this);
        }
    }

    auto PseudocodeGenerator::visit
        (Parenthesis const& p) -> void
    {
        out_->out("(");
        p.expression_->accept(*this);
        out_->out(")");
    }

    auto PseudocodeGenerator::visit
        (VarRef const& r) -> void
    {
        out_->out(r.name_, style_.variable_);
    }

    auto PseudocodeGenerator::visit
        (MemberVarRef const& m) -> void
    {
        this->visit_member_base(*m.base_);
        out_->out(m.name_, style_.memberVariable_);
    }

    auto PseudocodeGenerator::visit
        (UnaryOperator const& r) -> void
    {
        auto const output_arg = [this](auto const& var)
        {
            if (0 == var.index())
            {
                std::get<0>(var)->accept(*this);
            }
            else if (1 == var.index())
            {
                std::get<1>(var)->accept(*this);
            }
        };

        auto const opstr = un_op_to_string(r.op_);
        // auto const color = this->op_color(opstr); // TODO
        if (is_postfixx(r.op_))
        {
            output_arg(r.arg_);
            out_->out(opstr);
        }
        else if (is_bothtfix(r.op_))
        {
            out_->out(opstr);
            output_arg(r.arg_);
            out_->out(opstr);
        }
        else
        {
            out_->out(opstr);
            output_arg(r.arg_);
        }
    }

    auto PseudocodeGenerator::visit
        (New const& n) -> void
    {
        out_->out("vytvor ", style_.keyword_);
        n.type_->accept(*this);
        out_->out("(");
        this->visit_range(", ", std::begin(n.args_), std::end(n.args_));
        out_->out(")");
    }

    auto PseudocodeGenerator::visit
        (FunctionCall const& c) -> void
    {
        out_->out(c.name_, style_.function_);
        out_->out("(");
        this->visit_range(", ", std::begin(c.args_), std::end(c.args_));
        out_->out(")");
    }

    auto PseudocodeGenerator::visit
        (This const&) -> void
    {
        out_->out("self", style_.keyword_);
    }

    auto PseudocodeGenerator::visit
        (IfExpression const& c) -> void
    {
        out_->out("(");
        out_->out("Ak ", style_.keyword_);
        c.cond_->accept(*this);
        out_->out(" potom ", style_.keyword_);
        c.then_->accept(*this);
        out_->end_line();
        out_->begin_line();
        out_->out("inak ", style_.keyword_);
        c.else_->accept(*this);
        out_->out(")");
    }

    auto PseudocodeGenerator::visit
        (PrimType const& p) -> void
    {
        out_->out(p.name_, style_.primType_);
    }

    auto PseudocodeGenerator::visit
        (CustomType const& c) -> void
    {
        out_->out(c.name_, style_.customType_);
    }

    auto PseudocodeGenerator::visit
        (Indirection const& p) -> void
    {
        auto isVoidChecker = IsVoidVisitor();
        p.pointee_->accept(isVoidChecker);
        if (isVoidChecker.result())
        {
            out_->out("adresa", style_.primType_);
        }
        else
        {
            out_->out("↑");
            p.pointee_->accept(*this);
        }
    }

    auto PseudocodeGenerator::visit
        (Class const& c) -> void
    {
        // Class header.
        out_->begin_line();
        out_->out(is_interface(c) ? "Rozhranie " : "Trieda ", style_.keyword_);
        out_->out(c.name_.empty() ? c.qualName_ : c.name_, style_.customType_);

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
            out_->out(" implementuje ", style_.keyword_);
            while (itImpl != basesMid)
            {
                auto const& name = (*itImpl)->name_.empty() ? (*itImpl)->qualName_ : (*itImpl)->name_;
                out_->out(name, style_.customType_);
                ++itImpl;
                if ((itImpl != basesMid))
                {
                    out_->out(", ");
                }
            }
        }

        // Print base classes.
        auto itExt = basesMid;
        if (itExt != basesEnd)
        {
            out_->out(" rozširuje ", style_.keyword_);
            while (itExt != basesEnd)
            {
                auto const& name = (*itExt)->name_.empty() ? (*itExt)->qualName_ : (*itExt)->name_;
                out_->out(name, style_.customType_);
                ++itExt;
                if (itExt != basesEnd)
                {
                    out_->out(", ");
                }
            }
        }

        out_->out(" {");
        out_->end_line();
        out_->inc_indent();

        // Visit constructor declarations.
        for (auto const& con : c.constructors_)
        {
            this->visit_decl(con);
        }

        // Visit method declarations.
        auto const end = std::end(c.methods_);
        auto it = std::begin(c.methods_);
        while (it != end)
        {
            this->visit_decl(*it);
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

        // Visit constructor definitions.
        // Visit constructor declarations.
        for (auto const& constr : c.constructors_)
        {
            this->visit_def(c, constr);
        }

        // Visit method definitions.
        it = std::begin(c.methods_);
        while (it != end)
        {
            this->visit_def(c, *it);
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
    }

    auto PseudocodeGenerator::visit
        (Method const& m) -> void
    {
        out_->begin_line();
        out_->out("operácia ", style_.keyword_);
        out_->out(m.name_, style_.function_);
        out_->out("(");

        this->visit_range(", ", std::begin(m.params_), std::end(m.params_));

        out_->out("): ");
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
        out_->out("Pre ", style_.keyword_);
        if (f.var_)
        {
            f.var_->accept(*this);
        }
            else
            {
                out_->out("null");
            }
        out_->out(" pokiaľ ", style_.keyword_);
        if (f.cond_)
        {
            f.cond_->accept(*this);
        }
        out_->out(" pričom ", style_.keyword_);
        if (f.inc_)
        {
            f.inc_->accept(*this);
        }
        f.body_.accept(*this);
    }

    auto PseudocodeGenerator::visit
        (WhileLoop const& w) -> void
    {
        out_->out("Pokiaľ ", style_.keyword_);
        w.loop_.condition_->accept(*this);
        out_->out(" opakuj", style_.keyword_);
        w.loop_.body_.accept(*this);
    }

    auto PseudocodeGenerator::visit
        (DoWhileLoop const& d) -> void
    {
        out_->out("Opakuj", style_.keyword_);
        d.loop_.body_.accept(*this);
        out_->out(" pokiaľ ", style_.keyword_);
        d.loop_.condition_->accept(*this);
    }

    auto PseudocodeGenerator::visit
        (VarDefCommon const& f) -> void
    {
        out_->out(f.name_, style_.variable_);
        out_->out(": ");
        f.type_->accept(*this);
        if (f.initializer_)
        {
            out_->out(" <- ");
            f.initializer_->accept(*this);
        }
    }

    auto PseudocodeGenerator::visit
        (FieldDefinition const& f) -> void
    {
        out_->begin_line();
        out_->out("vlastnosť ", style_.keyword_);
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
        out_->out("premenná ", style_.keyword_);
        v.var_.accept(*this);
    }

    auto PseudocodeGenerator::visit
        (CompoundStatement const& ss) -> void
    {
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
        out_->begin_line();
        out_->out("}");
    }

    auto PseudocodeGenerator::visit
        (ExpressionStatement const& e) -> void
    {
        e.expression_->accept(*this);
    }

    auto PseudocodeGenerator::visit
        (Return const& r) -> void
    {
        out_->out("Vráť ", style_.keyword_);
        r.expression_->accept(*this);
    }

    auto PseudocodeGenerator::visit
        (If const& i) -> void
    {
        out_->out("Ak ", style_.keyword_);
        i.condition_->accept(*this);
        out_->out(" potom", style_.keyword_);
        i.then_.accept(*this);
        if (i.else_)
        {
            out_->out(" inak", style_.keyword_);
            i.else_.value().accept(*this);
        }
    }

    auto PseudocodeGenerator::visit
        (Delete const& d) -> void
    {
        out_->out("zruš ", style_.keyword_);
        d.ex_->accept(*this);
    }

    auto PseudocodeGenerator::visit
        (ConstructorCall const& c) -> void
    {
        c.type_->accept(*this);
        out_->out("(");
        this->visit_range(", ", std::begin(c.args_), std::end(c.args_)); // TODO visit args
        out_->out(")");
    }

    auto PseudocodeGenerator::visit
        (DestructorCall const& d) -> void
    {
        out_->out("deštrutktor ", style_.keyword_);
        d.ex_->accept(*this);
    }

    auto PseudocodeGenerator::visit
        (MemberFunctionCall const& m) -> void
    {
        this->visit_member_base(*m.base_);
        out_->out(m.call_, style_.function_);
        out_->out("(");
        this->visit_range(", ", std::begin(m.args_), std::end(m.args_));
        out_->out(")");
    }

    auto PseudocodeGenerator::visit
        (ExpressionCall const& e) -> void
    {
        e.ex_->accept(*this);
        out_->out("(");
        this->visit_range(", ", std::begin(e.args_), std::end(e.args_));
        out_->out(")");
    }

    auto PseudocodeGenerator::visit
        (Lambda const& l) -> void
    {
        // out_->out("λ", colors_.keyword_, FontStyle::Bold);

        for (auto const& p : l.params_)
        {
            out_->out("λ", style_.keyword_);
            out_->out(p.var_.name_, style_.variable_);
            out_->out(" ");
        }

        // out_->out("(", colors_.plain_);
        // auto const pend = std::end(l.params_);
        // auto pit = std::begin(l.params_);
        // while (pit != pend)
        // {
        //     out_->out((*pit).var_.name_, colors_.variable_);
        //     ++pit;
        //     if (pit != pend)
        //     {
        //         out_->out(", ", colors_.plain_);
        //     }
        // }
        // out_->out(")", colors_.plain_);

        out_->out("-> { ");
        auto const end = std::end(l.body_.statements_);
        auto it = std::begin(l.body_.statements_);
        while (it != end)
        {
            (*it)->accept(*this);
            ++it;
            if (it != end)
            {
                out_->out("; ");
            }
        }
        out_->out(" }");
    }

    auto PseudocodeGenerator::visit
        (Throw const&) -> void
    {
        out_->out("CHYBA", style_.stringLiteral_);
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
            case BinOpcode::EQ:  return "=";
            case BinOpcode::NE:  return "≠";

            case BinOpcode::Assign: return "⇐";

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
            case UnOpcode::Deref:   return "↓";
            case UnOpcode::Address: return "adresa ";
            case UnOpcode::ArNot:   return "-";
            case UnOpcode::Sizeof:  return "|";
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
            case UnOpcode::DecPost:
            case UnOpcode::Deref:   return true;
            default:                return false;
        }
    }

    auto PseudocodeGenerator::is_bothtfix
        (UnOpcode const op) -> bool
    {
        switch (op)
        {
            case UnOpcode::Sizeof: return true;
            default:               return false;
        }
    }

    auto PseudocodeGenerator::visit
        (Constructor const& c, CompoundStatement const& b) -> void
    {
        out_->out(" {");
        out_->end_line();
        out_->inc_indent();

        for (auto const& base : c.baseInitList_)
        {
            out_->begin_line();
            out_->out(base.name_, style_.customType_);
            out_->out("(");
            this->visit_range(", ", std::begin(base.init_), std::end(base.init_));
            out_->out(")");
            out_->end_line();
        }

        for (auto const& i : c.initList_)
        {
            out_->begin_line();
            out_->out(i.name_, style_.memberVariable_);
            out_->out(" ");
            out_->out(bin_op_to_string(BinOpcode::Assign));
            out_->out(" ");
            this->visit_range(", ", std::begin(i.init_), std::end(i.init_));
            out_->end_line();
        }

        for (auto const& s : b.statements_)
        {
            out_->begin_line();
            s->accept(*this);
            out_->end_line();
        }

        out_->dec_indent();
        out_->begin_line();
        out_->out("}");
    }

    auto PseudocodeGenerator::visit_decl
        (Method const& m) -> void
    {
        out_->begin_line();
        out_->out("operácia ", style_.keyword_);
        out_->out(m.name_, style_.function_);
        out_->out("(");

        this->visit_range(", ", std::begin(m.params_), std::end(m.params_));

        out_->out("): ");
        m.retType_->accept(*this);
    }

    auto PseudocodeGenerator::visit_decl
        (Constructor const& c) -> void
    {
        out_->begin_line();
        out_->out("konštruktor ", style_.keyword_);
        out_->out("(");
        this->visit_range(", ", std::begin(c.params_), std::end(c.params_));
        out_->out(") ");
        out_->end_line();
    }

    auto PseudocodeGenerator::visit_decl
        (Destructor const&) -> void
    {
        out_->out("deštruktor ", style_.keyword_);
    }

    auto PseudocodeGenerator::visit_def
        (Class const& c, Method const& m) -> void
    {
        out_->begin_line();
        out_->out("operácia ", style_.keyword_);
        this->visit_class_name(c);
        out_->out(".");
        out_->out(m.name_, style_.function_);
        out_->out("(");

        this->visit_range(", ", std::begin(m.params_), std::end(m.params_));

        out_->out("): ");
        m.retType_->accept(*this);

        if (m.body_.has_value())
        {
            m.body_.value().accept(*this);
        }
        out_->end_line();
    }

    auto PseudocodeGenerator::visit_def
        (Class const& c, Constructor const& con) -> void
    {
        out_->begin_line();
        out_->out("konštruktor ", style_.keyword_);
        this->visit_class_name(c);
        out_->out("(");
        this->visit_range(", ", std::begin(con.params_), std::end(con.params_));
        out_->out(")");
        if (con.body_)
        {
            this->visit(con, *con.body_);
        }
        out_->end_line();
        if (not c.methods_.empty() or c.destructor_.has_value())
        {
            out_->blank_line();
        }
    }

    auto PseudocodeGenerator::visit_def
        (Class const& c, Destructor const& d) -> void
    {
        out_->begin_line();
        out_->out("deštruktor ", style_.keyword_);
        this->visit_class_name(c);
        if (d.body_)
        {
            d.body_->accept(*this);
        }
        out_->end_line();
        if (not c.methods_.empty())
        {
            out_->blank_line();
        }
    }

    auto PseudocodeGenerator::visit_class_name
        (Class const& c) -> void
    {
        out_->out(c.name_, style_.customType_);
    }

    auto PseudocodeGenerator::visit_member_base
        (Expression const& e) -> void
    {
        auto isThisChecker = IsThisVisitor();
        e.accept(isThisChecker);
        if (not isThisChecker.result())
        {
            e.accept(*this);
            out_->out("→");
        }
    }

    auto PseudocodeGenerator::op_color
        (std::string_view const s) -> Color
    {
        return not s.empty() and std::isalnum(s.front()) ? style_.keyword_.color_ : style_.plain_.color_;
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
                out_->out(sep);
            }
        }
    }

    auto IsCheckVisitorBase::result
        () const -> bool
    {
        return result_;
    }

    auto IsVoidVisitor::visit
        (PrimType const& t) -> void
    {
        result_ = t.name_ == "void";
    }

    auto IsThisVisitor::visit
        (This const&) -> void
    {
        result_ = true;
    }
}