#ifndef FRI_ABSTRACT_CODE_HPP
#define FRI_ABSTRACT_CODE_HPP

#include <string>
#include <vector>

namespace fri
{
    class CodeVisitor;

    struct Variable
    {
        bool        isStatic_;
        std::string type;
        std::string name_;
    };

    struct Statement
    {

    };

    struct Method
    {
        std::string name_;
    };

    struct Class
    {
        std::string           name_;
        std::vector<Variable> attributes_;
        std::vector<Method>   methods_;

        auto accept (CodeVisitor& visitor) const -> void;
    };

    /**
     *  @brief Code from translation unit. Just classes for now.
     */
    class TranslationUnit
    {
    public:
        TranslationUnit (std::vector<Class> classes);
        auto get_classes () const -> std::vector<Class> const&;

    private:
        std::vector<Class> classes_;
    };

    /**
     *  @brief Visitor interface for code classes.
     */
    class CodeVisitor
    {
    public:
        virtual ~CodeVisitor() = default;

        virtual auto visit (Class const& c)     -> void = 0;
        virtual auto visit (Method const& c)    -> void = 0;
        virtual auto visit (Statement const& c) -> void = 0;
    };
}

#endif