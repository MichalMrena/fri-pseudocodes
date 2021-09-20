#include "abstract_code.hpp"

#include <algorithm>

namespace fri
{
    IsConst::IsConst
        (bool const is) :
        is_ {is}
    {
    }

    IsConst::operator bool
        () const
    {
        return is_;
    }

    template<class Derived>
    CommonType<Derived>::CommonType
        (IsConst const is) :
        isConst_ (is)
    {
    }

    template<class Derived>
    auto CommonType<Derived>::is_const
        () const -> bool
    {
        return isConst_;
    }

    PrimType::PrimType
        (IsConst const is, std::string name) :
        CommonType<PrimType> (is),
        name_ (std::move(name))
    {
    }

    auto PrimType::to_string
        () const -> std::string
    {
        return name_;
    }

    CustomType::CustomType
        (IsConst const is, std::string name) :
        CommonType<CustomType> (is),
        name_ (std::move(name))
    {
    }

    auto CustomType::to_string
        () const -> std::string
    {
        return name_;
    }

    TemplatedType::TemplatedType
        ( IsConst const is
        , uptr<Type>             b
        , std::vector<arg_var_t> as ) :
        CommonType<TemplatedType> (is),
        base_ (std::move(b)),
        args_ (std::move(as))
    {
    }

    auto TemplatedType::to_string
        () const -> std::string
    {
        auto ret = base_->to_string();
        if (not args_.empty())
        {
            ret += "<";
        }
        auto const end = std::end(args_);
        auto it = std::begin(args_);
        while (it != end)
        {
            ret += "<template arg>";
            ++it;
            if (it != end)
            {
                ret += ", ";
            }
        }
        if (not args_.empty())
        {
            ret += ">";
        }
        return ret;
    }

    Indirection::Indirection
        (IsConst const is, uptr<Type> pointee) :
        CommonType<Indirection> (is),
        pointee_ (std::move(pointee))
    {
    }

    auto Indirection::to_string
        () const -> std::string
    {
        return pointee_->to_string() + "*";
    }

    Function::Function
        ( std::vector<uptr<Type>> ps
        , uptr<Type>              r ) :
        CommonType<Function> (IsConst(false)),
        params_ (std::move(ps)),
        ret_    (std::move(r))
    {
    }

    auto Function::to_string
        () const -> std::string
    {
        auto ret = std::string("(");
        auto const end = std::end(params_);
        auto it = std::begin(params_);
        while (it != end)
        {
            ret += (*it)->to_string();
            ++it;
            if (it != end)
            {
                ret += ", ";
            }
        }
        ret += ") -> ";
        ret += ret_->to_string();
        return ret;
    }

    Nested::Nested
        ( IsConst     c
        , uptr<Type>  t
        , std::string n ) :
        CommonType<Nested> (c),
        nest_ (std::move(t)),
        name_ (std::move(n))
    {
    }

    auto Nested::to_string
        () const -> std::string
    {
        auto ret = nest_->to_string();
        (ret += ".") += name_;
        return ret;
    }

    VarDefCommon::VarDefCommon
        (uptr<Type> t, std::string n) :
        type_        (std::move(t)),
        name_        (std::move(n))
    {
    }

    VarDefCommon::VarDefCommon
        (uptr<Type> t, std::string n, uptr<Expression> i) :
        type_        (std::move(t)),
        name_        (std::move(n)),
        initializer_ (std::move(i))
    {
    }

    ParamDefinition::ParamDefinition
        (uptr<Type> t, std::string n) :
        var_ (std::move(t), std::move(n))
    {
    }

    ParamDefinition::ParamDefinition
        (uptr<Type> t, std::string n, uptr<Expression> i) :
        var_ (std::move(t), std::move(n), std::move(i))
    {
    }

    FieldDefinition::FieldDefinition
        (uptr<Type> t, std::string n) :
        var_ (std::move(t), std::move(n))
    {
    }

