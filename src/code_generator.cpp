#include "code_generator.hpp"

#include <iostream>
#include <algorithm>
#include <type_traits>
#include <cctype>
#include <cassert>
#include <cmath>

#include "utils.hpp"

namespace fri
{
// Color definitions:

    auto to_string (Color const& c) -> std::string
    {
        auto res = std::string("Color(");
        res += std::to_string(c.r_);
        res += ", ";
        res += std::to_string(c.g_);
        res += ", ";
        res += std::to_string(c.b_);
        res += ")";
        return res;
    }

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

// CommonCodePrinter definitions:

    CommonCodePrinter::CommonCodePrinter
        (OutputSettings const& s) :
        indentStep_    {s.indentSpaces},
        indentCurrent_ {0}
    {
    }

    CommonCodePrinter::CommonCodePrinter
        (IndentState s) :
        indentStep_    {s.step},
        indentCurrent_ {s.current}
    {
    }

    auto CommonCodePrinter::inc_indent
        () -> void
    {
        ++indentCurrent_;
    }

    auto CommonCodePrinter::dec_indent
        () -> void
    {
        if (indentCurrent_ > 0)
        {
            --indentCurrent_;
        }
    }

    auto CommonCodePrinter::wrap_line
        () -> void
    {
        this->end_line();
        this->begin_line();
    }

    auto CommonCodePrinter::current_indent
        () const -> IndentState
    {
        return IndentState { .step = indentStep_
                           , .current = indentCurrent_ };
    }

    auto CommonCodePrinter::get_indent
        () const -> std::string_view
    {
        auto const sc = std::min(Spaces.size(), indentCurrent_ * indentStep_);
        return Spaces.substr(0, sc);
    }

// ConsoleCodePrinter definitions:

    ConsoleCodePrinter::ConsoleCodePrinter
        (OutputSettings const& settings) :
        base (settings)
    {
    }

    auto ConsoleCodePrinter::begin_line
        () -> void
    {
        std::cout << base::get_indent();
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

    auto ConsoleCodePrinter::end_region
        () -> void
    {
        this->blank_line();
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
                     :                              "\x1B[97m" );
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
        op(st.controlKeyword_.color_);
        op(st.plain_.color_);
        op(st.customType_.color_);
        op(st.primType_.color_);
        op(st.stringLiteral_.color_);
        op(st.valLiteral_.color_);
        op(st.numLiteral_.color_);
        op(st.lineNumber_.color_);
    }

// RtfCodePrinter definitions:

    RtfCodePrinter::RtfCodePrinter
        (std::ofstream& ofst, OutputSettings const& settings) :
        base  (settings),
        ofst_ (&ofst)
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

