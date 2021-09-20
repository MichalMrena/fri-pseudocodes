#include <memory>

namespace fri
{
    template<class T>
    using uptr = std::unique_ptr<T>;
}