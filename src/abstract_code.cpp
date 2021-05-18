#include "abstract_code.hpp"

namespace fri
{
// TranslationUnit definitions:

    TranslationUnit::TranslationUnit
        (std::vector<Class> classes) :
        classes_ (std::move(classes))
    {
    }

    auto TranslationUnit::get_classes
        () const -> std::vector<Class> const&
    {
        return classes_;
    }
}