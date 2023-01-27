#ifndef FRI_UTILS_HPP
#define FRI_UTILS_HPP

#include <libtuor/types.hpp>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace fri
{
    auto to_words (std::string_view s) -> std::vector<std::string>;

    auto to_lowercase (std::string_view s) -> std::string;

    auto to_uppercase (std::string_view s) -> std::string;

    template<class Num>
    auto parse (std::string_view s) -> std::optional<Num>;









    template<class T>
    struct is_smart_pointer : public std::false_type
    {
    };

    template<class T>
    struct is_smart_pointer<std::unique_ptr<T>> : public std::true_type
    {
    };

    template<class T>
    struct is_smart_pointer<std::shared_ptr<T>> : public std::true_type
    {
    };

    template<class T>
    auto constexpr is_smart_pointer_v = is_smart_pointer<T>::value;
}

#endif