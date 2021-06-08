#include "clang_utils.hpp"

namespace fri
{
    auto extract_type (clang::QualType qt) -> std::unique_ptr<Type>
    {
        auto const t = qt.getTypePtr();
        if (t->isPointerType())
        {
            auto const ptr = t->getAs<clang::PointerType>();
            return std::make_unique<Indirection>(extract_type(ptr->getPointeeType()));
        }
        else if (t->isReferenceType())
        {
            auto const ref = t->getAs<clang::ReferenceType>();
            return std::make_unique<Indirection>(extract_type(ref->getPointeeType()));
        }
        else if (t->isBuiltinType())
        {
            return std::make_unique<PrimType>(qt.getAsString());
        }
        else if (t->isRecordType())
        {
            auto const r = t->getAs<clang::RecordType>();
            return std::make_unique<CustomType>(r->getAsRecordDecl()->getName().str());
        }
        else if (t->isTemplateTypeParmType())
        {
            return std::make_unique<CustomType>(qt.getAsString());
        }
        else
        {
            return std::make_unique<PrimType>("<unknown type>");
        }
    }

    auto switch_operator (clang::BinaryOperatorKind op) -> BinOpcode
    {
        switch (op)
        {
            case clang::BinaryOperatorKind::BO_Add: return BinOpcode::Add;
            case clang::BinaryOperatorKind::BO_Sub: return BinOpcode::Sub;
            case clang::BinaryOperatorKind::BO_Mul: return BinOpcode::Mul;
            case clang::BinaryOperatorKind::BO_Div: return BinOpcode::Div;
            case clang::BinaryOperatorKind::BO_Rem: return BinOpcode::Mod;
            case clang::BinaryOperatorKind::BO_And: return BinOpcode::And;
            case clang::BinaryOperatorKind::BO_Or:  return BinOpcode::Or;
            case clang::BinaryOperatorKind::BO_LT:  return BinOpcode::LT;
            case clang::BinaryOperatorKind::BO_LE:  return BinOpcode::LE;
            case clang::BinaryOperatorKind::BO_GT:  return BinOpcode::GT;
            case clang::BinaryOperatorKind::BO_GE:  return BinOpcode::GE;
            case clang::BinaryOperatorKind::BO_EQ:  return BinOpcode::EQ;
            case clang::BinaryOperatorKind::BO_NE:  return BinOpcode::NE;
            default:                                return BinOpcode::Unknown;
        }
    }
}