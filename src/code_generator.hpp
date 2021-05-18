#ifndef FRI_CODE_GENERATOR_HPP
#define FRI_CODE_GENERATOR_HPP

#include "abstract_code.hpp"

#include <ostream>
#include <string_view>

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
        auto out        () -> std::ostream&;
        auto inc_indent () -> void;
        auto dec_indent () -> void;
        auto begin_line () -> void;
        auto end_line   () -> void;
        auto blank_line () -> void;

    private:
        std::size_t      indentStep_;
        std::size_t      currentIndent_;
        std::ostream*    ost_;
        std::string_view spaces_;
    };

    /**
     *  @brief Prints pseudocode.
     */
    class PseudocodePrinter : public CodePrinter, public CodeVisitor
    {
    public:
        PseudocodePrinter(std::ostream& ost);

        auto visit (Class const& c)              -> void override;
        auto visit (Method const& c)             -> void override;
        auto visit (ForLoop const& c)            -> void override;
        auto visit (WhileLoop const& c)          -> void override;
        auto visit (DoWhileLoop const& c)        -> void override;
        auto visit (FieldDefinition const& c)    -> void override;
        auto visit (VariableDefinition const& c) -> void override;
        auto visit (CompoundStatement const& c)  -> void override;
    };
}

#endif