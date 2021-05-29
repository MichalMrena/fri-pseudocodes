#include "abstract_code.hpp"

#include <algorithm>

namespace fri
{
// ValueType definitions:

    ValueType::ValueType
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

// Method definitions:

    auto is_pure_virtual (Method const& m) -> bool
    {
        return m.body_.has_value();
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