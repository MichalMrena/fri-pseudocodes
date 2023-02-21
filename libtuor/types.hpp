#ifndef FRI_TYPES_HPP
#define FRI_TYPES_HPP

#include <cstdint>
#include <memory>

namespace fri
{
    using int32 = std::int32_t;
    using int64 = std::int64_t;

    template<class T>
    using uptr = std::unique_ptr<T>;

    [[nodiscard]]
    inline auto constexpr as_uindex(int32 const i)
    {
        return static_cast<std::size_t>(i);
    }

    [[nodiscard]]
    inline auto constexpr as_uindex(int64 const i)
    {
        return static_cast<std::size_t>(i);
    }

    [[nodiscard]]
    inline auto constexpr as_usize(int32 const s)
    {
        return static_cast<std::size_t>(s);
    }

    [[nodiscard]]
    inline auto constexpr as_usize(int64 const s)
    {
        return static_cast<std::size_t>(s);
    }
}

#endif