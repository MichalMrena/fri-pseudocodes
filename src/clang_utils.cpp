#include "clang_utils.hpp"
#include "clang_expression_visitor.hpp"

namespace fri
{
    auto extract_type ( clang::PrintingPolicy const& pp
                      , clang::Type const*           typePtr
                      , ExpressionVisitor&           ex
                      , IsConst                      isConst ) -> uptr<Type>
    {
        if (auto const ptr = clang::dyn_cast<clang::PointerType>(typePtr))
        {
            return std::make_unique<Indirection>(isConst, extract_type(pp, ptr->getPointeeType(), ex));
        }
        else if (auto const ref = clang::dyn_cast<clang::ReferenceType>(typePtr))
        {
            return std::make_unique<Indirection>(isConst, extract_type(pp, ref->getPointeeType(), ex));
        }
        else if (auto const builtin = clang::dyn_cast<clang::BuiltinType>(typePtr))
        {
            return std::make_unique<PrimType>(isConst, builtin->getName(pp).str());
        }
        else if (auto const typedefed = clang::dyn_cast<clang::TypedefType>(typePtr))
        {
            using uptr = uptr<Type>;
            return clang::isa<clang::BuiltinType>(typedefed->desugar().getTypePtr())
                ? uptr(std::make_unique<PrimType>(isConst, typedefed->getDecl()->getName().str()))
                : std::make_unique<CustomType>(isConst, typedefed->getDecl()->getName().str());
        }
        else if (auto const record = clang::dyn_cast<clang::RecordType>(typePtr))
        {
            return std::make_unique<CustomType>(isConst, record->getAsRecordDecl()->getName().str());
        }
        else if (auto temParam = clang::dyn_cast<clang::TemplateTypeParmType>(typePtr))
        {
            auto paramDecl = temParam->getDecl();
            return std::make_unique<CustomType>(isConst, paramDecl->getIdentifier()->getName().str());
        }
        else if (auto tem = clang::dyn_cast<clang::TemplateSpecializationType>(typePtr))
        {
            using variant_t = TemplatedType::arg_var_t;
            auto temDecl = tem->getTemplateName().getAsTemplateDecl();
            auto name = temDecl ? temDecl->getNameAsString() : "<some template>";
            auto args = std::vector<variant_t>();
            for (auto i = 0u; i < tem->getNumArgs(); ++i)
            {
                auto const arg = tem->getArg(i);
                switch (arg.getKind())
                {
                    case clang::TemplateArgument::ArgKind::Type:
                    {
                        auto const argType = arg.getAsType();
                        args.emplace_back(variant_t(extract_type(pp, argType, ex)));
                        break;
                    }

                    case clang::TemplateArgument::ArgKind::Expression:
                    {
                        args.emplace_back(variant_t(ex.read_expression(arg.getAsExpr())));
                        break;
                    }

                    case clang::TemplateArgument::ArgKind::Integral:
                    {
                        args.emplace_back(variant_t(std::make_unique<PrimType>(IsConst(false), "<dummy integral>")));
                        break;
                    }

                    default:
                    {
                        args.emplace_back(variant_t(std::make_unique<PrimType>(IsConst(false), "<dummy other>")));
                        break;
                    }
                }
            }
            return std::make_unique<TemplatedType>( isConst
                                                  , std::make_unique<CustomType>(IsConst(false), name)
                                                  , std::move(args) );
        }
        else if (auto elab = clang::dyn_cast<clang::ElaboratedType>(typePtr))
        {
            // TODO is sugared
            return extract_type(pp, elab->desugar(), ex);
        }
        else if (auto func = clang::dyn_cast<clang::FunctionProtoType>(typePtr))
        {
            auto params = std::vector<uptr<Type>>();
            for (auto const paramType : func->getParamTypes())
            {
                params.emplace_back(extract_type(pp, paramType, ex));
            }
            return std::make_unique<Function>( std::move(params)
                                            , extract_type(pp, func->getReturnType(), ex) );
        }
        else if (auto const icn = clang::dyn_cast<clang::InjectedClassNameType>(typePtr))
        {
            return extract_type(pp, icn->getInjectedTST(), ex, isConst);
        }
        else if (auto const dnt = clang::dyn_cast<clang::DependentNameType>(typePtr))
        {
            auto nestSpec = dnt->getQualifier();
            auto name = dnt->getIdentifier()->getName().str();
            switch (nestSpec->getKind())
            {
                case clang::NestedNameSpecifier::SpecifierKind::TypeSpec:
                {
                    auto nest = extract_type(pp, nestSpec->getAsType(), ex, IsConst(false));
                    return std::make_unique<Nested>(isConst, std::move(nest), std::move(name));
                }

                default:
                {
                    typePtr->dump();
                    return std::make_unique<PrimType>(IsConst(false), "<unknown nested type>");
                }
            }
        }
        else
        {
                typePtr->dump();
            // return std::make_unique<PrimType>(IsConst(false), std::string("<unknown type> (") + qt.getAsString() + std::string(")"));
            return std::make_unique<PrimType>(IsConst(false), std::string("<unknown type>"));
        }
    }

