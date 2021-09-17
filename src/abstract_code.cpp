#include "abstract_code.hpp"

#include <algorithm>

namespace fri
{
    PrimType::PrimType
        (std::string name) :
        name_ (std::move(name))
    {
    }

    CustomType::CustomType
        (std::string name) :
        name_ (std::move(name))
    {
    }

    TemplatedType::TemplatedType
        ( std::unique_ptr<Type>              b
        , std::vector<std::unique_ptr<Type>> as ) :
        base_ (std::move(b)),
        args_ (std::move(as))
    {
    }

    Indirection::Indirection
        (std::unique_ptr<Type> pointee) :
        pointee_ (std::move(pointee))
    {
    }

    VarDefCommon::VarDefCommon
        (std::unique_ptr<Type> t, std::string n) :
        type_        (std::move(t)),
        name_        (std::move(n))
    {
    }

    VarDefCommon::VarDefCommon
        (std::unique_ptr<Type> t, std::string n, std::unique_ptr<Expression> i) :
        type_        (std::move(t)),
        name_        (std::move(n)),
        initializer_ (std::move(i))
    {
    }

    ParamDefinition::ParamDefinition
        (std::unique_ptr<Type> t, std::string n) :
        var_ (std::move(t), std::move(n))
    {
    }

    ParamDefinition::ParamDefinition
        (std::unique_ptr<Type> t, std::string n, std::unique_ptr<Expression> i) :
        var_ (std::move(t), std::move(n), std::move(i))
    {
    }

    FieldDefinition::FieldDefinition
        (std::unique_ptr<Type> t, std::string n) :
        var_ (std::move(t), std::move(n))
    {
    }

    FieldDefinition::FieldDefinition
        (std::unique_ptr<Type> t, std::string n, std::unique_ptr<Expression> i) :
        var_ (std::move(t), std::move(n), std::move(i))
    {
    }

    VarDefinition::VarDefinition
        (std::unique_ptr<Type> t, std::string n) :
        var_ (std::move(t), std::move(n))
    {
    }

    VarDefinition::VarDefinition
        (std::unique_ptr<Type> t, std::string n, std::unique_ptr<Expression> i) :
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
        ( std::unique_ptr<Expression> lhs
        , BinOpcode                   op
        , std::unique_ptr<Expression> rhs ) :
        op_  (op),
        lhs_ (std::move(lhs)),
        rhs_ (std::move(rhs))
    {
    }

    UnaryOperator::UnaryOperator
        ( UnOpcode                    o
        , std::unique_ptr<Expression> a) :
        op_  (o),
        arg_ (std::move(a))
    {
    }

    UnaryOperator::UnaryOperator
        ( UnOpcode              o
        , std::unique_ptr<Type> a ) :
        op_  (o),
        arg_ (std::move(a))
    {
    }

    Parenthesis::Parenthesis
        (std::unique_ptr<Expression> expression) :
        expression_ (std::move(expression))
    {
    }

    VarRef::VarRef
        (std::string name) :
        name_ (std::move(name))
    {
    }

    MemberVarRef::MemberVarRef
        ( std::unique_ptr<Expression> b
        , std::string n ) :
        base_ (std::move(b)),
        name_ (std::move(n))
    {
    }

    New::New
        ( std::unique_ptr<Type> t
        , std::vector<std::unique_ptr<Expression>> as ) :
        type_ (std::move(t)),
        args_ (std::move(as))
    {
    }

    FunctionCall::FunctionCall
        ( std::string n
        , std::vector<std::unique_ptr<Expression>> as ) :
        name_ (std::move(n)),
        args_ (std::move(as))
    {
    }

    ConstructorCall::ConstructorCall
        ( std::unique_ptr<Type>                    t
        , std::vector<std::unique_ptr<Expression>> as ) :
        type_ (std::move(t)),
        args_ (std::move(as))
    {
    }

    DestructorCall::DestructorCall
        (std::unique_ptr<Expression> e) :
        ex_ (std::move(e))
    {
    }

    MemberFunctionCall::MemberFunctionCall
        ( std::unique_ptr<Expression>              e
        , std::string                              c
        , std::vector<std::unique_ptr<Expression>> as ) :
        base_ (std::move(e)),
        call_ (std::move(c)),
        args_ (std::move(as))
    {
    }

    ExpressionCall::ExpressionCall
        ( std::unique_ptr<Expression>              e
        , std::vector<std::unique_ptr<Expression>> as ) :
        ex_   (std::move(e)),
        args_ (std::move(as))
    {
    }

    Delete::Delete
        (std::unique_ptr<Expression> ex) :
        ex_ (std::move(ex))
    {
    }

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

    Return::Return
        (std::unique_ptr<Expression> e) :
        expression_ (std::move(e))
    {
    }

    If::If
        ( std::unique_ptr<Expression> c
        , CompoundStatement t ) :
        condition_ (std::move(c)),
        then_      (std::move(t))
    {
    }

    If::If
        ( std::unique_ptr<Expression> c
        , CompoundStatement t
        , CompoundStatement e) :
        condition_ (std::move(c)),
        then_      (std::move(t)),
        else_      (std::move(e))
    {
    }

    IfExpression::IfExpression
        ( std::unique_ptr<Expression> c
        , std::unique_ptr<Expression> t
        , std::unique_ptr<Expression> e ) :
        cond_ (std::move(c)),
        then_ (std::move(t)),
        else_ (std::move(e))
    {
    }

    ExpressionStatement::ExpressionStatement
        (std::unique_ptr<Expression> expression) :
        expression_ (std::move(expression))
    {
    }

    ForLoop::ForLoop
        ( std::unique_ptr<Statement>  var
        , std::unique_ptr<Expression> cond
        , std::unique_ptr<Expression> inc
        , CompoundStatement           b ) :
        var_  (std::move(var)),
        cond_ (std::move(cond)),
        inc_  (std::move(inc)),
        body_ (std::move(b))
    {
    }

    WhileLoop::WhileLoop
        ( std::unique_ptr<Expression> c
        , CompoundStatement           b ) :
        loop_ {std::move(c), std::move(b)}
    {
    }

    DoWhileLoop::DoWhileLoop
        ( std::unique_ptr<Expression> c
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
        ( std::string                              n
        , std::vector<std::unique_ptr<Expression>> es ) :
        name_ (std::move(n)),
        init_ (std::move(es))
    {
    }

    MemberInitPair::MemberInitPair
        ( std::string                              n
        , std::vector<std::unique_ptr<Expression>> es ) :
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
        , std::unique_ptr<Type> ret
        , std::vector<ParamDefinition> ps
        , std::optional<CompoundStatement> b ) :
        name_    (std::move(n)),
        retType_ (std::move(ret)),
        params_  (std::move(ps)),
        body_    (std::move(b))
    {
    }

    auto is_pure_virtual (Method const& m) -> bool
    {
        return not m.body_.has_value();
    }

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