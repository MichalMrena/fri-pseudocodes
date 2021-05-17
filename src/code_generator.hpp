#ifndef FRI_CODE_GENERATOR_HPP
#define FRI_CODE_GENERATOR_HPP

#include "abstract_code.hpp"

#include <ostream>

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

        virtual auto visit (Class const& c)  -> void;
        virtual auto visit (Method const& c) -> void;

        virtual auto visit (ForLoop const& c)           -> void;
        virtual auto visit (WhileLoop const& c)         -> void;
        virtual auto visit (DoWhileLoop const& c)       -> void;
        virtual auto visit (CompoundStatement const& c) -> void;
    };
}

#endif