#ifndef FRI_CODE_GENERATOR_HPP
#define FRI_CODE_GENERATOR_HPP

#include "abstract_code.hpp"

#include <ostream>

/**
 *  @file code_generator.hpp
 *  Definitions of classes that generate some code
 *  from abstract structure of a code.
 */

namespace fri
{
    /**
     *  @brief Base class for code printers.
     */
    class CodePrinter
    {
    public:
        CodePrinter(std::ostream& ost);

    protected:
        auto out () -> std::ostream&;

    private:
        std::ostream* ost_;
    };

    /**
     *  @brief Prints pseudocode.
     */
    class PseudocodePrinter : public CodePrinter, public CodeVisitor
    {
    public:
        PseudocodePrinter(std::ostream& ost);

        auto visit (Class const& c)     -> void override;
        auto visit (Method const& c)    -> void override;
        auto visit (Statement const& c) -> void override;
    };
}

#endif