    FieldDefinition::FieldDefinition
        (uptr<Type> t, std::string n, uptr<Expression> i) :
        var_ (std::move(t), std::move(n), std::move(i))
    {
    }

    VarDefinition::VarDefinition
        (uptr<Type> t, std::string n) :
        var_ (std::move(t), std::move(n))
    {
    }

    VarDefinition::VarDefinition
        (uptr<Type> t, std::string n, uptr<Expression> i) :
        var_ (std::move(t), std::move(n), std::move(i))
    {
    }

    IntLiteral::IntLiteral
        (std::int64_t const i) :
        num_ (i)
    {
    }

    FloatLiteral::FloatLiteral
        (double const d) :
        num_ (d)
    {
    }

    BoolLiteral::BoolLiteral
        (bool const b) :
        val_ (b)
    {
    }

    StringLiteral::StringLiteral
        (std::string str) :
        str_ (std::move(str))
    {
    }

    BinaryOperator::BinaryOperator
        ( uptr<Expression> lhs
        , BinOpcode                   op
        , uptr<Expression> rhs ) :
        op_  (op),
        lhs_ (std::move(lhs)),
        rhs_ (std::move(rhs))
    {
    }

    UnaryOperator::UnaryOperator
        ( UnOpcode                    o
        , uptr<Expression> a) :
        op_  (o),
        arg_ (std::move(a))
    {
    }

    UnaryOperator::UnaryOperator
        ( UnOpcode              o
        , uptr<Type> a ) :
        op_  (o),
        arg_ (std::move(a))
    {
    }

    Parenthesis::Parenthesis
        (uptr<Expression> expression) :
        expression_ (std::move(expression))
    {
    }

    VarRef::VarRef
        (std::string name) :
        name_ (std::move(name))
    {
    }

    MemberVarRef::MemberVarRef
        ( uptr<Expression> b
        , std::string n ) :
        base_ (std::move(b)),
        name_ (std::move(n))
    {
    }

    New::New
        ( uptr<Type> t
        , std::vector<uptr<Expression>> as ) :
        type_ (std::move(t)),
        args_ (std::move(as))
    {
    }

    FunctionCall::FunctionCall
        ( std::string n
        , std::vector<uptr<Expression>> as ) :
        name_ (std::move(n)),
        args_ (std::move(as))
    {
    }

    ConstructorCall::ConstructorCall
        ( uptr<Type>                    t
        , std::vector<uptr<Expression>> as ) :
        type_ (std::move(t)),
        args_ (std::move(as))
    {
    }

    DestructorCall::DestructorCall
        (uptr<Expression> e) :
        ex_ (std::move(e))
    {
    }

    MemberFunctionCall::MemberFunctionCall
        ( uptr<Expression>              e
        , std::string                              c
        , std::vector<uptr<Expression>> as ) :
        base_ (std::move(e)),
        call_ (std::move(c)),
        args_ (std::move(as))
    {
    }

    ExpressionCall::ExpressionCall
        ( uptr<Expression>              e
        , std::vector<uptr<Expression>> as ) :
        ex_   (std::move(e)),
        args_ (std::move(as))
    {
    }

    Delete::Delete
        (uptr<Expression> ex) :
        ex_ (std::move(ex))
    {
    }

    CompoundStatement::CompoundStatement
        (uptr<Statement> s)
    {
        statements_.emplace_back(std::move(s));
    }

    CompoundStatement::CompoundStatement
        (std::vector<uptr<Statement>> ss) :
        statements_ (std::move(ss))
    {
    }

    Return::Return
        (uptr<Expression> e) :
        expression_ (std::move(e))
    {
    }

    If::If
        ( uptr<Expression> c
        , CompoundStatement t ) :
        condition_ (std::move(c)),
        then_      (std::move(t))
    {
    }

    If::If
        ( uptr<Expression> c
        , CompoundStatement t
        , CompoundStatement e) :
        condition_ (std::move(c)),
        then_      (std::move(t)),
        else_      (std::move(e))
    {
    }