    auto extract_type ( clang::PrintingPolicy const& pp
                      , clang::QualType              qt
                      , ExpressionVisitor&           ex ) -> uptr<Type>
    {
        auto const isConst = qt.isConstQualified();
        auto const typePtr = qt.getTypePtr();
        return extract_type(pp, typePtr, ex, IsConst(isConst));
    }

    auto switch_bin_operator (clang::BinaryOperatorKind const op) -> BinOpcode
    {
        switch (op)
        {
            case clang::BinaryOperatorKind::BO_Add:  return BinOpcode::Add;
            case clang::BinaryOperatorKind::BO_Sub:  return BinOpcode::Sub;
            case clang::BinaryOperatorKind::BO_Mul:  return BinOpcode::Mul;
            case clang::BinaryOperatorKind::BO_Div:  return BinOpcode::Div;
            case clang::BinaryOperatorKind::BO_Rem:  return BinOpcode::Mod;
            case clang::BinaryOperatorKind::BO_LAnd: return BinOpcode::And;
            case clang::BinaryOperatorKind::BO_LOr:  return BinOpcode::Or;
            case clang::BinaryOperatorKind::BO_LT:   return BinOpcode::LT;
            case clang::BinaryOperatorKind::BO_LE:   return BinOpcode::LE;
            case clang::BinaryOperatorKind::BO_GT:   return BinOpcode::GT;
            case clang::BinaryOperatorKind::BO_GE:   return BinOpcode::GE;
            case clang::BinaryOperatorKind::BO_EQ:   return BinOpcode::EQ;
            case clang::BinaryOperatorKind::BO_NE:   return BinOpcode::NE;

            case clang::BinaryOperatorKind::BO_Assign: return BinOpcode::Assign;

            case clang::BinaryOperatorKind::BO_AddAssign: return BinOpcode::AddAssign;
            case clang::BinaryOperatorKind::BO_SubAssign: return BinOpcode::SubAssign;
            case clang::BinaryOperatorKind::BO_MulAssign: return BinOpcode::MulAssign;
            case clang::BinaryOperatorKind::BO_DivAssign: return BinOpcode::DivAssign;
            case clang::BinaryOperatorKind::BO_RemAssign: return BinOpcode::ModAssign;

            default: return BinOpcode::Unknown;
        }
    }

    auto switch_un_operator (clang::UnaryOperatorKind const op) -> UnOpcode
    {
        switch (op)
        {
            case clang::UnaryOperatorKind::UO_PreInc:  return UnOpcode::IncPre;
            case clang::UnaryOperatorKind::UO_PostInc: return UnOpcode::IncPost;
            case clang::UnaryOperatorKind::UO_PreDec:  return UnOpcode::DecPre;
            case clang::UnaryOperatorKind::UO_PostDec: return UnOpcode::DecPost;
            case clang::UnaryOperatorKind::UO_LNot:    return UnOpcode::LogNot;
            case clang::UnaryOperatorKind::UO_Deref:   return UnOpcode::Deref;
            case clang::UnaryOperatorKind::UO_AddrOf:  return UnOpcode::Address;
            case clang::UnaryOperatorKind::UO_Minus:   return UnOpcode::ArNot;
            default: return UnOpcode::Unknown;
        }
    }
}