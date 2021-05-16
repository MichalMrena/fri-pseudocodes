#include "code_generator.hpp"

namespace fri
{
    CodePrinter::CodePrinter
        (std::ostream& ost) :
        ost_ (&ost)
    {
    }

    auto CodePrinter::out
        () -> std::ostream&
    {
        return *ost_;
    }

    PseudocodePrinter::PseudocodePrinter
        (std::ostream& ost) :
        CodePrinter(ost)
    {
    }
}