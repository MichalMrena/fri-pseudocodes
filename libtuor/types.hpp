#include <cstdint>
#include <memory>

namespace fri
{
    using int32 = std::int32_t;
    using int64 = std::int64_t;

    template<class T>
    using uptr = std::unique_ptr<T>;
}