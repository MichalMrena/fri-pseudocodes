#include <iterator>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <charconv>
#include <cassert>

namespace fri
{
    auto to_words (std::string s) -> std::vector<std::string>
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


    // TODO toto sa asi ani nikde nepouziva, prec...
    template<class Iterator, class Predicate>
    class FilterIterator
    {
    public:
        using difference_type   = typename Iterator::defference_type;
        using value_type        = typename Iterator::value_type;
        using pointer           = typename Iterator::pointer;
        using reference         = typename Iterator::reference;
        using iterator_category = std::input_iterator_tag;

    public:
        FilterIterator (Iterator, Iterator, Predicate);

        auto operator*  () const -> reference;
        auto operator-> () const -> pointer;
        auto operator++ ()       -> FilterIterator&;
        auto operator++ (int)    -> FilterIterator;
        auto operator== (FilterIterator const&) const -> bool;
        auto operator!= (FilterIterator const&) const -> bool;

    private:
        Iterator  it_;
        Iterator  end_;
        Predicate p_;
    };

    template<class Iterator, class Predicate>
    FilterIterator<Iterator, Predicate>::FilterIterator
        (Iterator it, Iterator end, Predicate p) :
        it_  (it),
        end_ (end),
        p_   (p)
    {
    }

    template<class Iterator, class Predicate>
    auto FilterIterator<Iterator, Predicate>::operator*
        () const -> reference
    {
        return *it_;
    }

    template<class Iterator, class Predicate>
    auto FilterIterator<Iterator, Predicate>::operator->
        () const -> pointer
    {
        return std::addressof(**this);
    }

    template<class Iterator, class Predicate>
    auto FilterIterator<Iterator, Predicate>::operator++
        () -> FilterIterator&
    {
        while (it_ != end_ and not p_(*this))
        {
            ++it_;
        }
        return *this;
    }

    template<class Iterator, class Predicate>
    auto FilterIterator<Iterator, Predicate>::operator++
        (int) -> FilterIterator
    {
        auto const ret = *this;
        ++(*this);
        return ret;
    }

    template<class Iterator, class Predicate>
    auto FilterIterator<Iterator, Predicate>::operator==
        (FilterIterator const& rhs) const -> bool
    {
        return it_ == rhs.it_;
    }

    template<class Iterator, class Predicate>
    auto FilterIterator<Iterator, Predicate>::operator!=
        (FilterIterator const& rhs) const -> bool
    {
        return not (*this == rhs);
    }
}