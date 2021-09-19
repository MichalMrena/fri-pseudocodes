#include "clang_utils.hpp"

namespace fri
{
    auto extract_type (clang::PrintingPolicy const& pp, clang::QualType qt) -> std::unique_ptr<Type>
    {
        auto const isConst = qt.isConstQualified();
        auto const typePtr = qt.getTypePtr();
        if (auto const ptr = clang::dyn_cast<clang::PointerType>(typePtr))
        {
            return std::make_unique<Indirection>(isConst, extract_type(pp, ptr->getPointeeType()));
        }
        else if (auto const ref = clang::dyn_cast<clang::ReferenceType>(typePtr))
        {
            return std::make_unique<Indirection>(isConst, extract_type(pp, ref->getPointeeType()));
        }
        else if (auto const builtin = clang::dyn_cast<clang::BuiltinType>(typePtr))
        {
            return std::make_unique<PrimType>(isConst, builtin->getName(pp).str());
        }
        else if (auto const typedefed = clang::dyn_cast<clang::TypedefType>(typePtr))
        {
            using uptr = std::unique_ptr<Type>;
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
            auto temDecl = tem->getTemplateName().getAsTemplateDecl();
            auto name = temDecl ? temDecl->getNameAsString() : "<some template>";
            auto args = std::vector<std::unique_ptr<Type>>();
            for (auto i = 0u; i < tem->getNumArgs(); ++i)
            {
                auto const arg = tem->getArg(i);
                switch (arg.getKind())
                {
                    case clang::TemplateArgument::ArgKind::Type:
                    {
                        auto const argType = arg.getAsType();
                        args.emplace_back(extract_type(pp, argType));
                        break;
                    }

                    case clang::TemplateArgument::ArgKind::Expression:
                    {
                        args.emplace_back(std::make_unique<PrimType>(IsConst(false), "<dummy expr>"));
                        break;
                    }

                    case clang::TemplateArgument::ArgKind::Integral:
                    {
                        args.emplace_back(std::make_unique<PrimType>(IsConst(false), "<dummy integral>"));
                        break;
                    }

                    default:
                    {
                        args.emplace_back(std::make_unique<PrimType>(IsConst(false), "<dummy other>"));
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
            return extract_type(pp, elab->desugar());
        }
        else if (auto func = clang::dyn_cast<clang::FunctionProtoType>(typePtr))
        {
            auto params = std::vector<std::unique_ptr<Type>>();
            for (auto const paramType : func->getParamTypes())
            {
                params.emplace_back(extract_type(pp, paramType));
            }
            return std::make_unique<Function>( std::move(params)
                                             , extract_type(pp, func->getReturnType()) );
        }
        else
        {
                typePtr->dump();
            return std::make_unique<PrimType>(IsConst(false), std::string("<unknown type> (") + qt.getAsString() + std::string(")"));
        }
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