    IfExpression::IfExpression
        ( uptr<Expression> c
        , uptr<Expression> t
        , uptr<Expression> e ) :
        cond_ (std::move(c)),
        then_ (std::move(t)),
        else_ (std::move(e))
    {
    }

    ExpressionStatement::ExpressionStatement
        (uptr<Expression> expression) :
        expression_ (std::move(expression))
    {
    }

    ForLoop::ForLoop
        ( uptr<Statement>  var
        , uptr<Expression> cond
        , uptr<Expression> inc
        , CompoundStatement           b ) :
        var_  (std::move(var)),
        cond_ (std::move(cond)),
        inc_  (std::move(inc)),
        body_ (std::move(b))
    {
    }

    WhileLoop::WhileLoop
        ( uptr<Expression> c
        , CompoundStatement           b ) :
        loop_ {std::move(c), std::move(b)}
    {
    }

    DoWhileLoop::DoWhileLoop
        ( uptr<Expression> c
        , CompoundStatement           b ) :
        loop_ {std::move(c), std::move(b)}
    {
    }

    Lambda::Lambda
        ( std::vector<ParamDefinition> ps
        , CompoundStatement            b ) :
        params_ (std::move(ps)),
        body_   (std::move(b))
    {
    }

    BaseInitPair::BaseInitPair
        ( uptr<Type>                    t
        , std::vector<uptr<Expression>> es ) :
        base_ (std::move(t)),
        init_ (std::move(es))
    {
    }

    MemberInitPair::MemberInitPair
        ( std::string                              n
        , std::vector<uptr<Expression>> es ) :
        name_ (std::move(n)),
        init_ (std::move(es))
    {
    }

    Constructor::Constructor
        ( std::vector<ParamDefinition>     ps
        , std::vector<BaseInitPair>        bis
        , std::vector<MemberInitPair>      is
        , std::optional<CompoundStatement> b ) :
        params_       (std::move(ps)),
        baseInitList_ (std::move(bis)),
        initList_     (std::move(is)),
        body_         (std::move(b))
    {
    }

    Destructor::Destructor
        (std::optional<CompoundStatement> b) :
        body_ (std::move(b))
    {
    }

    Method::Method
        ( std::string n
        , uptr<Type> ret
        , std::vector<ParamDefinition> ps
        , std::optional<CompoundStatement> b ) :
        name_    (std::move(n)),
        retType_ (std::move(ret)),
        params_  (std::move(ps)),
        body_    (std::move(b))
    {
    }

    Class::Class
        (std::string qualName) :
        qualName_ (std::move(qualName))
    {
    }

    auto Class::name
        () const -> std::string
    {
        return name_;
    }

    namespace
    {
        constexpr auto Interfaces = { "AbstractMemoryType"
                                    , "Table"
                                    , "Stack"
                                    , "Queue"
                                    , "PriorityQueue"
                                    , "List"
                                    , "Array" };

        auto is_interface (std::string_view const name) -> bool
        {
            auto const arrowIt = std::ranges::find(name, '<');
            auto const pos     = std::distance(std::begin(name), arrowIt);
            auto const prefix  = arrowIt == std::end(name)
                ? name
                : name.substr(0, static_cast<std::size_t>(pos));
            return std::ranges::find(Interfaces, prefix) != std::end(Interfaces);
        }
    }

    auto is_interface (Class const& c) -> bool
    {
        return is_interface(std::string_view(c.name()));
    }

    auto is_interface (Type const& t) -> bool
    {
        return is_interface(std::string_view(t.to_string()));
    }

    TranslationUnit::TranslationUnit
        (std::vector<uptr<Class>> classes) :
        classes_ (std::move(classes))
    {
    }

    auto TranslationUnit::get_classes
        () const -> std::vector<uptr<Class>> const&
    {
        return classes_;
    }
}