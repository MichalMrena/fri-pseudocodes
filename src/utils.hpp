#ifndef FRI_UTILS_HPP
#define FRI_UTILS_HPP

#include <iterator>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <charconv>
#include <cassert>

namespace fri
{
    inline auto to_words
        (std::string s) -> std::vector<std::string>
    {
        auto ws  = std::vector<std::string>();
        auto ist = std::stringstream(s);
        auto w   = std::string();
        while (ist >> w)
        {
            ws.emplace_back(std::move(w));
        }
        return ws;
    }

    template<class Num>
    struct parse_result
    {
        Num  value_;
        bool isValid_;
        operator Num  () const   { assert(isValid_); return value_; }
        operator bool () const   { return isValid_; }
        auto unsafe_get () const { return value_; }
    };

    template<class Num>
    auto parse (std::string_view in) -> parse_result<Num>
    {
        auto ret    = Num {};
        auto result = std::from_chars(in.data(), in.data() + in.size(), ret);
        return {ret, std::errc {} == result.ec && result.ptr == in.data() + in.size()};
    }

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