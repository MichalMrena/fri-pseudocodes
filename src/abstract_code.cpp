#include "abstract_code.hpp"

#include <algorithm>

namespace fri
{
// PrimType definition:

    PrimType::PrimType
        (std::string name) :
        name_ (std::move(name))
    {
    }

// CustomType definition:

    CustomType::CustomType
        (std::string name) :
        name_ (std::move(name))
    {
    }

// Indirection definition:

    Indirection::Indirection
        (std::unique_ptr<Type> pointee) :
        pointee_ (std::move(pointee))
    {
    }

// IntLiteral definitions:

    IntLiteral::IntLiteral
        (std::int64_t const i) :
        num_ (i)
    {
    }

// FloatLiteral definition:

    FloatLiteral::FloatLiteral
        (double const d) :
        num_ (d)
    {
    }

// StringLiteral definition:

    StringLiteral::StringLiteral
        (std::string str) :
        str_ (std::move(str))
    {
    }

// BinaryOperator definition:

    BinaryOperator::BinaryOperator
        ( std::unique_ptr<Expression> lhs
        , BinOpcode                   op
        , std::unique_ptr<Expression> rhs ) :
        op_  (op),
        lhs_ (std::move(lhs)),
        rhs_ (std::move(rhs))
    {
    }

// Parenthesis definition:

    Parenthesis::Parenthesis
        (std::unique_ptr<Expression> expression) :
        expression_ (std::move(expression))
    {
    }

// VarRef definition:

    VarRef::VarRef
        (std::string name) :
        name_ (std::move(name))
    {
    }

// Assignment definition:

    Assignment::Assignment
        (std::unique_ptr<Expression> l, std::unique_ptr<Expression> r) :
        lhs_ (std::move(l)),
        rhs_ (std::move(r))
    {
    }

// CompoundStatement definitions:

    CompoundStatement::CompoundStatement
        (std::unique_ptr<Statement> s)
    {
        statements_.emplace_back(std::move(s));
    }

    CompoundStatement::CompoundStatement
        (std::vector<std::unique_ptr<Statement>> ss) :
        statements_ (std::move(ss))
    {
    }

// Return definition:

    Return::Return
        (std::unique_ptr<Expression> e) :
        expression_ (std::move(e))
    {
    }

// If definitions:

    If::If
        (std::unique_ptr<Expression> c, CompoundStatement t) :
        condition_ (std::move(c)),
        then_      (std::move(t))
    {
    }

    If::If
        (std::unique_ptr<Expression> c, CompoundStatement t, CompoundStatement e) :
        condition_ (std::move(c)),
        then_      (std::move(t)),
        else_      (std::move(e))
    {
    }

// ExpressionStatement definition:

    ExpressionStatement::ExpressionStatement
        (std::unique_ptr<Expression> expression) :
        expression_ (std::move(expression))
    {
    }

// WhileLoop definition:

    WhileLoop::WhileLoop
        (std::unique_ptr<Expression> c, CompoundStatement b) :
        loop_ {std::move(c), std::move(b)}
    {
    }

// DoWhileLoop definition:

    DoWhileLoop::DoWhileLoop
        (std::unique_ptr<Expression> c, CompoundStatement b) :
        loop_ {std::move(c), std::move(b)}
    {
    }

// Method definitions:

    auto is_pure_virtual (Method const& m) -> bool
    {
        return not m.body_.has_value();
    }

// Class definitions:

    Class::Class
        (std::string qualName) :
        qualName_ (std::move(qualName))
    {
    }

    auto is_interface (Class const& c) -> bool
    {
        return c.fields_.empty()
           and std::all_of(std::begin(c.bases_), std::end(c.bases_), [](auto const b){ return is_interface(*b); })
           and std::all_of(std::begin(c.methods_), std::end(c.methods_), [](auto const& m){ return is_pure_virtual(m); });
    }

// TranslationUnit definitions:

    TranslationUnit::TranslationUnit
        (std::vector<std::unique_ptr<Class>> classes) :
        classes_ (std::move(classes))
    {
    }

    auto TranslationUnit::get_classes
        () const -> std::vector<std::unique_ptr<Class>> const&
    {
        return classes_;
    }
}