    auto RtfCodePrinter::begin_line
        () -> void
    {
        *ofst_ << base::get_indent();
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

    auto RtfCodePrinter::end_region
        () -> void
    {
        this->blank_line();
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
        this->out(s);
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

// DummyCodePrinter definitions:

    DummyCodePrinter::DummyCodePrinter
        (IndentState s) :
        base (s)
    {
    }

    auto DummyCodePrinter::begin_line
        () -> void
    {
        currentColumn_ += base::get_indent().size();
    }

    auto DummyCodePrinter::end_line
        () -> void
    {
        currentColumn_ = 0;
    }

    auto DummyCodePrinter::blank_line
        () -> void
    {
        this->end_line();
    }

    auto DummyCodePrinter::end_region
        () -> void
    {
        this->blank_line();
    }

    auto DummyCodePrinter::out
        (std::string_view const s) -> DummyCodePrinter&
    {
        currentColumn_ += s.size();
        return *this;
    }

    auto DummyCodePrinter::out
        (std::string_view const s, TextStyle const&) -> DummyCodePrinter&
    {
        this->out(s);
        return *this;
    }

    auto DummyCodePrinter::get_column
        () const -> std::size_t
    {
        return currentColumn_;
    }

// NumberedCodePrinter definitions:

    NumberedCodePrinter::NumberedCodePrinter
        (ICodePrinter& d, std::size_t const w, TextStyle s) :
        decoree_    {&d},
        numWidth_   {w},
        numStyle_   {s},
        currentNum_ {1}
    {
    }

    auto NumberedCodePrinter::inc_indent
        () -> void
    {
        decoree_->inc_indent();
    }

    auto NumberedCodePrinter::dec_indent
        () -> void
    {
        decoree_->dec_indent();
    }

    auto NumberedCodePrinter::begin_line
        () -> void
    {
        this->out_number();
        decoree_->out(" ");
        decoree_->begin_line();
    }

    auto NumberedCodePrinter::end_line
        () -> void
    {
        decoree_->end_line();
    }

    auto NumberedCodePrinter::wrap_line
        () -> void
    {
        decoree_->end_line();
        this->out_spaces();
        decoree_->begin_line();
    }

    auto NumberedCodePrinter::blank_line
        () -> void
    {
        decoree_->blank_line();
    }

    auto NumberedCodePrinter::end_region
        () -> void
    {
        currentNum_ = 1;
        decoree_->end_region();
    }

    auto NumberedCodePrinter::out
        (std::string_view const s) -> NumberedCodePrinter&
    {
        decoree_->out(s);
        return *this;
    }

    auto NumberedCodePrinter::out
        (std::string_view const s, TextStyle const& st) -> NumberedCodePrinter&
    {
        decoree_->out(s, st);
        return *this;
    }

    auto NumberedCodePrinter::current_indent
        () const -> IndentState
    {
        return decoree_->current_indent();
    }

    auto NumberedCodePrinter::out_number
        () -> void
    {
        auto const len = static_cast<long>(std::ceil(std::log10(static_cast<double>(currentNum_) + 0.1)));
        auto const numWidth = static_cast<long>(numWidth_);
        auto const spaceCount = static_cast<std::size_t>(std::max(numWidth - len, 0l));
        auto const spacesOffset = std::min(Spaces.size(), spaceCount);

        auto out = std::string();
        out += Spaces.substr(0, spacesOffset);
        out += std::to_string(currentNum_);
        out += ".";
        decoree_->out(out, numStyle_);
        ++currentNum_;
    }

    auto NumberedCodePrinter::out_spaces
        () -> void
    {
        decoree_->out(Spaces.substr(0, std::min(numWidth_ + 2, Spaces.size())));
    }

// PseudocodeGenerator definitions:

    PseudocodeGenerator::PseudocodeGenerator
        (ICodePrinter& out, CodeStyleInfo style) :
        out_       (&out),
        style_     (std::move(style)),
        funcNames_ { {"free", "zruš"}
                   , {"swap", "vymeň"}
                   , {"memmove", "presuňPamäť"}
                   , {"memcpy", "skopírujPamäť"}
                   , {"memcmp", "porovnajPamäť"}
                   , {"realloc", "zmeňVeľkosťPamäte"} }
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
        if (is_compound_op(b.op_))
        {
            b.lhs_->accept(*this);
            out_->out(" ");
            out_->out(bin_op_to_string(BinOpcode::Assign));
            out_->out(" ");
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
        out_->out(simplify_member_name(m.name_), style_.memberVariable_);
    }

    auto PseudocodeGenerator::visit
        (UnaryOperator const& r) -> void
    {
        auto const accept_this = [this](auto const& var)
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
        if (is_postfixx(r.op_))
        {
            accept_this(r.arg_);
            out_->out(opstr);
        }
        else if (is_bothtfix(r.op_))
        {
            out_->out(opstr);
            accept_this(r.arg_);
            out_->out(opstr);
        }
        else if (is_call(r.op_))
        {
            out_->out(un_op_to_string(r.op_), style_.function_);
            out_->out("(");
            accept_this(r.arg_);
            out_->out(")");
        }
        else // is_prefix
        {
            out_->out(opstr);
            accept_this(r.arg_);
        }
    }

    auto PseudocodeGenerator::visit
        (New const& n) -> void
    {
        out_->out("vytvor ", style_.keyword_);
        n.type_->accept(*this);
        out_->out("(");
        this->visit_args(n.args_);
        out_->out(")");
    }

    auto PseudocodeGenerator::visit
        (FunctionCall const& c) -> void
    {
        if (c.name_ == "free")
        {
            out_->out("zruš ", style_.keyword_);
            this->visit_args(c.args_);
        }
        else
        {
            out_->out(this->map_func_name(c.name_), style_.function_);
            out_->out("(");
            this->visit_args(c.args_);
            out_->out(")");
        }
    }

    auto PseudocodeGenerator::visit
        (This const&) -> void
    {
        out_->out("self", style_.keyword_);
    }

    auto PseudocodeGenerator::visit
        (IfExpression const& c) -> void
    {
        out_->out("Keď platí ", style_.controlKeyword_);
        out_->out("(");
        c.cond_->accept(*this);
        out_->out(")");
        out_->inc_indent();
        out_->wrap_line();
        out_->out("tak vráť ", style_.controlKeyword_);
        c.then_->accept(*this);
        out_->wrap_line();
        out_->out("inak vráť ", style_.controlKeyword_);
        c.else_->accept(*this);
        out_->dec_indent();
    }

    auto PseudocodeGenerator::visit
        (PrimType const& p) -> void
    {
        out_->out(simplify_type_name(p.name_), style_.primType_);
    }

    auto PseudocodeGenerator::visit
        (CustomType const& c) -> void
    {
        out_->out(c.name_, style_.customType_);
    }

    auto PseudocodeGenerator::visit
        (TemplatedType const& t) -> void
    {
        auto const out_args = [this, &t]()
        {
            this->visit_range(t.args_,
                [this](auto const& var)
                {
                    std::visit([this](auto const& p)
                    {
                        p->accept(*this);
                    }, var);
                },
                // TODO out comma member lambda
                [this]()
                {
                    out_->out(", ");
                });
        };

        if (t.to_string().starts_with("function"))
        {
            out_args();
        }
        else
        {
            t.base_->accept(*this);
            out_->out("<");
            out_args();
            out_->out(">");
        }
    }

    auto PseudocodeGenerator::visit
        (Indirection const& p) -> void
    {
        if (p.pointee_->to_string() == "void")
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
        (Function const& f) -> void
    {
        out_->out("λ", style_.keyword_);
        out_->out("(");
        this->visit_range(f.params_, [this]()
        {
            out_->out(", ");
        });
        out_->out(")");
        if (f.ret_->to_string() != "void")
        {
            out_->out(" → ");
            f.ret_->accept(*this);
        }
    }

    auto PseudocodeGenerator::visit
        (Nested const& n) -> void
    {
        n.nest_->accept(*this);
        out_->out(".");
        out_->out(n.name_, style_.customType_);
    }

    auto PseudocodeGenerator::visit
        (Class const& c) -> void
    {
        // Class header.
        out_->begin_line();
        out_->out(is_interface(c) ? "Rozhranie " : "Trieda ", style_.keyword_);
        this->visit_class_name(c);

        auto const baseCount = std::ranges::count_if(c.bases_, [](auto const& t)
        {
            return not is_interface(*t);
        });
        auto const interfaceCount = c.bases_.size() - static_cast<std::size_t>(baseCount);

        // Print base classes.
        if (baseCount)
        {
            out_->end_line();
            out_->inc_indent();
            out_->inc_indent();
            out_->begin_line();

            out_->out("rozširuje ", style_.keyword_);
            auto const end = std::end(c.bases_);
            auto it = std::begin(c.bases_);
            while (it != end and is_interface(**it))
            {
                ++it;
            }
            while (it != end)
            {
                (*it)->accept(*this);
                ++it;
                while (it != end and is_interface(**it))
                {
                    ++it;
                }
                if (it != end)
                {
                    out_->out(", ");
                }
            }

            if (interfaceCount)
            {
                out_->end_line();
            }
            out_->dec_indent();
            out_->dec_indent();
        }

        // Print base interfaces.
        if (interfaceCount)
        {
            if (not baseCount)
            {
                out_->end_line();
            }
            out_->inc_indent();
            out_->inc_indent();
            out_->begin_line();

            out_->out("realizuje ", style_.keyword_);
            auto const end = std::end(c.bases_);
            auto it = std::begin(c.bases_);
            while (it != end and not is_interface(**it))
            {
                ++it;
            }
            while (it != end)
            {
                (*it)->accept(*this);
                ++it;
                while (it != end and not is_interface(**it))
                {
                    ++it;
                }
                if (it != end)
                {
                    out_->out(", ");
                }
            }
            out_->dec_indent();
            out_->dec_indent();
        }

        // Begin class member definitions / declarations.
        out_->out(" {");
        out_->end_line();
        out_->inc_indent();

        // Visit constructor declarations.
        for (auto const& con : c.constructors_)
        {
            this->visit_decl(c, con, IsInline::Inline);
            out_->end_line();
        }

        // Visit method declarations.
        for (auto const& method : c.methods_)
        {
            this->visit_decl(c, method, IsInline::Inline);
            out_->end_line();
        }

        // Visit fields.
        for (auto const& f : c.fields_)
        {
            f.accept(*this);
        }

        // Class declaration end.
        out_->dec_indent();
        out_->begin_line();
        out_->out("}");
        out_->end_line();
        if (c.alias_)
        {
            out_->begin_line();
            out_->out(c.name_, style_.customType_);
            out_->out(" má skratku ", style_.keyword_);
            out_->out(*c.alias_, style_.customType_);
            out_->end_line();
        }
        out_->end_region();

        // Visit constructor definitions.
        for (auto const& constr : c.constructors_)
        {
            this->visit_def(c, constr);
            out_->end_region();
        }

        // Visit destructor definitions.
        if (c.destructor_)
        {
            this->visit_def(c, *c.destructor_);
            out_->end_region();
        }

        // Visit method definitions.
        for (auto const& method : c.methods_)
        {
            this->visit_def(c, method);
            out_->end_region();
        }
    }

    auto PseudocodeGenerator::visit
        (Method const&) -> void
    {
        out_->out("<visit(Method) not implemented>");
    }

    auto PseudocodeGenerator::visit
        (ForLoop const& f) -> void
    {
        auto forVar  = ForVarDefVisitor(*this);
        auto forFrom = ForFromVisitor(*this);
        auto forTo   = ForToVisitor(*this);

        out_->out("Opakuj pre premennú ", style_.controlKeyword_);
        if (f.var_)
        {
            f.var_->accept(forVar);
        }

        out_->out(" od ", style_.controlKeyword_);
        if (f.var_)
        {
            f.var_->accept(forFrom);
        }
        out_->out(" do ", style_.controlKeyword_);
        if (f.cond_)
        {
            f.cond_->accept(forTo);
        }
        f.body_.accept(*this);
    }

    auto PseudocodeGenerator::visit
        (WhileLoop const& w) -> void
    {
        out_->out("Pokiaľ ", style_.controlKeyword_);
        out_->out("(");
        w.loop_.condition_->accept(*this);
        out_->out(")");
        out_->out(" opakuj", style_.controlKeyword_);
        w.loop_.body_.accept(*this);
    }

    auto PseudocodeGenerator::visit
        (DoWhileLoop const& d) -> void
    {
        out_->out("Opakuj", style_.controlKeyword_);
        d.loop_.body_.accept(*this);
        out_->out(" pokiaľ ", style_.controlKeyword_);
        out_->out("(");
        d.loop_.condition_->accept(*this);
        out_->out(")");
    }

    auto PseudocodeGenerator::visit
        (VarDefCommon const& f) -> void
    {
        using SingleLine = struct
        {
            bool val;
        };

        auto const make_out = [this, &f](auto const singleLine)
        {
            return [this, singleLine, &f]()
            {
                out_->out(f.name_, style_.variable_);
                out_->out(": ");
                f.type_->accept(*this);
                if (f.initializer_)
                {
                    out_->out(" ");
                    out_->out(bin_op_to_string(BinOpcode::Assign));
                    if (singleLine.val)
                    {
                        out_->out(" ");
                        f.initializer_->accept(*this);
                    }
                    else
                    {
                        out_->inc_indent();
                        out_->wrap_line();
                        f.initializer_->accept(*this);
                        out_->dec_indent();
                    }
                }
            };
        };

        auto const out_inline = make_out(SingleLine {true});
        auto const out_wrapped = make_out(SingleLine {false});
        if (this->try_output_length(out_inline) > 59)
        {
            out_wrapped();
        }
        else
        {
            out_inline();
        }
    }

    auto PseudocodeGenerator::visit
        (FieldDefinition const& f) -> void
    {
        using namespace std::string_view_literals;
        auto const keyword = f.var_.type_->is_const() ? "konštanta "sv
                                                      : "vlastnosť "sv;
        out_->begin_line();
        out_->out(keyword, style_.keyword_);
        out_->out(simplify_member_name(f.var_.name_), style_.memberVariable_);
        out_->out(": ");
        f.var_.type_->accept(*this);
        if (f.var_.initializer_)
        {
            out_->out(" ");
            out_->out(bin_op_to_string(BinOpcode::Assign));
            out_->out(" ");
            f.var_.initializer_->accept(*this);
        }
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
        out_->out("definuj premennú ", style_.keyword_);
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
        if (!isa<IfExpression>(r.expression_))
        {
            out_->out("Vráť ", style_.controlKeyword_);
        }
        r.expression_->accept(*this);
    }

    auto PseudocodeGenerator::visit
        (If const& i) -> void
    {
        out_->out("Ak ", style_.controlKeyword_);
        out_->out("(");
        i.condition_->accept(*this);
        out_->out(")");
        out_->out(" potom", style_.controlKeyword_);
        i.then_.accept(*this);
        if (i.else_)
        {
            out_->end_line();
            out_->begin_line();
            out_->out("inak", style_.controlKeyword_);
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
        this->visit_args(c.args_);
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
        this->visit_args(m.args_);
        out_->out(")");
    }

    auto PseudocodeGenerator::visit
        (ExpressionCall const& e) -> void
    {
        e.ex_->accept(*this);
        out_->out("(");
        this->visit_args(e.args_);
        out_->out(")");
    }

    auto PseudocodeGenerator::visit
        (Lambda const& l) -> void
    {
        out_->out("λ", style_.keyword_);
        out_->out("(");
        this->visit_range(l.params_, [this]()
        {
            out_->out(", ");
        });
        out_->out(")");

        out_->out(" { ");
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

    auto PseudocodeGenerator::visit
        (Break const&) -> void
    {
    }

    auto PseudocodeGenerator::visit
        (Case const& c) -> void
    {
        out_->begin_line();
        out_->out("hodnotu ", style_.controlKeyword_);
        if (c.expr_)
        {
            c.expr_->accept(*this);
        }
        out_->out(" tak", style_.controlKeyword_);
        c.body_.accept(*this);
    }

    auto PseudocodeGenerator::visit
        (Switch const& s) -> void
    {
        out_->out("Keď ", style_.controlKeyword_);
        out_->out("(");
        s.cond_->accept(*this);
        out_->out(")");
        out_->out(" nadobúda", style_.controlKeyword_);
        out_->out(" {");
        out_->end_line();
        out_->inc_indent();
        for (auto const& swCase : s.cases_)
        {
            swCase.accept(*this);
            out_->end_line();
        }
        if (s.default_)
        {
            out_->begin_line();
            out_->out("žiadnu z uvedených hodnôt", style_.controlKeyword_);
            (*s.default_).accept(*this);
            out_->end_line();
        }
        out_->dec_indent();
        out_->begin_line();
        out_->out("}");
    }

    auto PseudocodeGenerator::out_plain
        (std::string_view const s) -> void
    {
        out_->out(s, style_.plain_);
    }

    auto PseudocodeGenerator::out_var_name
        (std::string_view const s) -> void
    {
        out_->out(s, style_.variable_);
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
            case BinOpcode::And: return "∧";
            case BinOpcode::Or:  return "∨";
            case BinOpcode::LT:  return "<";
            case BinOpcode::LE:  return "≤";
            case BinOpcode::GT:  return ">";
            case BinOpcode::GE:  return "≥";
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
            case UnOpcode::LogNot:  return "¬";
            case UnOpcode::Deref:   return "↓";
            case UnOpcode::Address: return "dajAdresu";
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

    auto PseudocodeGenerator::is_call
        (UnOpcode const op) -> bool
    {
        switch (op)
        {
            case UnOpcode::Address: return true;
            default:                return false;
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

    auto PseudocodeGenerator::simplify_type_name
        (std::string_view const name) -> std::string_view
    {
        return name == "size_t" ? "int" : name;
    }

    auto PseudocodeGenerator::simplify_member_name
        (std::string_view const name) -> std::string_view
    {
        return name.ends_with("_")
            ? name.substr(0, name.size() - 1)
            : name;
    }

    auto PseudocodeGenerator::visit_decl
        (Class const& c, Method const& m, IsInline const isIn) -> void
    {
        auto const out_name = [this, &c, &m, isIn]()
        {
            out_->begin_line();
            out_->out("operácia ", style_.keyword_);
            if (isIn == IsInline::NoInline)
            {
                this->visit_class_name(c);
                out_->out(".");
            }
            out_->out(m.name_, style_.function_);
        };

        auto const out_type = [this, &m]()
        {
            if (m.retType_->to_string() != "void")
            {
                out_->out(": ");
                m.retType_->accept(*this);
            }
        };

        this->visit_decl(out_name, m.params_, out_type);
    }

    auto PseudocodeGenerator::visit_decl
        (Class const& c, Constructor const& con, IsInline const isIn) -> void
    {
        auto const out_name = [this, &c, isIn]()
        {
            out_->begin_line();
            out_->out("konštruktor", style_.keyword_);
            if (isIn == IsInline::NoInline)
            {
                out_->out(" ");
                this->visit_class_name(c);
            }
        };

        this->visit_decl(out_name, con.params_, [](){});
    }

    auto PseudocodeGenerator::visit_decl
        (Class const&, Destructor const&) -> void
    {
        out_->out("deštruktor ", style_.keyword_);
    }

    auto PseudocodeGenerator::visit_def
        (Class const& c, Method const& m) -> void
    {
        if (not m.body_)
        {
            return;
        }

        this->visit_decl(c, m, IsInline::NoInline);

        if (m.body_.has_value())
        {
            m.body_->accept(*this);
        }
        out_->end_line();
        out_->blank_line();
    }

    auto PseudocodeGenerator::visit_def
        (Class const& c, Constructor const& con) -> void
    {
        if (con.baseInitList_.empty() and con.initList_.empty() and not con.body_)
        {
            return;
        }

        this->visit_decl(c, con, IsInline::NoInline);

        if (con.body_)
        {
            out_->out(" {");
            out_->end_line();
            out_->inc_indent();

            for (auto const& base : con.baseInitList_)
            {
                auto const baseName = base.base_->to_string();

                out_->begin_line();
                if (baseName.starts_with(c.name_))
                {
                    out_->out("inicializuj ", style_.keyword_);
                }
                else
                {
                    out_->out("inicializuj predka ", style_.keyword_);
                }
                base.base_->accept(*this);
                out_->out("(");
                this->visit_args(base.init_);
                out_->out(")");
                out_->end_line();
            }

            for (auto const& i : con.initList_)
            {
                out_->begin_line();
                out_->out(simplify_member_name(i.name_), style_.memberVariable_);
                out_->out(" ");
                out_->out(bin_op_to_string(BinOpcode::Assign));
                out_->out(" ");
                this->visit_args(i.init_);
                out_->end_line();
            }

            for (auto const& s : con.body_->statements_)
            {
                out_->begin_line();
                s->accept(*this);
                out_->end_line();
            }

            out_->dec_indent();
            out_->begin_line();
            out_->out("}");
        }
        out_->end_line();
        out_->blank_line();
    }

    auto PseudocodeGenerator::visit_def
        (Class const& c, Destructor const& d) -> void
    {
        if (not d.body_)
        {
            return;
        }

        out_->begin_line();
        out_->out("deštruktor ", style_.keyword_);
        this->visit_class_name(c);

        out_->out(" {");
        out_->inc_indent();
        out_->end_line();

        if (d.body_)
        {
            for (auto const& s : (*d.body_).statements_)
            {
                out_->begin_line();
                s->accept(*this);
                out_->end_line();
            }
        }

        if (not c.bases_.empty())
        {
            for (auto const& b : c.bases_)
            {
                out_->begin_line();
                out_->out("finalizuj predka ", style_.keyword_);
                b->accept(*this);
                out_->end_line();
            }
        }

        out_->dec_indent();
        out_->begin_line();
        out_->out("}");
        out_->end_line();
        out_->blank_line();
    }

    auto PseudocodeGenerator::visit_class_name
        (Class const& c) -> void
    {
        out_->out(c.name_.empty() ? c.qualName_ : c.name_, style_.customType_);
        if (not c.templateParams_.empty())
        {
            out_->out("<");
            this->output_range(c.templateParams_, ", ", style_.customType_);
            out_->out(">");
        }
    }

    auto PseudocodeGenerator::visit_member_base
        (Expression const& e) -> void
    {
        if (not isa<This>(e))
        {
            e.accept(*this);
            out_->out("→");
        }
    }

    template<class OutputName, class OutputType>
    auto PseudocodeGenerator::visit_decl
        ( OutputName&&                        name
        , std::vector<ParamDefinition> const& params
        , OutputType&&                        type ) -> void
    {
        // Outputs method header into single line.
        auto const out_single_line = [this, &name, &params, &type]()
        {
            name();
            out_->out("(");
            this->visit_range(params, [this]()
            {
                out_->out(", ");
            });
            out_->out(")");
            type();
        };

        // Outputs method header into multiple lines.
        // Each parameter is on a separate line.
        auto const out_multi_line = [this, &name, &params, &type]()
        {
            name();
            out_->out("(");
            if (not params.empty())
            {
                out_->inc_indent();
            }
            out_->wrap_line();
            this->visit_range(params, [this]()
            {
                out_->out(",");
                out_->wrap_line();
            });
            if (not params.empty())
            {
                out_->dec_indent();
                out_->wrap_line();
            }
            out_->out(")");
            type();
        };

        auto const col = this->try_output_length(out_single_line);
        if (col > 75)
        {
            out_multi_line();
        }
        else
        {
            out_single_line();
        }
    }

    template<class Range>
    auto PseudocodeGenerator::output_range
        (Range&& xs, std::string_view glue, TextStyle const& s) -> void
    {
        auto const end = std::end(xs);
        auto it = std::begin(xs);
        while (it != end)
        {
            out_->out(*it, s);
            ++it;
            if (it != end)
            {
                out_->out(glue);
            }
        }
    }

    auto PseudocodeGenerator::visit_args
        (std::vector<std::unique_ptr<Expression>> const& as) -> void
    {
        this->visit_range(as, [this]()
        {
            out_->out(", ");
        });
    }

    template<class Range, class OutputSep>
    auto PseudocodeGenerator::visit_range
        (Range&& r, OutputSep&& s) -> void
    {
        this->visit_range(r, [this](auto const& elem)
        {
            using pointee_t = std::remove_cv_t<std::remove_reference_t<decltype(elem)>>;
            if constexpr (std::is_pointer_v<pointee_t> or is_smart_pointer_v<pointee_t>)
            {
                elem->accept(*this);
            }
            else
            {
                elem.accept(*this);
            }
        }, s);
    }

    template<class Range, class Visitor, class OutputSep>
    auto PseudocodeGenerator::visit_range
        (Range&& r, Visitor&& v, OutputSep&& s) -> void
    {
        auto const last = std::end(r);
        auto first = std::begin(r);

        while (first != last)
        {
            v(*first);
            ++first;
            if (first != last)
            {
                s();
            }
        }
    }

    template<class LineOut>
    auto PseudocodeGenerator::try_output_length
        (LineOut&& o) -> std::size_t
    {
        auto const realOutput = out_;
        auto dummyOutput = DummyCodePrinter(out_->current_indent());
        out_ = &dummyOutput;
        o();
        auto const column = dummyOutput.get_column();
        out_ = realOutput;
        return column;
    }

    auto PseudocodeGenerator::map_func_name
        (std::string const& s) const -> std::string_view
    {
        namespace rs = std::ranges;
        auto it = funcNames_.find(s);
        return it == rs::end(funcNames_) ? s : (*it).second;
    }

    ForVarDefVisitor::ForVarDefVisitor
        (PseudocodeGenerator& v) :
        real_ (&v)
    {
    }

    auto ForVarDefVisitor::visit
        (VarDefinition const& v) -> void
    {
        real_->out_var_name(v.var_.name_);
        real_->out_plain(": ");
        v.var_.type_->accept(*real_);
    }

    ForFromVisitor::ForFromVisitor
        (PseudocodeGenerator& v) :
        real_ (&v)
    {
    }

    auto ForFromVisitor::visit
        (VarDefinition const& b) -> void
    {
        if (b.var_.initializer_)
        {
            b.var_.initializer_->accept(*real_);
        }
    }

    ForToVisitor::ForToVisitor
        (PseudocodeGenerator& v) :
        real_ (&v)
    {
    }

    auto ForToVisitor::visit
        (BinaryOperator const& b) -> void
    {
        b.rhs_->accept(*real_);
        real_->out_plain(" - ");
        real_->visit(IntLiteral(1));
    }
}