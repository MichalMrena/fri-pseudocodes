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
}