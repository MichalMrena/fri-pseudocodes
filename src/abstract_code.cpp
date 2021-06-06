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

// StringLiteral definition:

    StringLiteral::StringLiteral
        (std::string str) :
        str_ (std::move(str))
    {
    }

// Return definition:

    Return::Return
        (std::unique_ptr<Expression> e) :
        expression_ (std::move(e))
    {
    }

// ExpressionStatement definition:

    ExpressionStatement::ExpressionStatement
        (std::unique_ptr<Expression> expression) :
        expression_ (std::move(expression))
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