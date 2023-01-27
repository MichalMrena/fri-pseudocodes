#include <libtuor/utils.hpp>
#include <algorithm>
#include <cctype>
#include <charconv>
#include <optional>
#include <sstream>

namespace fri
{
    auto to_words (std::string_view s) -> std::vector<std::string>
    {
        auto ws  = std::vector<std::string>();
        auto ist = std::stringstream(std::string(s));
        auto w   = std::string();
        while (ist >> w)
        {
            ws.emplace_back(std::move(w));
        }
        return ws;
    }

    template<class F>
    auto transform_string (std::string_view s, F f) -> std::string
    {
        auto result = std::string(size(s), '\0');
        std::transform(begin(s), end(s), begin(result), f);
        return result;
    }

    auto to_lowercase (std::string_view s) -> std::string
    {
        return transform_string(s, [](char const c)
        {
            return std::tolower(c);
        });
    }

    auto to_uppercase (std::string_view s) -> std::string
    {
        return transform_string(s, [](char const c)
        {
            return std::toupper(c);
        });
    }

    template<class Num>
    auto parse (std::string_view s) -> std::optional<Num>
    {
        auto ret    = Num{};
        auto result = std::from_chars(s.data(), s.data() + size(s), ret);
        auto const success = result.ec == std::errc()
                          && result.ptr == s.data() + size(s);
        return success ? std::optional<Num>(ret) : std::nullopt;
    }

    template auto parse<int32>(std::string_view) -> std::optional<int32>;
    template auto parse<int64>(std::string_view) -> std::optional<int64>;
    template auto parse<double>(std::string_view) -> std::optional<double>;
}